/*
 * Copyright (C) 2006, 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nuanti Ltd.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "FocusController.h"

#include "AXObjectCache.h"
#include "Chrome.h"
#include "Document.h"
#include "Editor.h"
#include "EditorClient.h"
#include "Element.h"
#include "Event.h"
#include "EventHandler.h"
#include "EventNames.h"
#include "ExceptionCode.h"
#include "Frame.h"
#include "FrameTree.h"
#include "FrameView.h"
#include "HitTestResult.h"
#include "HTMLFrameOwnerElement.h"
#include "HTMLNames.h"
#include "KeyboardEvent.h"
#include "Page.h"
#include "Range.h"
#include "RenderLayer.h"
#include "RenderObject.h"
#include "RenderWidget.h"
#include "SelectionController.h"
#include "Settings.h"
#include "SpatialNavigation.h"
#include "Widget.h"

namespace WebCore {

using namespace HTMLNames;
using namespace std;

static void updateFocusCandidateIfNeeded(FocusDirection direction, const IntRect& startingRect, FocusCandidate& candidate, FocusCandidate& closest);
static inline void dispatchEventsOnWindowAndFocusedNode(Document* document, bool focused)
{
    // If we have a focused node we should dispatch blur on it before we blur the window.
    // If we have a focused node we should dispatch focus on it after we focus the window.
    // https://bugs.webkit.org/show_bug.cgi?id=27105

    // Do not fire events while modal dialogs are up.  See https://bugs.webkit.org/show_bug.cgi?id=33962
    if (Page* page = document->page()) {
        if (page->defersLoading())
            return;
    }

    if (!focused && document->focusedNode())
        document->focusedNode()->dispatchBlurEvent();
    document->dispatchWindowEvent(Event::create(focused ? eventNames().focusEvent : eventNames().blurEvent, false, false));
    if (focused && document->focusedNode())
        document->focusedNode()->dispatchFocusEvent();
}

FocusController::FocusController(Page* page)
    : m_page(page)
    , m_isActive(false)
    , m_isFocused(false)
    , m_isChangingFocusedFrame(false)
{
}

void FocusController::setFocusedFrame(PassRefPtr<Frame> frame)
{
    ASSERT(!frame || frame->page() == m_page);
    if (m_focusedFrame == frame || m_isChangingFocusedFrame)
        return;

    m_isChangingFocusedFrame = true;

    RefPtr<Frame> oldFrame = m_focusedFrame;
    RefPtr<Frame> newFrame = frame;

    m_focusedFrame = newFrame;

    // Now that the frame is updated, fire events and update the selection focused states of both frames.
    if (oldFrame && oldFrame->view()) {
        oldFrame->selection()->setFocused(false);
        oldFrame->document()->dispatchWindowEvent(Event::create(eventNames().blurEvent, false, false));
    }

    if (newFrame && newFrame->view() && isFocused()) {
        newFrame->selection()->setFocused(true);
        newFrame->document()->dispatchWindowEvent(Event::create(eventNames().focusEvent, false, false));
    }

    m_page->chrome()->focusedFrameChanged(newFrame.get());

    m_isChangingFocusedFrame = false;
}

Frame* FocusController::focusedOrMainFrame() const
{
    if (Frame* frame = focusedFrame())
        return frame;
    return m_page->mainFrame();
}

void FocusController::setFocused(bool focused)
{
    if (isFocused() == focused)
        return;
    
    m_isFocused = focused;

    if (!m_focusedFrame)
        setFocusedFrame(m_page->mainFrame());

    if (m_focusedFrame->view()) {
        m_focusedFrame->selection()->setFocused(focused);
        dispatchEventsOnWindowAndFocusedNode(m_focusedFrame->document(), focused);
    }
}

static Node* deepFocusableNode(FocusDirection direction, Node* node, KeyboardEvent* event)
{
    // The node we found might be a HTMLFrameOwnerElement, so descend down the frame tree until we find either:
    // 1) a focusable node, or
    // 2) the deepest-nested HTMLFrameOwnerElement
    while (node && node->isFrameOwnerElement()) {
        HTMLFrameOwnerElement* owner = static_cast<HTMLFrameOwnerElement*>(node);
        if (!owner->contentFrame())
            break;

        Document* document = owner->contentFrame()->document();

        node = (direction == FocusDirectionForward)
            ? document->nextFocusableNode(0, event)
            : document->previousFocusableNode(0, event);
        if (!node) {
            node = owner;
            break;
        }
    }

    return node;
}

bool FocusController::setInitialFocus(FocusDirection direction, KeyboardEvent* event)
{
    return advanceFocus(direction, event, true);
}

bool FocusController::advanceFocus(FocusDirection direction, KeyboardEvent* event, bool initialFocus)
{
    switch (direction) {
    case FocusDirectionForward:
    case FocusDirectionBackward:
        return advanceFocusInDocumentOrder(direction, event, initialFocus);
    case FocusDirectionLeft:
    case FocusDirectionRight:
    case FocusDirectionUp:
    case FocusDirectionDown:
        return advanceFocusDirectionally(direction, event);
    default:
        ASSERT_NOT_REACHED();
    }

    return false;
}

bool FocusController::advanceFocusInDocumentOrder(FocusDirection direction, KeyboardEvent* event, bool initialFocus)
{
    Frame* frame = focusedOrMainFrame();
    ASSERT(frame);
    Document* document = frame->document();

    Node* currentNode = document->focusedNode();
    // FIXME: Not quite correct when it comes to focus transitions leaving/entering the WebView itself
    bool caretBrowsing = focusedOrMainFrame()->settings()->caretBrowsingEnabled();

    if (caretBrowsing && !currentNode)
        currentNode = frame->selection()->start().node();

    document->updateLayoutIgnorePendingStylesheets();

    Node* node = (direction == FocusDirectionForward)
        ? document->nextFocusableNode(currentNode, event)
        : document->previousFocusableNode(currentNode, event);
            
    // If there's no focusable node to advance to, move up the frame tree until we find one.
    while (!node && frame) {
        Frame* parentFrame = frame->tree()->parent();
        if (!parentFrame)
            break;

        Document* parentDocument = parentFrame->document();

        HTMLFrameOwnerElement* owner = frame->ownerElement();
        if (!owner)
            break;

        node = (direction == FocusDirectionForward)
            ? parentDocument->nextFocusableNode(owner, event)
            : parentDocument->previousFocusableNode(owner, event);

        frame = parentFrame;
    }

    node = deepFocusableNode(direction, node, event);

    if (!node) {
        // We didn't find a node to focus, so we should try to pass focus to Chrome.
        if (!initialFocus && m_page->chrome()->canTakeFocus(direction)) {
            document->setFocusedNode(0);
            setFocusedFrame(0);
            m_page->chrome()->takeFocus(direction);
            return true;
        }

        // Chrome doesn't want focus, so we should wrap focus.
        Document* d = m_page->mainFrame()->document();
        node = (direction == FocusDirectionForward)
            ? d->nextFocusableNode(0, event)
            : d->previousFocusableNode(0, event);

        node = deepFocusableNode(direction, node, event);

        if (!node)
            return false;
    }

    ASSERT(node);

    if (node == document->focusedNode())
        // Focus wrapped around to the same node.
        return true;

    if (!node->isElementNode())
        // FIXME: May need a way to focus a document here.
        return false;

    if (node->isFrameOwnerElement()) {
        // We focus frames rather than frame owners.
        // FIXME: We should not focus frames that have no scrollbars, as focusing them isn't useful to the user.
        HTMLFrameOwnerElement* owner = static_cast<HTMLFrameOwnerElement*>(node);
        if (!owner->contentFrame())
            return false;

        document->setFocusedNode(0);
        setFocusedFrame(owner->contentFrame());
        return true;
    }
    
    // FIXME: It would be nice to just be able to call setFocusedNode(node) here, but we can't do
    // that because some elements (e.g. HTMLInputElement and HTMLTextAreaElement) do extra work in
    // their focus() methods.

    Document* newDocument = node->document();

    if (newDocument != document)
        // Focus is going away from this document, so clear the focused node.
        document->setFocusedNode(0);

    if (newDocument)
        setFocusedFrame(newDocument->frame());

    if (caretBrowsing) {
        VisibleSelection newSelection(Position(node, 0), Position(node, 0), DOWNSTREAM);
        if (frame->selection()->shouldChangeSelection(newSelection))
            frame->selection()->setSelection(newSelection);
    }

    static_cast<Element*>(node)->focus(false);
    return true;
}

static bool relinquishesEditingFocus(Node *node)
{
    ASSERT(node);
    ASSERT(node->isContentEditable());

    Node* root = node->rootEditableElement();
    Frame* frame = node->document()->frame();
    if (!frame || !root)
        return false;

    return frame->editor()->shouldEndEditing(rangeOfContents(root).get());
}

static void clearSelectionIfNeeded(Frame* oldFocusedFrame, Frame* newFocusedFrame, Node* newFocusedNode)
{
    if (!oldFocusedFrame || !newFocusedFrame)
        return;
        
    if (oldFocusedFrame->document() != newFocusedFrame->document())
        return;
    
    SelectionController* s = oldFocusedFrame->selection();
    if (s->isNone())
        return;

    bool caretBrowsing = oldFocusedFrame->settings()->caretBrowsingEnabled();
    if (caretBrowsing)
        return;

    Node* selectionStartNode = s->selection().start().node();
    if (selectionStartNode == newFocusedNode || selectionStartNode->isDescendantOf(newFocusedNode) || selectionStartNode->shadowAncestorNode() == newFocusedNode)
        return;
        
    if (Node* mousePressNode = newFocusedFrame->eventHandler()->mousePressNode()) {
        if (mousePressNode->renderer() && !mousePressNode->canStartSelection()) {
            // Don't clear the selection for contentEditable elements, but do clear it for input and textarea. See bug 38696.
            Node * root = s->rootEditableElement();
            if (!root)
                return;

            if (Node* shadowAncestorNode = root->shadowAncestorNode()) {
                if (!shadowAncestorNode->hasTagName(inputTag) && !shadowAncestorNode->hasTagName(textareaTag))
                    return;
            }
        }
    }
    
    s->clear();
}

bool FocusController::setFocusedNode(Node* node, PassRefPtr<Frame> newFocusedFrame)
{
    RefPtr<Frame> oldFocusedFrame = focusedFrame();
    RefPtr<Document> oldDocument = oldFocusedFrame ? oldFocusedFrame->document() : 0;
    
    Node* oldFocusedNode = oldDocument ? oldDocument->focusedNode() : 0;
    if (oldFocusedNode == node)
        return true;

    // FIXME: Might want to disable this check for caretBrowsing
    if (oldFocusedNode && oldFocusedNode->rootEditableElement() == oldFocusedNode && !relinquishesEditingFocus(oldFocusedNode))
        return false;

    m_page->editorClient()->willSetInputMethodState();

    clearSelectionIfNeeded(oldFocusedFrame.get(), newFocusedFrame.get(), node);

    if (!node) {
        if (oldDocument)
            oldDocument->setFocusedNode(0);
        m_page->editorClient()->setInputMethodState(false);
        return true;
    }

    RefPtr<Document> newDocument = node->document();

    if (newDocument && newDocument->focusedNode() == node) {
        m_page->editorClient()->setInputMethodState(node->shouldUseInputMethod());
        return true;
    }
    
    if (oldDocument && oldDocument != newDocument)
        oldDocument->setFocusedNode(0);
    
    setFocusedFrame(newFocusedFrame);

    // Setting the focused node can result in losing our last reft to node when JS event handlers fire.
    RefPtr<Node> protect = node;
    if (newDocument) {
        bool successfullyFocused = newDocument->setFocusedNode(node);
        if (!successfullyFocused)
            return false;
    }

    if (newDocument->focusedNode() == node)
        m_page->editorClient()->setInputMethodState(node->shouldUseInputMethod());

    return true;
}

void FocusController::setActive(bool active)
{
    if (m_isActive == active)
        return;

    m_isActive = active;

    if (FrameView* view = m_page->mainFrame()->view()) {
        if (!view->platformWidget()) {
            view->updateLayoutAndStyleIfNeededRecursive();
            view->updateControlTints();
        }
    }

    focusedOrMainFrame()->selection()->pageActivationChanged();
    
    if (m_focusedFrame && isFocused())
        dispatchEventsOnWindowAndFocusedNode(m_focusedFrame->document(), active);
}

void updateFocusCandidateIfNeeded(FocusDirection direction, const IntRect& startingRect, FocusCandidate& candidate, FocusCandidate& closest)
{
    if (!candidate.node->isElementNode() || !candidate.node->renderer())
        return;

    // Ignore iframes that don't have a src attribute
    if (candidate.node->isFrameOwnerElement() && !static_cast<HTMLFrameOwnerElement*>(candidate.node)->contentFrame())
        return;

    // Ignore off screen child nodes of containers that do not scroll (overflow:hidden)
    if (hasOffscreenRect(candidate.node) && !canBeScrolledIntoView(direction, candidate))
        return;

    FocusCandidate current;
    current.rect = startingRect;
    distanceDataForNode(direction, current, candidate);
    if (candidate.distance == maxDistance())
        return;

    if (hasOffscreenRect(candidate.node, direction) && candidate.alignment < Full)
        return;

    if (closest.isNull()) {
        closest = candidate;
        return;
    }

    IntRect intersectionRect = intersection(nodeRectInAbsoluteCoordinates(candidate.node, true), nodeRectInAbsoluteCoordinates(closest.node, true));
    if (!intersectionRect.isEmpty()) {
        // If 2 nodes are intersecting, do hit test to find which node in on top.
        int x = intersectionRect.x() + intersectionRect.width() / 2;
        int y = intersectionRect.y() + intersectionRect.height() / 2;
        HitTestResult result = candidate.node->document()->page()->mainFrame()->eventHandler()->hitTestResultAtPoint(IntPoint(x, y), false, true);
        if (candidate.node->contains(result.innerNode())) {
            closest = candidate;
            return;
        }
        if (closest.node->contains(result.innerNode()))
            return;
    }

    if (candidate.alignment == closest.alignment) {
        if (candidate.distance < closest.distance)
            closest = candidate;
        return;
    }

    if (candidate.alignment > closest.alignment)
        closest = candidate;
}

void FocusController::findFocusCandidateInContainer(Node* container, const IntRect& startingRect, FocusDirection direction, KeyboardEvent* event, FocusCandidate& closest)
{
    ASSERT(container);
    Node* focusedNode = (focusedFrame() && focusedFrame()->document()) ? focusedFrame()->document()->focusedNode() : 0;

    Node* node = container->firstChild();
    for (; node; node = (node->isFrameOwnerElement() || canScrollInDirection(direction, node)) ? node->traverseNextSibling(container) : node->traverseNextNode(container)) {
        if (node == focusedNode)
            continue;

        if (!node->renderer())
            continue;

        if (!node->isKeyboardFocusable(event) && !node->isFrameOwnerElement() && !canScrollInDirection(direction, node))
            continue;

        FocusCandidate candidate(node);
        candidate.enclosingScrollableBox = container;
        updateFocusCandidateIfNeeded(direction, startingRect, candidate, closest);
    }
}

bool FocusController::advanceFocusDirectionallyInContainer(Node* container, const IntRect& startingRect, FocusDirection direction, KeyboardEvent* event)
{
    if (!container || !container->document())
        return false;

    IntRect newStartingRect = startingRect;

    if (startingRect.isEmpty())
        newStartingRect = virtualRectForDirection(direction, nodeRectInAbsoluteCoordinates(container));

    // Find the closest node within current container in the direction of the navigation.
    FocusCandidate focusCandidate;
    findFocusCandidateInContainer(container, newStartingRect, direction, event, focusCandidate);

    if (focusCandidate.isNull()) {
        if (canScrollInDirection(direction, container)) {
            // Nothing to focus, scroll if possible.
            scrollInDirection(container, direction);
            return true;
        }
        // Return false will cause a re-try, skipping this container.
        return false;
    }
    if (focusCandidate.node->isFrameOwnerElement()) {
        HTMLFrameOwnerElement* frameElement = static_cast<HTMLFrameOwnerElement*>(focusCandidate.node);
        // If we have an iframe without the src attribute, it will not have a contentFrame().
        // We ASSERT here to make sure that
        // updateFocusCandidateIfNeeded() will never consider such an iframe as a candidate.
        ASSERT(frameElement->contentFrame());

        if (hasOffscreenRect(focusCandidate.node, direction)) {
            scrollInDirection(focusCandidate.node->document(), direction);
            return true;
        }
        // Navigate into a new frame.
        IntRect rect;
        Node* focusedNode = focusedOrMainFrame()->document()->focusedNode();
        if (focusedNode && !hasOffscreenRect(focusedNode))
            rect = nodeRectInAbsoluteCoordinates(focusedNode, true /* ignore border */);
        frameElement->contentFrame()->document()->updateLayoutIgnorePendingStylesheets();
        if (!advanceFocusDirectionallyInContainer(frameElement->contentFrame()->document(), rect, direction, event)) {
            // The new frame had nothing interesting, need to find another candidate.
            return advanceFocusDirectionallyInContainer(container, nodeRectInAbsoluteCoordinates(focusCandidate.node, true), direction, event);
        }
        return true;
    }
    if (canScrollInDirection(direction, focusCandidate.node)) {
        if (hasOffscreenRect(focusCandidate.node, direction)) {
            scrollInDirection(focusCandidate.node, direction);
            return true;
        }
        // Navigate into a new scrollable container.
        IntRect startingRect;
        Node* focusedNode = focusedOrMainFrame()->document()->focusedNode();
        if (focusedNode && !hasOffscreenRect(focusedNode))
            startingRect = nodeRectInAbsoluteCoordinates(focusedNode, true);
        return advanceFocusDirectionallyInContainer(focusCandidate.node, startingRect, direction, event);
    }
    if (hasOffscreenRect(focusCandidate.node, direction)) {
        Node* container = focusCandidate.enclosingScrollableBox;
        scrollInDirection(container, direction);
        return true;
    }

    // We found a new focus node, navigate to it.
    Element* element = toElement(focusCandidate.node);
    ASSERT(element);

    element->focus(false);
    return true;
}

bool FocusController::advanceFocusDirectionally(FocusDirection direction, KeyboardEvent* event)
{
    Frame* curFrame = focusedOrMainFrame();
    ASSERT(curFrame);

    Document* focusedDocument = curFrame->document();
    if (!focusedDocument)
        return false;

    Node* focusedNode = focusedDocument->focusedNode();
    Node* container = focusedDocument;

    // Figure out the starting rect.
    IntRect startingRect;
    if (focusedNode && !hasOffscreenRect(focusedNode)) {
        container = scrollableEnclosingBoxOrParentFrameForNodeInDirection(direction, focusedNode);
        startingRect = nodeRectInAbsoluteCoordinates(focusedNode, true /* ignore border */);
    }

    bool consumed = false;
    do {
        if (container->isDocumentNode())
            static_cast<Document*>(container)->updateLayoutIgnorePendingStylesheets();
        consumed = advanceFocusDirectionallyInContainer(container, startingRect, direction, event);
        startingRect = nodeRectInAbsoluteCoordinates(container, true /* ignore border */);
        container = scrollableEnclosingBoxOrParentFrameForNodeInDirection(direction, container);
    } while (!consumed && container);

    return consumed;
}

} // namespace WebCore
