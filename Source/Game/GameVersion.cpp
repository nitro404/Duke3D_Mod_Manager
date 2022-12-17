#include "GameVersion.h"

#include "Mod/ModGameVersion.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string_view>
#include <vector>

static constexpr const char * JSON_NAME_PROPERTY_NAME = "name";
static constexpr const char * JSON_GAME_PATH_PROPERTY_NAME = "gamePath";
static constexpr const char * JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME = "gameExecutableName";
static constexpr const char * JSON_SETUP_EXECUTABLE_NAME_PROPERTY_NAME = "setupExecutableName";
static constexpr const char * JSON_GROUP_FILE_INSTALL_PATH_PROPERTY_NAME = "groupFileInstallPath";
static constexpr const char * JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME = "relativeConFilePath";
static constexpr const char * JSON_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME = "supportsSubdirectories";
static constexpr const char * JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME = "conFileArgumentFlag";
static constexpr const char * JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME = "groupFileArgumentFlag";
static constexpr const char * JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME = "defFileArgumentFlag";
static constexpr const char * JSON_MAP_FILE_ARGUMENT_FLAG_PROPERTY_NAME = "mapFileArgumentFlag";
static constexpr const char * JSON_EPISODE_ARGUMENT_FLAG_PROPERTY_NAME = "episodeArgumentFlag";
static constexpr const char * JSON_LEVEL_ARGUMENT_FLAG_PROPERTY_NAME = "levelArgumentFlag";
static constexpr const char * JSON_SKILL_ARGUMENT_FLAG_PROPERTY_NAME = "skillArgumentFlag";
static constexpr const char * JSON_RECORD_DEMO_ARGUMENT_FLAG_PROPERTY_NAME = "recordDemoArgumentFlag";
static constexpr const char * JSON_PLAY_DEMO_ARGUMENT_FLAG_PROPERTY_NAME = "playDemoArgumentFlag";
static constexpr const char * JSON_RESPAWN_MODE_ARGUMENT_FLAG_PROPERTY_NAME = "respawnModeArgumentFlag";
static constexpr const char * JSON_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG_PROPERTY_NAME = "weaponSwitchOrderArgumentFlag";
static constexpr const char * JSON_DISABLE_MONSTERS_ARGUMENT_FLAG_PROPERTY_NAME = "disableMonstersArgumentFlag";
static constexpr const char * JSON_DISABLE_SOUND_ARGUMENT_FLAG_PROPERTY_NAME = "disableSoundArgumentFlag";
static constexpr const char * JSON_DISABLE_MUSIC_ARGUMENT_FLAG_PROPERTY_NAME = "disableMusicArgumentFlag";
static constexpr const char * JSON_REQUIRES_COMBINED_GROUP_PROPERTY_NAME = "requiresCombinedGroup";
static constexpr const char * JSON_REQUIRES_DOSBOX_PROPERTY_NAME = "requiresDOSBox";
static constexpr const char * JSON_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME = "localWorkingDirectory";
static constexpr const char * JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME = "modDirectoryName";
static constexpr const char * JSON_WEBSITE_PROPERTY_NAME = "website";
static constexpr const char * JSON_SOURCE_CODE_URL_PROPERTY_NAME = "sourceCodeURL";
static constexpr const char * JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME = "supportedOperatingSystems";
static constexpr const char * JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME = "compatibleGameVersions";
static const std::array<std::string_view, 29> JSON_PROPERTY_NAMES = {
	JSON_NAME_PROPERTY_NAME,
	JSON_GAME_PATH_PROPERTY_NAME,
	JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME,
	JSON_SETUP_EXECUTABLE_NAME_PROPERTY_NAME,
	JSON_GROUP_FILE_INSTALL_PATH_PROPERTY_NAME,
	JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME,
	JSON_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME,
	JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_MAP_FILE_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_EPISODE_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_LEVEL_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_SKILL_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_RECORD_DEMO_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_PLAY_DEMO_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_RESPAWN_MODE_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_DISABLE_MONSTERS_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_DISABLE_SOUND_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_DISABLE_MUSIC_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_REQUIRES_COMBINED_GROUP_PROPERTY_NAME,
	JSON_REQUIRES_DOSBOX_PROPERTY_NAME,
	JSON_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME,
	JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME,
	JSON_WEBSITE_PROPERTY_NAME,
	JSON_SOURCE_CODE_URL_PROPERTY_NAME,
	JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME,
	JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME
};

const std::string GameVersion::ALL_VERSIONS = "All Versions";

const GameVersion GameVersion::ORIGINAL_REGULAR_VERSION("Regular Version 1.3",         "", "DUKE3D.EXE",         true,  false, true,  "/x",  "/g",  "-map ", "/v", "/l", "/s", "/r", {},    "/t", "/u", "/m", "/ns", "/nm", "Regular",   "SETUP.EXE", "", {},    true,  true,  "https://www.dukenukem.com",                                     "",                                                   { OperatingSystem::DOS });
const GameVersion GameVersion::ORIGINAL_ATOMIC_EDITION ("Atomic Edition 1.4/1.5",      "", "DUKE3D.EXE",         true,  false, true,  "/x",  "/g",  "-map ", "/v", "/l", "/s", "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", "Atomic",    "SETUP.EXE", "", {},    {},    true,  "https://www.dukenukem.com",                                     "",                                                   { OperatingSystem::DOS });
const GameVersion GameVersion::JFDUKE3D                ("JFDuke3D",                    "", "duke3d.exe",         true,  false, true,  "/x",  "/g",  "-map ", "/v", "/l", "/s", "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", "JFDuke3D",  {},          {}, {},    {},    {},    "http://www.jonof.id.au/jfduke3d",                               "https://github.com/jonof/jfduke3d",                  { OperatingSystem::Windows, OperatingSystem::MacOS },                         { ORIGINAL_REGULAR_VERSION.getName(), ORIGINAL_ATOMIC_EDITION.getName() });
const GameVersion GameVersion::EDUKE32                 ("eDuke32",                     "", "eduke32.exe",        false, true,  true,  "-x ", "-g ", "-map ", "-v", "-l", "-s", "-r", "-d ", "-t", "-u", "-m", "-ns", "-nm", "eDuke32",   {},          {}, "-h ", {},    {},    "https://www.eduke32.com",                                       "https://voidpoint.io/terminx/eduke32",               { OperatingSystem::Windows },                                                 { ORIGINAL_REGULAR_VERSION.getName(), ORIGINAL_ATOMIC_EDITION.getName(), JFDUKE3D.getName() });
//const GameVersion GameVersion::MEGATON_EDITION         ("Megaton Edition",             "", "duke3d.exe",         true,  false, true,  "/x",  "/g",  "-map ", "/v", "/l", "/s", "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", "Megaton",   {},          {}, {},    {},    {},    "https://store.steampowered.com/app/225140",                     "https://github.com/TermiT/duke3d-megaton",           { OperatingSystem::Windows },                                                 { ORIGINAL_REGULAR_VERSION.getName(), ORIGINAL_ATOMIC_EDITION.getName(), JFDUKE3D.getName() });
const GameVersion GameVersion::WORLD_TOUR              ("20th Anniversary World Tour", "", "duke3d.exe",         true,  false, true,  "/x",  "/g",  "-map ", "/v", "/l", "/s", "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", "WorldTour", {},          {}, {},    {},    {},    "https://www.gearboxsoftware.com/game/duke-3d-20th",             "",                                                   { OperatingSystem::Windows },                                                 { ORIGINAL_REGULAR_VERSION.getName(), ORIGINAL_ATOMIC_EDITION.getName() });
//const GameVersion GameVersion::BUILDGDX                ("BuildGDX",                    "", "BuildGDX.jar",       true,  true,  true,  "",    "",    "-map ", "/v", "/l", "/s", "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", "BuildGDX",  {},          {}, {},    {},    {},    "https://m210.duke4.net/index.php/downloads/category/8-java",    "https://gitlab.com/m210/BuildGDX",                   { OperatingSystem::Windows, OperatingSystem::Linux, OperatingSystem::MacOS }, { ORIGINAL_REGULAR_VERSION.getName(), ORIGINAL_ATOMIC_EDITION.getName() });
const GameVersion GameVersion::RAZE                    ("Raze",                        "", "raze.exe",           true,  true,  true,  "-x ", "-g ", "-map ", "-v", "-l", "-s", "-r", "-d ", "-t", "-u", "-m", "-ns", "-nm", "Raze",      {},          {}, "-h ", {},    {},    "https://raze.zdoom.org/about",                                  "https://github.com/coelckers/Raze",                  { OperatingSystem::Windows, OperatingSystem::Linux, OperatingSystem::MacOS }, { ORIGINAL_ATOMIC_EDITION.getName(),  JFDUKE3D.getName() });
const GameVersion GameVersion::RED_NUKEM               ("RedNukem",                    "", "rednukem.exe",       false, true,  true,  "-x ", "-g ", "-map ", "-v", "-l", "-s", "-r", "-d ", "-t", "-u", "-m", "-ns", "-nm", "RedNukem",  {},          {}, "-h ", {},    {},    "https://lerppu.net/wannabethesis",                              "https://github.com/nukeykt/NRedneck",                { OperatingSystem::Windows },                                                 { ORIGINAL_ATOMIC_EDITION.getName(), JFDUKE3D.getName() });
const GameVersion GameVersion::CHOCOLATE_DUKE3D        ("Chocolate Duke3D",            "", "Game.exe",           true,  false, false, "/x",  "/g",  "-map ", "/v", "/l", "/s", "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", "Chocolate", {},          {}, {},    {},    {},    "https://fabiensanglard.net/duke3d/chocolate_duke_nukem_3D.php", "https://github.com/fabiensanglard/chocolate_duke3D", { OperatingSystem::Windows },                                                 { ORIGINAL_ATOMIC_EDITION.getName() });
const GameVersion GameVersion::BELGIAN_CHOCOLATE_DUKE3D("Belgian Chocolate Duke3D",    "", "ChocoDuke3D.64.exe", true,  false, false, "/x",  "/g",  "-map ", "/v", "/l", "/s", "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", "Belgian",   {},          "", {},    {},    {},    "",                                                              "https://github.com/GPSnoopy/BelgianChocolateDuke3D", { OperatingSystem::Windows, OperatingSystem::Linux, OperatingSystem::MacOS }, { ORIGINAL_ATOMIC_EDITION.getName() });

