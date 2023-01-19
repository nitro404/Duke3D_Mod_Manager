#ifndef _GROUP_EDITOR_PANEL_H_
#define _GROUP_EDITOR_PANEL_H_

#include <wx/wxprec.h>

#include "GroupPanel.h"
#include "Group/Group.h"

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/bookctrl.h>

#include <memory>

class GroupEditorPanel final : public wxPanel,
							   public GroupPanel::Listener {
public:
	GroupEditorPanel(wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~GroupEditorPanel();

	bool hasGroupPanel(const GroupPanel * groupPanel) const;
	bool hasPanelWithGroup(const Group * group) const;
	size_t indexOfGroupPanel(const GroupPanel * groupPanel) const;
	size_t indexOfPanelWithGroup(const Group * group) const;
	size_t indexOfPanelWithGroupFilePath(const std::string & filePath) const;
	GroupPanel * getGroupPanel(size_t groupPanelIndex) const;
	GroupPanel * getPanelWithGroup(const Group * group) const;
	GroupPanel * getPanelWithGroupFilePath(const std::string & filePath) const;
	GroupPanel * getCurrentGroupPanel() const;
	Group * getGroup(size_t groupPanelIndex) const;
	Group * getCurrentGroup() const;

	void update();
	bool updateGroupPanel(size_t groupPanelIndex);
	void updateGroupPanelNames();
	bool updateGroupPanelName(size_t groupPanelIndex);
	void updateButtons();

	bool addGroupPanel(GroupPanel * groupPanel);
	bool newGroup();
	bool openGroup(const std::string & filePath);
	size_t openGroups();
	bool createGroupFromDirectory(const std::string & directoryPath);
	bool createGroupFromDirectory();
	bool saveGroup(Group * group);
	bool saveCurrentGroup();
	bool saveGroupAs(Group * group);
	bool saveCurrentGroupAs();
	size_t addFilesToGroup(Group * group);
	size_t addFilesToCurrentGroup();
	size_t removeSelectedFilesFromGroup(Group * group);
	size_t removeSelectedFilesFromCurrentGroup();
	bool replaceSelectedFileInGroup(Group * group);
	bool replaceSelectedFileInCurrentGroup();
	bool renameSelectedFileInGroup(Group * group);
	bool renameSelectedFileInCurrentGroup();
	size_t extractFilesFromGroup(const Group * group, const std::vector<std::shared_ptr<GroupFile>> & files);
	size_t extractSelectedFilesFromGroup(const Group * group);
	size_t extractSelectedFilesFromCurrentGroup();
	size_t extractAllFilesFromGroup(const Group * group);
	size_t extractAllFilesFromCurrentGroup();
	bool closeGroupPanel(size_t index);
	bool closeCurrentGroupPanel();
	bool closeAllGroupPanels();

	void onNotebookPageChanged(wxBookCtrlEvent & event);
	void onNewButtonPressed(wxCommandEvent & event);
	void onOpenButtonPressed(wxCommandEvent & event);
	void onCreateFromButtonPressed(wxCommandEvent & event);
	void onSaveButtonPressed(wxCommandEvent & event);
	void onSaveAsButtonPressed(wxCommandEvent & event);
	void onAddFilesButtonPressed(wxCommandEvent & event);
	void onRemoveFilesButtonPressed(wxCommandEvent & event);
	void onReplaceFileButtonPressed(wxCommandEvent & event);
	void onRenameFileButtonPressed(wxCommandEvent & event);
	void onExtractFilesButtonPressed(wxCommandEvent & event);
	void onExtractAllFilesButtonPressed(wxCommandEvent & event);
	void onCloseButtonPressed(wxCommandEvent & event);
	void onCloseAllButtonPressed(wxCommandEvent & event);

	// GroupPanel::Listener Virtuals
	virtual void groupModified(GroupPanel * groupPanel, bool modified) override;
	virtual void groupFileSelectionChanged(GroupPanel * groupPanel) override;

private:
	wxNotebook * m_notebook;
	wxButton * m_newButton;
	wxButton * m_openButton;
	wxButton * m_createFromButton;
	wxButton * m_saveButton;
	wxButton * m_saveAsButton;
	wxButton * m_addFilesButton;
	wxButton * m_removeFilesButton;
	wxButton * m_replaceFileButton;
	wxButton * m_renameFileButton;
	wxButton * m_extractFilesButton;
	wxButton * m_extractAllFilesButton;
	wxButton * m_closeButton;
	wxButton * m_closeAllButton;
};

#endif // _GROUP_EDITOR_PANEL_H_