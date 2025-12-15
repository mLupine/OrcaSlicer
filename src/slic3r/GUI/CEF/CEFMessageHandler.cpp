#include "CEFMessageHandler.hpp"
#include <nlohmann/json.hpp>
#include <include/wrapper/cef_helpers.h>

using json = nlohmann::json;

namespace Slic3r { namespace GUI {

CEFMessageHandler::CEFMessageHandler() {
}

CEFMessageHandler::~CEFMessageHandler() {
}

bool CEFMessageHandler::OnQuery(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 int64_t query_id,
                                 const CefString& request,
                                 bool persistent,
                                 CefRefPtr<Callback> callback) {
    CEF_REQUIRE_UI_THREAD();

    std::string request_str = request.ToString();
    std::string response = HandleCommand(request_str);

    if (!response.empty()) {
        callback->Success(response);
        return true;
    }

    callback->Failure(0, "Unknown command");
    return true;
}

void CEFMessageHandler::OnQueryCanceled(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        int64_t query_id) {
    CEF_REQUIRE_UI_THREAD();
}

void CEFMessageHandler::RegisterCommand(const std::string& command, CommandHandler handler) {
    command_handlers_[command] = handler;
}

std::string CEFMessageHandler::HandleCommand(const std::string& request) {
    try {
        auto request_json = json::parse(request);

        if (!request_json.contains("type")) {
            json error_response;
            error_response["success"] = false;
            error_response["error"] = "Missing command type";
            return error_response.dump();
        }

        std::string command_type = request_json["type"];
        std::string payload;

        if (request_json.contains("payload")) {
            payload = request_json["payload"].dump();
        }

        auto handler_it = command_handlers_.find(command_type);
        if (handler_it != command_handlers_.end()) {
            return handler_it->second(payload);
        }

        json error_response;
        error_response["success"] = false;
        error_response["error"] = "Unknown command: " + command_type;
        return error_response.dump();

    } catch (const json::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["error"] = std::string("JSON parse error: ") + e.what();
        return error_response.dump();
    }
}

}} // namespace Slic3r::GUI
