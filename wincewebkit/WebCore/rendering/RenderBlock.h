/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2007 David Smith (catfish.man@gmail.com)
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef RenderBlock_h
#define RenderBlock_h

#include "DeprecatedPtrList.h"
#include "GapRects.h"
#include "RenderBox.h"
#include "RenderLineBoxList.h"
#include "RootInlineBox.h"
#include <wtf/ListHashSet.h>

namespace WebCore {

class ColumnInfo;
class InlineIterator;
class LayoutStateMaintainer;
class RenderInline;

struct BidiRun;

template <class Iterator, class Run> class BidiResolver;
template <class Iterator> struct MidpointState;
typedef BidiResolver<InlineIterator, BidiRun> InlineBidiResolver;
typedef MidpointState<InlineIterator> LineMidpointState;

enum CaretType { CursorCaret, DragCaret };

class RenderBlock : public RenderBox {
public:
    RenderBlock(Node*);
    virtual ~RenderBlock();

    const RenderObjectChildList* children() const { return &m_children; }
    RenderObjectChildList* children() { return &m_children; }

    virtual void destroy();

    // These two functions are overridden for inline-block.
    virtual int lineHeight(bool firstLine, LineDirectionMode, LinePositionMode = PositionOnContainingLine) const;
    virtual int baselinePosition(FontBaseline, bool firstLine, LineDirectionMode, LinePositionMode = PositionOnContainingLine) const;

    RenderLineBoxList* lineBoxes() { return &m_lineBoxes; }
    const RenderLineBoxList* lineBoxes() const { return &m_lineBoxes; }

    InlineFlowBox* firstLineBox() const { return m_lineBoxes.firstLineBox(); }
    InlineFlowBox* lastLineBox() const { return m_lineBoxes.lastLineBox(); }

    void deleteLineBoxTree();

    virtual void addChild(RenderObject* newChild, RenderObject* beforeChild = 0);
    virtual void removeChild(RenderObject*);

    virtual void layoutBlock(bool relayoutChildren, int pageHeight = 0);

    void insertPositionedObject(RenderBox*);
    void removePositionedObject(RenderBox*);
    void removePositionedObjects(RenderBlock*);

    typedef ListHashSet<RenderBox*, 4> PositionedObjectsListHashSet;
    PositionedObjectsListHashSet* positionedObjects() const { return m_positionedObjects; }

    void addPercentHeightDescendant(RenderBox*);
    static void removePercentHeightDescendant(RenderBox*);
    HashSet<RenderBox*>* percentHeightDescendants() const;

    RootInlineBox* createAndAppendRootInlineBox();

    bool generatesLineBoxesForInlineChild(RenderObject*, bool isLineEmpty = true, bool previousLineBrokeCleanly = true);

    void markAllDescendantsWithFloatsForLayout(RenderBox* floatToRemove = 0, bool inLayout = true);
    void markPositionedObjectsForLayout();
    virtual void markForPaginationRelayoutIfNeeded();
    
    bool containsFloats() { return m_floatingObjects && !m_floatingObjects->isEmpty(); }
    bool containsFloat(RenderObject*);

    int availableLogicalWidthForLine(int position, bool firstLine) const;
    int logicalRightOffsetForLine(int position, bool firstLine) const { return logicalRightOffsetForLine(position, logicalRightOffsetForContent(), firstLine); }
    int logicalLeftOffsetForLine(int position, bool firstLine) const { return logicalLeftOffsetForLine(position, logicalLeftOffsetForContent(), firstLine); }

    virtual int topmostPosition(bool includeOverflowInterior = true, bool includeSelf = true, ApplyTransform = IncludeTransform) const;
    virtual int lowestPosition(bool includeOverflowInterior = true, bool includeSelf = true, ApplyTransform = IncludeTransform) const;
    virtual int rightmostPosition(bool includeOverflowInterior = true, bool includeSelf = true, ApplyTransform = IncludeTransform) const;
    virtual int leftmostPosition(bool includeOverflowInterior = true, bool includeSelf = true, ApplyTransform = IncludeTransform) const;
    
    virtual VisiblePosition positionForPoint(const IntPoint&);
    
    // Block flows subclass availableWidth to handle multi column layout (shrinking the width available to children when laying out.)
    virtual int availableLogicalWidth() const;

    RootInlineBox* firstRootBox() const { return static_cast<RootInlineBox*>(firstLineBox()); }
    RootInlineBox* lastRootBox() const { return static_cast<RootInlineBox*>(lastLineBox()); }

    bool containsNonZeroBidiLevel() const;

