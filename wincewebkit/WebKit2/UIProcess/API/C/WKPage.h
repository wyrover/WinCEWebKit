/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WKPage_h
#define WKPage_h

#include <WebKit2/WKBase.h>
#include <WebKit2/WKEvent.h>
#include <WebKit2/WKGeometry.h>
#include <WebKit2/WKNativeEvent.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum {
    kWKFrameNavigationTypeLinkClicked = 0,
    kWKFrameNavigationTypeFormSubmitted = 1,
    kWKFrameNavigationTypeBackForward = 2,
    kWKFrameNavigationTypeReload = 3,
    kWKFrameNavigationTypeFormResubmitted = 4,
    kWKFrameNavigationTypeOther = 5
};
typedef uint32_t WKFrameNavigationType;

// FrameLoad Client
typedef void (*WKPageDidStartProvisionalLoadForFrameCallback)(WKPageRef page, WKFrameRef frame, WKTypeRef userData, const void *clientInfo);
typedef void (*WKPageDidReceiveServerRedirectForProvisionalLoadForFrameCallback)(WKPageRef page, WKFrameRef frame, WKTypeRef userData, const void *clientInfo);
typedef void (*WKPageDidFailProvisionalLoadWithErrorForFrameCallback)(WKPageRef page, WKFrameRef frame, WKErrorRef error, WKTypeRef userData, const void *clientInfo);
typedef void (*WKPageDidCommitLoadForFrameCallback)(WKPageRef page, WKFrameRef frame, WKTypeRef userData, const void *clientInfo);
typedef void (*WKPageDidFinishDocumentLoadForFrameCallback)(WKPageRef page, WKFrameRef frame, WKTypeRef userData, const void *clientInfo);
typedef void (*WKPageDidFinishLoadForFrameCallback)(WKPageRef page, WKFrameRef frame, WKTypeRef userData, const void *clientInfo);
typedef void (*WKPageDidFailLoadWithErrorForFrameCallback)(WKPageRef page, WKFrameRef frame, WKErrorRef error, WKTypeRef userData, const void *clientInfo);
typedef void (*WKPageDidReceiveTitleForFrameCallback)(WKPageRef page, WKStringRef title, WKFrameRef frame, WKTypeRef userData, const void *clientInfo);
typedef void (*WKPageDidFirstLayoutForFrameCallback)(WKPageRef page, WKFrameRef frame, WKTypeRef userData, const void *clientInfo);
typedef void (*WKPageDidFirstVisuallyNonEmptyLayoutForFrameCallback)(WKPageRef page, WKFrameRef frame, WKTypeRef userData, const void *clientInfo);
typedef void (*WKPageDidRemoveFrameFromHierarchyCallback)(WKPageRef page, WKFrameRef frame, WKTypeRef userData, const void *clientInfo);
typedef void (*WKPageDidDisplayInsecureContentForFrameCallback)(WKPageRef page, WKFrameRef frame, WKTypeRef userData, const void *clientInfo);
typedef void (*WKPageDidRunInsecureContentForFrameCallback)(WKPageRef page, WKFrameRef frame, WKTypeRef userData, const void *clientInfo);

// Progress Client
typedef void (*WKPageDidStartProgressCallback)(WKPageRef page, const void *clientInfo);
typedef void (*WKPageDidChangeProgressCallback)(WKPageRef page, const void *clientInfo);
typedef void (*WKPageDidFinishProgressCallback)(WKPageRef page, const void *clientInfo);

// WebPageNamespace Client
// FIXME: These three functions should not be part of this client.
typedef void (*WKPageProcessDidBecomeUnresponsiveCallback)(WKPageRef page, const void *clientInfo);
typedef void (*WKPageProcessDidBecomeResponsiveCallback)(WKPageRef page, const void *clientInfo);
typedef void (*WKPageProcessDidCrashCallback)(WKPageRef page, const void *clientInfo);

typedef void (*WKPageDidChangeBackForwardListCallback)(WKPageRef page, const void *clientInfo);

