#ifndef _METADATA_PANEL_H_
#define _METADATA_PANEL_H_

#include <ByteBuffer.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <string>
#include <vector>

class MetadataPanel final : public wxScrolledWindow {
public:
	MetadataPanel(wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER, const std::string & title = {});
	virtual ~MetadataPanel();

	void setMetadata(const std::vector<std::pair<std::string, std::string>> & metadata);
	void clearMetadata();

private:
	std::vector<std::pair<wxStaticText *, wxStaticText *>> m_metadataText;
	wxFlexGridSizer * m_metadataPanelSizer;

	MetadataPanel(const MetadataPanel &) = delete;
	const MetadataPanel & operator = (const MetadataPanel &) = delete;
};

#endif // _METADATA_PANEL_H_