    GapRects selectionGapRectsForRepaint(RenderBoxModelObject* repaintContainer);
    IntRect logicalLeftSelectionGap(RenderBlock* rootBlock, const IntPoint& rootBlockPhysicalPosition, const IntSize& offsetFromRootBlock,
                                    RenderObject* selObj, int logicalLeft, int logicalTop, int logicalHeight, const PaintInfo*);
    IntRect logicalRightSelectionGap(RenderBlock* rootBlock, const IntPoint& rootBlockPhysicalPosition, const IntSize& offsetFromRootBlock,
                                     RenderObject* selObj, int logicalRight, int logicalTop, int logicalHeight, const PaintInfo*);
    void getSelectionGapInfo(SelectionState, bool& leftGap, bool& rightGap);
    IntRect logicalRectToPhysicalRect(const IntPoint& physicalPosition, const IntRect& logicalRect);
        
    // Helper methods for computing line counts and heights for line counts.
    RootInlineBox* lineAtIndex(int);
    int lineCount();
    int heightForLineCount(int);
    void clearTruncation();

    void adjustRectForColumns(IntRect&) const;
    virtual void adjustForColumns(IntSize&, const IntPoint&) const;

    void addContinuationWithOutline(RenderInline*);

    RenderBoxModelObject* continuation() const { return m_continuation; }
    void setContinuation(RenderBoxModelObject* c) { m_continuation = c; }
    virtual RenderBoxModelObject* virtualContinuation() const { return continuation(); }
    bool isAnonymousBlockContinuation() const { return continuation() && isAnonymousBlock(); }
    RenderInline* inlineElementContinuation() const;
    RenderBlock* blockElementContinuation() const;

    // This function is a convenience helper for creating an anonymous block that inherits its
    // style from this RenderBlock.
    RenderBlock* createAnonymousBlock(bool isFlexibleBox = false) const;
    RenderBlock* createAnonymousColumnsBlock() const;
    RenderBlock* createAnonymousColumnSpanBlock() const;
    RenderBlock* createAnonymousBlockWithSameTypeAs(RenderBlock* otherAnonymousBlock) const;
    
    static void appendRunsForObject(int start, int end, RenderObject*, InlineBidiResolver&);    
    static bool requiresLineBox(const InlineIterator&, bool isLineEmpty = true, bool previousLineBrokeCleanly = true);

    ColumnInfo* columnInfo() const;
    int columnGap() const;
    
    // These two functions take the ColumnInfo* to avoid repeated lookups of the info in the global HashMap.
    unsigned columnCount(ColumnInfo*) const;
    IntRect columnRectAt(ColumnInfo*, unsigned) const;

    int paginationStrut() const { return m_rareData ? m_rareData->m_paginationStrut : 0; }
    int pageY() const { return m_rareData ? m_rareData->m_pageY : 0; }
    void setPaginationStrut(int strut);
    void setPageY(int y);

    // Accessors for logical width/height and margins in the containing block's block-flow direction.
    enum ApplyLayoutDeltaMode { ApplyLayoutDelta, DoNotApplyLayoutDelta };
    int logicalWidthForChild(RenderBox* child) { return style()->isHorizontalWritingMode() ? child->width() : child->height(); }
    int logicalHeightForChild(RenderBox* child) { return style()->isHorizontalWritingMode() ? child->height() : child->width(); }
    int logicalTopForChild(RenderBox* child) { return style()->isHorizontalWritingMode() ? child->y() : child->x(); }
    void setLogicalLeftForChild(RenderBox* child, int logicalLeft, ApplyLayoutDeltaMode = DoNotApplyLayoutDelta);
    void setLogicalTopForChild(RenderBox* child, int logicalTop, ApplyLayoutDeltaMode = DoNotApplyLayoutDelta);
    int marginBeforeForChild(RenderBoxModelObject* child) const;
    int marginAfterForChild(RenderBoxModelObject* child) const;
    int marginStartForChild(RenderBoxModelObject* child) const;
    int marginEndForChild(RenderBoxModelObject* child) const;
    void setMarginStartForChild(RenderBox* child, int);
    void setMarginEndForChild(RenderBox* child, int);
    void setMarginBeforeForChild(RenderBox* child, int);
    void setMarginAfterForChild(RenderBox* child, int);
    int collapsedMarginBeforeForChild(RenderBox* child) const;
    int collapsedMarginAfterForChild(RenderBox* child) const;

    virtual void updateFirstLetter();

    class MarginValues {
    public:
        MarginValues(int beforePos, int beforeNeg, int afterPos, int afterNeg)
            : m_positiveMarginBefore(beforePos)
            , m_negativeMarginBefore(beforeNeg)
            , m_positiveMarginAfter(afterPos)
            , m_negativeMarginAfter(afterNeg)
        { }
        
        int positiveMarginBefore() const { return m_positiveMarginBefore; }
        int negativeMarginBefore() const { return m_negativeMarginBefore; }
        int positiveMarginAfter() const { return m_positiveMarginAfter; }
        int negativeMarginAfter() const { return m_negativeMarginAfter; }
        
        void setPositiveMarginBefore(int pos) { m_positiveMarginBefore = pos; }
        void setNegativeMarginBefore(int neg) { m_negativeMarginBefore = neg; }
        void setPositiveMarginAfter(int pos) { m_positiveMarginAfter = pos; }
        void setNegativeMarginAfter(int neg) { m_negativeMarginAfter = neg; }
    
