#ifndef _GROUP_INFO_PANEL_H_
#define _GROUP_INFO_PANEL_H_

#include "SunstormInteractiveMetadataPanel.h"

#include <boost/signals2.hpp>
#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

class GameFile;
class Group;
class GroupFile;
class MetadataPanel;

class GroupPanel final : public wxPanel {
public:
	GroupPanel(std::unique_ptr<Group> group, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~GroupPanel();

	std::string getPanelName() const;
	const Group * getGroup() const;
	Group * getGroup();
	size_t numberOfFilesSelected() const;
	std::vector<std::shared_ptr<GroupFile>> getSelectedFiles() const;
	size_t getTotalSizeOfSelectedFiles() const;
	size_t extractSelectedFiles(const std::string & directoryPath) const;

	void update();
	void updateFileInfo();

	boost::signals2::signal<void (GroupPanel & /* groupPanel*/)> groupModified;
	boost::signals2::signal<void (GroupPanel & /* groupPanel*/)> groupFileSelectionChanged;

private:
	void onFileSelected(wxCommandEvent & event);
	void onGroupModified(const GameFile & group);

	std::shared_ptr<Group> m_group;
	boost::signals2::connection m_groupModifiedConnection;
	wxStaticText * m_numberOfFilesText;
	wxStaticText * m_groupSizeText;
	wxStaticText * m_fileExtensionsText;
	wxListBox * m_fileListBox;
	wxStaticBox * m_fileInfoBox;
	MetadataPanel * m_fileInfoPanel;
	wxStaticBox * m_ssiMetadataBox;
	SunstormInteractiveMetadataPanel * m_ssiMetadataPanel;
	wxBoxSizer * m_fileInfoBoxSizer;
	wxFlexGridSizer * m_groupPropertiesSizer;

	GroupPanel(const GroupPanel &) = delete;
	const GroupPanel & operator = (const GroupPanel &) = delete;
};

#endif // _GROUP_INFO_PANEL_H_
