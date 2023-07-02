#include "GameManagerPanel.h"

#include "Game/GameManager.h"
#include "Game/GameVersionCollection.h"
#include "GameVersionPanel.h"
#include "Manager/SettingsManager.h"
#include "SettingPanel.h"
#include "WXUtilities.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/gbsizer.h>
#include <wx/progdlg.h>
#include <wx/textdlg.h>
#include <wx/wrapsizer.h>

#include <filesystem>
#include <sstream>

GameManagerPanel::GameManagerPanel(std::shared_ptr<GameManager> gameManager, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Game Manager")
	, m_gameManager(gameManager)
	, m_notebook(nullptr)
	, m_newButton(nullptr)
	, m_installButton(nullptr)
	, m_uninstallButton(nullptr)
	, m_saveButton(nullptr)
	, m_discardChangesButton(nullptr)
	, m_resetButton(nullptr)
	, m_removeButton(nullptr) {
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
	gameManagerPanelSizer->AddGrowableRow(0, 0);
	gameManagerPanelSizer->AddGrowableCol(0, 0);
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

	return true;
}

bool GameManagerPanel::selectPanelWithGameVersion(const GameVersion & gameVersion) {
	size_t gameVersionPanelIndex = indexOfPanelWithGameVersion(&gameVersion);

	if(gameVersionPanelIndex == std::numeric_limits<size_t>::max()) {
		return false;
	}

	m_notebook->ChangeSelection(gameVersionPanelIndex);

	return true;
}

bool GameManagerPanel::selectPanelWithGameVersionID(const std::string & gameVersionID) {
	size_t gameVersionPanelIndex = indexOfPanelWithGameVersionID(gameVersionID);

	if(gameVersionPanelIndex == std::numeric_limits<size_t>::max()) {
		return false;
	}

	m_notebook->ChangeSelection(gameVersionPanelIndex);

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
	std::shared_ptr<GameVersion> gameVersion = gameVersionPanel != nullptr ? gameVersionPanel->getGameVersion() : nullptr;

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
		gameVersionTemplate = std::shared_ptr<GameVersion>(GameVersion::DEFAULT_GAME_VERSIONS[selectedGameVersionTemplateIndex - 1]->createTemplateFrom().release());
	}

	return addGameVersionPanel(new GameVersionPanel(gameVersionTemplate, m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL));
}

bool GameManagerPanel::installGameVersion(size_t index) {
	GameVersionPanel * gameVersionPanel = getGameVersionPanel(index);

	if(gameVersionPanel == nullptr) {
		return false;
	}

	std::shared_ptr<GameVersion> gameVersion = gameVersionPanel->getGameVersion();

	if(gameVersion->hasGamePath() || !gameVersion->hasID() || !GameManager::isGameDownloadable(gameVersion->getID()) || !m_gameManager->isInitialized()) {
		return false;
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

	wxProgressDialog * installingProgressDialog = new wxProgressDialog("Installing", fmt::format("Installing '{}', please wait...", gameVersion->getLongName()), 1, this, wxPD_AUTO_HIDE);
	installingProgressDialog->SetIcon(wxICON(D3DMODMGR_ICON));

	if(!m_gameManager->installGame(*gameVersion, destinationDirectoryPath)) {
		installingProgressDialog->Destroy();

		wxMessageBox(fmt::format("Failed to install '{}' to '{}'!\n\nCheck console for details.", gameVersion->getLongName(), destinationDirectoryPath), "Installation Failed", wxOK | wxICON_ERROR, this);

		return false;
	}

	gameVersionPanel->discardGamePathChanges();
	gameVersion->setGamePath(destinationDirectoryPath);

	saveGameVersions();

	updateButtons();

	installingProgressDialog->Update(1);
	installingProgressDialog->Destroy();

	wxMessageBox(fmt::format("'{}' was successfully installed to: '{}'!", gameVersion->getLongName(), destinationDirectoryPath), "Game Installed", wxOK | wxICON_INFORMATION, this);

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

	std::shared_ptr<GameVersion> gameVersion = gameVersionPanel->getGameVersion();

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

	gameVersionPanel->discardGamePathChanges();
	gameVersion->setGamePath("");

	saveGameVersions();

	updateButtons();

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

	std::shared_ptr<GameVersion> gameVersion = gameVersionPanel->getGameVersion();

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

	return saveGameVersions();
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

	std::shared_ptr<GameVersion> gameVersion = gameVersionPanel->getGameVersion();

	int resetResult = wxMessageBox(fmt::format("Are you sure you want to reset the '{}' game version to its default configuration?", gameVersion->hasID() ? gameVersion->getLongName() : "NEW"), "Reset Game", wxICON_QUESTION | wxYES_NO | wxCANCEL, this);

	if(resetResult != wxYES) {
		return false;
	}

	gameVersionPanel->reset();

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
