#pragma once

#include <cef_client.h>
#include <cef_life_span_handler.h>
#include <cef_load_handler.h>
#include <cef_display_handler.h>
#include <cef_request_handler.h>
#include <wrapper/cef_message_router.h>
#include <functional>

namespace Slic3r { namespace GUI {

class CEFBrowserHandler : public CefClient,
                          public CefLifeSpanHandler,
                          public CefLoadHandler,
                          public CefDisplayHandler,
                          public CefRequestHandler {
public:
    using StateChangedCallback = std::function<void()>;

    CEFBrowserHandler();
    virtual ~CEFBrowserHandler();

    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
    CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
    CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
    CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }

    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    void OnLoadStart(CefRefPtr<CefBrowser> browser,
                     CefRefPtr<CefFrame> frame,
                     TransitionType transition_type) override;

    void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                   CefRefPtr<CefFrame> frame,
                   int httpStatusCode) override;

    void OnLoadError(CefRefPtr<CefBrowser> browser,
                     CefRefPtr<CefFrame> frame,
                     ErrorCode errorCode,
                     const CefString& errorText,
                     const CefString& failedUrl) override;

    bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                        CefRefPtr<CefFrame> frame,
                        CefRefPtr<CefRequest> request,
                        bool user_gesture,
                        bool is_redirect) override;

    void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                  TerminationStatus status,
                                  int error_code,
                                  const CefString& error_string) override;

    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  CefProcessId source_process,
                                  CefRefPtr<CefProcessMessage> message) override;

    CefRefPtr<CefBrowser> GetBrowser() const { return browser_; }
    CefRefPtr<CefMessageRouterBrowserSide> GetMessageRouter() const { return message_router_; }

    void SetStateChangedCallback(StateChangedCallback callback) {
        state_changed_callback_ = callback;
    }

private:
    CefRefPtr<CefBrowser> browser_;
    CefRefPtr<CefMessageRouterBrowserSide> message_router_;
    StateChangedCallback state_changed_callback_;

    IMPLEMENT_REFCOUNTING(CEFBrowserHandler);
};

}} // namespace Slic3r::GUI
