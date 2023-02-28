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
static constexpr const char * JSON_REMOVABLE_PROPERTY_NAME = "removable";
static constexpr const char * JSON_RENAMABLE_PROPERTY_NAME = "renamable";
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
static constexpr const char * JSON_SKILL_START_VALUE_PROPERTY_NAME = "skillValueOffset";
static constexpr const char * JSON_RECORD_DEMO_ARGUMENT_FLAG_PROPERTY_NAME = "recordDemoArgumentFlag";
static constexpr const char * JSON_PLAY_DEMO_ARGUMENT_FLAG_PROPERTY_NAME = "playDemoArgumentFlag";
static constexpr const char * JSON_RESPAWN_MODE_ARGUMENT_FLAG_PROPERTY_NAME = "respawnModeArgumentFlag";
static constexpr const char * JSON_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG_PROPERTY_NAME = "weaponSwitchOrderArgumentFlag";
static constexpr const char * JSON_DISABLE_MONSTERS_ARGUMENT_FLAG_PROPERTY_NAME = "disableMonstersArgumentFlag";
static constexpr const char * JSON_DISABLE_SOUND_ARGUMENT_FLAG_PROPERTY_NAME = "disableSoundArgumentFlag";
static constexpr const char * JSON_DISABLE_MUSIC_ARGUMENT_FLAG_PROPERTY_NAME = "disableMusicArgumentFlag";
static constexpr const char * JSON_REQUIRES_COMBINED_GROUP_PROPERTY_NAME = "requiresCombinedGroup";
static constexpr const char * JSON_REQUIRES_GROUP_FILE_EXTRACTION_PROPERTY_NAME = "requiresGroupFileExtraction";
static constexpr const char * JSON_REQUIRES_DOSBOX_PROPERTY_NAME = "requiresDOSBox";
static constexpr const char * JSON_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME = "localWorkingDirectory";
static constexpr const char * JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME = "modDirectoryName";
static constexpr const char * JSON_WEBSITE_PROPERTY_NAME = "website";
static constexpr const char * JSON_SOURCE_CODE_URL_PROPERTY_NAME = "sourceCodeURL";
static constexpr const char * JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME = "supportedOperatingSystems";
static constexpr const char * JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME = "compatibleGameVersions";
static const std::array<std::string_view, 33> JSON_PROPERTY_NAMES = {
	JSON_NAME_PROPERTY_NAME,
	JSON_REMOVABLE_PROPERTY_NAME,
	JSON_RENAMABLE_PROPERTY_NAME,
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
	JSON_SKILL_START_VALUE_PROPERTY_NAME,
	JSON_RECORD_DEMO_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_PLAY_DEMO_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_RESPAWN_MODE_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_DISABLE_MONSTERS_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_DISABLE_SOUND_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_DISABLE_MUSIC_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_REQUIRES_COMBINED_GROUP_PROPERTY_NAME,
	JSON_REQUIRES_GROUP_FILE_EXTRACTION_PROPERTY_NAME,
	JSON_REQUIRES_DOSBOX_PROPERTY_NAME,
	JSON_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME,
	JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME,
	JSON_WEBSITE_PROPERTY_NAME,
	JSON_SOURCE_CODE_URL_PROPERTY_NAME,
	JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME,
	JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME
};

const std::string GameVersion::ALL_VERSIONS = "All Versions";

const GameVersion GameVersion::LAMEDUKE                ("Beta 1.3.95 (LameDuke)",      false, false, "", "D3D.EXE",            true,  false, false, "LameDuke",  {},    {},    {},      "/v", "/l", "/s", 0, "/r", {},    {},   {},   {},   {},    {},    "SETUP.EXE", {}, {},    {},    true, true,  "https://www.dukenukem.com",                                     "",                                                                         { OperatingSystem::DOS });
const GameVersion GameVersion::ORIGINAL_REGULAR_VERSION("Regular Version 1.3",         false, false, "", "DUKE3D.EXE",         true,  false, true,  "Regular",   "/x",  "/g",  "-map ", "/v", "/l", "/s", 1, "/r", {},    "/t", "/u", "/m", "/ns", "/nm", "SETUP.EXE", "", {},    true,  {},   true,  "https://www.dukenukem.com",                                     "",                                                                         { OperatingSystem::DOS });
const GameVersion GameVersion::ORIGINAL_ATOMIC_EDITION ("Atomic Edition 1.4/1.5",      false, false, "", "DUKE3D.EXE",         true,  false, true,  "Atomic",    "/x",  "/g",  "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", "SETUP.EXE", "", {},    {},    {},   true,  "https://www.dukenukem.com",                                     "",                                                                         { OperatingSystem::DOS });
const GameVersion GameVersion::JFDUKE3D                ("JFDuke3D",                    false, false, "", "duke3d.exe",         true,  false, true,  "JFDuke3D",  "/x",  "/g",  "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", {},          {}, {},    {},    {},   {},    "http://www.jonof.id.au/jfduke3d",                               "https://github.com/jonof/jfduke3d",                                        { OperatingSystem::Windows, OperatingSystem::MacOS },                         { ORIGINAL_REGULAR_VERSION.getName(), ORIGINAL_ATOMIC_EDITION.getName() });
const GameVersion GameVersion::EDUKE32                 ("eDuke32",                     false, false, "", "eduke32.exe",        false, true,  true,  "eDuke32",   "-x ", "-g ", "-map ", "-v", "-l", "-s", 1, "-r", "-d ", "-t", "-u", "-m", "-ns", "-nm", {},          {}, "-h ", {},    {},   {},    "https://www.eduke32.com",                                       "https://voidpoint.io/terminx/eduke32",                                     { OperatingSystem::Windows },                                                 { ORIGINAL_REGULAR_VERSION.getName(), ORIGINAL_ATOMIC_EDITION.getName(), JFDUKE3D.getName() });
const GameVersion GameVersion::NETDUKE32               ("NetDuke32",                   true,  false, "", "netduke32.exe",      false, true,  true,  "NetDuke",   "-x ", "-g ", "-map ", "-v", "-l", "-s", 1, "-r", "-d ", "-t", "-u", "-m", "-ns", "-nm", {},          {}, "-h ", {},    {},   {},    "https://wiki.eduke32.com/wiki/NetDuke32",                       "https://voidpoint.io/StrikerTheHedgefox/eduke32-csrefactor/-/tree/master", { OperatingSystem::Windows },                                                 { ORIGINAL_REGULAR_VERSION.getName(), ORIGINAL_ATOMIC_EDITION.getName(), JFDUKE3D.getName(), EDUKE32.getName() });
//const GameVersion GameVersion::MEGATON_EDITION         ("Megaton Edition",             true,  false, "", "duke3d.exe",         true,  false, true,  "Megaton",   "/x",  "/g",  "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", {},          {}, {},    {},    {},   {},    "https://store.steampowered.com/app/225140",                     "https://github.com/TermiT/duke3d-megaton",                                 { OperatingSystem::Windows },                                                 { ORIGINAL_REGULAR_VERSION.getName(), ORIGINAL_ATOMIC_EDITION.getName(), JFDUKE3D.getName() });
//const GameVersion GameVersion::WORLD_TOUR              ("20th Anniversary World Tour", true,  false, "", "duke3d.exe",         true,  false, true,  "WorldTour", "/x",  "/g",  "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", {},          {}, {},    {},    {},   {},    "https://www.gearboxsoftware.com/game/duke-3d-20th",             "",                                                                         { OperatingSystem::Windows },                                                 { ORIGINAL_REGULAR_VERSION.getName(), ORIGINAL_ATOMIC_EDITION.getName() });
//const GameVersion GameVersion::BUILDGDX                ("BuildGDX",                    true,  false, "", "BuildGDX.jar",       true,  true,  true,  "BuildGDX",  "",    "",    "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", {},          {}, {},    {},    {},   {},    "https://m210.duke4.net/index.php/downloads/category/8-java",    "https://gitlab.com/m210/BuildGDX",                                         { OperatingSystem::Windows, OperatingSystem::Linux, OperatingSystem::MacOS }, { ORIGINAL_REGULAR_VERSION.getName(), ORIGINAL_ATOMIC_EDITION.getName() });
const GameVersion GameVersion::RAZE                    ("Raze",                        true,  false, "", "raze.exe",           true,  true,  true,  "Raze",      "-x ", "-g ", "-map ", "-v", "-l", "-s", 1, "-r", "-d ", "-t", "-u", "-m", "-ns", "-nm", {},          {}, {},    {},    {},   {},    "https://raze.zdoom.org/about",                                  "https://github.com/coelckers/Raze",                                        { OperatingSystem::Windows, OperatingSystem::Linux, OperatingSystem::MacOS }, { ORIGINAL_ATOMIC_EDITION.getName(),  JFDUKE3D.getName() });
const GameVersion GameVersion::RED_NUKEM               ("RedNukem",                    true,  false, "", "rednukem.exe",       false, true,  true,  "RedNukem",  "-x ", "-g ", "-map ", "-v", "-l", "-s", 1, "-r", "-d ", "-t", "-u", "-m", "-ns", "-nm", {},          {}, "-h ", {},    {},   {},    "https://lerppu.net/wannabethesis",                              "https://github.com/nukeykt/NRedneck",                                      { OperatingSystem::Windows },                                                 { ORIGINAL_ATOMIC_EDITION.getName(),  JFDUKE3D.getName() });
const GameVersion GameVersion::CHOCOLATE_DUKE3D        ("Chocolate Duke3D",            true,  false, "", "Game.exe",           true,  false, false, "Chocolate", "/x",  "/g",  "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", {},          {}, {},    {},    {},   {},    "https://fabiensanglard.net/duke3d/chocolate_duke_nukem_3D.php", "https://github.com/fabiensanglard/chocolate_duke3D",                       { OperatingSystem::Windows },                                                 { ORIGINAL_REGULAR_VERSION.getName(), ORIGINAL_ATOMIC_EDITION.getName() });
const GameVersion GameVersion::BELGIAN_CHOCOLATE_DUKE3D("Belgian Chocolate Duke3D",    true,  false, "", "ChocoDuke3D.64.exe", true,  false, false, "Belgian",   "/x",  "/g",  "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", {},          "", {},    {},    {},   {},    "",                                                              "https://github.com/GPSnoopy/BelgianChocolateDuke3D",                       { OperatingSystem::Windows, OperatingSystem::Linux, OperatingSystem::MacOS }, { ORIGINAL_REGULAR_VERSION.getName(), ORIGINAL_ATOMIC_EDITION.getName() });
const GameVersion GameVersion::DUKE3DW                 ("Duke3dw",                     true,  false, "", "Duke3dw.exe",        true,  false, true,  "Duke3dw",   "/x",  "/g",  "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", {},          "", "/h",  {},    {},   {},    "http://www.proasm.com/duke/Duke3dw.html",                       "",                                                                         { OperatingSystem::Windows },                                                 { ORIGINAL_REGULAR_VERSION.getName(), ORIGINAL_ATOMIC_EDITION.getName(), JFDUKE3D.getName() });
const GameVersion GameVersion::DUKE3D_W32              ("Duke3d_w32",                  true,  false, "", "duke3d_w32.exe",     true,  false, false, "Duke_w32",  "/x",  "/g",  "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", {},          "", {},    {},    {},   {},    "http://www.rancidmeat.com/project.php3?id=1",                   "",                                                                         { OperatingSystem::Windows },                                                 { ORIGINAL_ATOMIC_EDITION.getName() });