struct WKPageLoaderClient {
    int                                                                 version;
    const void *                                                        clientInfo;
    WKPageDidStartProvisionalLoadForFrameCallback                       didStartProvisionalLoadForFrame;
    WKPageDidReceiveServerRedirectForProvisionalLoadForFrameCallback    didReceiveServerRedirectForProvisionalLoadForFrame;
    WKPageDidFailProvisionalLoadWithErrorForFrameCallback               didFailProvisionalLoadWithErrorForFrame;
    WKPageDidCommitLoadForFrameCallback                                 didCommitLoadForFrame;
    WKPageDidFinishDocumentLoadForFrameCallback                         didFinishDocumentLoadForFrame;
    WKPageDidFinishLoadForFrameCallback                                 didFinishLoadForFrame;
    WKPageDidFailLoadWithErrorForFrameCallback                          didFailLoadWithErrorForFrame;
    WKPageDidReceiveTitleForFrameCallback                               didReceiveTitleForFrame;
    WKPageDidFirstLayoutForFrameCallback                                didFirstLayoutForFrame;
    WKPageDidFirstVisuallyNonEmptyLayoutForFrameCallback                didFirstVisuallyNonEmptyLayoutForFrame;
    WKPageDidRemoveFrameFromHierarchyCallback                           didRemoveFrameFromHierarchy;
    WKPageDidDisplayInsecureContentForFrameCallback                     didDisplayInsecureContentForFrame;
    WKPageDidRunInsecureContentForFrameCallback                         didRunInsecureContentForFrame;

    // FIXME: Move to progress client.
    WKPageDidStartProgressCallback                                      didStartProgress;
    WKPageDidChangeProgressCallback                                     didChangeProgress;
    WKPageDidFinishProgressCallback                                     didFinishProgress;

    // FIXME: These three functions should not be part of this client.
    WKPageProcessDidBecomeUnresponsiveCallback                          processDidBecomeUnresponsive;
    WKPageProcessDidBecomeResponsiveCallback                            processDidBecomeResponsive;
    WKPageProcessDidCrashCallback                                       processDidCrash;

    WKPageDidChangeBackForwardListCallback                              didChangeBackForwardList;
};
typedef struct WKPageLoaderClient WKPageLoaderClient;

// Policy Client.
typedef void (*WKPageDecidePolicyForNavigationActionCallback)(WKPageRef page, WKFrameNavigationType navigationType, WKEventModifiers modifiers, WKEventMouseButton mouseButton, WKURLRef url, WKFrameRef frame, WKFramePolicyListenerRef listener, const void *clientInfo);
typedef void (*WKPageDecidePolicyForNewWindowActionCallback)(WKPageRef page, WKFrameNavigationType navigationType, WKEventModifiers modifiers, WKEventMouseButton mouseButton, WKURLRef url, WKFrameRef frame, WKFramePolicyListenerRef listener, const void *clientInfo);
typedef void (*WKPageDecidePolicyForMIMETypeCallback)(WKPageRef page, WKStringRef MIMEType, WKURLRef url, WKFrameRef frame, WKFramePolicyListenerRef listener, const void *clientInfo);

struct WKPagePolicyClient {
    int                                                                 version;
    const void *                                                        clientInfo;
    WKPageDecidePolicyForNavigationActionCallback                       decidePolicyForNavigationAction;
    WKPageDecidePolicyForNewWindowActionCallback                        decidePolicyForNewWindowAction;
    WKPageDecidePolicyForMIMETypeCallback                               decidePolicyForMIMEType;
};
typedef struct WKPagePolicyClient WKPagePolicyClient;

// Form Client.
typedef void (*WKPageWillSubmitFormCallback)(WKPageRef page, WKFrameRef frame, WKFrameRef sourceFrame, WKDictionaryRef values, WKTypeRef userData, WKFormSubmissionListenerRef listener, const void* clientInfo);

struct WKPageFormClient {
    int                                                                 version;
    const void *                                                        clientInfo;
    WKPageWillSubmitFormCallback                                        willSubmitForm;
};
typedef struct WKPageFormClient WKPageFormClient;