    private:
        int m_positiveMarginBefore;
        int m_negativeMarginBefore;
        int m_positiveMarginAfter;
        int m_negativeMarginAfter;
    };
    MarginValues marginValuesForChild(RenderBox* child);

protected:
    // These functions are only used internally to manipulate the render tree structure via remove/insert/appendChildNode.
    // Since they are typically called only to move objects around within anonymous blocks (which only have layers in
    // the case of column spans), the default for fullRemoveInsert is false rather than true.
    void moveChildTo(RenderBlock* to, RenderObject* child, bool fullRemoveInsert = false)
    {
        return moveChildTo(to, child, 0, fullRemoveInsert);
    }
    void moveChildTo(RenderBlock* to, RenderObject* child, RenderObject* beforeChild, bool fullRemoveInsert = false);
    void moveAllChildrenTo(RenderBlock* to, bool fullRemoveInsert = false)
    {
        return moveAllChildrenTo(to, 0, fullRemoveInsert);
    }
    void moveAllChildrenTo(RenderBlock* to, RenderObject* beforeChild, bool fullRemoveInsert = false)
    {
        return moveChildrenTo(to, firstChild(), 0, beforeChild, fullRemoveInsert);
    }
    // Move all of the kids from |startChild| up to but excluding |endChild|.  0 can be passed as the endChild to denote
    // that all the kids from |startChild| onwards should be added.
    void moveChildrenTo(RenderBlock* to, RenderObject* startChild, RenderObject* endChild, bool fullRemoveInsert = false)
    {
        return moveChildrenTo(to, startChild, endChild, 0, fullRemoveInsert);
    }
    void moveChildrenTo(RenderBlock* to, RenderObject* startChild, RenderObject* endChild, RenderObject* beforeChild, bool fullRemoveInsert = false);
    
    int maxPositiveMarginBefore() const { return m_rareData ? m_rareData->m_margins.positiveMarginBefore() : RenderBlockRareData::positiveMarginBeforeDefault(this); }
    int maxNegativeMarginBefore() const { return m_rareData ? m_rareData->m_margins.negativeMarginBefore() : RenderBlockRareData::negativeMarginBeforeDefault(this); }
    int maxPositiveMarginAfter() const { return m_rareData ? m_rareData->m_margins.positiveMarginAfter() : RenderBlockRareData::positiveMarginAfterDefault(this); }
    int maxNegativeMarginAfter() const { return m_rareData ? m_rareData->m_margins.negativeMarginAfter() : RenderBlockRareData::negativeMarginAfterDefault(this); }
    
    void setMaxMarginBeforeValues(int pos, int neg);
    void setMaxMarginAfterValues(int pos, int neg);

    void initMaxMarginValues()
    {
        if (m_rareData) {
            m_rareData->m_margins = MarginValues(RenderBlockRareData::positiveMarginBeforeDefault(this) , RenderBlockRareData::negativeMarginBeforeDefault(this),
                                                 RenderBlockRareData::positiveMarginAfterDefault(this), RenderBlockRareData::negativeMarginAfterDefault(this));
            m_rareData->m_paginationStrut = 0;
        }
    }

    virtual void layout();

    void layoutPositionedObjects(bool relayoutChildren);

    virtual void paint(PaintInfo&, int tx, int ty);
    virtual void paintObject(PaintInfo&, int tx, int ty);

    int logicalRightOffsetForContent() const { return style()->isHorizontalWritingMode() ? borderLeft() + paddingLeft() + availableLogicalWidth() : borderTop() + paddingTop() + availableLogicalWidth(); }
    int logicalLeftOffsetForContent() const { return style()->isHorizontalWritingMode() ? borderLeft() + paddingLeft() : borderTop() + paddingTop(); }
    int logicalRightOffsetForLine(int position, int fixedOffset, bool applyTextIndent = true, int* logicalHeightRemaining = 0) const;
    int logicalLeftOffsetForLine(int position, int fixedOffset, bool applyTextIndent = true, int* logicalHeightRemaining = 0) const;

    virtual bool nodeAtPoint(const HitTestRequest&, HitTestResult&, int x, int y, int tx, int ty, HitTestAction);

    virtual void computePreferredLogicalWidths();

    virtual int firstLineBoxBaseline() const;
    virtual int lastLineBoxBaseline() const;

    virtual void updateHitTestResult(HitTestResult&, const IntPoint&);

    // Delay update scrollbar until finishDelayRepaint() will be
    // called. This function is used when a flexbox is laying out its
    // descendant. If multiple calls are made to startDelayRepaint(),
    // finishDelayRepaint() will do nothing until finishDelayRepaint()
    // is called the same number of times.
    static void startDelayUpdateScrollInfo();
    static void finishDelayUpdateScrollInfo();

    virtual void styleWillChange(StyleDifference, const RenderStyle* newStyle);
    virtual void styleDidChange(StyleDifference, const RenderStyle* oldStyle);