const std::vector<const GameVersion *> GameVersion::DEFAULT_GAME_VERSIONS = {
	&LAMEDUKE,
	&ORIGINAL_REGULAR_VERSION,
	&ORIGINAL_ATOMIC_EDITION,
	&JFDUKE3D,
	&EDUKE32,
	&NETDUKE32,
//	&MEGATON_EDITION,
//	&WORLD_TOUR,
//	&BUILDGDX,
	&RAZE,
	&RED_NUKEM,
	&CHOCOLATE_DUKE3D,
	&BELGIAN_CHOCOLATE_DUKE3D,
	&DUKE3DW,
	&DUKE3D_W32
};

const bool GameVersion::DEFAULT_LOCAL_WORKING_DIRECTORY = true;
const bool GameVersion::DEFAULT_RELATIVE_CON_FILE_PATH = false;
const bool GameVersion::DEFAULT_SUPPORTS_SUBDIRECTORIES = true;
const std::string GameVersion::DEFAULT_CON_FILE_ARGUMENT_FLAG("/x");
const std::string GameVersion::DEFAULT_GROUP_FILE_ARGUMENT_FLAG("/g");
const std::string GameVersion::DEFAULT_MAP_FILE_ARGUMENT_FLAG("-map ");
const std::string GameVersion::DEFAULT_EPISODE_ARGUMENT_FLAG("/v");
const std::string GameVersion::DEFAULT_LEVEL_ARGUMENT_FLAG("/l");
const std::string GameVersion::DEFAULT_SKILL_ARGUMENT_FLAG("/s");
const uint8_t GameVersion::DEFAULT_SKILL_START_VALUE = 1;
const std::string GameVersion::DEFAULT_RECORD_DEMO_ARGUMENT_FLAG("/r");
const std::string GameVersion::DEFAULT_PLAY_DEMO_ARGUMENT_FLAG("/d");
const std::string GameVersion::DEFAULT_RESPAWN_MODE_ARGUMENT_FLAG("/t");
const std::string GameVersion::DEFAULT_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG("/u");
const std::string GameVersion::DEFAULT_DISABLE_MONSTERS_ARGUMENT_FLAG("/m");
const std::string GameVersion::DEFAULT_DISABLE_SOUND_ARGUMENT_FLAG("/ns");
const std::string GameVersion::DEFAULT_DISABLE_MUSIC_ARGUMENT_FLAG("/nm");

GameVersion::GameVersion()
	: m_removable(true)
	, m_renamable(true)
	, m_localWorkingDirectory(DEFAULT_LOCAL_WORKING_DIRECTORY)
	, m_relativeConFilePath(DEFAULT_RELATIVE_CON_FILE_PATH)
	, m_supportsSubdirectories(DEFAULT_SUPPORTS_SUBDIRECTORIES)
	, m_conFileArgumentFlag(DEFAULT_CON_FILE_ARGUMENT_FLAG)
	, m_groupFileArgumentFlag(DEFAULT_GROUP_FILE_ARGUMENT_FLAG)
	, m_mapFileArgumentFlag(DEFAULT_MAP_FILE_ARGUMENT_FLAG)
	, m_episodeArgumentFlag(DEFAULT_EPISODE_ARGUMENT_FLAG)
	, m_levelArgumentFlag(DEFAULT_LEVEL_ARGUMENT_FLAG)
	, m_skillArgumentFlag(DEFAULT_SKILL_ARGUMENT_FLAG)
	, m_skillStartValue(DEFAULT_SKILL_START_VALUE)
	, m_recordDemoArgumentFlag(DEFAULT_RECORD_DEMO_ARGUMENT_FLAG)
	, m_playDemoArgumentFlag(DEFAULT_PLAY_DEMO_ARGUMENT_FLAG)
	, m_respawnModeArgumentFlag(DEFAULT_RESPAWN_MODE_ARGUMENT_FLAG)
	, m_weaponSwitchOrderArgumentFlag(DEFAULT_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG)
	, m_disableMonstersArgumentFlag(DEFAULT_DISABLE_MONSTERS_ARGUMENT_FLAG)
	, m_disableSoundArgumentFlag(DEFAULT_DISABLE_SOUND_ARGUMENT_FLAG)
	, m_disableMusicArgumentFlag(DEFAULT_DISABLE_MUSIC_ARGUMENT_FLAG)
	, m_modified(false) { }

