#include "GameVersionCollection.h"

#include "GameVersion.h"
#include "GameVersionCollectionListener.h"
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

static constexpr const char * JSON_FILE_FORMAT_VERSION_PROPERTY_NAME = "fileFormatVersion";
static constexpr const char * JSON_GAME_VERSIONS_PROPERTY_NAME = "gameVersions";

const std::string GameVersionCollection::FILE_FORMAT_VERSION = "1.0.0";

GameVersionCollection::GameVersionCollection() = default;

GameVersionCollection::GameVersionCollection(const std::vector<GameVersion> & gameVersions) {
	addGameVersions(gameVersions);
}

GameVersionCollection::GameVersionCollection(const std::vector<std::shared_ptr<GameVersion>> & gameVersions) {
	addGameVersions(gameVersions);
}

GameVersionCollection::GameVersionCollection(GameVersionCollection && g) noexcept
	: m_gameVersions(std::move(g.m_gameVersions)) { }

GameVersionCollection::GameVersionCollection(const GameVersionCollection & g) {
	for(std::vector<std::shared_ptr<GameVersion>>::const_iterator i = g.m_gameVersions.begin(); i != g.m_gameVersions.end(); ++i) {
		m_gameVersions.push_back(std::make_shared<GameVersion>(**i));
	}
}

GameVersionCollection & GameVersionCollection::operator = (GameVersionCollection && g) noexcept {
	if(this != &g) {
		m_gameVersions = std::move(g.m_gameVersions);
	}

	return *this;
}

GameVersionCollection & GameVersionCollection::operator = (const GameVersionCollection & g) {
	m_gameVersions.clear();

	for(std::vector<std::shared_ptr<GameVersion>>::const_iterator i = g.m_gameVersions.begin(); i != g.m_gameVersions.end(); ++i) {
		m_gameVersions.push_back(std::make_shared<GameVersion>(**i));
	}

	return *this;
}

GameVersionCollection::~GameVersionCollection() { }

size_t GameVersionCollection::numberOfGameVersions() const {
	return m_gameVersions.size();
}

bool GameVersionCollection::hasGameVersion(const GameVersion & gameVersion) const {
	for(std::vector<std::shared_ptr<GameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), gameVersion.getName())) {
			return true;
		}
	}

	return false;
}

bool GameVersionCollection::hasGameVersion(const std::string & name) const {
	if(name.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<GameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			return true;
		}
	}

	return false;
}

size_t GameVersionCollection::indexOfGameVersion(const GameVersion & gameVersion) const {
	for(size_t i = 0; i < m_gameVersions.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_gameVersions[i]->getName(), gameVersion.getName())) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t GameVersionCollection::indexOfGameVersion(const std::string & name) const {
	if(name.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_gameVersions.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_gameVersions[i]->getName(), name)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<GameVersion> GameVersionCollection::getGameVersion(size_t index) const {
	if(index >= m_gameVersions.size()) {
		return nullptr;
	}

	return m_gameVersions[index];
}

std::shared_ptr<GameVersion> GameVersionCollection::getGameVersion(const std::string & name) const {
	if(name.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<GameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			return *i;
		}
	}

	return nullptr;
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

		if((*i)->hasCompatibleGameVersion(gameVersion->getName())) {
			compatibleGameVersions.push_back(*i);
		}
	}

	return compatibleGameVersions;
}

std::vector<std::shared_ptr<GameVersion>> GameVersionCollection::getGameVersionsCompatibleWith(const std::string & name, bool includeSupported, std::optional<bool> configured) const {
	return getGameVersionsCompatibleWith(indexOfGameVersion(name), includeSupported, configured);
}

std::vector<std::shared_ptr<GameVersion>> GameVersionCollection::getGameVersionsCompatibleWith(const GameVersion & gameVersion, bool includeSupported, std::optional<bool> configured) const {
	return getGameVersionsCompatibleWith(indexOfGameVersion(gameVersion), includeSupported, configured);
}

std::vector<std::shared_ptr<GameVersion>> GameVersionCollection::getGameVersionsCompatibleWith(const ModGameVersion & modGameVersion, bool includeSupported, std::optional<bool> configured) const {
	return getGameVersionsCompatibleWith(indexOfGameVersion(modGameVersion.getGameVersion()), includeSupported, configured);
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

bool GameVersionCollection::addGameVersion(const GameVersion & gameVersion) {
	if(!gameVersion.isValid() || hasGameVersion(gameVersion)) {
		return false;
	}

	m_gameVersions.push_back(std::make_shared<GameVersion>(gameVersion));

	notifyCollectionChanged();

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


size_t GameVersionCollection::addGameVersions(const std::vector<std::shared_ptr<GameVersion>> & gameVersions) {
	size_t numberOfGameVersionsAdded = 0;

	for(std::vector<std::shared_ptr<GameVersion>>::const_iterator i = gameVersions.begin(); i != gameVersions.end(); ++i) {
		if(*i == nullptr) {
			continue;
		}

		if(addGameVersion(**i)) {
			numberOfGameVersionsAdded++;
		}
	}

	return numberOfGameVersionsAdded;
}

bool GameVersionCollection::removeGameVersion(size_t index) {
	if(index >= m_gameVersions.size()) {
		return false;
	}

	m_gameVersions.erase(m_gameVersions.begin() + index);

	notifyCollectionChanged();

	return true;
}

bool GameVersionCollection::removeGameVersion(const GameVersion & gameVersion) {
	for(std::vector<std::shared_ptr<GameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), gameVersion.getName())) {
			m_gameVersions.erase(i);

			notifyCollectionChanged();

			return true;
		}
	}

	return false;
}

