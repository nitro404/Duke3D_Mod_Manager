#include "GameVersion.h"

#include "Mod/ModGameVersion.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <fmt/core.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

static constexpr const char * JSON_NAME_PROPERTY_NAME = "name";
static constexpr const char * JSON_GAME_PATH_PROPERTY_NAME = "gamePath";
static constexpr const char * JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME = "gameExecutableName";
static constexpr const char * JSON_SETUP_EXECUTABLE_NAME_PROPERTY_NAME = "setupExecutableName";
static constexpr const char * JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME = "relativeConFilePath";
static constexpr const char * JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME = "conFileArgumentFlag";
static constexpr const char * JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME = "groupFileArgumentFlag";
static constexpr const char * JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME = "defFileArgumentFlag";
static constexpr const char * JSON_MAP_FILE_ARGUMENT_FLAG_PROPERTY_NAME = "mapFileArgumentFlag";
static constexpr const char * JSON_REQUIRES_DOSBOX_PROPERTY_NAME = "requiresDOSBox";
static constexpr const char * JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME = "modDirectoryName";
static constexpr const char * JSON_WEBSITE_PROPERTY_NAME = "website";
static constexpr const char * JSON_SOURCE_CODE_URL_PROPERTY_NAME = "sourceCodeURL";
static constexpr const char * JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME = "compatibleGameVersions";
static const std::array<std::string_view, 14> JSON_PROPERTY_NAMES = {
	JSON_NAME_PROPERTY_NAME,
	JSON_GAME_PATH_PROPERTY_NAME,
	JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME,
	JSON_SETUP_EXECUTABLE_NAME_PROPERTY_NAME,
	JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME,
	JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_MAP_FILE_ARGUMENT_FLAG_PROPERTY_NAME,
	JSON_REQUIRES_DOSBOX_PROPERTY_NAME,
	JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME,
	JSON_WEBSITE_PROPERTY_NAME,
	JSON_SOURCE_CODE_URL_PROPERTY_NAME,
	JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME
};

const std::string GameVersion::ALL_VERSIONS = "All Versions";