    virtual bool hasLineIfEmpty() const;
    bool layoutOnlyPositionedObjects();

#if ENABLE(SVG)
protected:

    // Only used by RenderSVGText, which explicitely overrides RenderBlock::layoutBlock(), do NOT use for anything else.
    void forceLayoutInlineChildren()
    {
        int repaintLogicalTop = 0;
        int repaintLogicalBottom = 0;
        layoutInlineChildren(true, repaintLogicalTop, repaintLogicalBottom);
    }
#endif

    void addOverflowFromBlockChildren();

private:
    virtual RenderObjectChildList* virtualChildren() { return children(); }
    virtual const RenderObjectChildList* virtualChildren() const { return children(); }

    virtual const char* renderName() const;

    virtual bool isRenderBlock() const { return true; }
    virtual bool isBlockFlow() const { return (!isInline() || isReplaced()) && !isTable(); }
    virtual bool isInlineBlockOrInlineTable() const { return isInline() && isReplaced(); }

    void makeChildrenNonInline(RenderObject* insertionPoint = 0);
    virtual void removeLeftoverAnonymousBlock(RenderBlock* child);

    virtual void dirtyLinesFromChangedChild(RenderObject* child) { m_lineBoxes.dirtyLinesFromChangedChild(this, child); }

    void addChildToContinuation(RenderObject* newChild, RenderObject* beforeChild);
    void addChildIgnoringContinuation(RenderObject* newChild, RenderObject* beforeChild);
    void addChildToAnonymousColumnBlocks(RenderObject* newChild, RenderObject* beforeChild);
    virtual void addChildIgnoringAnonymousColumnBlocks(RenderObject* newChild, RenderObject* beforeChild = 0);
    
    virtual bool isSelfCollapsingBlock() const;

    virtual int collapsedMarginBefore() const { return maxPositiveMarginBefore() - maxNegativeMarginBefore(); }
    virtual int collapsedMarginAfter() const { return maxPositiveMarginAfter() - maxNegativeMarginAfter(); }

    virtual void repaintOverhangingFloats(bool paintAllDescendants);

    void layoutBlockChildren(bool relayoutChildren, int& maxFloatLogicalBottom);
    void layoutInlineChildren(bool relayoutChildren, int& repaintLogicalTop, int& repaintLogicalBottom);

    virtual void positionListMarker() { }

    virtual void borderFitAdjust(int& x, int& w) const; // Shrink the box in which the border paints if border-fit is set.

    virtual void updateBeforeAfterContent(PseudoId);

    virtual RootInlineBox* createRootInlineBox(); // Subclassed by SVG and Ruby.

    // Called to lay out the legend for a fieldset or the ruby text of a ruby run.
    virtual RenderObject* layoutSpecialExcludedChild(bool /*relayoutChildren*/) { return 0; }

    struct FloatWithRect {
        FloatWithRect(RenderBox* f)
            : object(f)
            , rect(IntRect(f->x() - f->marginLeft(), f->y() - f->marginTop(), f->width() + f->marginLeft() + f->marginRight(), f->height() + f->marginTop() + f->marginBottom()))
            , everHadLayout(f->m_everHadLayout)
        {
        }

        RenderBox* object;
        IntRect rect;
        bool everHadLayout;
    };

    struct FloatingObject : Noncopyable {
        // Note that Type uses bits so you can use FloatBoth as a mask to query for both left and right.
        enum Type { FloatLeft = 1, FloatRight = 2, FloatBoth = 3 };

        FloatingObject(Type type)
            : m_renderer(0)
            , m_paginationStrut(0)
            , m_type(type)
            , m_shouldPaint(true)
            , m_isDescendant(false)
            , m_isPlaced(false)
        {
        }

        FloatingObject(Type type, const IntRect& frameRect)
            : m_renderer(0)
            , m_frameRect(frameRect)
            , m_paginationStrut(0)
            , m_type(type)
            , m_shouldPaint(true)
            , m_isDescendant(false)
            , m_isPlaced(true)
        {
        }

        Type type() const { return static_cast<Type>(m_type); }
        RenderBox* renderer() const { return m_renderer; }
        
        bool isPlaced() const { return m_isPlaced; }
        void setIsPlaced(bool placed = true) { m_isPlaced = placed; }

        int left() const { ASSERT(isPlaced()); return m_frameRect.x(); }
        int right() const { ASSERT(isPlaced()); return m_frameRect.right(); }
        int top() const { ASSERT(isPlaced()); return m_frameRect.y(); }
        int bottom() const { ASSERT(isPlaced()); return m_frameRect.bottom(); }
        int width() const { return m_frameRect.width(); }
        int height() const { return m_frameRect.height(); }
    
        void setLeft(int left) { m_frameRect.setX(left); }
        void setTop(int top) { m_frameRect.setY(top); }
        void setWidth(int width) { m_frameRect.setWidth(width); }
        void setHeight(int height) { m_frameRect.setHeight(height); }

