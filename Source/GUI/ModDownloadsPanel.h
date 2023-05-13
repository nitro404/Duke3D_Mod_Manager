#ifndef _MOD_DOWNLOADS_PANEL_H_
#define _MOD_DOWNLOADS_PANEL_H_

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <memory>
#include <vector>

class Mod;
class ModDownloadPanel;

class ModDownloadsPanel final : public wxPanel {
public:
	ModDownloadsPanel(wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~ModDownloadsPanel();

	void setMod(std::shared_ptr<Mod> mod);

private:
	std::shared_ptr<Mod> m_mod;

	std::vector<ModDownloadPanel *> m_downloadPanels;
	wxFlexGridSizer * m_downloadsPanelSizer;
};

#endif // _MOD_DOWNLOADS_PANEL_H_
