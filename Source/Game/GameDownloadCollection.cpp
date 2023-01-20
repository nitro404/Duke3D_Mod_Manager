#include "GameDownloadCollection.h"

#include "GameDownload.h"
#include "GameDownloadCollectionListener.h"

#include <Date.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/Utilities.h>

#include <fmt/core.h>
#include <magic_enum.hpp>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>

static constexpr const char * JSON_FILE_FORMAT_VERSION_PROPERTY_NAME = "fileFormatVersion";
static constexpr const char * JSON_GAME_DOWNLOADS_PROPERTY_NAME = "gameDownloads";

const std::string GameDownloadCollection::FILE_FORMAT_VERSION = "1.0.0";

GameDownloadCollection::GameDownloadCollection()
	: GameDownloadCollectionBroadcaster() { }

GameDownloadCollection::GameDownloadCollection(GameDownloadCollection && c) noexcept
	: GameDownloadCollectionBroadcaster(std::move(c))
	, m_downloads(std::move(c.m_downloads)) { }

GameDownloadCollection::GameDownloadCollection(const GameDownloadCollection & c)
	: GameDownloadCollectionBroadcaster(c) {
	for(std::vector<std::shared_ptr<GameDownload>>::const_iterator i = c.m_downloads.begin(); i != c.m_downloads.end(); ++i) {
		m_downloads.push_back(std::make_shared<GameDownload>(**i));
	}
}

GameDownloadCollection & GameDownloadCollection::operator = (GameDownloadCollection && c) noexcept {
	if(this != &c) {
		GameDownloadCollectionBroadcaster::operator = (c);

		m_downloads = std::move(c.m_downloads);
	}

	return *this;
}

GameDownloadCollection & GameDownloadCollection::operator = (const GameDownloadCollection & c) {
	GameDownloadCollectionBroadcaster::operator = (c);

	m_downloads.clear();

	for(std::vector<std::shared_ptr<GameDownload>>::const_iterator i = c.m_downloads.begin(); i != c.m_downloads.end(); ++i) {
		m_downloads.push_back(std::make_shared<GameDownload>(**i));
	}

	return *this;
}

GameDownloadCollection::~GameDownloadCollection() { }

size_t GameDownloadCollection::numberOfDownloads() const {
	return m_downloads.size();
}

bool GameDownloadCollection::hasDownload(const GameDownload & mod) const {
	for(std::vector<std::shared_ptr<GameDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), mod.getName())) {
			return true;
		}
	}

	return false;
}

bool GameDownloadCollection::hasDownloadWithName(const std::string & name) const {
	if(name.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<GameDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			return true;
		}
	}

	return false;
}

size_t GameDownloadCollection::indexOfDownload(const GameDownload & download) const {
	for(size_t i = 0; i < m_downloads.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_downloads[i]->getName(), download.getName())) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t GameDownloadCollection::indexOfDownloadWithName(const std::string & name) const {
	if(name.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_downloads.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_downloads[i]->getName(), name)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<GameDownload> GameDownloadCollection::getDownload(size_t index) const {
	if(index >= m_downloads.size()) {
		return nullptr;
	}

	return m_downloads[index];
}

std::shared_ptr<GameDownload> GameDownloadCollection::getDownloadWithName(const std::string & name) const {
	if(name.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<GameDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			return *i;
		}
	}

	return nullptr;
}

std::shared_ptr<GameDownloadFile> GameDownloadCollection::getLatestGameDownloadFile(const std::string & gameName, GameDownloadFile::Type downloadType, DeviceInformationBridge::OperatingSystemType operatingSystemType, std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> optionalOperatingSystemArchitectureType) const {
	if(gameName.empty()) {
		return nullptr;
	}

	std::optional<GameVersion::OperatingSystem> optionalOperatingSystem(GameVersion::convertOperatingSystemType(operatingSystemType));

	if(!optionalOperatingSystem.has_value()) {
		return nullptr;
	}

	std::optional<GameDownloadFile::ProcessorArchitecture> optionalProcessorArchitecture;

	if(optionalOperatingSystemArchitectureType.has_value()) {
		optionalProcessorArchitecture = GameDownloadFile::convertOperatingSystemArchitectureType(optionalOperatingSystemArchitectureType.value());

		if(!optionalProcessorArchitecture.has_value()) {
			return nullptr;
		}
	}

	return getLatestGameDownloadFile(gameName, downloadType, optionalOperatingSystem.value(), optionalProcessorArchitecture);
}

