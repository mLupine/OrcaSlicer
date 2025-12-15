#pragma once

#include <include/wrapper/cef_message_router.h>
#include <functional>
#include <string>
#include <map>

namespace Slic3r { namespace GUI {

class CEFMessageHandler : public CefMessageRouterBrowserSide::Handler {
public:
    using CommandHandler = std::function<std::string(const std::string& payload)>;

    CEFMessageHandler();
    virtual ~CEFMessageHandler();

    bool OnQuery(CefRefPtr<CefBrowser> browser,
                 CefRefPtr<CefFrame> frame,
                 int64_t query_id,
                 const CefString& request,
                 bool persistent,
                 CefRefPtr<Callback> callback) override;

    void OnQueryCanceled(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         int64_t query_id) override;

    void RegisterCommand(const std::string& command, CommandHandler handler);

private:
    std::string HandleCommand(const std::string& request);

    std::map<std::string, CommandHandler> command_handlers_;
};

}} // namespace Slic3r::GUI
