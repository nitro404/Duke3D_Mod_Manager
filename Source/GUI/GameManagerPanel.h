#ifndef _GAME_MANAGER_PANEL_H_
#define _GAME_MANAGER_PANEL_H_

#include <wx/wxprec.h>

#include "GameVersionPanel.h"

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/bookctrl.h>

#include <memory>

class GameManager;
class GameVersionCollection;
class SettingPanel;

class GameManagerPanel final : public wxPanel,
							   public GameVersionPanel::Listener {
public:
	GameManagerPanel(std::shared_ptr<GameManager> gameManager, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~GameManagerPanel();

	bool hasGameVersionPanel(const GameVersionPanel * gameVersionPanel) const;
	bool hasPanelWithGameVersion(const GameVersion * gameVersion) const;
	bool hasPanelWithGameVersionName(const std::string & name) const;
	size_t indexOfGameVersionPanel(const GameVersionPanel * gameVersionPanel) const;
	size_t indexOfPanelWithGameVersion(const GameVersion * gameVersion) const;
	size_t indexOfPanelWithGameVersionName(const std::string & name) const;
	GameVersionPanel * getGameVersionPanel(size_t gameVersionPanelIndex) const;
	GameVersionPanel * getPanelWithGameVersion(const GameVersion * gameVersion) const;
	GameVersionPanel * getPanelWithGameVersionName(const std::string & name) const;
	GameVersionPanel * getCurrentGameVersionPanel() const;
	std::shared_ptr<GameVersion> getGameVersion(size_t gameVersionPanelIndex) const;
	std::shared_ptr<GameVersion> getCurrentGameVersion() const;
	bool selectGameVersionPanel(size_t gameVersionPanelIndex);
	bool selectPanelWithGameVersion(const GameVersion & gameVersion);
	bool selectPanelWithGameVersionName(const std::string & name);

	bool saveGameVersions();

	void update();
	bool updateGameVersionPanel(size_t gameVersionPanelIndex);
	void updateGameVersionPanelNames();
	bool updateGameVersionPanelName(size_t gameVersionPanelIndex);
	void updateButtons();

	bool addGameVersionPanel(GameVersionPanel * gameVersionPanel);
	bool newGameVersion();
	bool installGameVersion(size_t index);
	bool installCurrentGameVersion();
	bool uninstallGameVersion(size_t index);
	bool uninstallCurrentGameVersion();
	bool saveGameVersion(size_t index);
	bool saveCurrentGameVersion();
	bool discardGameVersionChanges(size_t index);
	bool discardCurrentGameVersionChanges();
	bool resetGameVersion(size_t index);
	bool resetCurrentGameVersion();
	bool removeGameVersion(size_t index);
	bool removeCurrentGameVersion();

	void onNotebookPageChanged(wxBookCtrlEvent & event);
	void onNewButtonPressed(wxCommandEvent & event);
	void onInstallButtonPressed(wxCommandEvent & event);
	void onUninstallButtonPressed(wxCommandEvent & event);
	void onSaveButtonPressed(wxCommandEvent & event);
	void onDiscardChangesButtonPressed(wxCommandEvent & event);
	void onResetButtonPressed(wxCommandEvent & event);
	void onRemoveButtonPressed(wxCommandEvent & event);

	// GameVersionPanel::Listener Virtuals
	virtual void gameVersionSettingChanged(GameVersionPanel & gameVersionPanel, SettingPanel & settingPanel) override;
	virtual void gameVersionChangesDiscarded(GameVersionPanel & gameVersionPanel) override;
	virtual void gameVersionReset(GameVersionPanel & gameVersionPanel) override;
	virtual void gameVersionSaved(GameVersionPanel & gameVersionPanel) override;

private:
	std::shared_ptr<GameManager> m_gameManager;
	wxNotebook * m_notebook;
	wxButton * m_newButton;
	wxButton * m_installButton;
	wxButton * m_uninstallButton;
	wxButton * m_saveButton;
	wxButton * m_discardChangesButton;
	wxButton * m_resetButton;
	wxButton * m_removeButton;
};

#endif // _GAME_MANAGER_PANEL_H_
