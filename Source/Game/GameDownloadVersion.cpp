#include "GameDownloadVersion.h"

#include "GameDownload.h"
#include "GameDownloadFile.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

#include <array>
#include <string_view>

static constexpr const char * JSON_GAME_DOWNLOAD_VERSION_VERSION_PROPERTY_NAME = "version";
static constexpr const char * JSON_GAME_DOWNLOAD_VERSION_RELEASE_DATE_PROPERTY_NAME = "releaseDate";
static constexpr const char * JSON_GAME_DOWNLOAD_VERSION_FILES_PROPERTY_NAME = "files";
static const std::array<std::string_view, 3> JSON_GAME_DOWNLOAD_VERSION_PROPERTY_NAMES = {
	JSON_GAME_DOWNLOAD_VERSION_VERSION_PROPERTY_NAME,
	JSON_GAME_DOWNLOAD_VERSION_RELEASE_DATE_PROPERTY_NAME,
	JSON_GAME_DOWNLOAD_VERSION_FILES_PROPERTY_NAME
};

GameDownloadVersion::GameDownloadVersion(const std::string & version, const std::optional<Date> & releaseDate)
	: m_version(Utilities::trimString(version))
	, m_releaseDate(releaseDate)
	, m_parentGameDownload(nullptr) { }

GameDownloadVersion::GameDownloadVersion(GameDownloadVersion && v) noexcept
	: m_version(std::move(v.m_version))
	, m_releaseDate(v.m_releaseDate)
	, m_files(std::move(v.m_files))
	, m_parentGameDownload(nullptr) {
	updateParent();
}

GameDownloadVersion::GameDownloadVersion(const GameDownloadVersion & v)
	: m_version(v.m_version)
	, m_releaseDate(v.m_releaseDate)
	, m_parentGameDownload(nullptr) {
	for(std::vector<std::shared_ptr<GameDownloadFile>>::const_iterator i = v.m_files.begin(); i != v.m_files.end(); ++i) {
		m_files.push_back(std::make_shared<GameDownloadFile>(**i));
	}

	updateParent();
}

GameDownloadVersion & GameDownloadVersion::operator = (GameDownloadVersion && v) noexcept {
	if(this != &v) {
		m_version = std::move(v.m_version);
		m_releaseDate = v.m_releaseDate;
		m_files = std::move(v.m_files);

		updateParent();
	}

	return *this;
}

GameDownloadVersion & GameDownloadVersion::operator = (const GameDownloadVersion & v) {
	m_files.clear();

	m_version = v.m_version;
	m_releaseDate = v.m_releaseDate;

	for(std::vector<std::shared_ptr<GameDownloadFile>>::const_iterator i = v.m_files.begin(); i != v.m_files.end(); ++i) {
		m_files.push_back(std::make_shared<GameDownloadFile>(**i));
	}

	updateParent();

	return *this;
}

GameDownloadVersion::~GameDownloadVersion() {
	m_parentGameDownload = nullptr;
}

const std::string & GameDownloadVersion::getVersion() const {
	return m_version;
}

std::string GameDownloadVersion::getFullName() const {
	if(!GameDownload::isValid(m_parentGameDownload)) {
		return "";
	}

	return m_parentGameDownload->getName() + " (" + m_version + ")";
}

bool GameDownloadVersion::hasReleaseDate() const {
	return m_releaseDate.has_value();
}

std::optional<Date> GameDownloadVersion::getReleaseDate() const {
	return m_releaseDate;
}

const GameDownload * GameDownloadVersion::getParentGameDownload() const {
	return m_parentGameDownload;
}

void GameDownloadVersion::setParentGameDownload(const GameDownload * gameDownload) {
	m_parentGameDownload = gameDownload;
}

size_t GameDownloadVersion::numberOfFiles() const {
	return m_files.size();
}

bool GameDownloadVersion::hasFile(const GameDownloadFile & file) const {
	for(std::vector<std::shared_ptr<GameDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), file.getFileName())) {
			return true;
		}
	}

	return false;
}

bool GameDownloadVersion::hasFileWithName(const std::string & fileName) const {
	if(fileName.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<GameDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), fileName)) {
			return true;
		}
	}

	return false;
}

