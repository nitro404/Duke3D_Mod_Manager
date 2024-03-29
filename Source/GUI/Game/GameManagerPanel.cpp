#include "GameManagerPanel.h"

#include "../SettingPanel.h"
#include "../WXUtilities.h"
#include "Game/GameManager.h"
#include "Game/GameVersionCollection.h"
#include "GameVersionPanel.h"
#include "Manager/SettingsManager.h"

#include <Analytics/Segment/SegmentAnalytics.h>
#include <Network/HTTPRequest.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/TimeUtilities.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/gbsizer.h>
#include <wx/progdlg.h>
#include <wx/textdlg.h>
#include <wx/wrapsizer.h>

#include <any>
#include <filesystem>
#include <map>
#include <sstream>

wxDECLARE_EVENT(EVENT_GAME_INSTALL_PROGRESS, GameInstallProgressEvent);

class GameInstallProgressEvent final : public wxEvent {
public:
	GameInstallProgressEvent(int value = 0, const std::string & message = {})
		: wxEvent(0, EVENT_GAME_INSTALL_PROGRESS)
		, m_value(value)
		, m_message(message) { }

	virtual ~GameInstallProgressEvent() { }

	int getValue() const {
		return m_value;
	}

	void setValue(int value) {
		m_value = value;
	}

	const std::string & getMessage() const {
		return m_message;
	}

	void setMessage(const std::string & message) {
		m_message = message;
	}

	virtual wxEvent * Clone() const override {
		return new GameInstallProgressEvent(*this);
	}

	DECLARE_DYNAMIC_CLASS(GameInstallProgressEvent);

private:
	int m_value;
	std::string m_message;
};

IMPLEMENT_DYNAMIC_CLASS(GameInstallProgressEvent, wxEvent);

wxDECLARE_EVENT(EVENT_GAME_INSTALL_DONE, GameInstallDoneEvent);

class GameInstallDoneEvent final : public wxEvent {
public:
	GameInstallDoneEvent(bool success = true)
		: wxEvent(0, EVENT_GAME_INSTALL_DONE)
		, m_success(success) { }

	virtual ~GameInstallDoneEvent() { }

	virtual wxEvent * Clone() const override {
		return new GameInstallDoneEvent(*this);
	}

	bool wasSuccessful() const {
		return m_success;
	}

	DECLARE_DYNAMIC_CLASS(GameInstallDoneEvent);

private:
	bool m_success;
};

IMPLEMENT_DYNAMIC_CLASS(GameInstallDoneEvent, wxEvent);

wxDEFINE_EVENT(EVENT_GAME_INSTALL_PROGRESS, GameInstallProgressEvent);
wxDEFINE_EVENT(EVENT_GAME_INSTALL_DONE, GameInstallDoneEvent);