GameVersion::GameVersion(const std::string & name, bool removable, bool renamable, const std::string & gamePath, const std::string & gameExecutableName, bool localWorkingDirectory, bool relativeConFilePath, bool supportsSubdirectories, const std::string & modDirectoryName, const std::optional<std::string> & conFileArgumentFlag, const std::optional<std::string> & groupFileArgumentFlag, const std::optional<std::string> & mapFileArgumentFlag, const std::string & episodeArgumentFlag, const std::string & levelArgumentFlag, const std::string & skillArgumentFlag, uint8_t skillStartValue, const std::string & recordDemoArgumentFlag, const std::optional<std::string> & playDemoArgumentFlag, const std::optional<std::string> & respawnModeArgumentFlag, const std::optional<std::string> & weaponSwitchOrderArgumentFlag, const std::optional<std::string> & disableMonstersArgumentFlag, const std::optional<std::string> & disableSoundArgumentFlag, const std::optional<std::string> & disableMusicArgumentFlag, const std::optional<std::string> & setupExecutableName, const std::optional<std::string> & groupFileInstallPath, const std::optional<std::string> & defFileArgumentFlag, const std::optional<bool> & requiresCombinedGroup, const std::optional<bool> & requiresGroupFileExtraction, const std::optional<bool> & requiresDOSBox, const std::string & website, const std::string & sourceCodeURL, const std::vector<OperatingSystem> & supportedOperatingSystems, const std::vector<std::string> & compatibleGameVersions)
	: m_name(Utilities::trimString(name))
	, m_removable(removable)
	, m_renamable(renamable)
	, m_gamePath(Utilities::trimString(gamePath))
	, m_gameExecutableName(Utilities::trimString(gameExecutableName))
	, m_requiresCombinedGroup(requiresCombinedGroup)
	, m_requiresGroupFileExtraction(requiresGroupFileExtraction)
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
	, m_skillStartValue(skillStartValue)
	, m_recordDemoArgumentFlag(recordDemoArgumentFlag)
	, m_playDemoArgumentFlag(playDemoArgumentFlag)
	, m_respawnModeArgumentFlag(respawnModeArgumentFlag)
	, m_weaponSwitchOrderArgumentFlag(weaponSwitchOrderArgumentFlag)
	, m_disableMonstersArgumentFlag(disableMonstersArgumentFlag)
	, m_disableSoundArgumentFlag(disableSoundArgumentFlag)
	, m_disableMusicArgumentFlag(disableMusicArgumentFlag)
	, m_modified(false) {
	if(setupExecutableName.has_value()) {
		m_setupExecutableName = Utilities::trimString(setupExecutableName.value());
	}

	if(groupFileInstallPath.has_value()) {
		m_groupFileInstallPath = Utilities::trimString(groupFileInstallPath.value());
	}

	addSupportedOperatingSystems(supportedOperatingSystems);
	addCompatibleGameVersionNames(compatibleGameVersions);

	setModified(false);
}

GameVersion::GameVersion(GameVersion && gameVersion) noexcept
	: m_name(std::move(gameVersion.m_name))
	, m_removable(gameVersion.m_removable)
	, m_renamable(gameVersion.m_renamable)
	, m_gamePath(std::move(gameVersion.m_gamePath))
	, m_gameExecutableName(std::move(gameVersion.m_gameExecutableName))
	, m_setupExecutableName(std::move(gameVersion.m_setupExecutableName))
	, m_groupFileInstallPath(std::move(gameVersion.m_groupFileInstallPath))
	, m_requiresCombinedGroup(gameVersion.m_requiresCombinedGroup)
	, m_requiresGroupFileExtraction(gameVersion.m_requiresGroupFileExtraction)
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
	, m_skillStartValue(gameVersion.m_skillStartValue)
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
	, m_removable(gameVersion.m_removable)
	, m_renamable(gameVersion.m_renamable)
	, m_gamePath(gameVersion.m_gamePath)
	, m_gameExecutableName(gameVersion.m_gameExecutableName)
	, m_setupExecutableName(gameVersion.m_setupExecutableName)
	, m_groupFileInstallPath(gameVersion.m_groupFileInstallPath)
	, m_requiresCombinedGroup(gameVersion.m_requiresCombinedGroup)
	, m_requiresGroupFileExtraction(gameVersion.m_requiresGroupFileExtraction)
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
	, m_skillStartValue(gameVersion.m_skillStartValue)
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
		m_removable = gameVersion.m_removable;
		m_renamable = gameVersion.m_renamable;
		m_gamePath = std::move(gameVersion.m_gamePath);
		m_gameExecutableName = std::move(gameVersion.m_gameExecutableName);
		m_setupExecutableName = std::move(gameVersion.m_setupExecutableName);
		m_groupFileInstallPath = std::move(gameVersion.m_groupFileInstallPath);
		m_requiresCombinedGroup = gameVersion.m_requiresCombinedGroup;
		m_requiresGroupFileExtraction = gameVersion.m_requiresGroupFileExtraction;
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
		m_skillStartValue = gameVersion.m_skillStartValue;
		m_recordDemoArgumentFlag = std::move(gameVersion.m_recordDemoArgumentFlag);
		m_playDemoArgumentFlag = std::move(gameVersion.m_playDemoArgumentFlag);
		m_respawnModeArgumentFlag = std::move(gameVersion.m_respawnModeArgumentFlag);
		m_weaponSwitchOrderArgumentFlag = std::move(gameVersion.m_weaponSwitchOrderArgumentFlag);
		m_disableMonstersArgumentFlag = std::move(gameVersion.m_disableMonstersArgumentFlag);
		m_disableSoundArgumentFlag = std::move(gameVersion.m_disableSoundArgumentFlag);
		m_disableMusicArgumentFlag = std::move(gameVersion.m_disableMusicArgumentFlag);
		m_supportedOperatingSystems = std::move(gameVersion.m_supportedOperatingSystems);
		m_compatibleGameVersions = std::move(gameVersion.m_compatibleGameVersions);
		setModified(true);
	}

	return *this;
}

GameVersion & GameVersion::operator = (const GameVersion & gameVersion) {
	m_name = gameVersion.m_name;
	m_removable = gameVersion.m_removable;
	m_renamable = gameVersion.m_renamable;
	m_gamePath = gameVersion.m_gamePath;
	m_gameExecutableName = gameVersion.m_gameExecutableName;
	m_setupExecutableName = gameVersion.m_setupExecutableName;
	m_groupFileInstallPath = gameVersion.m_groupFileInstallPath;
	m_requiresCombinedGroup = gameVersion.m_requiresCombinedGroup;
	m_requiresGroupFileExtraction = gameVersion.m_requiresGroupFileExtraction;
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
	m_skillStartValue = gameVersion.m_skillStartValue;
	m_recordDemoArgumentFlag = gameVersion.m_recordDemoArgumentFlag;
	m_playDemoArgumentFlag = gameVersion.m_playDemoArgumentFlag;
	m_respawnModeArgumentFlag = gameVersion.m_respawnModeArgumentFlag;
	m_weaponSwitchOrderArgumentFlag = gameVersion.m_weaponSwitchOrderArgumentFlag;
	m_disableMonstersArgumentFlag = gameVersion.m_disableMonstersArgumentFlag;
	m_disableSoundArgumentFlag = gameVersion.m_disableSoundArgumentFlag;
	m_disableMusicArgumentFlag = gameVersion.m_disableMusicArgumentFlag;
	m_supportedOperatingSystems = gameVersion.m_supportedOperatingSystems;
	m_compatibleGameVersions = gameVersion.m_compatibleGameVersions;
	setModified(true);

	return *this;
}

GameVersion::~GameVersion() = default;

bool GameVersion::isModified() const {
	return m_modified;
}

void GameVersion::setModified(bool modified) {
	m_modified = modified;

	notifyGameVersionModified();
}

bool GameVersion::hasName() const {
	return !m_name.empty();
}

const std::string & GameVersion::getName() const {
	return m_name;
}

bool GameVersion::setName(const std::string & name) {
	if(!m_renamable) {
		return false;
	}

	std::string formattedName(Utilities::trimString(name));

	if(Utilities::areStringsEqual(m_name, formattedName)) {
		return true;
	}

	m_name = std::move(formattedName);

	setModified(true);

	return true;
}

bool GameVersion::isRemovable() const {
	return m_removable;
}

bool GameVersion::isRenamable() const {
	return m_renamable;
}

bool GameVersion::hasGamePath() const {
	return !m_gamePath.empty();
}

const std::string & GameVersion::getGamePath() const {
	return m_gamePath;
}

void GameVersion::setGamePath(const std::string & gamePath) {
	std::string formattedGamePath(Utilities::trimString(gamePath));

	if(Utilities::areStringsEqual(m_gamePath, formattedGamePath)) {
		return;
	}

	m_gamePath = std::move(formattedGamePath);

	setModified(true);
}

const std::string & GameVersion::getGameExecutableName() const {
	return m_gameExecutableName;
}

void GameVersion::setGameExecutableName(const std::string & gameExecutableName) {
	std::string formattedGameExecutableName(Utilities::trimString(gameExecutableName));

	if(Utilities::areStringsEqual(m_gameExecutableName, formattedGameExecutableName)) {
		return;
	}

	m_gameExecutableName = std::move(formattedGameExecutableName);

	setModified(true);
}

bool GameVersion::hasSetupExecutableName() const {
	return m_setupExecutableName.has_value();
}

const std::optional<std::string> & GameVersion::getSetupExecutableName() const {
	return m_setupExecutableName;
}

void GameVersion::setSetupExecutableName(const std::string & setupExecutableName) {
	std::string formattedSetupExecutableName(Utilities::trimString(setupExecutableName));

	if(m_setupExecutableName.has_value() && Utilities::areStringsEqual(m_setupExecutableName.value(), formattedSetupExecutableName)) {
		return;
	}

	m_setupExecutableName = std::move(formattedSetupExecutableName);

	setModified(true);
}

void GameVersion::clearSetupExecutableName() {
	if(!m_setupExecutableName.has_value()) {
		return;
	}

	m_setupExecutableName.reset();

	setModified(true);
}