const std::vector<GameVersion> GameVersion::DEFAULT_GAME_VERSIONS = {
	GameVersion::ORIGINAL_REGULAR_VERSION,
	GameVersion::ORIGINAL_ATOMIC_EDITION,
	GameVersion::JFDUKE3D,
	GameVersion::EDUKE32,
//	GameVersion::MEGATON_EDITION,
	GameVersion::WORLD_TOUR,
//	GameVersion::BUILDGDX,
	GameVersion::RAZE,
	GameVersion::RED_NUKEM,
	GameVersion::CHOCOLATE_DUKE3D,
	GameVersion::BELGIAN_CHOCOLATE_DUKE3D
};

GameVersion::GameVersion(const std::string & name, const std::string & gamePath, const std::string & gameExecutableName, bool localWorkingDirectory, bool relativeConFilePath, bool supportsSubdirectories, const std::string & conFileArgumentFlag, const std::string & groupFileArgumentFlag, const std::string & mapFileArgumentFlag, const std::string & episodeArgumentFlag, const std::string & levelArgumentFlag, const std::string & skillArgumentFlag, const std::string & recordDemoArgumentFlag, const std::optional<std::string> & playDemoArgumentFlag, const std::string & respawnModeArgumentFlag, const std::string & weaponSwitchOrderArgumentFlag, const std::string & disableMonstersArgumentFlag, const std::string & disableSoundArgumentFlag, const std::string & disableMusicArgumentFlag, const std::string & modDirectoryName, const std::optional<std::string> & setupExecutableName, const std::optional<std::string> & groupFileInstallPath, const std::optional<std::string> & defFileArgumentFlag, const std::optional<bool> & requiresCombinedGroup, const std::optional<bool> & requiresDOSBox, const std::string & website, const std::string & sourceCodeURL, const std::vector<OperatingSystem> & supportedOperatingSystems, const std::vector<std::string> & compatibleGameVersions)
	: m_name(Utilities::trimString(name))
	, m_gamePath(Utilities::trimString(gamePath))
	, m_gameExecutableName(Utilities::trimString(gameExecutableName))
	, m_requiresCombinedGroup(requiresCombinedGroup)
	, m_requiresDOSBox(requiresDOSBox)
	, m_modDirectoryName(Utilities::trimString(modDirectoryName))
	, m_website(Utilities::trimString(website))
	, m_sourceCodeURL(Utilities::trimString(sourceCodeURL))
	, m_localWorkingDirectory(localWorkingDirectory)
	, m_relativeConFilePath(relativeConFilePath)
	, m_supportsSubdirectories(supportsSubdirectories)
	, m_conFileArgumentFlag(conFileArgumentFlag)
	, m_groupFileArgumentFlag(groupFileArgumentFlag)
	, m_defFileArgumentFlag(defFileArgumentFlag)
	, m_mapFileArgumentFlag(mapFileArgumentFlag)
	, m_episodeArgumentFlag(episodeArgumentFlag)
	, m_levelArgumentFlag(levelArgumentFlag)
	, m_skillArgumentFlag(skillArgumentFlag)
	, m_recordDemoArgumentFlag(recordDemoArgumentFlag)
	, m_playDemoArgumentFlag(playDemoArgumentFlag)
	, m_respawnModeArgumentFlag(respawnModeArgumentFlag)
	, m_weaponSwitchOrderArgumentFlag(weaponSwitchOrderArgumentFlag)
	, m_disableMonstersArgumentFlag(disableMonstersArgumentFlag)
	, m_disableSoundArgumentFlag(disableSoundArgumentFlag)
	, m_disableMusicArgumentFlag(disableMusicArgumentFlag) {
	if(setupExecutableName.has_value()) {
		m_setupExecutableName = Utilities::trimString(setupExecutableName.value());
	}

	if(groupFileInstallPath.has_value()) {
		m_groupFileInstallPath = Utilities::trimString(groupFileInstallPath.value());
	}

	addSupportedOperatingSystems(supportedOperatingSystems);
	addCompatibleGameVersions(compatibleGameVersions);
}

GameVersion::GameVersion(GameVersion && gameVersion) noexcept
	: m_name(std::move(gameVersion.m_name))
	, m_gamePath(std::move(gameVersion.m_gamePath))
	, m_gameExecutableName(std::move(gameVersion.m_gameExecutableName))
	, m_setupExecutableName(std::move(gameVersion.m_setupExecutableName))
	, m_groupFileInstallPath(std::move(gameVersion.m_groupFileInstallPath))
	, m_requiresCombinedGroup(gameVersion.m_requiresCombinedGroup)
	, m_requiresDOSBox(gameVersion.m_requiresDOSBox)
	, m_localWorkingDirectory(gameVersion.m_localWorkingDirectory)
	, m_modDirectoryName(std::move(gameVersion.m_modDirectoryName))
	, m_website(std::move(gameVersion.m_website))
	, m_sourceCodeURL(std::move(gameVersion.m_sourceCodeURL))
	, m_relativeConFilePath(gameVersion.m_relativeConFilePath)
	, m_supportsSubdirectories(gameVersion.m_supportsSubdirectories)
	, m_conFileArgumentFlag(std::move(gameVersion.m_conFileArgumentFlag))
	, m_groupFileArgumentFlag(std::move(gameVersion.m_groupFileArgumentFlag))
	, m_defFileArgumentFlag(std::move(gameVersion.m_defFileArgumentFlag))
	, m_mapFileArgumentFlag(std::move(gameVersion.m_mapFileArgumentFlag))
	, m_episodeArgumentFlag(std::move(gameVersion.m_episodeArgumentFlag))
	, m_levelArgumentFlag(std::move(gameVersion.m_levelArgumentFlag))
	, m_skillArgumentFlag(std::move(gameVersion.m_skillArgumentFlag))
	, m_recordDemoArgumentFlag(std::move(gameVersion.m_recordDemoArgumentFlag))
	, m_playDemoArgumentFlag(std::move(gameVersion.m_playDemoArgumentFlag))
	, m_respawnModeArgumentFlag(std::move(gameVersion.m_respawnModeArgumentFlag))
	, m_weaponSwitchOrderArgumentFlag(std::move(gameVersion.m_weaponSwitchOrderArgumentFlag))
	, m_disableMonstersArgumentFlag(std::move(gameVersion.m_disableMonstersArgumentFlag))
	, m_disableSoundArgumentFlag(std::move(gameVersion.m_disableSoundArgumentFlag))
	, m_disableMusicArgumentFlag(std::move(gameVersion.m_disableMusicArgumentFlag))
	, m_supportedOperatingSystems(std::move(gameVersion.m_supportedOperatingSystems))
	, m_compatibleGameVersions(std::move(gameVersion.m_compatibleGameVersions)) { }

GameVersion::GameVersion(const GameVersion & gameVersion)
	: m_name(gameVersion.m_name)
	, m_gamePath(gameVersion.m_gamePath)
	, m_gameExecutableName(gameVersion.m_gameExecutableName)
	, m_setupExecutableName(gameVersion.m_setupExecutableName)
	, m_groupFileInstallPath(gameVersion.m_groupFileInstallPath)
	, m_requiresCombinedGroup(gameVersion.m_requiresCombinedGroup)
	, m_requiresDOSBox(gameVersion.m_requiresDOSBox)
	, m_localWorkingDirectory(gameVersion.m_localWorkingDirectory)
	, m_modDirectoryName(gameVersion.m_modDirectoryName)
	, m_website(gameVersion.m_website)
	, m_sourceCodeURL(gameVersion.m_sourceCodeURL)
	, m_relativeConFilePath(gameVersion.m_relativeConFilePath)
	, m_supportsSubdirectories(gameVersion.m_supportsSubdirectories)
	, m_conFileArgumentFlag(gameVersion.m_conFileArgumentFlag)
	, m_groupFileArgumentFlag(gameVersion.m_groupFileArgumentFlag)
	, m_defFileArgumentFlag(gameVersion.m_defFileArgumentFlag)
	, m_mapFileArgumentFlag(gameVersion.m_mapFileArgumentFlag)
	, m_episodeArgumentFlag(gameVersion.m_episodeArgumentFlag)
	, m_levelArgumentFlag(gameVersion.m_levelArgumentFlag)
	, m_skillArgumentFlag(gameVersion.m_skillArgumentFlag)
	, m_recordDemoArgumentFlag(gameVersion.m_recordDemoArgumentFlag)
	, m_playDemoArgumentFlag(gameVersion.m_playDemoArgumentFlag)
	, m_respawnModeArgumentFlag(gameVersion.m_respawnModeArgumentFlag)
	, m_weaponSwitchOrderArgumentFlag(gameVersion.m_weaponSwitchOrderArgumentFlag)
	, m_disableMonstersArgumentFlag(gameVersion.m_disableMonstersArgumentFlag)
	, m_disableSoundArgumentFlag(gameVersion.m_disableSoundArgumentFlag)
	, m_disableMusicArgumentFlag(gameVersion.m_disableMusicArgumentFlag)
	, m_supportedOperatingSystems(gameVersion.m_supportedOperatingSystems)
	, m_compatibleGameVersions(gameVersion.m_compatibleGameVersions) { }