const GameVersion GameVersion::REGULAR_VERSION         ("Regular Version 1.3",         "", "DUKE3D.EXE",                 false, "/x",  "/g",  "-map ", "Regular",          "SETUP.EXE", {},    true, "https://www.dukenukem.com");
const GameVersion GameVersion::ATOMIC_EDITION          ("Atomic Edition 1.4/1.5",      "", "DUKE3D.EXE",                 false, "/x",  "/g",  "-map ", "Atomic",           "SETUP.EXE", {},    true, "https://www.dukenukem.com");
const GameVersion GameVersion::JFDUKE3D                ("JFDuke3D",                    "", "duke3d.exe",                 false, "/x",  "/g",  "-map ", "JFDuke3D",         {},          {},    {},   "http://www.jonof.id.au/jfduke3d",                               "https://github.com/jonof/jfduke3d",                  { GameVersion::REGULAR_VERSION.getName(), GameVersion::ATOMIC_EDITION.getName() });
const GameVersion GameVersion::EDUKE32                 ("eDuke32",                     "", "eduke32.exe",                true,  "-x ", "-g ", "-map ", "eDuke32",          {},          "-h ", {},   "https://www.eduke32.com",                                       "https://voidpoint.io/terminx/eduke32",               { GameVersion::REGULAR_VERSION.getName(), GameVersion::ATOMIC_EDITION.getName(), GameVersion::JFDUKE3D.getName() });
//const GameVersion GameVersion::MEGATON_EDITION         ("Megaton Edition",             "", "duke3d.exe",                 false, "/x",  "/g",  "-map ", "Megaton",          {},          {},    {},   "https://store.steampowered.com/app/225140",                     "https://github.com/TermiT/duke3d-megaton",           { GameVersion::REGULAR_VERSION.getName(), GameVersion::ATOMIC_EDITION.getName(), GameVersion::JFDUKE3D.getName() });
const GameVersion GameVersion::WORLD_TOUR              ("20th Anniversary World Tour", "", "duke3d.exe",                 false, "/x",  "/g",  "-map ", "WorldTour",        {},          {},    {},   "https://www.gearboxsoftware.com/game/duke-3d-20th",             "",                                                   { GameVersion::REGULAR_VERSION.getName(), GameVersion::ATOMIC_EDITION.getName() });
//const GameVersion BUILDGDX                             ("BuildGDX",                    "", "BuildGDX.jar",               true,  "",    "",    "-map ", "BuildGDX",         {},          {},    {},   "https://m210.duke4.net/index.php/downloads/category/8-java",    "https://gitlab.com/m210/BuildGDX",                   { GameVersion::REGULAR_VERSION.getName(), GameVersion::ATOMIC_EDITION.getName() });
const GameVersion GameVersion::RAZE                    ("Raze",                        "", "raze.exe",                   true,  "-x ", "-g ", "-map ", "Raze",             {},          "-h ", {},   "https://raze.zdoom.org/about",                                  "https://github.com/coelckers/Raze",                  { GameVersion::ATOMIC_EDITION.getName(),  GameVersion::JFDUKE3D.getName(), GameVersion::EDUKE32.getName() });
const GameVersion GameVersion::REDNUKEM                ("RedNukem",                    "", "rednukem.exe",               true,  "-x ", "-g ", "-map ", "RedNukem",         {},          "-h ", {},   "https://lerppu.net/wannabethesis",                              "https://github.com/nukeykt/NRedneck",                { GameVersion::REGULAR_VERSION.getName(), GameVersion::ATOMIC_EDITION.getName(), GameVersion::JFDUKE3D.getName(), GameVersion::EDUKE32.getName() });
const GameVersion GameVersion::CHOCOLATE_DUKE3D        ("Chocolate Duke3D",            "", "Game.exe",                   false, "/x",  "/g",  "-map ", "Chocolate",        {},          {},    {},   "https://fabiensanglard.net/duke3d/chocolate_duke_nukem_3D.php", "https://github.com/fabiensanglard/chocolate_duke3D", { GameVersion::REGULAR_VERSION.getName(), GameVersion::ATOMIC_EDITION.getName() });
const GameVersion GameVersion::BELGIAN_CHOCOLATE_DUKE3D("Belgian Chocolate Duke3D",    "", "BelgianChocolateDuke3D.exe", false, "/x",  "/g",  "-map ", "BelgianChocolate", {},          {},    {},   "",                                                              "https://github.com/GPSnoopy/BelgianChocolateDuke3D", { GameVersion::REGULAR_VERSION.getName(), GameVersion::ATOMIC_EDITION.getName() });

const std::vector<GameVersion> GameVersion::DEFAULT_GAME_VERSIONS = {
	GameVersion::REGULAR_VERSION,
	GameVersion::ATOMIC_EDITION,
	GameVersion::JFDUKE3D,
	GameVersion::EDUKE32,
//	GameVersion::MEGATON_EDITION,
	GameVersion::WORLD_TOUR,
//	GameVersion::BUILDGDX,
	GameVersion::RAZE,
	GameVersion::REDNUKEM,
	GameVersion::CHOCOLATE_DUKE3D,
	GameVersion::BELGIAN_CHOCOLATE_DUKE3D
};

