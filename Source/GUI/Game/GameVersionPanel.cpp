#include "GameVersionPanel.h"

#include "../SettingPanel.h"
#include "../DOSBox/Configuration/DOSBoxConfigurationPanel.h"
#include "Game/GameVersion.h"
#include "Game/GameVersionCollection.h"
#include "GameVersionPanel.h"
#include "Project.h"

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <wx/gbsizer.h>
#include <wx/wrapsizer.h>

#include "../WXUtilities.h"

#include <sstream>

GameVersionPanel::GameVersionPanel(std::shared_ptr<GameVersion> gameVersion, wxWindow * parent, wxWindowID windowID, const wxPoint & position, const wxSize & size, long style)
	: wxPanel(parent, windowID, position, size, style, "Game Version")
	, m_gameVersion(gameVersion != nullptr ? gameVersion : std::make_shared<GameVersion>())
	, m_gameVersionIDSettingPanel(nullptr)
	, m_gamePathSettingPanel(nullptr)
	, m_dosboxConfigurationBox(nullptr)
	, m_dosboxConfigurationPanel(nullptr)
	, m_notesTextField(nullptr)
	, m_modified(false) {
	const GameVersion * defaultGameVersion = nullptr;

	for(const GameVersion * currentDefaultGameVersion : GameVersion::DEFAULT_GAME_VERSIONS) {
		if(Utilities::areStringsEqualIgnoreCase(m_gameVersion->getID(), currentDefaultGameVersion->getID())) {
			defaultGameVersion = currentDefaultGameVersion;
			break;
		}
	}

	wxWrapSizer * gameInformationSizer = new wxWrapSizer(wxHORIZONTAL);
	wxStaticBox * gameInformationBox = new wxStaticBox(this, wxID_ANY, "Game Information", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Game Information");
	gameInformationBox->SetOwnFont(gameInformationBox->GetFont().MakeBold());

	wxPanel * gameInformationPanel = new wxPanel(gameInformationBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&GameVersion::getID, m_gameVersion.get()), std::bind(&GameVersion::setID, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getID() : "", "Game Version Identifier", gameInformationPanel, gameInformationSizer, 1));
	m_gameVersionIDSettingPanel = m_settingsPanels.back();
	m_gameVersionIDSettingPanel->setEditable(!m_gameVersion->hasID());
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&GameVersion::getLongName, m_gameVersion.get()), std::bind(&GameVersion::setLongName, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getLongName() : "", "Game Version Long Name", gameInformationPanel, gameInformationSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&GameVersion::getShortName, m_gameVersion.get()), std::bind(&GameVersion::setShortName, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getShortName() : "", "Game Version Short Name", gameInformationPanel, gameInformationSizer, 1));
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
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&GameVersion::getLaunchArguments, m_gameVersion.get()), std::bind(&GameVersion::setLaunchArguments, m_gameVersion.get(), std::placeholders::_1), Utilities::emptyString, "Launch Arguments", gameConfigurationPanel, gameConfigurationSizer));
	m_settingsPanels.push_back(SettingPanel::createOptionalStringSettingPanel<void>(std::bind(&GameVersion::getGroupFileInstallPath, m_gameVersion.get()), std::bind(&GameVersion::setGroupFileInstallPath, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearGroupFileInstallPath, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getGroupFileInstallPath() : std::optional<std::string>(), "Group File Install Path", gameConfigurationPanel, gameConfigurationSizer));
	m_settingsPanels.push_back(SettingPanel::createOptionalBooleanSettingPanel(std::bind(&GameVersion::getRequiresCombinedGroup, m_gameVersion.get()), std::bind(&GameVersion::setRequiresCombinedGroup, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearRequiresCombinedGroup, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getRequiresCombinedGroup() : std::optional<bool>(), "Requires Combined Group", gameConfigurationPanel, gameConfigurationSizer));
	m_settingsPanels.push_back(SettingPanel::createOptionalBooleanSettingPanel(std::bind(&GameVersion::getRequiresGroupFileExtraction, m_gameVersion.get()), std::bind(&GameVersion::setRequiresGroupFileExtraction, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearRequiresGroupFileExtraction, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getRequiresGroupFileExtraction() : std::optional<bool>(), "Requires Group File Extraction", gameConfigurationPanel, gameConfigurationSizer));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<void>(std::bind(&GameVersion::getModDirectoryName, m_gameVersion.get()), std::bind(&GameVersion::setModDirectoryName, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getModDirectoryName() : "", "Mod Subdirectory Name", gameConfigurationPanel, gameConfigurationSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createBooleanSettingPanel(std::bind(&GameVersion::hasLocalWorkingDirectory, m_gameVersion.get()), std::bind(&GameVersion::setLocalWorkingDirectory, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->hasLocalWorkingDirectory() : GameVersion::DEFAULT_LOCAL_WORKING_DIRECTORY, "Local Working Directory", gameConfigurationPanel, gameConfigurationSizer));
	m_settingsPanels.push_back(SettingPanel::createBooleanSettingPanel(std::bind(&GameVersion::hasRelativeConFilePath, m_gameVersion.get()), std::bind(&GameVersion::setRelativeConFilePath, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->hasRelativeConFilePath() : GameVersion::DEFAULT_RELATIVE_CON_FILE_PATH, "Relative CON File Path", gameConfigurationPanel, gameConfigurationSizer));
	m_settingsPanels.push_back(SettingPanel::createBooleanSettingPanel(std::bind(&GameVersion::doesSupportSubdirectories, m_gameVersion.get()), std::bind(&GameVersion::setSupportsSubdirectories, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->doesSupportSubdirectories() : GameVersion::DEFAULT_SUPPORTS_SUBDIRECTORIES, "Supports Subdirectories", gameConfigurationPanel, gameConfigurationSizer));
	m_settingsPanels.push_back(SettingPanel::createBooleanSettingPanel(std::bind(&GameVersion::isWorldTourGroupSupported, m_gameVersion.get()), std::bind(&GameVersion::setWorldTourGroupSupported, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->isWorldTourGroupSupported() : GameVersion::DEFAULT_WORLD_TOUR_GROUP_SUPPORTED, "Supports World Tour Group", gameConfigurationPanel, gameConfigurationSizer));

	wxStaticBox * compatibleGameVersionsBox = new wxStaticBox(this, wxID_ANY, "Compatible Game Versions", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Compatible Game Versions");
	compatibleGameVersionsBox->SetOwnFont(compatibleGameVersionsBox->GetFont().MakeBold());

	std::vector<std::string> compatibleGameVersionIdentifiers(GameVersionCollection::getGameVersionIdentifiersFrom(GameVersion::DEFAULT_GAME_VERSIONS));
	std::vector<std::string> compatibleGameVersionShortNames(GameVersionCollection::getGameVersionShortNamesFrom(GameVersion::DEFAULT_GAME_VERSIONS, false));

	std::vector<std::string>::const_iterator currentGameVersionIdentifierIterator(std::find_if(compatibleGameVersionIdentifiers.cbegin(), compatibleGameVersionIdentifiers.cend(), [this](const std::string & currentGameVersionID) {
		return Utilities::areStringsEqualIgnoreCase(m_gameVersion->getID(), currentGameVersionID);
	}));

	if(currentGameVersionIdentifierIterator != compatibleGameVersionIdentifiers.cend()) {
		size_t currentGameversionIndex = currentGameVersionIdentifierIterator - compatibleGameVersionIdentifiers.cbegin();
		compatibleGameVersionIdentifiers.erase(compatibleGameVersionIdentifiers.cbegin() + currentGameversionIndex);
		compatibleGameVersionShortNames.erase(compatibleGameVersionShortNames.cbegin() + currentGameversionIndex);
	}

	SettingPanel * compatibleGameVersionsPanel = SettingPanel::createStringMultiChoiceSettingPanel(std::bind(&GameVersion::getCompatibleGameVersionIdentifiers, m_gameVersion.get()), std::bind(&GameVersion::hasCompatibleGameVersionWithID, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::addCompatibleGameVersionWithID, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::removeCompatibleGameVersionWithID, m_gameVersion.get(), std::placeholders::_1), "Compatible Game Versions", false, compatibleGameVersionShortNames, compatibleGameVersionsBox, 0, nullptr, compatibleGameVersionIdentifiers, defaultGameVersion != nullptr ? defaultGameVersion->getCompatibleGameVersionIdentifiers() : std::vector<std::string>());
	m_settingsPanels.push_back(compatibleGameVersionsPanel);

	wxStaticBox * supportedOperatingSystemsBox = new wxStaticBox(this, wxID_ANY, "Supported Operating Systems", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Supported Operating Systems");
	supportedOperatingSystemsBox->SetOwnFont(supportedOperatingSystemsBox->GetFont().MakeBold());

	SettingPanel * supportedOperatingSystemsPanel = SettingPanel::createEnumMultiChoiceSettingPanel<GameVersion::OperatingSystem>(std::bind(&GameVersion::getSupportedOperatingSystems, m_gameVersion.get()), std::bind(&GameVersion::hasSupportedOperatingSystem, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::addSupportedOperatingSystem, m_gameVersion.get(), std::placeholders::_1), std::bind(static_cast<bool(GameVersion::*)(GameVersion::OperatingSystem)>(&GameVersion::removeSupportedOperatingSystem), m_gameVersion.get(), std::placeholders::_1), "Supported Operating Systems", supportedOperatingSystemsBox, 1, nullptr, defaultGameVersion != nullptr ? defaultGameVersion->getSupportedOperatingSystems() : std::vector<GameVersion::OperatingSystem>());
	m_settingsPanels.push_back(supportedOperatingSystemsPanel);

	wxWrapSizer * argumentsSizer = new wxWrapSizer(wxHORIZONTAL);
	wxStaticBox * argumentsBox = new wxStaticBox(this, wxID_ANY, "Executable Argument Flags", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Argument Flags");
	argumentsBox->SetOwnFont(argumentsBox->GetFont().MakeBold());

	wxPanel * argumentsPanel = new wxPanel(argumentsBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	m_settingsPanels.push_back(SettingPanel::createOptionalStringSettingPanel<bool>(std::bind(&GameVersion::getConFileArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setConFileArgumentFlag, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearConFileArgumentFlag, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getConFileArgumentFlag() : std::optional<std::string>(), "Con File", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createOptionalStringSettingPanel<bool>(std::bind(&GameVersion::getExtraConFileArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setExtraConFileArgumentFlag, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearExtraConFileArgumentFlag, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getExtraConFileArgumentFlag() : std::optional<std::string>(), "Extra Con File", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createOptionalStringSettingPanel<bool>(std::bind(&GameVersion::getGroupFileArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setGroupFileArgumentFlag, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearGroupFileArgumentFlag, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getGroupFileArgumentFlag() : std::optional<std::string>(), "Group File", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createOptionalStringSettingPanel<bool>(std::bind(&GameVersion::getDefFileArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setDefFileArgumentFlag, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearDefFileArgumentFlag, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getDefFileArgumentFlag() : std::optional<std::string>(), "Def File", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createOptionalStringSettingPanel<bool>(std::bind(&GameVersion::getExtraDefFileArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setExtraDefFileArgumentFlag, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearExtraDefFileArgumentFlag, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getExtraDefFileArgumentFlag() : std::optional<std::string>(), "Extra Def File", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createOptionalStringSettingPanel<bool>(std::bind(&GameVersion::getMapFileArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setMapFileArgumentFlag, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearMapFileArgumentFlag, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getMapFileArgumentFlag() : std::optional<std::string>(), "Map File", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<bool>(std::bind(&GameVersion::getEpisodeArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setEpisodeArgumentFlag, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getEpisodeArgumentFlag() : "", "Episode", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<bool>(std::bind(&GameVersion::getLevelArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setLevelArgumentFlag, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getLevelArgumentFlag() : "", "Level", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<bool>(std::bind(&GameVersion::getSkillArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setSkillArgumentFlag, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getSkillArgumentFlag() : "", "Skill", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createIntegerSettingPanel<uint8_t>(std::bind(&GameVersion::getSkillStartValue, m_gameVersion.get()), std::bind(&GameVersion::setSkillStartValue, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getSkillStartValue() : GameVersion::DEFAULT_SKILL_START_VALUE, "Skill Start Value", argumentsPanel, argumentsSizer));
	m_settingsPanels.push_back(SettingPanel::createStringSettingPanel<bool>(std::bind(&GameVersion::getRecordDemoArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setRecordDemoArgumentFlag, m_gameVersion.get(), std::placeholders::_1), defaultGameVersion != nullptr ? defaultGameVersion->getRecordDemoArgumentFlag() : "", "Record Demo", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createOptionalStringSettingPanel<bool>(std::bind(&GameVersion::getPlayDemoArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setPlayDemoArgumentFlag, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearPlayDemoArgumentFlag, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getPlayDemoArgumentFlag() : std::optional<std::string>(), "Play Demo", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createOptionalStringSettingPanel<bool>(std::bind(&GameVersion::getRespawnModeArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setRespawnModeArgumentFlag, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearRespawnModeArgumentFlag, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getRespawnModeArgumentFlag() : std::optional<std::string>(), "Respawn Mode", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createOptionalStringSettingPanel<bool>(std::bind(&GameVersion::getWeaponSwitchOrderArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setWeaponSwitchOrderArgumentFlag, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearWeaponSwitchOrderArgumentFlag, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getWeaponSwitchOrderArgumentFlag() : std::optional<std::string>(), "Weapon Switch Order", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createOptionalStringSettingPanel<bool>(std::bind(&GameVersion::getDisableMonstersArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setDisableMonstersArgumentFlag, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearDisableMonstersArgumentFlag, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getDisableMonstersArgumentFlag() : std::optional<std::string>(), "Disable Monsters", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createOptionalStringSettingPanel<bool>(std::bind(&GameVersion::getDisableSoundArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setDisableSoundArgumentFlag, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearDisableSoundArgumentFlag, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getDisableSoundArgumentFlag() : std::optional<std::string>(), "Disable Sound", argumentsPanel, argumentsSizer, 1));
	m_settingsPanels.push_back(SettingPanel::createOptionalStringSettingPanel<bool>(std::bind(&GameVersion::getDisableMusicArgumentFlag, m_gameVersion.get()), std::bind(&GameVersion::setDisableMusicArgumentFlag, m_gameVersion.get(), std::placeholders::_1), std::bind(&GameVersion::clearDisableMusicArgumentFlag, m_gameVersion.get()), defaultGameVersion != nullptr ? defaultGameVersion->getDisableMusicArgumentFlag() : std::optional<std::string>(), "Disable Music", argumentsPanel, argumentsSizer, 1));

	m_dosboxConfigurationBox = new wxStaticBox(this, wxID_ANY, (m_gameVersion->hasLongName() ? m_gameVersion->getLongName() + " " : "") + "DOSBox Configuration", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Game DOSBox Configuration");
	m_dosboxConfigurationBox->SetOwnFont(m_dosboxConfigurationBox->GetFont().MakeBold());

	m_dosboxConfigurationPanel = new DOSBoxConfigurationPanel(m_gameVersion->getDOSBoxConfiguration(), m_dosboxConfigurationBox);
	m_dosboxConfigurationPanel->setEnabled(m_gameVersion->doesRequireDOSBox());

	wxStaticBox * notesBox = new wxStaticBox(this, wxID_ANY, "Notes", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, "Notes");
	notesBox->SetOwnFont(notesBox->GetFont().MakeBold());
	notesBox->SetMaxClientSize(wxSize(notesBox->GetMaxClientSize().x, 80));

	wxPanel * notesPanel = new wxPanel(notesBox, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	notesPanel->SetMaxClientSize(wxSize(notesPanel->GetMaxClientSize().x, 60));

	m_notesTextField = new wxTextCtrl(notesPanel, wxID_ANY, m_gameVersion->getNotesAsString(), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	m_notesTextField->SetMaxClientSize(wxSize(m_notesTextField->GetMaxSize().x, 70));
	m_notesTextField->Bind(wxEVT_TEXT, std::bind(&GameVersionPanel::onNotesModified, this, std::placeholders::_1), wxID_ANY, wxID_ANY);

	int border = 2;

	wxBoxSizer * dosboxConfigurationBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	dosboxConfigurationBoxSizer->Add(m_dosboxConfigurationPanel, 1, wxEXPAND | wxALL, 20);
	m_dosboxConfigurationBox->SetSizerAndFit(dosboxConfigurationBoxSizer);

	wxBoxSizer * notesSizer = new wxBoxSizer(wxVERTICAL);
	notesSizer->Add(m_notesTextField, 1, wxEXPAND | wxHORIZONTAL);
	notesPanel->SetSizerAndFit(notesSizer);

	gameInformationPanel->SetSizerAndFit(gameInformationSizer);
	gameConfigurationPanel->SetSizerAndFit(gameConfigurationSizer);
	argumentsPanel->SetSizerAndFit(argumentsSizer);

	wxBoxSizer * gameInformationBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	gameInformationBoxSizer->Add(gameInformationPanel, 1, wxEXPAND | wxALL, 18);
	gameInformationBox->SetSizerAndFit(gameInformationBoxSizer);

	wxBoxSizer * gameConfigurationBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	gameConfigurationBoxSizer->Add(gameConfigurationPanel, 1, wxEXPAND | wxALL, 18);
	gameConfigurationBox->SetSizerAndFit(gameConfigurationBoxSizer);

	wxBoxSizer * compatibleGameVersionsBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	compatibleGameVersionsBoxSizer->Add(compatibleGameVersionsPanel, 1, wxEXPAND | wxALL, 18);
	compatibleGameVersionsBox->SetSizerAndFit(compatibleGameVersionsBoxSizer);

	wxBoxSizer * supportedOperatingSystemsBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	supportedOperatingSystemsBoxSizer->Add(supportedOperatingSystemsPanel, 1, wxEXPAND | wxALL, 18);
	supportedOperatingSystemsBox->SetSizerAndFit(supportedOperatingSystemsBoxSizer);

	wxBoxSizer * argumentsBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	argumentsBoxSizer->Add(argumentsPanel, 1, wxEXPAND | wxALL, 18);
	argumentsBox->SetSizerAndFit(argumentsBoxSizer);

	wxBoxSizer * notesBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	notesBoxSizer->Add(notesPanel, 1, wxEXPAND | wxALL, 18);
	notesBox->SetSizerAndFit(notesBoxSizer);

	wxGridBagSizer * gameVersionConfigurationSizer = new wxGridBagSizer(border, border);
	gameVersionConfigurationSizer->Add(gameInformationBox, wxGBPosition(0, 0), wxGBSpan(1, 2), wxEXPAND | wxALL, border);
	gameVersionConfigurationSizer->Add(gameConfigurationBox, wxGBPosition(1, 0), wxGBSpan(1, 2), wxEXPAND | wxALL, border);
	gameVersionConfigurationSizer->Add(compatibleGameVersionsBox, wxGBPosition(2, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	gameVersionConfigurationSizer->Add(supportedOperatingSystemsBox, wxGBPosition(2, 1), wxGBSpan(1, 1), wxEXPAND | wxALL, border);
	gameVersionConfigurationSizer->Add(argumentsBox, wxGBPosition(3, 0), wxGBSpan(1, 2), wxEXPAND | wxALL, border);
	gameVersionConfigurationSizer->Add(m_dosboxConfigurationBox, wxGBPosition(4, 0), wxGBSpan(1, 2), wxEXPAND | wxALL, border);
	gameVersionConfigurationSizer->Add(notesBox, wxGBPosition(5, 0), wxGBSpan(1, 2), wxEXPAND | wxALL, border);
	gameVersionConfigurationSizer->AddGrowableRow(0, 1);
	gameVersionConfigurationSizer->AddGrowableRow(1, 3);
	gameVersionConfigurationSizer->AddGrowableRow(2, 1);
	gameVersionConfigurationSizer->AddGrowableRow(3, 6);
	gameVersionConfigurationSizer->AddGrowableRow(4, 4);
	gameVersionConfigurationSizer->AddGrowableRow(5, 1);
	gameVersionConfigurationSizer->AddGrowableCol(0, 6);
	gameVersionConfigurationSizer->AddGrowableCol(1, 1);
	SetSizerAndFit(gameVersionConfigurationSizer);

	m_gameVersionModifiedConnection = m_gameVersion->modified.connect(std::bind(&GameVersionPanel::onGameVersionModified, this, std::placeholders::_1));

	for(SettingPanel * settingPanel : m_settingsPanels) {
		m_settingModifiedConnections.push_back(settingPanel->settingModified.connect(std::bind(&GameVersionPanel::onSettingModified, this, std::placeholders::_1)));
	}
}

GameVersionPanel::~GameVersionPanel() {
	m_gameVersionModifiedConnection.disconnect();

	for(boost::signals2::connection & settingModifiedConnection : m_settingModifiedConnections) {
		settingModifiedConnection.disconnect();
	}
}

bool GameVersionPanel::isModified() const {
	return m_modified;
}

std::string GameVersionPanel::getPanelName() const {
	return m_gameVersion->hasShortName() ? fmt::format("{}{}", m_gameVersion->getShortName(), m_modified ? " *" : "") : "NEW GAME *";
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

	m_gameVersion->setNotes(m_notesTextField->GetValue());

	m_modified = false;

	m_gameVersionIDSettingPanel->setEditable(false);

	gameVersionSaved(*this);

	return true;
}

void GameVersionPanel::discard() {
	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->discard();
	}

	m_notesTextField->SetValue(m_gameVersion->getNotesAsString());

	m_modified = false;

	gameVersionChangesDiscarded(*this);
}

void GameVersionPanel::reset() {
	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->reset();
	}

	std::vector<const GameVersion *>::const_iterator gameVersionIterator(std::find_if(GameVersion::DEFAULT_GAME_VERSIONS.cbegin(), GameVersion::DEFAULT_GAME_VERSIONS.cend(), [this](const GameVersion * currentGameVersion) {
		return Utilities::areStringsEqual(m_gameVersion->getID(), currentGameVersion->getID());
	}));

	if(gameVersionIterator != GameVersion::DEFAULT_GAME_VERSIONS.cend()) {
		m_notesTextField->SetValue((*gameVersionIterator)->getNotesAsString());
	}

	m_modified = true;

	gameVersionReset(*this);
}

void GameVersionPanel::discardGamePathChanges() {
	m_gamePathSettingPanel->discard();
}

void GameVersionPanel::update() {
	for(SettingPanel * settingPanel : m_settingsPanels) {
		settingPanel->update();
	}

	m_notesTextField->SetValue(m_gameVersion->getNotesAsString());

	m_dosboxConfigurationPanel->setEnabled(m_gameVersion->doesRequireDOSBox());
	m_dosboxConfigurationBox->SetLabelText((m_gameVersion->hasLongName() ? m_gameVersion->getLongName() + " " : "") + "DOSBox Configuration");
}

void GameVersionPanel::onGameVersionModified(GameVersion & gameVersion) {
	update();
}

void GameVersionPanel::onSettingModified(SettingPanel & settingPanel) {
	if(settingPanel.isModified()) {
		m_modified = true;

		gameVersionSettingChanged(*this, settingPanel);
	}
}

void GameVersionPanel::onNotesModified(wxCommandEvent & event) {
	gameVersionNotesChanged(*this);
}
