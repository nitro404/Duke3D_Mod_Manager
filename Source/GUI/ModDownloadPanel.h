#ifndef _MOD_DOWNLOAD_PANEL_H_
#define _MOD_DOWNLOAD_PANEL_H_

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/hyperlink.h>

#include <memory>

class ModDownload;

class ModDownloadPanel final : public wxPanel {
public:
	ModDownloadPanel(std::shared_ptr<ModDownload> modDownload, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~ModDownloadPanel();

private:
	void onModDownloadHyperlinkClicked(wxHyperlinkEvent & event);

	std::shared_ptr<ModDownload> m_download;

	ModDownloadPanel(const ModDownloadPanel &) = delete;
	const ModDownloadPanel & operator = (const ModDownloadPanel &) = delete;
};

#endif // _MOD_DOWNLOAD_PANEL_H_
