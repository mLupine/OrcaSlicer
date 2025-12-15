#include "MainFrame.hpp"
#include "CEF/CEFUtils.hpp"
#include "CEF/CEFMessageHandler.hpp"
#include "Plater.hpp"
#include "Notebook.hpp"
#include <nlohmann/json.hpp>
#include <wx/event.h>

#include <optional>

namespace Slic3r {
namespace GUI {

namespace {

using json = nlohmann::json;

std::optional<int> find_page_index(const Notebook* tabpanel, const wxWindow* page)
{
    if (!tabpanel || !page)
        return std::nullopt;

    const int index = tabpanel->FindPage(const_cast<wxWindow*>(page));
    if (index == wxNOT_FOUND)
        return std::nullopt;

    return index;
}

std::optional<int> find_plater_occurrence_index(const Notebook* tabpanel, const wxWindow* plater, const int occurrence)
{
    if (!tabpanel || !plater || occurrence < 0)
        return std::nullopt;

    int found_count = 0;
    const size_t page_count = tabpanel->GetPageCount();
    for (size_t i = 0; i < page_count; ++i) {
        if (tabpanel->GetPage(i) == plater) {
            if (found_count == occurrence)
                return static_cast<int>(i);
            ++found_count;
        }
    }

    return std::nullopt;
}

std::optional<int> find_device_index(const Notebook* tabpanel, const wxWindow* monitor, const wxWindow* printer_view)
{
    if (auto monitor_idx = find_page_index(tabpanel, monitor); monitor_idx.has_value())
        return monitor_idx;

    return find_page_index(tabpanel, printer_view);
}

json build_navbar_state(const Notebook* tabpanel,
                        const wxWindow* webview,
                        const wxWindow* plater,
                        const wxWindow* monitor,
                        const wxWindow* printer_view,
                        const wxWindow* multi_machine,
                        const wxWindow* project,
                        const wxWindow* calibration,
                        const wxString& title,
                        const bool has_unsaved_changes,
                        const bool can_undo,
                        const bool can_redo,
                        const bool can_save)
{
    const auto home_idx = find_page_index(tabpanel, webview);
    const auto prepare_idx = find_plater_occurrence_index(tabpanel, plater, 0);
    const auto preview_idx = find_plater_occurrence_index(tabpanel, plater, 1);
    const auto device_idx = find_device_index(tabpanel, monitor, printer_view);
    const auto multi_device_idx = find_page_index(tabpanel, multi_machine);
    const auto project_idx = find_page_index(tabpanel, project);
    const auto calibration_idx = find_page_index(tabpanel, calibration);

    const int selection = tabpanel ? tabpanel->GetSelection() : wxNOT_FOUND;
    std::string active_tab_id = "prepare";
    if (home_idx.has_value() && selection == *home_idx)
        active_tab_id = "home";
    else if (prepare_idx.has_value() && selection == *prepare_idx)
        active_tab_id = "prepare";
    else if (preview_idx.has_value() && selection == *preview_idx)
        active_tab_id = "preview";
    else if (device_idx.has_value() && selection == *device_idx)
        active_tab_id = "device";
    else if (multi_device_idx.has_value() && selection == *multi_device_idx)
        active_tab_id = "multi-device";
    else if (project_idx.has_value() && selection == *project_idx)
        active_tab_id = "project";
    else if (calibration_idx.has_value() && selection == *calibration_idx)
        active_tab_id = "calibration";

    json tab_visibility;
    tab_visibility["home"] = home_idx.has_value();
    tab_visibility["prepare"] = prepare_idx.has_value();
    tab_visibility["preview"] = preview_idx.has_value();
    tab_visibility["device"] = device_idx.has_value();
    tab_visibility["multi-device"] = multi_device_idx.has_value();
    tab_visibility["project"] = project_idx.has_value();
    tab_visibility["calibration"] = calibration_idx.has_value();
    tab_visibility["auxiliary"] = false;
    tab_visibility["debug-tool"] = false;

    json state;
    state["activeTabId"] = active_tab_id;
    state["tabVisibility"] = tab_visibility;
    state["title"] = title.ToStdString();
    state["hasUnsavedChanges"] = has_unsaved_changes;
    state["canUndo"] = can_undo;
    state["canRedo"] = can_redo;
    state["canSave"] = can_save;
    return state;
}

std::optional<int> find_tab_index_by_id(const Notebook* tabpanel,
                                       const wxWindow* webview,
                                       const wxWindow* plater,
                                       const wxWindow* monitor,
                                       const wxWindow* printer_view,
                                       const wxWindow* multi_machine,
                                       const wxWindow* project,
                                       const wxWindow* calibration,
                                       const std::string& tab_id)
{
    if (tab_id == "home")
        return find_page_index(tabpanel, webview);
    if (tab_id == "prepare")
        return find_plater_occurrence_index(tabpanel, plater, 0);
    if (tab_id == "preview")
        return find_plater_occurrence_index(tabpanel, plater, 1);
    if (tab_id == "device")
        return find_device_index(tabpanel, monitor, printer_view);
    if (tab_id == "multi-device")
        return find_page_index(tabpanel, multi_machine);
    if (tab_id == "project")
        return find_page_index(tabpanel, project);
    if (tab_id == "calibration")
        return find_page_index(tabpanel, calibration);
    return std::nullopt;
}

} // namespace

void MainFrame::OnCEFTabSelected(wxCommandEvent& evt) {
    if (!m_tabpanel)
        return;

    int tab_index = evt.GetInt();
    if (tab_index >= 0 && tab_index < m_tabpanel->GetPageCount()) {
        select_tab(tab_index);
    }
}

void MainFrame::UpdateCEFNavbarState() {
    if (!m_cef_navbar) return;

    const bool has_unsaved_changes = (m_plater != nullptr) && m_plater->is_project_dirty();
    const bool can_undo = (m_plater != nullptr) ? m_plater->can_undo() : false;
    const bool can_redo = (m_plater != nullptr) ? m_plater->can_redo() : false;
    const bool can_save_state = can_save();

    json state = build_navbar_state(m_tabpanel, m_webview, m_plater, m_monitor, m_printer_view, m_multi_machine, m_project, m_calibration,
                                   GetTitle(), has_unsaved_changes, can_undo, can_redo, can_save_state);
    m_cef_navbar->UpdateState(state.dump());
}

void MainFrame::RegisterCEFCommands() {
    if (!m_cef_navbar) return;

    auto browser_handler = m_cef_navbar->GetHandler();
    if (!browser_handler) return;

    auto message_router = browser_handler->GetMessageRouter();
    if (!message_router) return;

    auto handler = new CEFMessageHandler();

    browser_handler->SetStateChangedCallback([this]() {
        CallAfter([this]() { UpdateCEFNavbarState(); });
    });

    handler->RegisterCommand("getState", [this](const std::string& payload) {
        json response;

        const bool has_unsaved_changes = (m_plater != nullptr) && m_plater->is_project_dirty();
        const bool can_undo = (m_plater != nullptr) ? m_plater->can_undo() : false;
        const bool can_redo = (m_plater != nullptr) ? m_plater->can_redo() : false;
        const bool can_save_state = can_save();

        response["success"] = true;
        response["state"] = build_navbar_state(m_tabpanel, m_webview, m_plater, m_monitor, m_printer_view, m_multi_machine, m_project, m_calibration,
                                               GetTitle(), has_unsaved_changes, can_undo, can_redo, can_save_state);
        return response.dump();
    });

    handler->RegisterCommand("selectTab", [this](const std::string& payload) {
        wxCommandEvent evt(EVT_CEF_TAB_SELECTED);
        try {
            auto j = nlohmann::json::parse(payload);
            if (j.contains("tabId")) {
                std::string tab_id = j["tabId"];
                auto tab_index = find_tab_index_by_id(m_tabpanel, m_webview, m_plater, m_monitor, m_printer_view, m_multi_machine, m_project, m_calibration, tab_id);
                if (!tab_index.has_value()) {
                    json response;
                    response["success"] = false;
                    response["error"] = "Unknown or unavailable tabId: " + tab_id;
                    return response.dump();
                }

                evt.SetInt(*tab_index);
                wxPostEvent(this, evt);
            }

            json response;
            response["success"] = true;
            return response.dump();
        } catch (const std::exception& e) {
            json response;
            response["success"] = false;
            response["error"] = e.what();
            return response.dump();
        }
    });

    handler->RegisterCommand("save", [this](const std::string& payload) {
        save_project();
        json response;
        response["success"] = true;
        CallAfter([this]() { UpdateCEFNavbarState(); });
        return response.dump();
    });

    handler->RegisterCommand("undo", [this](const std::string& payload) {
        if (m_plater && m_plater->is_view3D_shown()) {
            m_plater->undo();
        }
        json response;
        response["success"] = true;
        CallAfter([this]() { UpdateCEFNavbarState(); });
        return response.dump();
    });

    handler->RegisterCommand("redo", [this](const std::string& payload) {
        if (m_plater && m_plater->is_view3D_shown()) {
            m_plater->redo();
        }
        json response;
        response["success"] = true;
        CallAfter([this]() { UpdateCEFNavbarState(); });
        return response.dump();
    });

    handler->RegisterCommand("windowMinimize", [this](const std::string& payload) {
        Iconize(true);
        json response;
        response["success"] = true;
        return response.dump();
    });

    handler->RegisterCommand("windowMaximize", [this](const std::string& payload) {
        Maximize(!IsMaximized());
        json response;
        response["success"] = true;
        return response.dump();
    });

    handler->RegisterCommand("windowClose", [this](const std::string& payload) {
        Close();
        json response;
        response["success"] = true;
        return response.dump();
    });

    message_router->AddHandler(handler, false);
}

}
}