// UI Client
typedef WKPageRef (*WKPageCreateNewPageCallback)(WKPageRef page, WKDictionaryRef features, WKEventModifiers modifiers, WKEventMouseButton mouseButton, const void *clientInfo);
typedef void (*WKPageShowPageCallback)(WKPageRef page, const void *clientInfo);
typedef void (*WKPageCloseCallback)(WKPageRef page, const void *clientInfo);
typedef void (*WKPageRunJavaScriptAlertCallback)(WKPageRef page, WKStringRef alertText, WKFrameRef frame, const void *clientInfo);
typedef bool (*WKPageRunJavaScriptConfirmCallback)(WKPageRef page, WKStringRef message, WKFrameRef frame, const void *clientInfo);
typedef WKStringRef (*WKPageRunJavaScriptPromptCallback)(WKPageRef page, WKStringRef message, WKStringRef defaultValue, WKFrameRef frame, const void *clientInfo);
typedef void (*WKPageSetStatusTextCallback)(WKPageRef page, WKStringRef text, const void *clientInfo);
typedef void (*WKPageMouseDidMoveOverElementCallback)(WKPageRef page, WKEventModifiers modifiers, WKTypeRef userData, const void *clientInfo);
typedef void (*WKPageDidNotHandleKeyEventCallback)(WKPageRef page, WKNativeEventPtr event, const void *clientInfo);
typedef bool (*WKPageGetToolbarsAreVisibleCallback)(WKPageRef page, const void *clientInfo);
typedef void (*WKPageSetToolbarsAreVisibleCallback)(WKPageRef page, bool toolbarsVisible, const void *clientInfo);
typedef bool (*WKPageGetMenuBarIsVisibleCallback)(WKPageRef page, const void *clientInfo);
typedef void (*WKPageSetMenuBarIsVisibleCallback)(WKPageRef page, bool menuBarVisible, const void *clientInfo);
typedef bool (*WKPageGetStatusBarIsVisibleCallback)(WKPageRef page, const void *clientInfo);
typedef void (*WKPageSetStatusBarIsVisibleCallback)(WKPageRef page, bool statusBarVisible, const void *clientInfo);
typedef bool (*WKPageGetIsResizableCallback)(WKPageRef page, const void *clientInfo);
typedef void (*WKPageSetIsResizableCallback)(WKPageRef page, bool resizable, const void *clientInfo);
typedef WKRect (*WKPageGetWindowFrameCallback)(WKPageRef page, const void *clientInfo);
typedef void (*WKPageSetWindowFrameCallback)(WKPageRef page, WKRect frame, const void *clientInfo);
typedef bool (*WKPageRunBeforeUnloadConfirmPanelCallback)(WKPageRef page, WKStringRef message, WKFrameRef frame, const void *clientInfo);
typedef void (*WKPageDidDrawCallback)(WKPageRef page, const void *clientInfo);
typedef void (*WKPageDidScrollCallback)(WKPageRef page, const void *clientInfo);


struct WKPageUIClient {
    int                                                                 version;
    const void *                                                        clientInfo;
    WKPageCreateNewPageCallback                                         createNewPage;
    WKPageShowPageCallback                                              showPage;
    WKPageCloseCallback                                                 close;
    WKPageRunJavaScriptAlertCallback                                    runJavaScriptAlert;
    WKPageRunJavaScriptConfirmCallback                                  runJavaScriptConfirm;
    WKPageRunJavaScriptPromptCallback                                   runJavaScriptPrompt;
    WKPageSetStatusTextCallback                                         setStatusText;
    WKPageMouseDidMoveOverElementCallback                               mouseDidMoveOverElement;
    WKPageDidNotHandleKeyEventCallback                                  didNotHandleKeyEvent;
    WKPageGetToolbarsAreVisibleCallback                                 toolbarsAreVisible;
    WKPageSetToolbarsAreVisibleCallback                                 setToolbarsAreVisible;
    WKPageGetMenuBarIsVisibleCallback                                   menuBarIsVisible;
    WKPageSetMenuBarIsVisibleCallback                                   setMenuBarIsVisible;
    WKPageGetStatusBarIsVisibleCallback                                 statusBarIsVisible;
    WKPageSetStatusBarIsVisibleCallback                                 setStatusBarIsVisible;
    WKPageGetIsResizableCallback                                        isResizable;
    WKPageSetIsResizableCallback                                        setIsResizable;
    WKPageGetWindowFrameCallback                                        getWindowFrame;
    WKPageSetWindowFrameCallback                                        setWindowFrame;
    WKPageRunBeforeUnloadConfirmPanelCallback                           runBeforeUnloadConfirmPanel;
    WKPageDidDrawCallback                                               didDraw;
    WKPageDidScrollCallback                                             pageDidScroll;
};
typedef struct WKPageUIClient WKPageUIClient;

