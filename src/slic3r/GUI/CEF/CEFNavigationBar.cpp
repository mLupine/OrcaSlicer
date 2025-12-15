#include "CEFNavigationBar.hpp"
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

wxBEGIN_EVENT_TABLE(CEFNavigationBar, wxPanel)
    EVT_SIZE(CEFNavigationBar::OnSize)
    EVT_SET_FOCUS(CEFNavigationBar::OnSetFocus)
    EVT_SHOW(CEFNavigationBar::OnShow)
wxEND_EVENT_TABLE()

CEFNavigationBar::CEFNavigationBar(wxWindow* parent,
                                   wxWindowID id,
                                   const wxPoint& pos,
                                   const wxSize& size)
    : wxPanel(parent, id, pos, size)
    , browser_created_(false) {

    SetMinSize(wxSize(-1, 46));

    handler_ = new CEFBrowserHandler();

    handler_->SetStateChangedCallback([]() {
    });
}

CEFNavigationBar::~CEFNavigationBar() {
    if (handler_ && handler_->GetBrowser()) {
        handler_->GetBrowser()->GetHost()->CloseBrowser(true);
    }
}

void CEFNavigationBar::CreateBrowser(const std::string& url) {
    if (browser_created_) {
        return;
    }

    CefWindowInfo window_info;
    CefBrowserSettings browser_settings;

    browser_settings.background_color = CefColorSetARGB(255, 255, 255, 255);

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
    CEFViewHelper::ConfigureParentView(handle);
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

void CEFNavigationBar::LoadURL(const std::string& url) {
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

void CEFNavigationBar::OnShow(wxShowEvent& event) {
    event.Skip();
}

void CEFNavigationBar::UpdateState(const std::string& json_state) {
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

void CEFNavigationBar::ExecuteJavaScript(const std::string& code) {
    if (handler_ && handler_->GetBrowser()) {
        handler_->GetBrowser()->GetMainFrame()->ExecuteJavaScript(
            code,
            handler_->GetBrowser()->GetMainFrame()->GetURL(),
            0
        );
    }
}

CefRefPtr<CefBrowser> CEFNavigationBar::GetBrowser() const {
    if (handler_) {
        return handler_->GetBrowser();
    }
    return nullptr;
}

void CEFNavigationBar::OnSize(wxSizeEvent& event) {
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
}

void CEFNavigationBar::OnSetFocus(wxFocusEvent& event) {
    event.Skip();

    if (handler_ && handler_->GetBrowser()) {
        handler_->GetBrowser()->GetHost()->SetFocus(true);
    }
}

std::string CEFNavigationBar::GetPlatformSpecificURL(const std::string& url) {
    if (url.find("://") != std::string::npos) {
        return url;
    }

    std::string resources_path = CEFUtils::GetResourcesPath();
    return "file://" + resources_path + "/" + url;
}

}} // namespace Slic3r::GUI