std::shared_ptr<GameDownloadFile> GameDownloadCollection::getLatestGameDownloadFile(const std::string & gameName, GameDownloadFile::Type downloadType, GameVersion::OperatingSystem operatingSystem, std::optional<GameDownloadFile::ProcessorArchitecture> optionalProcessorArchitecture) const {
	if(gameName.empty()) {
		return nullptr;
	}

	std::shared_ptr<GameDownload> gameDownload(getDownloadWithName(gameName));

	if(gameDownload == nullptr) {
		return nullptr;
	}

	return gameDownload->getLatestGameDownloadFile(downloadType, operatingSystem, optionalProcessorArchitecture);
}

const std::vector<std::shared_ptr<GameDownload>> & GameDownloadCollection::getDownloads() const {
	return m_downloads;
}

bool GameDownloadCollection::addDownload(const GameDownload & download) {
	if(!download.isValid() || hasDownload(download)) {
		return false;
	}

	m_downloads.push_back(std::make_shared<GameDownload>(download));

	notifyCollectionChanged();

	return true;
}

bool GameDownloadCollection::removeDownload(size_t index) {
	if(index >= m_downloads.size()) {
		return false;
	}

	m_downloads.erase(m_downloads.begin() + index);

	notifyCollectionChanged();

	return true;
}

bool GameDownloadCollection::removeDownload(const GameDownload & download) {
	for(std::vector<std::shared_ptr<GameDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), download.getName())) {
			m_downloads.erase(i);

			notifyCollectionChanged();

			return true;
		}
	}

	return false;
}

bool GameDownloadCollection::removeDownloadWithName(const std::string & name) {
	if(name.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<GameDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			m_downloads.erase(i);

			notifyCollectionChanged();

			return true;
		}
	}

	return false;
}

void GameDownloadCollection::clearDownloads() {
	m_downloads.clear();

	notifyCollectionChanged();
}