bool GameDownloadVersion::hasFileOfType(GameDownloadFile::Type type) const {
	for(std::vector<std::shared_ptr<GameDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if((*i)->getType() == type) {
			return true;
		}
	}

	return false;
}

size_t GameDownloadVersion::indexOfFile(const GameDownloadFile & file) const {
	for(size_t i = 0; i < m_files.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_files[i]->getFileName(), file.getFileName())) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t GameDownloadVersion::indexOfFileWithName(const std::string & fileName) const {
	if(fileName.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_files.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_files[i]->getFileName(), fileName)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t GameDownloadVersion::indexOfFileOfType(GameDownloadFile::Type type) const {
	for(size_t i = 0; i < m_files.size(); i++) {
		if(m_files[i]->getType() == type) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<GameDownloadFile> GameDownloadVersion::getFile(size_t index) const {
	if(index >= m_files.size()) {
		return nullptr;
	}

	return m_files[index];
}

std::shared_ptr<GameDownloadFile> GameDownloadVersion::getFileWithName(const std::string & fileName) const {
	if(fileName.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<GameDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), fileName)) {
			return *i;
		}
	}

	return nullptr;
}

std::shared_ptr<GameDownloadFile> GameDownloadVersion::getFileOfType(GameDownloadFile::Type type) const {
	for(std::vector<std::shared_ptr<GameDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if((*i)->getType() == type) {
			return *i;
		}
	}

	return nullptr;
}

std::shared_ptr<GameDownloadFile> GameDownloadVersion::findFirstMatchingFile(std::optional<GameDownloadFile::Type> type, std::optional<GameVersion::OperatingSystem> operatingSystem, std::optional<GameDownloadFile::ProcessorArchitecture> processorArchitecture) const {
	if(!type.has_value() && !operatingSystem.has_value() && !processorArchitecture.has_value()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<GameDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(type.has_value() && (*i)->getType() != type.value()) {
			continue;
		}

		if(operatingSystem.has_value() && (*i)->getOperatingSystem() != operatingSystem.value()) {
			continue;
		}

		if(processorArchitecture.has_value() && (*i)->getProcessorArchitecture() != processorArchitecture.value()) {
			continue;
		}

		return *i;
	}

	return nullptr;
}

std::shared_ptr<GameDownloadFile> GameDownloadVersion::findLastMatchingFile(std::optional<GameDownloadFile::Type> type, std::optional<GameVersion::OperatingSystem> operatingSystem, std::optional<GameDownloadFile::ProcessorArchitecture> processorArchitecture) const {
	if(!type.has_value() && !operatingSystem.has_value() && !processorArchitecture.has_value()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<GameDownloadFile>>::const_reverse_iterator i = m_files.rbegin(); i != m_files.rend(); ++i) {
		if(type.has_value() && (*i)->getType() != type.value()) {
			continue;
		}

		if(operatingSystem.has_value() && (*i)->getOperatingSystem() != operatingSystem.value()) {
			continue;
		}

		if(processorArchitecture.has_value() && (*i)->hasProcessorArchitecture() && (*i)->getProcessorArchitecture() != processorArchitecture.value()) {
			continue;
		}

		return *i;
	}

	return nullptr;
}

std::vector<std::shared_ptr<GameDownloadFile>> GameDownloadVersion::findAllMatchingFiles(std::optional<GameDownloadFile::Type> type, std::optional<GameVersion::OperatingSystem> operatingSystem, std::optional<GameDownloadFile::ProcessorArchitecture> processorArchitecture) const {
	if(!type.has_value() && !operatingSystem.has_value() && !processorArchitecture.has_value()) {
		return {};
	}

	std::vector<std::shared_ptr<GameDownloadFile>> matchingFiles;

	for(std::vector<std::shared_ptr<GameDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(type.has_value() && (*i)->getType() != type.value()) {
			continue;
		}

		if(operatingSystem.has_value() && (*i)->getOperatingSystem() != operatingSystem.value()) {
			continue;
		}

		if(processorArchitecture.has_value() && (*i)->getProcessorArchitecture() != processorArchitecture.value()) {
			continue;
		}

		matchingFiles.push_back(*i);
	}

	return matchingFiles;
}

const std::vector<std::shared_ptr<GameDownloadFile>> & GameDownloadVersion::getFiles() const {
	return m_files;
}