        const IntRect& frameRect() const { ASSERT(isPlaced()); return m_frameRect; }
        void setFrameRect(const IntRect& frameRect) { m_frameRect = frameRect; }

        RenderBox* m_renderer;
        IntRect m_frameRect;
        int m_paginationStrut;
        unsigned m_type : 2; // Type (left or right aligned)
        bool m_shouldPaint : 1;
        bool m_isDescendant : 1;
        bool m_isPlaced : 1;
    };

    int logicalTopForFloat(FloatingObject* child) const { return style()->isHorizontalWritingMode() ? child->top() : child->left(); }
    int logicalBottomForFloat(FloatingObject* child) const { return style()->isHorizontalWritingMode() ? child->bottom() : child->right(); }
    int logicalLeftForFloat(FloatingObject* child) const { return style()->isHorizontalWritingMode() ? child->left() : child->top(); }
    int logicalRightForFloat(FloatingObject* child) const { return style()->isHorizontalWritingMode() ? child->right() : child->bottom(); }
    int logicalWidthForFloat(FloatingObject* child) const { return style()->isHorizontalWritingMode() ? child->width() : child->height(); }
    void setLogicalTopForFloat(FloatingObject* child, int logicalTop)
    {
        if (style()->isHorizontalWritingMode())
            child->setTop(logicalTop);
        else
            child->setLeft(logicalTop);
    }
    void setLogicalLeftForFloat(FloatingObject* child, int logicalLeft)
    {
        if (style()->isHorizontalWritingMode())
            child->setLeft(logicalLeft);
        else
            child->setTop(logicalLeft);
    }
    void setLogicalHeightForFloat(FloatingObject* child, int logicalHeight)
    {
        if (style()->isHorizontalWritingMode())
            child->setHeight(logicalHeight);
        else
            child->setWidth(logicalHeight);
    }
    void setLogicalWidthForFloat(FloatingObject* child, int logicalWidth)
    {
        if (style()->isHorizontalWritingMode())
            child->setWidth(logicalWidth);
        else
            child->setHeight(logicalWidth);
    }

    // The following functions' implementations are in RenderBlockLineLayout.cpp.
    RootInlineBox* determineStartPosition(bool& firstLine, bool& fullLayout, bool& previousLineBrokeCleanly,
                                          InlineBidiResolver&, Vector<FloatWithRect>& floats, unsigned& numCleanFloats,
                                          bool& useRepaintBounds, int& repaintTop, int& repaintBottom);
    RootInlineBox* determineEndPosition(RootInlineBox* startBox, InlineIterator& cleanLineStart,
                                        BidiStatus& cleanLineBidiStatus,
                                        int& yPos);
    bool matchedEndLine(const InlineBidiResolver&, const InlineIterator& endLineStart, const BidiStatus& endLineStatus,
                        RootInlineBox*& endLine, int& endYPos, int& repaintBottom, int& repaintTop);

    void skipTrailingWhitespace(InlineIterator&, bool isLineEmpty, bool previousLineBrokeCleanly);
    int skipLeadingWhitespace(InlineBidiResolver&, bool firstLine, bool isLineEmpty, bool previousLineBrokeCleanly, FloatingObject* lastFloatFromPreviousLine);
    void fitBelowFloats(int widthToFit, bool firstLine, int& availableWidth);
    InlineIterator findNextLineBreak(InlineBidiResolver&, bool firstLine, bool& isLineEmpty, bool& previousLineBrokeCleanly, bool& hyphenated, EClear*, FloatingObject* lastFloatFromPreviousLine);
    RootInlineBox* constructLine(unsigned runCount, BidiRun* firstRun, BidiRun* lastRun, bool firstLine, bool lastLine, RenderObject* endObject);
    InlineFlowBox* createLineBoxes(RenderObject*, bool firstLine);
    void computeInlineDirectionPositionsForLine(RootInlineBox*, bool firstLine, BidiRun* firstRun, BidiRun* trailingSpaceRun, bool reachedEnd, GlyphOverflowAndFallbackFontsMap&);
    void computeBlockDirectionPositionsForLine(RootInlineBox*, BidiRun*, GlyphOverflowAndFallbackFontsMap&, VerticalPositionCache&);
    void deleteEllipsisLineBoxes();
    void checkLinesForTextOverflow();
    void addOverflowFromInlineChildren();
    int beforeSideVisibleOverflowForLine(RootInlineBox*) const;
    int afterSideVisibleOverflowForLine(RootInlineBox*) const;
    int beforeSideLayoutOverflowForLine(RootInlineBox*) const;
    int afterSideLayoutOverflowForLine(RootInlineBox*) const;
    // End of functions defined in RenderBlockLineLayout.cpp.

    void addOverflowFromFloats();