GameManagerPanel::GameManagerPanel(std::shared_ptr<GameManager> gameManager, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Game Manager")
	, m_gameManager(gameManager)
	, m_gameInstallationCancelled(false)
	, m_notebook(nullptr)
	, m_newButton(nullptr)
	, m_installButton(nullptr)
	, m_uninstallButton(nullptr)
	, m_saveButton(nullptr)
	, m_discardChangesButton(nullptr)
	, m_resetButton(nullptr)
	, m_removeButton(nullptr)
	, m_installProgressDialog(nullptr) {
	m_notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP | wxNB_MULTILINE, "Game Versions");
	m_notebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &GameManagerPanel::onNotebookPageChanged, this);

	wxScrolledWindow * actionsPanel = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, "Game Version Actions");
	actionsPanel->SetScrollRate(5, 5);

	m_newButton = new wxButton(actionsPanel, wxID_ANY, "New", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Create New Game Version");
	m_newButton->Bind(wxEVT_BUTTON, &GameManagerPanel::onNewButtonPressed, this);

	m_installButton = new wxButton(actionsPanel, wxID_ANY, "Install", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Install Game Version");
	m_installButton->Bind(wxEVT_BUTTON, &GameManagerPanel::onInstallButtonPressed, this);
	m_installButton->Disable();

	m_uninstallButton = new wxButton(actionsPanel, wxID_ANY, "Uninstall", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Uninstall Game Version");
	m_uninstallButton->Bind(wxEVT_BUTTON, &GameManagerPanel::onUninstallButtonPressed, this);
	m_uninstallButton->Disable();

	m_resetButton = new wxButton(actionsPanel, wxID_ANY, "Reset", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Reset Game Version");
	m_resetButton->Bind(wxEVT_BUTTON, &GameManagerPanel::onResetButtonPressed, this);
	m_resetButton->Disable();

	m_removeButton = new wxButton(actionsPanel, wxID_ANY, "Remove", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Remove Game Version");
	m_removeButton->Bind(wxEVT_BUTTON, &GameManagerPanel::onRemoveButtonPressed, this);
	m_removeButton->Disable();

	m_saveButton = new wxButton(actionsPanel, wxID_ANY, "Save", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Save Game Version");
	m_saveButton->Bind(wxEVT_BUTTON, &GameManagerPanel::onSaveButtonPressed, this);
	m_saveButton->Disable();

	m_discardChangesButton = new wxButton(actionsPanel, wxID_ANY, "Discard Changes", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Discard Game Version Changes");
	m_discardChangesButton->Bind(wxEVT_BUTTON, &GameManagerPanel::onDiscardChangesButtonPressed, this);
	m_discardChangesButton->Disable();

	int border = 2;

	wxWrapSizer * actionsPanelSizer = new wxWrapSizer(wxHORIZONTAL);
	actionsPanelSizer->Add(m_newButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanelSizer->Add(m_installButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanelSizer->Add(m_uninstallButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanelSizer->Add(m_resetButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanelSizer->Add(m_removeButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanelSizer->Add(m_saveButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanelSizer->Add(m_discardChangesButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanel->SetSizerAndFit(actionsPanelSizer);

	wxGridBagSizer * gameManagerPanelSizer = new wxGridBagSizer(border, border);
	gameManagerPanelSizer->Add(m_notebook, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	gameManagerPanelSizer->Add(actionsPanel, wxGBPosition(1, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	gameManagerPanelSizer->AddGrowableRow(0, 1);
	gameManagerPanelSizer->AddGrowableRow(1, 0);
	gameManagerPanelSizer->AddGrowableCol(0, 1);
	SetSizerAndFit(gameManagerPanelSizer);

	GameVersionPanel * gameVersionPanel = nullptr;

	for(const std::shared_ptr<GameVersion> & gameVersion : m_gameManager->getGameVersions()->getGameVersions()) {
		gameVersionPanel = new GameVersionPanel(gameVersion, m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

		m_gameVersionPanelSignalConnectionGroups.push_back(SignalConnectionGroup(
			gameVersionPanel->gameVersionSettingChanged.connect(std::bind(&GameManagerPanel::onGameVersionSettingChanged, this, std::placeholders::_1, std::placeholders::_2)),
			gameVersionPanel->gameVersionNotesChanged.connect(std::bind(&GameManagerPanel::onGameVersionNotesChanged, this, std::placeholders::_1)),
			gameVersionPanel->gameVersionChangesDiscarded.connect(std::bind(&GameManagerPanel::onGameVersionChangesDiscarded, this, std::placeholders::_1)),
			gameVersionPanel->gameVersionReset.connect(std::bind(&GameManagerPanel::onGameVersionReset, this, std::placeholders::_1)),
			gameVersionPanel->gameVersionSaved.connect(std::bind(&GameManagerPanel::onGameVersionSaved, this, std::placeholders::_1))
		));

		addGameVersionPanel(gameVersionPanel);
	}

	update();

	Bind(EVENT_GAME_INSTALL_PROGRESS, &GameManagerPanel::onInstallProgress, this);
	Bind(EVENT_GAME_INSTALL_DONE, &GameManagerPanel::onInstallDone, this);
}

GameManagerPanel::~GameManagerPanel() {
	for(SignalConnectionGroup & gameVersionPanelSignalConnectionGroup : m_gameVersionPanelSignalConnectionGroups) {
		gameVersionPanelSignalConnectionGroup.disconnect();
	}
}

bool GameManagerPanel::hasGameVersionPanel(const GameVersionPanel * gameVersionPanel) const {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		if(m_notebook->GetPage(i) == gameVersionPanel) {
			return true;
		}
	}

	return false;
}

bool GameManagerPanel::hasPanelWithGameVersion(const GameVersion * gameVersion) const {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		if(getGameVersion(i).get() == gameVersion) {
			return true;
		}
	}

	return false;
}

bool GameManagerPanel::hasPanelWithGameVersionID(const std::string & gameVersionID) const {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(getGameVersion(i)->getID(), gameVersionID)) {
			return true;
		}
	}

	return false;
}

size_t GameManagerPanel::indexOfGameVersionPanel(const GameVersionPanel * gameVersionPanel) const {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		if(m_notebook->GetPage(i) == gameVersionPanel) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t GameManagerPanel::indexOfPanelWithGameVersion(const GameVersion * gameVersion) const {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		if(getGameVersion(i).get() == gameVersion) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t GameManagerPanel::indexOfPanelWithGameVersionID(const std::string & gameVersionID) const {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(getGameVersion(i)->getID(), gameVersionID)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

GameVersionPanel * GameManagerPanel::getGameVersionPanel(size_t gameVersionPanelIndex) const {
	if(gameVersionPanelIndex >= m_notebook->GetPageCount()) {
		return nullptr;
	}

	return dynamic_cast<GameVersionPanel *>(m_notebook->GetPage(gameVersionPanelIndex));
}

GameVersionPanel * GameManagerPanel::getPanelWithGameVersion(const GameVersion * gameVersion) const {
	return getGameVersionPanel(indexOfPanelWithGameVersion(gameVersion));
}

GameVersionPanel * GameManagerPanel::getPanelWithGameVersionID(const std::string & gameVersionID) const {
	return getGameVersionPanel(indexOfPanelWithGameVersionID(gameVersionID));
}

GameVersionPanel * GameManagerPanel::getCurrentGameVersionPanel() const {
	return dynamic_cast<GameVersionPanel *>(m_notebook->GetCurrentPage());
}

std::shared_ptr<GameVersion> GameManagerPanel::getGameVersion(size_t gameVersionPanelIndex) const {
	GameVersionPanel * gameVersionPanel = getGameVersionPanel(gameVersionPanelIndex);

	if(gameVersionPanel == nullptr) {
		return nullptr;
	}

	return gameVersionPanel->getGameVersion();
}

std::shared_ptr<GameVersion> GameManagerPanel::getCurrentGameVersion() const {
	GameVersionPanel * gameVersionPanel = getCurrentGameVersionPanel();

	if(gameVersionPanel == nullptr) {
		return nullptr;
	}

	return gameVersionPanel->getGameVersion();
}

bool GameManagerPanel::selectGameVersionPanel(size_t gameVersionPanelIndex) {
	if(gameVersionPanelIndex >= m_notebook->GetPageCount()) {
		return false;
	}

	m_notebook->ChangeSelection(gameVersionPanelIndex);

	updateButtons();

	return true;
}

bool GameManagerPanel::selectPanelWithGameVersion(const GameVersion & gameVersion) {
	size_t gameVersionPanelIndex = indexOfPanelWithGameVersion(&gameVersion);

	if(gameVersionPanelIndex == std::numeric_limits<size_t>::max()) {
		return false;
	}

	m_notebook->ChangeSelection(gameVersionPanelIndex);

	updateButtons();

	return true;
}

bool GameManagerPanel::selectPanelWithGameVersionID(const std::string & gameVersionID) {
	size_t gameVersionPanelIndex = indexOfPanelWithGameVersionID(gameVersionID);

	if(gameVersionPanelIndex == std::numeric_limits<size_t>::max()) {
		return false;
	}

	m_notebook->ChangeSelection(gameVersionPanelIndex);

	updateButtons();

	return true;
}

bool GameManagerPanel::saveGameVersions() {
	SettingsManager * settings = SettingsManager::getInstance();

	if(!m_gameManager->getGameVersions()->saveTo(settings->gameVersionsListFilePath)) {
		spdlog::error("Failed to save game versions to: '{}'!", settings->gameVersionsListFilePath);
		return false;
	}

	spdlog::info("Game versions saved to: '{}'.", settings->gameVersionsListFilePath);

	return true;
}

void GameManagerPanel::update() {
	updateButtons();
	updateGameVersionPanelNames();
}

bool GameManagerPanel::updateGameVersionPanel(size_t gameVersionPanelIndex) {
	GameVersionPanel * gameVersionPanel = getGameVersionPanel(gameVersionPanelIndex);

	if(gameVersionPanel == nullptr) {
		return false;
	}

	updateButtons();
	updateGameVersionPanelName(gameVersionPanelIndex);

	return true;
}

void GameManagerPanel::updateGameVersionPanelNames() {
	for(size_t i = 0; i < m_notebook->GetPageCount(); i++) {
		updateGameVersionPanelName(i);
	}
}

bool GameManagerPanel::updateGameVersionPanelName(size_t gameVersionPanelIndex) {
	GameVersionPanel * gameVersionPanel = getGameVersionPanel(gameVersionPanelIndex);

	if(gameVersionPanel == nullptr) {
		return false;
	}

	m_notebook->SetPageText(gameVersionPanelIndex, wxString::FromUTF8(gameVersionPanel->getPanelName()));

	return true;
}

void GameManagerPanel::updateButtons() {
	GameVersionPanel * gameVersionPanel = getCurrentGameVersionPanel();
	std::shared_ptr<GameVersion> gameVersion;

	if(gameVersionPanel != nullptr) {
		gameVersion = gameVersionPanel->getGameVersion();
	}

	bool isGameVersionModified = gameVersionPanel != nullptr ? gameVersionPanel->isModified() : false;
	bool isGameVersionInstallable = gameVersion != nullptr ? GameManager::isGameDownloadable(gameVersion->getID()) : false;
	bool isGameVersionInstalled = gameVersion != nullptr ? gameVersion->hasGamePath() : false;
	bool isGameVersionRemovable = gameVersion != nullptr ? gameVersion->isRemovable() : false;

	WXUtilities::setButtonEnabled(m_installButton, isGameVersionInstallable && !isGameVersionInstalled);
	WXUtilities::setButtonEnabled(m_uninstallButton, isGameVersionInstalled);
	WXUtilities::setButtonEnabled(m_saveButton, isGameVersionModified);
	WXUtilities::setButtonEnabled(m_discardChangesButton, isGameVersionModified);
	WXUtilities::setButtonEnabled(m_resetButton, gameVersionPanel != nullptr);
	WXUtilities::setButtonEnabled(m_removeButton, isGameVersionRemovable);
}

bool GameManagerPanel::addGameVersionPanel(GameVersionPanel * gameVersionPanel) {
	if(gameVersionPanel == nullptr || hasGameVersionPanel(gameVersionPanel)) {
		return false;
	}

	m_gameVersionPanelSignalConnectionGroups.push_back(SignalConnectionGroup(
		gameVersionPanel->gameVersionSettingChanged.connect(std::bind(&GameManagerPanel::onGameVersionSettingChanged, this, std::placeholders::_1, std::placeholders::_2)),
		gameVersionPanel->gameVersionNotesChanged.connect(std::bind(&GameManagerPanel::onGameVersionNotesChanged, this, std::placeholders::_1)),
		gameVersionPanel->gameVersionChangesDiscarded.connect(std::bind(&GameManagerPanel::onGameVersionChangesDiscarded, this, std::placeholders::_1)),
		gameVersionPanel->gameVersionReset.connect(std::bind(&GameManagerPanel::onGameVersionReset, this, std::placeholders::_1)),
		gameVersionPanel->gameVersionSaved.connect(std::bind(&GameManagerPanel::onGameVersionSaved, this, std::placeholders::_1))
	));

	m_notebook->AddPage(gameVersionPanel, wxString::FromUTF8(gameVersionPanel->getPanelName()));
	m_notebook->ChangeSelection(m_notebook->GetPageCount() - 1);

	updateButtons();

	return true;
}

bool GameManagerPanel::newGameVersion() {
	std::vector<std::string> gameVersionTemplates({ "Blank" });

	for(const GameVersion * gameVersion : GameVersion::DEFAULT_GAME_VERSIONS) {
		gameVersionTemplates.push_back(gameVersion->getLongName());
	}

	int selectedGameVersionTemplateIndex = wxGetSingleChoiceIndex(
		"Please choose a game version to inherit settings from:",
		"Choose Game Version Template",
		WXUtilities::createItemWXArrayString(gameVersionTemplates),
		0,
		this
	);

	if(selectedGameVersionTemplateIndex == wxNOT_FOUND || selectedGameVersionTemplateIndex > gameVersionTemplates.size()) {
		return false;
	}

	std::shared_ptr<GameVersion> gameVersionTemplate;

	if(selectedGameVersionTemplateIndex != 0) {
		gameVersionTemplate = GameVersion::DEFAULT_GAME_VERSIONS[selectedGameVersionTemplateIndex - 1]->createTemplateFrom();
	}

	if(!addGameVersionPanel(new GameVersionPanel(gameVersionTemplate, m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL))) {
		return false;
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["templateGameVersionID"] = gameVersionTemplate->getID();

		SegmentAnalytics::getInstance()->track("New Game Version Created", properties);
	}

	return true;
}

bool GameManagerPanel::installGameVersion(size_t index) {
	if(m_installProgressDialog) {
		spdlog::error("Game installation already in progress!");
		return false;
	}

	m_gameInstallationCancelled = false;

	GameVersionPanel * gameVersionPanel = getGameVersionPanel(index);

	if(gameVersionPanel == nullptr) {
		return false;
	}

	std::shared_ptr<GameVersion> gameVersion(gameVersionPanel->getGameVersion());

	if(gameVersion->hasGamePath() || !gameVersion->hasID() || !GameManager::isGameDownloadable(gameVersion->getID()) || !m_gameManager->isInitialized()) {
		return false;
	}

	SettingsManager * settings = SettingsManager::getInstance();

	if(!settings->gameDisclaimerAcknowledged) {
		int result = wxMessageBox("By default the latest custom Duke Nukem 3D port game version will be automatically downloaded externally from the official website. While this is generally quite safe, I assume no responsibility for any damages incurred as a result of this in the case that the website is compromised or malicious files are distributed on it. Do you understand the risk and wish to proceed?", "Comfirm Installation", wxOK | wxCANCEL | wxICON_WARNING, this);

		if(result == wxCANCEL) {
			return false;
		}

		settings->gameDisclaimerAcknowledged = true;
	}

	wxDirDialog selectInstallDirectoryDialog(this, "Install Game to Directory", std::filesystem::current_path().string(), wxDD_DIR_MUST_EXIST, wxDefaultPosition, wxDefaultSize, "Install Game");
	int selectInstallDirectoryResult = selectInstallDirectoryDialog.ShowModal();

	if(selectInstallDirectoryResult == wxID_CANCEL) {
		return false;
	}

	std::string destinationDirectoryPath(selectInstallDirectoryDialog.GetPath());

	if(Utilities::areStringsEqual(Utilities::getAbsoluteFilePath(destinationDirectoryPath), Utilities::getAbsoluteFilePath(std::filesystem::current_path().string()))) {
		wxMessageBox("Cannot install game directly into mod manager directory!", "Invalid Installation Directory", wxOK | wxICON_ERROR, this);
		return false;
	}

	if(!std::filesystem::is_directory(std::filesystem::path(destinationDirectoryPath))) {
		wxMessageBox("Invalid game installation directory selected!", "Invalid Installation Directory", wxOK | wxICON_ERROR, this);
		return false;
	}

	for(const std::filesystem::directory_entry & entry : std::filesystem::directory_iterator(std::filesystem::path(destinationDirectoryPath))) {
		int installToNonEmptyDirectoryResult = wxMessageBox(fmt::format("Game installation installation directory '{}' is not empty!\n\nInstalling to a directory which already contains files may cause issues. Are you sure you would like to install '{}' to this directory?", destinationDirectoryPath, gameVersion->getLongName()), "Non-Empty Installation Directory", wxYES_NO | wxCANCEL | wxICON_WARNING, this);

		if(installToNonEmptyDirectoryResult != wxYES) {
			return false;
		}

		break;
	}

	bool groupFileDownloaded = m_gameManager->isGroupFileDownloaded(gameVersion->getID());

	m_installProgressDialog = new wxProgressDialog("Installing", fmt::format("Installing '{}', please wait...", gameVersion->getLongName()), 101, this, wxPD_CAN_ABORT | wxPD_REMAINING_TIME);
	m_installProgressDialog->SetIcon(wxICON(D3DMODMGR_ICON));

	SignalConnectionGroup gameDownloadConnections(
		m_gameManager->installStatusChanged.connect([this](const std::string & statusMessage) {
			QueueEvent(new GameInstallProgressEvent(m_installProgressDialog->GetValue(), statusMessage));
		}),
		m_gameManager->gameDownloadProgress.connect([this, groupFileDownloaded](GameVersion & gameVersion, HTTPRequest & request, size_t numberOfBytesDownloaded, size_t totalNumberOfBytes) {
			QueueEvent(new GameInstallProgressEvent(
				static_cast<int>(static_cast<double>(numberOfBytesDownloaded) / static_cast<double>(totalNumberOfBytes) * (groupFileDownloaded ? 100.0 : 50.0)),
				fmt::format("Downloaded {} / {} of '{}' game files from: '{}'.", Utilities::fileSizeToString(numberOfBytesDownloaded), Utilities::fileSizeToString(totalNumberOfBytes), gameVersion.getLongName(), request.getUrl())
			));

			return !m_gameInstallationCancelled;
		})
	);

	if(!groupFileDownloaded) {
		gameDownloadConnections.addConnection(m_gameManager->groupDownloadProgress.connect([this](GameVersion & gameVersion, HTTPRequest & request, size_t numberOfBytesDownloaded, size_t totalNumberOfBytes) {
			QueueEvent(new GameInstallProgressEvent(
				static_cast<int>(static_cast<double>(numberOfBytesDownloaded) / static_cast<double>(totalNumberOfBytes) * 50.0) + 50,
				fmt::format("Downloaded {} / {} of '{}' group file from: '{}'.", Utilities::fileSizeToString(numberOfBytesDownloaded), Utilities::fileSizeToString(totalNumberOfBytes), m_gameManager->getGroupGameVersion(gameVersion.getID())->getLongName(), request.getUrl())
			));

			return !m_gameInstallationCancelled;
		}));
	}

	m_installGameFuture = std::async(std::launch::async, [this, gameVersion, gameVersionPanel, destinationDirectoryPath, gameDownloadConnections]() mutable {
		std::string newGameExecutableName;
		bool aborted = false;
		bool gameInstalled = m_gameManager->installGame(gameVersion->getID(), destinationDirectoryPath, &newGameExecutableName, false, true, &aborted);

		gameDownloadConnections.disconnect();

		if(aborted) {
			QueueEvent(new GameInstallDoneEvent(false));

			return false;
		}
		else if(!gameInstalled) {
			QueueEvent(new GameInstallDoneEvent(false));

			wxMessageBox(fmt::format("Failed to install '{}' to '{}'!\n\nCheck console for details.", gameVersion->getLongName(), destinationDirectoryPath), "Installation Failed", wxOK | wxICON_ERROR, this);

			return false;
		}

		spdlog::info("Setting '{}' game path to '{}'.", gameVersion->getLongName(), destinationDirectoryPath);

		gameVersionPanel->discardGamePathChanges();
		gameVersion->setGamePath(destinationDirectoryPath);

		if(!newGameExecutableName.empty() && !Utilities::areStringsEqual(gameVersion->getGameExecutableName(), newGameExecutableName)) {
			spdlog::info("Updating '{}' game executable name from '{}' to '{}'.", gameVersion->getLongName(), gameVersion->getGameExecutableName(), newGameExecutableName);

			gameVersion->setGameExecutableName(newGameExecutableName);
		}

		saveGameVersions();

		updateButtons();

		QueueEvent(new GameInstallDoneEvent(true));

		wxMessageBox(fmt::format("'{}' was successfully installed to: '{}'!", gameVersion->getLongName(), destinationDirectoryPath), "Game Installed", wxOK | wxICON_INFORMATION, this);

		return true;
	});

	return true;
}

bool GameManagerPanel::installCurrentGameVersion() {
	return installGameVersion(m_notebook->GetSelection());
}

bool GameManagerPanel::uninstallGameVersion(size_t index) {
	GameVersionPanel * gameVersionPanel = getGameVersionPanel(index);

	if(gameVersionPanel == nullptr) {
		return false;
	}

	std::shared_ptr<GameVersion> gameVersion(gameVersionPanel->getGameVersion());

	if(!gameVersion->hasID() || !gameVersion->hasGamePath()) {
		return false;
	}

	if(!std::filesystem::is_directory(std::filesystem::path(gameVersion->getGamePath()))) {
		wxMessageBox(fmt::format("'{}' path '{}' is not a valid directory.", gameVersion->getLongName(), gameVersion->getGamePath()), "Uninstall Failed", wxOK | wxICON_WARNING, this);

		return false;
	}

	int uninstallConfirmationResult = wxMessageBox(fmt::format("Are you sure you want to uninstall '{}' from '{}'? This will remove all files located in this directory, so be sure to back up any valuable data!", gameVersion->getLongName(), gameVersion->getGamePath()), "Uninstall Confirmation", wxYES_NO | wxCANCEL | wxICON_QUESTION, this);

	if(uninstallConfirmationResult != wxYES) {
		return false;
	}

	std::error_code errorCode;
	std::filesystem::remove_all(std::filesystem::path(gameVersion->getGamePath()), errorCode);

	if(errorCode) {
		wxMessageBox(fmt::format("Failed to remove '{}' game directory '{}': {}", gameVersion->getLongName(), gameVersion->getGamePath(), errorCode.message()), "Directory Removal Failed", wxOK | wxICON_ERROR, this);
		return false;
	}

	spdlog::info("Clearing '{}' game path.", gameVersion->getLongName());

	gameVersionPanel->discardGamePathChanges();
	gameVersion->clearGamePath();

	saveGameVersions();

	updateButtons();

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		gameVersion->addMetadata(properties);

		SegmentAnalytics::getInstance()->track("Uninstalled Game", properties);
	}

	wxMessageBox(fmt::format("'{}' was successfully uninstalled!", gameVersion->getLongName()), "Game Uninstalled", wxOK | wxICON_INFORMATION, this);

	return true;
}

bool GameManagerPanel::uninstallCurrentGameVersion() {
	return uninstallGameVersion(m_notebook->GetSelection());
}

bool GameManagerPanel::saveGameVersion(size_t index) {
	GameVersionPanel * gameVersionPanel = getGameVersionPanel(index);

	if(gameVersionPanel == nullptr) {
		return false;
	}

	std::shared_ptr<GameVersion> gameVersion(gameVersionPanel->getGameVersion());

	if(!gameVersionPanel->save()) {
		return false;
	}

	if(!gameVersion->isValid()) {
		wxMessageBox(fmt::format("'{}' game version is not valid!", gameVersion->getLongName()), "Invalid Game", wxOK | wxICON_WARNING, this);

		return false;
	}

	std::shared_ptr<GameVersionCollection> gameVersions(m_gameManager->getGameVersions());

	if(gameVersions->addGameVersion(gameVersion)) {
		spdlog::info("Added new game version to collection with name '{}'.", gameVersion->getLongName());
	}

	if(!saveGameVersions()) {
		return false;
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		gameVersion->addMetadata(properties);

		SegmentAnalytics::getInstance()->track("Saved Game Configuration", properties);
	}

	return true;
}

bool GameManagerPanel::saveCurrentGameVersion() {
	return saveGameVersion(m_notebook->GetSelection());
}

bool GameManagerPanel::discardGameVersionChanges(size_t index) {
	GameVersionPanel * gameVersionPanel = getGameVersionPanel(index);

	if(gameVersionPanel == nullptr) {
		return false;
	}

	gameVersionPanel->discard();

	return true;
}

bool GameManagerPanel::discardCurrentGameVersionChanges() {
	return discardGameVersionChanges(m_notebook->GetSelection());
}

bool GameManagerPanel::resetGameVersion(size_t gameVersionPanelIndex) {
	GameVersionPanel * gameVersionPanel = getGameVersionPanel(gameVersionPanelIndex);

	if(gameVersionPanel == nullptr) {
		return false;
	}

	std::shared_ptr<GameVersion> gameVersion(gameVersionPanel->getGameVersion());

	int resetResult = wxMessageBox(fmt::format("Are you sure you want to reset the '{}' game version to its default configuration?", gameVersion->hasID() ? gameVersion->getLongName() : "NEW"), "Reset Game", wxICON_QUESTION | wxYES_NO | wxCANCEL, this);

	if(resetResult != wxYES) {
		return false;
	}

	gameVersionPanel->reset();

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["ganeID"] = gameVersion->getID();

		SegmentAnalytics::getInstance()->track("Reset Game Configuration", properties);
	}

	return true;
}

bool GameManagerPanel::resetCurrentGameVersion() {
	return resetGameVersion(m_notebook->GetSelection());
}

bool GameManagerPanel::removeGameVersion(size_t gameVersionPanelIndex) {
	std::shared_ptr<GameVersion> gameVersion = getGameVersion(gameVersionPanelIndex);

	if(gameVersion == nullptr || !gameVersion->isRemovable()) {
		return false;
	}

	int removeResult = wxMessageBox(fmt::format("Are you sure you want to remove the '{}' game version?", gameVersion->hasID() ? gameVersion->getLongName() : "NEW"), "Remove Game", wxICON_QUESTION | wxYES_NO | wxCANCEL, this);

	if(removeResult != wxYES) {
		return false;
	}

	m_gameManager->getGameVersions()->removeGameVersion(*gameVersion);

	saveGameVersions();

	if(m_notebook->GetPageCount() == 1) {
		m_newButton->SetFocus();
	}

	m_gameVersionPanelSignalConnectionGroups[gameVersionPanelIndex].disconnect();
	m_gameVersionPanelSignalConnectionGroups.erase(m_gameVersionPanelSignalConnectionGroups.begin() + gameVersionPanelIndex);

	GameVersionPanel * gameVersionPanel = getGameVersionPanel(gameVersionPanelIndex);
	m_notebook->RemovePage(gameVersionPanelIndex);
	delete gameVersionPanel;

	updateButtons();

	return true;
}

bool GameManagerPanel::removeCurrentGameVersion() {
	return removeGameVersion(m_notebook->GetSelection());
}

void GameManagerPanel::onNotebookPageChanged(wxBookCtrlEvent & event) {
	updateButtons();

	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->segmentAnalyticsEnabled) {
		std::shared_ptr<GameVersion> currentGameVersion(getCurrentGameVersion());

		if(currentGameVersion != nullptr && currentGameVersion->hasID() && currentGameVersion->hasLongName()) {
			std::map<std::string, std::any> properties;
			currentGameVersion->addMetadata(properties);

			SegmentAnalytics::getInstance()->screen(currentGameVersion->getLongName(), "Game Manager", properties);
		}
	}
}

void GameManagerPanel::onNewButtonPressed(wxCommandEvent & event) {
	newGameVersion();
}

void GameManagerPanel::onInstallButtonPressed(wxCommandEvent & event) {
	installCurrentGameVersion();
}

void GameManagerPanel::onUninstallButtonPressed(wxCommandEvent & event) {
	uninstallCurrentGameVersion();
}

void GameManagerPanel::onSaveButtonPressed(wxCommandEvent & event) {
	saveCurrentGameVersion();
}

void GameManagerPanel::onDiscardChangesButtonPressed(wxCommandEvent & event) {
	discardCurrentGameVersionChanges();
}

void GameManagerPanel::onResetButtonPressed(wxCommandEvent & event) {
	resetCurrentGameVersion();
}

void GameManagerPanel::onRemoveButtonPressed(wxCommandEvent & event) {
	removeCurrentGameVersion();
}

void GameManagerPanel::onInstallProgress(GameInstallProgressEvent & event) {
	if(m_installProgressDialog == nullptr) {
		return;
	}

	bool updateResult = m_installProgressDialog->Update(event.getValue(), event.getMessage());
	m_installProgressDialog->Fit();

	if(!updateResult) {
		m_gameInstallationCancelled = true;
	}
}

void GameManagerPanel::onInstallDone(GameInstallDoneEvent & event) {
	if(m_installProgressDialog == nullptr) {
		return;
	}

	m_installProgressDialog->Destroy();
	m_installProgressDialog = nullptr;
}

void GameManagerPanel::onGameVersionChangesDiscarded(GameVersionPanel & gameVersionPanel) {
	updateGameVersionPanel(indexOfGameVersionPanel(&gameVersionPanel));
}

void GameManagerPanel::onGameVersionSettingChanged(GameVersionPanel & gameVersionPanel, SettingPanel & settingPanel) {
	updateGameVersionPanel(indexOfGameVersionPanel(&gameVersionPanel));
}

void GameManagerPanel::onGameVersionNotesChanged(GameVersionPanel & gameVersionPanel) {
	updateGameVersionPanel(indexOfGameVersionPanel(&gameVersionPanel));
}

void GameManagerPanel::onGameVersionReset(GameVersionPanel & gameVersionPanel) {
	updateGameVersionPanel(indexOfGameVersionPanel(&gameVersionPanel));
}

void GameManagerPanel::onGameVersionSaved(GameVersionPanel & gameVersionPanel) {
	updateGameVersionPanel(indexOfGameVersionPanel(&gameVersionPanel));
}
