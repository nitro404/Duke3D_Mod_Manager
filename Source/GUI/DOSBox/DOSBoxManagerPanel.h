#ifndef _DOSBOX_MANAGER_PANEL_H_
#define _DOSBOX_MANAGER_PANEL_H_

#include "DOSBoxSettingsPanel.h"
#include "DOSBoxVersionPanel.h"
#include "Manager/ModManager.h"

#include <Signal/SignalConnectionGroup.h>

#include <boost/signals2.hpp>
#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/bookctrl.h>

#include <future>
#include <string>
#include <vector>

class DOSBoxInstallProgressEvent;
class DOSBoxInstallDoneEvent;

class wxProgressDialog;

class DOSBoxManagerPanel final : public wxPanel {
public:
	DOSBoxManagerPanel(std::shared_ptr<ModManager> modManager, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~DOSBoxManagerPanel();

	bool hasDOSBoxVersionPanel(const DOSBoxVersionPanel * dosboxVersionPanel) const;
	bool hasPanelWithDOSBoxVersion(const DOSBoxVersion * dosboxVersion) const;
	bool hasPanelWithDOSBoxVersionID(const std::string & dosboxVersionID) const;
	size_t indexOfDOSBoxVersionPanel(const DOSBoxVersionPanel * dosboxVersionPanel) const;
	size_t indexOfPanelWithDOSBoxVersion(const DOSBoxVersion * dosboxVersion) const;
	size_t indexOfPanelWithDOSBoxVersionID(const std::string & dosboxVersionID) const;
	DOSBoxVersionPanel * getDOSBoxVersionPanel(size_t dosboxVersionPanelIndex) const;
	DOSBoxVersionPanel * getPanelWithDOSBoxVersion(const DOSBoxVersion * dosboxVersion) const;
	DOSBoxVersionPanel * getPanelWithDOSBoxVersionID(const std::string & dosboxVersionID) const;
	DOSBoxVersionPanel * getCurrentDOSBoxVersionPanel() const;
	std::shared_ptr<DOSBoxVersion> getDOSBoxVersion(size_t dosboxVersionPanelIndex) const;
	std::shared_ptr<DOSBoxVersion> getCurrentDOSBoxVersion() const;
	bool selectDOSBoxVersionPanel(size_t dosboxVersionPanelIndex);
	bool selectPanelWithDOSBoxVersion(const DOSBoxVersion & dosboxVersion);
	bool selectPanelWithDOSBoxVersionID(const std::string & dosboxVersionID);

	bool saveDOSBoxVersions();

	bool areDOSBoxSettingsModified() const;
	bool areDOSBOxSettingsValid() const;
	void resetDefaultDOSBoxSettings();
	void discardDOSBoxSettings();
	bool saveDOSBoxSettings();

	void update();
	bool updateDOSBoxVersionPanel(size_t dosboxVersionPanelIndex);
	void updateDOSBoxVersionPanelNames();
	bool updateDOSBoxVersionPanelName(size_t dosboxVersionPanelIndex);
	void updateButtons();

	bool addDOSBoxVersionPanel(DOSBoxVersionPanel * dosboxVersionPanel);
	bool newDOSBoxVersion();
	bool installDOSBoxVersion(size_t index);
	bool installCurrentDOSBoxVersion();
	bool uninstallDOSBoxVersion(size_t index);
	bool uninstallCurrentDOSBoxVersion();
	bool saveDOSBoxVersion(size_t index);
	bool saveCurrentDOSBoxVersion();
	bool discardDOSBoxVersionChanges(size_t index);
	bool discardCurrentDOSBoxVersionChanges();
	bool resetDOSBoxVersion(size_t index);
	bool resetCurrentDOSBoxVersion();
	bool removeDOSBoxVersion(size_t index);
	bool removeCurrentDOSBoxVersion();

	boost::signals2::signal<void ()> dosboxSettingsChanged;
	boost::signals2::signal<void ()> dosboxSettingsReset;
	boost::signals2::signal<void ()> dosboxSettingsDiscarded;
	boost::signals2::signal<void ()> dosboxSettingsSaved;

private:
	void onNotebookPageChanged(wxBookCtrlEvent & event);
	void onNewDOSBoxVersionButtonPressed(wxCommandEvent & event);
	void onInstallDOSBoxVersionButtonPressed(wxCommandEvent & event);
	void onUninstallDOSBoxVersionButtonPressed(wxCommandEvent & event);
	void onSaveDOSBoxVersionButtonPressed(wxCommandEvent & event);
	void onDiscardDOSBoxVersionChangesButtonPressed(wxCommandEvent & event);
	void onResetDOSBoxVersionButtonPressed(wxCommandEvent & event);
	void onRemoveDOSBoxVersionButtonPressed(wxCommandEvent & event);
	void onSaveDOSBoxSettingsButtonPressed(wxCommandEvent & event);
	void onDiscardDOSBoxSettingsChangesButtonPressed(wxCommandEvent & event);
	void onResetDefaultDOSBoxSettingsButtonPressed(wxCommandEvent & event);
	void onInstallProgress(DOSBoxInstallProgressEvent & event);
	void onInstallDone(DOSBoxInstallDoneEvent & event);
	void onDOSBoxSettingModified(SettingPanel & settingPanel);
	void onDOSBoxVersionSettingChanged(DOSBoxVersionPanel & dosboxVersionPanel, SettingPanel & settingPanel);
	void onDOSBoxVersionChangesDiscarded(DOSBoxVersionPanel & dosboxVersionPanel);
	void onDOSBoxVersionReset(DOSBoxVersionPanel & dosboxVersionPanel);
	void onDOSBoxVersionSaved(DOSBoxVersionPanel & dosboxVersionPanel);

	std::shared_ptr<ModManager> m_modManager;
	std::future<bool> m_installDOSBoxFuture;
	std::vector<SignalConnectionGroup> m_dosboxVersionPanelSignalConnectionGroups;
	wxNotebook * m_notebook;
	wxButton * m_newDOSBoxVersionButton;
	wxButton * m_installDOSBoxVersionButton;
	wxButton * m_uninstallDOSBoxVersionButton;
	wxButton * m_saveDOSBoxVersionButton;
	wxButton * m_discardDOSBoxVersionChangesButton;
	wxButton * m_resetDOSBoxVersionButton;
	wxButton * m_removeDOSBoxVersionButton;
	wxButton * m_saveDOSBoxSettingsButton;
	wxButton * m_discardDOSBoxSettingsChangesButton;
	DOSBoxSettingsPanel * m_dosboxSettingsPanel;
	wxProgressDialog * m_installProgressDialog;
	boost::signals2::connection m_dosboxSettingModifiedConnection;
	bool m_modified;

	DOSBoxManagerPanel(const DOSBoxManagerPanel &) = delete;
	const DOSBoxManagerPanel & operator = (const DOSBoxManagerPanel &) = delete;
};

#endif // _DOSBOX_MANAGER_PANEL_H_
