#ifndef _GAME_MANAGER_PANEL_H_
#define _GAME_MANAGER_PANEL_H_

#include <Signal/SignalConnectionGroup.h>

#include <wx/wxprec.h>

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/bookctrl.h>

#include <future>
#include <memory>
#include <string>
#include <vector>

class GameInstallDoneEvent;
class GameInstallProgressEvent;
class GameManager;
class GameVersion;
class GameVersionCollection;
class GameVersionPanel;
class SettingPanel;

class wxProgressDialog;

class GameManagerPanel final : public wxPanel {
public:
	GameManagerPanel(std::shared_ptr<GameManager> gameManager, wxWindow * parent, wxWindowID windowID = wxID_ANY, const wxPoint & position = wxDefaultPosition, const wxSize & size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
	virtual ~GameManagerPanel();

	bool hasGameVersionPanel(const GameVersionPanel * gameVersionPanel) const;
	bool hasPanelWithGameVersion(const GameVersion * gameVersion) const;
	bool hasPanelWithGameVersionID(const std::string & gameVersionID) const;
	size_t indexOfGameVersionPanel(const GameVersionPanel * gameVersionPanel) const;
	size_t indexOfPanelWithGameVersion(const GameVersion * gameVersion) const;
	size_t indexOfPanelWithGameVersionID(const std::string & gameVersionID) const;
	GameVersionPanel * getGameVersionPanel(size_t gameVersionPanelIndex) const;
	GameVersionPanel * getPanelWithGameVersion(const GameVersion * gameVersion) const;
	GameVersionPanel * getPanelWithGameVersionID(const std::string & gameVersionID) const;
	GameVersionPanel * getCurrentGameVersionPanel() const;
	std::shared_ptr<GameVersion> getGameVersion(size_t gameVersionPanelIndex) const;
	std::shared_ptr<GameVersion> getCurrentGameVersion() const;
	bool selectGameVersionPanel(size_t gameVersionPanelIndex);
	bool selectPanelWithGameVersion(const GameVersion & gameVersion);
	bool selectPanelWithGameVersionID(const std::string & gameVersionID);

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

private:
	void onNotebookPageChanged(wxBookCtrlEvent & event);
	void onNewButtonPressed(wxCommandEvent & event);
	void onInstallButtonPressed(wxCommandEvent & event);
	void onUninstallButtonPressed(wxCommandEvent & event);
	void onSaveButtonPressed(wxCommandEvent & event);
	void onDiscardChangesButtonPressed(wxCommandEvent & event);
	void onResetButtonPressed(wxCommandEvent & event);
	void onRemoveButtonPressed(wxCommandEvent & event);
	void onInstallProgress(GameInstallProgressEvent & event);
	void onInstallDone(GameInstallDoneEvent & event);
	void onGameVersionSettingChanged(GameVersionPanel & gameVersionPanel, SettingPanel & settingPanel);
	void onGameVersionNotesChanged(GameVersionPanel & gameVersionPanel);
	void onGameVersionChangesDiscarded(GameVersionPanel & gameVersionPanel);
	void onGameVersionReset(GameVersionPanel & gameVersionPanel);
	void onGameVersionSaved(GameVersionPanel & gameVersionPanel);

	std::shared_ptr<GameManager> m_gameManager;
	std::future<bool> m_installGameFuture;
	std::atomic<bool> m_gameInstallationCancelled;
	std::vector<SignalConnectionGroup> m_gameVersionPanelSignalConnectionGroups;
	wxNotebook * m_notebook;
	wxButton * m_newButton;
	wxButton * m_installButton;
	wxButton * m_uninstallButton;
	wxButton * m_saveButton;
	wxButton * m_discardChangesButton;
	wxButton * m_resetButton;
	wxButton * m_removeButton;
	wxProgressDialog * m_installProgressDialog;

	GameManagerPanel(const GameManagerPanel &) = delete;
	const GameManagerPanel & operator = (const GameManagerPanel &) = delete;
};

#endif // _GAME_MANAGER_PANEL_H_
