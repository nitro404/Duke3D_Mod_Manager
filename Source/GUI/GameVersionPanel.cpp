#include "GameVersionPanel.h"

#include "Game/GameVersionCollection.h"
#include "Project.h"

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <wx/gbsizer.h>
#include <wx/wrapsizer.h>

#include "WXUtilities.h"

#include <sstream>

GameVersionPanel::GameVersionPanel(std::shared_ptr<GameVersion> gameVersion, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Game Version")
	, m_gameVersion(gameVersion != nullptr ? gameVersion : std::make_shared<GameVersion>())
	, m_gamePathSettingPanel(nullptr)
	, m_modified(false) {
	const GameVersion * defaultGameVersion = nullptr;

	for(const GameVersion * currentDefaultGameVersion : GameVersion::DEFAULT_GAME_VERSIONS) {
		if(Utilities::areStringsEqualIgnoreCase(m_gameVersion->getName(), currentDefaultGameVersion->getName())) {
			defaultGameVersion = currentDefaultGameVersion;
			break;
		}
	}

	int border = 5;

	wxGridBagSizer * gameVersionConfigurationSizer = new wxGridBagSizer(border, border);

	wxWrapSizer * gameInformationSizer = new wxWrapSizer(wxHORIZONTAL);
	wxStaticBox * gameInformationBox = new wxStaticBox(this, wxID_ANY, "Game Information", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Game Information");
	gameInformationBox->SetOwnFont(gameInformationBox->GetFont().MakeBold());

	wxPanel * gameInformationPanel = new wxPanel(gameInformationBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&GameVersion::getName, m_gameVersion.get()), std::bind(&GameVersion::setName, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getName() : "", "Game Version Name", gameInformationPanel, gameInformationSizer, 1));
	m_settingsPanels.back()->setEditable(m_gameVersion->isRenamable());
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&GameVersion::getWebsite, m_gameVersion.get()), std::bind(&GameVersion::setWebsite, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getWebsite() : "", "Website", gameInformationPanel, gameInformationSizer));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&GameVersion::getSourceCodeURL, m_gameVersion.get()), std::bind(&GameVersion::setSourceCodeURL, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getSourceCodeURL() : "", "Source Code URL", gameInformationPanel, gameInformationSizer));

	wxWrapSizer * gameConfigurationSizer = new wxWrapSizer(wxHORIZONTAL);
	wxStaticBox * gameConfigurationBox = new wxStaticBox(this, wxID_ANY, "Game Configuration", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Game Configuration");
	gameConfigurationBox->SetOwnFont(gameConfigurationBox->GetFont().MakeBold());

	wxPanel * gameConfigurationPanel = new wxPanel(gameConfigurationBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&GameVersion::getGamePath, m_gameVersion.get()), std::bind(&GameVersion::setGamePath, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getGamePath() : "", "Game Path", gameConfigurationPanel, gameConfigurationSizer));
	m_gamePathSettingPanel = m_settingsPanels.back();
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&GameVersion::getGameExecutableName, m_gameVersion.get()), std::bind(&GameVersion::setGameExecutableName, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getGameExecutableName() : "", "Game Executable Name", gameConfigurationPanel, gameConfigurationSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createOptionalStringSettingPanel<void>(std::bind(&GameVersion::getSetupExecutableName, m_gameVersion.get()), std::bind(&GameVersion::setSetupExecutableName, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearSetupExecutableName, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getSetupExecutableName() : std::optional<std::string>(), "Setup Executable Name", gameConfigurationPanel, gameConfigurationSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createOptionalStringSettingPanel<void>(std::bind(&GameVersion::getGroupFileInstallPath, m_gameVersion.get()), std::bind(&GameVersion::setGroupFileInstallPath, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearGroupFileInstallPath, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getGroupFileInstallPath() : std::optional<std::string>(), "Group File Install Path", gameConfigurationPanel, gameConfigurationSizer));
	m_settingsPanels.push_back(SettingPanel::createOptionalBooleanSettingPanel(std::bind(&GameVersion::getRequiresCombinedGroup, m_gameVersion.get()), std::bind(&GameVersion::setRequiresCombinedGroup, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearRequiresCombinedGroup, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getRequiresCombinedGroup() : std::optional<bool>(), "Requires Combined Group", gameConfigurationPanel, gameConfigurationSizer));
	m_settingsPanels.push_back(SettingPanel::createOptionalBooleanSettingPanel(std::bind(&GameVersion::getRequiresDOSBox, m_gameVersion.get()), std::bind(&GameVersion::setRequiresDOSBox, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearRequiresDOSBox, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getRequiresDOSBox() : std::optional<bool>(), "Requires DOSBox", gameConfigurationPanel, gameConfigurationSizer));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&GameVersion::getModDirectoryName, m_gameVersion.get()), std::bind(&GameVersion::setModDirectoryName, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getModDirectoryName() : "", "Mod Subdirectory Name", gameConfigurationPanel, gameConfigurationSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createBooleanSettingPanel(std::bind(&GameVersion::hasLocalWorkingDirectory, m_gameVersion.get()), std::bind(&GameVersion::setLocalWorkingDirectory, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->hasLocalWorkingDirectory() : GameVersion::DEFAULT_LOCAL_WORKING_DIRECTORY, "Local Working Directory", gameConfigurationPanel, gameConfigurationSizer));
	m_settingsPanels.push_back(SettingPanel::createBooleanSettingPanel(std::bind(&GameVersion::hasRelativeConFilePath, m_gameVersion.get()), std::bind(&GameVersion::setRelativeConFilePath, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->hasRelativeConFilePath() : GameVersion::DEFAULT_RELATIVE_CON_FILE_PATH, "Relative CON File Path", gameConfigurationPanel, gameConfigurationSizer));
	m_settingsPanels.push_back(SettingPanel::createBooleanSettingPanel(std::bind(&GameVersion::doesSupportSubdirectories, m_gameVersion.get()), std::bind(&GameVersion::setSupportsSubdirectories, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->doesSupportSubdirectories() : GameVersion::DEFAULT_SUPPORTS_SUBDIRECTORIES, "Supports Subdirectories", gameConfigurationPanel, gameConfigurationSizer));

	wxStaticBox * compatibleGameVersionsBox = new wxStaticBox(this, wxID_ANY, "Compatible Game Versions", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Compatible Game Versions");
	compatibleGameVersionsBox->SetOwnFont(compatibleGameVersionsBox->GetFont().MakeBold());

	std::vector<std::string> compatibleGameVersionNames(GameVersionCollection::getGameVersionDisplayNamesFrom(GameVersion::DEFAULT_GAME_VERSIONS, false));

	std::vector<std::string>::const_iterator currentGameVersionNameIterator(std::find_if(compatibleGameVersionNames.cbegin(), compatibleGameVersionNames.cend(), [this](const std::string & currentGameVersionName) {
		return Utilities::areStringsEqualIgnoreCase(m_gameVersion->getName(), currentGameVersionName);
	}));

	if(currentGameVersionNameIterator != compatibleGameVersionNames.cend()) {
		compatibleGameVersionNames.erase(currentGameVersionNameIterator);
	}

	SettingPanel * compatibleGameVersionsPanel = SettingPanel::createStringMultiChoiceSettingPanel(std::bind(&GameVersion::getCompatibleGameVersions, m_gameVersion.get()), std::bind(&GameVersion::hasCompatibleGameVersionWithName, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::addCompatibleGameVersionWithName, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::removeCompatibleGameVersionWithName, m_gameVersion.get(), std::placeholders::_1), "Compatible Game Versions", false, compatibleGameVersionNames, compatibleGameVersionsBox);
	m_settingsPanels.push_back(compatibleGameVersionsPanel);

	wxStaticBox * supportedOperatingSystemsBox = new wxStaticBox(this, wxID_ANY, "Supported Operating Systems", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Supported Operating Systems");
	supportedOperatingSystemsBox->SetOwnFont(supportedOperatingSystemsBox->GetFont().MakeBold());

	SettingPanel * supportedOperatingSystemsPanel = SettingPanel::createEnumMultiChoiceSettingPanel<GameVersion::OperatingSystem>(std::bind(&GameVersion::getSupportedOperatingSystems, m_gameVersion.get()), std::bind(&GameVersion::hasSupportedOperatingSystem, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::addSupportedOperatingSystem, m_gameVersion.get(), std::placeholders::_1), std::bind(static_cast<bool(GameVersion::*)(GameVersion::OperatingSystem)>(&GameVersion::removeSupportedOperatingSystem), m_gameVersion.get(), std::placeholders::_1), "Supported Operating Systems", supportedOperatingSystemsBox, 1);
	m_settingsPanels.push_back(supportedOperatingSystemsPanel);

	wxWrapSizer * argumentsSizer = new wxWrapSizer(wxHORIZONTAL);
	wxStaticBox * argumentsBox = new wxStaticBox(this, wxID_ANY, "Executable Argument Flags", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Argument Flags");
	argumentsBox->SetOwnFont(argumentsBox->GetFont().MakeBold());

	wxPanel * argumentsPanel = new wxPanel(argumentsBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<bool>(std::bind(&GameVersion::getConFileArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setConFileArgumentFlag, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getConFileArgumentFlag() : GameVersion::DEFAULT_CON_FILE_ARGUMENT_FLAG, "Con File", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<bool>(std::bind(&GameVersion::getGroupFileArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setGroupFileArgumentFlag, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getGroupFileArgumentFlag() : GameVersion::DEFAULT_GROUP_FILE_ARGUMENT_FLAG, "Group File", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createOptionalStringSettingPanel<bool>(std::bind(&GameVersion::getDefFileArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setDefFileArgumentFlag, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearDefFileArgumentFlag, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getDefFileArgumentFlag() : std::optional<std::string>(), "Def File", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<bool>(std::bind(&GameVersion::getMapFileArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setMapFileArgumentFlag, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getMapFileArgumentFlag() : GameVersion::DEFAULT_MAP_FILE_ARGUMENT_FLAG, "Map File", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<bool>(std::bind(&GameVersion::getEpisodeArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setEpisodeArgumentFlag, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getEpisodeArgumentFlag() : GameVersion::DEFAULT_EPISODE_ARGUMENT_FLAG, "Episode", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<bool>(std::bind(&GameVersion::getLevelArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setLevelArgumentFlag, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getLevelArgumentFlag() : GameVersion::DEFAULT_LEVEL_ARGUMENT_FLAG, "Level", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<bool>(std::bind(&GameVersion::getSkillArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setSkillArgumentFlag, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getSkillArgumentFlag() : GameVersion::DEFAULT_SKILL_ARGUMENT_FLAG, "Skill", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<bool>(std::bind(&GameVersion::getRecordDemoArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setRecordDemoArgumentFlag, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getRecordDemoArgumentFlag() : GameVersion::DEFAULT_RECORD_DEMO_ARGUMENT_FLAG, "Record Demo", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createOptionalStringSettingPanel<bool>(std::bind(&GameVersion::getPlayDemoArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setPlayDemoArgumentFlag, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearPlayDemoArgumentFlag, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getPlayDemoArgumentFlag() : GameVersion::DEFAULT_PLAY_DEMO_ARGUMENT_FLAG, "Play Demo", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<bool>(std::bind(&GameVersion::getRespawnModeArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setRespawnModeArgumentFlag, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getRespawnModeArgumentFlag() : GameVersion::DEFAULT_RESPAWN_MODE_ARGUMENT_FLAG, "Respawn Mode", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<bool>(std::bind(&GameVersion::getWeaponSwitchOrderArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setWeaponSwitchOrderArgumentFlag, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getWeaponSwitchOrderArgumentFlag() : GameVersion::DEFAULT_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG, "Weapon Switch Order", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<bool>(std::bind(&GameVersion::getDisableMonstersArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setDisableMonstersArgumentFlag, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getDisableMonstersArgumentFlag() : GameVersion::DEFAULT_DISABLE_MONSTERS_ARGUMENT_FLAG, "Disable Monsters", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<bool>(std::bind(&GameVersion::getDisableSoundArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setDisableSoundArgumentFlag, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getDisableSoundArgumentFlag() : GameVersion::DEFAULT_DISABLE_SOUND_ARGUMENT_FLAG, "Disable Sound", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<bool>(std::bind(&GameVersion::getDisableMusicArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setDisableMusicArgumentFlag, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getDisableMusicArgumentFlag() : GameVersion::DEFAULT_DISABLE_MUSIC_ARGUMENT_FLAG, "Disable Music", argumentsPanel, argumentsSizer, 1));

	gameInformationPanel->SetSizerAndFit(gameInformationSizer);
	gameConfigurationPanel->SetSizerAndFit(gameConfigurationSizer);
	argumentsPanel->SetSizerAndFit(argumentsSizer);

	wxBoxSizer * gameInformationBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	gameInformationBoxSizer->Add(gameInformationPanel, 1, wxEXPAND | wxALL, 20);
	gameInformationBox->SetSizer(gameInformationBoxSizer);

	wxBoxSizer * gameConfigurationBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	gameConfigurationBoxSizer->Add(gameConfigurationPanel, 1, wxEXPAND | wxALL, 20);
	gameConfigurationBox->SetSizer(gameConfigurationBoxSizer);

	wxBoxSizer * compatibleGameVersionsBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	compatibleGameVersionsBoxSizer->Add(compatibleGameVersionsPanel, 1, wxEXPAND | wxALL, 20);
	compatibleGameVersionsBox->SetSizer(compatibleGameVersionsBoxSizer);

	wxBoxSizer * supportedOperatingSystemsBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	supportedOperatingSystemsBoxSizer->Add(supportedOperatingSystemsPanel, 1, wxEXPAND | wxALL, 20);
	supportedOperatingSystemsBox->SetSizer(supportedOperatingSystemsBoxSizer);

	wxBoxSizer * argumentsBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	argumentsBoxSizer->Add(argumentsPanel, 1, wxEXPAND | wxALL, 20);
	argumentsBox->SetSizer(argumentsBoxSizer);

	gameVersionConfigurationSizer->Add(gameInformationBox, wxGBPosition(0, 0), wxGBSpan(1, 2), wxEXPAND | wxALL, border);
	gameVersionConfigurationSizer->Add(gameConfigurationBox, wxGBPosition(1, 0), wxGBSpan(1, 2), wxEXPAND | wxALL, border);
	gameVersionConfigurationSizer->Add(compatibleGameVersionsBox, wxGBPosition(2, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	gameVersionConfigurationSizer->Add(supportedOperatingSystemsBox, wxGBPosition(2, 1), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	gameVersionConfigurationSizer->Add(argumentsBox, wxGBPosition(3, 0), wxGBSpan(1, 2), wxEXPAND | wxALL, border);
	gameVersionConfigurationSizer->AddGrowableRow(0, 1);
	gameVersionConfigurationSizer->AddGrowableRow(1, 6);
	gameVersionConfigurationSizer->AddGrowableRow(2, 1);
	gameVersionConfigurationSizer->AddGrowableRow(3, 12);
	gameVersionConfigurationSizer->AddGrowableCol(0, 6);
	gameVersionConfigurationSizer->AddGrowableCol(1, 1);
	SetSizer(gameVersionConfigurationSizer);

	m_gameVersion->addListener(*this);

	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->addListener(*this);
	}
}

GameVersionPanel::~GameVersionPanel() {
	m_gameVersion->removeListener(*this);

	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->removeListener(*this);
	}
}

bool GameVersionPanel::isModified() const {
	return m_modified;
}

std::string GameVersionPanel::getPanelName() const {
	return m_gameVersion->getName().empty() ? "NEW GAME *" : fmt::format("{}{}", m_gameVersion->getName(), m_modified ? " *" : "");
}

std::shared_ptr<GameVersion> GameVersionPanel::getGameVersion() const {
	return m_gameVersion;
}

bool GameVersionPanel::isValid() const {
	for(const SettingPanel * settingPanel : m_settingsPanels) {
		if(!settingPanel->isValid()) {
			return false;
		}
	}

	return true;
}

bool GameVersionPanel::save() {
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
				"Failed to validate {0} game properties!\n"
				"\n"
				"The following game {1} {2} invalid value{3}: {4}",
				APPLICATION_NAME,
				numberOfInvalidSettings == 1 ? "property" : "properties",
				numberOfInvalidSettings == 1 ? "has an" : "have",
				numberOfInvalidSettings == 1 ? "" : "s",
				invalidSettingPanelNames.str()
			),
			"Invalid Game Properties",
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

	notifyGameVersionSaved();

	return true;
}

void GameVersionPanel::discard() {
	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->discard();
	}

	m_modified = false;

	notifyGameVersionChangesDiscarded();
}

void GameVersionPanel::reset() {
	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->reset();
	}

	m_modified = true;

	notifyGameVersionReset();
}

void GameVersionPanel::discardGamePathChanges() {
	m_gamePathSettingPanel->discard();
}

void GameVersionPanel::update() {
	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->update();
	}
}

GameVersionPanel::Listener::~Listener() { }

size_t GameVersionPanel::numberOfListeners() const {
	return m_listeners.size();
}

bool GameVersionPanel::hasListener(const Listener & listener) const {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			return true;
		}
	}

	return false;
}

size_t GameVersionPanel::indexOfListener(const Listener & listener) const {
	for(size_t i = 0; i < m_listeners.size(); i++) {
		if(m_listeners[i] == &listener) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

GameVersionPanel::Listener * GameVersionPanel::getListener(size_t index) const {
	if(index >= m_listeners.size()) {
		return nullptr;
	}

	return m_listeners[index];
}

bool GameVersionPanel::addListener(Listener & listener) {
	if(!hasListener(listener)) {
		m_listeners.push_back(&listener);

		return true;
	}

	return false;
}

bool GameVersionPanel::removeListener(size_t index) {
	if(index >= m_listeners.size()) {
		return false;
	}

	m_listeners.erase(m_listeners.cbegin() + index);

	return true;
}

bool GameVersionPanel::removeListener(const Listener & listener) {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			m_listeners.erase(i);

			return true;
		}
	}

	return false;
}

void GameVersionPanel::clearListeners() {
	m_listeners.clear();
}

void GameVersionPanel::notifyGameVersionSettingChanged(SettingPanel & settingPanel) {
	for(Listener * listener : m_listeners) {
		listener->gameVersionSettingChanged(*this, settingPanel);
	}
}

void GameVersionPanel::notifyGameVersionChangesDiscarded() {
	for(Listener * listener : m_listeners) {
		listener->gameVersionChangesDiscarded(*this);
	}
}

void GameVersionPanel::notifyGameVersionReset() {
	m_modified = true;

	for(Listener * listener : m_listeners) {
		listener->gameVersionReset(*this);
	}
}

void GameVersionPanel::notifyGameVersionSaved() {
	m_modified = false;

	for(Listener * listener : m_listeners) {
		listener->gameVersionSaved(*this);
	}
}

void GameVersionPanel::gameVersionModified(GameVersion & gameVersion) {
	update();
}

void GameVersionPanel::settingModified(SettingPanel & settingPanel) {
	if(settingPanel.isModified()) {
		m_modified = true;

		notifyGameVersionSettingChanged(settingPanel);
	}
}
