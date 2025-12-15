#pragma once

#include <wx/panel.h>
#include <cef_browser.h>
#include "CEFBrowserHandler.hpp"
#include <string>

namespace Slic3r { namespace GUI {

class CEFNavigationBar : public wxPanel {
public:
    CEFNavigationBar(wxWindow* parent,
                     wxWindowID id = wxID_ANY,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize);

    virtual ~CEFNavigationBar();

    void LoadURL(const std::string& url);
    void UpdateState(const std::string& json_state);
    void ExecuteJavaScript(const std::string& code);

    CefRefPtr<CefBrowser> GetBrowser() const;
    CefRefPtr<CEFBrowserHandler> GetHandler() const { return handler_; }

private:
    void CreateBrowser(const std::string& url);
    void OnSize(wxSizeEvent& event);
    void OnSetFocus(wxFocusEvent& event);
    void OnShow(wxShowEvent& event);

    std::string GetPlatformSpecificURL(const std::string& url);

    CefRefPtr<CEFBrowserHandler> handler_;
    bool browser_created_;
    std::string pending_url_;

    wxDECLARE_EVENT_TABLE();
};

}} // namespace Slic3r::GUI
