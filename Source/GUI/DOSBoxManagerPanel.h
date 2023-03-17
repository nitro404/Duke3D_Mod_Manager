#ifndef _DOSBOX_MANAGER_PANEL_H_
#define _DOSBOX_MANAGER_PANEL_H_

#include "DOSBoxSettingsPanel.h"
#include "DOSBoxVersionPanel.h"
#include "Manager/ModManager.h"

#include <boost/signals2.hpp>
#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/bookctrl.h>

class DOSBoxManagerPanel final
	: public wxPanel
	, public DOSBoxVersionPanel::Listener {
public:
	class Listener {
	public:
		virtual ~Listener();

		virtual void dosboxSettingsChanged() = 0;
		virtual void dosboxSettingsReset() = 0;
		virtual void dosboxSettingsDiscarded() = 0;
		virtual void dosboxSettingsSaved() = 0;
	};

	DOSBoxManagerPanel(std::shared_ptr<ModManager> modManager, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~DOSBoxManagerPanel();

	bool hasDOSBoxVersionPanel(const DOSBoxVersionPanel * dosboxVersionPanel) const;
	bool hasPanelWithDOSBoxVersion(const DOSBoxVersion * dosboxVersion) const;
	bool hasPanelWithDOSBoxVersionName(const std::string & name) const;
	size_t indexOfDOSBoxVersionPanel(const DOSBoxVersionPanel * dosboxVersionPanel) const;
	size_t indexOfPanelWithDOSBoxVersion(const DOSBoxVersion * dosboxVersion) const;
	size_t indexOfPanelWithDOSBoxVersionName(const std::string & name) const;
	DOSBoxVersionPanel * getDOSBoxVersionPanel(size_t dosboxVersionPanelIndex) const;
	DOSBoxVersionPanel * getPanelWithDOSBoxVersion(const DOSBoxVersion * dosboxVersion) const;
	DOSBoxVersionPanel * getPanelWithDOSBoxVersionName(const std::string & name) const;
	DOSBoxVersionPanel * getCurrentDOSBoxVersionPanel() const;
	std::shared_ptr<DOSBoxVersion> getDOSBoxVersion(size_t dosboxVersionPanelIndex) const;
	std::shared_ptr<DOSBoxVersion> getCurrentDOSBoxVersion() const;
	bool selectDOSBoxVersionPanel(size_t dosboxVersionPanelIndex);
	bool selectPanelWithDOSBoxVersion(const DOSBoxVersion & dosboxVersion);
	bool selectPanelWithDOSBoxVersionName(const std::string & name);

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

	size_t numberOfListeners() const;
	bool hasListener(const Listener & listener) const;
	size_t indexOfListener(const Listener & listener) const;
	Listener * getListener(size_t index) const;
	bool addListener(Listener & listener);
	bool removeListener(size_t index);
	bool removeListener(const Listener & listener);
	void clearListeners();

	// DOSBoxVersionPanel::Listener Virtuals
	virtual void dosboxVersionSettingChanged(DOSBoxVersionPanel & dosboxVersionPanel, SettingPanel & settingPanel) override;
	virtual void dosboxVersionChangesDiscarded(DOSBoxVersionPanel & dosboxVersionPanel) override;
	virtual void dosboxVersionReset(DOSBoxVersionPanel & dosboxVersionPanel) override;
	virtual void dosboxVersionSaved(DOSBoxVersionPanel & dosboxVersionPanel) override;

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
	void onDOSBoxSettingModified(SettingPanel & settingPanel);
	void notifyDOSBoxSettingsChanged();
	void notifyDOSBoxSettingsReset();
	void notifyDOSBoxSettingsDiscarded();
	void notifyDOSBoxSettingsSaved();

	std::shared_ptr<ModManager> m_modManager;
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
	boost::signals2::connection m_dosboxSettingModifiedConnection;
	bool m_modified;
	std::vector<Listener *> m_listeners;
};

#endif // _DOSBOX_MANAGER_PANEL_H_