GameVersion & GameVersion::operator = (GameVersion && gameVersion) noexcept {
	if(this != &gameVersion) {
		m_name = std::move(gameVersion.m_name);
		m_gamePath = std::move(gameVersion.m_gamePath);
		m_gameExecutableName = std::move(gameVersion.m_gameExecutableName);
		m_setupExecutableName = std::move(gameVersion.m_setupExecutableName);
		m_groupFileInstallPath = std::move(gameVersion.m_groupFileInstallPath);
		m_requiresCombinedGroup = gameVersion.m_requiresCombinedGroup;
		m_requiresDOSBox = std::move(gameVersion.m_requiresDOSBox);
		m_localWorkingDirectory = gameVersion.m_localWorkingDirectory;
		m_modDirectoryName = std::move(gameVersion.m_modDirectoryName);
		m_website = std::move(gameVersion.m_website);
		m_sourceCodeURL = std::move(gameVersion.m_sourceCodeURL);
		m_relativeConFilePath = gameVersion.m_relativeConFilePath;
		m_supportsSubdirectories = gameVersion.m_supportsSubdirectories;
		m_conFileArgumentFlag = std::move(gameVersion.m_conFileArgumentFlag);
		m_groupFileArgumentFlag = std::move(gameVersion.m_groupFileArgumentFlag);
		m_defFileArgumentFlag = std::move(gameVersion.m_defFileArgumentFlag);
		m_mapFileArgumentFlag = std::move(gameVersion.m_mapFileArgumentFlag);
		m_episodeArgumentFlag = std::move(gameVersion.m_episodeArgumentFlag);
		m_levelArgumentFlag = std::move(gameVersion.m_levelArgumentFlag);
		m_skillArgumentFlag = std::move(gameVersion.m_skillArgumentFlag);
		m_recordDemoArgumentFlag = std::move(gameVersion.m_recordDemoArgumentFlag);
		m_playDemoArgumentFlag = std::move(gameVersion.m_playDemoArgumentFlag);
		m_respawnModeArgumentFlag = std::move(gameVersion.m_respawnModeArgumentFlag);
		m_weaponSwitchOrderArgumentFlag = std::move(gameVersion.m_weaponSwitchOrderArgumentFlag);
		m_disableMonstersArgumentFlag = std::move(gameVersion.m_disableMonstersArgumentFlag);
		m_disableSoundArgumentFlag = std::move(gameVersion.m_disableSoundArgumentFlag);
		m_disableMusicArgumentFlag = std::move(gameVersion.m_disableMusicArgumentFlag);
		m_supportedOperatingSystems = std::move(gameVersion.m_supportedOperatingSystems);
		m_compatibleGameVersions = std::move(gameVersion.m_compatibleGameVersions);
	}

	return *this;
}

GameVersion & GameVersion::operator = (const GameVersion & gameVersion) {
	m_name = gameVersion.m_name;
	m_gamePath = gameVersion.m_gamePath;
	m_gameExecutableName = gameVersion.m_gameExecutableName;
	m_setupExecutableName = gameVersion.m_setupExecutableName;
	m_groupFileInstallPath = gameVersion.m_groupFileInstallPath;
	m_requiresCombinedGroup = gameVersion.m_requiresCombinedGroup;
	m_requiresDOSBox = gameVersion.m_requiresDOSBox;
	m_localWorkingDirectory = gameVersion.m_localWorkingDirectory;
	m_modDirectoryName = gameVersion.m_modDirectoryName;
	m_website = gameVersion.m_website;
	m_sourceCodeURL = gameVersion.m_sourceCodeURL;
	m_relativeConFilePath = gameVersion.m_relativeConFilePath;
	m_supportsSubdirectories = gameVersion.m_supportsSubdirectories;
	m_conFileArgumentFlag = gameVersion.m_conFileArgumentFlag;
	m_groupFileArgumentFlag = gameVersion.m_groupFileArgumentFlag;
	m_defFileArgumentFlag = gameVersion.m_defFileArgumentFlag;
	m_mapFileArgumentFlag = gameVersion.m_mapFileArgumentFlag;
	m_episodeArgumentFlag = gameVersion.m_episodeArgumentFlag;
	m_levelArgumentFlag = gameVersion.m_levelArgumentFlag;
	m_skillArgumentFlag = gameVersion.m_skillArgumentFlag;
	m_recordDemoArgumentFlag = gameVersion.m_recordDemoArgumentFlag;
	m_playDemoArgumentFlag = gameVersion.m_playDemoArgumentFlag;
	m_respawnModeArgumentFlag = gameVersion.m_respawnModeArgumentFlag;
	m_weaponSwitchOrderArgumentFlag = gameVersion.m_weaponSwitchOrderArgumentFlag;
	m_disableMonstersArgumentFlag = gameVersion.m_disableMonstersArgumentFlag;
	m_disableSoundArgumentFlag = gameVersion.m_disableSoundArgumentFlag;
	m_disableMusicArgumentFlag = gameVersion.m_disableMusicArgumentFlag;
	m_supportedOperatingSystems = gameVersion.m_supportedOperatingSystems;
	m_compatibleGameVersions = gameVersion.m_compatibleGameVersions;

	return *this;
}

GameVersion::~GameVersion() = default;

const std::string & GameVersion::getName() const {
	return m_name;
}

void GameVersion::setName(const std::string & name) {
	m_name = Utilities::trimString(name);
}

const std::string & GameVersion::getGamePath() const {
	return m_gamePath;
}

void GameVersion::setGamePath(const std::string & gamePath) {
	m_gamePath = Utilities::trimString(gamePath);
}

const std::string & GameVersion::getGameExecutableName() const {
	return m_gameExecutableName;
}

void GameVersion::setGameExecutableName(const std::string & gameExecutableName) {
	m_gameExecutableName = Utilities::trimString(gameExecutableName);
}

bool GameVersion::hasSetupExecutableName() const {
	return m_setupExecutableName.has_value();
}

std::optional<std::string> GameVersion::getSetupExecutableName() const {
	return m_setupExecutableName;
}

void GameVersion::setSetupExecutableName(const std::string & setupExecutableName) {
	m_setupExecutableName = Utilities::trimString(setupExecutableName);
}

void GameVersion::clearSetupExecutableName() {
	m_setupExecutableName.reset();
}

bool GameVersion::hasGroupFileInstallPath() const {
	return m_groupFileInstallPath.has_value();
}

std::optional<std::string> GameVersion::getGroupFileInstallPath() const {
	return m_groupFileInstallPath;
}

void GameVersion::setGroupFileInstallPath(const std::string & GroupFileInstallPath) {
	m_groupFileInstallPath = Utilities::trimString(GroupFileInstallPath);
}

void GameVersion::clearGroupFileInstallPath() {
	m_groupFileInstallPath.reset();
}

bool GameVersion::doesRequireCombinedGroup() const {
	return m_requiresCombinedGroup.has_value() && m_requiresCombinedGroup.value();
}

std::optional<bool> GameVersion::getRequiresCombinedGroup() const {
	return m_requiresCombinedGroup;
}

void GameVersion::setRequiresCombinedGroup(bool requiresCombinedGroup) {
	m_requiresCombinedGroup = requiresCombinedGroup;
}

void GameVersion::clearRequiresCombinedGroup() {
	m_requiresCombinedGroup.reset();
}

bool GameVersion::doesRequireDOSBox() const {
	return m_requiresDOSBox.has_value() && m_requiresDOSBox.value();
}

std::optional<bool> GameVersion::getRequiresDOSBox() const {
	return m_requiresDOSBox;
}

void GameVersion::setRequiresDOSBox(bool requiresDOSBox) {
	m_requiresDOSBox = requiresDOSBox;
}

void GameVersion::clearRequiresDOSBox() {
	m_requiresDOSBox.reset();
}

bool GameVersion::hasLocalWorkingDirectory() const {
	return m_localWorkingDirectory;
}

void GameVersion::setLocalWorkingDirectory(bool localWorkingDirectory) {
	m_localWorkingDirectory = localWorkingDirectory;
}

const std::string & GameVersion::getModDirectoryName() const {
	return m_modDirectoryName;
}

void GameVersion::setModDirectoryName(const std::string & modDirectoryName) {
	m_modDirectoryName = Utilities::trimString(modDirectoryName);
}

bool GameVersion::hasRelativeConFilePath() const {
	return m_relativeConFilePath;
}

void GameVersion::setRelativeConFilePath(bool relativeConFilePath) {
	m_relativeConFilePath = relativeConFilePath;
}

bool GameVersion::doesSupportSubdirectories() const {
	return m_supportsSubdirectories;
}

void GameVersion::setSupportsSubdirectories(bool supportsSubdirectories) {
	m_supportsSubdirectories = supportsSubdirectories;
}

const std::string & GameVersion::getConFileArgumentFlag() const {
	return m_conFileArgumentFlag;
}