// Find client.
typedef void (*WKPageDidFindStringCallback)(WKPageRef page, WKStringRef string, unsigned matchCount, const void* clientInfo);
typedef void (*WKPageDidFailToFindStringCallback)(WKPageRef page, WKStringRef string, const void* clientInfo);
typedef void (*WKPageDidCountStringMatchesCallback)(WKPageRef page, WKStringRef string, unsigned matchCount, const void* clientInfo);

struct WKPageFindClient {
    int                                                                 version;
    const void *                                                        clientInfo;
    WKPageDidFindStringCallback                                         didFindString;
    WKPageDidFailToFindStringCallback                                   didFailToFindString;
    WKPageDidCountStringMatchesCallback                                 didCountStringMatches;
};
typedef struct WKPageFindClient WKPageFindClient;

enum {
    kWKMoreThanMaximumMatchCount = -1
};

// ContextMenu client
typedef void (*WKPageGetContextMenuFromProposedContextMenuCallback)(WKPageRef page, WKArrayRef proposedMenu, WKArrayRef* newMenu, WKTypeRef userData, const void* clientInfo);
typedef void (*WKPageCustomContextMenuItemSelectedCallback)(WKPageRef page, WKContextMenuItemRef contextMenuItem, const void* clientInfo);

struct WKPageContextMenuClient {
    int                                                                 version;
    const void *                                                        clientInfo;
    WKPageGetContextMenuFromProposedContextMenuCallback                 getContextMenuFromProposedMenu;
    WKPageCustomContextMenuItemSelectedCallback                         customContextMenuItemSelected;
};
typedef struct WKPageContextMenuClient WKPageContextMenuClient;

WK_EXPORT WKTypeID WKPageGetTypeID();

WK_EXPORT WKPageNamespaceRef WKPageGetPageNamespace(WKPageRef page);

WK_EXPORT void WKPageLoadURL(WKPageRef page, WKURLRef url);
WK_EXPORT void WKPageLoadURLRequest(WKPageRef page, WKURLRequestRef urlRequest);
WK_EXPORT void WKPageLoadHTMLString(WKPageRef page, WKStringRef htmlString, WKURLRef baseURL);
WK_EXPORT void WKPageLoadAlternateHTMLString(WKPageRef page, WKStringRef htmlString, WKURLRef baseURL, WKURLRef unreachableURL);
WK_EXPORT void WKPageLoadPlainTextString(WKPageRef page, WKStringRef plainTextString);

WK_EXPORT void WKPageStopLoading(WKPageRef page);
WK_EXPORT void WKPageReload(WKPageRef page);
WK_EXPORT void WKPageReloadFromOrigin(WKPageRef page);

WK_EXPORT bool WKPageTryClose(WKPageRef page);
WK_EXPORT void WKPageClose(WKPageRef page);
WK_EXPORT bool WKPageIsClosed(WKPageRef page);

WK_EXPORT void WKPageGoForward(WKPageRef page);
WK_EXPORT bool WKPageCanGoForward(WKPageRef page);
WK_EXPORT void WKPageGoBack(WKPageRef page);
WK_EXPORT bool WKPageCanGoBack(WKPageRef page);
WK_EXPORT void WKPageGoToBackForwardListItem(WKPageRef page, WKBackForwardListItemRef item);
WK_EXPORT WKBackForwardListRef WKPageGetBackForwardList(WKPageRef page);

WK_EXPORT bool WKPageCanShowMIMEType(WKPageRef page, WKStringRef mimeType);

WK_EXPORT WKStringRef WKPageCopyTitle(WKPageRef page);

WK_EXPORT WKFrameRef WKPageGetMainFrame(WKPageRef page);
WK_EXPORT WKFrameRef WKPageGetFocusedFrame(WKPageRef page); // The focused frame may be inactive.
WK_EXPORT double WKPageGetEstimatedProgress(WKPageRef page);

#if defined(ENABLE_INSPECTOR) && ENABLE_INSPECTOR
WK_EXPORT WKInspectorRef WKPageGetInspector(WKPageRef page);
#endif

WK_EXPORT WKStringRef WKPageCopyCustomUserAgent(WKPageRef page);
WK_EXPORT void WKPageSetCustomUserAgent(WKPageRef page, WKStringRef userAgent);

WK_EXPORT void WKPageTerminate(WKPageRef page);

WK_EXPORT WKStringRef WKPageGetSessionHistoryURLValueType(void);