    void paintFloats(PaintInfo&, int tx, int ty, bool preservePhase = false);
    void paintContents(PaintInfo&, int tx, int ty);
    void paintColumnContents(PaintInfo&, int tx, int ty, bool paintFloats = false);
    void paintColumnRules(PaintInfo&, int tx, int ty);
    void paintChildren(PaintInfo&, int tx, int ty);
    void paintEllipsisBoxes(PaintInfo&, int tx, int ty);
    void paintSelection(PaintInfo&, int tx, int ty);
    void paintCaret(PaintInfo&, int tx, int ty, CaretType);

    FloatingObject* insertFloatingObject(RenderBox*);
    void removeFloatingObject(RenderBox*);
    void removeFloatingObjectsBelow(FloatingObject*, int y);
    
    // Called from lineWidth, to position the floats added in the last line.
    // Returns true if and only if it has positioned any floats.
    bool positionNewFloats();
    
    // Positions new floats and also adjust all floats encountered on the line if any of them
    // have to move to the next page/column.
    bool positionNewFloatOnLine(FloatingObject* newFloat, FloatingObject* lastFloatFromPreviousLine);

    void clearFloats();
    int getClearDelta(RenderBox* child, int yPos);

    virtual bool avoidsFloats() const;

    bool hasOverhangingFloats() { return parent() && !hasColumns() && lowestFloatLogicalBottom() > logicalHeight(); }
    void addIntrudingFloats(RenderBlock* prev, int xoffset, int yoffset);
    int addOverhangingFloats(RenderBlock* child, int xoffset, int yoffset, bool makeChildPaintOtherFloats);

    int lowestFloatLogicalBottom(FloatingObject::Type = FloatingObject::FloatBoth) const;
    int nextFloatLogicalBottomBelow(int) const;
    
    virtual bool hitTestColumns(const HitTestRequest&, HitTestResult&, int x, int y, int tx, int ty, HitTestAction);
    virtual bool hitTestContents(const HitTestRequest&, HitTestResult&, int x, int y, int tx, int ty, HitTestAction);
    bool hitTestFloats(const HitTestRequest&, HitTestResult&, int x, int y, int tx, int ty);

    virtual bool isPointInOverflowControl(HitTestResult&, int x, int y, int tx, int ty);

    void computeInlinePreferredLogicalWidths();
    void computeBlockPreferredLogicalWidths();

    // Obtains the nearest enclosing block (including this block) that contributes a first-line style to our inline
    // children.
    virtual RenderBlock* firstLineBlock() const;

    virtual IntRect rectWithOutlineForRepaint(RenderBoxModelObject* repaintContainer, int outlineWidth);
    virtual RenderStyle* outlineStyleForRepaint() const;
    
    virtual RenderObject* hoverAncestor() const;
    virtual void updateDragState(bool dragOn);
    virtual void childBecameNonInline(RenderObject* child);

    virtual IntRect selectionRectForRepaint(RenderBoxModelObject* repaintContainer, bool /*clipToVisibleContent*/)
    {
        return selectionGapRectsForRepaint(repaintContainer);
    }
    virtual bool shouldPaintSelectionGaps() const;
    bool isSelectionRoot() const;
    GapRects selectionGaps(RenderBlock* rootBlock, const IntPoint& rootBlockPhysicalPosition, const IntSize& offsetFromRootBlock,
                           int& lastLogicalTop, int& lastLogicalLeft, int& lastLogicalRight, const PaintInfo* = 0);
    GapRects inlineSelectionGaps(RenderBlock* rootBlock, const IntPoint& rootBlockPhysicalPosition, const IntSize& offsetFromRootBlock,
                           int& lastLogicalTop, int& lastLogicalLeft, int& lastLogicalRight, const PaintInfo*);
    GapRects blockSelectionGaps(RenderBlock* rootBlock, const IntPoint& rootBlockPhysicalPosition, const IntSize& offsetFromRootBlock,
                           int& lastLogicalTop, int& lastLogicalLeft, int& lastLogicalRight, const PaintInfo*);
    IntRect blockSelectionGap(RenderBlock* rootBlock, const IntPoint& rootBlockPhysicalPosition, const IntSize& offsetFromRootBlock,
                              int lastLogicalTop, int lastLogicalLeft, int lastLogicalRight, int logicalBottom, const PaintInfo*);
    int logicalLeftSelectionOffset(RenderBlock* rootBlock, int position);
    int logicalRightSelectionOffset(RenderBlock* rootBlock, int position);
    
    virtual void absoluteRects(Vector<IntRect>&, int tx, int ty);
    virtual void absoluteQuads(Vector<FloatQuad>&);

    int desiredColumnWidth() const;
    unsigned desiredColumnCount() const;
    void setDesiredColumnCountAndWidth(int count, int width);

    void paintContinuationOutlines(PaintInfo&, int tx, int ty);

    virtual IntRect localCaretRect(InlineBox*, int caretOffset, int* extraWidthToEndOfLine = 0);

    virtual void addFocusRingRects(Vector<IntRect>&, int tx, int ty);