bool GameVersion::hasGroupFileInstallPath() const {
	return m_groupFileInstallPath.has_value();
}

const std::optional<std::string> & GameVersion::getGroupFileInstallPath() const {
	return m_groupFileInstallPath;
}

void GameVersion::setGroupFileInstallPath(const std::string & groupFileInstallPath) {
	std::string formattedGroupFileInstallPath(Utilities::trimString(groupFileInstallPath));

	if(m_groupFileInstallPath.has_value() && Utilities::areStringsEqual(m_groupFileInstallPath.value(), formattedGroupFileInstallPath)) {
		return;
	}

	m_groupFileInstallPath = std::move(formattedGroupFileInstallPath);

	setModified(true);
}

void GameVersion::clearGroupFileInstallPath() {
	if(!m_groupFileInstallPath.has_value()) {
		return;
	}

	m_groupFileInstallPath.reset();

	setModified(true);
}

bool GameVersion::doesRequireCombinedGroup() const {
	return m_requiresCombinedGroup.has_value() && m_requiresCombinedGroup.value();
}

const std::optional<bool> & GameVersion::getRequiresCombinedGroup() const {
	return m_requiresCombinedGroup;
}

void GameVersion::setRequiresCombinedGroup(bool requiresCombinedGroup) {
	if(m_requiresCombinedGroup == requiresCombinedGroup) {
		return;
	}

	m_requiresCombinedGroup = requiresCombinedGroup;

	setModified(true);
}

void GameVersion::clearRequiresCombinedGroup() {
	if(!m_requiresCombinedGroup.has_value()) {
		return;
	}

	m_requiresCombinedGroup.reset();

	setModified(true);
}

bool GameVersion::doesRequireGroupFileExtraction() const {
	return m_requiresGroupFileExtraction.has_value() && m_requiresGroupFileExtraction.value();
}

const std::optional<bool> & GameVersion::getRequiresGroupFileExtraction() const {
	return m_requiresGroupFileExtraction;
}

void GameVersion::setRequiresGroupFileExtraction(bool requiresGroupFileExtraction) {
	if(m_requiresGroupFileExtraction == requiresGroupFileExtraction) {
		return;
	}

	m_requiresGroupFileExtraction = requiresGroupFileExtraction;

	setModified(true);
}

void GameVersion::clearRequiresGroupFileExtraction() {
	if(!m_requiresGroupFileExtraction.has_value()) {
		return;
	}

	m_requiresGroupFileExtraction.reset();

	setModified(true);
}

bool GameVersion::doesRequireDOSBox() const {
	return m_requiresDOSBox.has_value() && m_requiresDOSBox.value();
}

const std::optional<bool> & GameVersion::getRequiresDOSBox() const {
	return m_requiresDOSBox;
}

void GameVersion::setRequiresDOSBox(bool requiresDOSBox) {
	if(m_requiresDOSBox == requiresDOSBox) {
		return;
	}

	m_requiresDOSBox = requiresDOSBox;

	setModified(true);
}

void GameVersion::clearRequiresDOSBox() {
	if(!m_requiresDOSBox.has_value()) {
		return;
	}

	m_requiresDOSBox.reset();

	setModified(true);
}

bool GameVersion::hasLocalWorkingDirectory() const {
	return m_localWorkingDirectory;
}

void GameVersion::setLocalWorkingDirectory(bool localWorkingDirectory) {
	if(m_localWorkingDirectory == localWorkingDirectory) {
		return;
	}

	m_localWorkingDirectory = localWorkingDirectory;

	setModified(true);
}

const std::string & GameVersion::getModDirectoryName() const {
	return m_modDirectoryName;
}

void GameVersion::setModDirectoryName(const std::string & modDirectoryName) {
	if(Utilities::areStringsEqual(m_modDirectoryName, modDirectoryName)) {
		return;
	}

	m_modDirectoryName = Utilities::trimString(modDirectoryName);

	setModified(true);
}

bool GameVersion::hasRelativeConFilePath() const {
	return m_relativeConFilePath;
}

void GameVersion::setRelativeConFilePath(bool relativeConFilePath) {
	if(m_relativeConFilePath == relativeConFilePath) {
		return;
	}

	m_relativeConFilePath = relativeConFilePath;

	setModified(true);
}

bool GameVersion::doesSupportSubdirectories() const {
	return m_supportsSubdirectories;
}

void GameVersion::setSupportsSubdirectories(bool supportsSubdirectories) {
	if(m_supportsSubdirectories == supportsSubdirectories) {
		return;
	}

	m_supportsSubdirectories = supportsSubdirectories;

	setModified(true);
}

bool GameVersion::hasConFileArgumentFlag() const {
	return m_conFileArgumentFlag.has_value();
}

const std::optional<std::string> & GameVersion::getConFileArgumentFlag() const {
	return m_conFileArgumentFlag;
}

bool GameVersion::setConFileArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	if(m_conFileArgumentFlag.has_value() && Utilities::areStringsEqual(m_conFileArgumentFlag.value(), flag)) {
		return true;
	}

	m_conFileArgumentFlag = flag;

	setModified(true);

	return true;
}

void GameVersion::clearConFileArgumentFlag() {
	if(!m_conFileArgumentFlag.has_value()) {
		return;
	}

	m_conFileArgumentFlag.reset();

	setModified(true);
}

bool GameVersion::hasGroupFileArgumentFlag() const {
	return m_groupFileArgumentFlag.has_value();
}

const std::optional<std::string> & GameVersion::getGroupFileArgumentFlag() const {
	return m_groupFileArgumentFlag;
}

bool GameVersion::setGroupFileArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	if(m_groupFileArgumentFlag.has_value() && Utilities::areStringsEqual(m_groupFileArgumentFlag.value(), flag)) {
		return true;
	}

	m_groupFileArgumentFlag = flag;

	setModified(true);

	return true;
}

void GameVersion::clearGroupFileArgumentFlag() {
	if(!m_groupFileArgumentFlag.has_value()) {
		return;
	}

	m_groupFileArgumentFlag.reset();

	setModified(true);
}

bool GameVersion::hasDefFileArgumentFlag() const {
	return m_defFileArgumentFlag.has_value();
}

const std::optional<std::string> & GameVersion::getDefFileArgumentFlag() const {
	return m_defFileArgumentFlag;
}

bool GameVersion::setDefFileArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	if(m_defFileArgumentFlag.has_value() && Utilities::areStringsEqual(m_defFileArgumentFlag.value(), flag)) {
		return true;
	}

	m_defFileArgumentFlag = flag;

	setModified(true);

	return true;
}

void GameVersion::clearDefFileArgumentFlag() {
	if(!m_defFileArgumentFlag.has_value()) {
		return;
	}

	m_defFileArgumentFlag.reset();

	setModified(true);
}

bool GameVersion::hasMapFileArgumentFlag() const {
	return m_mapFileArgumentFlag.has_value();
}

const std::optional<std::string> & GameVersion::getMapFileArgumentFlag() const {
	return m_mapFileArgumentFlag;
}

bool GameVersion::setMapFileArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	if(m_mapFileArgumentFlag.has_value() && Utilities::areStringsEqual(m_mapFileArgumentFlag.value(), flag)) {
		return true;
	}

	m_mapFileArgumentFlag = flag;

	setModified(true);

	return true;
}

void GameVersion::clearMapFileArgumentFlag() {
	if(!m_mapFileArgumentFlag.has_value()) {
		return;
	}

	m_mapFileArgumentFlag.reset();

	setModified(true);
}

const std::string & GameVersion::getEpisodeArgumentFlag() const {
	return m_episodeArgumentFlag;
}

bool GameVersion::setEpisodeArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	if(Utilities::areStringsEqual(m_episodeArgumentFlag, flag)) {
		return true;
	}

	m_episodeArgumentFlag = flag;

	setModified(true);

	return true;
}

const std::string & GameVersion::getLevelArgumentFlag() const {
	return m_levelArgumentFlag;
}

bool GameVersion::setLevelArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	if(Utilities::areStringsEqual(m_levelArgumentFlag, flag)) {
		return true;
	}

	m_levelArgumentFlag = flag;

	setModified(true);

	return true;
}

const std::string & GameVersion::getSkillArgumentFlag() const {
	return m_skillArgumentFlag;
}

bool GameVersion::setSkillArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	if(Utilities::areStringsEqual(m_skillArgumentFlag, flag)) {
		return true;
	}

	m_skillArgumentFlag = flag;

	setModified(true);

	return true;
}

uint8_t GameVersion::getSkillStartValue() const {
	return m_skillStartValue;
}