bool GameVersion::setConFileArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	m_conFileArgumentFlag = flag;

	return true;
}

const std::string & GameVersion::getGroupFileArgumentFlag() const {
	return m_groupFileArgumentFlag;
}

bool GameVersion::setGroupFileArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	m_groupFileArgumentFlag = flag;

	return true;
}

bool GameVersion::hasDefFileArgumentFlag() const {
	return m_defFileArgumentFlag.has_value();
}

std::optional<std::string> GameVersion::getDefFileArgumentFlag() const {
	return m_defFileArgumentFlag;
}

bool GameVersion::setDefFileArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	m_defFileArgumentFlag = flag;

	return true;
}

void GameVersion::clearDefFileArgumentFlag() {
	m_defFileArgumentFlag.reset();
}

const std::string & GameVersion::getMapFileArgumentFlag() const {
	return m_mapFileArgumentFlag;
}

bool GameVersion::setMapFileArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	m_mapFileArgumentFlag = flag;

	return true;
}

const std::string & GameVersion::getEpisodeArgumentFlag() const {
	return m_episodeArgumentFlag;
}

bool GameVersion::setEpisodeArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	m_episodeArgumentFlag = flag;

	return true;
}

const std::string & GameVersion::getLevelArgumentFlag() const {
	return m_levelArgumentFlag;
}

bool GameVersion::setLevelArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	m_levelArgumentFlag = flag;

	return true;
}

const std::string & GameVersion::getSkillArgumentFlag() const {
	return m_skillArgumentFlag;
}

bool GameVersion::setSkillArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	m_skillArgumentFlag = flag;

	return true;
}

const std::string & GameVersion::getRecordDemoArgumentFlag() const {
	return m_recordDemoArgumentFlag;
}

bool GameVersion::setRecordDemoArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	m_recordDemoArgumentFlag = flag;

	return true;
}

bool GameVersion::hasPlayDemoArgumentFlag() const {
	return m_playDemoArgumentFlag.has_value();
}

std::optional<std::string> GameVersion::getPlayDemoArgumentFlag() const {
	return m_playDemoArgumentFlag;
}

bool GameVersion::setPlayDemoArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	m_playDemoArgumentFlag = flag;

	return true;
}

void GameVersion::clearPlayDemoArgumentFlag() {
	m_playDemoArgumentFlag.reset();
}

const std::string & GameVersion::getRespawnModeArgumentFlag() const {
	return m_respawnModeArgumentFlag;
}

bool GameVersion::setRespawnModeArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	m_respawnModeArgumentFlag = flag;

	return true;
}

const std::string & GameVersion::getWeaponSwitchOrderArgumentFlag() const {
	return m_weaponSwitchOrderArgumentFlag;
}

bool GameVersion::setWeaponSwitchOrderArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	m_weaponSwitchOrderArgumentFlag = flag;

	return true;
}

const std::string & GameVersion::getDisableMonstersArgumentFlag() const {
	return m_disableMonstersArgumentFlag;
}

bool GameVersion::setDisableMonstersArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	m_disableMonstersArgumentFlag = flag;

	return true;
}

const std::string & GameVersion::getDisableSoundArgumentFlag() const {
	return m_disableSoundArgumentFlag;
}

bool GameVersion::setDisableSoundArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	m_disableSoundArgumentFlag = flag;

	return true;
}

const std::string & GameVersion::getDisableMusicArgumentFlag() const {
	return m_disableMusicArgumentFlag;
}

bool GameVersion::setDisableMusicArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	m_disableMusicArgumentFlag = flag;

	return true;
}

const std::string & GameVersion::getWebsite() const {
	return m_website;
}

void GameVersion::setWebsite(const std::string & website) {
	m_website = Utilities::trimString(website);
}

const std::string & GameVersion::getSourceCodeURL() const {
	return m_sourceCodeURL;
}

void GameVersion::setSourceCodeURL(const std::string & sourceCodeURL) {
	m_sourceCodeURL = Utilities::trimString(sourceCodeURL);
}

size_t GameVersion::numberOfSupportedOperatingSystems() const {
	return m_supportedOperatingSystems.size();
}

bool GameVersion::hasSupportedOperatingSystem(OperatingSystem operatingSystem) const {
	return std::find(std::begin(m_supportedOperatingSystems), std::end(m_supportedOperatingSystems), operatingSystem) != std::end(m_supportedOperatingSystems);
}

bool GameVersion::hasSupportedOperatingSystemWithName(const std::string & operatingSystemName) const {
	std::optional<OperatingSystem> optionalOperatingSystem(magic_enum::enum_cast<OperatingSystem>(operatingSystemName));

	if(!optionalOperatingSystem.has_value()) {
		return std::numeric_limits<size_t>::max();
	}

	return std::find(std::begin(m_supportedOperatingSystems), std::end(m_supportedOperatingSystems), optionalOperatingSystem.value()) != std::end(m_supportedOperatingSystems);
}

size_t GameVersion::indexOfSupportedOperatingSystem(OperatingSystem operatingSystem) const {
	const auto operatingSystemIterator = std::find(std::begin(m_supportedOperatingSystems), std::end(m_supportedOperatingSystems), operatingSystem);

	if(operatingSystemIterator == std::end(m_supportedOperatingSystems)) {
		return std::numeric_limits<size_t>::max();
	}

	return operatingSystemIterator - std::begin(m_supportedOperatingSystems);
}

size_t GameVersion::indexOfSupportedOperatingSystemWithName(const std::string & operatingSystemName) const {
	std::optional<OperatingSystem> optionalOperatingSystem(magic_enum::enum_cast<OperatingSystem>(operatingSystemName));

	if(!optionalOperatingSystem.has_value()) {
		return std::numeric_limits<size_t>::max();
	}

	return indexOfSupportedOperatingSystem(optionalOperatingSystem.value());
}

std::optional<GameVersion::OperatingSystem> GameVersion::getSupportedOperatingSystem(size_t index) const {
	if(index < 0 || index >= m_supportedOperatingSystems.size()) {
		return {};
	}

	return m_supportedOperatingSystems[index];
}

bool GameVersion::addSupportedOperatingSystem(OperatingSystem operatingSystem) {
	if(hasSupportedOperatingSystem(operatingSystem)) {
		return false;
	}

	m_supportedOperatingSystems.push_back(operatingSystem);

	return true;
}

bool GameVersion::addSupportedOperatingSystemWithName(const std::string & operatingSystemName) {
	std::optional<OperatingSystem> optionalOperatingSystem(magic_enum::enum_cast<OperatingSystem>(operatingSystemName));

	if(!optionalOperatingSystem.has_value()) {
		return false;
	}

	return addSupportedOperatingSystem(optionalOperatingSystem.value());
}

size_t GameVersion::addSupportedOperatingSystems(const std::vector<OperatingSystem> & operatingSystems) {
	size_t numberOfOperatingSystemsAdded = 0;

	for(OperatingSystem operatingSystem : operatingSystems) {
		if(addSupportedOperatingSystem(operatingSystem)) {
			numberOfOperatingSystemsAdded++;
		}
	}

	return numberOfOperatingSystemsAdded;
}

bool GameVersion::removeSupportedOperatingSystem(size_t index) {
	if(index < 0 || index >= m_supportedOperatingSystems.size()) {
		return false;
	}

	m_supportedOperatingSystems.erase(std::begin(m_supportedOperatingSystems) + index);

	return true;
}

bool GameVersion::removeSupportedOperatingSystem(OperatingSystem operatingSystem) {
	return removeSupportedOperatingSystem(indexOfSupportedOperatingSystem(operatingSystem));
}

bool GameVersion::removeSupportedOperatingSystemWithName(const std::string & operatingSystemName) {
	return removeSupportedOperatingSystem(indexOfSupportedOperatingSystemWithName(operatingSystemName));
}

void GameVersion::clearSupportedOperatingSystems() {
	m_supportedOperatingSystems.clear();
}

size_t GameVersion::numberOfCompatibleGameVersions() const {
	return m_compatibleGameVersions.size();
}

bool GameVersion::hasCompatibleGameVersion(const std::string & gameVersion) const {
	for(std::vector<std::string>::const_iterator i = m_compatibleGameVersions.begin(); i != m_compatibleGameVersions.end(); ++i) {
		if(*i == gameVersion) {
			return true;
		}
	}

	return false;
}

bool GameVersion::hasCompatibleGameVersion(const GameVersion & gameVersion) const {
	return hasCompatibleGameVersion(gameVersion.getName());
}

size_t GameVersion::indexOfCompatibleGameVersion(const std::string & gameVersion) const {
	for(size_t i = 0; i < m_compatibleGameVersions.size(); i++) {
		if(m_compatibleGameVersions[i] == gameVersion) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t GameVersion::indexOfCompatibleGameVersion(const GameVersion & gameVersion) const {
	return indexOfCompatibleGameVersion(gameVersion.getName());
}

std::optional<std::string> GameVersion::getCompatibleGameVersion(size_t index) const {
	if(index >= m_compatibleGameVersions.size()) {
		return {};
	}

	return m_compatibleGameVersions[index];
}

std::shared_ptr<ModGameVersion> GameVersion::getMostCompatibleModGameVersion(const std::vector<std::shared_ptr<ModGameVersion>> & modGameVersions) const {
	for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = modGameVersions.begin(); i != modGameVersions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase(m_name, (*i)->getGameVersion())) {
			return *i;
		}
	}

	for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = modGameVersions.begin(); i != modGameVersions.end(); ++i) {
		if(hasCompatibleGameVersion((*i)->getGameVersion())) {
			return *i;
		}
	}

	return nullptr;
}

