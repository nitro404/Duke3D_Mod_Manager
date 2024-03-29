#include "GameDownloadCollection.h"

#include "GameDownload.h"

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

static constexpr const char * JSON_FILE_TYPE_PROPERTY_NAME = "fileType";
static constexpr const char * JSON_FILE_FORMAT_VERSION_PROPERTY_NAME = "fileFormatVersion";
static constexpr const char * JSON_GAME_DOWNLOADS_PROPERTY_NAME = "gameDownloads";
static constexpr const char * JSON_FILE_REVISION_PROPERTY_NAME = "fileRevision";

const std::string GameDownloadCollection::FILE_TYPE = "Game Downloads";
const uint32_t GameDownloadCollection::FILE_FORMAT_VERSION = 1;

GameDownloadCollection::GameDownloadCollection(uint32_t fileRevision)
	: m_fileRevision(fileRevision) { }

GameDownloadCollection::GameDownloadCollection(GameDownloadCollection && c) noexcept
	: m_fileRevision(c.m_fileRevision)
	, m_downloads(std::move(c.m_downloads)) { }

GameDownloadCollection::GameDownloadCollection(const GameDownloadCollection & c)
	: m_fileRevision(c.m_fileRevision) {
	for(std::vector<std::shared_ptr<GameDownload>>::const_iterator i = c.m_downloads.begin(); i != c.m_downloads.end(); ++i) {
		m_downloads.push_back(std::make_shared<GameDownload>(**i));
	}
}

GameDownloadCollection & GameDownloadCollection::operator = (GameDownloadCollection && c) noexcept {
	if(this != &c) {
		m_fileRevision = c.m_fileRevision;
		m_downloads = std::move(c.m_downloads);
	}

	return *this;
}

GameDownloadCollection & GameDownloadCollection::operator = (const GameDownloadCollection & c) {
	m_downloads.clear();

	m_fileRevision = c.m_fileRevision;

	for(std::vector<std::shared_ptr<GameDownload>>::const_iterator i = c.m_downloads.begin(); i != c.m_downloads.end(); ++i) {
		m_downloads.push_back(std::make_shared<GameDownload>(**i));
	}

	return *this;
}

GameDownloadCollection::~GameDownloadCollection() = default;

uint32_t GameDownloadCollection::getFileRevision() const {
	return m_fileRevision;
}

size_t GameDownloadCollection::numberOfDownloads() const {
	return m_downloads.size();
}

bool GameDownloadCollection::hasDownload(const GameDownload & download) const {
	for(std::vector<std::shared_ptr<GameDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getID(), download.getID())) {
			return true;
		}
	}

	return false;
}

bool GameDownloadCollection::hasDownloadWithID(const std::string & gameVersionID) const {
	if(gameVersionID.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<GameDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getID(), gameVersionID)) {
			return true;
		}
	}

	return false;
}

size_t GameDownloadCollection::indexOfDownload(const GameDownload & download) const {
	for(size_t i = 0; i < m_downloads.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_downloads[i]->getID(), download.getID())) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t GameDownloadCollection::indexOfDownloadWithID(const std::string & gameVersionID) const {
	if(gameVersionID.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_downloads.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_downloads[i]->getID(), gameVersionID)) {
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

std::shared_ptr<GameDownload> GameDownloadCollection::getDownloadWithID(const std::string & gameVersionID) const {
	if(gameVersionID.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<GameDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getID(), gameVersionID)) {
			return *i;
		}
	}

	return nullptr;
}

