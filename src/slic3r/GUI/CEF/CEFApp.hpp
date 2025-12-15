#pragma once

#include <cef_app.h>
#include <cef_browser_process_handler.h>

namespace Slic3r { namespace GUI {

class CEFApp : public CefApp, public CefBrowserProcessHandler {
public:
    CEFApp();

    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
        return this;
    }

    void OnContextInitialized() override;

    void OnBeforeCommandLineProcessing(
        const CefString& process_type,
        CefRefPtr<CefCommandLine> command_line) override;

private:
    IMPLEMENT_REFCOUNTING(CEFApp);
};

}} // namespace Slic3r::GUI