std::vector<std::shared_ptr<ModGameVersion>> GameVersion::getCompatibleModGameVersions(const std::vector<std::shared_ptr<ModGameVersion>> & modGameVersions) const {
	std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions;

	for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = modGameVersions.begin(); i != modGameVersions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase(m_name, (*i)->getGameVersion())) {
			compatibleModGameVersions.insert(compatibleModGameVersions.begin(), *i);
			continue;
		}

		if(hasCompatibleGameVersion((*i)->getGameVersion())) {
			compatibleModGameVersions.push_back(*i);
		}
	}

	return compatibleModGameVersions;
}

bool GameVersion::addCompatibleGameVersion(const std::string & gameVersion) {
	if(gameVersion.empty() || hasCompatibleGameVersion(gameVersion)) {
		return false;
	}

	m_compatibleGameVersions.emplace_back(gameVersion);

	return true;
}

bool GameVersion::addCompatibleGameVersion(const GameVersion & gameVersion) {
	return addCompatibleGameVersion(gameVersion.getName());
}

size_t GameVersion::addCompatibleGameVersions(const std::vector<std::string> & gameVersions) {
	size_t numberOfCompatibleGameVersionsAdded = 0;

	for(std::vector<std::string>::const_iterator i = gameVersions.begin(); i != gameVersions.end(); ++i) {
		if(addCompatibleGameVersion(*i)) {
			numberOfCompatibleGameVersionsAdded++;
		}
	}

	return numberOfCompatibleGameVersionsAdded;
}

bool GameVersion::removeCompatibleGameVersion(size_t index) {
	if(index >= m_compatibleGameVersions.size()) {
		return false;
	}

	m_compatibleGameVersions.erase(m_compatibleGameVersions.begin() + index);

	return true;
}

bool GameVersion::removeCompatibleGameVersion(const std::string & gameVersion) {
	for(std::vector<std::string>::const_iterator i = m_compatibleGameVersions.begin(); i != m_compatibleGameVersions.end(); ++i) {
		if(*i == gameVersion) {
			return true;
		}
	}

	return false;
}

bool GameVersion::removeCompatibleGameVersion(const GameVersion & gameVersion) {
	return removeCompatibleGameVersion(gameVersion.getName());
}

void GameVersion::clearCompatibleGameVersions() {
	m_compatibleGameVersions.clear();
}

size_t GameVersion::checkForMissingExecutables() const {
	if(!isConfigured()) {
		return 0;
	}

	size_t numberOfMissingExecutables = 0;

	std::string fullGameExecutablePath(Utilities::joinPaths(m_gamePath, m_gameExecutableName));

	if(!std::filesystem::is_regular_file(std::filesystem::path(fullGameExecutablePath))) {
		numberOfMissingExecutables++;

		spdlog::error("Missing '{}' game executable: '{}'.", m_name, fullGameExecutablePath);
	}

	if(m_setupExecutableName.has_value()) {
		std::string fullSetupExecutablePath(Utilities::joinPaths(m_gamePath, m_setupExecutableName.value()));

		if(!std::filesystem::is_regular_file(std::filesystem::path(fullSetupExecutablePath))) {
			numberOfMissingExecutables++;

			spdlog::error("Missing '{}' setup executable: '{}'.", m_name, fullSetupExecutablePath);
		}
	}

	return numberOfMissingExecutables;
}

rapidjson::Value GameVersion::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value gameVersionValue(rapidjson::kObjectType);

	rapidjson::Value nameValue(m_name.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_NAME_PROPERTY_NAME), nameValue, allocator);

	rapidjson::Value gamePathValue(m_gamePath.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_GAME_PATH_PROPERTY_NAME), gamePathValue, allocator);

	rapidjson::Value gameExecutableNameValue(m_gameExecutableName.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME), gameExecutableNameValue, allocator);

	gameVersionValue.AddMember(rapidjson::StringRef(JSON_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME), rapidjson::Value(m_localWorkingDirectory), allocator);

	gameVersionValue.AddMember(rapidjson::StringRef(JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME), rapidjson::Value(m_relativeConFilePath), allocator);

	gameVersionValue.AddMember(rapidjson::StringRef(JSON_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME), rapidjson::Value(m_supportsSubdirectories), allocator);

	rapidjson::Value conFileArgumentFlagValue(m_conFileArgumentFlag.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME), conFileArgumentFlagValue, allocator);

	rapidjson::Value groupFileArgumentFlagValue(m_groupFileArgumentFlag.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME), groupFileArgumentFlagValue, allocator);

	if(m_defFileArgumentFlag.has_value()) {
		rapidjson::Value defFileArgumentFlagValue(m_defFileArgumentFlag.value().c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME), defFileArgumentFlagValue, allocator);
	}

	rapidjson::Value mapFileArgumentFlagValue(m_mapFileArgumentFlag.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_MAP_FILE_ARGUMENT_FLAG_PROPERTY_NAME), mapFileArgumentFlagValue, allocator);

	rapidjson::Value episodeArgumentFlagValue(m_episodeArgumentFlag.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_EPISODE_ARGUMENT_FLAG_PROPERTY_NAME), episodeArgumentFlagValue, allocator);

	rapidjson::Value levelArgumentFlagValue(m_levelArgumentFlag.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_LEVEL_ARGUMENT_FLAG_PROPERTY_NAME), levelArgumentFlagValue, allocator);

	rapidjson::Value skillArgumentFlagValue(m_skillArgumentFlag.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_SKILL_ARGUMENT_FLAG_PROPERTY_NAME), skillArgumentFlagValue, allocator);

	rapidjson::Value recordDemoArgumentFlagValue(m_recordDemoArgumentFlag.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_RECORD_DEMO_ARGUMENT_FLAG_PROPERTY_NAME), recordDemoArgumentFlagValue, allocator);

	if(m_playDemoArgumentFlag.has_value()) {
		rapidjson::Value playDemoArgumentFlagValue(m_playDemoArgumentFlag.value().c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_PLAY_DEMO_ARGUMENT_FLAG_PROPERTY_NAME), playDemoArgumentFlagValue, allocator);
	}

	rapidjson::Value respawnModeArgumentFlagValue(m_respawnModeArgumentFlag.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_RESPAWN_MODE_ARGUMENT_FLAG_PROPERTY_NAME), respawnModeArgumentFlagValue, allocator);

	rapidjson::Value weaponSwitchOrderArgumentFlagValue(m_weaponSwitchOrderArgumentFlag.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG_PROPERTY_NAME), weaponSwitchOrderArgumentFlagValue, allocator);

	rapidjson::Value disableMonstersArgumentFlagValue(m_disableMonstersArgumentFlag.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_DISABLE_MONSTERS_ARGUMENT_FLAG_PROPERTY_NAME), disableMonstersArgumentFlagValue, allocator);

	rapidjson::Value disableSoundArgumentFlagValue(m_disableSoundArgumentFlag.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_DISABLE_SOUND_ARGUMENT_FLAG_PROPERTY_NAME), disableSoundArgumentFlagValue, allocator);

	rapidjson::Value disableMusicArgumentFlagValue(m_disableMusicArgumentFlag.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_DISABLE_MUSIC_ARGUMENT_FLAG_PROPERTY_NAME), disableMusicArgumentFlagValue, allocator);

	if(m_setupExecutableName.has_value()) {
		rapidjson::Value setupExecutableNameValue(m_setupExecutableName.value().c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_SETUP_EXECUTABLE_NAME_PROPERTY_NAME), setupExecutableNameValue, allocator);
	}

	if(m_groupFileInstallPath.has_value()) {
		rapidjson::Value groupFileInstallPathValue(m_groupFileInstallPath.value().c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_GROUP_FILE_INSTALL_PATH_PROPERTY_NAME), groupFileInstallPathValue, allocator);
	}

	if(m_requiresCombinedGroup.has_value()) {
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_REQUIRES_COMBINED_GROUP_PROPERTY_NAME), rapidjson::Value(m_requiresCombinedGroup.value()), allocator);
	}

	if(m_requiresDOSBox.has_value()) {
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_REQUIRES_DOSBOX_PROPERTY_NAME), rapidjson::Value(m_requiresDOSBox.value()), allocator);
	}

	rapidjson::Value modDirectoryNameValue(m_modDirectoryName.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME), modDirectoryNameValue, allocator);

	if(!m_website.empty()) {
		rapidjson::Value websiteValue(m_website.c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_WEBSITE_PROPERTY_NAME), websiteValue, allocator);
	}

	if(!m_sourceCodeURL.empty()) {
		rapidjson::Value sourceCodeURLValue(m_sourceCodeURL.c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_SOURCE_CODE_URL_PROPERTY_NAME), sourceCodeURLValue, allocator);
	}

	if(!m_supportedOperatingSystems.empty()) {
		rapidjson::Value supportedOperatingSystemsValue(rapidjson::kArrayType);

		for(std::vector<OperatingSystem>::const_iterator i = m_supportedOperatingSystems.begin(); i != m_supportedOperatingSystems.end(); ++i) {
			rapidjson::Value supportedOperatingSystemValue(std::string(magic_enum::enum_name(*i)).c_str(), allocator);
			supportedOperatingSystemsValue.PushBack(supportedOperatingSystemValue, allocator);
		}

		gameVersionValue.AddMember(rapidjson::StringRef(JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME), supportedOperatingSystemsValue, allocator);
	}

	if(!m_compatibleGameVersions.empty()) {
		rapidjson::Value compatibleGameVersionsValue(rapidjson::kArrayType);

		for(std::vector<std::string>::const_iterator i = m_compatibleGameVersions.begin(); i != m_compatibleGameVersions.end(); ++i) {
			rapidjson::Value compatibleGameVersionValue((*i).c_str(), allocator);
			compatibleGameVersionsValue.PushBack(compatibleGameVersionValue, allocator);
		}

		gameVersionValue.AddMember(rapidjson::StringRef(JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME), compatibleGameVersionsValue, allocator);
	}

	return gameVersionValue;
}