GameVersion::GameVersion(const std::string & name, const std::string & gamePath, const std::string & gameExecutableName, bool relativeConFilePath, const std::string & conFileArgumentFlag, const std::string & groupFileArgumentFlag, const std::string & mapFileArgumentFlag, const std::string & modDirectoryName, std::optional<std::string> setupExecutableName, std::optional<std::string> defFileArgumentFlag, std::optional<bool> requiresDOSBox, const std::string & website, const std::string & sourceCodeURL, const std::vector<std::string> & compatibleGameVersions)
	: m_name(Utilities::trimString(name))
	, m_gamePath(Utilities::trimString(gamePath))
	, m_gameExecutableName(Utilities::trimString(gameExecutableName))
	, m_requiresDOSBox(requiresDOSBox)
	, m_modDirectoryName(Utilities::trimString(modDirectoryName))
	, m_website(Utilities::trimString(website))
	, m_sourceCodeURL(Utilities::trimString(sourceCodeURL))
	, m_relativeConFilePath(relativeConFilePath)
	, m_conFileArgumentFlag(conFileArgumentFlag)
	, m_groupFileArgumentFlag(groupFileArgumentFlag)
	, m_defFileArgumentFlag(defFileArgumentFlag)
	, m_mapFileArgumentFlag(mapFileArgumentFlag) {
	if(setupExecutableName.has_value()) {
		m_setupExecutableName = Utilities::trimString(setupExecutableName.value());
	}

	addCompatibleGameVersions(compatibleGameVersions);
}

GameVersion::GameVersion(GameVersion && gameVersion) noexcept
	: m_name(std::move(gameVersion.m_name))
	, m_gamePath(std::move(gameVersion.m_gamePath))
	, m_gameExecutableName(std::move(gameVersion.m_gameExecutableName))
	, m_setupExecutableName(std::move(gameVersion.m_setupExecutableName))
	, m_requiresDOSBox(gameVersion.m_requiresDOSBox)
	, m_modDirectoryName(std::move(gameVersion.m_modDirectoryName))
	, m_website(std::move(gameVersion.m_website))
	, m_sourceCodeURL(std::move(gameVersion.m_sourceCodeURL))
	, m_relativeConFilePath(gameVersion.m_relativeConFilePath)
	, m_conFileArgumentFlag(std::move(gameVersion.m_conFileArgumentFlag))
	, m_groupFileArgumentFlag(std::move(gameVersion.m_groupFileArgumentFlag))
	, m_defFileArgumentFlag(std::move(gameVersion.m_defFileArgumentFlag))
	, m_mapFileArgumentFlag(std::move(gameVersion.m_mapFileArgumentFlag))
	, m_compatibleGameVersions(std::move(gameVersion.m_compatibleGameVersions)) { }

GameVersion::GameVersion(const GameVersion & gameVersion)
	: m_name(gameVersion.m_name)
	, m_gamePath(gameVersion.m_gamePath)
	, m_gameExecutableName(gameVersion.m_gameExecutableName)
	, m_setupExecutableName(gameVersion.m_setupExecutableName)
	, m_requiresDOSBox(gameVersion.m_requiresDOSBox)
	, m_modDirectoryName(gameVersion.m_modDirectoryName)
	, m_website(gameVersion.m_website)
	, m_sourceCodeURL(gameVersion.m_sourceCodeURL)
	, m_relativeConFilePath(gameVersion.m_relativeConFilePath)
	, m_conFileArgumentFlag(gameVersion.m_conFileArgumentFlag)
	, m_groupFileArgumentFlag(gameVersion.m_groupFileArgumentFlag)
	, m_defFileArgumentFlag(gameVersion.m_defFileArgumentFlag)
	, m_mapFileArgumentFlag(gameVersion.m_mapFileArgumentFlag)
	, m_compatibleGameVersions(gameVersion.m_compatibleGameVersions) { }

GameVersion & GameVersion::operator = (GameVersion && gameVersion) noexcept {
	if(this != &gameVersion) {
		m_name = std::move(gameVersion.m_name);
		m_gamePath = std::move(gameVersion.m_gamePath);
		m_gameExecutableName = std::move(gameVersion.m_gameExecutableName);
		m_setupExecutableName = std::move(gameVersion.m_setupExecutableName);
		m_requiresDOSBox = std::move(gameVersion.m_requiresDOSBox);
		m_modDirectoryName = std::move(gameVersion.m_modDirectoryName);
		m_website = std::move(gameVersion.m_website);
		m_sourceCodeURL = std::move(gameVersion.m_sourceCodeURL);
		m_relativeConFilePath = gameVersion.m_relativeConFilePath;
		m_conFileArgumentFlag = std::move(gameVersion.m_conFileArgumentFlag);
		m_groupFileArgumentFlag = std::move(gameVersion.m_groupFileArgumentFlag);
		m_defFileArgumentFlag = std::move(gameVersion.m_defFileArgumentFlag);
		m_mapFileArgumentFlag = std::move(gameVersion.m_mapFileArgumentFlag);
		m_compatibleGameVersions = std::move(gameVersion.m_compatibleGameVersions);
	}

	return *this;
}

