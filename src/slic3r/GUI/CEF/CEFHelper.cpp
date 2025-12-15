#include <cef_app.h>
#include <cef_command_line.h>
#include <wrapper/cef_message_router.h>
#include <wrapper/cef_library_loader.h>
#include <iostream>

class CEFHelperApp : public CefApp, public CefRenderProcessHandler {
public:
    CEFHelperApp() {
        CefMessageRouterConfig config;
        config.js_query_function = "cefQuery";
        config.js_cancel_function = "cefQueryCancel";
        message_router_ = CefMessageRouterRendererSide::Create(config);
    }

    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override { return this; }

    void OnBeforeCommandLineProcessing(
        const CefString& process_type,
        CefRefPtr<CefCommandLine> command_line) override
    {
#ifdef __APPLE__
        command_line->AppendSwitch("in-process-gpu");
        command_line->AppendSwitch("disable-gpu-sandbox");
        command_line->AppendSwitch("use-mock-keychain");
#endif
        command_line->AppendSwitch("disable-gpu");
        command_line->AppendSwitch("disable-gpu-compositing");
        command_line->AppendSwitch("enable-begin-frame-scheduling");

        command_line->AppendSwitch("allow-file-access-from-files");
        command_line->AppendSwitch("allow-file-access");
        command_line->AppendSwitch("disable-web-security");
    }

    void OnContextCreated(CefRefPtr<CefBrowser> browser,
                          CefRefPtr<CefFrame> frame,
                          CefRefPtr<CefV8Context> context) override
    {
        if (message_router_)
            message_router_->OnContextCreated(browser, frame, context);
    }

    void OnContextReleased(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           CefRefPtr<CefV8Context> context) override
    {
        if (message_router_)
            message_router_->OnContextReleased(browser, frame, context);
    }

    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  CefProcessId source_process,
                                  CefRefPtr<CefProcessMessage> message) override
    {
        if (message_router_ && message_router_->OnProcessMessageReceived(browser, frame, source_process, message))
            return true;

        return false;
    }

private:
    CefRefPtr<CefMessageRouterRendererSide> message_router_;

    IMPLEMENT_REFCOUNTING(CEFHelperApp);
};

int main(int argc, char* argv[]) {
    CefScopedLibraryLoader library_loader;
    if (!library_loader.LoadInHelper()) {
        std::cerr << "[CEF Helper] Failed to load CEF library" << std::endl;
        return 1;
    }

    CefMainArgs main_args(argc, argv);
    CefRefPtr<CefApp> app = new CEFHelperApp();

    return CefExecuteProcess(main_args, app.get(), nullptr);
}
