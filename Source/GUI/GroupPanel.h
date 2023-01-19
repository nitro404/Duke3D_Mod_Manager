#ifndef _GROUP_INFO_PANEL_H_
#define _GROUP_INFO_PANEL_H_

#include "Group/Group.h"

#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

class Group;

class GroupPanel final : public wxPanel,
						 public Group::Listener {
public:
	class Listener {
	public:
		virtual ~Listener();

		virtual void groupModified(GroupPanel * groupPanel, bool modified) = 0;
		virtual void groupFileSelectionChanged(GroupPanel * groupPanel) = 0;
	};

	GroupPanel(std::unique_ptr<Group> group, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~GroupPanel();

	std::string getPanelName() const;
	const Group * getGroup() const;
	Group * getGroup();
	size_t numberOfFilesSelected() const;
	std::vector<std::shared_ptr<GroupFile>> getSelectedFiles() const;
	size_t getTotalSizeOfSelectedFiles() const;
	std::string getTotalSizeOfSelectedFilesAsString() const;
	size_t extractSelectedFiles(const std::string & directoryPath) const;

	void update();
	void updateFileInfo();

	void onFileSelected(wxCommandEvent & event);

	size_t numberOfListeners() const;
	bool hasListener(const Listener & listener) const;
	size_t indexOfListener(const Listener & listener) const;
	Listener * getListener(size_t index) const;
	bool addListener(Listener & listener);
	bool removeListener(size_t index);
	bool removeListener(const Listener & listener);
	void clearListeners();

	// Group::Listener Virtuals
	virtual void groupModified(const Group * group, bool modified) override;

private:
	void notifyGroupFileSelectionChanged();

	std::unique_ptr<Group> m_group;
	mutable std::vector<Listener *> m_listeners;

	wxStaticText * m_numberOfFilesText;
	wxStaticText * m_groupSizeText;
	wxStaticText * m_fileExtensionsText;
	wxListBox * m_fileListBox;
	wxStaticBox * m_fileInfoBox;
	wxPanel * m_fileInfoPanel;
	wxStaticText * m_fileNameLabel;
	wxStaticText * m_fileNameText;
	wxStaticText * m_numberOfFilesSelectedLabel;
	wxStaticText * m_numberOfFilesSelectedText;
	wxStaticText * m_fileSizeLabel;
	wxStaticText * m_fileSizeText;
	wxBoxSizer * m_fileInfoBoxSizer;
	wxFlexGridSizer * m_groupPropertiesSizer;
};

#endif // _GROUP_INFO_PANEL_H_
