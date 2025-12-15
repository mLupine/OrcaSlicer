#include "CEFBrowserHandler.hpp"
#include <include/wrapper/cef_helpers.h>

namespace Slic3r { namespace GUI {

CEFBrowserHandler::CEFBrowserHandler() {
    CefMessageRouterConfig config;
    config.js_query_function = "cefQuery";
    config.js_cancel_function = "cefQueryCancel";
    message_router_ = CefMessageRouterBrowserSide::Create(config);
}

CEFBrowserHandler::~CEFBrowserHandler() {
    message_router_ = nullptr;
}

void CEFBrowserHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();

    if (!browser_) {
        browser_ = browser;
        browser->GetHost()->WasResized();
        if (state_changed_callback_) {
            state_changed_callback_();
        }
    }
}

void CEFBrowserHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();

    if (browser_->IsSame(browser)) {
        message_router_->OnBeforeClose(browser);
        browser_ = nullptr;
    }
}

void CEFBrowserHandler::OnLoadStart(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     TransitionType transition_type) {
    CEF_REQUIRE_UI_THREAD();
}

void CEFBrowserHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   int httpStatusCode) {
    CEF_REQUIRE_UI_THREAD();

    if (state_changed_callback_) {
        state_changed_callback_();
    }
}

void CEFBrowserHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    ErrorCode errorCode,
                                    const CefString& errorText,
                                    const CefString& failedUrl) {
    CEF_REQUIRE_UI_THREAD();

    if (errorCode == ERR_ABORTED) {
        return;
    }
}

bool CEFBrowserHandler::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message) {
    CEF_REQUIRE_UI_THREAD();

    return message_router_->OnProcessMessageReceived(browser, frame, source_process, message);
}

bool CEFBrowserHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefRefPtr<CefRequest> request,
                                        bool user_gesture,
                                        bool is_redirect) {
    CEF_REQUIRE_UI_THREAD();
    if (message_router_) {
        message_router_->OnBeforeBrowse(browser, frame);
    }
    return false;
}

void CEFBrowserHandler::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                                 TerminationStatus status,
                                                 int error_code,
                                                 const CefString& error_string)
{
    CEF_REQUIRE_UI_THREAD();
    if (message_router_) {
        message_router_->OnRenderProcessTerminated(browser);
    }
}

}} // namespace Slic3r::GUI