bool GameDownloadVersion::addFile(const GameDownloadFile & file) {
	if(!file.isValid() || hasFile(file)) {
		return false;
	}

	std::shared_ptr<GameDownloadFile> newFile = std::make_shared<GameDownloadFile>(file);
	newFile->setParentGameDownloadVersion(this);

	m_files.push_back(newFile);

	return true;
}

bool GameDownloadVersion::removeFile(size_t index) {
	if(index >= m_files.size()) {
		return false;
	}

	m_files[index]->setParentGameDownloadVersion(nullptr);
	m_files.erase(m_files.begin() + index);

	return true;
}

bool GameDownloadVersion::removeFile(const GameDownloadFile & file) {
	for(std::vector<std::shared_ptr<GameDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), file.getFileName())) {
			(*i)->setParentGameDownloadVersion(nullptr);
			m_files.erase(i);

			return true;
		}
	}

	return false;
}

bool GameDownloadVersion::removeFileWithName(const std::string & fileName) {
	if(fileName.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<GameDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), fileName)) {
			(*i)->setParentGameDownloadVersion(nullptr);
			m_files.erase(i);

			return true;
		}
	}

	return false;
}

bool GameDownloadVersion::removeFileOfType(GameDownloadFile::Type type) {
	for(std::vector<std::shared_ptr<GameDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if((*i)->getType() == type) {
			(*i)->setParentGameDownloadVersion(nullptr);
			m_files.erase(i);

			return true;
		}
	}

	return false;
}

void GameDownloadVersion::clearFiles() {
	m_files.clear();
}

void GameDownloadVersion::updateParent() {
	for(std::vector<std::shared_ptr<GameDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		(*i)->setParentGameDownloadVersion(this);
	}
}

rapidjson::Value GameDownloadVersion::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value gameDownloadVersionValue(rapidjson::kObjectType);

	rapidjson::Value versionValue(m_version.c_str(), allocator);
	gameDownloadVersionValue.AddMember(rapidjson::StringRef(JSON_GAME_DOWNLOAD_VERSION_VERSION_PROPERTY_NAME), versionValue, allocator);

	if(m_releaseDate.has_value()) {
		rapidjson::Value releaseDateValue(m_releaseDate->toString().c_str(), allocator);
		gameDownloadVersionValue.AddMember(rapidjson::StringRef(JSON_GAME_DOWNLOAD_VERSION_RELEASE_DATE_PROPERTY_NAME), releaseDateValue, allocator);
	}

	rapidjson::Value filesValue(rapidjson::kArrayType);
	filesValue.Reserve(m_files.size(), allocator);

	for(const std::shared_ptr<GameDownloadFile> & file : m_files) {
		filesValue.PushBack(file->toJSON(allocator), allocator);
	}

	gameDownloadVersionValue.AddMember(rapidjson::StringRef(JSON_GAME_DOWNLOAD_VERSION_FILES_PROPERTY_NAME), filesValue, allocator);

	return gameDownloadVersionValue;
}

