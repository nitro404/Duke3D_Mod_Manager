#include "GameVersion.h"

#include "Mod/ModGameVersion.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/TimeUtilities.h>

#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string_view>
#include <vector>

static constexpr const char * JSON_ID_PROPERTY_NAME = "id";
static constexpr const char * JSON_LONG_NAME_PROPERTY_NAME = "longName";
static constexpr const char * JSON_SHORT_NAME_PROPERTY_NAME = "shortName";
static constexpr const char * JSON_VERSION_PROPERTY_NAME = "version";
static constexpr const char * JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME = "installed";
static constexpr const char * JSON_LAST_PLAYED_TIMESTAMP_PROPERTY_NAME = "lastPlayed";
static constexpr const char * JSON_BASE_PROPERTY_NAME = "base";
static constexpr const char * JSON_REMOVABLE_PROPERTY_NAME = "removable";
static constexpr const char * JSON_GAME_PATH_PROPERTY_NAME = "gamePath";
static constexpr const char * JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME = "gameExecutableName";
static constexpr const char * JSON_SETUP_EXECUTABLE_NAME_PROPERTY_NAME = "setupExecutableName";
static constexpr const char * JSON_GROUP_FILE_INSTALL_PATH_PROPERTY_NAME = "groupFileInstallPath";
static constexpr const char * JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME = "relativeConFilePath";
static constexpr const char * JSON_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME = "supportsSubdirectories";
static constexpr const char * JSON_SUPPORTS_WORLD_TOUR_GROUP_PROPERTY_NAME = "worldTourGroupSupported";
static constexpr const char * JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME = "conFileArgumentFlag";
static constexpr const char * JSON_EXTRA_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME = "extraConFileArgumentFlag";
static constexpr const char * JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME = "groupFileArgumentFlag";
static constexpr const char * JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME = "defFileArgumentFlag";
static constexpr const char * JSON_EXTRA_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME = "extraDefFileArgumentFlag";
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
static constexpr const char * JSON_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME = "localWorkingDirectory";
static constexpr const char * JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME = "modDirectoryName";
static constexpr const char * JSON_WEBSITE_PROPERTY_NAME = "website";
static constexpr const char * JSON_SOURCE_CODE_URL_PROPERTY_NAME = "sourceCodeURL";
static constexpr const char * JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME = "supportedOperatingSystems";
static constexpr const char * JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME = "compatibleGameVersions";
static constexpr const char * JSON_NOTES_PROPERTY_NAME = "notes";
static const std::array<std::string_view, 41> JSON_PROPERTY_NAMES = {
	JSON_ID_PROPERTY_NAME,
	JSON_LONG_NAME_PROPERTY_NAME,
	JSON_SHORT_NAME_PROPERTY_NAME,
	JSON_VERSION_PROPERTY_NAME,
	JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME,
	JSON_LAST_PLAYED_TIMESTAMP_PROPERTY_NAME,
	JSON_BASE_PROPERTY_NAME,
	JSON_REMOVABLE_PROPERTY_NAME,
	JSON_GAME_PATH_PROPERTY_NAME,
	JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME,
	JSON_SETUP_EXECUTABLE_NAME_PROPERTY_NAME,
	JSON_GROUP_FILE_INSTALL_PATH_PROPERTY_NAME,
	JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME,
	JSON_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME,
	JSON_SUPPORTS_WORLD_TOUR_GROUP_PROPERTY_NAME,
	JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_EXTRA_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_EXTRA_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME,
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
	JSON_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME,
	JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME,
	JSON_WEBSITE_PROPERTY_NAME,
	JSON_SOURCE_CODE_URL_PROPERTY_NAME,
	JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME,
	JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME,
	JSON_NOTES_PROPERTY_NAME
};

const std::string GameVersion::ALL_VERSIONS("all");
const std::string GameVersion::STANDALONE("stand-alone");
const std::string GameVersion::STANDALONE_DIRECTORY_NAME("stand-alone");

// Note: Duke Nukem 3D: 1.3D & Atomic Edition 1.5 game version identifiers are pre-defined separately in order to avoid a circular dependency:
static const std::string REGULAR_ID("regular");
static const std::string ATOMIC_ID("atomic");
const GameVersion GameVersion::LAMEDUKE                ("lameduke",   "Duke Nukem 3D Beta 1.3.95 (LameDuke)",       "LameDuke",                 false, "", "D3D.EXE",            true,  false, false, false, "LameDuke",  {},    {},     {},    {},      "/v", "/l", "/s", 0, "/r", {},    {},   {},   {},   {},    {},    "SETUP.EXE", {}, {},    {},     {},    true, "https://www.dukenukem.com",                                     "",                                                                         { OperatingSystem::DOS });
const GameVersion GameVersion::ORIGINAL_BETA_VERSION   ("beta",       "Duke Nukem 3D Beta 0.99",                    "Duke 3D Beta 0.99",        false, "", "DUKE3D.EXE",         true,  false, false, false, "Beta",      {},    {},     {},    {},      "/v", "/l", "/s", 0, "/r", {},    "/t", {},   "/m", {},    {},    "SETUP.EXE", "", {},    {},     {},    true, "https://www.dukenukem.com",                                     "",                                                                         { OperatingSystem::DOS },                                                     { REGULAR_ID },                                                                                                                           { "Has extremely poor support for mods.", "Does not function properly out of the box." });
const GameVersion GameVersion::ORIGINAL_REGULAR_VERSION(REGULAR_ID,   "Duke Nukem 3D 1.3D",                         "Duke 3D 1.3D",             false, "", "DUKE3D.EXE",         true,  false, true,  false, "Regular",   "/x",  {},     "/g",  "-map ", "/v", "/l", "/s", 1, "/r", {},    "/t", "/u", "/m", "/ns", "/nm", "SETUP.EXE", "", {},    {},     true,  {},   "https://www.dukenukem.com",                                     "",                                                                         { OperatingSystem::DOS },                                                     { ORIGINAL_BETA_VERSION.getID() });
const GameVersion GameVersion::ORIGINAL_PLUTONIUM_PAK  ("plutonium",  "Duke Nukem 3D: Plutonium Pak 1.4",           "Plutonium Pak",            false, "", "DUKE3D.EXE",         true,  false, true,  false, "PlutPak",   "/x",  {},     "/g",  "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", "SETUP.EXE", "", {},    {},     {},    {},   "https://www.dukenukem.com",                                     "",                                                                         { OperatingSystem::DOS },                                                     { ATOMIC_ID },                                                                                                                            { "Virtually identical to Duke Nukem 3D Atomic Edition." });
const GameVersion GameVersion::ORIGINAL_ATOMIC_EDITION (ATOMIC_ID,    "Duke Nukem 3D: Atomic Edition 1.5",          "Atomic Edition",           false, "", "DUKE3D.EXE",         true,  false, true,  true,  "Atomic",    "/x",  {},     "/g",  "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", "SETUP.EXE", "", {},    {},     {},    {},   "https://www.dukenukem.com",                                     "",                                                                         { OperatingSystem::DOS },                                                     { ORIGINAL_PLUTONIUM_PAK.getID() });
const GameVersion GameVersion::JFDUKE3D                ("jfduke3d",   "JFDuke3D",                                   "JFDuke3D",                 false, "", "duke3d.exe",         true,  false, true,  true,  "JFDuke3D",  "/x",  {},     "/g",  "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", {},          {}, {},    {},     {},    {},   "http://www.jonof.id.au/jfduke3d",                               "https://github.com/jonof/jfduke3d",                                        { OperatingSystem::Windows, OperatingSystem::MacOS },                         { ORIGINAL_REGULAR_VERSION.getID(), ORIGINAL_PLUTONIUM_PAK.getID(), ORIGINAL_ATOMIC_EDITION.getID() });
const GameVersion GameVersion::EDUKE32                 ("eduke32",    "eDuke32",                                    "eDuke32",                  false, "", "eduke32.exe",        false, true,  true,  true,  "eDuke32",   "-x ", "-mx ", "-g ", "-map ", "-v", "-l", "-s", 1, "-r", "-d ", "-t", "-u", "-m", "-ns", "-nm", {},          {}, "-h ", "-mh ", {},    {},   "https://www.eduke32.com",                                       "https://voidpoint.io/terminx/eduke32",                                     { OperatingSystem::Windows },                                                 { ORIGINAL_REGULAR_VERSION.getID(), ORIGINAL_PLUTONIUM_PAK.getID(), ORIGINAL_ATOMIC_EDITION.getID(), JFDUKE3D.getID() });
const GameVersion GameVersion::NETDUKE32               ("netduke32",  "NetDuke32",                                  "NetDuke32",                true,  "", "netduke32.exe",      false, true,  true,  true,  "NetDuke",   "-x ", "-mx ", "-g ", "-map ", "-v", "-l", "-s", 1, "-r", "-d ", "-t", "-u", "-m", "-ns", "-nm", {},          {}, "-h ", "-mh ", {},    {},   "https://wiki.eduke32.com/wiki/NetDuke32",                       "https://voidpoint.io/StrikerTheHedgefox/eduke32-csrefactor/-/tree/master", { OperatingSystem::Windows },                                                 { ORIGINAL_REGULAR_VERSION.getID(), ORIGINAL_PLUTONIUM_PAK.getID(), ORIGINAL_ATOMIC_EDITION.getID(), JFDUKE3D.getID(), EDUKE32.getID() });
//const GameVersion GameVersion::MEGATON_EDITION         ("megaton",    "Duke Nukem 3D: Megaton Edition",             "Megaton Edition",          true,  "", "duke3d.exe",         true,  false, true,  true,  "Megaton",   "/x",  {},     "/g",  "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", {},          {}, {},    {},     {},    {},   "https://store.steampowered.com/app/225140",                     "https://github.com/TermiT/duke3d-megaton",                                 { OperatingSystem::Windows },                                                 { ORIGINAL_REGULAR_VERSION.getID(), ORIGINAL_PLUTONIUM_PAK.getID(), ORIGINAL_ATOMIC_EDITION.getID(), JFDUKE3D.getID() });
//const GameVersion GameVersion::WORLD_TOUR              ("world_tour", "Duke Nukem 3D: 20th Anniversary World Tour", "World Tour",               true,  "", "duke3d.exe",         true,  false, true,  true,  "WorldTour", "/x",  {},     "/g",  "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", {},          {}, {},    {},     {},    {},   "https://www.gearboxsoftware.com/game/duke-3d-20th",             "",                                                                         { OperatingSystem::Windows },                                                 { ORIGINAL_REGULAR_VERSION.getID(), ORIGINAL_PLUTONIUM_PAK.getID(), ORIGINAL_ATOMIC_EDITION.getID() });
//const GameVersion GameVersion::BUILDGDX                ("buildgdx",   "BuildGDX",                                   "BuildGDX".                 true,  "", "BuildGDX.jar",       true,  true,  true,  true,  "BuildGDX",  "",    {},     "",    "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", {},          {}, {},    {},     {},    {},   "https://m210.duke4.net/index.php/downloads/category/8-java",    "https://gitlab.com/m210/BuildGDX",                                         { OperatingSystem::Windows, OperatingSystem::Linux, OperatingSystem::MacOS }, { ORIGINAL_REGULAR_VERSION.getID(), ORIGINAL_PLUTONIUM_PAK.getID(), ORIGINAL_ATOMIC_EDITION.getID() });
const GameVersion GameVersion::RAZE                    ("raze",       "Raze",                                       "Raze",                     true,  "", "raze.exe",           true,  true,  true,  true,  "Raze",      "-x ", {},     "-g ", "-map ", "-v", "-l", "-s", 1, "-r", "-d ", "-t", "-u", "-m", "-ns", "-nm", {},          {}, {},    {},     {},    {},   "https://raze.zdoom.org/about",                                  "https://github.com/coelckers/Raze",                                        { OperatingSystem::Windows, OperatingSystem::Linux, OperatingSystem::MacOS }, { ORIGINAL_PLUTONIUM_PAK.getID(), ORIGINAL_ATOMIC_EDITION.getID(),  JFDUKE3D.getID() });
const GameVersion GameVersion::RED_NUKEM               ("rednukem",   "RedNukem",                                   "RedNukem",                 true,  "", "rednukem.exe",       false, true,  true,  true,  "RedNukem",  "-x ", "-mx ", "-g ", "-map ", "-v", "-l", "-s", 1, "-r", "-d ", "-t", "-u", "-m", "-ns", "-nm", {},          {}, "-h ", "-mh ", {},    {},   "https://lerppu.net/wannabethesis",                              "https://github.com/nukeykt/NRedneck",                                      { OperatingSystem::Windows },                                                 { ORIGINAL_PLUTONIUM_PAK.getID(), ORIGINAL_ATOMIC_EDITION.getID(),  JFDUKE3D.getID() });
//const GameVersion GameVersion::CHOCOLATE_DUKE3D        ("chocolate",  "Chocolate Duke Nukem 3D",                    "Chocolate Duke3D",         true,  "", "Game.exe",           true,  false, false, true,  "Chocolate", "/x",  {},     "/g",  "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", {},          {}, {},    {},     {},    {},   "https://fabiensanglard.net/duke3d/chocolate_duke_nukem_3D.php", "https://github.com/fabiensanglard/chocolate_duke3D",                       { OperatingSystem::Windows },                                                 { ORIGINAL_REGULAR_VERSION.getID(), ORIGINAL_PLUTONIUM_PAK.getID(), ORIGINAL_ATOMIC_EDITION.getID() });
const GameVersion GameVersion::BELGIAN_CHOCOLATE_DUKE3D("belgian",    "Belgian Chocolate Duke Nukem 3D",            "Belgian Chocolate Duke3D", true,  "", "ChocoDuke3D.64.exe", true,  false, false, true,  "Belgian",   "/x",  {},     "/g",  "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", {},          "", {},    {},     {},    {},   "",                                                              "https://github.com/GPSnoopy/BelgianChocolateDuke3D",                       { OperatingSystem::Windows, OperatingSystem::Linux, OperatingSystem::MacOS }, { ORIGINAL_REGULAR_VERSION.getID(), ORIGINAL_PLUTONIUM_PAK.getID(), ORIGINAL_ATOMIC_EDITION.getID() });
const GameVersion GameVersion::DUKE3DW                 ("duke3dw",    "Duke3dw",                                    "Duke3dw",                  true,  "", "Duke3dw.exe",        true,  false, true,  true,  "Duke3dw",   "/x",  {},     "/g",  "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", {},          "", "/h",  {},     {},    {},   "http://www.proasm.com/duke/Duke3dw.html",                       "",                                                                         { OperatingSystem::Windows },                                                 { ORIGINAL_REGULAR_VERSION.getID(), ORIGINAL_PLUTONIUM_PAK.getID(), ORIGINAL_ATOMIC_EDITION.getID(), JFDUKE3D.getID() });
const GameVersion GameVersion::PKDUKE3D                ("pkduke3d",   "pkDuke3D",                                   "pkDuke3D",                 true,  "", "pkDuke3d.exe",       true,  false, true,  true,  "pkDuke3D",  "/x",  {},     "/g",  "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", {},          "", {},    {},     {},    {},   "https://bitbucket.org/pogokeen/pkduke3d/downloads",             "https://bitbucket.org/pogokeen/pkduke3d",                                  { OperatingSystem::Windows },                                                 { ORIGINAL_REGULAR_VERSION.getID(), ORIGINAL_PLUTONIUM_PAK.getID(), ORIGINAL_ATOMIC_EDITION.getID(), JFDUKE3D.getID() },                  { "Has some issues running mods, such as missing episode names." });
const GameVersion GameVersion::XDUKE                   ("xduke",      "xDuke",                                      "xDuke",                    true,  "", "duke3d_w32.exe",     true,  false, false, false, "xDuke",     "/x",  {},     "/g",  "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", {},          "", {},    {},     {},    {},   "http://vision.gel.ulaval.ca/~klein/duke3d",                     "",                                                                         { OperatingSystem::Windows },                                                 { ORIGINAL_PLUTONIUM_PAK.getID(), ORIGINAL_ATOMIC_EDITION.getID() });
const GameVersion GameVersion::RDUKE                   ("rduke",      "rDuke",                                      "rDuke",                    true,  "", "rduke_r10.exe",      true,  false, false, true,  "rDuke",     "/x",  {},     "/g",  "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", {},          "", {},    {},     {},    {},   "",                                                              "https://github.com/radar-duker/radars-xduke-fork",                         { OperatingSystem::Windows },                                                 { ORIGINAL_PLUTONIUM_PAK.getID(), ORIGINAL_ATOMIC_EDITION.getID() });
const GameVersion GameVersion::DUKE3D_W32              ("duke3d_w32", "Duke3d_w32",                                 "Duke3d_w32",               true,  "", "duke3d_w32.exe",     true,  false, false, true,  "Duke_w32",  "/x",  {},     "/g",  "-map ", "/v", "/l", "/s", 1, "/r", "/d",  "/t", "/u", "/m", "/ns", "/nm", {},          "", {},    {},     {},    {},   "http://www.rancidmeat.com/project.php3?id=1",                   "",                                                                         { OperatingSystem::Windows },                                                 { ORIGINAL_PLUTONIUM_PAK.getID(), ORIGINAL_ATOMIC_EDITION.getID() });