rapidjson::Document GameDownloadCollection::toJSON() const {
	rapidjson::Document gameDownloadCollectionValue(rapidjson::kObjectType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = gameDownloadCollectionValue.GetAllocator();

	rapidjson::Value fileFormatVersionValue(FILE_FORMAT_VERSION.c_str(), allocator);
	gameDownloadCollectionValue.AddMember(rapidjson::StringRef(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME), fileFormatVersionValue, allocator);

	rapidjson::Value gameDownloadsValue(rapidjson::kArrayType);

	for(std::vector<std::shared_ptr<GameDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		gameDownloadsValue.PushBack((*i)->toJSON(allocator), allocator);
	}

	gameDownloadCollectionValue.AddMember(rapidjson::StringRef(JSON_GAME_DOWNLOADS_PROPERTY_NAME), gameDownloadsValue, allocator);

	return gameDownloadCollectionValue;
}

std::unique_ptr<GameDownloadCollection> GameDownloadCollection::parseFrom(const rapidjson::Value & modCollectionValue) {
	if(!modCollectionValue.IsObject()) {
		spdlog::error("Invalid game download collection type: '{}', expected 'object'.", Utilities::typeToString(modCollectionValue.GetType()));
		return nullptr;
	}

	if(modCollectionValue.HasMember(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & fileFormatVersionValue = modCollectionValue[JSON_FILE_FORMAT_VERSION_PROPERTY_NAME];

		if(!fileFormatVersionValue.IsString()) {
			spdlog::error("Invalid game download collection file format version type: '{}', expected: 'string'.", Utilities::typeToString(fileFormatVersionValue.GetType()));
			return false;
		}

		std::optional<std::uint8_t> optionalVersionComparison(Utilities::compareVersions(fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION));

		if(!optionalVersionComparison.has_value()) {
			spdlog::error("Invalid game download collection file format version: '{}'.", fileFormatVersionValue.GetString());
			return false;
		}

		if(*optionalVersionComparison != 0) {
			spdlog::error("Unsupported game download collection file format version: '{}', only version '{}' is supported.", fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION);
			return false;
		}
	}
	else {
		spdlog::warn("Game download collection JSON data is missing file format version, and may fail to load correctly!");
	}

	if(!modCollectionValue.HasMember(JSON_GAME_DOWNLOADS_PROPERTY_NAME)) {
		spdlog::error("Game download collection is missing '{}' property'.", JSON_GAME_DOWNLOADS_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & gameDownloadsValue = modCollectionValue[JSON_GAME_DOWNLOADS_PROPERTY_NAME];

	if(!gameDownloadsValue.IsArray()) {
		spdlog::error("Invalid game download collection games list type: '{}', expected 'array'.", Utilities::typeToString(gameDownloadsValue.GetType()));
		return nullptr;
	}

	std::unique_ptr<GameDownloadCollection> newGameDownloadCollection(std::make_unique<GameDownloadCollection>());

	if(gameDownloadsValue.Empty()) {
		return newGameDownloadCollection;
	}

	std::unique_ptr<GameDownload> newDownload;

	for(rapidjson::Value::ConstValueIterator i = gameDownloadsValue.Begin(); i != gameDownloadsValue.End(); ++i) {
		newDownload = GameDownload::parseFrom(*i);

		if(!GameDownload::isValid(newDownload.get())) {
			spdlog::error("Failed to parse game download #{}{}!", newGameDownloadCollection->m_downloads.size() + 1, newGameDownloadCollection->numberOfDownloads() == 0 ? "" : fmt::format(" (after game download with ID '{}')", newGameDownloadCollection->getDownload(newGameDownloadCollection->numberOfDownloads() - 1)->getName()));
			return nullptr;
		}

		if(newGameDownloadCollection->hasDownload(*newDownload.get())) {
			spdlog::warn("Encountered duplicate game download #{}{}.", newGameDownloadCollection->m_downloads.size() + 1, newGameDownloadCollection->numberOfDownloads() == 0 ? "" : fmt::format(" (after game download with ID '{}')", newGameDownloadCollection->getDownload(newGameDownloadCollection->numberOfDownloads() - 1)->getName()));
		}

		newGameDownloadCollection->m_downloads.push_back(std::shared_ptr<GameDownload>(newDownload.release()));
	}

	newGameDownloadCollection->notifyCollectionChanged();

	return newGameDownloadCollection;
}

bool GameDownloadCollection::loadFrom(const std::string & filePath) {
	if(filePath.empty()) {
		return false;
	}

	std::string fileExtension(Utilities::getFileExtension(filePath));

	if(fileExtension.empty()) {
		return false;
	}
	else if(Utilities::areStringsEqualIgnoreCase(fileExtension, "json")) {
		return loadFromJSON(filePath);
	}

	return false;
}

bool GameDownloadCollection::loadFromJSON(const std::string & filePath) {
	if(filePath.empty() || !std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		return false;
	}

	std::ifstream fileStream(filePath);

	if(!fileStream.is_open()) {
		return false;
	}

	rapidjson::Document modsValue;
	rapidjson::IStreamWrapper fileStreamWrapper(fileStream);
	if(modsValue.ParseStream(fileStreamWrapper).HasParseError()) {
		return false;
	}

	fileStream.close();

	std::unique_ptr<GameDownloadCollection> modCollection(parseFrom(modsValue));

	if(!GameDownloadCollection::isValid(modCollection.get())) {
		spdlog::error("Failed to parse mod collection from JSON file '{}'.", filePath);
		return false;
	}

	m_downloads = std::move(modCollection->m_downloads);

	notifyCollectionChanged();

	return true;
}

bool GameDownloadCollection::saveTo(const std::string & filePath, bool overwrite) const {
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

bool GameDownloadCollection::saveToJSON(const std::string & filePath, bool overwrite) const {
	if(!overwrite && std::filesystem::exists(std::filesystem::path(filePath))) {
		spdlog::warn("File '{}' already exists, use overwrite to force write.", filePath);
		return false;
	}

	std::ofstream fileStream(filePath);

	if(!fileStream.is_open()) {
		return false;
	}

	rapidjson::Document mods(toJSON());

	rapidjson::OStreamWrapper fileStreamWrapper(fileStream);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> fileStreamWriter(fileStreamWrapper);
	fileStreamWriter.SetIndent('\t', 1);
	mods.Accept(fileStreamWriter);
	fileStream.close();

	return true;
}

void GameDownloadCollection::notifyCollectionChanged() {
	for(size_t i = 0; i < numberOfListeners(); i++) {
		getListener(i)->gameDownloadCollectionUpdated(*this);
	}
}

bool GameDownloadCollection::isValid() const {
	for(std::vector<std::shared_ptr<GameDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(!(*i)->isValid()) {
			return false;
		}
	}

	return true;
}

bool GameDownloadCollection::isValid(const GameDownloadCollection * c) {
	return c != nullptr && c->isValid();
}

bool GameDownloadCollection::operator == (const GameDownloadCollection & c) const {
	if(m_downloads.size() != c.m_downloads.size()) {
		return false;
	}

	for(size_t i = 0; i < c.m_downloads.size(); i++) {
		if(*m_downloads[i] != *c.m_downloads[i]) {
			return false;
		}
	}

	return true;
}

bool GameDownloadCollection::operator != (const GameDownloadCollection & c) const {
	return !operator == (c);
}