std::unique_ptr<GameDownloadVersion> GameDownloadVersion::parseFrom(const rapidjson::Value & gameDownloadVersionValue) {
	if(!gameDownloadVersionValue.IsObject()) {
		spdlog::error("Invalid game download version type: '{}', expected 'object'.", Utilities::typeToString(gameDownloadVersionValue.GetType()));
		return nullptr;
	}

	// check for unhandled game download version properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = gameDownloadVersionValue.MemberBegin(); i != gameDownloadVersionValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_GAME_DOWNLOAD_VERSION_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::warn("Game download version has unexpected property '{}'.", i->name.GetString());
		}
	}

	// parse the version property
	if(!gameDownloadVersionValue.HasMember(JSON_GAME_DOWNLOAD_VERSION_VERSION_PROPERTY_NAME)) {
		spdlog::error("Game download version is missing '{}' property.", JSON_GAME_DOWNLOAD_VERSION_VERSION_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & versionValue = gameDownloadVersionValue[JSON_GAME_DOWNLOAD_VERSION_VERSION_PROPERTY_NAME];

	if(!versionValue.IsString()) {
		spdlog::error("Game download version '{}' property has invalid type: '{}', expected 'string'.", JSON_GAME_DOWNLOAD_VERSION_VERSION_PROPERTY_NAME, Utilities::typeToString(versionValue.GetType()));
		return nullptr;
	}

	std::string version(versionValue.GetString());

	// parse the release date property
	std::optional<Date> optionalReleaseDate;

	if(gameDownloadVersionValue.HasMember(JSON_GAME_DOWNLOAD_VERSION_RELEASE_DATE_PROPERTY_NAME)) {
		const rapidjson::Value & releaseDateValue = gameDownloadVersionValue[JSON_GAME_DOWNLOAD_VERSION_RELEASE_DATE_PROPERTY_NAME];

		if(!releaseDateValue.IsString()) {
			spdlog::error("Game download version '{}' property has invalid type: '{}', expected 'string'.", JSON_GAME_DOWNLOAD_VERSION_RELEASE_DATE_PROPERTY_NAME, Utilities::typeToString(releaseDateValue.GetType()));
			return nullptr;
		}

		optionalReleaseDate = Date::parseFrom(releaseDateValue.GetString());

		if(!optionalReleaseDate.has_value()) {
			spdlog::error("Game download version '{}' property has invalid value: '{}'.", JSON_GAME_DOWNLOAD_VERSION_RELEASE_DATE_PROPERTY_NAME, Utilities::valueToString(releaseDateValue));
			return nullptr;
		}
	}

	// initialize the game download version
	std::unique_ptr<GameDownloadVersion> newGameDownloadVersion(std::make_unique<GameDownloadVersion>(version, optionalReleaseDate));

	// parse the files property
	if(!gameDownloadVersionValue.HasMember(JSON_GAME_DOWNLOAD_VERSION_FILES_PROPERTY_NAME)) {
		spdlog::error("Game download version is missing '{}' property.", JSON_GAME_DOWNLOAD_VERSION_FILES_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & filesValue = gameDownloadVersionValue[JSON_GAME_DOWNLOAD_VERSION_FILES_PROPERTY_NAME];

	if(!filesValue.IsArray()) {
		spdlog::error("Game download version '{}' property has invalid type: '{}', expected 'array'.", JSON_GAME_DOWNLOAD_VERSION_FILES_PROPERTY_NAME, Utilities::typeToString(filesValue.GetType()));
		return nullptr;
	}

	std::shared_ptr<GameDownloadFile> newFile;

	for(rapidjson::Value::ConstValueIterator i = filesValue.Begin(); i != filesValue.End(); ++i) {
		newFile = std::shared_ptr<GameDownloadFile>(GameDownloadFile::parseFrom(*i).release());

		if(!GameDownloadFile::isValid(newFile.get())) {
			spdlog::error("Failed to parse mod file #{}.", newGameDownloadVersion->m_files.size() + 1);
			return nullptr;
		}

		newFile->setParentGameDownloadVersion(newGameDownloadVersion.get());

		if(newGameDownloadVersion->hasFile(*newFile.get())) {
			spdlog::error("Encountered duplicate mod file #{}.", newGameDownloadVersion->m_files.size() + 1);
			return nullptr;
		}

		newGameDownloadVersion->m_files.push_back(newFile);
	}

	return newGameDownloadVersion;
}

bool GameDownloadVersion::isValid() const {
	if(m_version.empty() ||
	   m_files.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<GameDownloadFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(!(*i)->isValid()) {
			return false;
		}

		if((*i)->getParentGameDownloadVersion() != this) {
			return false;
		}

		for(std::vector<std::shared_ptr<GameDownloadFile>>::const_iterator j = i + 1; j != m_files.end(); ++j) {
			if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), (*j)->getFileName())) {
				return false;
			}
		}
	}

	return true;
}

bool GameDownloadVersion::isValid(const GameDownloadVersion * v) {
	return v != nullptr && v->isValid();
}

bool GameDownloadVersion::operator == (const GameDownloadVersion & v) const {
	if(m_files.size() != v.m_files.size() ||
	   !Utilities::areStringsEqualIgnoreCase(m_version, v.m_version)) {
		return false;
	}

	for(size_t i = 0; i < m_files.size(); i++) {
		if(m_files[i] != v.m_files[i]) {
			return false;
		}
	}

	return true;
}

bool GameDownloadVersion::operator != (const GameDownloadVersion & v) const {
	return !operator == (v);
}