typedef bool (*WKPageSessionStateFilterCallback)(WKPageRef page, WKStringRef valueType, WKTypeRef value, void* context);
WK_EXPORT WKDataRef WKPageCopySessionState(WKPageRef page, void* context, WKPageSessionStateFilterCallback urlAllowedCallback);
WK_EXPORT void WKPageRestoreFromSessionState(WKPageRef page, WKDataRef sessionStateData);

WK_EXPORT double WKPageGetTextZoomFactor(WKPageRef page);
WK_EXPORT void WKPageSetTextZoomFactor(WKPageRef page, double zoomFactor);
WK_EXPORT double WKPageGetPageZoomFactor(WKPageRef page);
WK_EXPORT void WKPageSetPageZoomFactor(WKPageRef page, double zoomFactor);
WK_EXPORT void WKPageSetPageAndTextZoomFactors(WKPageRef page, double pageZoomFactor, double textZoomFactor);

WK_EXPORT void WKPageScaleWebView(WKPageRef page, double scale, WKPoint origin);
WK_EXPORT double WKPageGetViewScaleFactor(WKPageRef page);

// Find.
enum {
    kWKFindDirectionForward,
    kWKFindDirectionBackward
};
typedef uint32_t WKFindDirection;

enum {
    kWKFindOptionsCaseInsensitive = 1 << 0,
    kWKFindOptionsWrapAround = 1 << 1,
    kWKFindOptionsShowOverlay = 1 << 2,
    kWKFindOptionsShowFindIndicator = 1 << 3
};
typedef uint32_t WKFindOptions;

WK_EXPORT void WKPageFindString(WKPageRef page, WKStringRef string, WKFindDirection findDirection, WKFindOptions findOptions, unsigned maxMatchCount);
WK_EXPORT void WKPageHideFindUI(WKPageRef page);
WK_EXPORT void WKPageCountStringMatches(WKPageRef page, WKStringRef string, bool caseInsensitive, unsigned maxMatchCount);

WK_EXPORT void WKPageSetPageContextMenuClient(WKPageRef page, const WKPageContextMenuClient* client);
WK_EXPORT void WKPageSetPageFindClient(WKPageRef page, const WKPageFindClient* client);
WK_EXPORT void WKPageSetPageFormClient(WKPageRef page, const WKPageFormClient* client);
WK_EXPORT void WKPageSetPageLoaderClient(WKPageRef page, const WKPageLoaderClient* client);
WK_EXPORT void WKPageSetPagePolicyClient(WKPageRef page, const WKPagePolicyClient* client);
WK_EXPORT void WKPageSetPageUIClient(WKPageRef page, const WKPageUIClient* client);

typedef void (*WKPageRunJavaScriptFunction)(WKStringRef, WKErrorRef, void*);
WK_EXPORT void WKPageRunJavaScriptInMainFrame(WKPageRef page, WKStringRef script, void* context, WKPageRunJavaScriptFunction function);
#ifdef __BLOCKS__
typedef void (^WKPageRunJavaScriptBlock)(WKStringRef, WKErrorRef);
WK_EXPORT void WKPageRunJavaScriptInMainFrame_b(WKPageRef page, WKStringRef script, WKPageRunJavaScriptBlock block);
#endif

typedef void (*WKPageGetSourceForFrameFunction)(WKStringRef, WKErrorRef, void*);
WK_EXPORT void WKPageGetSourceForFrame(WKPageRef page, WKFrameRef frame, void* context, WKPageGetSourceForFrameFunction function);
#ifdef __BLOCKS__
typedef void (^WKPageGetSourceForFrameBlock)(WKStringRef, WKErrorRef);
WK_EXPORT void WKPageGetSourceForFrame_b(WKPageRef page, WKFrameRef frame, WKPageGetSourceForFrameBlock block);
#endif

typedef void (*WKPageGetContentsAsStringFunction)(WKStringRef, WKErrorRef, void*);
WK_EXPORT void WKPageGetContentsAsString(WKPageRef page, void* context, WKPageGetContentsAsStringFunction function);
#ifdef __BLOCKS__
typedef void (^WKPageGetContentsAsStringBlock)(WKStringRef, WKErrorRef);
WK_EXPORT void WKPageGetContentsAsString_b(WKPageRef page, WKPageGetContentsAsStringBlock block);
#endif

#ifdef __cplusplus
}
#endif

#endif /* WKPage_h */