void GameVersion::setSkillStartValue(uint8_t skillStartValue) {
	m_skillStartValue = skillStartValue;
}

const std::string & GameVersion::getRecordDemoArgumentFlag() const {
	return m_recordDemoArgumentFlag;
}

bool GameVersion::setRecordDemoArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	if(Utilities::areStringsEqual(m_recordDemoArgumentFlag, flag)) {
		return true;
	}

	m_recordDemoArgumentFlag = flag;

	setModified(true);

	return true;
}

bool GameVersion::hasPlayDemoArgumentFlag() const {
	return m_playDemoArgumentFlag.has_value();
}

const std::optional<std::string> & GameVersion::getPlayDemoArgumentFlag() const {
	return m_playDemoArgumentFlag;
}

bool GameVersion::setPlayDemoArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	if(m_playDemoArgumentFlag.has_value() && Utilities::areStringsEqual(m_playDemoArgumentFlag.value(), flag)) {
		return true;
	}

	m_playDemoArgumentFlag = flag;

	setModified(true);

	return true;
}

void GameVersion::clearPlayDemoArgumentFlag() {
	if(!m_playDemoArgumentFlag.has_value()) {
		return;
	}

	m_playDemoArgumentFlag.reset();

	setModified(true);
}

bool GameVersion::hasRespawnModeArgumentFlag() const {
	return m_respawnModeArgumentFlag.has_value();
}

const std::optional<std::string> & GameVersion::getRespawnModeArgumentFlag() const {
	return m_respawnModeArgumentFlag;
}

bool GameVersion::setRespawnModeArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	if(m_respawnModeArgumentFlag.has_value() && Utilities::areStringsEqual(m_respawnModeArgumentFlag.value(), flag)) {
		return true;
	}

	m_respawnModeArgumentFlag = flag;

	setModified(true);

	return true;
}

void GameVersion::clearRespawnModeArgumentFlag() {
	if(!m_respawnModeArgumentFlag.has_value()) {
		return;
	}

	m_respawnModeArgumentFlag.reset();

	setModified(true);
}

bool GameVersion::hasWeaponSwitchOrderArgumentFlag() const {
	return m_weaponSwitchOrderArgumentFlag.has_value();
}

const std::optional<std::string> & GameVersion::getWeaponSwitchOrderArgumentFlag() const {
	return m_weaponSwitchOrderArgumentFlag;
}

bool GameVersion::setWeaponSwitchOrderArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	if(m_weaponSwitchOrderArgumentFlag.has_value() && Utilities::areStringsEqual(m_weaponSwitchOrderArgumentFlag.value(), flag)) {
		return true;
	}

	m_weaponSwitchOrderArgumentFlag = flag;

	setModified(true);

	return true;
}

void GameVersion::clearWeaponSwitchOrderArgumentFlag() {
	if(!m_weaponSwitchOrderArgumentFlag.has_value()) {
		return;
	}

	m_weaponSwitchOrderArgumentFlag.reset();

	setModified(true);
}

bool GameVersion::hasDisableMonstersArgumentFlag() const {
	return m_disableMonstersArgumentFlag.has_value();
}

const std::optional<std::string> & GameVersion::getDisableMonstersArgumentFlag() const {
	return m_disableMonstersArgumentFlag;
}

bool GameVersion::setDisableMonstersArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	if(m_disableMonstersArgumentFlag.has_value() && Utilities::areStringsEqual(m_disableMonstersArgumentFlag.value(), flag)) {
		return true;
	}

	m_disableMonstersArgumentFlag = flag;

	setModified(true);

	return true;
}

void GameVersion::clearDisableMonstersArgumentFlag() {
	if(!m_disableMonstersArgumentFlag.has_value()) {
		return;
	}

	m_disableMonstersArgumentFlag.reset();

	setModified(true);
}

bool GameVersion::hasDisableSoundArgumentFlag() const {
	return m_disableSoundArgumentFlag.has_value();
}

const std::optional<std::string> & GameVersion::getDisableSoundArgumentFlag() const {
	return m_disableSoundArgumentFlag;
}

bool GameVersion::setDisableSoundArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	if(m_disableSoundArgumentFlag.has_value() && Utilities::areStringsEqual(m_disableSoundArgumentFlag.value(), flag)) {
		return true;
	}

	m_disableSoundArgumentFlag = flag;

	setModified(true);

	return true;
}

void GameVersion::clearDisableSoundArgumentFlag() {
	if(!m_disableSoundArgumentFlag.has_value()) {
		return;
	}

	m_disableSoundArgumentFlag.reset();

	setModified(true);
}

bool GameVersion::hasDisableMusicArgumentFlag() const {
	return m_disableMusicArgumentFlag.has_value();
}

const std::optional<std::string> & GameVersion::getDisableMusicArgumentFlag() const {
	return m_disableMusicArgumentFlag;
}

bool GameVersion::setDisableMusicArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	if(m_disableMusicArgumentFlag.has_value() && Utilities::areStringsEqual(m_disableMusicArgumentFlag.value(), flag)) {
		return true;
	}

	m_disableMusicArgumentFlag = flag;

	setModified(true);

	return true;
}

void GameVersion::clearDisableMusicArgumentFlag() {
	if(!m_disableMusicArgumentFlag.has_value()) {
		return;
	}

	m_disableMusicArgumentFlag.reset();

	setModified(true);
}

const std::string & GameVersion::getWebsite() const {
	return m_website;
}

void GameVersion::setWebsite(const std::string & website) {
	std::string formattedWebsite(Utilities::trimString(website));

	if(Utilities::areStringsEqual(m_website, formattedWebsite)) {
		return;
	}

	m_website = formattedWebsite;

	setModified(true);
}

const std::string & GameVersion::getSourceCodeURL() const {
	return m_sourceCodeURL;
}

void GameVersion::setSourceCodeURL(const std::string & sourceCodeURL) {
	std::string formattedSourceCodeURL(Utilities::trimString(sourceCodeURL));

	if(Utilities::areStringsEqual(m_sourceCodeURL, formattedSourceCodeURL)) {
		return;
	}

	m_sourceCodeURL = formattedSourceCodeURL;

	setModified(true);
}

size_t GameVersion::numberOfSupportedOperatingSystems() const {
	return m_supportedOperatingSystems.size();
}

bool GameVersion::hasSupportedOperatingSystem(OperatingSystem operatingSystem) const {
	return std::find(std::begin(m_supportedOperatingSystems), std::end(m_supportedOperatingSystems), operatingSystem) != std::end(m_supportedOperatingSystems);
}

