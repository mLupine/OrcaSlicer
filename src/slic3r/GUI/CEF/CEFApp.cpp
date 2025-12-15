#include "CEFApp.hpp"
#include <cef_command_line.h>

namespace Slic3r { namespace GUI {

CEFApp::CEFApp() {
}

void CEFApp::OnContextInitialized() {
}

void CEFApp::OnBeforeCommandLineProcessing(
    const CefString& process_type,
    CefRefPtr<CefCommandLine> command_line) {

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

}} // namespace Slic3r::GUI