const std::vector<const GameVersion *> GameVersion::DEFAULT_GAME_VERSIONS = {
	&LAMEDUKE,
	&ORIGINAL_BETA_VERSION,
	&ORIGINAL_REGULAR_VERSION,
	&ORIGINAL_PLUTONIUM_PAK,
	&ORIGINAL_ATOMIC_EDITION,
	&JFDUKE3D,
	&EDUKE32,
	&NETDUKE32,
//	&MEGATON_EDITION,
//	&WORLD_TOUR,
//	&BUILDGDX,
	&RAZE,
	&RED_NUKEM,
//	&CHOCOLATE_DUKE3D,
	&BELGIAN_CHOCOLATE_DUKE3D,
	&DUKE3DW,
	&PKDUKE3D,
	&XDUKE,
	&RDUKE,
	&DUKE3D_W32
};

const bool GameVersion::DEFAULT_LOCAL_WORKING_DIRECTORY = true;
const bool GameVersion::DEFAULT_RELATIVE_CON_FILE_PATH = false;
const bool GameVersion::DEFAULT_SUPPORTS_SUBDIRECTORIES = true;
const bool GameVersion::DEFAULT_WORLD_TOUR_GROUP_SUPPORTED = true;
const uint8_t GameVersion::DEFAULT_SKILL_START_VALUE = 1;

GameVersion::GameVersion()
	: m_standAlone(false)
	, m_removable(true)
	, m_localWorkingDirectory(DEFAULT_LOCAL_WORKING_DIRECTORY)
	, m_relativeConFilePath(DEFAULT_RELATIVE_CON_FILE_PATH)
	, m_supportsSubdirectories(DEFAULT_SUPPORTS_SUBDIRECTORIES)
	, m_worldTourGroupSupported(DEFAULT_WORLD_TOUR_GROUP_SUPPORTED)
	, m_skillStartValue(DEFAULT_SKILL_START_VALUE)
	, m_modified(false) { }

GameVersion::GameVersion(const std::string & id, const std::string & longName, const std::string & shortName, bool removable, const std::string & gamePath, const std::string & gameExecutableName, bool localWorkingDirectory, bool relativeConFilePath, bool supportsSubdirectories, std::optional<bool> worldTourGroupSupported, const std::string & modDirectoryName, const std::optional<std::string> & conFileArgumentFlag, const std::optional<std::string> & extraConFileArgumentFlag, const std::optional<std::string> & groupFileArgumentFlag, const std::optional<std::string> & mapFileArgumentFlag, const std::string & episodeArgumentFlag, const std::string & levelArgumentFlag, const std::string & skillArgumentFlag, uint8_t skillStartValue, const std::string & recordDemoArgumentFlag, const std::optional<std::string> & playDemoArgumentFlag, const std::optional<std::string> & respawnModeArgumentFlag, const std::optional<std::string> & weaponSwitchOrderArgumentFlag, const std::optional<std::string> & disableMonstersArgumentFlag, const std::optional<std::string> & disableSoundArgumentFlag, const std::optional<std::string> & disableMusicArgumentFlag, const std::optional<std::string> & setupExecutableName, const std::optional<std::string> & groupFileInstallPath, const std::optional<std::string> & defFileArgumentFlag, const std::optional<std::string> & extraDefFileArgumentFlag,const std::optional<bool> & requiresCombinedGroup, const std::optional<bool> & requiresGroupFileExtraction, const std::string & website, const std::string & sourceCodeURL, const std::vector<OperatingSystem> & supportedOperatingSystems, const std::vector<std::string> & compatibleGameVersionIdentifiers, const std::vector<std::string> & notes)
	: m_id(Utilities::trimString(id))
	, m_longName(Utilities::trimString(longName))
	, m_shortName(Utilities::trimString(shortName))
	, m_standAlone(false)
	, m_removable(removable)
	, m_gamePath(Utilities::trimString(gamePath))
	, m_gameExecutableName(Utilities::trimString(gameExecutableName))
	, m_requiresCombinedGroup(requiresCombinedGroup)
	, m_requiresGroupFileExtraction(requiresGroupFileExtraction)
	, m_modDirectoryName(Utilities::trimString(modDirectoryName))
	, m_website(Utilities::trimString(website))
	, m_sourceCodeURL(Utilities::trimString(sourceCodeURL))
	, m_localWorkingDirectory(localWorkingDirectory)
	, m_relativeConFilePath(relativeConFilePath)
	, m_supportsSubdirectories(supportsSubdirectories)
	, m_worldTourGroupSupported(worldTourGroupSupported)
	, m_conFileArgumentFlag(conFileArgumentFlag)
	, m_extraConFileArgumentFlag(extraConFileArgumentFlag)
	, m_groupFileArgumentFlag(groupFileArgumentFlag)
	, m_defFileArgumentFlag(defFileArgumentFlag)
	, m_extraDefFileArgumentFlag(extraDefFileArgumentFlag)
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
	, m_notes(notes)
	, m_modified(false) {
	if(!gamePath.empty()) {
		m_installedTimePoint = std::chrono::system_clock::now();
	}

	if(setupExecutableName.has_value()) {
		m_setupExecutableName = Utilities::trimString(setupExecutableName.value());
	}

	if(groupFileInstallPath.has_value()) {
		m_groupFileInstallPath = Utilities::trimString(groupFileInstallPath.value());
	}

	addSupportedOperatingSystems(supportedOperatingSystems);
	addCompatibleGameVersionIdentifiers(compatibleGameVersionIdentifiers);

	setModified(false);
}