    void adjustPointToColumnContents(IntPoint&) const;
    void adjustForBorderFit(int x, int& left, int& right) const; // Helper function for borderFitAdjust

    void markLinesDirtyInBlockRange(int logicalTop, int logicalBottom, RootInlineBox* highest = 0);

    void newLine(EClear);

    Position positionForBox(InlineBox*, bool start = true) const;
    Position positionForRenderer(RenderObject*, bool start = true) const;
    VisiblePosition positionForPointWithInlineChildren(const IntPoint&);

    // Adjust tx and ty from painting offsets to the local coords of this renderer
    void offsetForContents(int& tx, int& ty) const;

    void calcColumnWidth();
    bool layoutColumns(bool hasSpecifiedPageHeight, int pageHeight, LayoutStateMaintainer&);
    void makeChildrenAnonymousColumnBlocks(RenderObject* beforeChild, RenderBlock* newBlockBox, RenderObject* newChild);

    bool expandsToEncloseOverhangingFloats() const;

    void updateScrollInfoAfterLayout();

    RenderObject* splitAnonymousBlocksAroundChild(RenderObject* beforeChild);
    void splitBlocks(RenderBlock* fromBlock, RenderBlock* toBlock, RenderBlock* middleBlock,
                     RenderObject* beforeChild, RenderBoxModelObject* oldCont);
    void splitFlow(RenderObject* beforeChild, RenderBlock* newBlockBox,
                   RenderObject* newChild, RenderBoxModelObject* oldCont);
    RenderBlock* clone() const;
    RenderBlock* continuationBefore(RenderObject* beforeChild);
    RenderBlock* containingColumnsBlock(bool allowAnonymousColumnBlock = true);
    RenderBlock* columnsBlockForSpanningElement(RenderObject* newChild);

    class MarginInfo {
        // Collapsing flags for whether we can collapse our margins with our children's margins.
        bool m_canCollapseWithChildren : 1;
        bool m_canCollapseMarginBeforeWithChildren : 1;
        bool m_canCollapseMarginAfterWithChildren : 1;

        // Whether or not we are a quirky container, i.e., do we collapse away top and bottom
        // margins in our container.  Table cells and the body are the common examples. We
        // also have a custom style property for Safari RSS to deal with TypePad blog articles.
        bool m_quirkContainer : 1;

        // This flag tracks whether we are still looking at child margins that can all collapse together at the beginning of a block.  
        // They may or may not collapse with the top margin of the block (|m_canCollapseTopWithChildren| tells us that), but they will
        // always be collapsing with one another.  This variable can remain set to true through multiple iterations 
        // as long as we keep encountering self-collapsing blocks.
        bool m_atBeforeSideOfBlock : 1;

        // This flag is set when we know we're examining bottom margins and we know we're at the bottom of the block.
        bool m_atAfterSideOfBlock : 1;

        // These variables are used to detect quirky margins that we need to collapse away (in table cells
        // and in the body element).
        bool m_marginBeforeQuirk : 1;
        bool m_marginAfterQuirk : 1;
        bool m_determinedMarginBeforeQuirk : 1;

        // These flags track the previous maximal positive and negative margins.
        int m_positiveMargin;
        int m_negativeMargin;

    public:
        MarginInfo(RenderBlock* b, int beforeBorderPadding, int afterBorderPadding);

        void setAtBeforeSideOfBlock(bool b) { m_atBeforeSideOfBlock = b; }
        void setAtAfterSideOfBlock(bool b) { m_atAfterSideOfBlock = b; }
        void clearMargin() { m_positiveMargin = m_negativeMargin = 0; }
        void setMarginBeforeQuirk(bool b) { m_marginBeforeQuirk = b; }
        void setMarginAfterQuirk(bool b) { m_marginAfterQuirk = b; }
        void setDeterminedMarginBeforeQuirk(bool b) { m_determinedMarginBeforeQuirk = b; }
        void setPositiveMargin(int p) { m_positiveMargin = p; }
        void setNegativeMargin(int n) { m_negativeMargin = n; }
        void setPositiveMarginIfLarger(int p) { if (p > m_positiveMargin) m_positiveMargin = p; }
        void setNegativeMarginIfLarger(int n) { if (n > m_negativeMargin) m_negativeMargin = n; }

        void setMargin(int p, int n) { m_positiveMargin = p; m_negativeMargin = n; }

        bool atBeforeSideOfBlock() const { return m_atBeforeSideOfBlock; }
        bool canCollapseWithMarginBefore() const { return m_atBeforeSideOfBlock && m_canCollapseMarginBeforeWithChildren; }
        bool canCollapseWithMarginAfter() const { return m_atAfterSideOfBlock && m_canCollapseMarginAfterWithChildren; }
        bool canCollapseMarginBeforeWithChildren() const { return m_canCollapseMarginBeforeWithChildren; }
        bool canCollapseMarginAfterWithChildren() const { return m_canCollapseMarginAfterWithChildren; }
        bool quirkContainer() const { return m_quirkContainer; }
        bool determinedMarginBeforeQuirk() const { return m_determinedMarginBeforeQuirk; }
        bool marginBeforeQuirk() const { return m_marginBeforeQuirk; }
        bool marginAfterQuirk() const { return m_marginAfterQuirk; }
        int positiveMargin() const { return m_positiveMargin; }
        int negativeMargin() const { return m_negativeMargin; }
        int margin() const { return m_positiveMargin - m_negativeMargin; }
    };