bool GameVersion::hasSupportedOperatingSystemType(DeviceInformationBridge::OperatingSystemType operatingSystemType) const {
	std::optional<OperatingSystem> optionalOperatingSystem(convertOperatingSystemType(operatingSystemType));

	if(!optionalOperatingSystem.has_value()) {
		return false;
	}

	return hasSupportedOperatingSystem(optionalOperatingSystem.value());
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

size_t GameVersion::indexOfSupportedOperatingSystemType(DeviceInformationBridge::OperatingSystemType operatingSystemType) const {
	std::optional<OperatingSystem> optionalOperatingSystem(convertOperatingSystemType(operatingSystemType));

	if(!optionalOperatingSystem.has_value()) {
		return std::numeric_limits<size_t>::max();
	}

	return indexOfSupportedOperatingSystem(optionalOperatingSystem.value());
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

const std::vector<GameVersion::OperatingSystem> & GameVersion::getSupportedOperatingSystems() const {
	return m_supportedOperatingSystems;
}

bool GameVersion::addSupportedOperatingSystem(OperatingSystem operatingSystem) {
	if(hasSupportedOperatingSystem(operatingSystem)) {
		return false;
	}

	m_supportedOperatingSystems.push_back(operatingSystem);

	setModified(true);

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

	setModified(true);

	return true;
}

bool GameVersion::removeSupportedOperatingSystem(OperatingSystem operatingSystem) {
	return removeSupportedOperatingSystem(indexOfSupportedOperatingSystem(operatingSystem));
}

bool GameVersion::removeSupportedOperatingSystemWithName(const std::string & operatingSystemName) {
	return removeSupportedOperatingSystem(indexOfSupportedOperatingSystemWithName(operatingSystemName));
}

void GameVersion::clearSupportedOperatingSystems() {
	if(m_supportedOperatingSystems.empty()) {
		return;
	}

	m_supportedOperatingSystems.clear();

	setModified(true);
}

size_t GameVersion::numberOfCompatibleGameVersions() const {
	return m_compatibleGameVersions.size();
}

bool GameVersion::hasCompatibleGameVersion(const GameVersion & gameVersion) const {
	return hasCompatibleGameVersionWithName(gameVersion.getName());
}

bool GameVersion::hasCompatibleGameVersionWithName(const std::string & name) const {
	return indexOfCompatibleGameVersionWithName(name) != std::numeric_limits<size_t>::max();
}

size_t GameVersion::indexOfCompatibleGameVersionWithName(const std::string & name) const {
	auto compatibleGameVersionIterator = std::find_if(m_compatibleGameVersions.cbegin(), m_compatibleGameVersions.cend(), [&name](const std::string & currentGameVersionName) {
		return Utilities::areStringsEqualIgnoreCase(name, currentGameVersionName);
	});

	if(compatibleGameVersionIterator == m_compatibleGameVersions.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return compatibleGameVersionIterator - m_compatibleGameVersions.cbegin();
}

size_t GameVersion::indexOfCompatibleGameVersion(const GameVersion & gameVersion) const {
	return indexOfCompatibleGameVersionWithName(gameVersion.getName());
}

std::optional<std::string> GameVersion::getCompatibleGameVersion(size_t index) const {
	if(index >= m_compatibleGameVersions.size()) {
		return {};
	}

	return m_compatibleGameVersions[index];
}

const std::vector<std::string> & GameVersion::getCompatibleGameVersions() const {
	return m_compatibleGameVersions;
}

std::shared_ptr<ModGameVersion> GameVersion::getMostCompatibleModGameVersion(const std::vector<std::shared_ptr<ModGameVersion>> & modGameVersions) const {
	for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = modGameVersions.begin(); i != modGameVersions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase(m_name, (*i)->getGameVersion())) {
			return *i;
		}
	}

	for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = modGameVersions.begin(); i != modGameVersions.end(); ++i) {
		if(hasCompatibleGameVersionWithName((*i)->getGameVersion())) {
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

		if(hasCompatibleGameVersionWithName((*i)->getGameVersion())) {
			compatibleModGameVersions.push_back(*i);
		}
	}

	return compatibleModGameVersions;
}

bool GameVersion::addCompatibleGameVersion(const GameVersion & gameVersion) {
	return addCompatibleGameVersionWithName(gameVersion.getName());
}

bool GameVersion::addCompatibleGameVersionWithName(const std::string & name) {
	if(name.empty() || hasCompatibleGameVersionWithName(name)) {
		return false;
	}

	m_compatibleGameVersions.emplace_back(name);

	setModified(true);

	return true;
}

size_t GameVersion::addCompatibleGameVersionNames(const std::vector<std::string> & gameVersionNames) {
	size_t numberOfCompatibleGameVersionsAdded = 0;

	for(std::vector<std::string>::const_iterator i = gameVersionNames.begin(); i != gameVersionNames.end(); ++i) {
		if(addCompatibleGameVersionWithName(*i)) {
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

	setModified(true);

	return true;
}

bool GameVersion::removeCompatibleGameVersion(const GameVersion & gameVersion) {
	return removeCompatibleGameVersion(indexOfCompatibleGameVersion(gameVersion));
}

bool GameVersion::removeCompatibleGameVersionWithName(const std::string & name) {
	return removeCompatibleGameVersion(indexOfCompatibleGameVersionWithName(name));
}

void GameVersion::clearCompatibleGameVersions() {
	if(m_compatibleGameVersions.empty()) {
		return;
	}

	m_compatibleGameVersions.clear();

	setModified(true);
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

	gameVersionValue.AddMember(rapidjson::StringRef(JSON_REMOVABLE_PROPERTY_NAME), rapidjson::Value(m_removable), allocator);

	gameVersionValue.AddMember(rapidjson::StringRef(JSON_RENAMABLE_PROPERTY_NAME), rapidjson::Value(m_renamable), allocator);

	rapidjson::Value gamePathValue(m_gamePath.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_GAME_PATH_PROPERTY_NAME), gamePathValue, allocator);

	rapidjson::Value gameExecutableNameValue(m_gameExecutableName.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME), gameExecutableNameValue, allocator);

	gameVersionValue.AddMember(rapidjson::StringRef(JSON_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME), rapidjson::Value(m_localWorkingDirectory), allocator);

	gameVersionValue.AddMember(rapidjson::StringRef(JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME), rapidjson::Value(m_relativeConFilePath), allocator);

	gameVersionValue.AddMember(rapidjson::StringRef(JSON_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME), rapidjson::Value(m_supportsSubdirectories), allocator);

	if(m_conFileArgumentFlag.has_value()) {
		rapidjson::Value conFileArgumentFlagValue(m_conFileArgumentFlag.value().c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME), conFileArgumentFlagValue, allocator);
	}

	if(m_groupFileArgumentFlag.has_value()) {
		rapidjson::Value groupFileArgumentFlagValue(m_groupFileArgumentFlag.value().c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME), groupFileArgumentFlagValue, allocator);
	}

	if(m_defFileArgumentFlag.has_value()) {
		rapidjson::Value defFileArgumentFlagValue(m_defFileArgumentFlag.value().c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME), defFileArgumentFlagValue, allocator);
	}

	if(m_mapFileArgumentFlag.has_value()) {
		rapidjson::Value mapFileArgumentFlagValue(m_mapFileArgumentFlag.value().c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_MAP_FILE_ARGUMENT_FLAG_PROPERTY_NAME), mapFileArgumentFlagValue, allocator);
	}

	rapidjson::Value episodeArgumentFlagValue(m_episodeArgumentFlag.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_EPISODE_ARGUMENT_FLAG_PROPERTY_NAME), episodeArgumentFlagValue, allocator);

	rapidjson::Value levelArgumentFlagValue(m_levelArgumentFlag.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_LEVEL_ARGUMENT_FLAG_PROPERTY_NAME), levelArgumentFlagValue, allocator);

	rapidjson::Value skillArgumentFlagValue(m_skillArgumentFlag.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_SKILL_ARGUMENT_FLAG_PROPERTY_NAME), skillArgumentFlagValue, allocator);

	gameVersionValue.AddMember(rapidjson::StringRef(JSON_SKILL_START_VALUE_PROPERTY_NAME), rapidjson::Value(m_skillStartValue), allocator);

	rapidjson::Value recordDemoArgumentFlagValue(m_recordDemoArgumentFlag.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_RECORD_DEMO_ARGUMENT_FLAG_PROPERTY_NAME), recordDemoArgumentFlagValue, allocator);

	if(m_playDemoArgumentFlag.has_value()) {
		rapidjson::Value playDemoArgumentFlagValue(m_playDemoArgumentFlag.value().c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_PLAY_DEMO_ARGUMENT_FLAG_PROPERTY_NAME), playDemoArgumentFlagValue, allocator);
	}

	if(m_respawnModeArgumentFlag.has_value()) {
		rapidjson::Value respawnModeArgumentFlagValue(m_respawnModeArgumentFlag.value().c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_RESPAWN_MODE_ARGUMENT_FLAG_PROPERTY_NAME), respawnModeArgumentFlagValue, allocator);
	}

	if(m_weaponSwitchOrderArgumentFlag.has_value()) {
		rapidjson::Value weaponSwitchOrderArgumentFlagValue(m_weaponSwitchOrderArgumentFlag.value().c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG_PROPERTY_NAME), weaponSwitchOrderArgumentFlagValue, allocator);
	}

	if(m_disableMonstersArgumentFlag.has_value()) {
		rapidjson::Value disableMonstersArgumentFlagValue(m_disableMonstersArgumentFlag.value().c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_DISABLE_MONSTERS_ARGUMENT_FLAG_PROPERTY_NAME), disableMonstersArgumentFlagValue, allocator);
	}

	if(m_disableSoundArgumentFlag.has_value()) {
		rapidjson::Value disableSoundArgumentFlagValue(m_disableSoundArgumentFlag.value().c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_DISABLE_SOUND_ARGUMENT_FLAG_PROPERTY_NAME), disableSoundArgumentFlagValue, allocator);
	}

	if(m_disableMusicArgumentFlag.has_value()) {
		rapidjson::Value disableMusicArgumentFlagValue(m_disableMusicArgumentFlag.value().c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_DISABLE_MUSIC_ARGUMENT_FLAG_PROPERTY_NAME), disableMusicArgumentFlagValue, allocator);
	}

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

	if(m_requiresGroupFileExtraction.has_value()) {
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_REQUIRES_GROUP_FILE_EXTRACTION_PROPERTY_NAME), rapidjson::Value(m_requiresGroupFileExtraction.value()), allocator);
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
			spdlog::warn("Game version has unexpected property '{}'.", i->name.GetString());
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

	// parse removable property
	bool removable = true;

	if(gameVersionValue.HasMember(JSON_REMOVABLE_PROPERTY_NAME)) {
		const rapidjson::Value & removableValue = gameVersionValue[JSON_REMOVABLE_PROPERTY_NAME];

		if(!removableValue.IsBool()) {
			spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'boolean'.", JSON_REMOVABLE_PROPERTY_NAME, Utilities::typeToString(removableValue.GetType()));
			return nullptr;
		}

		removable = removableValue.GetBool();
	}
	else {
		spdlog::warn("Game version is missing '{}' property.", JSON_REMOVABLE_PROPERTY_NAME);
	}

	// parse renamable property
	bool renamable = true;

	if(gameVersionValue.HasMember(JSON_RENAMABLE_PROPERTY_NAME)) {
		const rapidjson::Value & renamableValue = gameVersionValue[JSON_RENAMABLE_PROPERTY_NAME];

		if(!renamableValue.IsBool()) {
			spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'boolean'.", JSON_RENAMABLE_PROPERTY_NAME, Utilities::typeToString(renamableValue.GetType()));
			return nullptr;
		}

		renamable = renamableValue.GetBool();
	}
	else {
		spdlog::warn("Game version is missing '{}' property.", JSON_RENAMABLE_PROPERTY_NAME);
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
	std::optional<std::string> optionalConFileArgumentFlag;

	if(gameVersionValue.HasMember(JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & conFileArgumentFlagValue = gameVersionValue[JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!conFileArgumentFlagValue.IsString()) {
			spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(conFileArgumentFlagValue.GetType()));
			return nullptr;
		}

		optionalConFileArgumentFlag = conFileArgumentFlagValue.GetString();

		if(optionalConFileArgumentFlag.value().empty()) {
			spdlog::error("Game version '{}' property cannot be empty.", JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME);
			return nullptr;
		}
	}

	// parse game version group file argument flag
	std::optional<std::string> optionalGroupFileArgumentFlag;

	if(gameVersionValue.HasMember(JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & groupFileArgumentFlagValue = gameVersionValue[JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!groupFileArgumentFlagValue.IsString()) {
			spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(groupFileArgumentFlagValue.GetType()));
			return nullptr;
		}

		optionalGroupFileArgumentFlag = groupFileArgumentFlagValue.GetString();

		if(optionalGroupFileArgumentFlag.value().empty()) {
			spdlog::error("Game version '{}' property cannot be empty.", JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME);
			return nullptr;
		}
	}

	// parse game version map file argument flag
	std::optional<std::string> optionalMapFileArgumentFlag;

	if(gameVersionValue.HasMember(JSON_MAP_FILE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & mapFileArgumentFlagValue = gameVersionValue[JSON_MAP_FILE_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!mapFileArgumentFlagValue.IsString()) {
			spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_MAP_FILE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(mapFileArgumentFlagValue.GetType()));
			return nullptr;
		}

		optionalMapFileArgumentFlag = mapFileArgumentFlagValue.GetString();

		if(optionalMapFileArgumentFlag.value().empty()) {
			spdlog::error("Game version '{}' property cannot be empty.", JSON_MAP_FILE_ARGUMENT_FLAG_PROPERTY_NAME);
			return nullptr;
		}
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

	// parse skill start value option
	uint8_t skillStartValue = DEFAULT_SKILL_START_VALUE;

	if(gameVersionValue.HasMember(JSON_SKILL_START_VALUE_PROPERTY_NAME)) {
		const rapidjson::Value & skillStartValueValue = gameVersionValue[JSON_SKILL_START_VALUE_PROPERTY_NAME];

		if(!skillStartValueValue.IsUint()) {
			spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'boolean'.", JSON_SKILL_START_VALUE_PROPERTY_NAME, Utilities::typeToString(skillStartValueValue.GetType()));
			return nullptr;
		}

		unsigned int tempSkillStartValue = skillStartValueValue.GetUint();

		if(tempSkillStartValue > std::numeric_limits<uint8_t>::max()) {
			spdlog::error("Game version '{}' property value of {} exceeds the maximum value of {}.", JSON_SKILL_START_VALUE_PROPERTY_NAME, tempSkillStartValue, std::numeric_limits<uint8_t>::max());
			return nullptr;
		}

		skillStartValue = static_cast<uint8_t>(tempSkillStartValue);
	}
	else {
		spdlog::warn("Game version is missing '{}' property.", JSON_SKILL_START_VALUE_PROPERTY_NAME);
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
			spdlog::error("Game version '{}' property has invalid type: '{}', expected 'string'.", JSON_PLAY_DEMO_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(playDemoArgumentFlagValue.GetType()));
			return nullptr;
		}

		optionalPlayDemoArgumentFlag = playDemoArgumentFlagValue.GetString();
	}

	// parse game version respawn mode argument flag
	std::optional<std::string> optionalRespawnModeArgumentFlag;

	if(gameVersionValue.HasMember(JSON_RESPAWN_MODE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & respawnModeArgumentFlagValue = gameVersionValue[JSON_RESPAWN_MODE_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!respawnModeArgumentFlagValue.IsString()) {
			spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_RESPAWN_MODE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(respawnModeArgumentFlagValue.GetType()));
			return nullptr;
		}

		optionalRespawnModeArgumentFlag = respawnModeArgumentFlagValue.GetString();

		if(optionalRespawnModeArgumentFlag.value().empty()) {
			spdlog::error("Game version '{}' property cannot be empty.", JSON_RESPAWN_MODE_ARGUMENT_FLAG_PROPERTY_NAME);
			return nullptr;
		}
	}

	// parse game version weapon switch order argument flag
	std::optional<std::string> optionalWeaponSwitchOrderArgumentFlag;

	if(gameVersionValue.HasMember(JSON_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & weaponSwitchOrderArgumentFlagValue = gameVersionValue[JSON_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!weaponSwitchOrderArgumentFlagValue.IsString()) {
			spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(weaponSwitchOrderArgumentFlagValue.GetType()));
			return nullptr;
		}

		optionalWeaponSwitchOrderArgumentFlag = weaponSwitchOrderArgumentFlagValue.GetString();

		if(optionalWeaponSwitchOrderArgumentFlag.value().empty()) {
			spdlog::error("Game version '{}' property cannot be empty.", JSON_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG_PROPERTY_NAME);
			return nullptr;
		}
	}

	// parse game version disable monsters argument flag
	std::optional<std::string> optionalDisableMonstersArgumentFlag;

	if(gameVersionValue.HasMember(JSON_DISABLE_MONSTERS_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & disableMonstersArgumentFlagValue = gameVersionValue[JSON_DISABLE_MONSTERS_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!disableMonstersArgumentFlagValue.IsString()) {
			spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_DISABLE_MONSTERS_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(disableMonstersArgumentFlagValue.GetType()));
			return nullptr;
		}

		optionalDisableMonstersArgumentFlag = disableMonstersArgumentFlagValue.GetString();

		if(optionalDisableMonstersArgumentFlag.value().empty()) {
			spdlog::error("Game version '{}' property cannot be empty.", JSON_DISABLE_MONSTERS_ARGUMENT_FLAG_PROPERTY_NAME);
			return nullptr;
		}
	}

	// parse game version disable sound argument flag
	std::optional<std::string> optionalDisableSoundArgumentFlag;

	if(gameVersionValue.HasMember(JSON_DISABLE_SOUND_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & disableSoundArgumentFlagValue = gameVersionValue[JSON_DISABLE_SOUND_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!disableSoundArgumentFlagValue.IsString()) {
			spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_DISABLE_SOUND_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(disableSoundArgumentFlagValue.GetType()));
			return nullptr;
		}

		optionalDisableSoundArgumentFlag = disableSoundArgumentFlagValue.GetString();

		if(optionalDisableSoundArgumentFlag.value().empty()) {
			spdlog::error("Game version '{}' property cannot be empty.", JSON_DISABLE_SOUND_ARGUMENT_FLAG_PROPERTY_NAME);
			return nullptr;
		}
	}

	// parse game version disable music argument flag
	std::optional<std::string> optionalDisableMusicArgumentFlag;

	if(gameVersionValue.HasMember(JSON_DISABLE_MUSIC_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & disableMusicArgumentFlagValue = gameVersionValue[JSON_DISABLE_MUSIC_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!disableMusicArgumentFlagValue.IsString()) {
			spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_DISABLE_MUSIC_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(disableMusicArgumentFlagValue.GetType()));
			return nullptr;
		}

		optionalDisableMusicArgumentFlag = disableMusicArgumentFlagValue.GetString();

		if(optionalDisableMusicArgumentFlag.value().empty()) {
			spdlog::error("Game version '{}' property cannot be empty.", JSON_DISABLE_MUSIC_ARGUMENT_FLAG_PROPERTY_NAME);
			return nullptr;
		}
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
	std::unique_ptr<GameVersion> newGameVersion = std::make_unique<GameVersion>(name, removable, renamable, gamePath, gameExecutableName, localWorkingDirectory, relativeConFilePath, supportsSubdirectories, modDirectoryName, optionalConFileArgumentFlag, optionalGroupFileArgumentFlag, optionalMapFileArgumentFlag, episodeArgumentFlag, levelArgumentFlag, skillArgumentFlag, skillStartValue, recordDemoArgumentFlag, optionalPlayDemoArgumentFlag, optionalRespawnModeArgumentFlag, optionalWeaponSwitchOrderArgumentFlag, optionalDisableMonstersArgumentFlag, optionalDisableSoundArgumentFlag, optionalDisableMusicArgumentFlag, setupExecutableNameOptional, groupFileInstallPathOptional);

	// parse game version def file argument flag
	if(gameVersionValue.HasMember(JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & defFileArgumentFlagValue = gameVersionValue[JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!defFileArgumentFlagValue.IsString()) {
			spdlog::error("Game version '{}' property has invalid type: '{}', expected 'string'.", JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(defFileArgumentFlagValue.GetType()));
			return nullptr;
		}

		newGameVersion->m_defFileArgumentFlag = defFileArgumentFlagValue.GetString();
	}

	// parse the game version requires full combined group property
	if(gameVersionValue.HasMember(JSON_REQUIRES_COMBINED_GROUP_PROPERTY_NAME)) {
		const rapidjson::Value & requiresCombinedGroupValue = gameVersionValue[JSON_REQUIRES_COMBINED_GROUP_PROPERTY_NAME];

		if(!requiresCombinedGroupValue.IsBool()) {
			spdlog::error("Game version '{}' property has invalid type: '{}', expected 'boolean'.", JSON_REQUIRES_COMBINED_GROUP_PROPERTY_NAME, Utilities::typeToString(requiresCombinedGroupValue.GetType()));
			return nullptr;
		}

		newGameVersion->m_requiresCombinedGroup = requiresCombinedGroupValue.GetBool();
	}

	// parse the game version requires group file extraction property
	if(gameVersionValue.HasMember(JSON_REQUIRES_GROUP_FILE_EXTRACTION_PROPERTY_NAME)) {
		const rapidjson::Value & requiresGroupFileExtractionValue = gameVersionValue[JSON_REQUIRES_GROUP_FILE_EXTRACTION_PROPERTY_NAME];

		if(!requiresGroupFileExtractionValue.IsBool()) {
			spdlog::error("Game version '{}' property has invalid type: '{}', expected 'boolean'.", JSON_REQUIRES_GROUP_FILE_EXTRACTION_PROPERTY_NAME, Utilities::typeToString(requiresGroupFileExtractionValue.GetType()));
			return nullptr;
		}

		newGameVersion->m_requiresGroupFileExtraction = requiresGroupFileExtractionValue.GetBool();
	}

	// parse the game version requires dosbox property
	if(gameVersionValue.HasMember(JSON_REQUIRES_DOSBOX_PROPERTY_NAME)) {
		const rapidjson::Value & requiresDOSBoxValue = gameVersionValue[JSON_REQUIRES_DOSBOX_PROPERTY_NAME];

		if(!requiresDOSBoxValue.IsBool()) {
			spdlog::error("Game version '{}' property has invalid type: '{}', expected 'boolean'.", JSON_REQUIRES_DOSBOX_PROPERTY_NAME, Utilities::typeToString(requiresDOSBoxValue.GetType()));
			return nullptr;
		}

		newGameVersion->m_requiresDOSBox = requiresDOSBoxValue.GetBool();
	}

	// parse the game version website property
	if(gameVersionValue.HasMember(JSON_WEBSITE_PROPERTY_NAME)) {
		const rapidjson::Value & websiteValue = gameVersionValue[JSON_WEBSITE_PROPERTY_NAME];

		if(!websiteValue.IsString()) {
			spdlog::error("Game version '{}' property has invalid type: '{}', expected 'string'.", JSON_WEBSITE_PROPERTY_NAME, Utilities::typeToString(websiteValue.GetType()));
			return nullptr;
		}

		newGameVersion->m_website = websiteValue.GetString();
	}

	// parse the game version source code url property
	if(gameVersionValue.HasMember(JSON_SOURCE_CODE_URL_PROPERTY_NAME)) {
		const rapidjson::Value & sourceCodeURLValue = gameVersionValue[JSON_SOURCE_CODE_URL_PROPERTY_NAME];

		if(!sourceCodeURLValue.IsString()) {
			spdlog::error("Game version '{}' property has invalid type: '{}', expected 'string'.", JSON_SOURCE_CODE_URL_PROPERTY_NAME, Utilities::typeToString(sourceCodeURLValue.GetType()));
			return nullptr;
		}

		newGameVersion->m_sourceCodeURL = sourceCodeURLValue.GetString();
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

			if(newGameVersion->hasCompatibleGameVersionWithName(compatibleGameVersion)) {
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

bool GameVersion::isConfigured(const GameVersion * gameVersion) {
	return gameVersion != nullptr && gameVersion->isConfigured();
}

bool GameVersion::isValid() const {
	if(m_name.empty() ||
	   m_gameExecutableName.empty() ||
	   m_modDirectoryName.empty() ||
	   (m_conFileArgumentFlag.has_value() && m_conFileArgumentFlag.value().empty()) ||
	   (m_groupFileArgumentFlag.has_value() && m_groupFileArgumentFlag.value().empty()) ||
	   (m_mapFileArgumentFlag.has_value() && m_mapFileArgumentFlag.value().empty()) ||
	   m_episodeArgumentFlag.empty() ||
	   m_levelArgumentFlag.empty() ||
	   m_skillArgumentFlag.empty() ||
	   m_recordDemoArgumentFlag.empty() ||
	   (m_playDemoArgumentFlag.has_value() && m_playDemoArgumentFlag.value().empty()) ||
	   (m_respawnModeArgumentFlag.has_value() && m_respawnModeArgumentFlag.value().empty()) ||
	   (m_weaponSwitchOrderArgumentFlag.has_value() && m_weaponSwitchOrderArgumentFlag.value().empty()) ||
	   (m_disableMonstersArgumentFlag.has_value() && m_disableMonstersArgumentFlag.value().empty()) ||
	   (m_disableSoundArgumentFlag.has_value() && m_disableSoundArgumentFlag.value().empty()) ||
	   (m_disableMusicArgumentFlag.has_value() && m_disableMusicArgumentFlag.value().empty())||
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

GameVersion::Listener::~Listener() { }

size_t GameVersion::numberOfListeners() const {
	return m_listeners.size();
}

bool GameVersion::hasListener(const Listener & listener) const {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			return true;
		}
	}

	return false;
}

size_t GameVersion::indexOfListener(const Listener & listener) const {
	for(size_t i = 0; i < m_listeners.size(); i++) {
		if(m_listeners[i] == &listener) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

GameVersion::Listener * GameVersion::getListener(size_t index) const {
	if(index >= m_listeners.size()) {
		return nullptr;
	}

	return m_listeners[index];
}

bool GameVersion::addListener(Listener & listener) {
	if(!hasListener(listener)) {
		m_listeners.push_back(&listener);

		return true;
	}

	return false;
}

bool GameVersion::removeListener(size_t index) {
	if(index >= m_listeners.size()) {
		return false;
	}

	m_listeners.erase(m_listeners.cbegin() + index);

	return true;
}

bool GameVersion::removeListener(const Listener & listener) {
	for(std::vector<Listener *>::const_iterator i = m_listeners.cbegin(); i != m_listeners.cend(); ++i) {
		if(*i == &listener) {
			m_listeners.erase(i);

			return true;
		}
	}

	return false;
}

void GameVersion::clearListeners() {
	m_listeners.clear();
}

void GameVersion::notifyGameVersionModified() {
	for(Listener * listener : m_listeners) {
		listener->gameVersionModified(*this);
	}
}

bool GameVersion::operator == (const GameVersion & gameVersion) const {
	if(m_removable != gameVersion.m_removable ||
	   m_renamable != gameVersion.m_renamable ||
	   m_requiresDOSBox != gameVersion.m_requiresDOSBox ||
	   m_requiresCombinedGroup != gameVersion.m_requiresCombinedGroup ||
	   m_requiresGroupFileExtraction != gameVersion.m_requiresGroupFileExtraction ||
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
	   m_skillStartValue != gameVersion.m_skillStartValue ||
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
