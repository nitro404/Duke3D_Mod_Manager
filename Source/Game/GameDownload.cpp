#include "GameDownload.h"

#include "GameDownloadVersion.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

#include <array>
#include <string_view>

static constexpr const char * JSON_GAME_DOWNLOAD_NAME_PROPERTY_NAME = "name";
static constexpr const char * JSON_GAME_DOWNLOAD_VERSIONS_PROPERTY_NAME = "versions";
static const std::array<std::string_view, 2> JSON_GAME_DOWNLOAD_PROPERTY_NAMES = {
	JSON_GAME_DOWNLOAD_NAME_PROPERTY_NAME,
	JSON_GAME_DOWNLOAD_VERSIONS_PROPERTY_NAME
};

GameDownload::GameDownload(const std::string & name)
	: m_name(Utilities::trimString(name)) { }

GameDownload::GameDownload(GameDownload && v) noexcept
	: m_name(std::move(v.m_name))
	, m_versions(std::move(v.m_versions)) {
	updateParent();
}

GameDownload::GameDownload(const GameDownload & d)
	: m_name(std::move(d.m_name)) {
	for(std::vector<std::shared_ptr<GameDownloadVersion>>::const_iterator i = d.m_versions.begin(); i != d.m_versions.end(); ++i) {
		m_versions.push_back(std::make_shared<GameDownloadVersion>(**i));
	}

	updateParent();
}

GameDownload & GameDownload::operator = (GameDownload && d) noexcept {
	if(this != &d) {
		m_name = std::move(d.m_name);
		m_versions = std::move(d.m_versions);

		updateParent();
	}

	return *this;
}

GameDownload & GameDownload::operator = (const GameDownload & d) {
	m_versions.clear();

	m_name = d.m_name;

	for(std::vector<std::shared_ptr<GameDownloadVersion>>::const_iterator i = d.m_versions.begin(); i != d.m_versions.end(); ++i) {
		m_versions.push_back(std::make_shared<GameDownloadVersion>(**i));
	}

	updateParent();

	return *this;
}

GameDownload::~GameDownload() { }

const std::string & GameDownload::getName() const {
	return m_name;
}

size_t GameDownload::numberOfVersions() const {
	return m_versions.size();
}

bool GameDownload::hasVersion(const GameDownloadVersion & file) const {
	for(std::vector<std::shared_ptr<GameDownloadVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), file.getVersion())) {
			return true;
		}
	}

	return false;
}

bool GameDownload::hasVersion(const std::string & fileName) const {
	if(fileName.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<GameDownloadVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), fileName)) {
			return true;
		}
	}

	return false;
}

size_t GameDownload::indexOfVersion(const GameDownloadVersion & file) const {
	for(size_t i = 0; i < m_versions.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_versions[i]->getVersion(), file.getVersion())) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t GameDownload::indexOfVersion(const std::string & fileName) const {
	if(fileName.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_versions.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_versions[i]->getVersion(), fileName)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<GameDownloadVersion> GameDownload::getVersion(size_t index) const {
	if(index >= m_versions.size()) {
		return nullptr;
	}

	return m_versions[index];
}

std::shared_ptr<GameDownloadVersion> GameDownload::getVersion(const std::string & fileName) const {
	if(fileName.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<GameDownloadVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), fileName)) {
			return *i;
		}
	}

	return nullptr;
}


std::shared_ptr<GameDownloadFile> GameDownload::getLatestGameDownloadFile(GameDownloadFile::Type downloadType, DeviceInformationBridge::OperatingSystemType operatingSystemType, std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> optionalOperatingSystemArchitectureType) const {
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

	return getLatestGameDownloadFile(downloadType, optionalOperatingSystem.value(), optionalProcessorArchitecture);
}

std::shared_ptr<GameDownloadFile> GameDownload::getLatestGameDownloadFile(GameDownloadFile::Type downloadType, GameVersion::OperatingSystem operatingSystem, std::optional<GameDownloadFile::ProcessorArchitecture> optionalProcessorArchitecture) const {
	std::shared_ptr<GameDownloadFile> matchingDownloadFile;
	std::shared_ptr<GameDownloadFile> latestGameDownloadFile;
	std::optional<Date> latestReleaseDate;

	for(std::vector<std::shared_ptr<GameDownloadVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		matchingDownloadFile = (*i)->findFirstMatchingFile(downloadType, operatingSystem, optionalProcessorArchitecture);

		if(matchingDownloadFile == nullptr || ((*i)->hasReleaseDate() && latestReleaseDate.has_value() && (*i)->getReleaseDate().value() < latestReleaseDate.value())) {
			continue;
		}

		latestGameDownloadFile = matchingDownloadFile;
		latestReleaseDate = (*i)->getReleaseDate();
	}

	return latestGameDownloadFile;
}

const std::vector<std::shared_ptr<GameDownloadVersion>> & GameDownload::getVersions() const {
	return m_versions;
}

bool GameDownload::addVersion(const GameDownloadVersion & file) {
	if(!file.isValid() || hasVersion(file)) {
		return false;
	}

	std::shared_ptr<GameDownloadVersion> newGameDownloadVersion(std::make_shared<GameDownloadVersion>(file));
	newGameDownloadVersion->setParentGameDownload(this);

	m_versions.push_back(newGameDownloadVersion);

	return true;
}

bool GameDownload::removeVersion(size_t index) {
	if(index >= m_versions.size()) {
		return false;
	}

	m_versions[index]->setParentGameDownload(nullptr);
	m_versions.erase(m_versions.begin() + index);

	return true;
}

