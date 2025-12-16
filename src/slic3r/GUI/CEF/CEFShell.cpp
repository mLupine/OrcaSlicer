#include "CEFShell.hpp"
#include "CEFUtils.hpp"
#include <cef_browser.h>
#include <cef_app.h>
#include <wx/sizer.h>

#ifdef __WXMSW__
#include <wx/msw/private.h>
#endif

#ifdef __WXGTK__
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#endif

#ifdef __WXOSX__
#include "CEFViewHelper.hpp"
#endif

namespace Slic3r { namespace GUI {

wxBEGIN_EVENT_TABLE(CEFShell, wxPanel)
    EVT_SIZE(CEFShell::OnSize)
    EVT_SET_FOCUS(CEFShell::OnSetFocus)
    EVT_SHOW(CEFShell::OnShow)
wxEND_EVENT_TABLE()

CEFShell::CEFShell(wxWindow* parent,
                   wxWindowID id,
                   const wxPoint& pos,
                   const wxSize& size)
    : wxPanel(parent, id, pos, size)
    , browser_created_(false) {

    handler_ = new CEFBrowserHandler();

    handler_->SetStateChangedCallback([]() {
    });
}

CEFShell::~CEFShell() {
    if (handler_ && handler_->GetBrowser()) {
        handler_->GetBrowser()->GetHost()->CloseBrowser(true);
    }
}

void CEFShell::CreateBrowser(const std::string& url) {
    if (browser_created_) {
        return;
    }

    CefWindowInfo window_info;
    CefBrowserSettings browser_settings;

    browser_settings.background_color = CefColorSetARGB(0, 0, 0, 0);

    wxSize client_size = GetClientSize();

#ifdef __WXMSW__
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = client_size.GetWidth();
    rect.bottom = client_size.GetHeight();
    window_info.SetAsChild((HWND)GetHWND(), rect);
#elif defined(__WXOSX__)
    CefRect rect(0, 0, client_size.GetWidth(), client_size.GetHeight());
    void* handle = GetHandle();
    CEFViewHelper::ConfigureParentView(handle, true);
    window_info.SetAsChild((CefWindowHandle)handle, rect);
#else
    GtkWidget* widget = (GtkWidget*)GetHandle();
    if (widget && gtk_widget_get_window(widget)) {
        CefRect rect(0, 0, client_size.GetWidth(), client_size.GetHeight());
        window_info.SetAsChild(GDK_WINDOW_XID(gtk_widget_get_window(widget)), rect);
    }
#endif

    std::string final_url = GetPlatformSpecificURL(url);

    CefBrowserHost::CreateBrowser(window_info, handler_, final_url,
                                  browser_settings, nullptr, nullptr);
    browser_created_ = true;
}

void CEFShell::LoadURL(const std::string& url) {
    pending_url_ = url;
    if (handler_ && handler_->GetBrowser()) {
        handler_->GetBrowser()->GetMainFrame()->LoadURL(GetPlatformSpecificURL(url));
    } else if (!browser_created_) {
        CallAfter([this]() {
            if (!browser_created_ && !pending_url_.empty()) {
                CreateBrowser(pending_url_);
            }
        });
    }
}

void CEFShell::OnShow(wxShowEvent& event) {
    event.Skip();
}

void CEFShell::UpdateState(const std::string& json_state) {
    if (handler_ && handler_->GetBrowser()) {
        std::string script = "if (window.updateAppState) { window.updateAppState(" +
                           json_state + "); }";
        handler_->GetBrowser()->GetMainFrame()->ExecuteJavaScript(
            script,
            handler_->GetBrowser()->GetMainFrame()->GetURL(),
            0
        );
    }
}

void CEFShell::ExecuteJavaScript(const std::string& code) {
    if (handler_ && handler_->GetBrowser()) {
        handler_->GetBrowser()->GetMainFrame()->ExecuteJavaScript(
            code,
            handler_->GetBrowser()->GetMainFrame()->GetURL(),
            0
        );
    }
}

CefRefPtr<CefBrowser> CEFShell::GetBrowser() const {
    if (handler_) {
        return handler_->GetBrowser();
    }
    return nullptr;
}

void CEFShell::OnSize(wxSizeEvent& event) {
    event.Skip();

    if (handler_ && handler_->GetBrowser()) {
        wxSize size = GetClientSize();

#ifdef __WXMSW__
        HWND hwnd = handler_->GetBrowser()->GetHost()->GetWindowHandle();
        if (hwnd) {
            SetWindowPos(hwnd, nullptr, 0, 0,
                        size.GetWidth(), size.GetHeight(),
                        SWP_NOZORDER | SWP_NOACTIVATE);
        }
#elif defined(__WXOSX__)
        handler_->GetBrowser()->GetHost()->NotifyMoveOrResizeStarted();
#else
        handler_->GetBrowser()->GetHost()->WasResized();
#endif
    }

    RepositionNativePanels();
}

void CEFShell::OnSetFocus(wxFocusEvent& event) {
    event.Skip();

    if (handler_ && handler_->GetBrowser()) {
        handler_->GetBrowser()->GetHost()->SetFocus(true);
    }
}

std::string CEFShell::GetPlatformSpecificURL(const std::string& url) {
    if (url.find("://") != std::string::npos) {
        return url;
    }

    std::string resources_path = CEFUtils::GetResourcesPath();
    return "file://" + resources_path + "/" + url;
}

void CEFShell::RegisterHole(const std::string& id, wxWindow* panel) {
    HoleRegion region;
    region.id = id;
    region.native_panel = panel;
    region.bounds = wxRect(0, 0, 0, 0);

    hole_regions_[id] = region;
}

void CEFShell::UnregisterHole(const std::string& id) {
    hole_regions_.erase(id);
}

void CEFShell::UpdateHoleBounds(const std::string& id, const wxRect& bounds) {
    auto it = hole_regions_.find(id);
    if (it != hole_regions_.end()) {
        it->second.bounds = bounds;
        RepositionNativePanels();
    }
}

wxRect CEFShell::GetHoleBounds(const std::string& id) const {
    auto it = hole_regions_.find(id);
    if (it != hole_regions_.end()) {
        return it->second.bounds;
    }
    return wxRect(0, 0, 0, 0);
}

void CEFShell::RepositionNativePanels() {
    for (auto& [id, region] : hole_regions_) {
        if (region.native_panel && region.bounds.GetWidth() > 0 && region.bounds.GetHeight() > 0) {
            region.native_panel->SetPosition(wxPoint(region.bounds.GetX(), region.bounds.GetY()));
            region.native_panel->SetSize(wxSize(region.bounds.GetWidth(), region.bounds.GetHeight()));
        }
    }
}

}} // namespace Slic3r::GUI