std::shared_ptr<GameDownloadFile> GameDownloadCollection::getLatestGameDownloadFile(const std::string & gameVersionID, GameDownloadFile::Type downloadType, DeviceInformationBridge::OperatingSystemType operatingSystemType, std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> optionalOperatingSystemArchitectureType) const {
	if(gameVersionID.empty()) {
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

	return getLatestGameDownloadFile(gameVersionID, downloadType, optionalOperatingSystem.value(), optionalProcessorArchitecture);
}

std::shared_ptr<GameDownloadFile> GameDownloadCollection::getLatestGameDownloadFile(const std::string & gameVersionID, GameDownloadFile::Type downloadType, GameVersion::OperatingSystem operatingSystem, std::optional<GameDownloadFile::ProcessorArchitecture> optionalProcessorArchitecture) const {
	if(gameVersionID.empty()) {
		return nullptr;
	}

	std::shared_ptr<GameDownload> gameDownload(getDownloadWithID(gameVersionID));

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

	updated(*this);

	return true;
}

bool GameDownloadCollection::removeDownload(size_t index) {
	if(index >= m_downloads.size()) {
		return false;
	}

	m_downloads.erase(m_downloads.begin() + index);

	updated(*this);

	return true;
}

bool GameDownloadCollection::removeDownload(const GameDownload & download) {
	for(std::vector<std::shared_ptr<GameDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getID(), download.getID())) {
			m_downloads.erase(i);

			updated(*this);

			return true;
		}
	}

	return false;
}

bool GameDownloadCollection::removeDownloadWithID(const std::string & gameVersionID) {
	if(gameVersionID.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<GameDownload>>::const_iterator i = m_downloads.begin(); i != m_downloads.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getID(), gameVersionID)) {
			m_downloads.erase(i);

			updated(*this);

			return true;
		}
	}

	return false;
}

void GameDownloadCollection::clearDownloads() {
	m_downloads.clear();

	updated(*this);
}