bool GameDownload::removeVersion(const GameDownloadVersion & file) {
	for(std::vector<std::shared_ptr<GameDownloadVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), file.getVersion())) {
			(*i)->setParentGameDownload(nullptr);
			m_versions.erase(i);

			return true;
		}
	}

	return false;
}

bool GameDownload::removeVersion(const std::string & fileName) {
	if(fileName.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<GameDownloadVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), fileName)) {
			(*i)->setParentGameDownload(nullptr);
			m_versions.erase(i);

			return true;
		}
	}

	return false;
}

void GameDownload::clearVersions() {
	m_versions.clear();
}

void GameDownload::updateParent() {
	for(std::vector<std::shared_ptr<GameDownloadVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		(*i)->setParentGameDownload(this);
	}
}

rapidjson::Value GameDownload::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value gameDownloadVersionValue(rapidjson::kObjectType);

	rapidjson::Value nameValue(m_name.c_str(), allocator);
	gameDownloadVersionValue.AddMember(rapidjson::StringRef(JSON_GAME_DOWNLOAD_NAME_PROPERTY_NAME), nameValue, allocator);

	rapidjson::Value versionsValue(rapidjson::kArrayType);

	for(std::vector<std::shared_ptr<GameDownloadVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		versionsValue.PushBack((*i)->toJSON(allocator), allocator);
	}

	gameDownloadVersionValue.AddMember(rapidjson::StringRef(JSON_GAME_DOWNLOAD_VERSIONS_PROPERTY_NAME), versionsValue, allocator);

	return gameDownloadVersionValue;
}

std::unique_ptr<GameDownload> GameDownload::parseFrom(const rapidjson::Value & gameDownloadVersionValue) {
	if(!gameDownloadVersionValue.IsObject()) {
		spdlog::error("Invalid game download type: '{}', expected 'object'.", Utilities::typeToString(gameDownloadVersionValue.GetType()));
		return nullptr;
	}

	// check for unhandled game download properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = gameDownloadVersionValue.MemberBegin(); i != gameDownloadVersionValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_GAME_DOWNLOAD_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::warn("Game download has unexpected property '{}'.", i->name.GetString());
		}
	}

	// parse the version property
	if(!gameDownloadVersionValue.HasMember(JSON_GAME_DOWNLOAD_NAME_PROPERTY_NAME)) {
		spdlog::error("Game download is missing '{}' property'.", JSON_GAME_DOWNLOAD_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & nameValue = gameDownloadVersionValue[JSON_GAME_DOWNLOAD_NAME_PROPERTY_NAME];

	if(!nameValue.IsString()) {
		spdlog::error("Game download '{}' property has invalid type: '{}', expected 'string'.", JSON_GAME_DOWNLOAD_NAME_PROPERTY_NAME, Utilities::typeToString(nameValue.GetType()));
		return nullptr;
	}

	// initialize the name property
	std::unique_ptr<GameDownload> newGameDownload(std::make_unique<GameDownload>(nameValue.GetString()));

	// parse the versions property
	if(!gameDownloadVersionValue.HasMember(JSON_GAME_DOWNLOAD_VERSIONS_PROPERTY_NAME)) {
		spdlog::error("Game download is missing '{}' property'.", JSON_GAME_DOWNLOAD_VERSIONS_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & versionsValue = gameDownloadVersionValue[JSON_GAME_DOWNLOAD_VERSIONS_PROPERTY_NAME];

	if(!versionsValue.IsArray()) {
		spdlog::error("Game download '{}' property has invalid type: '{}', expected 'array'.", JSON_GAME_DOWNLOAD_VERSIONS_PROPERTY_NAME, Utilities::typeToString(versionsValue.GetType()));
		return nullptr;
	}

	std::shared_ptr<GameDownloadVersion> newVersion;

	for(rapidjson::Value::ConstValueIterator i = versionsValue.Begin(); i != versionsValue.End(); ++i) {
		newVersion = std::shared_ptr<GameDownloadVersion>(GameDownloadVersion::parseFrom(*i).release());

		if(!GameDownloadVersion::isValid(newVersion.get())) {
			spdlog::error("Failed to parse mod file #{}.", newGameDownload->m_versions.size() + 1);
			return nullptr;
		}

		newVersion->setParentGameDownload(newGameDownload.get());

		if(newGameDownload->hasVersion(*newVersion.get())) {
			spdlog::error("Encountered duplicate mod file #{}.", newGameDownload->m_versions.size() + 1);
			return nullptr;
		}

		newGameDownload->m_versions.push_back(newVersion);
	}

	return newGameDownload;
}

bool GameDownload::isValid() const {
	if(m_name.empty() ||
	   m_versions.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<GameDownloadVersion>>::const_iterator i = m_versions.begin(); i != m_versions.end(); ++i) {
		if(!(*i)->isValid()) {
			return false;
		}

		if((*i)->getParentGameDownload() != this) {
			return false;
		}

		for(std::vector<std::shared_ptr<GameDownloadVersion>>::const_iterator j = i + 1; j != m_versions.end(); ++j) {
			if(Utilities::areStringsEqualIgnoreCase((*i)->getVersion(), (*j)->getVersion())) {
				return false;
			}
		}
	}

	return true;
}

bool GameDownload::isValid(const GameDownload * d) {
	return d != nullptr && d->isValid();
}

bool GameDownload::operator == (const GameDownload & v) const {
	if(m_versions.size() != v.m_versions.size() ||
	   !Utilities::areStringsEqualIgnoreCase(m_name, v.m_name)) {
		return false;
	}

	for(size_t i = 0; i < m_versions.size(); i++) {
		if(m_versions[i] != v.m_versions[i]) {
			return false;
		}
	}

	return true;
}

bool GameDownload::operator != (const GameDownload & d) const {
	return !operator == (d);
}