bool GameVersionCollection::removeGameVersion(const std::string & name) {
	if(name.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<GameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			m_gameVersions.erase(i);

			notifyCollectionChanged();

			return true;
		}
	}

	return false;
}

size_t GameVersionCollection::addMissingDefaultGameVersions() {
	size_t numberOfGameVersionsAdded = 0;

	for(std::vector<GameVersion>::const_iterator i = GameVersion::DEFAULT_GAME_VERSIONS.begin(); i != GameVersion::DEFAULT_GAME_VERSIONS.end(); ++i) {
		if(hasGameVersion(*i)) {
			continue;
		}

		spdlog::info("Adding missing default game version '{}'.", i->getName());

		addGameVersion(*i);

		numberOfGameVersionsAdded++;
	}

	return numberOfGameVersionsAdded;
}

void GameVersionCollection::setDefaultGameVersions() {
	clearGameVersions();

	addGameVersions(GameVersion::DEFAULT_GAME_VERSIONS);
}

void GameVersionCollection::clearGameVersions() {
	m_gameVersions.clear();

	notifyCollectionChanged();
}

size_t GameVersionCollection::checkForMissingExecutables() const {
	size_t numberOfMissingExecutables = 0;

	for(std::vector<std::shared_ptr<GameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		numberOfMissingExecutables += (*i)->checkForMissingExecutables();
	}

	return numberOfMissingExecutables++;
}

size_t GameVersionCollection::checkForMissingExecutables(const std::string & name) const {
	std::shared_ptr<GameVersion> gameVersion = getGameVersion(name);

	if(gameVersion == nullptr) {
		return 0;
	}

	return gameVersion->checkForMissingExecutables();
}

rapidjson::Document GameVersionCollection::toJSON() const {
	rapidjson::Document gameVersionsDocument(rapidjson::kObjectType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = gameVersionsDocument.GetAllocator();

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
		spdlog::error("Game version collection is missing '{}' property'.", JSON_GAME_VERSIONS_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & gameVersionsValue = gameVersionCollectionValue[JSON_GAME_VERSIONS_PROPERTY_NAME];

	if(!gameVersionsValue.IsArray()) {
		spdlog::error("Invalid game version collection '{}' type: '{}', expected 'array'.", Utilities::typeToString(gameVersionsValue.GetType()));
		return nullptr;
	}

	std::unique_ptr<GameVersionCollection> newGameVersionCollection = std::make_unique<GameVersionCollection>();

	if(gameVersionsValue.Empty()) {
		return newGameVersionCollection;
	}

	std::unique_ptr<GameVersion> newGameVersion;

	for(rapidjson::Value::ConstValueIterator i = gameVersionsValue.Begin(); i != gameVersionsValue.End(); ++i) {
		newGameVersion = GameVersion::parseFrom(*i);

		if(!GameVersion::isValid(newGameVersion.get())) {
			spdlog::error("Failed to parse game version #{}{}!", newGameVersionCollection->m_gameVersions.size() + 1, newGameVersionCollection->numberOfGameVersions() == 0 ? "" : fmt::format(" (after game version '{}')", newGameVersionCollection->getGameVersion(newGameVersionCollection->numberOfGameVersions() - 1)->getName()));
			return nullptr;
		}

		if(newGameVersionCollection->hasGameVersion(*newGameVersion.get())) {
			spdlog::error("Encountered duplicate game version #{}{}.", newGameVersionCollection->m_gameVersions.size() + 1, newGameVersionCollection->numberOfGameVersions() == 0 ? "" : fmt::format(" (after game version '{}')", newGameVersionCollection->getGameVersion(newGameVersionCollection->numberOfGameVersions() - 1)->getName()));
			return nullptr;
		}

		newGameVersionCollection->m_gameVersions.push_back(std::shared_ptr<GameVersion>(newGameVersion.release()));
	}

	newGameVersionCollection->notifyCollectionChanged();

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

	std::unique_ptr<GameVersionCollection> gameVersionCollection = parseFrom(gameVersionsValue);

	if(!GameVersionCollection::isValid(gameVersionCollection.get())) {
		spdlog::error("Failed to parse gameVersion collection from JSON file '{}'.", filePath);
		return false;
	}

	m_gameVersions = gameVersionCollection->m_gameVersions;

	notifyCollectionChanged();

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

void GameVersionCollection::notifyCollectionChanged() const {
	for(size_t i = 0; i < numberOfListeners(); i++) {
		getListener(i)->gameVersionCollectionUpdated();
	}
}

bool GameVersionCollection::isValid() const {
	for(std::vector<std::shared_ptr<GameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		if(!(*i)->isValid()) {
			return false;
		}

		for(std::vector<std::shared_ptr<GameVersion>>::const_iterator j = i + 1; j != m_gameVersions.end(); ++j) {
			if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), (*j)->getName()) ||
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