rapidjson::Document GameDownloadCollection::toJSON() const {
	rapidjson::Document gameDownloadCollectionValue(rapidjson::kObjectType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = gameDownloadCollectionValue.GetAllocator();

	rapidjson::Value fileTypeValue(FILE_TYPE.c_str(), allocator);
	gameDownloadCollectionValue.AddMember(rapidjson::StringRef(JSON_FILE_TYPE_PROPERTY_NAME), fileTypeValue, allocator);

	gameDownloadCollectionValue.AddMember(rapidjson::StringRef(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME), rapidjson::Value(FILE_FORMAT_VERSION), allocator);

	gameDownloadCollectionValue.AddMember(rapidjson::StringRef(JSON_FILE_REVISION_PROPERTY_NAME), rapidjson::Value(m_fileRevision), allocator);

	rapidjson::Value gameDownloadsValue(rapidjson::kArrayType);
	gameDownloadsValue.Reserve(m_downloads.size(), allocator);

	for(const std::shared_ptr<GameDownload> & download : m_downloads) {
		gameDownloadsValue.PushBack(download->toJSON(allocator), allocator);
	}

	gameDownloadCollectionValue.AddMember(rapidjson::StringRef(JSON_GAME_DOWNLOADS_PROPERTY_NAME), gameDownloadsValue, allocator);

	return gameDownloadCollectionValue;
}

std::unique_ptr<GameDownloadCollection> GameDownloadCollection::parseFrom(const rapidjson::Value & gameDownloadCollectionValue) {
	if(!gameDownloadCollectionValue.IsObject()) {
		spdlog::error("Invalid game download collection type: '{}', expected 'object'.", Utilities::typeToString(gameDownloadCollectionValue.GetType()));
		return nullptr;
	}

	if(gameDownloadCollectionValue.HasMember(JSON_FILE_TYPE_PROPERTY_NAME)) {
		const rapidjson::Value & fileTypeValue = gameDownloadCollectionValue[JSON_FILE_TYPE_PROPERTY_NAME];

		if(!fileTypeValue.IsString()) {
			spdlog::error("Invalid game download collection file type type: '{}', expected: 'string'.", Utilities::typeToString(fileTypeValue.GetType()));
			return nullptr;
		}

		if(!Utilities::areStringsEqualIgnoreCase(fileTypeValue.GetString(), FILE_TYPE)) {
			spdlog::error("Incorrect game download collection file type: '{}', expected: '{}'.", fileTypeValue.GetString(), FILE_TYPE);
			return nullptr;
		}
	}
	else {
		spdlog::warn("Game download collection JSON data is missing file type, and may fail to load correctly!");
	}

	if(gameDownloadCollectionValue.HasMember(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & fileFormatVersionValue = gameDownloadCollectionValue[JSON_FILE_FORMAT_VERSION_PROPERTY_NAME];

		if(!fileFormatVersionValue.IsUint()) {
			spdlog::error("Invalid game download collection file format version type: '{}', expected unsigned integer 'number'.", Utilities::typeToString(fileFormatVersionValue.GetType()));
			return nullptr;
		}

		if(fileFormatVersionValue.GetUint() != FILE_FORMAT_VERSION) {
			spdlog::error("Unsupported game download collection file format version: {}, only version {} is supported.", fileFormatVersionValue.GetUint(), FILE_FORMAT_VERSION);
			return nullptr;
		}
	}
	else {
		spdlog::warn("Game download collection JSON data is missing file format version, and may fail to load correctly!");
	}

	uint32_t fileRevision = 1;

	if(gameDownloadCollectionValue.HasMember(JSON_FILE_REVISION_PROPERTY_NAME)) {
		const rapidjson::Value & fileRevisionValue = gameDownloadCollectionValue[JSON_FILE_REVISION_PROPERTY_NAME];

		if(!fileRevisionValue.IsUint()) {
			spdlog::error("Invalid game download collection file revision type: '{}', expected unsigned integer 'number'.", Utilities::typeToString(fileRevisionValue.GetType()));
			return nullptr;
		}

		fileRevision = fileRevisionValue.GetUint();
	}
	else {
		spdlog::warn("Game download collection JSON data is missing file revision!");
	}


	if(!gameDownloadCollectionValue.HasMember(JSON_GAME_DOWNLOADS_PROPERTY_NAME)) {
		spdlog::error("Game download collection is missing '{}' property.", JSON_GAME_DOWNLOADS_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & gameDownloadsValue = gameDownloadCollectionValue[JSON_GAME_DOWNLOADS_PROPERTY_NAME];

	if(!gameDownloadsValue.IsArray()) {
		spdlog::error("Invalid game download collection games list type: '{}', expected 'array'.", Utilities::typeToString(gameDownloadsValue.GetType()));
		return nullptr;
	}

	std::unique_ptr<GameDownloadCollection> newGameDownloadCollection(std::make_unique<GameDownloadCollection>(fileRevision));

	if(gameDownloadsValue.Empty()) {
		return newGameDownloadCollection;
	}

	std::unique_ptr<GameDownload> newDownload;

	for(rapidjson::Value::ConstValueIterator i = gameDownloadsValue.Begin(); i != gameDownloadsValue.End(); ++i) {
		newDownload = GameDownload::parseFrom(*i);

		if(!GameDownload::isValid(newDownload.get())) {
			spdlog::error("Failed to parse game download #{}{}!", newGameDownloadCollection->m_downloads.size() + 1, newGameDownloadCollection->numberOfDownloads() == 0 ? "" : fmt::format(" (after game download with ID '{}')", newGameDownloadCollection->getDownload(newGameDownloadCollection->numberOfDownloads() - 1)->getID()));
			return nullptr;
		}

		if(newGameDownloadCollection->hasDownload(*newDownload)) {
			spdlog::warn("Encountered duplicate game download #{}{}.", newGameDownloadCollection->m_downloads.size() + 1, newGameDownloadCollection->numberOfDownloads() == 0 ? "" : fmt::format(" (after game download with ID '{}')", newGameDownloadCollection->getDownload(newGameDownloadCollection->numberOfDownloads() - 1)->getID()));
		}

		newGameDownloadCollection->m_downloads.emplace_back(std::move(newDownload));
	}

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

	updated(*this);

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
	if(m_fileRevision != c.m_fileRevision ||
	   m_downloads.size() != c.m_downloads.size()) {
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