std::unique_ptr<GameVersion> GameVersion::parseFrom(const rapidjson::Value & gameVersionValue) {
	if(!gameVersionValue.IsObject()) {
		spdlog::error("Invalid game version type: '{}', expected 'object'.", Utilities::typeToString(gameVersionValue.GetType()));
		return nullptr;
	}

	// check for unhandled game version properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = gameVersionValue.MemberBegin(); i != gameVersionValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::error("Game version has unexpected property '{}'.", i->name.GetString());
			return nullptr;
		}
	}

	// parse game version name
	if(!gameVersionValue.HasMember(JSON_NAME_PROPERTY_NAME)) {
		spdlog::error("Game version is missing '{}' property.", JSON_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & nameValue = gameVersionValue[JSON_NAME_PROPERTY_NAME];

	if(!nameValue.IsString()) {
		spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_NAME_PROPERTY_NAME, Utilities::typeToString(nameValue.GetType()));
		return nullptr;
	}

	std::string name(Utilities::trimString(nameValue.GetString()));

	if(name.empty()) {
		spdlog::error("Game version '{}' property cannot be empty.", JSON_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version game path
	if(!gameVersionValue.HasMember(JSON_GAME_PATH_PROPERTY_NAME)) {
		spdlog::error("Game version is missing '{}' property.", JSON_GAME_PATH_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & gamePathValue = gameVersionValue[JSON_GAME_PATH_PROPERTY_NAME];

	if(!gamePathValue.IsString()) {
		spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_GAME_PATH_PROPERTY_NAME, Utilities::typeToString(gamePathValue.GetType()));
		return nullptr;
	}

	std::string gamePath(Utilities::trimString(gamePathValue.GetString()));

	// parse game version game executable name
	if(!gameVersionValue.HasMember(JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME)) {
		spdlog::error("Game version is missing '{}' property.", JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & gameExecutableNameValue = gameVersionValue[JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME];

	if(!gameExecutableNameValue.IsString()) {
		spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME, Utilities::typeToString(gameExecutableNameValue.GetType()));
		return nullptr;
	}

	std::string gameExecutableName(Utilities::trimString(gameExecutableNameValue.GetString()));

	if(gameExecutableName.empty()) {
		spdlog::error("Game version '{}' property cannot be empty.", JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version setup executable name
	std::optional<std::string> setupExecutableNameOptional;

	if(gameVersionValue.HasMember(JSON_SETUP_EXECUTABLE_NAME_PROPERTY_NAME)) {
		const rapidjson::Value & setupExecutableNameValue = gameVersionValue[JSON_SETUP_EXECUTABLE_NAME_PROPERTY_NAME];

		if(!setupExecutableNameValue.IsString()) {
			spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_SETUP_EXECUTABLE_NAME_PROPERTY_NAME, Utilities::typeToString(setupExecutableNameValue.GetType()));
			return nullptr;
		}

		setupExecutableNameOptional = Utilities::trimString(setupExecutableNameValue.GetString());

		if(setupExecutableNameOptional.value().empty()) {
			spdlog::error("Game version '{}' property cannot be empty.", JSON_SETUP_EXECUTABLE_NAME_PROPERTY_NAME);
			return nullptr;
		}
	}

	// parse group file install path
	std::optional<std::string> groupFileInstallPathOptional;

	if(gameVersionValue.HasMember(JSON_GROUP_FILE_INSTALL_PATH_PROPERTY_NAME)) {
		const rapidjson::Value & groupFileInstallPathValue = gameVersionValue[JSON_GROUP_FILE_INSTALL_PATH_PROPERTY_NAME];

		if(!groupFileInstallPathValue.IsString()) {
			spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_GROUP_FILE_INSTALL_PATH_PROPERTY_NAME, Utilities::typeToString(groupFileInstallPathValue.GetType()));
			return nullptr;
		}

		groupFileInstallPathOptional = Utilities::trimString(groupFileInstallPathValue.GetString());
	}

	// parse local working directory option
	if(!gameVersionValue.HasMember(JSON_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME)) {
		spdlog::error("Game version is missing '{}' property.", JSON_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & localWorkingDirectoryValue = gameVersionValue[JSON_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME];

	if(!localWorkingDirectoryValue.IsBool()) {
		spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'boolean'.", JSON_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME, Utilities::typeToString(localWorkingDirectoryValue.GetType()));
		return nullptr;
	}

	bool localWorkingDirectory = localWorkingDirectoryValue.GetBool();

	// parse relative con file path option
	if(!gameVersionValue.HasMember(JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME)) {
		spdlog::error("Game version is missing '{}' property.", JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & relativeConFilePathValue = gameVersionValue[JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME];

	if(!relativeConFilePathValue.IsBool()) {
		spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'boolean'.", JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME, Utilities::typeToString(relativeConFilePathValue.GetType()));
		return nullptr;
	}

	bool relativeConFilePath = relativeConFilePathValue.GetBool();

	// parse supports subdirectories links option
	if(!gameVersionValue.HasMember(JSON_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME)) {
		spdlog::error("Game version is missing '{}' property.", JSON_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & supportsSubdirectoriesValue = gameVersionValue[JSON_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME];

	if(!supportsSubdirectoriesValue.IsBool()) {
		spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'boolean'.", JSON_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME, Utilities::typeToString(supportsSubdirectoriesValue.GetType()));
		return nullptr;
	}

	bool supportsSubdirectories = supportsSubdirectoriesValue.GetBool();

	// parse game version con file argument flag
	if(!gameVersionValue.HasMember(JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		spdlog::error("Game version is missing '{}' property.", JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & conFileArgumentFlagValue = gameVersionValue[JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME];

	if(!conFileArgumentFlagValue.IsString()) {
		spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(conFileArgumentFlagValue.GetType()));
		return nullptr;
	}

	std::string conFileArgumentFlag(conFileArgumentFlagValue.GetString());

	if(conFileArgumentFlag.empty()) {
		spdlog::error("Game version '{}' property cannot be empty.", JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version group file argument flag
	if(!gameVersionValue.HasMember(JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		spdlog::error("Game version is missing '{}' property.", JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & groupFileArgumentFlagValue = gameVersionValue[JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME];

	if(!groupFileArgumentFlagValue.IsString()) {
		spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(groupFileArgumentFlagValue.GetType()));
		return nullptr;
	}

	std::string groupFileArgumentFlag(groupFileArgumentFlagValue.GetString());

	if(groupFileArgumentFlag.empty()) {
		spdlog::error("Game version '{}' property cannot be empty.", JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version map file argument flag
	if(!gameVersionValue.HasMember(JSON_MAP_FILE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		spdlog::error("Game version is missing '{}' property.", JSON_MAP_FILE_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & mapFileArgumentFlagValue = gameVersionValue[JSON_MAP_FILE_ARGUMENT_FLAG_PROPERTY_NAME];

	if(!mapFileArgumentFlagValue.IsString()) {
		spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_MAP_FILE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(mapFileArgumentFlagValue.GetType()));
		return nullptr;
	}

	std::string mapFileArgumentFlag(mapFileArgumentFlagValue.GetString());

	if(mapFileArgumentFlag.empty()) {
		spdlog::error("Game version '{}' property cannot be empty.", JSON_MAP_FILE_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version episode argument flag
	if(!gameVersionValue.HasMember(JSON_EPISODE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		spdlog::error("Game version is missing '{}' property.", JSON_EPISODE_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & episodeArgumentFlagValue = gameVersionValue[JSON_EPISODE_ARGUMENT_FLAG_PROPERTY_NAME];

	if(!episodeArgumentFlagValue.IsString()) {
		spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_EPISODE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(episodeArgumentFlagValue.GetType()));
		return nullptr;
	}

	std::string episodeArgumentFlag(episodeArgumentFlagValue.GetString());

	if(episodeArgumentFlag.empty()) {
		spdlog::error("Game version '{}' property cannot be empty.", JSON_EPISODE_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version level argument flag
	if(!gameVersionValue.HasMember(JSON_LEVEL_ARGUMENT_FLAG_PROPERTY_NAME)) {
		spdlog::error("Game version is missing '{}' property.", JSON_LEVEL_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & levelArgumentFlagValue = gameVersionValue[JSON_LEVEL_ARGUMENT_FLAG_PROPERTY_NAME];

	if(!levelArgumentFlagValue.IsString()) {
		spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_LEVEL_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(levelArgumentFlagValue.GetType()));
		return nullptr;
	}

	std::string levelArgumentFlag(levelArgumentFlagValue.GetString());

	if(levelArgumentFlag.empty()) {
		spdlog::error("Game version '{}' property cannot be empty.", JSON_LEVEL_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version skill argument flag
	if(!gameVersionValue.HasMember(JSON_SKILL_ARGUMENT_FLAG_PROPERTY_NAME)) {
		spdlog::error("Game version is missing '{}' property.", JSON_SKILL_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & skillArgumentFlagValue = gameVersionValue[JSON_SKILL_ARGUMENT_FLAG_PROPERTY_NAME];

	if(!skillArgumentFlagValue.IsString()) {
		spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_SKILL_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(skillArgumentFlagValue.GetType()));
		return nullptr;
	}

	std::string skillArgumentFlag(skillArgumentFlagValue.GetString());

	if(skillArgumentFlag.empty()) {
		spdlog::error("Game version '{}' property cannot be empty.", JSON_SKILL_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version record demo argument flag
	if(!gameVersionValue.HasMember(JSON_RECORD_DEMO_ARGUMENT_FLAG_PROPERTY_NAME)) {
		spdlog::error("Game version is missing '{}' property.", JSON_RECORD_DEMO_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & recordDemoArgumentFlagValue = gameVersionValue[JSON_RECORD_DEMO_ARGUMENT_FLAG_PROPERTY_NAME];

	if(!recordDemoArgumentFlagValue.IsString()) {
		spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_RECORD_DEMO_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(recordDemoArgumentFlagValue.GetType()));
		return nullptr;
	}

	std::string recordDemoArgumentFlag(recordDemoArgumentFlagValue.GetString());

	if(recordDemoArgumentFlag.empty()) {
		spdlog::error("Game version '{}' property cannot be empty.", JSON_RECORD_DEMO_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version play demo argument flag
	std::optional<std::string> optionalPlayDemoArgumentFlag;

	if(gameVersionValue.HasMember(JSON_PLAY_DEMO_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & playDemoArgumentFlagValue = gameVersionValue[JSON_PLAY_DEMO_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!playDemoArgumentFlagValue.IsString()) {
			spdlog::error("Mod file '{}' property has invalid type: '{}', expected 'string'.", JSON_PLAY_DEMO_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(playDemoArgumentFlagValue.GetType()));
			return nullptr;
		}

		optionalPlayDemoArgumentFlag = playDemoArgumentFlagValue.GetString();
	}

	// parse game version respawn mode argument flag
	if(!gameVersionValue.HasMember(JSON_RESPAWN_MODE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		spdlog::error("Game version is missing '{}' property.", JSON_RESPAWN_MODE_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & respawnModeArgumentFlagValue = gameVersionValue[JSON_RESPAWN_MODE_ARGUMENT_FLAG_PROPERTY_NAME];

	if(!respawnModeArgumentFlagValue.IsString()) {
		spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_RESPAWN_MODE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(respawnModeArgumentFlagValue.GetType()));
		return nullptr;
	}

	std::string respawnModeArgumentFlag(respawnModeArgumentFlagValue.GetString());

	if(respawnModeArgumentFlag.empty()) {
		spdlog::error("Game version '{}' property cannot be empty.", JSON_RESPAWN_MODE_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version weapon switch order argument flag
	if(!gameVersionValue.HasMember(JSON_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG_PROPERTY_NAME)) {
		spdlog::error("Game version is missing '{}' property.", JSON_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & weaponSwitchOrderArgumentFlagValue = gameVersionValue[JSON_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG_PROPERTY_NAME];

	if(!weaponSwitchOrderArgumentFlagValue.IsString()) {
		spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(weaponSwitchOrderArgumentFlagValue.GetType()));
		return nullptr;
	}

	std::string weaponSwitchOrderArgumentFlag(weaponSwitchOrderArgumentFlagValue.GetString());

	if(weaponSwitchOrderArgumentFlag.empty()) {
		spdlog::error("Game version '{}' property cannot be empty.", JSON_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version disable monsters argument flag
	if(!gameVersionValue.HasMember(JSON_DISABLE_MONSTERS_ARGUMENT_FLAG_PROPERTY_NAME)) {
		spdlog::error("Game version is missing '{}' property.", JSON_DISABLE_MONSTERS_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & disableMonstersArgumentFlagValue = gameVersionValue[JSON_DISABLE_MONSTERS_ARGUMENT_FLAG_PROPERTY_NAME];

	if(!disableMonstersArgumentFlagValue.IsString()) {
		spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_DISABLE_MONSTERS_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(disableMonstersArgumentFlagValue.GetType()));
		return nullptr;
	}

	std::string disableMonstersArgumentFlag(disableMonstersArgumentFlagValue.GetString());

	if(disableMonstersArgumentFlag.empty()) {
		spdlog::error("Game version '{}' property cannot be empty.", JSON_DISABLE_MONSTERS_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version disable sound argument flag
	if(!gameVersionValue.HasMember(JSON_DISABLE_SOUND_ARGUMENT_FLAG_PROPERTY_NAME)) {
		spdlog::error("Game version is missing '{}' property.", JSON_DISABLE_SOUND_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & disableSoundArgumentFlagValue = gameVersionValue[JSON_DISABLE_SOUND_ARGUMENT_FLAG_PROPERTY_NAME];

	if(!disableSoundArgumentFlagValue.IsString()) {
		spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_DISABLE_SOUND_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(disableSoundArgumentFlagValue.GetType()));
		return nullptr;
	}

	std::string disableSoundArgumentFlag(disableSoundArgumentFlagValue.GetString());

	if(disableSoundArgumentFlag.empty()) {
		spdlog::error("Game version '{}' property cannot be empty.", JSON_DISABLE_SOUND_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version disable music argument flag
	if(!gameVersionValue.HasMember(JSON_DISABLE_MUSIC_ARGUMENT_FLAG_PROPERTY_NAME)) {
		spdlog::error("Game version is missing '{}' property.", JSON_DISABLE_MUSIC_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & disableMusicArgumentFlagValue = gameVersionValue[JSON_DISABLE_MUSIC_ARGUMENT_FLAG_PROPERTY_NAME];

	if(!disableMusicArgumentFlagValue.IsString()) {
		spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_DISABLE_MUSIC_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(disableMusicArgumentFlagValue.GetType()));
		return nullptr;
	}

	std::string disableMusicArgumentFlag(disableMusicArgumentFlagValue.GetString());

	if(disableMusicArgumentFlag.empty()) {
		spdlog::error("Game version '{}' property cannot be empty.", JSON_DISABLE_MUSIC_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version mod directory name
	if(!gameVersionValue.HasMember(JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME)) {
		spdlog::error("Game version is missing '{}' property.", JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modDirectoryNameValue = gameVersionValue[JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME];

	if(!modDirectoryNameValue.IsString()) {
		spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME, Utilities::typeToString(modDirectoryNameValue.GetType()));
		return nullptr;
	}

	std::string modDirectoryName(Utilities::trimString(modDirectoryNameValue.GetString()));

	if(modDirectoryName.empty()) {
		spdlog::error("Game version '{}' property cannot be empty.", JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// initialize the game version
	std::unique_ptr<GameVersion> newGameVersion = std::make_unique<GameVersion>(name, gamePath, gameExecutableName, localWorkingDirectory, relativeConFilePath, supportsSubdirectories, conFileArgumentFlag, groupFileArgumentFlag, mapFileArgumentFlag, episodeArgumentFlag, levelArgumentFlag, skillArgumentFlag, recordDemoArgumentFlag, optionalPlayDemoArgumentFlag, respawnModeArgumentFlag, weaponSwitchOrderArgumentFlag, disableMonstersArgumentFlag, disableSoundArgumentFlag, disableMusicArgumentFlag, modDirectoryName, setupExecutableNameOptional, groupFileInstallPathOptional);

	// parse game version def file argument flag
	if(gameVersionValue.HasMember(JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & defFileArgumentFlagValue = gameVersionValue[JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!defFileArgumentFlagValue.IsString()) {
			spdlog::error("Mod file '{}' property has invalid type: '{}', expected 'string'.", JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(defFileArgumentFlagValue.GetType()));
			return nullptr;
		}

		newGameVersion->setDefFileArgumentFlag(defFileArgumentFlagValue.GetString());
	}

	// parse the game version requires full group property
	if(gameVersionValue.HasMember(JSON_REQUIRES_COMBINED_GROUP_PROPERTY_NAME)) {
		const rapidjson::Value & requiresCombinedGroupValue = gameVersionValue[JSON_REQUIRES_COMBINED_GROUP_PROPERTY_NAME];

		if(!requiresCombinedGroupValue.IsBool()) {
			spdlog::error("Game version '{}' property has invalid type: '{}', expected 'boolean'.", JSON_REQUIRES_COMBINED_GROUP_PROPERTY_NAME, Utilities::typeToString(requiresCombinedGroupValue.GetType()));
			return nullptr;
		}

		newGameVersion->setRequiresCombinedGroup(requiresCombinedGroupValue.GetBool());
	}

	// parse the game version requires dosbox property
	if(gameVersionValue.HasMember(JSON_REQUIRES_DOSBOX_PROPERTY_NAME)) {
		const rapidjson::Value & requiresDOSBoxValue = gameVersionValue[JSON_REQUIRES_DOSBOX_PROPERTY_NAME];

		if(!requiresDOSBoxValue.IsBool()) {
			spdlog::error("Game version '{}' property has invalid type: '{}', expected 'boolean'.", JSON_REQUIRES_DOSBOX_PROPERTY_NAME, Utilities::typeToString(requiresDOSBoxValue.GetType()));
			return nullptr;
		}

		newGameVersion->setRequiresDOSBox(requiresDOSBoxValue.GetBool());
	}

	// parse the game version website property
	if(gameVersionValue.HasMember(JSON_WEBSITE_PROPERTY_NAME)) {
		const rapidjson::Value & websiteValue = gameVersionValue[JSON_WEBSITE_PROPERTY_NAME];

		if(!websiteValue.IsString()) {
			spdlog::error("Mod file '{}' property has invalid type: '{}', expected 'string'.", JSON_WEBSITE_PROPERTY_NAME, Utilities::typeToString(websiteValue.GetType()));
			return nullptr;
		}

		newGameVersion->setWebsite(websiteValue.GetString());
	}

	// parse the game version source code url property
	if(gameVersionValue.HasMember(JSON_SOURCE_CODE_URL_PROPERTY_NAME)) {
		const rapidjson::Value & sourceCodeURLValue = gameVersionValue[JSON_SOURCE_CODE_URL_PROPERTY_NAME];

		if(!sourceCodeURLValue.IsString()) {
			spdlog::error("Mod file '{}' property has invalid type: '{}', expected 'string'.", JSON_SOURCE_CODE_URL_PROPERTY_NAME, Utilities::typeToString(sourceCodeURLValue.GetType()));
			return nullptr;
		}

		newGameVersion->setSourceCodeURL(sourceCodeURLValue.GetString());
	}

	// parse the supported operating systems property
	if(gameVersionValue.HasMember(JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME)) {
		const rapidjson::Value & supportedOperatingSystemsValue = gameVersionValue[JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME];

		if(!supportedOperatingSystemsValue.IsArray()) {
			spdlog::error("Game version '{}' property has invalid type: '{}', expected 'array'.", JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME, Utilities::typeToString(supportedOperatingSystemsValue.GetType()));
			return nullptr;
		}

		for(rapidjson::Value::ConstValueIterator i = supportedOperatingSystemsValue.Begin(); i != supportedOperatingSystemsValue.End(); ++i) {
			if(!i->IsString()) {
				spdlog::error("Game version '{}' property contains invalid supported operating system type: '{}', expected 'string'.", JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME, Utilities::typeToString(i->GetType()));
				return nullptr;
			}

			std::string supportedOperatingSystemName(Utilities::trimString(i->GetString()));

			std::optional<OperatingSystem> optionalSupportedOperatingSystem(magic_enum::enum_cast<OperatingSystem>(supportedOperatingSystemName));

			if(!optionalSupportedOperatingSystem.has_value()) {
				std::stringstream validOperatingSystems;

				for(const auto operatingSystem : magic_enum::enum_names<OperatingSystem>()) {
					if(validOperatingSystems.tellp() != 0) {
						validOperatingSystems << ", ";
					}

					validOperatingSystems << "'" << operatingSystem << "'";
				}

				spdlog::error("Game version '{}' property contains invalid supported operating system value: '{}', expected one of: {}.", JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME, supportedOperatingSystemName, validOperatingSystems.str());
				return nullptr;
			}

			if(newGameVersion->hasSupportedOperatingSystem(optionalSupportedOperatingSystem.value())) {
				spdlog::warn("Game version '{}' property contains duplicate supported operating system: '{}'.", JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME, supportedOperatingSystemName);
				continue;
			}

			newGameVersion->m_supportedOperatingSystems.push_back(optionalSupportedOperatingSystem.value());
		}
	}

	// parse the compatible game versions property
	if(gameVersionValue.HasMember(JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME)) {
		const rapidjson::Value & compatibleGameVersionsValue = gameVersionValue[JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME];

		if(!compatibleGameVersionsValue.IsArray()) {
			spdlog::error("Game version '{}' property has invalid type: '{}', expected 'array'.", JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME, Utilities::typeToString(compatibleGameVersionsValue.GetType()));
			return nullptr;
		}

		for(rapidjson::Value::ConstValueIterator i = compatibleGameVersionsValue.Begin(); i != compatibleGameVersionsValue.End(); ++i) {
			if(!i->IsString()) {
				spdlog::error("Game version '{}' property contains invalid compatible game version type: '{}', expected 'string'.", JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME, Utilities::typeToString(i->GetType()));
				return nullptr;
			}

			std::string compatibleGameVersion(Utilities::trimString(i->GetString()));

			if(newGameVersion->hasCompatibleGameVersion(compatibleGameVersion)) {
				spdlog::warn("Game version '{}' property contains duplicate compatible game version: '{}'.", JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME, compatibleGameVersion);
				continue;
			}

			newGameVersion->m_compatibleGameVersions.push_back(compatibleGameVersion);
		}
	}

	return newGameVersion;
}

std::optional<GameVersion::OperatingSystem> GameVersion::convertOperatingSystemType(DeviceInformationBridge::OperatingSystemType operatingSystemType) {
	switch(operatingSystemType) {
		case DeviceInformationBridge::OperatingSystemType::Windows: {
			return OperatingSystem::Windows;
		}
		case DeviceInformationBridge::OperatingSystemType::Linux: {
			return OperatingSystem::Linux;
		}
		case DeviceInformationBridge::OperatingSystemType::MacOS: {
			return OperatingSystem::MacOS;
		}
	}

	return {};
}

bool GameVersion::isConfigured() const {
	return isValid() &&
		   !m_gamePath.empty();
}

bool GameVersion::isValid() const {
	if(m_name.empty() ||
	   m_gameExecutableName.empty() ||
	   m_modDirectoryName.empty() ||
	   m_conFileArgumentFlag.empty() ||
	   m_groupFileArgumentFlag.empty() ||
	   m_mapFileArgumentFlag.empty() ||
	   m_episodeArgumentFlag.empty() ||
	   m_levelArgumentFlag.empty() ||
	   m_skillArgumentFlag.empty() ||
	   m_recordDemoArgumentFlag.empty() ||
	   m_respawnModeArgumentFlag.empty() ||
	   m_weaponSwitchOrderArgumentFlag.empty() ||
	   m_disableMonstersArgumentFlag.empty() ||
	   m_disableSoundArgumentFlag.empty() ||
	   m_disableMusicArgumentFlag.empty() ||
	   m_supportedOperatingSystems.empty()) {
		return false;
	}

	if(m_defFileArgumentFlag.has_value() && m_defFileArgumentFlag.value().empty()) {
		return false;
	}

	return true;
}

bool GameVersion::isValid(const GameVersion * gameVersion) {
	return gameVersion != nullptr && gameVersion->isValid();
}

bool GameVersion::operator == (const GameVersion & gameVersion) const {
	if(m_requiresDOSBox != gameVersion.m_requiresDOSBox ||
	   m_requiresCombinedGroup != gameVersion.m_requiresCombinedGroup ||
	   m_localWorkingDirectory != gameVersion.m_localWorkingDirectory ||
	   m_relativeConFilePath != gameVersion.m_relativeConFilePath ||
	   m_supportsSubdirectories != gameVersion.m_supportsSubdirectories ||
	   !Utilities::areStringsEqualIgnoreCase(m_name, gameVersion.m_name) ||
	   m_gamePath != gameVersion.m_gamePath ||
	   m_gameExecutableName != gameVersion.m_gameExecutableName ||
	   m_setupExecutableName != gameVersion.m_setupExecutableName ||
	   m_groupFileInstallPath != gameVersion.m_groupFileInstallPath ||
	   m_modDirectoryName != gameVersion.m_modDirectoryName ||
	   m_website != gameVersion.m_website ||
	   m_sourceCodeURL != gameVersion.m_sourceCodeURL ||
	   m_conFileArgumentFlag != gameVersion.m_conFileArgumentFlag ||
	   m_groupFileArgumentFlag != gameVersion.m_groupFileArgumentFlag ||
	   m_defFileArgumentFlag != gameVersion.m_defFileArgumentFlag ||
	   m_mapFileArgumentFlag != gameVersion.m_mapFileArgumentFlag ||
	   m_episodeArgumentFlag != gameVersion.m_episodeArgumentFlag ||
	   m_levelArgumentFlag != gameVersion.m_levelArgumentFlag ||
	   m_skillArgumentFlag != gameVersion.m_skillArgumentFlag ||
	   m_recordDemoArgumentFlag != gameVersion.m_recordDemoArgumentFlag ||
	   m_playDemoArgumentFlag != gameVersion.m_playDemoArgumentFlag ||
	   m_respawnModeArgumentFlag != gameVersion.m_respawnModeArgumentFlag ||
	   m_weaponSwitchOrderArgumentFlag != gameVersion.m_weaponSwitchOrderArgumentFlag ||
	   m_disableMonstersArgumentFlag != gameVersion.m_disableMonstersArgumentFlag ||
	   m_disableSoundArgumentFlag != gameVersion.m_disableSoundArgumentFlag ||
	   m_disableMusicArgumentFlag != gameVersion.m_disableMusicArgumentFlag ||
	   m_compatibleGameVersions.size() != gameVersion.m_compatibleGameVersions.size() ||
	   m_supportedOperatingSystems != gameVersion.m_supportedOperatingSystems) {
		return false;
	}

	for(size_t i = 0; i < m_compatibleGameVersions.size(); i++) {
		if(!Utilities::areStringsEqualIgnoreCase(m_compatibleGameVersions[i], gameVersion.m_compatibleGameVersions[i])) {
			return false;
		}
	}

	return true;
}

bool GameVersion::operator != (const GameVersion & gameVersion) const {
	return !operator == (gameVersion);
}
