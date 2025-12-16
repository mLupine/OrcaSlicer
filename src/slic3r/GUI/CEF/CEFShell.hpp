#pragma once

#include <wx/panel.h>
#include <cef_browser.h>
#include "CEFBrowserHandler.hpp"
#include <string>
#include <map>

namespace Slic3r { namespace GUI {

struct HoleRegion {
    std::string id;
    wxRect bounds;
    wxWindow* native_panel{nullptr};
};

class CEFShell : public wxPanel {
public:
    CEFShell(wxWindow* parent,
             wxWindowID id = wxID_ANY,
             const wxPoint& pos = wxDefaultPosition,
             const wxSize& size = wxDefaultSize);

    virtual ~CEFShell();

    void LoadURL(const std::string& url);
    void UpdateState(const std::string& json_state);
    void ExecuteJavaScript(const std::string& code);

    CefRefPtr<CefBrowser> GetBrowser() const;
    CefRefPtr<CEFBrowserHandler> GetHandler() const { return handler_; }

    void RegisterHole(const std::string& id, wxWindow* panel);
    void UnregisterHole(const std::string& id);
    void UpdateHoleBounds(const std::string& id, const wxRect& bounds);
    void RepositionNativePanels();

    wxRect GetHoleBounds(const std::string& id) const;

private:
    void CreateBrowser(const std::string& url);
    void OnSize(wxSizeEvent& event);
    void OnSetFocus(wxFocusEvent& event);
    void OnShow(wxShowEvent& event);

    std::string GetPlatformSpecificURL(const std::string& url);

    CefRefPtr<CEFBrowserHandler> handler_;
    bool browser_created_;
    std::string pending_url_;

    std::map<std::string, HoleRegion> hole_regions_;

    wxDECLARE_EVENT_TABLE();
};

}} // namespace Slic3r::GUI