GameVersion::GameVersion(GameVersion && gameVersion) noexcept
	: m_id(std::move(gameVersion.m_id))
	, m_longName(std::move(gameVersion.m_longName))
	, m_shortName(std::move(gameVersion.m_shortName))
	, m_installedTimePoint(std::move(gameVersion.m_installedTimePoint))
	, m_lastPlayedTimePoint(std::move(gameVersion.m_lastPlayedTimePoint))
	, m_standAlone(gameVersion.m_standAlone)
	, m_base(std::move(gameVersion.m_base))
	, m_removable(gameVersion.m_removable)
	, m_gamePath(std::move(gameVersion.m_gamePath))
	, m_gameExecutableName(std::move(gameVersion.m_gameExecutableName))
	, m_setupExecutableName(std::move(gameVersion.m_setupExecutableName))
	, m_groupFileInstallPath(std::move(gameVersion.m_groupFileInstallPath))
	, m_requiresCombinedGroup(gameVersion.m_requiresCombinedGroup)
	, m_requiresGroupFileExtraction(gameVersion.m_requiresGroupFileExtraction)
	, m_localWorkingDirectory(gameVersion.m_localWorkingDirectory)
	, m_modDirectoryName(std::move(gameVersion.m_modDirectoryName))
	, m_website(std::move(gameVersion.m_website))
	, m_sourceCodeURL(std::move(gameVersion.m_sourceCodeURL))
	, m_relativeConFilePath(gameVersion.m_relativeConFilePath)
	, m_supportsSubdirectories(gameVersion.m_supportsSubdirectories)
	, m_worldTourGroupSupported(gameVersion.m_worldTourGroupSupported)
	, m_conFileArgumentFlag(std::move(gameVersion.m_conFileArgumentFlag))
	, m_extraConFileArgumentFlag(std::move(gameVersion.m_extraConFileArgumentFlag))
	, m_groupFileArgumentFlag(std::move(gameVersion.m_groupFileArgumentFlag))
	, m_defFileArgumentFlag(std::move(gameVersion.m_defFileArgumentFlag))
	, m_extraDefFileArgumentFlag(std::move(gameVersion.m_extraDefFileArgumentFlag))
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
	, m_compatibleGameVersionIdentifiers(std::move(gameVersion.m_compatibleGameVersionIdentifiers))
	, m_notes(std::move(gameVersion.m_notes))
	, m_modified(false) { }

GameVersion::GameVersion(const GameVersion & gameVersion)
	: m_id(gameVersion.m_id)
	, m_longName(gameVersion.m_longName)
	, m_shortName(gameVersion.m_shortName)
	, m_installedTimePoint(gameVersion.m_installedTimePoint)
	, m_lastPlayedTimePoint(gameVersion.m_lastPlayedTimePoint)
	, m_standAlone(gameVersion.m_standAlone)
	, m_base(gameVersion.m_base)
	, m_removable(gameVersion.m_removable)
	, m_gamePath(gameVersion.m_gamePath)
	, m_gameExecutableName(gameVersion.m_gameExecutableName)
	, m_setupExecutableName(gameVersion.m_setupExecutableName)
	, m_groupFileInstallPath(gameVersion.m_groupFileInstallPath)
	, m_requiresCombinedGroup(gameVersion.m_requiresCombinedGroup)
	, m_requiresGroupFileExtraction(gameVersion.m_requiresGroupFileExtraction)
	, m_localWorkingDirectory(gameVersion.m_localWorkingDirectory)
	, m_modDirectoryName(gameVersion.m_modDirectoryName)
	, m_website(gameVersion.m_website)
	, m_sourceCodeURL(gameVersion.m_sourceCodeURL)
	, m_relativeConFilePath(gameVersion.m_relativeConFilePath)
	, m_supportsSubdirectories(gameVersion.m_supportsSubdirectories)
	, m_worldTourGroupSupported(gameVersion.m_worldTourGroupSupported)
	, m_conFileArgumentFlag(gameVersion.m_conFileArgumentFlag)
	, m_extraConFileArgumentFlag(gameVersion.m_extraConFileArgumentFlag)
	, m_groupFileArgumentFlag(gameVersion.m_groupFileArgumentFlag)
	, m_defFileArgumentFlag(gameVersion.m_defFileArgumentFlag)
	, m_extraDefFileArgumentFlag(gameVersion.m_extraDefFileArgumentFlag)
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
	, m_compatibleGameVersionIdentifiers(gameVersion.m_compatibleGameVersionIdentifiers)
	, m_notes(gameVersion.m_notes)
	, m_modified(false)  { }

GameVersion & GameVersion::operator = (GameVersion && gameVersion) noexcept {
	if(this != &gameVersion) {
		m_id = std::move(gameVersion.m_id);
		m_longName = std::move(gameVersion.m_longName);
		m_shortName = std::move(gameVersion.m_shortName);
		m_installedTimePoint = std::move(gameVersion.m_installedTimePoint);
		m_lastPlayedTimePoint = std::move(gameVersion.m_lastPlayedTimePoint);
		m_standAlone = gameVersion.m_standAlone;
		m_base = std::move(gameVersion.m_base);
		m_removable = gameVersion.m_removable;
		m_gamePath = std::move(gameVersion.m_gamePath);
		m_gameExecutableName = std::move(gameVersion.m_gameExecutableName);
		m_setupExecutableName = std::move(gameVersion.m_setupExecutableName);
		m_groupFileInstallPath = std::move(gameVersion.m_groupFileInstallPath);
		m_requiresCombinedGroup = gameVersion.m_requiresCombinedGroup;
		m_requiresGroupFileExtraction = gameVersion.m_requiresGroupFileExtraction;
		m_localWorkingDirectory = gameVersion.m_localWorkingDirectory;
		m_modDirectoryName = std::move(gameVersion.m_modDirectoryName);
		m_website = std::move(gameVersion.m_website);
		m_sourceCodeURL = std::move(gameVersion.m_sourceCodeURL);
		m_relativeConFilePath = gameVersion.m_relativeConFilePath;
		m_supportsSubdirectories = gameVersion.m_supportsSubdirectories;
		m_worldTourGroupSupported = gameVersion.m_worldTourGroupSupported;
		m_conFileArgumentFlag = std::move(gameVersion.m_conFileArgumentFlag);
		m_extraConFileArgumentFlag = std::move(gameVersion.m_extraConFileArgumentFlag);
		m_groupFileArgumentFlag = std::move(gameVersion.m_groupFileArgumentFlag);
		m_defFileArgumentFlag = std::move(gameVersion.m_defFileArgumentFlag);
		m_extraDefFileArgumentFlag = std::move(gameVersion.m_extraDefFileArgumentFlag);
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
		m_compatibleGameVersionIdentifiers = std::move(gameVersion.m_compatibleGameVersionIdentifiers);
		m_notes = std::move(gameVersion.m_notes);
		setModified(true);
	}

	return *this;
}

GameVersion & GameVersion::operator = (const GameVersion & gameVersion) {
	m_id = gameVersion.m_id;
	m_longName = gameVersion.m_longName;
	m_shortName = gameVersion.m_shortName;
	m_installedTimePoint = gameVersion.m_installedTimePoint;
	m_lastPlayedTimePoint = gameVersion.m_lastPlayedTimePoint;
	m_standAlone = gameVersion.m_standAlone;
	m_base = gameVersion.m_base;
	m_removable = gameVersion.m_removable;
	m_gamePath = gameVersion.m_gamePath;
	m_gameExecutableName = gameVersion.m_gameExecutableName;
	m_setupExecutableName = gameVersion.m_setupExecutableName;
	m_groupFileInstallPath = gameVersion.m_groupFileInstallPath;
	m_requiresCombinedGroup = gameVersion.m_requiresCombinedGroup;
	m_requiresGroupFileExtraction = gameVersion.m_requiresGroupFileExtraction;
	m_localWorkingDirectory = gameVersion.m_localWorkingDirectory;
	m_modDirectoryName = gameVersion.m_modDirectoryName;
	m_website = gameVersion.m_website;
	m_sourceCodeURL = gameVersion.m_sourceCodeURL;
	m_relativeConFilePath = gameVersion.m_relativeConFilePath;
	m_supportsSubdirectories = gameVersion.m_supportsSubdirectories;
	m_worldTourGroupSupported = gameVersion.m_worldTourGroupSupported;
	m_conFileArgumentFlag = gameVersion.m_conFileArgumentFlag;
	m_extraConFileArgumentFlag = gameVersion.m_extraConFileArgumentFlag;
	m_groupFileArgumentFlag = gameVersion.m_groupFileArgumentFlag;
	m_defFileArgumentFlag = gameVersion.m_defFileArgumentFlag;
	m_extraDefFileArgumentFlag = gameVersion.m_extraDefFileArgumentFlag;
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
	m_compatibleGameVersionIdentifiers = gameVersion.m_compatibleGameVersionIdentifiers;
	m_notes = gameVersion.m_notes;

	setModified(true);

	return *this;
}

GameVersion::~GameVersion() { }

bool GameVersion::isModified() const {
	return m_modified;
}

void GameVersion::setModified(bool value) {
	m_modified = value;

	modified(*this);
}

bool GameVersion::hasID() const {
	return !m_id.empty();
}

const std::string & GameVersion::getID() const {
	return m_id;
}

bool GameVersion::setID(const std::string & id) {
	std::string formattedID(Utilities::trimString(id));

	if(Utilities::areStringsEqual(m_id, formattedID)) {
		return true;
	}

	m_id = std::move(formattedID);

	setModified(true);

	return true;
}

bool GameVersion::hasLongName() const {
	return !m_longName.empty();
}

const std::string & GameVersion::getLongName() const {
	return m_longName;
}

bool GameVersion::setLongName(const std::string & longName) {
	std::string formattedLongName(Utilities::trimString(longName));

	if(Utilities::areStringsEqual(m_longName, formattedLongName)) {
		return true;
	}

	m_longName = std::move(formattedLongName);

	setModified(true);

	return true;
}

bool GameVersion::hasShortName() const {
	return !m_shortName.empty();
}

const std::string & GameVersion::getShortName() const {
	return m_shortName;
}

bool GameVersion::setShortName(const std::string & shortName) {
	std::string formattedShortName(Utilities::trimString(shortName));

	if(Utilities::areStringsEqual(m_shortName, formattedShortName)) {
		return true;
	}

	m_shortName = std::move(formattedShortName);

	setModified(true);

	return true;
}

bool GameVersion::hasInstalledTimePoint() const {
	return m_installedTimePoint.has_value();
}

const std::optional<std::chrono::time_point<std::chrono::system_clock>> & GameVersion::getInstalledTimePoint() const {
	return m_installedTimePoint;
}

void GameVersion::setInstalledTimePoint(std::chrono::time_point<std::chrono::system_clock> installedTimePoint) {
	if(m_installedTimePoint == installedTimePoint) {
		return;
	}

	m_installedTimePoint = installedTimePoint;

	setModified(true);
}

void GameVersion::clearInstalledTimePoint() {
	m_installedTimePoint.reset();
}

bool GameVersion::hasLastPlayedTimePoint() const {
	return m_lastPlayedTimePoint.has_value();
}

const std::optional<std::chrono::time_point<std::chrono::system_clock>> & GameVersion::getLastPlayedTimePoint() const {
	return m_lastPlayedTimePoint;
}

void GameVersion::setLastPlayedTimePoint(std::chrono::time_point<std::chrono::system_clock> lastPlayedTimePoint) {
	if(m_lastPlayedTimePoint == lastPlayedTimePoint) {
		return;
	}

	m_lastPlayedTimePoint = lastPlayedTimePoint;

	setModified(true);
}

void GameVersion::updateLastPlayedTimePoint() {
	setLastPlayedTimePoint(std::chrono::system_clock::now());
}

