#include "GameVersionCollection.h"

#include "GameVersion.h"
#include "Mod/ModGameVersion.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/Utilities.h>

#include <magic_enum.hpp>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>

static constexpr const char * JSON_FILE_TYPE_PROPERTY_NAME = "fileType";
static constexpr const char * JSON_FILE_FORMAT_VERSION_PROPERTY_NAME = "fileFormatVersion";
static constexpr const char * JSON_GAME_VERSIONS_PROPERTY_NAME = "gameVersions";

const std::string GameVersionCollection::FILE_TYPE = "Game Versions";
const std::string GameVersionCollection::FILE_FORMAT_VERSION = "1.0.0";

GameVersionCollection::GameVersionCollection() = default;

GameVersionCollection::GameVersionCollection(const std::vector<GameVersion> & gameVersions) {
	addGameVersions(gameVersions);
}

GameVersionCollection::GameVersionCollection(const std::vector<const GameVersion *> & gameVersions) {
	addGameVersions(gameVersions);
}

GameVersionCollection::GameVersionCollection(const std::vector<std::shared_ptr<GameVersion>> & gameVersions) {
	addGameVersions(gameVersions);
}

GameVersionCollection::GameVersionCollection(GameVersionCollection && g) noexcept
	: m_gameVersions(std::move(g.m_gameVersions)) {
	for(std::shared_ptr<GameVersion> & gameVersion : m_gameVersions) {
		m_gameVersionConnections.push_back(gameVersion->modified.connect(std::bind(&GameVersionCollection::onGameVersionModified, this, std::placeholders::_1)));
	}
}

GameVersionCollection::GameVersionCollection(const GameVersionCollection & g) {
	for(std::vector<std::shared_ptr<GameVersion>>::const_iterator i = g.m_gameVersions.begin(); i != g.m_gameVersions.end(); ++i) {
		m_gameVersions.push_back(std::make_shared<GameVersion>(**i));
		m_gameVersionConnections.push_back(m_gameVersions.back()->modified.connect(std::bind(&GameVersionCollection::onGameVersionModified, this, std::placeholders::_1)));
	}
}

GameVersionCollection & GameVersionCollection::operator = (GameVersionCollection && g) noexcept {
	if(this != &g) {
		for(boost::signals2::connection & gameVersionConnection : m_gameVersionConnections) {
			gameVersionConnection.disconnect();
		}

		m_gameVersionConnections.clear();

		m_gameVersions = std::move(g.m_gameVersions);

		for(std::shared_ptr<GameVersion> & gameVersion : m_gameVersions) {
			m_gameVersionConnections.push_back(gameVersion->modified.connect(std::bind(&GameVersionCollection::onGameVersionModified, this, std::placeholders::_1)));
		}
	}

	return *this;
}

GameVersionCollection & GameVersionCollection::operator = (const GameVersionCollection & g) {
	m_gameVersions.clear();

	for(std::vector<std::shared_ptr<GameVersion>>::const_iterator i = g.m_gameVersions.begin(); i != g.m_gameVersions.end(); ++i) {
		m_gameVersions.push_back(std::make_shared<GameVersion>(**i));
		m_gameVersionConnections.push_back(m_gameVersions.back()->modified.connect(std::bind(&GameVersionCollection::onGameVersionModified, this, std::placeholders::_1)));
	}

	return *this;
}

GameVersionCollection::~GameVersionCollection() {
	for(boost::signals2::connection & gameVersionConnection : m_gameVersionConnections) {
		gameVersionConnection.disconnect();
	}
}

size_t GameVersionCollection::numberOfGameVersions() const {
	return m_gameVersions.size();
}

bool GameVersionCollection::hasGameVersion(const GameVersion & gameVersion) const {
	return indexOfGameVersion(gameVersion) != std::numeric_limits<size_t>::max();
}

bool GameVersionCollection::hasGameVersionWithID(const std::string & gameVersionID) const {
	return indexOfGameVersionWithID(gameVersionID) != std::numeric_limits<size_t>::max();
}

