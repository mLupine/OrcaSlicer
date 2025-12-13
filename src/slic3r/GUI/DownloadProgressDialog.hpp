#ifndef slic3r_DownloadProgressDialog_hpp_
#define slic3r_DownloadProgressDialog_hpp_

#include <string>
#include <unordered_map>

#include "GUI_Utils.hpp"
#include <wx/dialog.h>
#include <wx/font.h>
#include <wx/bitmap.h>
#include <wx/msgdlg.h>
#include <wx/richmsgdlg.h>
#include <wx/textctrl.h>
#include <wx/statline.h>
#include <wx/simplebook.h>
#include <wx/combobox.h>
#include "Widgets/Button.hpp"
#include "BBLStatusBar.hpp"
#include "BBLStatusBarSend.hpp"
#include "Jobs/Worker.hpp"
#include "Jobs/UpgradeNetworkJob.hpp"

class wxBoxSizer;
class wxCheckBox;
class wxStaticBitmap;
class wxComboBox;

#define MSG_DIALOG_BUTTON_SIZE wxSize(FromDIP(58), FromDIP(24))
#define MSG_DIALOG_MIDDLE_BUTTON_SIZE wxSize(FromDIP(76), FromDIP(24))
#define MSG_DIALOG_LONG_BUTTON_SIZE wxSize(FromDIP(90), FromDIP(24))


namespace Slic3r {
namespace GUI {

class DownloadProgressDialog : public DPIDialog
{
protected:
    bool Show(bool show) override;
    void on_close(wxCloseEvent& event);

public:
    DownloadProgressDialog(wxString title, bool show_version_selector = false, wxString error_message = wxEmptyString);
    wxString format_text(wxStaticText* st, wxString str, int warp);
    ~DownloadProgressDialog();

    void on_dpi_changed(const wxRect &suggested_rect) override;
    void update_release_note(std::string release_note, std::string version);
    std::string get_selected_version() const;

    wxSimplebook* m_simplebook_status{nullptr};

	std::shared_ptr<BBLStatusBarSend> m_status_bar;
    std::unique_ptr<Worker>           m_worker;
    wxPanel *                         m_panel_download;
    wxComboBox*                       m_version_selector{nullptr};
    wxStaticText*                     m_error_message{nullptr};

protected:
    virtual std::unique_ptr<UpgradeNetworkJob> make_job();
    virtual void                               on_finish();

private:
    bool m_show_version_selector{false};
};


}
}

#endif
