#include "SettingsManagerPanel.h"

#include "Manager/ModManager.h"
#include "Manager/SettingsManager.h"
#include "Game/GameVersionCollection.h"
#include "Project.h"
#include "SettingPanel.h"
#include "WXUtilities.h"

#include <wx/gbsizer.h>
#include <wx/wrapsizer.h>

#include <sstream>

SettingsManagerPanel::SettingsManagerPanel(std::shared_ptr<ModManager> modManager, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Settings")
	, m_modManager(modManager)
	, m_modified(false)
	, m_discardChangesButton(nullptr)
	, m_saveSettingsButton(nullptr) {
	SettingsManager * settings = SettingsManager::getInstance();

	m_modManager->getGameVersions()->addListener(*this);

	int wrapSizerOrientation = wxHORIZONTAL;

	wxWrapSizer * generalSettingsSizer = new wxWrapSizer(wrapSizerOrientation);
	wxStaticBox * generalSettingsBox = new wxStaticBox(this, wxID_ANY, "General", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "General");
	generalSettingsBox->SetOwnFont(generalSettingsBox->GetFont().MakeBold());

	wxPanel * generalSettingsPanel = new wxPanel(generalSettingsBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->favouriteModsListFilePath, SettingsManager::DEFAULT_FAVOURITE_MODS_LIST_FILE_PATH, "Favourite Mods List File Path", generalSettingsPanel, generalSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->gameVersionsListFilePath, SettingsManager::DEFAULT_GAME_VERSIONS_LIST_FILE_PATH, "Game Versions List File Path", generalSettingsPanel, generalSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->dataDirectoryPath, SettingsManager::DEFAULT_DATA_DIRECTORY_PATH, "Data Directory Path", generalSettingsPanel, generalSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->timeZoneDataDirectoryName, SettingsManager::DEFAULT_TIME_ZONE_DATA_DIRECTORY_NAME, "Time Zone Data Directory Name", generalSettingsPanel, generalSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->curlDataDirectoryName, SettingsManager::DEFAULT_CURL_DATA_DIRECTORY_NAME, "cURL Data Directory Name", generalSettingsPanel, generalSettingsSizer));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->gameSymlinkName, SettingsManager::DEFAULT_GAME_SYMLINK_NAME, "Game Symbolic Link Name", generalSettingsPanel, generalSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->modsDirectoryPath, SettingsManager::DEFAULT_MODS_DIRECTORY_PATH, "Mods Directory Path", generalSettingsPanel, generalSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->modsSymlinkName, SettingsManager::DEFAULT_MODS_SYMLINK_NAME, "Mods Symbolic Link Name", generalSettingsPanel, generalSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->mapsDirectoryPath, SettingsManager::DEFAULT_MAPS_DIRECTORY_PATH, "Maps Directory Path", generalSettingsPanel, generalSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->mapsSymlinkName, SettingsManager::DEFAULT_MAPS_SYMLINK_NAME, "Maps Symbolic Link Name", generalSettingsPanel, generalSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->appTempDirectoryPath, SettingsManager::DEFAULT_APP_TEMP_DIRECTORY_PATH, "App Temp Directory Path", generalSettingsPanel, generalSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->gameTempDirectoryName, SettingsManager::DEFAULT_GAME_TEMP_DIRECTORY_NAME, "Game Temp Directory Name", generalSettingsPanel, generalSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->tempSymlinkName, SettingsManager::DEFAULT_TEMP_SYMLINK_NAME, "Temp Symbolic Link Name", generalSettingsPanel, generalSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->cacheDirectoryPath, SettingsManager::DEFAULT_CACHE_DIRECTORY_PATH, "Cache Directory Path", generalSettingsPanel, generalSettingsSizer, 1));
	m_preferredGameVersionSettingPanel = SettingPanel::createStringChoiceSettingPanel(settings->preferredGameVersion, SettingsManager::DEFAULT_PREFERRED_GAME_VERSION, "Preferred Game Version", modManager->getGameVersions()->getGameVersionDisplayNames(false), generalSettingsPanel, generalSettingsSizer);
	m_settingsPanels.push_back(m_preferredGameVersionSettingPanel);
	m_settingsPanels.push_back(SettingPanel::createEnumSettingPanel<GameType>(settings->gameType, SettingsManager::DEFAULT_GAME_TYPE, "Game Type", generalSettingsPanel, generalSettingsSizer));

	wxWrapSizer * downloadsSettingsSizer = new wxWrapSizer(wrapSizerOrientation);
	wxStaticBox * downloadsSettingsBox = new wxStaticBox(this, wxID_ANY, "Downloads", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Downloads");
	downloadsSettingsBox->SetOwnFont(downloadsSettingsBox->GetFont().MakeBold());

	wxPanel * downloadsSettingsPanel = new wxPanel(downloadsSettingsBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->downloadsDirectoryPath, SettingsManager::DEFAULT_DOWNLOADS_DIRECTORY_PATH, "Downloads Directory Path", downloadsSettingsPanel, downloadsSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->downloadCacheFileName, SettingsManager::DEFAULT_DOWNLOAD_CACHE_FILE_NAME, "Download Cache File Name", downloadsSettingsPanel, downloadsSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->modDownloadsDirectoryName, SettingsManager::DEFAULT_MOD_DOWNLOADS_DIRECTORY_NAME, "Mod Downloads Directory Name", downloadsSettingsPanel, downloadsSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->mapDownloadsDirectoryName, SettingsManager::DEFAULT_MAP_DOWNLOADS_DIRECTORY_NAME, "Map Downloads Directory Name", downloadsSettingsPanel, downloadsSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->gameDownloadsDirectoryName, SettingsManager::DEFAULT_GAME_DOWNLOADS_DIRECTORY_NAME, "Game Downloads Directory Name", downloadsSettingsPanel, downloadsSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createChronoSettingPanel(settings->connectionTimeout, SettingsManager::DEFAULT_CONNECTION_TIMEOUT, "Connection Timeout", downloadsSettingsPanel, downloadsSettingsSizer));
	m_settingsPanels.push_back(SettingPanel::createChronoSettingPanel(settings->networkTimeout, SettingsManager::DEFAULT_NETWORK_TIMEOUT, "Network Timeout", downloadsSettingsPanel, downloadsSettingsSizer));
	m_settingsPanels.push_back(SettingPanel::createChronoSettingPanel(settings->transferTimeout, SettingsManager::DEFAULT_TRANSFER_TIMEOUT, "Transfer Timeout", downloadsSettingsPanel, downloadsSettingsSizer));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->apiBaseURL, SettingsManager::DEFAULT_API_BASE_URL, "API Base URL", downloadsSettingsPanel, downloadsSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createBooleanSettingPanel(settings->downloadThrottlingEnabled, SettingsManager::DEFAULT_DOWNLOAD_THROTTLING_ENABLED, "Download Throttling", downloadsSettingsPanel, downloadsSettingsSizer));
	m_settingsPanels.push_back(SettingPanel::createChronoSettingPanel(settings->modListUpdateFrequency, SettingsManager::DEFAULT_MOD_LIST_UPDATE_FREQUENCY, "Mod List Update Frequency", downloadsSettingsPanel, downloadsSettingsSizer));
	m_settingsPanels.push_back(SettingPanel::createChronoSettingPanel(settings->gameDownloadListUpdateFrequency, SettingsManager::DEFAULT_GAME_DOWNLOAD_LIST_UPDATE_FREQUENCY, "Game Download List Update Frequency", downloadsSettingsPanel, downloadsSettingsSizer));
	m_settingsPanels.push_back(SettingPanel::createChronoSettingPanel(settings->cacertUpdateFrequency, SettingsManager::DEFAULT_TIME_ZONE_DATA_UPDATE_FREQUENCY, "CACert Update Frequency", downloadsSettingsPanel, downloadsSettingsSizer));

	wxWrapSizer * dosboxSettingsSizer = new wxWrapSizer(wrapSizerOrientation);
	wxStaticBox * dosboxSettingsBox = new wxStaticBox(this, wxID_ANY, "DOSBox", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "DOSBox");
	dosboxSettingsBox->SetOwnFont(dosboxSettingsBox->GetFont().MakeBold());

	wxPanel * dosboxSettingsPanel = new wxPanel(dosboxSettingsBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->dosboxExecutableFileName, SettingsManager::DEFAULT_DOSBOX_EXECUTABLE_FILE_NAME, "Application Executable Name", dosboxSettingsPanel, dosboxSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->dosboxDirectoryPath, SettingsManager::DEFAULT_DOSBOX_DIRECTORY_PATH, "Application Directory Path", dosboxSettingsPanel, dosboxSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->dosboxArguments, SettingsManager::DEFAULT_DOSBOX_ARGUMENTS, "Application Arguments", dosboxSettingsPanel, dosboxSettingsSizer));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->dosboxDataDirectoryName, SettingsManager::DEFAULT_DOSBOX_DATA_DIRECTORY_NAME, "Data Directory Name", dosboxSettingsPanel, dosboxSettingsSizer));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->dosboxServerIPAddress, SettingsManager::DEFAULT_DOSBOX_SERVER_IP_ADDRESS, "Server IP Address", dosboxSettingsPanel, dosboxSettingsSizer));
	m_settingsPanels.push_back(SettingPanel::createIntegerSettingPanel<uint16_t>(settings->dosboxRemoteServerPort, SettingsManager::DEFAULT_DOSBOX_REMOTE_SERVER_PORT, "Remote Server Port", dosboxSettingsPanel, dosboxSettingsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createIntegerSettingPanel<uint16_t>(settings->dosboxLocalServerPort, SettingsManager::DEFAULT_DOSBOX_LOCAL_SERVER_PORT, "Local Server Port", dosboxSettingsPanel, dosboxSettingsSizer, 1));

	wxWrapSizer * analyticsSettingsSizer = new wxWrapSizer(wrapSizerOrientation);
	wxStaticBox * analyticsSettingsBox = new wxStaticBox(this, wxID_ANY, "Analytics", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Analytics");
	analyticsSettingsBox->SetOwnFont(analyticsSettingsBox->GetFont().MakeBold());

	wxPanel * analyticsSettingsPanel = new wxPanel(analyticsSettingsBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	m_settingsPanels.push_back(SettingPanel::createBooleanSettingPanel(settings->segmentAnalyticsEnabled, SettingsManager::DEFAULT_SEGMENT_ANALYTICS_ENABLED, "Segment Analytics Enabled", analyticsSettingsPanel, analyticsSettingsSizer));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel(settings->segmentAnalyticsDataFileName, SettingsManager::DEFAULT_SEGMENT_ANALYTICS_DATA_FILE_NAME, "Analytics Data File Name", analyticsSettingsPanel, analyticsSettingsSizer));

	wxPanel * actionsPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	m_saveSettingsButton = new wxButton(actionsPanel, wxID_ANY, "Save Settings", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Save Settings");
	m_saveSettingsButton->Bind(wxEVT_BUTTON, &SettingsManagerPanel::onSaveSettingsButtonPressed, this);
	m_saveSettingsButton->Disable();

	m_discardChangesButton = new wxButton(actionsPanel, wxID_ANY, "Discard Changes", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Discard Changes");
	m_discardChangesButton->Bind(wxEVT_BUTTON, &SettingsManagerPanel::onDiscardChangesButtonPressed, this);
	m_discardChangesButton->Disable();

	wxButton * resetDefaultsButton = new wxButton(actionsPanel, wxID_ANY, "Reset Defaults", wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, "Reset Defaults");
	resetDefaultsButton->Bind(wxEVT_BUTTON, &SettingsManagerPanel::onResetDefaultsButtonPressed, this);

	int border = 5;

	generalSettingsPanel->SetSizerAndFit(generalSettingsSizer);
	downloadsSettingsPanel->SetSizerAndFit(downloadsSettingsSizer);
	dosboxSettingsPanel->SetSizerAndFit(dosboxSettingsSizer);
	analyticsSettingsPanel->SetSizerAndFit(analyticsSettingsSizer);

	wxBoxSizer * generalSettingsBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	generalSettingsBoxSizer->Add(generalSettingsPanel, 1, wxEXPAND | wxALL, 20);
	generalSettingsBox->SetSizer(generalSettingsBoxSizer);

	wxBoxSizer * downloadsSettingsBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	downloadsSettingsBoxSizer->Add(downloadsSettingsPanel, 1, wxEXPAND | wxALL, 20);
	downloadsSettingsBox->SetSizer(downloadsSettingsBoxSizer);

	wxBoxSizer * dosboxSettingsBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	dosboxSettingsBoxSizer->Add(dosboxSettingsPanel, 1, wxEXPAND | wxALL, 20);
	dosboxSettingsBox->SetSizer(dosboxSettingsBoxSizer);

	wxBoxSizer * analyticsSettingsBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	analyticsSettingsBoxSizer->Add(analyticsSettingsPanel, 1, wxEXPAND | wxALL, 20);
	analyticsSettingsBox->SetSizer(analyticsSettingsBoxSizer);

	wxWrapSizer * actionsSizer = new wxWrapSizer(wxHORIZONTAL);
	actionsSizer->Add(m_saveSettingsButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsSizer->Add(m_discardChangesButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsSizer->Add(resetDefaultsButton, 1, wxEXPAND | wxHORIZONTAL, border);
	actionsPanel->SetSizer(actionsSizer);

	wxGridBagSizer * settingsPanelSizer = new wxGridBagSizer(border, border);
	settingsPanelSizer->Add(generalSettingsBox, wxGBPosition(0, 0), wxGBSpan(1, 2), wxEXPAND | wxALL, border);
	settingsPanelSizer->Add(downloadsSettingsBox, wxGBPosition(1, 0), wxGBSpan(1, 2), wxEXPAND | wxALL, border);
	settingsPanelSizer->Add(dosboxSettingsBox, wxGBPosition(2, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	settingsPanelSizer->Add(analyticsSettingsBox, wxGBPosition(2, 1), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	settingsPanelSizer->Add(actionsPanel, wxGBPosition(3, 0), wxGBSpan(1, 2), wxEXPAND | wxHORIZONTAL, border);
	settingsPanelSizer->AddGrowableRow(0, 2);
	settingsPanelSizer->AddGrowableRow(1, 2);
	settingsPanelSizer->AddGrowableRow(2, 1);
	settingsPanelSizer->AddGrowableCol(0, 3);
	settingsPanelSizer->AddGrowableCol(1, 1);
	SetSizerAndFit(settingsPanelSizer);

	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->addListener(*this);
	}
}

SettingsManagerPanel::~SettingsManagerPanel() {
	m_modManager->getGameVersions()->removeListener(*this);

	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->removeListener(*this);
	}
}

bool SettingsManagerPanel::isModified() const {
	return m_modified;
}

bool SettingsManagerPanel::isValid() const {
	for(const SettingPanel * settingPanel : m_settingsPanels) {
		if(!settingPanel->isValid()) {
			return false;
		}
	}

	return true;
}

void SettingsManagerPanel::reset() {
	SettingsManager * settings = SettingsManager::getInstance();

	settings->reset();

	m_modified = false;

	updateButtons();
	notifySettingsReset();
}

void SettingsManagerPanel::discard() {
	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->discard();
	}

	m_modified = false;

	updateButtons();
	notifySettingsDiscarded();
}

bool SettingsManagerPanel::save() {
	size_t numberOfInvalidSettings = 0;
	std::stringstream invalidSettingPanelNames;

	for(const SettingPanel * settingPanel : m_settingsPanels) {
		if(!settingPanel->isValid()) {
			numberOfInvalidSettings++;

			if(invalidSettingPanelNames.tellp() != 0) {
				invalidSettingPanelNames << ", ";
			}

			invalidSettingPanelNames << "'" << settingPanel->getName() << "'";
		}
	}

	if(numberOfInvalidSettings != 0) {
		wxMessageBox(
			fmt::format(
				"Failed to validate {0} settings values!\n"
				"\n"
				"The following setting{1} {2} invalid value{1}: {3}",
				APPLICATION_NAME,
				numberOfInvalidSettings == 1 ? "" : "s",
				numberOfInvalidSettings == 1 ? "has an" : "have",
				invalidSettingPanelNames.str()
			),
			"Invalid Settings",
			wxOK | wxICON_WARNING,
			this
		);

		return false;
	}

	for(SettingPanel * settingPanel : m_settingsPanels) {
		if(!settingPanel->save()) {
			return false;
		}
	}

	SettingsManager * settings = SettingsManager::getInstance();

	if(!settings->save()) {
		wxMessageBox("Failed to save settings!", "Save Failed", wxOK | wxICON_ERROR, this);
		return false;
	}

	m_modified = false;

	updateButtons();
	notifySettingsSaved();

	return true;
}

void SettingsManagerPanel::updateButtons() {
	WXUtilities::setButtonEnabled(m_discardChangesButton, m_modified);
	WXUtilities::setButtonEnabled(m_saveSettingsButton, m_modified);
}

void SettingsManagerPanel::onResetDefaultsButtonPressed(wxCommandEvent & event) {
	int resetDefaultsResult = wxMessageBox("Are you sure you want to reset to default settings? Application will automatically re-load after settings reset.", "Reset Settings", wxYES_NO | wxCANCEL | wxICON_WARNING, this);

	if(resetDefaultsResult == wxYES) {
		reset();
	}
}

void SettingsManagerPanel::onDiscardChangesButtonPressed(wxCommandEvent & event) {
	discard();
}

void SettingsManagerPanel::onSaveSettingsButtonPressed(wxCommandEvent & event) {
	save();
}

SettingsManagerPanel::Listener::~Listener() { }

size_t SettingsManagerPanel::numberOfListeners() const {
	return m_listeners.size();
}

bool SettingsManagerPanel::hasListener(const Listener & listener) const {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			return true;
		}
	}

	return false;
}

size_t SettingsManagerPanel::indexOfListener(const Listener & listener) const {
	for(size_t i = 0; i < m_listeners.size(); i++) {
		if(m_listeners[i] == &listener) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

SettingsManagerPanel::Listener * SettingsManagerPanel::getListener(size_t index) const {
	if(index >= m_listeners.size()) {
		return nullptr;
	}

	return m_listeners[index];
}

bool SettingsManagerPanel::addListener(Listener & listener) {
	if(!hasListener(listener)) {
		m_listeners.push_back(&listener);

		return true;
	}

	return false;
}

bool SettingsManagerPanel::removeListener(size_t index) {
	if(index >= m_listeners.size()) {
		return false;
	}

	m_listeners.erase(m_listeners.cbegin() + index);

	return true;
}

bool SettingsManagerPanel::removeListener(const Listener & listener) {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			m_listeners.erase(i);

			return true;
		}
	}

	return false;
}

void SettingsManagerPanel::clearListeners() {
	m_listeners.clear();
}

void SettingsManagerPanel::notifySettingsChanged() {
	for(Listener * listener : m_listeners) {
		listener->settingsChanged();
	}
}

void SettingsManagerPanel::notifySettingsReset() {
	for(Listener * listener : m_listeners) {
		listener->settingsReset();
	}
}

void SettingsManagerPanel::notifySettingsDiscarded() {
	for(Listener * listener : m_listeners) {
		listener->settingsDiscarded();
	}
}

void SettingsManagerPanel::notifySettingsSaved() {
	for(Listener * listener : m_listeners) {
		listener->settingsSaved();
	}
}

void SettingsManagerPanel::settingModified(SettingPanel & settingPanel) {
	if(settingPanel.isModified()) {
		m_modified = true;

		updateButtons();
		notifySettingsChanged();
	}
}

void SettingsManagerPanel::gameVersionCollectionSizeChanged(GameVersionCollection & gameVersionCollection) {
	m_preferredGameVersionSettingPanel->setChoices(gameVersionCollection.getGameVersionDisplayNames(false));
}

void SettingsManagerPanel::gameVersionCollectionItemModified(GameVersionCollection & gameVersionCollection, GameVersion & gameVersion) {
	m_preferredGameVersionSettingPanel->setChoices(gameVersionCollection.getGameVersionDisplayNames(false));
}