void GameVersion::clearLastPlayedTimePoint() {
	m_lastPlayedTimePoint.reset();
}

bool GameVersion::isStandAlone() const {
	return m_standAlone;
}

void GameVersion::setStandAlone(bool standAlone) {
	m_standAlone = standAlone;
}

std::string GameVersion::getBase() const {
	return m_base;
}

void GameVersion::setBase(const std::string & base) {
	m_base = base;
}

bool GameVersion::isRemovable() const {
	return m_removable;
}

bool GameVersion::hasGroupFile() const {
	return !Utilities::areStringsEqualIgnoreCase(m_id, LAMEDUKE.getID());
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

	if(m_gamePath.empty()) {
		m_installedTimePoint.reset();
	}
	else {
		m_installedTimePoint = std::chrono::system_clock::now();
	}

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
	return hasSupportedOperatingSystem(OperatingSystem::DOS);
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
	if(isStandAlone()) {
		return STANDALONE_DIRECTORY_NAME;
	}

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

bool GameVersion::isWorldTourGroupSupported() const {
	if(isStandAlone()) {
		return false;
	}

	return m_worldTourGroupSupported.value_or(false);
}

void GameVersion::setWorldTourGroupSupported(bool worldTourGroupSupported) {
	if(m_worldTourGroupSupported == worldTourGroupSupported) {
		return;
	}

	m_worldTourGroupSupported = worldTourGroupSupported;

	setModified(true);
}

std::optional<bool> GameVersion::getWorldTourGroupSupported() const {
	if(isStandAlone()) {
		return {};
	}

	return m_worldTourGroupSupported;
}

void GameVersion::clearWorldTourGroupSupported() {
	if(!m_worldTourGroupSupported.has_value()) {
		return;
	}

	m_worldTourGroupSupported.reset();

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

bool GameVersion::hasExtraConFileArgumentFlag() const {
	return m_extraConFileArgumentFlag.has_value();
}

const std::optional<std::string> & GameVersion::getExtraConFileArgumentFlag() const {
	return m_extraConFileArgumentFlag;
}

bool GameVersion::setExtraConFileArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	if(m_extraConFileArgumentFlag.has_value() && Utilities::areStringsEqual(m_extraConFileArgumentFlag.value(), flag)) {
		return true;
	}

	m_extraConFileArgumentFlag = flag;

	setModified(true);

	return true;
}

void GameVersion::clearExtraConFileArgumentFlag() {
	if(!m_extraConFileArgumentFlag.has_value()) {
		return;
	}

	m_extraConFileArgumentFlag.reset();

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

bool GameVersion::hasExtraDefFileArgumentFlag() const {
	return m_extraDefFileArgumentFlag.has_value();
}

const std::optional<std::string> & GameVersion::getExtraDefFileArgumentFlag() const {
	return m_extraDefFileArgumentFlag;
}

bool GameVersion::setExtraDefFileArgumentFlag(const std::string & flag) {
	if(flag.empty()) {
		return false;
	}

	if(m_extraDefFileArgumentFlag.has_value() && Utilities::areStringsEqual(m_extraDefFileArgumentFlag.value(), flag)) {
		return true;
	}

	m_extraDefFileArgumentFlag = flag;

	setModified(true);

	return true;
}

void GameVersion::clearExtraDefFileArgumentFlag() {
	if(!m_extraDefFileArgumentFlag.has_value()) {
		return;
	}

	m_extraDefFileArgumentFlag.reset();

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

std::string GameVersion::getSupportedOperatingSystemName(size_t index) const {
	if(index < 0 || index >= m_supportedOperatingSystems.size()) {
		return {};
	}

	return std::string(magic_enum::enum_name(m_supportedOperatingSystems[index]));
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
	return m_compatibleGameVersionIdentifiers.size();
}

bool GameVersion::hasCompatibleGameVersion(const GameVersion & gameVersion) const {
	return hasCompatibleGameVersionWithID(gameVersion.getID());
}

bool GameVersion::hasCompatibleGameVersionWithID(const std::string & gameVersionID) const {
	return indexOfCompatibleGameVersionWithID(gameVersionID) != std::numeric_limits<size_t>::max();
}

size_t GameVersion::indexOfCompatibleGameVersionWithID(const std::string & gameVersionID) const {
	auto compatibleGameVersionIterator = std::find_if(m_compatibleGameVersionIdentifiers.cbegin(), m_compatibleGameVersionIdentifiers.cend(), [&gameVersionID](const std::string & currentGameVersionID) {
		return Utilities::areStringsEqualIgnoreCase(gameVersionID, currentGameVersionID);
	});

	if(compatibleGameVersionIterator == m_compatibleGameVersionIdentifiers.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return compatibleGameVersionIterator - m_compatibleGameVersionIdentifiers.cbegin();
}

size_t GameVersion::indexOfCompatibleGameVersion(const GameVersion & gameVersion) const {
	return indexOfCompatibleGameVersionWithID(gameVersion.getID());
}

std::optional<std::string> GameVersion::getCompatibleGameVersionID(size_t index) const {
	if(index >= m_compatibleGameVersionIdentifiers.size()) {
		return {};
	}

	return m_compatibleGameVersionIdentifiers[index];
}

const std::vector<std::string> & GameVersion::getCompatibleGameVersionIdentifiers() const {
	return m_compatibleGameVersionIdentifiers;
}

std::shared_ptr<ModGameVersion> GameVersion::getMostCompatibleModGameVersion(const std::vector<std::shared_ptr<ModGameVersion>> & modGameVersions) const {
	for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = modGameVersions.begin(); i != modGameVersions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase(m_id, (*i)->getGameVersionID())) {
			return *i;
		}
	}

	for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = modGameVersions.begin(); i != modGameVersions.end(); ++i) {
		if(hasCompatibleGameVersionWithID((*i)->getGameVersionID())) {
			return *i;
		}
	}

	return nullptr;
}

std::vector<std::shared_ptr<ModGameVersion>> GameVersion::getCompatibleModGameVersions(const std::vector<std::shared_ptr<ModGameVersion>> & modGameVersions) const {
	std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions;

	for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = modGameVersions.begin(); i != modGameVersions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase(m_id, (*i)->getGameVersionID())) {
			compatibleModGameVersions.insert(compatibleModGameVersions.begin(), *i);
			continue;
		}

		if(hasCompatibleGameVersionWithID((*i)->getGameVersionID())) {
			compatibleModGameVersions.push_back(*i);
		}
	}

	return compatibleModGameVersions;
}

bool GameVersion::addCompatibleGameVersion(const GameVersion & gameVersion) {
	return addCompatibleGameVersionWithID(gameVersion.getID());
}

bool GameVersion::addCompatibleGameVersionWithID(const std::string & gameVersionID) {
	if(gameVersionID.empty() || hasCompatibleGameVersionWithID(gameVersionID)) {
		return false;
	}

	m_compatibleGameVersionIdentifiers.emplace_back(gameVersionID);

	setModified(true);

	return true;
}

size_t GameVersion::addCompatibleGameVersionIdentifiers(const std::vector<std::string> & gameVersionIdentifiers) {
	size_t numberOfCompatibleGameVersionsAdded = 0;

	for(std::vector<std::string>::const_iterator i = gameVersionIdentifiers.begin(); i != gameVersionIdentifiers.end(); ++i) {
		if(addCompatibleGameVersionWithID(*i)) {
			numberOfCompatibleGameVersionsAdded++;
		}
	}

	return numberOfCompatibleGameVersionsAdded;
}

bool GameVersion::removeCompatibleGameVersion(size_t index) {
	if(index >= m_compatibleGameVersionIdentifiers.size()) {
		return false;
	}

	m_compatibleGameVersionIdentifiers.erase(m_compatibleGameVersionIdentifiers.begin() + index);

	setModified(true);

	return true;
}

bool GameVersion::removeCompatibleGameVersion(const GameVersion & gameVersion) {
	return removeCompatibleGameVersion(indexOfCompatibleGameVersion(gameVersion));
}

bool GameVersion::removeCompatibleGameVersionWithID(const std::string & gameVersionID) {
	return removeCompatibleGameVersion(indexOfCompatibleGameVersionWithID(gameVersionID));
}

void GameVersion::clearCompatibleGameVersions() {
	if(m_compatibleGameVersionIdentifiers.empty()) {
		return;
	}

	m_compatibleGameVersionIdentifiers.clear();

	setModified(true);
}

size_t GameVersion::numberOfNotes() const {
	return m_notes.size();
}

bool GameVersion::hasNote(const std::string & note) const {
	for(std::vector<std::string>::const_iterator i = m_notes.begin(); i != m_notes.end(); ++i) {
		if(*i == note) {
			return true;
		}
	}

	return false;
}