GameVersion & GameVersion::operator = (const GameVersion & gameVersion) {
	m_name = gameVersion.m_name;
	m_gamePath = gameVersion.m_gamePath;
	m_gameExecutableName = gameVersion.m_gameExecutableName;
	m_setupExecutableName = gameVersion.m_setupExecutableName;
	m_requiresDOSBox = gameVersion.m_requiresDOSBox;
	m_modDirectoryName = gameVersion.m_modDirectoryName;
	m_website = gameVersion.m_website;
	m_sourceCodeURL = gameVersion.m_sourceCodeURL;
	m_relativeConFilePath = gameVersion.m_relativeConFilePath;
	m_conFileArgumentFlag = gameVersion.m_conFileArgumentFlag;
	m_groupFileArgumentFlag = gameVersion.m_groupFileArgumentFlag;
	m_defFileArgumentFlag = gameVersion.m_defFileArgumentFlag;
	m_mapFileArgumentFlag = gameVersion.m_mapFileArgumentFlag;
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

bool GameVersion::hasSetupExecutableName() const {
	return m_setupExecutableName.has_value();
}

std::optional<std::string> GameVersion::getSetupExecutableName() const {
	return m_setupExecutableName;
}

void GameVersion::setGameExecutableName(const std::string & gameExecutableName) {
	m_gameExecutableName = Utilities::trimString(gameExecutableName);
}

void GameVersion::setSetupExecutableName(const std::string & setupExecutableName) {
	m_setupExecutableName = Utilities::trimString(setupExecutableName);
}

void GameVersion::clearSetupExecutableName() {
	m_setupExecutableName.reset();
}

bool GameVersion::doesRequiresDOSBox() const {
	return m_requiresDOSBox.has_value() ? m_requiresDOSBox.value() : false;
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
		if(Utilities::compareStringsIgnoreCase(m_name, (*i)->getGameVersion()) == 0) {
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
		if(Utilities::compareStringsIgnoreCase(m_name, (*i)->getGameVersion()) == 0) {
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

		fmt::print("Missing '{}' game executable: '{}'.\n", m_name, fullGameExecutablePath);
	}

	if(m_setupExecutableName.has_value()) {
		std::string fullSetupExecutablePath(Utilities::joinPaths(m_gamePath, m_setupExecutableName.value()));

		if(!std::filesystem::is_regular_file(std::filesystem::path(fullSetupExecutablePath))) {
			numberOfMissingExecutables++;

			fmt::print("Missing '{}' setup executable: '{}'.\n", m_name, fullSetupExecutablePath);
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

	gameVersionValue.AddMember(rapidjson::StringRef(JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME), rapidjson::Value(m_relativeConFilePath), allocator);

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

	if(m_setupExecutableName.has_value()) {
		rapidjson::Value setupExecutableNameValue(m_setupExecutableName.value().c_str(), allocator);
		gameVersionValue.AddMember(rapidjson::StringRef(JSON_SETUP_EXECUTABLE_NAME_PROPERTY_NAME), setupExecutableNameValue, allocator);
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
		fmt::print("Invalid game version type: '{}', expected 'object'.\n", Utilities::typeToString(gameVersionValue.GetType()));
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
			fmt::print("Game version has unexpected property '{}'.\n", i->name.GetString());
			return nullptr;
		}
	}

	// parse game version name
	if(!gameVersionValue.HasMember(JSON_NAME_PROPERTY_NAME)) {
		fmt::print("Game version is missing '{}' property.\n", JSON_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & nameValue = gameVersionValue[JSON_NAME_PROPERTY_NAME];

	if(!nameValue.IsString()) {
		fmt::print("Game version has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_NAME_PROPERTY_NAME, Utilities::typeToString(nameValue.GetType()));
		return nullptr;
	}

	std::string name(Utilities::trimString(nameValue.GetString()));

	if(name.empty()) {
		fmt::print("Game version '{}' property cannot be empty.\n", JSON_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version game path
	if(!gameVersionValue.HasMember(JSON_GAME_PATH_PROPERTY_NAME)) {
		fmt::print("Game version is missing '{}' property.\n", JSON_GAME_PATH_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & gamePathValue = gameVersionValue[JSON_GAME_PATH_PROPERTY_NAME];

	if(!gamePathValue.IsString()) {
		fmt::print("Game version has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_GAME_PATH_PROPERTY_NAME, Utilities::typeToString(gamePathValue.GetType()));
		return nullptr;
	}

	std::string gamePath(Utilities::trimString(gamePathValue.GetString()));

	// parse game version game executable name
	if(!gameVersionValue.HasMember(JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME)) {
		fmt::print("Game version is missing '{}' property.\n", JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & gameExecutableNameValue = gameVersionValue[JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME];

	if(!gameExecutableNameValue.IsString()) {
		fmt::print("Game version has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME, Utilities::typeToString(gameExecutableNameValue.GetType()));
		return nullptr;
	}

	std::string gameExecutableName(Utilities::trimString(gameExecutableNameValue.GetString()));

	if(gameExecutableName.empty()) {
		fmt::print("Game version '{}' property cannot be empty.\n", JSON_GAME_EXECUTABLE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version setup executable name
	std::optional<std::string> setupExecutableNameOptional;

	if(gameVersionValue.HasMember(JSON_SETUP_EXECUTABLE_NAME_PROPERTY_NAME)) {
		const rapidjson::Value & setupExecutableNameValue = gameVersionValue[JSON_SETUP_EXECUTABLE_NAME_PROPERTY_NAME];

		if(!setupExecutableNameValue.IsString()) {
			fmt::print("Game version has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_SETUP_EXECUTABLE_NAME_PROPERTY_NAME, Utilities::typeToString(setupExecutableNameValue.GetType()));
			return nullptr;
		}

		setupExecutableNameOptional = Utilities::trimString(setupExecutableNameValue.GetString());

		if(setupExecutableNameOptional.value().empty()) {
			fmt::print("Game version '{}' property cannot be empty.\n", JSON_SETUP_EXECUTABLE_NAME_PROPERTY_NAME);
			return nullptr;
		}
	}

	// parse relative con file path
	if(!gameVersionValue.HasMember(JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME)) {
		fmt::print("Game version is missing '{}' property.\n", JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & relativeConFilePathValue = gameVersionValue[JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME];

	if(!relativeConFilePathValue.IsBool()) {
		fmt::print("Game version has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_RELATIVE_CON_FILE_PATH_PROPERTY_NAME, Utilities::typeToString(relativeConFilePathValue.GetType()));
		return nullptr;
	}

	bool relativeConFilePath = relativeConFilePathValue.GetBool();

	// parse game version con file argument flag
	if(!gameVersionValue.HasMember(JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		fmt::print("Game version is missing '{}' property.\n", JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & conFileArgumentFlagValue = gameVersionValue[JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME];

	if(!conFileArgumentFlagValue.IsString()) {
		fmt::print("Game version has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(conFileArgumentFlagValue.GetType()));
		return nullptr;
	}

	std::string conFileArgumentFlag(conFileArgumentFlagValue.GetString());

	if(conFileArgumentFlag.empty()) {
		fmt::print("Game version '{}' property cannot be empty.\n", JSON_CON_FILE_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version group file argument flag
	if(!gameVersionValue.HasMember(JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		fmt::print("Game version is missing '{}' property.\n", JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & groupFileArgumentFlagValue = gameVersionValue[JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME];

	if(!groupFileArgumentFlagValue.IsString()) {
		fmt::print("Game version has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(groupFileArgumentFlagValue.GetType()));
		return nullptr;
	}

	std::string groupFileArgumentFlag(groupFileArgumentFlagValue.GetString());

	if(groupFileArgumentFlag.empty()) {
		fmt::print("Game version '{}' property cannot be empty.\n", JSON_GROUP_FILE_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version map file argument flag
	if(!gameVersionValue.HasMember(JSON_MAP_FILE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		fmt::print("Game version is missing '{}' property.\n", JSON_MAP_FILE_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & mapFileArgumentFlagValue = gameVersionValue[JSON_MAP_FILE_ARGUMENT_FLAG_PROPERTY_NAME];

	if(!mapFileArgumentFlagValue.IsString()) {
		fmt::print("Game version has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_MAP_FILE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(mapFileArgumentFlagValue.GetType()));
		return nullptr;
	}

	std::string mapFileArgumentFlag(mapFileArgumentFlagValue.GetString());

	if(mapFileArgumentFlag.empty()) {
		fmt::print("Game version '{}' property cannot be empty.\n", JSON_MAP_FILE_ARGUMENT_FLAG_PROPERTY_NAME);
		return nullptr;
	}

	// parse game version mod directory name
	if(!gameVersionValue.HasMember(JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME)) {
		fmt::print("Game version is missing '{}' property.\n", JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modDirectoryNameValue = gameVersionValue[JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME];

	if(!modDirectoryNameValue.IsString()) {
		fmt::print("Game version has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME, Utilities::typeToString(modDirectoryNameValue.GetType()));
		return nullptr;
	}

	std::string modDirectoryName(Utilities::trimString(modDirectoryNameValue.GetString()));

	if(modDirectoryName.empty()) {
		fmt::print("Game version '{}' property cannot be empty.\n", JSON_MOD_DIRECTORY_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// initialize the game version
	std::unique_ptr<GameVersion> newGameVersion = std::make_unique<GameVersion>(name, gamePath, gameExecutableName, relativeConFilePath, conFileArgumentFlag, groupFileArgumentFlag, mapFileArgumentFlag, modDirectoryName, setupExecutableNameOptional);

	// parse game version def file argument flag
	if(gameVersionValue.HasMember(JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME)) {
		const rapidjson::Value & defFileArgumentFlagValue = gameVersionValue[JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME];

		if(!defFileArgumentFlagValue.IsString()) {
			fmt::print("Mod file '{}' property has invalid type: '{}', expected 'string'.\n", JSON_DEF_FILE_ARGUMENT_FLAG_PROPERTY_NAME, Utilities::typeToString(defFileArgumentFlagValue.GetType()));
			return nullptr;
		}

		newGameVersion->setDefFileArgumentFlag(defFileArgumentFlagValue.GetString());
	}

	// parse the game version requires dosbox property
	if(gameVersionValue.HasMember(JSON_REQUIRES_DOSBOX_PROPERTY_NAME)) {
		const rapidjson::Value & requiresDOSBoxValue = gameVersionValue[JSON_REQUIRES_DOSBOX_PROPERTY_NAME];

		if(!requiresDOSBoxValue.IsBool()) {
			fmt::print("Game version '{}' property has invalid type: '{}', expected 'boolean'.\n", JSON_REQUIRES_DOSBOX_PROPERTY_NAME, Utilities::typeToString(requiresDOSBoxValue.GetType()));
			return nullptr;
		}

		newGameVersion->setRequiresDOSBox(requiresDOSBoxValue.GetBool());
	}

	// parse the game version website property
	if(gameVersionValue.HasMember(JSON_WEBSITE_PROPERTY_NAME)) {
		const rapidjson::Value & websiteValue = gameVersionValue[JSON_WEBSITE_PROPERTY_NAME];

		if(!websiteValue.IsString()) {
			fmt::print("Mod file '{}' property has invalid type: '{}', expected 'string'.\n", JSON_WEBSITE_PROPERTY_NAME, Utilities::typeToString(websiteValue.GetType()));
			return nullptr;
		}

		newGameVersion->setWebsite(websiteValue.GetString());
	}

	// parse the game version source code url property
	if(gameVersionValue.HasMember(JSON_SOURCE_CODE_URL_PROPERTY_NAME)) {
		const rapidjson::Value & sourceCodeURLValue = gameVersionValue[JSON_SOURCE_CODE_URL_PROPERTY_NAME];

		if(!sourceCodeURLValue.IsString()) {
			fmt::print("Mod file '{}' property has invalid type: '{}', expected 'string'.\n", JSON_SOURCE_CODE_URL_PROPERTY_NAME, Utilities::typeToString(sourceCodeURLValue.GetType()));
			return nullptr;
		}

		newGameVersion->setSourceCodeURL(sourceCodeURLValue.GetString());
	}

	// parse the compatible game versions property
	if(gameVersionValue.HasMember(JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME)) {
		const rapidjson::Value & compatibleGameVersionsValue = gameVersionValue[JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME];

		if(!compatibleGameVersionsValue.IsArray()) {
			fmt::print("Game version '{}' property has invalid type: '{}', expected 'array'.\n", JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME, Utilities::typeToString(compatibleGameVersionsValue.GetType()));
			return nullptr;
		}

		for(rapidjson::Value::ConstValueIterator i = compatibleGameVersionsValue.Begin(); i != compatibleGameVersionsValue.End(); ++i) {
			if(!i->IsString()) {
				fmt::print("Game version '{}' property contains invalid compatible game version type: '{}', expected 'string'.\n", JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME, Utilities::typeToString(i->GetType()));
				return nullptr;
			}

			std::string compatibleGameVersion(Utilities::trimString(i->GetString()));

			if(newGameVersion->hasCompatibleGameVersion(compatibleGameVersion)) {
				fmt::print("Game version '{}' property contains duplicate compatible game version: '{}'.\n", JSON_COMPATIBLE_GAME_VERSIONS_PROPERTY_NAME, compatibleGameVersion);
				return nullptr;
			}

			newGameVersion->m_compatibleGameVersions.push_back(compatibleGameVersion);
		}
	}

	return newGameVersion;
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
	   m_mapFileArgumentFlag.empty()) {
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
	   Utilities::compareStringsIgnoreCase(m_name, gameVersion.m_name) != 0 ||
	   m_gamePath != gameVersion.m_gamePath ||
	   m_gameExecutableName != gameVersion.m_gameExecutableName ||
	   m_setupExecutableName != gameVersion.m_setupExecutableName ||
	   m_modDirectoryName != gameVersion.m_modDirectoryName ||
	   m_website != gameVersion.m_website ||
	   m_sourceCodeURL != gameVersion.m_sourceCodeURL ||
	   m_relativeConFilePath != gameVersion.m_relativeConFilePath ||
	   m_conFileArgumentFlag != gameVersion.m_conFileArgumentFlag ||
	   m_groupFileArgumentFlag != gameVersion.m_groupFileArgumentFlag ||
	   m_defFileArgumentFlag != gameVersion.m_defFileArgumentFlag ||
	   m_mapFileArgumentFlag != gameVersion.m_mapFileArgumentFlag ||
	   m_compatibleGameVersions.size() != gameVersion.m_compatibleGameVersions.size()) {
		return false;
	}

	for(size_t i = 0; i < m_compatibleGameVersions.size(); i++) {
		if(Utilities::compareStringsIgnoreCase(m_compatibleGameVersions[i], gameVersion.m_compatibleGameVersions[i]) != 0) {
			return false;
		}
	}

	return true;
}

bool GameVersion::operator != (const GameVersion & gameVersion) const {
	return !operator == (gameVersion);
}
