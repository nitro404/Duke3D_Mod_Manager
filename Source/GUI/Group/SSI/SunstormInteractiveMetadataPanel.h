#ifndef _SUNSTORM_INTERACTIVE_METADATA_PANEL_H_
#define _SUNSTORM_INTERACTIVE_METADATA_PANEL_H_

#include "Game/File/Group/SSI/GroupSSI.h"

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <memory>
#include <string>

class SunstormInteractiveMetadataPanel final : public wxPanel {
public:
	SunstormInteractiveMetadataPanel(std::shared_ptr<GroupSSI> group, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER, const std::string & title = {});
	virtual ~SunstormInteractiveMetadataPanel();

private:
	void onTitleTextChanged(wxCommandEvent & event);
	void onVersionSelected(wxCommandEvent & event);
	void onDescriptionTextChanged(wxCommandEvent & event);
	void onRunFileTextChanged(wxCommandEvent & event);

	std::shared_ptr<GroupSSI> m_group;

	wxTextCtrl * m_titleTextField;
	wxComboBox * m_versionComboBox;
	std::array<wxTextCtrl *, GroupSSI::NUMBER_OF_DESCRIPTIONS> m_descriptionTextFields;
	wxTextCtrl * m_runFileTextField;

	SunstormInteractiveMetadataPanel(const SunstormInteractiveMetadataPanel &) = delete;
	const SunstormInteractiveMetadataPanel & operator = (const SunstormInteractiveMetadataPanel &) = delete;
};

#endif // _SUNSTORM_INTERACTIVE_METADATA_PANEL_H_