size_t GameVersion::indexOfNote(const std::string & note) const {
	for(size_t i = 0; i < m_notes.size(); i++) {
		if(m_notes[i] == note) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::string GameVersion::getNote(size_t index) const {
	if(index >= m_notes.size()) {
		return {};
	}

	return m_notes[index];
}

const std::vector<std::string> & GameVersion::getNotes() const {
	return m_notes;
}

std::string GameVersion::getNotesAsString() const {
	std::stringstream notesStream;

	for(const std::string & note : m_notes) {
		if(notesStream.tellp() != 0) {
			notesStream << '\n';
		}

		notesStream << note;
	}

	return notesStream.str();
}

bool GameVersion::addNote(const std::string & note) {
	if(note.empty() || hasNote(note)) {
		return false;
	}

	m_notes.emplace_back(note);

	return true;
}

void GameVersion::setNotes(const std::string & notes) {
	m_notes.clear();

	std::string formattedNote;
	std::vector<std::string> notesData(Utilities::regularExpressionStringSplit(notes, "\r?\n"));

	for(const std::string & note : notesData) {
		formattedNote = Utilities::trimString(note);

		if(note.empty()) {
			continue;
		}

		m_notes.emplace_back(std::move(formattedNote));
	}
}

bool GameVersion::removeNote(size_t index) {
	if(index >= m_notes.size()) {
		return false;
	}

	m_notes.erase(m_notes.begin() + index);

	return true;
}

bool GameVersion::removeNote(const std::string & note) {
	for(std::vector<std::string>::const_iterator i = m_notes.begin(); i != m_notes.end(); ++i) {
		if(*i == note) {
			return true;
		}
	}

	return false;
}

void GameVersion::clearNotes() {
	m_notes.clear();
}

void GameVersion::addMetadata(std::map<std::string, std::any> & metadata) const {
	metadata["gameID"] = m_id;
	metadata["longName"] = m_shortName;
	metadata["shortName"] = m_longName;

	if(m_installedTimePoint.has_value()) {
		metadata["installedAt"] = Utilities::timePointToString(m_installedTimePoint.value(), Utilities::TimeFormat::ISO8601);
	}

	if(m_lastPlayedTimePoint.has_value()) {
		metadata["lastPlayedAt"] = Utilities::timePointToString(m_lastPlayedTimePoint.value(), Utilities::TimeFormat::ISO8601);
	}

	metadata["hasGamePath"] = !m_gamePath.empty();
	metadata["gameExecutableName"] = m_gameExecutableName;

	if(m_setupExecutableName.has_value()) {
		metadata["setupExecutableName"] = m_setupExecutableName.value();
	}

	if(m_groupFileInstallPath.has_value()) {
		metadata["groupFileInstallPath"] = m_groupFileInstallPath.value();
	}

	if(m_requiresCombinedGroup.has_value()) {
		metadata["requiresCombinedGroup"] = m_requiresCombinedGroup.value();
	}

	if(m_requiresGroupFileExtraction.has_value()) {
		metadata["requiresGroupFileExtraction"] = m_requiresGroupFileExtraction.value();
	}

	metadata["requiresDOSBox"] = doesRequireDOSBox();
	metadata["localWorkingDirectory"] = m_localWorkingDirectory;
	metadata["relativeConFilePath"] = m_relativeConFilePath;
	metadata["supportsSubdirectories"] = m_supportsSubdirectories;
	metadata["modDirectoryName"] = m_modDirectoryName;

	if(m_worldTourGroupSupported.has_value()) {
		metadata["worldTourGroupSupported"] = m_worldTourGroupSupported.value();
	}

	if(m_conFileArgumentFlag.has_value()) {
		metadata["conFileArgumentFlag"] = m_conFileArgumentFlag.value();
	}

	if(m_extraConFileArgumentFlag.has_value()) {
		metadata["extraConFileArgumentFlag"] = m_extraConFileArgumentFlag.value();
	}

	if(m_groupFileArgumentFlag.has_value()) {
		metadata["groupFileArgumentFlag"] = m_groupFileArgumentFlag.value();
	}

	if(m_defFileArgumentFlag.has_value()) {
		metadata["defFileArgumentFlag"] = m_defFileArgumentFlag.value();
	}

	if(m_extraDefFileArgumentFlag.has_value()) {
		metadata["extraDefFileArgumentFlag"] = m_extraDefFileArgumentFlag.value();
	}

	if(m_mapFileArgumentFlag.has_value()) {
		metadata["mapFileArgumentFlag"] = m_mapFileArgumentFlag.value();
	}

	metadata["episodeArgumentFlag"] = m_episodeArgumentFlag;
	metadata["levelArgumentFlag"] = m_levelArgumentFlag;
	metadata["skillArgumentFlag"] = m_skillArgumentFlag;
	metadata["skillStartValue"] = m_skillStartValue;
	metadata["recordDemoArgumentFlag"] = m_recordDemoArgumentFlag;

	if(m_playDemoArgumentFlag.has_value()) {
		metadata["playDemoArgumentFlag"] = m_playDemoArgumentFlag.value();
	}

	if(m_respawnModeArgumentFlag.has_value()) {
		metadata["respawnModeArgumentFlag"] = m_respawnModeArgumentFlag.value();
	}

	if(m_weaponSwitchOrderArgumentFlag.has_value()) {
		metadata["weaponSwitchOrderArgumentFlag"] = m_weaponSwitchOrderArgumentFlag.value();
	}

	if(m_disableMonstersArgumentFlag.has_value()) {
		metadata["disableMonstersArgumentFlag"] = m_disableMonstersArgumentFlag.value();
	}

	if(m_disableSoundArgumentFlag.has_value()) {
		metadata["disableSoundArgumentFlag"] = m_disableSoundArgumentFlag.value();
	}

	if(m_disableMusicArgumentFlag.has_value()) {
		metadata["disableMusicArgumentFlag"] = m_disableMusicArgumentFlag.value();
	}

	metadata["numberOfSupportedOperatingSystems"] = m_supportedOperatingSystems.size();
	metadata["numberOfCompatibleGameVersions"] = m_compatibleGameVersionIdentifiers.size();
}

std::unique_ptr<GameVersion> GameVersion::createTemplateFrom() const {
	return std::make_unique<GameVersion>(
		Utilities::emptyString,
		Utilities::emptyString,
		Utilities::emptyString,
		true,
		Utilities::emptyString,
		m_gameExecutableName,
		m_localWorkingDirectory,
		m_relativeConFilePath,
		m_supportsSubdirectories,
		m_worldTourGroupSupported,
		Utilities::emptyString,
		m_conFileArgumentFlag,
		m_extraConFileArgumentFlag,
		m_groupFileArgumentFlag,
		m_mapFileArgumentFlag,
		m_episodeArgumentFlag,
		m_levelArgumentFlag,
		m_skillArgumentFlag,
		m_skillStartValue,
		m_recordDemoArgumentFlag,
		m_playDemoArgumentFlag,
		m_respawnModeArgumentFlag,
		m_weaponSwitchOrderArgumentFlag,
		m_disableMonstersArgumentFlag,
		m_disableSoundArgumentFlag,
		m_disableMusicArgumentFlag,
		m_setupExecutableName,
		m_groupFileInstallPath,
		m_defFileArgumentFlag,
		m_extraDefFileArgumentFlag,
		m_requiresCombinedGroup,
		m_requiresGroupFileExtraction
	);
}

size_t GameVersion::checkForMissingExecutables() const {
	if(!isConfigured()) {
		return 0;
	}

	size_t numberOfMissingExecutables = 0;

	std::string fullGameExecutablePath(Utilities::joinPaths(m_gamePath, m_gameExecutableName));

	if(!std::filesystem::is_regular_file(std::filesystem::path(fullGameExecutablePath))) {
		numberOfMissingExecutables++;

		spdlog::error("Missing '{}' game executable: '{}'.", m_longName, fullGameExecutablePath);
	}

	if(m_setupExecutableName.has_value()) {
		std::string fullSetupExecutablePath(Utilities::joinPaths(m_gamePath, m_setupExecutableName.value()));

		if(!std::filesystem::is_regular_file(std::filesystem::path(fullSetupExecutablePath))) {
			numberOfMissingExecutables++;

			spdlog::error("Missing '{}' setup executable: '{}'.", m_longName, fullSetupExecutablePath);
		}
	}

	return numberOfMissingExecutables;
}

rapidjson::Value GameVersion::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value gameVersionValue(rapidjson::kObjectType);

	rapidjson::Value idValue(m_id.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_ID_PROPERTY_NAME), idValue, allocator);

	rapidjson::Value longNameValue(m_longName.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_LONG_NAME_PROPERTY_NAME), longNameValue, allocator);

	rapidjson::Value shortNameValue(m_shortName.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_SHORT_NAME_PROPERTY_NAME), shortNameValue, allocator);

	if(m_installedTimePoint.has_value()) {
		rapidjson::Value installedTimestampValue(Utilities::timePointToString(m_installedTimePoint.value(), Utilities::TimeFormat::ISO8601).c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME), installedTimestampValue, allocator);
	}

	if(m_lastPlayedTimePoint.has_value()) {
		rapidjson::Value lastPlayedTimestampValue(Utilities::timePointToString(m_lastPlayedTimePoint.value(), Utilities::TimeFormat::ISO8601).c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_LAST_PLAYED_TIMESTAMP_PROPERTY_NAME), lastPlayedTimestampValue, allocator);
	}

	if(!m_base.empty()) {
		rapidjson::Value baseValue(m_base.c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_BASE_PROPERTY_NAME), baseValue, allocator);
	}

	gameVersionValue.AddMember(rapidjson::StringRef(JSON_REMOVABLE_PROPERTY_NAME), rapidjson::Value(m_removable), allocator);

	rapidjson::Value gamePathValue(m_gamePath.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_GAME_PATH_PROPERTY_NAME), gamePathValue, allocator);

	rapidjson::Value gameExecutableNameValue(m_gameExecutableName.c_str(), allocator);
	gameVersionValue.AddMember(rapidjson::StringRef(JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME), gameExecutableNameValue, allocator);

	gameVersionValue.AddMember(rapidjson::StringRef(JSON_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME), rapidjson::Value(m_localWorkingDirectory), allocator);

	gameVersionValue.AddMember(rapidjson::StringRef(JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME), rapidjson::Value(m_relativeConFilePath), allocator);

	gameVersionValue.AddMember(rapidjson::StringRef(JSON_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME), rapidjson::Value(m_supportsSubdirectories), allocator);

	if(m_worldTourGroupSupported.has_value()) {
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_SUPPORTS_WORLD_TOUR_GROUP_PROPERTY_NAME), rapidjson::Value(m_worldTourGroupSupported.value()), allocator);
	}

	if(m_conFileArgumentFlag.has_value()) {
		rapidjson::Value conFileArgumentFlagValue(m_conFileArgumentFlag.value().c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME), conFileArgumentFlagValue, allocator);
	}

	if(m_extraConFileArgumentFlag.has_value()) {
		rapidjson::Value extraConFileArgumentFlagValue(m_extraConFileArgumentFlag.value().c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_EXTRA_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME), extraConFileArgumentFlagValue, allocator);
	}

	if(m_groupFileArgumentFlag.has_value()) {
		rapidjson::Value groupFileArgumentFlagValue(m_groupFileArgumentFlag.value().c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME), groupFileArgumentFlagValue, allocator);
	}

	if(m_defFileArgumentFlag.has_value()) {
		rapidjson::Value defFileArgumentFlagValue(m_defFileArgumentFlag.value().c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME), defFileArgumentFlagValue, allocator);
	}

	if(m_extraDefFileArgumentFlag.has_value()) {
		rapidjson::Value extraDefFileArgumentFlagValue(m_extraDefFileArgumentFlag.value().c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_EXTRA_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME), extraDefFileArgumentFlagValue, allocator);
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

	if(!m_modDirectoryName.empty()) {
		rapidjson::Value modDirectoryNameValue(m_modDirectoryName.c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME), modDirectoryNameValue, allocator);
	}

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
		supportedOperatingSystemsValue.Reserve(m_supportedOperatingSystems.size(), allocator);

		for(std::vector<OperatingSystem>::const_iterator i = m_supportedOperatingSystems.begin(); i != m_supportedOperatingSystems.end(); ++i) {
			rapidjson::Value supportedOperatingSystemValue(std::string(magic_enum::enum_name(*i)).c_str(), allocator);
			supportedOperatingSystemsValue.PushBack(supportedOperatingSystemValue, allocator);
		}

		gameVersionValue.AddMember(rapidjson::StringRef(JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME), supportedOperatingSystemsValue, allocator);
	}

	if(!m_compatibleGameVersionIdentifiers.empty()) {
		rapidjson::Value compatibleGameVersionsValue(rapidjson::kArrayType);
		compatibleGameVersionsValue.Reserve(m_compatibleGameVersionIdentifiers.size(), allocator);

		for(std::vector<std::string>::const_iterator i = m_compatibleGameVersionIdentifiers.begin(); i != m_compatibleGameVersionIdentifiers.end(); ++i) {
			rapidjson::Value compatibleGameVersionValue((*i).c_str(), allocator);
			compatibleGameVersionsValue.PushBack(compatibleGameVersionValue, allocator);
		}

		gameVersionValue.AddMember(rapidjson::StringRef(JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME), compatibleGameVersionsValue, allocator);
	}

	if(!m_notes.empty()) {
		rapidjson::Value notesValue(rapidjson::kArrayType);
		notesValue.Reserve(m_notes.size(), allocator);

		for(const std::string & note : m_notes) {
			rapidjson::Value noteValue(note.c_str(), allocator);
			notesValue.PushBack(noteValue, allocator);
		}

		gameVersionValue.AddMember(rapidjson::StringRef(JSON_NOTES_PROPERTY_NAME), notesValue, allocator);
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

	// parse game version id
	if(!gameVersionValue.HasMember(JSON_ID_PROPERTY_NAME)) {
		spdlog::error("Game version is missing '{}' property.", JSON_ID_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & idValue = gameVersionValue[JSON_ID_PROPERTY_NAME];

	if(!idValue.IsString()) {
		spdlog::error("Game version has an invalid '{}' property type: '{}', expected 'string'.", JSON_ID_PROPERTY_NAME, Utilities::typeToString(idValue.GetType()));
		return nullptr;
	}

	std::string id(Utilities::trimString(idValue.GetString()));

	if(id.empty()) {
		spdlog::error("Game version '{}' property cannot be empty.", JSON_ID_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version long name
	if(!gameVersionValue.HasMember(JSON_LONG_NAME_PROPERTY_NAME)) {
		spdlog::error("Game version with ID '{}' is missing '{}' property.", id, JSON_LONG_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & longNameValue = gameVersionValue[JSON_LONG_NAME_PROPERTY_NAME];

	if(!longNameValue.IsString()) {
		spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_LONG_NAME_PROPERTY_NAME, Utilities::typeToString(longNameValue.GetType()));
		return nullptr;
	}

	std::string longName(Utilities::trimString(longNameValue.GetString()));

	if(longName.empty()) {
		spdlog::error("Game version with ID '{}' '{}' property cannot be empty.", id, JSON_LONG_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version short name
	if(!gameVersionValue.HasMember(JSON_SHORT_NAME_PROPERTY_NAME)) {
		spdlog::error("Game version with ID '{}' is missing '{}' property.", id, JSON_SHORT_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & shortNameValue = gameVersionValue[JSON_SHORT_NAME_PROPERTY_NAME];

	if(!shortNameValue.IsString()) {
		spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_SHORT_NAME_PROPERTY_NAME, Utilities::typeToString(shortNameValue.GetType()));
		return nullptr;
	}

	std::string shortName(Utilities::trimString(shortNameValue.GetString()));

	if(shortName.empty()) {
		spdlog::error("Game version with ID '{}' '{}' property cannot be empty.", id, JSON_SHORT_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse removable property
	bool removable = true;

	if(gameVersionValue.HasMember(JSON_REMOVABLE_PROPERTY_NAME)) {
		const rapidjson::Value & removableValue = gameVersionValue[JSON_REMOVABLE_PROPERTY_NAME];

		if(!removableValue.IsBool()) {
			spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'boolean'.", id, JSON_REMOVABLE_PROPERTY_NAME, Utilities::typeToString(removableValue.GetType()));
			return nullptr;
		}

		removable = removableValue.GetBool();
	}
	else {
		spdlog::warn("Game version with ID '{}' is missing '{}' property.", id, JSON_REMOVABLE_PROPERTY_NAME);
	}

	// parse game version game path
	if(!gameVersionValue.HasMember(JSON_GAME_PATH_PROPERTY_NAME)) {
		spdlog::error("Game version with ID '{}' is missing '{}' property.", id, JSON_GAME_PATH_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & gamePathValue = gameVersionValue[JSON_GAME_PATH_PROPERTY_NAME];

	if(!gamePathValue.IsString()) {
		spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_GAME_PATH_PROPERTY_NAME, Utilities::typeToString(gamePathValue.GetType()));
		return nullptr;
	}

	std::string gamePath(Utilities::trimString(gamePathValue.GetString()));

	// parse game version game executable name
	if(!gameVersionValue.HasMember(JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME)) {
		spdlog::error("Game version with ID '{}' is missing '{}' property.", id, JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & gameExecutableNameValue = gameVersionValue[JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME];

	if(!gameExecutableNameValue.IsString()) {
		spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME, Utilities::typeToString(gameExecutableNameValue.GetType()));
		return nullptr;
	}

	std::string gameExecutableName(Utilities::trimString(gameExecutableNameValue.GetString()));

	if(gameExecutableName.empty()) {
		spdlog::error("Game version with ID '{}' '{}' property cannot be empty.", id, JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version setup executable name
	std::optional<std::string> setupExecutableNameOptional;

	if(gameVersionValue.HasMember(JSON_SETUP_EXECUTABLE_NAME_PROPERTY_NAME)) {
		const rapidjson::Value & setupExecutableNameValue = gameVersionValue[JSON_SETUP_EXECUTABLE_NAME_PROPERTY_NAME];

		if(!setupExecutableNameValue.IsString()) {
			spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_SETUP_EXECUTABLE_NAME_PROPERTY_NAME, Utilities::typeToString(setupExecutableNameValue.GetType()));
			return nullptr;
		}

		setupExecutableNameOptional = Utilities::trimString(setupExecutableNameValue.GetString());

		if(setupExecutableNameOptional.value().empty()) {
			spdlog::error("Game version with ID '{}' '{}' property cannot be empty.", id, JSON_SETUP_EXECUTABLE_NAME_PROPERTY_NAME);
			return nullptr;
		}
	}

	// parse group file install path
	std::optional<std::string> groupFileInstallPathOptional;

	if(gameVersionValue.HasMember(JSON_GROUP_FILE_INSTALL_PATH_PROPERTY_NAME)) {
		const rapidjson::Value & groupFileInstallPathValue = gameVersionValue[JSON_GROUP_FILE_INSTALL_PATH_PROPERTY_NAME];

		if(!groupFileInstallPathValue.IsString()) {
			spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_GROUP_FILE_INSTALL_PATH_PROPERTY_NAME, Utilities::typeToString(groupFileInstallPathValue.GetType()));
			return nullptr;
		}

		groupFileInstallPathOptional = Utilities::trimString(groupFileInstallPathValue.GetString());
	}

	// parse local working directory option
	if(!gameVersionValue.HasMember(JSON_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME)) {
		spdlog::error("Game version with ID '{}' is missing '{}' property.", id, JSON_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & localWorkingDirectoryValue = gameVersionValue[JSON_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME];

	if(!localWorkingDirectoryValue.IsBool()) {
		spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'boolean'.", id, JSON_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME, Utilities::typeToString(localWorkingDirectoryValue.GetType()));
		return nullptr;
	}

	bool localWorkingDirectory = localWorkingDirectoryValue.GetBool();

	// parse relative con file path option
	if(!gameVersionValue.HasMember(JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME)) {
		spdlog::error("Game version with ID '{}' is missing '{}' property.", id, JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & relativeConFilePathValue = gameVersionValue[JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME];

	if(!relativeConFilePathValue.IsBool()) {
		spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'boolean'.", id, JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME, Utilities::typeToString(relativeConFilePathValue.GetType()));
		return nullptr;
	}

	bool relativeConFilePath = relativeConFilePathValue.GetBool();

	// parse supports subdirectories option
	if(!gameVersionValue.HasMember(JSON_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME)) {
		spdlog::error("Game version with ID '{}' is missing '{}' property.", id, JSON_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & supportsSubdirectoriesValue = gameVersionValue[JSON_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME];

	if(!supportsSubdirectoriesValue.IsBool()) {
		spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'boolean'.", id, JSON_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME, Utilities::typeToString(supportsSubdirectoriesValue.GetType()));
		return nullptr;
	}

	bool supportsSubdirectories = supportsSubdirectoriesValue.GetBool();

	// parse supports world tour group option
	std::optional<bool> worldTourGroupSupported;

	if(gameVersionValue.HasMember(JSON_SUPPORTS_WORLD_TOUR_GROUP_PROPERTY_NAME)) {
		const rapidjson::Value & worldTourGroupSupportedValue = gameVersionValue[JSON_SUPPORTS_WORLD_TOUR_GROUP_PROPERTY_NAME];

		if(!worldTourGroupSupportedValue.IsBool()) {
			spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'boolean'.", id, JSON_SUPPORTS_WORLD_TOUR_GROUP_PROPERTY_NAME, Utilities::typeToString(worldTourGroupSupportedValue.GetType()));
			return nullptr;
		}

		worldTourGroupSupported = worldTourGroupSupportedValue.GetBool();
	}

	// parse game version con file argument flag
	std::optional<std::string> optionalConFileArgumentFlag;

	if(gameVersionValue.HasMember(JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & conFileArgumentFlagValue = gameVersionValue[JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!conFileArgumentFlagValue.IsString()) {
			spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(conFileArgumentFlagValue.GetType()));
			return nullptr;
		}

		optionalConFileArgumentFlag = conFileArgumentFlagValue.GetString();

		if(optionalConFileArgumentFlag.value().empty()) {
			spdlog::error("Game version with ID '{}' '{}' property cannot be empty.", id, JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME);
			return nullptr;
		}
	}

	// parse game version extra con file argument flag
	std::optional<std::string> optionalExtraConFileArgumentFlag;

	if(gameVersionValue.HasMember(JSON_EXTRA_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & extraConFileArgumentFlagValue = gameVersionValue[JSON_EXTRA_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!extraConFileArgumentFlagValue.IsString()) {
			spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_EXTRA_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(extraConFileArgumentFlagValue.GetType()));
			return nullptr;
		}

		optionalExtraConFileArgumentFlag = extraConFileArgumentFlagValue.GetString();

		if(optionalExtraConFileArgumentFlag.value().empty()) {
			spdlog::error("Game version with ID '{}' '{}' property cannot be empty.", id, JSON_EXTRA_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME);
			return nullptr;
		}
	}

	// parse game version group file argument flag
	std::optional<std::string> optionalGroupFileArgumentFlag;

	if(gameVersionValue.HasMember(JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & groupFileArgumentFlagValue = gameVersionValue[JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!groupFileArgumentFlagValue.IsString()) {
			spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(groupFileArgumentFlagValue.GetType()));
			return nullptr;
		}

		optionalGroupFileArgumentFlag = groupFileArgumentFlagValue.GetString();

		if(optionalGroupFileArgumentFlag.value().empty()) {
			spdlog::error("Game version with ID '{}' '{}' property cannot be empty.", id, JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME);
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
		spdlog::error("Game version with ID '{}' is missing '{}' property.", id, JSON_EPISODE_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & episodeArgumentFlagValue = gameVersionValue[JSON_EPISODE_ARGUMENT_FLAG_PROPERTY_NAME];

	if(!episodeArgumentFlagValue.IsString()) {
		spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_EPISODE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(episodeArgumentFlagValue.GetType()));
		return nullptr;
	}

	std::string episodeArgumentFlag(episodeArgumentFlagValue.GetString());

	if(episodeArgumentFlag.empty()) {
		spdlog::error("Game version with ID '{}' '{}' property cannot be empty.", id, JSON_EPISODE_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version level argument flag
	if(!gameVersionValue.HasMember(JSON_LEVEL_ARGUMENT_FLAG_PROPERTY_NAME)) {
		spdlog::error("Game version with ID '{}' is missing '{}' property.", id, JSON_LEVEL_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & levelArgumentFlagValue = gameVersionValue[JSON_LEVEL_ARGUMENT_FLAG_PROPERTY_NAME];

	if(!levelArgumentFlagValue.IsString()) {
		spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_LEVEL_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(levelArgumentFlagValue.GetType()));
		return nullptr;
	}

	std::string levelArgumentFlag(levelArgumentFlagValue.GetString());

	if(levelArgumentFlag.empty()) {
		spdlog::error("Game version with ID '{}' '{}' property cannot be empty.", id, JSON_LEVEL_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version skill argument flag
	if(!gameVersionValue.HasMember(JSON_SKILL_ARGUMENT_FLAG_PROPERTY_NAME)) {
		spdlog::error("Game version with ID '{}' is missing '{}' property.", id, JSON_SKILL_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & skillArgumentFlagValue = gameVersionValue[JSON_SKILL_ARGUMENT_FLAG_PROPERTY_NAME];

	if(!skillArgumentFlagValue.IsString()) {
		spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_SKILL_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(skillArgumentFlagValue.GetType()));
		return nullptr;
	}

	std::string skillArgumentFlag(skillArgumentFlagValue.GetString());

	if(skillArgumentFlag.empty()) {
		spdlog::error("Game version with ID '{}' '{}' property cannot be empty.", id, JSON_SKILL_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	// parse skill start value option
	uint8_t skillStartValue = DEFAULT_SKILL_START_VALUE;

	if(gameVersionValue.HasMember(JSON_SKILL_START_VALUE_PROPERTY_NAME)) {
		const rapidjson::Value & skillStartValueValue = gameVersionValue[JSON_SKILL_START_VALUE_PROPERTY_NAME];

		if(!skillStartValueValue.IsUint()) {
			spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'boolean'.", id, JSON_SKILL_START_VALUE_PROPERTY_NAME, Utilities::typeToString(skillStartValueValue.GetType()));
			return nullptr;
		}

		unsigned int tempSkillStartValue = skillStartValueValue.GetUint();

		if(tempSkillStartValue > std::numeric_limits<uint8_t>::max()) {
			spdlog::error("Game version with ID '{}' '{}' property value of {} exceeds the maximum value of {}.", id, JSON_SKILL_START_VALUE_PROPERTY_NAME, tempSkillStartValue, std::numeric_limits<uint8_t>::max());
			return nullptr;
		}

		skillStartValue = static_cast<uint8_t>(tempSkillStartValue);
	}
	else {
		spdlog::warn("Game version with ID '{}' is missing '{}' property.", id, JSON_SKILL_START_VALUE_PROPERTY_NAME);
	}

	// parse game version record demo argument flag
	if(!gameVersionValue.HasMember(JSON_RECORD_DEMO_ARGUMENT_FLAG_PROPERTY_NAME)) {
		spdlog::error("Game version with ID '{}' is missing '{}' property.", id, JSON_RECORD_DEMO_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & recordDemoArgumentFlagValue = gameVersionValue[JSON_RECORD_DEMO_ARGUMENT_FLAG_PROPERTY_NAME];

	if(!recordDemoArgumentFlagValue.IsString()) {
		spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_RECORD_DEMO_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(recordDemoArgumentFlagValue.GetType()));
		return nullptr;
	}

	std::string recordDemoArgumentFlag(recordDemoArgumentFlagValue.GetString());

	if(recordDemoArgumentFlag.empty()) {
		spdlog::error("Game version with ID '{}' '{}' property cannot be empty.", id, JSON_RECORD_DEMO_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version play demo argument flag
	std::optional<std::string> optionalPlayDemoArgumentFlag;

	if(gameVersionValue.HasMember(JSON_PLAY_DEMO_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & playDemoArgumentFlagValue = gameVersionValue[JSON_PLAY_DEMO_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!playDemoArgumentFlagValue.IsString()) {
			spdlog::error("Game version with ID '{}' '{}' property has invalid type: '{}', expected 'string'.", id, JSON_PLAY_DEMO_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(playDemoArgumentFlagValue.GetType()));
			return nullptr;
		}

		optionalPlayDemoArgumentFlag = playDemoArgumentFlagValue.GetString();
	}

	// parse game version respawn mode argument flag
	std::optional<std::string> optionalRespawnModeArgumentFlag;

	if(gameVersionValue.HasMember(JSON_RESPAWN_MODE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & respawnModeArgumentFlagValue = gameVersionValue[JSON_RESPAWN_MODE_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!respawnModeArgumentFlagValue.IsString()) {
			spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_RESPAWN_MODE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(respawnModeArgumentFlagValue.GetType()));
			return nullptr;
		}

		optionalRespawnModeArgumentFlag = respawnModeArgumentFlagValue.GetString();

		if(optionalRespawnModeArgumentFlag.value().empty()) {
			spdlog::error("Game version with ID '{}' '{}' property cannot be empty.", id, JSON_RESPAWN_MODE_ARGUMENT_FLAG_PROPERTY_NAME);
			return nullptr;
		}
	}

	// parse game version weapon switch order argument flag
	std::optional<std::string> optionalWeaponSwitchOrderArgumentFlag;

	if(gameVersionValue.HasMember(JSON_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & weaponSwitchOrderArgumentFlagValue = gameVersionValue[JSON_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!weaponSwitchOrderArgumentFlagValue.IsString()) {
			spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(weaponSwitchOrderArgumentFlagValue.GetType()));
			return nullptr;
		}

		optionalWeaponSwitchOrderArgumentFlag = weaponSwitchOrderArgumentFlagValue.GetString();

		if(optionalWeaponSwitchOrderArgumentFlag.value().empty()) {
			spdlog::error("Game version with ID '{}' '{}' property cannot be empty.", id, JSON_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG_PROPERTY_NAME);
			return nullptr;
		}
	}

	// parse game version disable monsters argument flag
	std::optional<std::string> optionalDisableMonstersArgumentFlag;

	if(gameVersionValue.HasMember(JSON_DISABLE_MONSTERS_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & disableMonstersArgumentFlagValue = gameVersionValue[JSON_DISABLE_MONSTERS_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!disableMonstersArgumentFlagValue.IsString()) {
			spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_DISABLE_MONSTERS_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(disableMonstersArgumentFlagValue.GetType()));
			return nullptr;
		}

		optionalDisableMonstersArgumentFlag = disableMonstersArgumentFlagValue.GetString();

		if(optionalDisableMonstersArgumentFlag.value().empty()) {
			spdlog::error("Game version with ID '{}' '{}' property cannot be empty.", id, JSON_DISABLE_MONSTERS_ARGUMENT_FLAG_PROPERTY_NAME);
			return nullptr;
		}
	}

	// parse game version disable sound argument flag
	std::optional<std::string> optionalDisableSoundArgumentFlag;

	if(gameVersionValue.HasMember(JSON_DISABLE_SOUND_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & disableSoundArgumentFlagValue = gameVersionValue[JSON_DISABLE_SOUND_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!disableSoundArgumentFlagValue.IsString()) {
			spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_DISABLE_SOUND_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(disableSoundArgumentFlagValue.GetType()));
			return nullptr;
		}

		optionalDisableSoundArgumentFlag = disableSoundArgumentFlagValue.GetString();

		if(optionalDisableSoundArgumentFlag.value().empty()) {
			spdlog::error("Game version with ID '{}' '{}' property cannot be empty.", id, JSON_DISABLE_SOUND_ARGUMENT_FLAG_PROPERTY_NAME);
			return nullptr;
		}
	}

	// parse game version disable music argument flag
	std::optional<std::string> optionalDisableMusicArgumentFlag;

	if(gameVersionValue.HasMember(JSON_DISABLE_MUSIC_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & disableMusicArgumentFlagValue = gameVersionValue[JSON_DISABLE_MUSIC_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!disableMusicArgumentFlagValue.IsString()) {
			spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_DISABLE_MUSIC_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(disableMusicArgumentFlagValue.GetType()));
			return nullptr;
		}

		optionalDisableMusicArgumentFlag = disableMusicArgumentFlagValue.GetString();

		if(optionalDisableMusicArgumentFlag.value().empty()) {
			spdlog::error("Game version with ID '{}' '{}' property cannot be empty.", id, JSON_DISABLE_MUSIC_ARGUMENT_FLAG_PROPERTY_NAME);
			return nullptr;
		}
	}

	// parse game version mod directory name
	std::string modDirectoryName;

	if(gameVersionValue.HasMember(JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME)) {
		const rapidjson::Value & modDirectoryNameValue = gameVersionValue[JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME];

		if(!modDirectoryNameValue.IsString()) {
			spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME, Utilities::typeToString(modDirectoryNameValue.GetType()));
			return nullptr;
		}

		modDirectoryName = Utilities::trimString(modDirectoryNameValue.GetString());

		if(modDirectoryName.empty()) {
			spdlog::error("Game version with ID '{}' '{}' property cannot be empty.", id, JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME);
			return nullptr;
		}
	}

	// initialize the game version
	std::unique_ptr<GameVersion> newGameVersion(std::make_unique<GameVersion>(id, longName, shortName, removable, gamePath, gameExecutableName, localWorkingDirectory, relativeConFilePath, supportsSubdirectories, worldTourGroupSupported, modDirectoryName, optionalConFileArgumentFlag, optionalExtraConFileArgumentFlag,optionalGroupFileArgumentFlag, optionalMapFileArgumentFlag, episodeArgumentFlag, levelArgumentFlag, skillArgumentFlag, skillStartValue, recordDemoArgumentFlag, optionalPlayDemoArgumentFlag, optionalRespawnModeArgumentFlag, optionalWeaponSwitchOrderArgumentFlag, optionalDisableMonstersArgumentFlag, optionalDisableSoundArgumentFlag, optionalDisableMusicArgumentFlag, setupExecutableNameOptional, groupFileInstallPathOptional));

	// parse stand-alone mod installed timestamp
	std::optional<std::chrono::time_point<std::chrono::system_clock>> optionalInstalledTimePoint;

	if(gameVersionValue.HasMember(JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME)) {
		const rapidjson::Value & installedTimestampValue = gameVersionValue[JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME];

		if(!installedTimestampValue.IsString()) {
			spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME, Utilities::typeToString(installedTimestampValue.GetType()));
			return nullptr;
		}

		optionalInstalledTimePoint = Utilities::parseTimePointFromString(installedTimestampValue.GetString());

		if(!optionalInstalledTimePoint.has_value()) {
			spdlog::error("Game version with ID '{}' has an invalid '{}' timestamp value: '{}'.", id, JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME, installedTimestampValue.GetString());
			return nullptr;
		}

		newGameVersion->setInstalledTimePoint(optionalInstalledTimePoint.value());
	}

	// parse stand-alone mod last played timestamp
	std::optional<std::chrono::time_point<std::chrono::system_clock>> optionalLastPlayedTimePoint;

	if(gameVersionValue.HasMember(JSON_LAST_PLAYED_TIMESTAMP_PROPERTY_NAME)) {
		const rapidjson::Value & lastPlayedTimestampValue = gameVersionValue[JSON_LAST_PLAYED_TIMESTAMP_PROPERTY_NAME];

		if(!lastPlayedTimestampValue.IsString()) {
			spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_LAST_PLAYED_TIMESTAMP_PROPERTY_NAME, Utilities::typeToString(lastPlayedTimestampValue.GetType()));
			return nullptr;
		}

		optionalLastPlayedTimePoint = Utilities::parseTimePointFromString(lastPlayedTimestampValue.GetString());

		if(!optionalLastPlayedTimePoint.has_value()) {
			spdlog::error("Game version with ID '{}' has an invalid '{}' timestamp value: '{}'.", id, JSON_LAST_PLAYED_TIMESTAMP_PROPERTY_NAME, lastPlayedTimestampValue.GetString());
			return nullptr;
		}

		newGameVersion->setLastPlayedTimePoint(optionalLastPlayedTimePoint.value());
	}

	// parse game version basse
	if(gameVersionValue.HasMember(JSON_BASE_PROPERTY_NAME)) {
		const rapidjson::Value & baseValue = gameVersionValue[JSON_BASE_PROPERTY_NAME];

		if(!baseValue.IsString()) {
			spdlog::error("Game version with ID '{}' has an invalid '{}' property type: '{}', expected 'string'.", id, JSON_BASE_PROPERTY_NAME, Utilities::typeToString(baseValue.GetType()));
			return nullptr;
		}

		newGameVersion->setBase(Utilities::trimString(baseValue.GetString()));
	}

	// parse game version def file argument flag
	if(gameVersionValue.HasMember(JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & defFileArgumentFlagValue = gameVersionValue[JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!defFileArgumentFlagValue.IsString()) {
			spdlog::error("Game version with ID '{}' '{}' property has invalid type: '{}', expected 'string'.", id, JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(defFileArgumentFlagValue.GetType()));
			return nullptr;
		}

		newGameVersion->m_defFileArgumentFlag = defFileArgumentFlagValue.GetString();
	}

	// parse game version extra def file argument flag
	if(gameVersionValue.HasMember(JSON_EXTRA_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & extraDefFileArgumentFlagValue = gameVersionValue[JSON_EXTRA_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!extraDefFileArgumentFlagValue.IsString()) {
			spdlog::error("Game version with ID '{}' '{}' property has invalid type: '{}', expected 'string'.", id, JSON_EXTRA_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(extraDefFileArgumentFlagValue.GetType()));
			return nullptr;
		}

		newGameVersion->m_extraDefFileArgumentFlag = extraDefFileArgumentFlagValue.GetString();
	}

	// parse the game version requires full combined group property
	if(gameVersionValue.HasMember(JSON_REQUIRES_COMBINED_GROUP_PROPERTY_NAME)) {
		const rapidjson::Value & requiresCombinedGroupValue = gameVersionValue[JSON_REQUIRES_COMBINED_GROUP_PROPERTY_NAME];

		if(!requiresCombinedGroupValue.IsBool()) {
			spdlog::error("Game version with ID '{}' '{}' property has invalid type: '{}', expected 'boolean'.", id, JSON_REQUIRES_COMBINED_GROUP_PROPERTY_NAME, Utilities::typeToString(requiresCombinedGroupValue.GetType()));
			return nullptr;
		}

		newGameVersion->m_requiresCombinedGroup = requiresCombinedGroupValue.GetBool();
	}

	// parse the game version requires group file extraction property
	if(gameVersionValue.HasMember(JSON_REQUIRES_GROUP_FILE_EXTRACTION_PROPERTY_NAME)) {
		const rapidjson::Value & requiresGroupFileExtractionValue = gameVersionValue[JSON_REQUIRES_GROUP_FILE_EXTRACTION_PROPERTY_NAME];

		if(!requiresGroupFileExtractionValue.IsBool()) {
			spdlog::error("Game version with ID '{}' '{}' property has invalid type: '{}', expected 'boolean'.", id, JSON_REQUIRES_GROUP_FILE_EXTRACTION_PROPERTY_NAME, Utilities::typeToString(requiresGroupFileExtractionValue.GetType()));
			return nullptr;
		}

		newGameVersion->m_requiresGroupFileExtraction = requiresGroupFileExtractionValue.GetBool();
	}

	// parse the game version website property
	if(gameVersionValue.HasMember(JSON_WEBSITE_PROPERTY_NAME)) {
		const rapidjson::Value & websiteValue = gameVersionValue[JSON_WEBSITE_PROPERTY_NAME];

		if(!websiteValue.IsString()) {
			spdlog::error("Game version with ID '{}' '{}' property has invalid type: '{}', expected 'string'.", id, JSON_WEBSITE_PROPERTY_NAME, Utilities::typeToString(websiteValue.GetType()));
			return nullptr;
		}

		newGameVersion->m_website = websiteValue.GetString();
	}

	// parse the game version source code url property
	if(gameVersionValue.HasMember(JSON_SOURCE_CODE_URL_PROPERTY_NAME)) {
		const rapidjson::Value & sourceCodeURLValue = gameVersionValue[JSON_SOURCE_CODE_URL_PROPERTY_NAME];

		if(!sourceCodeURLValue.IsString()) {
			spdlog::error("Game version with ID '{}' '{}' property has invalid type: '{}', expected 'string'.", id, JSON_SOURCE_CODE_URL_PROPERTY_NAME, Utilities::typeToString(sourceCodeURLValue.GetType()));
			return nullptr;
		}

		newGameVersion->m_sourceCodeURL = sourceCodeURLValue.GetString();
	}

	// parse the supported operating systems property
	if(gameVersionValue.HasMember(JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME)) {
		const rapidjson::Value & supportedOperatingSystemsValue = gameVersionValue[JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME];

		if(!supportedOperatingSystemsValue.IsArray()) {
			spdlog::error("Game version with ID '{}' '{}' property has invalid type: '{}', expected 'array'.", id, JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME, Utilities::typeToString(supportedOperatingSystemsValue.GetType()));
			return nullptr;
		}

		for(rapidjson::Value::ConstValueIterator i = supportedOperatingSystemsValue.Begin(); i != supportedOperatingSystemsValue.End(); ++i) {
			if(!i->IsString()) {
				spdlog::error("Game version with ID '{}' '{}' property contains invalid supported operating system type: '{}', expected 'string'.", id, JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME, Utilities::typeToString(i->GetType()));
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

				spdlog::error("Game version with ID '{}' '{}' property contains invalid supported operating system value: '{}', expected one of: {}.", id, JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME, supportedOperatingSystemName, validOperatingSystems.str());
				return nullptr;
			}

			if(newGameVersion->hasSupportedOperatingSystem(optionalSupportedOperatingSystem.value())) {
				spdlog::warn("Game version with ID '{}' '{}' property contains duplicate supported operating system: '{}'.", id, JSON_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME, supportedOperatingSystemName);
				continue;
			}

			newGameVersion->m_supportedOperatingSystems.push_back(optionalSupportedOperatingSystem.value());
		}
	}

	// parse the compatible game versions property
	if(gameVersionValue.HasMember(JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME)) {
		const rapidjson::Value & compatibleGameVersionsValue = gameVersionValue[JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME];

		if(!compatibleGameVersionsValue.IsArray()) {
			spdlog::error("Game version with ID '{}' '{}' property has invalid type: '{}', expected 'array'.", id, JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME, Utilities::typeToString(compatibleGameVersionsValue.GetType()));
			return nullptr;
		}

		for(rapidjson::Value::ConstValueIterator i = compatibleGameVersionsValue.Begin(); i != compatibleGameVersionsValue.End(); ++i) {
			if(!i->IsString()) {
				spdlog::error("Game version with ID '{}' '{}' property contains invalid compatible game version type: '{}', expected 'string'.", id, JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME, Utilities::typeToString(i->GetType()));
				return nullptr;
			}

			std::string compatibleGameVersionID(Utilities::trimString(i->GetString()));

			if(newGameVersion->hasCompatibleGameVersionWithID(compatibleGameVersionID)) {
				spdlog::warn("Game version with ID '{}' '{}' property contains duplicate compatible game version with ID: '{}'.", id, JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME, compatibleGameVersionID);
				continue;
			}

			newGameVersion->m_compatibleGameVersionIdentifiers.push_back(compatibleGameVersionID);
		}
	}

	// parse the notes property
	if(gameVersionValue.HasMember(JSON_NOTES_PROPERTY_NAME)) {
		const rapidjson::Value & notesValue = gameVersionValue[JSON_NOTES_PROPERTY_NAME];

		if(!notesValue.IsArray()) {
			spdlog::error("Game version with ID '{}' '{}' property has invalid type: '{}', expected 'array'.", id, JSON_NOTES_PROPERTY_NAME, Utilities::typeToString(notesValue.GetType()));
			return nullptr;
		}

		for(rapidjson::Value::ConstValueIterator i = notesValue.Begin(); i != notesValue.End(); ++i) {
			std::string note(Utilities::trimString((*i).GetString()));

			if(note.empty()) {
				spdlog::error("Encountered empty note #{} for game version with ID '{}'.", newGameVersion->m_notes.size() + 1, id);
				return nullptr;
			}

			if(newGameVersion->hasNote(note)) {
				spdlog::error("Encountered duplicate note #{} for game version with ID '{}'.", newGameVersion->m_notes.size() + 1, id);
				return nullptr;
			}

			newGameVersion->m_notes.emplace_back(note);
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
	if(m_id.empty() ||
	   m_longName.empty() ||
	   m_shortName.empty() ||
	   (m_standAlone && m_base.empty()) ||
	   m_gameExecutableName.empty() ||
	   (!m_standAlone && m_modDirectoryName.empty()) ||
	   (m_conFileArgumentFlag.has_value() && m_conFileArgumentFlag.value().empty()) ||
	   (m_extraConFileArgumentFlag.has_value() && m_extraConFileArgumentFlag.value().empty()) ||
	   (m_groupFileArgumentFlag.has_value() && m_groupFileArgumentFlag.value().empty()) ||
	   (m_defFileArgumentFlag.has_value() && m_defFileArgumentFlag.value().empty()) ||
	   (m_extraDefFileArgumentFlag.has_value() && m_extraDefFileArgumentFlag.value().empty()) ||
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

	return true;
}

bool GameVersion::isValid(const GameVersion * gameVersion) {
	return gameVersion != nullptr && gameVersion->isValid();
}

bool GameVersion::operator == (const GameVersion & gameVersion) const {
	if(m_standAlone != gameVersion.m_standAlone ||
	   m_removable != gameVersion.m_removable ||
	   m_requiresCombinedGroup != gameVersion.m_requiresCombinedGroup ||
	   m_requiresGroupFileExtraction != gameVersion.m_requiresGroupFileExtraction ||
	   m_localWorkingDirectory != gameVersion.m_localWorkingDirectory ||
	   m_relativeConFilePath != gameVersion.m_relativeConFilePath ||
	   m_supportsSubdirectories != gameVersion.m_supportsSubdirectories ||
	   m_worldTourGroupSupported != gameVersion.m_worldTourGroupSupported ||
	   m_installedTimePoint != gameVersion.m_installedTimePoint ||
	   m_lastPlayedTimePoint != gameVersion.m_lastPlayedTimePoint ||
	   !Utilities::areStringsEqual(m_id, gameVersion.m_id) ||
	   !Utilities::areStringsEqual(m_longName, gameVersion.m_longName) ||
	   !Utilities::areStringsEqual(m_shortName, gameVersion.m_shortName) ||
	   !Utilities::areStringsEqual(m_base, gameVersion.m_base) ||
	   m_gamePath != gameVersion.m_gamePath ||
	   m_gameExecutableName != gameVersion.m_gameExecutableName ||
	   m_setupExecutableName != gameVersion.m_setupExecutableName ||
	   m_groupFileInstallPath != gameVersion.m_groupFileInstallPath ||
	   m_modDirectoryName != gameVersion.m_modDirectoryName ||
	   m_website != gameVersion.m_website ||
	   m_sourceCodeURL != gameVersion.m_sourceCodeURL ||
	   m_conFileArgumentFlag != gameVersion.m_conFileArgumentFlag ||
	   m_extraConFileArgumentFlag != gameVersion.m_extraConFileArgumentFlag ||
	   m_groupFileArgumentFlag != gameVersion.m_groupFileArgumentFlag ||
	   m_defFileArgumentFlag != gameVersion.m_defFileArgumentFlag ||
	   m_extraDefFileArgumentFlag != gameVersion.m_extraDefFileArgumentFlag ||
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
	   m_compatibleGameVersionIdentifiers.size() != gameVersion.m_compatibleGameVersionIdentifiers.size() ||
	   m_supportedOperatingSystems != gameVersion.m_supportedOperatingSystems ||
	   m_notes != gameVersion.m_notes) {
		return false;
	}

	for(size_t i = 0; i < m_compatibleGameVersionIdentifiers.size(); i++) {
		if(!Utilities::areStringsEqualIgnoreCase(m_compatibleGameVersionIdentifiers[i], gameVersion.m_compatibleGameVersionIdentifiers[i])) {
			return false;
		}
	}

	return true;
}

bool GameVersion::operator != (const GameVersion & gameVersion) const {
	return !operator == (gameVersion);
}