size_t GameVersionCollection::indexOfGameVersion(const GameVersion & gameVersion) const {
	auto gameVersionIterator = std::find_if(m_gameVersions.cbegin(), m_gameVersions.cend(), [&gameVersion](const std::shared_ptr<GameVersion> & currentGameVersion) {
		return &gameVersion == currentGameVersion.get();
	});

	if(gameVersionIterator == m_gameVersions.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return gameVersionIterator - m_gameVersions.cbegin();
}

size_t GameVersionCollection::indexOfGameVersionWithID(const std::string & gameVersionID) const {
	if(gameVersionID.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	auto gameVersionIterator = std::find_if(m_gameVersions.cbegin(), m_gameVersions.cend(), [&gameVersionID](const std::shared_ptr<GameVersion> & currentGameVersion) {
		return Utilities::areStringsEqualIgnoreCase(gameVersionID, currentGameVersion->getID());
	});

	if(gameVersionIterator == m_gameVersions.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return gameVersionIterator - m_gameVersions.cbegin();
}

std::shared_ptr<GameVersion> GameVersionCollection::getGameVersion(size_t index) const {
	if(index >= m_gameVersions.size()) {
		return nullptr;
	}

	return m_gameVersions[index];
}

std::shared_ptr<GameVersion> GameVersionCollection::getGameVersionWithID(const std::string & gameVersionID) const {
	return getGameVersion(indexOfGameVersionWithID(gameVersionID));
}

std::string GameVersionCollection::getLongNameOfGameVersionWithID(const std::string & gameVersionID) const {
	std::shared_ptr<GameVersion> gameVersion(getGameVersionWithID(gameVersionID));

	if(gameVersion == nullptr) {
		return {};
	}

	return gameVersion->getLongName();
}

std::string GameVersionCollection::getShortNameOfGameVersionWithID(const std::string & gameVersionID) const {
	std::shared_ptr<GameVersion> gameVersion(getGameVersionWithID(gameVersionID));

	if(gameVersion == nullptr) {
		return {};
	}

	return gameVersion->getShortName();
}

const std::vector<std::shared_ptr<GameVersion>> & GameVersionCollection::getGameVersions() const {
	return m_gameVersions;
}

std::vector<std::shared_ptr<GameVersion>> GameVersionCollection::getGameVersionsCompatibleWith(size_t index, bool includeSupported, std::optional<bool> configured) const {
	std::vector<std::shared_ptr<GameVersion>> compatibleGameVersions;

	if(index >= m_gameVersions.size()) {
		return compatibleGameVersions;
	}

	const std::shared_ptr<GameVersion> gameVersion = m_gameVersions[index];

	if(includeSupported) {
		compatibleGameVersions.push_back(gameVersion);
	}

	for(std::vector<std::shared_ptr<GameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		if(configured.has_value()) {
			if(configured.value() && !(*i)->isConfigured()) {
				continue;
			}
			else if(!configured.value() && (*i)->isConfigured()) {
				continue;
			}
		}

		if((*i)->hasCompatibleGameVersionWithID(gameVersion->getID())) {
			compatibleGameVersions.push_back(*i);
		}
	}

	return compatibleGameVersions;
}

std::vector<std::shared_ptr<GameVersion>> GameVersionCollection::getGameVersionsCompatibleWith(const std::string & gameVersionID, bool includeSupported, std::optional<bool> configured) const {
	return getGameVersionsCompatibleWith(indexOfGameVersionWithID(gameVersionID), includeSupported, configured);
}

std::vector<std::shared_ptr<GameVersion>> GameVersionCollection::getGameVersionsCompatibleWith(const GameVersion & gameVersion, bool includeSupported, std::optional<bool> configured) const {
	return getGameVersionsCompatibleWith(indexOfGameVersion(gameVersion), includeSupported, configured);
}

std::vector<std::shared_ptr<GameVersion>> GameVersionCollection::getGameVersionsCompatibleWith(const ModGameVersion & modGameVersion, bool includeSupported, std::optional<bool> configured) const {
	return getGameVersionsCompatibleWith(indexOfGameVersionWithID(modGameVersion.getGameVersionID()), includeSupported, configured);
}

std::vector<std::pair<std::shared_ptr<GameVersion>, std::vector<std::shared_ptr<ModGameVersion>>>> GameVersionCollection::getGameVersionsCompatibleWith(const std::vector<std::shared_ptr<ModGameVersion>> & modGameVersions, bool includeSupported, std::optional<bool> configured) const {
	std::vector<std::pair<std::shared_ptr<GameVersion>, std::vector<std::shared_ptr<ModGameVersion>>>> allCompatibleGameVersions;

	for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = modGameVersions.begin(); i != modGameVersions.end(); ++i) {
		bool shouldAddGameVersion = true;
		std::vector<std::shared_ptr<GameVersion>> compatibleGameVersions(getGameVersionsCompatibleWith(**i, includeSupported, configured));

		for(std::vector<std::shared_ptr<GameVersion>>::const_iterator j = compatibleGameVersions.begin(); j != compatibleGameVersions.end(); ++j) {
			for(std::vector<std::pair<std::shared_ptr<GameVersion>, std::vector<std::shared_ptr<ModGameVersion>>>>::iterator k = allCompatibleGameVersions.begin(); k != allCompatibleGameVersions.end(); ++k) {
				if(*(*k).first == **j) {
					(*k).second.push_back(*i);
					shouldAddGameVersion = false;
					break;
				}
			}

			if(shouldAddGameVersion) {
				std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions = { *i };
				allCompatibleGameVersions.emplace_back(*j, compatibleModGameVersions);
			}
		}
	}

	return allCompatibleGameVersions;
}

std::vector<std::shared_ptr<GameVersion>> GameVersionCollection::getConfiguredGameVersions() const {
	std::vector<std::shared_ptr<GameVersion>> configuredGameVersions;

	for(std::vector<std::shared_ptr<GameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		if(!(*i)->isConfigured()) {
			continue;
		}

		configuredGameVersions.push_back(*i);
	}

	return configuredGameVersions;
}

std::vector<std::shared_ptr<GameVersion>> GameVersionCollection::getUnconfiguredGameVersions() const {
	std::vector<std::shared_ptr<GameVersion>> unconfiguredGameVersions;

	for(std::vector<std::shared_ptr<GameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		if((*i)->isConfigured()) {
			continue;
		}

		unconfiguredGameVersions.push_back(*i);
	}

	return unconfiguredGameVersions;
}

std::vector<std::string> GameVersionCollection::getGameVersionIdentifiers() const {
	return getGameVersionIdentifiersFrom(m_gameVersions);
}

std::vector<std::string> GameVersionCollection::getGameVersionIdentifiersFrom(const std::vector<std::shared_ptr<GameVersion>> & gameVersions) {
	std::vector<std::string> gameVersionIdentifiers;

	for(size_t i = 0; i < gameVersions.size(); i++) {
		gameVersionIdentifiers.push_back(gameVersions[i]->getID());
	}

	return gameVersionIdentifiers;
}

std::vector<std::string> GameVersionCollection::getGameVersionIdentifiersFrom(const std::vector<const GameVersion *> & gameVersions) {
	std::vector<std::string> gameVersionIdentifiers;

	for(size_t i = 0; i < gameVersions.size(); i++) {
		gameVersionIdentifiers.push_back(gameVersions[i]->getID());
	}

	return gameVersionIdentifiers;
}

std::vector<std::string> GameVersionCollection::getGameVersionLongNames(bool prependItemNumber) const {
	return getGameVersionLongNamesFrom(m_gameVersions, prependItemNumber);
}

std::vector<std::string> GameVersionCollection::getGameVersionLongNamesFrom(const std::vector<std::shared_ptr<GameVersion>> & gameVersions, bool prependItemNumber) {
	std::vector<std::string> gameVersionLongNames;

	for(size_t i = 0; i < gameVersions.size(); i++) {
		std::stringstream gameVersionStream;

		if(prependItemNumber) {
			gameVersionStream << i + 1 << ": ";
		}

		gameVersionStream << gameVersions[i]->getLongName();

		gameVersionLongNames.push_back(gameVersionStream.str());
	}

	return gameVersionLongNames;
}

std::vector<std::string> GameVersionCollection::getGameVersionLongNamesFrom(const std::vector<const GameVersion *> & gameVersions, bool prependItemNumber) {
	std::vector<std::string> gameVersionLongNames;

	for(size_t i = 0; i < gameVersions.size(); i++) {
		std::stringstream gameVersionStream;

		if(prependItemNumber) {
			gameVersionStream << i + 1 << ": ";
		}

		gameVersionStream << gameVersions[i]->getLongName();

		gameVersionLongNames.push_back(gameVersionStream.str());
	}

	return gameVersionLongNames;
}

std::vector<std::string> GameVersionCollection::getGameVersionShortNames(bool prependItemNumber) const {
	return getGameVersionShortNamesFrom(m_gameVersions, prependItemNumber);
}

std::vector<std::string> GameVersionCollection::getGameVersionShortNamesFrom(const std::vector<std::shared_ptr<GameVersion>> & gameVersions, bool prependItemNumber) {
	std::vector<std::string> gameVersionShortNames;

	for(size_t i = 0; i < gameVersions.size(); i++) {
		std::stringstream gameVersionStream;

		if(prependItemNumber) {
			gameVersionStream << i + 1 << ": ";
		}

		gameVersionStream << gameVersions[i]->getShortName();

		gameVersionShortNames.push_back(gameVersionStream.str());
	}

	return gameVersionShortNames;
}

std::vector<std::string> GameVersionCollection::getGameVersionShortNamesFrom(const std::vector<const GameVersion *> & gameVersions, bool prependItemNumber) {
	std::vector<std::string> gameVersionShortNames;

	for(size_t i = 0; i < gameVersions.size(); i++) {
		std::stringstream gameVersionStream;

		if(prependItemNumber) {
			gameVersionStream << i + 1 << ": ";
		}

		gameVersionStream << gameVersions[i]->getShortName();

		gameVersionShortNames.push_back(gameVersionStream.str());
	}

	return gameVersionShortNames;
}

bool GameVersionCollection::addGameVersion(const GameVersion & gameVersion) {
	if(!gameVersion.isValid() || hasGameVersionWithID(gameVersion.getID())) {
		return false;
	}

	m_gameVersions.push_back(std::make_shared<GameVersion>(gameVersion));
	m_gameVersionConnections.push_back(m_gameVersions.back()->modified.connect(std::bind(&GameVersionCollection::onGameVersionModified, this, std::placeholders::_1)));

	sizeChanged(*this);

	return true;
}

bool GameVersionCollection::addGameVersion(std::shared_ptr<GameVersion> gameVersion) {
	if(!GameVersion::isValid(gameVersion.get()) || hasGameVersionWithID(gameVersion->getID())) {
		return false;
	}

	m_gameVersions.push_back(gameVersion);
	m_gameVersionConnections.push_back(m_gameVersions.back()->modified.connect(std::bind(&GameVersionCollection::onGameVersionModified, this, std::placeholders::_1)));

	sizeChanged(*this);

	return true;
}

size_t GameVersionCollection::addGameVersions(const std::vector<GameVersion> & gameVersions) {
	size_t numberOfGameVersionsAdded = 0;

	for(std::vector<GameVersion>::const_iterator i = gameVersions.begin(); i != gameVersions.end(); ++i) {
		if(addGameVersion(*i)) {
			numberOfGameVersionsAdded++;
		}
	}

	return numberOfGameVersionsAdded;
}

size_t GameVersionCollection::addGameVersions(const std::vector<const GameVersion *> & gameVersions) {
	size_t numberOfGameVersionsAdded = 0;

	for(std::vector<const GameVersion *>::const_iterator i = gameVersions.begin(); i != gameVersions.end(); ++i) {
		if(addGameVersion(**i)) {
			numberOfGameVersionsAdded++;
		}
	}

	return numberOfGameVersionsAdded;
}

size_t GameVersionCollection::addGameVersions(const std::vector<std::shared_ptr<GameVersion>> & gameVersions) {
	size_t numberOfGameVersionsAdded = 0;

	for(std::vector<std::shared_ptr<GameVersion>>::const_iterator i = gameVersions.begin(); i != gameVersions.end(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if(addGameVersion(*i)) {
			numberOfGameVersionsAdded++;
		}
	}

	return numberOfGameVersionsAdded;
}

bool GameVersionCollection::removeGameVersion(size_t index) {
	if(index >= m_gameVersions.size()) {
		return false;
	}

	m_gameVersionConnections[index].disconnect();
	m_gameVersionConnections.erase(m_gameVersionConnections.begin() + index);
	m_gameVersions.erase(m_gameVersions.begin() + index);

	sizeChanged(*this);

	return true;
}

bool GameVersionCollection::removeGameVersion(const GameVersion & gameVersion) {
	return removeGameVersion(indexOfGameVersion(gameVersion));
}

bool GameVersionCollection::removeGameVersionWithID(const std::string & gameVersionID) {
	return removeGameVersion(indexOfGameVersionWithID(gameVersionID));
}

size_t GameVersionCollection::addMissingDefaultGameVersions() {
	size_t numberOfGameVersionsAdded = 0;

	for(std::vector<const GameVersion *>::const_iterator i = GameVersion::DEFAULT_GAME_VERSIONS.begin(); i != GameVersion::DEFAULT_GAME_VERSIONS.end(); ++i) {
		if(hasGameVersionWithID((*i)->getID())) {
			continue;
		}

		spdlog::info("Adding missing default game version '{}'.", (*i)->getLongName());

		addGameVersion(**i);

		numberOfGameVersionsAdded++;
	}

	return numberOfGameVersionsAdded;
}

void GameVersionCollection::setDefaultGameVersions() {
	clearGameVersions();

	addGameVersions(GameVersion::DEFAULT_GAME_VERSIONS);
}

void GameVersionCollection::clearGameVersions() {
	for(boost::signals2::connection & gameVersionConnection : m_gameVersionConnections) {
		gameVersionConnection.disconnect();
	}

	m_gameVersionConnections.clear();
	m_gameVersions.clear();

	sizeChanged(*this);
}

size_t GameVersionCollection::checkForMissingExecutables() const {
	size_t numberOfMissingExecutables = 0;

	for(std::vector<std::shared_ptr<GameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		numberOfMissingExecutables += (*i)->checkForMissingExecutables();
	}

	return numberOfMissingExecutables++;
}

size_t GameVersionCollection::checkForMissingExecutables(const std::string & gameVersionID) const {
	std::shared_ptr<GameVersion> gameVersion(getGameVersionWithID(gameVersionID));

	if(gameVersion == nullptr) {
		return 0;
	}

	return gameVersion->checkForMissingExecutables();
}

rapidjson::Document GameVersionCollection::toJSON() const {
	rapidjson::Document gameVersionsDocument(rapidjson::kObjectType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = gameVersionsDocument.GetAllocator();

	rapidjson::Value fileTypeValue(FILE_TYPE.c_str(), allocator);
	gameVersionsDocument.AddMember(rapidjson::StringRef(JSON_FILE_TYPE_PROPERTY_NAME), fileTypeValue, allocator);

	rapidjson::Value fileFormatVersionValue(FILE_FORMAT_VERSION.c_str(), allocator);
	gameVersionsDocument.AddMember(rapidjson::StringRef(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME), fileFormatVersionValue, allocator);

	rapidjson::Value gamesVersionsValue(rapidjson::kArrayType);

	for(std::vector<std::shared_ptr<GameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		gamesVersionsValue.PushBack((*i)->toJSON(allocator), allocator);
	}

	gameVersionsDocument.AddMember(rapidjson::StringRef(JSON_GAME_VERSIONS_PROPERTY_NAME), gamesVersionsValue, allocator);

	return gameVersionsDocument;
}

std::unique_ptr<GameVersionCollection> GameVersionCollection::parseFrom(const rapidjson::Value & gameVersionCollectionValue) {
	if(!gameVersionCollectionValue.IsObject()) {
		spdlog::error("Invalid game version collection type: '{}', expected 'object'.", Utilities::typeToString(gameVersionCollectionValue.GetType()));
		return nullptr;
	}

	if(gameVersionCollectionValue.HasMember(JSON_FILE_TYPE_PROPERTY_NAME)) {
		const rapidjson::Value & fileTypeValue = gameVersionCollectionValue[JSON_FILE_TYPE_PROPERTY_NAME];

		if(!fileTypeValue.IsString()) {
			spdlog::error("Invalid game version collection file type type: '{}', expected: 'string'.", Utilities::typeToString(fileTypeValue.GetType()));
			return false;
		}

		if(!Utilities::areStringsEqualIgnoreCase(fileTypeValue.GetString(), FILE_TYPE)) {
			spdlog::error("Incorrect game version collection file type: '{}', expected: '{}'.", fileTypeValue.GetString(), FILE_TYPE);
			return false;
		}
	}
	else {
		spdlog::warn("Game version collection JSON data is missing file type, and may fail to load correctly!");
	}

	if(gameVersionCollectionValue.HasMember(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & fileFormatVersionValue = gameVersionCollectionValue[JSON_FILE_FORMAT_VERSION_PROPERTY_NAME];

		if(!fileFormatVersionValue.IsString()) {
			spdlog::error("Invalid game version collection file format version type: '{}', expected: 'string'.", Utilities::typeToString(fileFormatVersionValue.GetType()));
			return false;
		}

		std::optional<std::uint8_t> optionalVersionComparison(Utilities::compareVersions(fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION));

		if(!optionalVersionComparison.has_value()) {
			spdlog::error("Invalid game version collection file format version: '{}'.", fileFormatVersionValue.GetString());
			return false;
		}

		if(*optionalVersionComparison != 0) {
			spdlog::error("Unsupported game version collection file format version: '{}', only version '{}' is supported.", fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION);
			return false;
		}
	}
	else {
		spdlog::warn("Game version collection JSON data is missing file format version, and may fail to load correctly!");
	}

	if(!gameVersionCollectionValue.HasMember(JSON_GAME_VERSIONS_PROPERTY_NAME)) {
		spdlog::error("Game version collection is missing '{}' property.", JSON_GAME_VERSIONS_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & gameVersionsValue = gameVersionCollectionValue[JSON_GAME_VERSIONS_PROPERTY_NAME];

	if(!gameVersionsValue.IsArray()) {
		spdlog::error("Invalid game version collection '{}' type: '{}', expected 'array'.", JSON_GAME_VERSIONS_PROPERTY_NAME, Utilities::typeToString(gameVersionsValue.GetType()));
		return nullptr;
	}

	std::unique_ptr<GameVersionCollection> newGameVersionCollection(std::make_unique<GameVersionCollection>());

	if(gameVersionsValue.Empty()) {
		return newGameVersionCollection;
	}

	std::unique_ptr<GameVersion> newGameVersion;

	for(rapidjson::Value::ConstValueIterator i = gameVersionsValue.Begin(); i != gameVersionsValue.End(); ++i) {
		newGameVersion = GameVersion::parseFrom(*i);

		if(!GameVersion::isValid(newGameVersion.get())) {
			spdlog::error("Failed to parse game version #{}{}!", newGameVersionCollection->m_gameVersions.size() + 1, newGameVersionCollection->numberOfGameVersions() == 0 ? "" : fmt::format(" (after game version '{}')", newGameVersionCollection->getGameVersion(newGameVersionCollection->numberOfGameVersions() - 1)->getLongName()));
			return nullptr;
		}

		if(newGameVersionCollection->hasGameVersion(*newGameVersion.get())) {
			spdlog::error("Encountered duplicate game version #{}{}.", newGameVersionCollection->m_gameVersions.size() + 1, newGameVersionCollection->numberOfGameVersions() == 0 ? "" : fmt::format(" (after game version '{}')", newGameVersionCollection->getGameVersion(newGameVersionCollection->numberOfGameVersions() - 1)->getLongName()));
			return nullptr;
		}

		newGameVersionCollection->m_gameVersions.push_back(std::shared_ptr<GameVersion>(newGameVersion.release()));
	}

	return newGameVersionCollection;
}

bool GameVersionCollection::loadFrom(const std::string & filePath, bool autoCreate) {
	if(filePath.empty()) {
		return false;
	}

	std::string fileExtension(Utilities::getFileExtension(filePath));

	if(fileExtension.empty()) {
		return false;
	}
	else if(Utilities::areStringsEqualIgnoreCase(fileExtension, "json")) {
		return loadFromJSON(filePath, autoCreate);
	}

	return false;
}

bool GameVersionCollection::loadFromJSON(const std::string & filePath, bool autoCreate) {
	if(filePath.empty()) {
		return false;
	}

	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		if(autoCreate) {
			saveToJSON(filePath);
		}

		return false;
	}

	std::ifstream fileStream(filePath);

	if(!fileStream.is_open()) {
		return false;
	}

	rapidjson::Document gameVersionsValue;
	rapidjson::IStreamWrapper fileStreamWrapper(fileStream);
	gameVersionsValue.ParseStream(fileStreamWrapper);

	fileStream.close();

	std::unique_ptr<GameVersionCollection> gameVersionCollection(parseFrom(gameVersionsValue));

	if(!GameVersionCollection::isValid(gameVersionCollection.get())) {
		spdlog::error("Failed to parse game version collection from JSON file '{}'.", filePath);
		return false;
	}

	m_gameVersions = gameVersionCollection->m_gameVersions;

	sizeChanged(*this);

	return true;
}

bool GameVersionCollection::saveTo(const std::string & filePath, bool overwrite) const {
	if(filePath.empty()) {
		return false;
	}

	std::string fileExtension(Utilities::getFileExtension(filePath));

	if(fileExtension.empty()) {
		return false;
	}
	else if(Utilities::areStringsEqualIgnoreCase(fileExtension, "json")) {
		return saveToJSON(filePath, overwrite);
	}

	return false;
}

bool GameVersionCollection::saveToJSON(const std::string & filePath, bool overwrite) const {
	if (!overwrite && std::filesystem::exists(std::filesystem::path(filePath))) {
		spdlog::warn("File '{}' already exists, use overwrite to force write.", filePath);
		return false;
	}

	std::ofstream fileStream(filePath);

	if(!fileStream.is_open()) {
		return false;
	}

	rapidjson::Document gameVersions(toJSON());

	rapidjson::OStreamWrapper fileStreamWrapper(fileStream);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> fileStreamWriter(fileStreamWrapper);
	fileStreamWriter.SetIndent('\t', 1);
	gameVersions.Accept(fileStreamWriter);
	fileStream.close();

	return true;
}

bool GameVersionCollection::isValid() const {
	for(std::vector<std::shared_ptr<GameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		if(!(*i)->isValid()) {
			return false;
		}

		for(std::vector<std::shared_ptr<GameVersion>>::const_iterator j = i + 1; j != m_gameVersions.end(); ++j) {
			if(Utilities::areStringsEqualIgnoreCase((*i)->getID(), (*j)->getID()) ||
			   (*i)->getModDirectoryName() == (*j)->getModDirectoryName()) {
				return false;
			}
		}
	}

	return true;
}

bool GameVersionCollection::isValid(const GameVersionCollection * g) {
	return g != nullptr && g->isValid();
}

bool GameVersionCollection::operator == (const GameVersionCollection & g) const {
	if(m_gameVersions.size() != g.m_gameVersions.size()) {
		return false;
	}

	for(size_t i = 0; i < g.m_gameVersions.size(); i++) {
		if(*m_gameVersions[i] != *g.m_gameVersions[i]) {
			return false;
		}
	}

	return true;
}

bool GameVersionCollection::operator != (const GameVersionCollection & g) const {
	return !operator == (g);
}

void GameVersionCollection::onGameVersionModified(GameVersion & gameVersion) {
	itemModified(*this, gameVersion);
}