    void layoutBlockChild(RenderBox* child, MarginInfo&, int& previousFloatLogicalBottom, int& maxFloatLogicalBottom);
    void adjustPositionedBlock(RenderBox* child, const MarginInfo&);
    void adjustFloatingBlock(const MarginInfo&);
    bool handleSpecialChild(RenderBox* child, const MarginInfo&);
    bool handleFloatingChild(RenderBox* child, const MarginInfo&);
    bool handlePositionedChild(RenderBox* child, const MarginInfo&);
    bool handleRunInChild(RenderBox* child);
    int collapseMargins(RenderBox* child, MarginInfo&);
    int clearFloatsIfNeeded(RenderBox* child, MarginInfo&, int oldTopPosMargin, int oldTopNegMargin, int yPos);
    int estimateLogicalTopPosition(RenderBox* child, const MarginInfo&);
    void determineLogicalLeftPositionForChild(RenderBox* child);
    void handleAfterSideOfBlock(int top, int bottom, MarginInfo&);
    void setCollapsedBottomMargin(const MarginInfo&);
    // End helper functions and structs used by layoutBlockChildren.

    // Pagination routines.
    int nextPageTop(int yPos) const; // Returns the top of the next page following yPos.
    int applyBeforeBreak(RenderBox* child, int yPos); // If the child has a before break, then return a new yPos that shifts to the top of the next page/column.
    int applyAfterBreak(RenderBox* child, int yPos, MarginInfo& marginInfo); // If the child has an after break, then return a new yPos that shifts to the top of the next page/column.
    int adjustForUnsplittableChild(RenderBox* child, int yPos, bool includeMargins = false); // If the child is unsplittable and can't fit on the current page, return the top of the next page/column.
    void adjustLinePositionForPagination(RootInlineBox*, int& deltaY); // Computes a deltaY value that put a line at the top of the next page if it doesn't fit on the current page.

    typedef PositionedObjectsListHashSet::const_iterator Iterator;
    DeprecatedPtrList<FloatingObject>* m_floatingObjects;
    
    PositionedObjectsListHashSet* m_positionedObjects;

    // An inline can be split with blocks occurring in between the inline content.
    // When this occurs we need a pointer to our next object.  We can basically be
    // split into a sequence of inlines and blocks.  The continuation will either be
    // an anonymous block (that houses other blocks) or it will be an inline flow.
    RenderBoxModelObject* m_continuation;
    
    // Allocated only when some of these fields have non-default values
    struct RenderBlockRareData : Noncopyable {
        RenderBlockRareData(const RenderBlock* block) 
            : m_margins(positiveMarginBeforeDefault(block), negativeMarginBeforeDefault(block), positiveMarginAfterDefault(block), negativeMarginAfterDefault(block))
            , m_paginationStrut(0)
            , m_pageY(0)
        { 
        }

        static int positiveMarginBeforeDefault(const RenderBlock* block)
        { 
            return std::max(block->marginBefore(), 0);
        }
        
        static int negativeMarginBeforeDefault(const RenderBlock* block)
        { 
            return std::max(-block->marginBefore(), 0);
        }
        static int positiveMarginAfterDefault(const RenderBlock* block)
        {
            return std::max(block->marginAfter(), 0);
        }
        static int negativeMarginAfterDefault(const RenderBlock* block)
        {
            return std::max(-block->marginAfter(), 0);
        }
        
        MarginValues m_margins;
        int m_paginationStrut;
        int m_pageY;
     };

    OwnPtr<RenderBlockRareData> m_rareData;

    RenderObjectChildList m_children;
    RenderLineBoxList m_lineBoxes;   // All of the root line boxes created for this block flow.  For example, <div>Hello<br>world.</div> will have two total lines for the <div>.

    mutable int m_lineHeight;

    // RenderRubyBase objects need to be able to split and merge, moving their children around
    // (calling moveChildTo, moveAllChildrenTo, and makeChildrenNonInline).
    friend class RenderRubyBase;
};

inline RenderBlock* toRenderBlock(RenderObject* object)
{ 
    ASSERT(!object || object->isRenderBlock());
    return static_cast<RenderBlock*>(object);
}

inline const RenderBlock* toRenderBlock(const RenderObject* object)
{ 
    ASSERT(!object || object->isRenderBlock());
    return static_cast<const RenderBlock*>(object);
}

// This will catch anyone doing an unnecessary cast.
void toRenderBlock(const RenderBlock*);

} // namespace WebCore

#endif // RenderBlock_h
