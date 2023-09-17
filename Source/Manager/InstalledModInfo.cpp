#include "InstalledModInfo.h"

#include "Game/GameVersion.h"
#include "Mod/Mod.h"
#include "Mod/ModVersion.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/TimeUtilities.h>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>

const std::string InstalledModInfo::FILE_TYPE("Installed Mod Information");
const std::string InstalledModInfo::FILE_FORMAT_VERSION("1.0.0");
const std::string InstalledModInfo::DEFAULT_FILE_NAME(".duke3d_mod.json");

static const std::string JSON_FILE_TYPE_PROPERTY_NAME("fileType");
static const std::string JSON_FILE_FORMAT_VERSION_PROPERTY_NAME("fileFormatVersion");
static const std::string JSON_MOD_INFO_CATEGORY_PROPERTY_NAME("mod");
static const std::string JSON_MOD_ID_PROPERTY_NAME("id");
static const std::string JSON_MOD_NAME_PROPERTY_NAME("name");
static const std::string JSON_MOD_VERSION_PROPERTY_NAME("version");
static const std::string JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME("installedTimestamp");
static const std::string JSON_ORIGINAL_FILES_LIST_PROPERTY_NAME("originalFiles");
static const std::string JSON_MOD_FILES_LIST_PROPERTY_NAME("modFiles");

static const std::array<std::string, 6> JSON_INSTALLED_MOD_INFO_PROPERTY_NAMES = {
	JSON_FILE_TYPE_PROPERTY_NAME,
	JSON_FILE_FORMAT_VERSION_PROPERTY_NAME,
	JSON_MOD_INFO_CATEGORY_PROPERTY_NAME,
	JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME,
	JSON_ORIGINAL_FILES_LIST_PROPERTY_NAME,
	JSON_MOD_FILES_LIST_PROPERTY_NAME
};

InstalledModInfo::InstalledModInfo(const ModVersion * modVersion, const std::vector<std::string> & originalFiles, const std::vector<std::string> & modFiles)
	: m_modID(modVersion != nullptr ? modVersion->getParentMod()->getID() : "")
	, m_modName(modVersion != nullptr ? modVersion->getParentMod()->getName() : "")
	, m_modVersion(modVersion != nullptr ? modVersion->getVersion() : "")
	, m_installedTimestamp(std::chrono::system_clock::now())
	, m_originalFiles(originalFiles)
	, m_modFiles(modFiles) { }

InstalledModInfo::InstalledModInfo(const std::string & modID, const std::string & modName, const std::string & modVersion, std::chrono::time_point<std::chrono::system_clock> installedTimestamp, const std::vector<std::string> & originalFiles, const std::vector<std::string> & modFiles)
	: m_modID(modID)
	, m_modName(modName)
	, m_modVersion(modVersion)
	, m_installedTimestamp(installedTimestamp)
	, m_originalFiles(originalFiles)
	, m_modFiles(modFiles) { }

InstalledModInfo::InstalledModInfo(InstalledModInfo && i) noexcept
	: m_modID(std::move(i.m_modID))
	, m_modName(std::move(i.m_modName))
	, m_modVersion(std::move(i.m_modVersion))
	, m_installedTimestamp(i.m_installedTimestamp)
	, m_originalFiles(std::move(i.m_originalFiles))
	, m_modFiles(std::move(i.m_modFiles)) { }

InstalledModInfo::InstalledModInfo(const InstalledModInfo & i)
	: m_modID(i.m_modID)
	, m_modName(i.m_modName)
	, m_modVersion(i.m_modVersion)
	, m_installedTimestamp(i.m_installedTimestamp)
	, m_originalFiles(i.m_originalFiles)
	, m_modFiles(i.m_modFiles) { }

InstalledModInfo & InstalledModInfo::operator = (InstalledModInfo && i) noexcept {
	if(this != &i) {
		m_modID = std::move(i.m_modID);
		m_modName = std::move(i.m_modName);
		m_modVersion = std::move(i.m_modVersion);
		m_installedTimestamp = i.m_installedTimestamp;
		m_originalFiles = std::move(i.m_originalFiles);
		m_modFiles = std::move(i.m_modFiles);
	}

	return *this;
}

InstalledModInfo & InstalledModInfo::operator = (const InstalledModInfo & i) {
	m_modID = i.m_modID;
	m_modName = i.m_modName;
	m_modVersion = i.m_modVersion;
	m_installedTimestamp = i.m_installedTimestamp;
	m_originalFiles = i.m_originalFiles;
	m_modFiles = i.m_modFiles;

	return *this;
}

InstalledModInfo::~InstalledModInfo() = default;

const std::string & InstalledModInfo::getModID() const {
	return m_modID;
}

const std::string & InstalledModInfo::getModName() const {
	return m_modName;
}

const std::string & InstalledModInfo::getModVersion() const {
	return m_modVersion;
}

std::string InstalledModInfo::getFullModName() const {
	return m_modName + (m_modVersion.empty() ? "" : " ") + m_modVersion;
}

std::chrono::time_point<std::chrono::system_clock> InstalledModInfo::getInstalledTimestamp() const {
	return m_installedTimestamp;
}

bool InstalledModInfo::isEmpty() const {
	return m_originalFiles.empty() && m_modFiles.empty();
}

size_t InstalledModInfo::numberOfOriginalFiles() const {
	return m_originalFiles.size();
}

bool InstalledModInfo::hasOriginalFile(const std::string & filePath) const {
	return indexOfOriginalFile(filePath) != std::numeric_limits<size_t>::max();
}

size_t InstalledModInfo::indexOfOriginalFile(const std::string & filePath) const {
	std::vector<std::string>::const_iterator originalFilesIterator(std::find_if(m_originalFiles.cbegin(), m_originalFiles.cend(), [filePath](const std::string & currentFilePath) {
		return Utilities::areStringsEqualIgnoreCase(filePath, currentFilePath);
	}));

	if(originalFilesIterator == m_originalFiles.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return originalFilesIterator - m_originalFiles.cbegin();
}

std::string InstalledModInfo::getOriginalFile(size_t index) const {
	if(index >= m_originalFiles.size()) {
		return {};
	}

	return m_originalFiles[index];
}

const std::vector<std::string> InstalledModInfo::getOriginalFiles() const {
	return m_originalFiles;
}

bool InstalledModInfo::addOriginalFile(const std::string & filePath) {
	if(filePath.empty() || hasOriginalFile(filePath)) {
		return false;
	}

	m_originalFiles.push_back(filePath);

	return true;
}

bool InstalledModInfo::removeOriginalFile(size_t index) {
	if(index >= m_originalFiles.size()) {
		return false;
	}

	m_originalFiles.erase(m_originalFiles.cbegin() + index);

	return true;
}

bool InstalledModInfo::removeOriginalFile(const std::string filePath) {
	return removeOriginalFile(indexOfOriginalFile(filePath));
}

void InstalledModInfo::clearOriginalFiles() {
	m_originalFiles.clear();
}

size_t InstalledModInfo::numberOfModFiles() const {
	return m_modFiles.size();
}

bool InstalledModInfo::hasModFile(const std::string & filePath) const {
	return indexOfModFile(filePath) != std::numeric_limits<size_t>::max();
}

size_t InstalledModInfo::indexOfModFile(const std::string & filePath) const {
	std::vector<std::string>::const_iterator modFilesIterator(std::find_if(m_modFiles.cbegin(), m_modFiles.cend(), [filePath](const std::string & currentFilePath) {
		return Utilities::areStringsEqualIgnoreCase(filePath, currentFilePath);
	}));

	if(modFilesIterator == m_modFiles.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return modFilesIterator - m_modFiles.cbegin();
}

std::string InstalledModInfo::getModFile(size_t index) const {
	if(index >= m_modFiles.size()) {
		return {};
	}

	return m_modFiles[index];
}

const std::vector<std::string> InstalledModInfo::getModFiles() const {
	return m_modFiles;
}

bool InstalledModInfo::addModFile(const std::string & filePath) {
	if(filePath.empty() || hasModFile(filePath)) {
		return false;
	}

	m_modFiles.push_back(filePath);

	return true;
}

bool InstalledModInfo::removeModFile(size_t index) {
	if(index >= m_modFiles.size()) {
		return false;
	}

	m_modFiles.erase(m_modFiles.cbegin() + index);

	return true;
}

bool InstalledModInfo::removeModFile(const std::string filePath) {
	return removeModFile(indexOfModFile(filePath));
}

void InstalledModInfo::clearModFiles() {
	m_modFiles.clear();
}

rapidjson::Document InstalledModInfo::toJSON() const {
	rapidjson::Document installedModInfoDocument(rapidjson::kObjectType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = installedModInfoDocument.GetAllocator();

	installedModInfoDocument.AddMember(rapidjson::StringRef(JSON_FILE_TYPE_PROPERTY_NAME.c_str()), rapidjson::StringRef(FILE_TYPE.c_str()), allocator);

	installedModInfoDocument.AddMember(rapidjson::StringRef(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME.c_str()), rapidjson::StringRef(FILE_FORMAT_VERSION.c_str()), allocator);

	rapidjson::Value modInfoValue(rapidjson::kObjectType);

	if(!m_modID.empty()) {
		rapidjson::Value modIDValue(m_modID.c_str(), allocator);
		modInfoValue.AddMember(rapidjson::StringRef(JSON_MOD_ID_PROPERTY_NAME.c_str()), modIDValue, allocator);
	}

	if(!m_modName.empty()) {
		rapidjson::Value modNameValue(m_modName.c_str(), allocator);
		modInfoValue.AddMember(rapidjson::StringRef(JSON_MOD_NAME_PROPERTY_NAME.c_str()), modNameValue, allocator);
	}

	if(!m_modVersion.empty()) {
		rapidjson::Value modVersionValue(m_modVersion.c_str(), allocator);
		modInfoValue.AddMember(rapidjson::StringRef(JSON_MOD_VERSION_PROPERTY_NAME.c_str()), modVersionValue, allocator);
	}

	installedModInfoDocument.AddMember(rapidjson::StringRef(JSON_MOD_INFO_CATEGORY_PROPERTY_NAME.c_str(), JSON_MOD_INFO_CATEGORY_PROPERTY_NAME.length()), modInfoValue, allocator);

	rapidjson::Value installedTimestampValue(Utilities::timePointToString(m_installedTimestamp, Utilities::TimeFormat::ISO8601).c_str(), allocator);
	installedModInfoDocument.AddMember(rapidjson::StringRef(JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME.c_str()), installedTimestampValue, allocator);

	rapidjson::Value originalFilesValue(rapidjson::kArrayType);
	originalFilesValue.Reserve(m_originalFiles.size(), allocator);

	for(const std::string & originalFile : m_originalFiles) {
		rapidjson::Value originalFileValue(originalFile.c_str(), allocator);
		originalFilesValue.PushBack(originalFileValue, allocator);
	}

	installedModInfoDocument.AddMember(rapidjson::StringRef(JSON_ORIGINAL_FILES_LIST_PROPERTY_NAME.c_str()), originalFilesValue, allocator);

	rapidjson::Value modFilesValue(rapidjson::kArrayType);
	modFilesValue.Reserve(m_modFiles.size(), allocator);

	for(const std::string & modFile : m_modFiles) {
		rapidjson::Value modFileValue(modFile.c_str(), allocator);
		modFilesValue.PushBack(modFileValue, allocator);
	}

	installedModInfoDocument.AddMember(rapidjson::StringRef(JSON_MOD_FILES_LIST_PROPERTY_NAME.c_str()), modFilesValue, allocator);

	return installedModInfoDocument;
}

std::unique_ptr<InstalledModInfo> InstalledModInfo::parseFrom(const rapidjson::Value & installedModInfoValue) {
	if(!installedModInfoValue.IsObject()) {
		spdlog::error("Invalid installed mod info type: '{}', expected 'object'.", Utilities::typeToString(installedModInfoValue.GetType()));
		return nullptr;
	}

	// verify the file type
	if(installedModInfoValue.HasMember(JSON_FILE_TYPE_PROPERTY_NAME.c_str())) {
		const rapidjson::Value & fileTypeValue = installedModInfoValue[JSON_FILE_TYPE_PROPERTY_NAME.c_str()];

		if(!fileTypeValue.IsString()) {
			spdlog::error("Invalid installed mod info file type type: '{}', expected: 'string'.", Utilities::typeToString(fileTypeValue.GetType()));
			return false;
		}

		if(!Utilities::areStringsEqualIgnoreCase(fileTypeValue.GetString(), FILE_TYPE)) {
			spdlog::error("Incorrect installed mod info file type: '{}', expected: '{}'.", fileTypeValue.GetString(), FILE_TYPE);
			return false;
		}
	}
	else {
		spdlog::warn("Installed mod info JSON data is missing file type, and may fail to load correctly!");
	}

	// verify file format version
	if(installedModInfoValue.HasMember(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME.c_str())) {
		const rapidjson::Value & fileFormatVersionValue = installedModInfoValue[JSON_FILE_FORMAT_VERSION_PROPERTY_NAME.c_str()];

		if(!fileFormatVersionValue.IsString()) {
			spdlog::error("Invalid installed mod info file format version type: '{}', expected: 'string'.", Utilities::typeToString(fileFormatVersionValue.GetType()));
			return false;
		}

		std::optional<std::uint8_t> optionalVersionComparison(Utilities::compareVersions(fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION));

		if(!optionalVersionComparison.has_value()) {
			spdlog::error("Invalid installed mod info file format version: '{}'.", fileFormatVersionValue.GetString());
			return false;
		}

		if(*optionalVersionComparison != 0) {
			spdlog::error("Unsupported installed mod info file format version: '{}', only version '{}' is supported.", fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION);
			return false;
		}
	}
	else {
		spdlog::warn("Installed mod info JSON data is missing file format version, and may fail to load correctly!");
	}

	// check for unhandled installed mod info properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = installedModInfoValue.MemberBegin(); i != installedModInfoValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_INSTALLED_MOD_INFO_PROPERTY_NAMES) {
			if(Utilities::areStringsEqual(i->name.GetString(), propertyName)) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::warn("Installed mod info has unexpected property '{}'.", i->name.GetString());
		}
	}

	// retrieve mod info category
	if(!installedModInfoValue.HasMember(JSON_MOD_INFO_CATEGORY_PROPERTY_NAME.c_str())) {
		spdlog::error("Installed mod info is missing '{}' property.", JSON_MOD_INFO_CATEGORY_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modInfoValue = installedModInfoValue[JSON_MOD_INFO_CATEGORY_PROPERTY_NAME.c_str()];

	if(!modInfoValue.IsObject()) {
		spdlog::error("Installed mod info has an invalid '{}' property type: '{}', expected 'object'.", JSON_MOD_INFO_CATEGORY_PROPERTY_NAME, Utilities::typeToString(modInfoValue.GetType()));
		return nullptr;
	}

	// parse mod identifier
	std::string modID;

	if(modInfoValue.HasMember(JSON_MOD_ID_PROPERTY_NAME.c_str())) {
		const rapidjson::Value & modIDValue = modInfoValue[JSON_MOD_ID_PROPERTY_NAME.c_str()];

		if(!modIDValue.IsString()) {
			spdlog::error("Installed mod info has an invalid '{}' category '{}' property type: '{}', expected 'string'.", JSON_MOD_INFO_CATEGORY_PROPERTY_NAME, JSON_MOD_ID_PROPERTY_NAME, Utilities::typeToString(modIDValue.GetType()));
			return nullptr;
		}

		modID = Utilities::trimString(modIDValue.GetString());
	}

	// parse mod name
	std::string modName;

	if(modInfoValue.HasMember(JSON_MOD_NAME_PROPERTY_NAME.c_str())) {
		const rapidjson::Value & modNameValue = modInfoValue[JSON_MOD_NAME_PROPERTY_NAME.c_str()];

		if(!modNameValue.IsString()) {
			spdlog::error("Installed mod info has an invalid '{}' category '{}' property type: '{}', expected 'string'.", JSON_MOD_INFO_CATEGORY_PROPERTY_NAME, JSON_MOD_NAME_PROPERTY_NAME, Utilities::typeToString(modNameValue.GetType()));
			return nullptr;
		}

		modName = Utilities::trimString(modNameValue.GetString());
	}

	// parse mod version
	std::string modVersion;

	if(modInfoValue.HasMember(JSON_MOD_VERSION_PROPERTY_NAME.c_str())) {
		const rapidjson::Value & modVersionValue = modInfoValue[JSON_MOD_VERSION_PROPERTY_NAME.c_str()];

		if(!modVersionValue.IsString()) {
			spdlog::error("Installed mod info has an invalid '{}' category '{}' property type: '{}', expected 'string'.", JSON_MOD_INFO_CATEGORY_PROPERTY_NAME, JSON_MOD_VERSION_PROPERTY_NAME, Utilities::typeToString(modVersionValue.GetType()));
			return nullptr;
		}

		modVersion = Utilities::trimString(modVersionValue.GetString());
	}

	// parse mod installed timestamp
	if(!installedModInfoValue.HasMember(JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME.c_str())) {
		spdlog::error("Installed mod info is missing '{}' property.", JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & installedTimestampValue = installedModInfoValue[JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME.c_str()];

	if(!installedTimestampValue.IsString()) {
		spdlog::error("Installed mod info has an invalid '{}' property type: '{}', expected 'string'.", JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME, Utilities::typeToString(installedTimestampValue.GetType()));
		return nullptr;
	}

	std::optional<std::chrono::time_point<std::chrono::system_clock>> optionalInstalledTimestamp(Utilities::parseTimePointFromString(installedTimestampValue.GetString(), Utilities::TimeFormat::ISO8601));

	if(!optionalInstalledTimestamp.has_value()) {
		spdlog::error("Installed mod info has an invalid '{}' property value: '{}', expected valid ISO8601 timestamp string.", JSON_INSTALLED_TIMESTAMP_PROPERTY_NAME, installedTimestampValue.GetString());
		return nullptr;
	}

	// retrieve original files list
	if(!installedModInfoValue.HasMember(JSON_ORIGINAL_FILES_LIST_PROPERTY_NAME.c_str())) {
		spdlog::error("Installed mod info is missing '{}' property.", JSON_ORIGINAL_FILES_LIST_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & originalFilesValue = installedModInfoValue[JSON_ORIGINAL_FILES_LIST_PROPERTY_NAME.c_str()];

	if(!originalFilesValue.IsArray()) {
		spdlog::error("Invalid installed mod info '{}' property type: '{}', expected 'array'.", JSON_ORIGINAL_FILES_LIST_PROPERTY_NAME, Utilities::typeToString(originalFilesValue.GetType()));
		return nullptr;
	}

	// retrieve mod files list
	if(!installedModInfoValue.HasMember(JSON_MOD_FILES_LIST_PROPERTY_NAME.c_str())) {
		spdlog::error("Installed mod info is missing '{}' property.", JSON_MOD_FILES_LIST_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modFilesValue = installedModInfoValue[JSON_MOD_FILES_LIST_PROPERTY_NAME.c_str()];

	if(!modFilesValue.IsArray()) {
		spdlog::error("Invalid installed mod info '{}' property type: '{}', expected 'array'.", JSON_MOD_FILES_LIST_PROPERTY_NAME, Utilities::typeToString(modFilesValue.GetType()));
		return nullptr;
	}

	std::vector<std::string> originalFiles;

	for(rapidjson::Value::ConstValueIterator i = originalFilesValue.Begin(); i != originalFilesValue.End(); ++i) {
		std::string originalFile(Utilities::trimString((*i).GetString()));

		if(originalFile.empty()) {
			spdlog::warn("Skipping invalid empty installed mod info original file.");
			continue;
		}

		std::vector<std::string>::const_iterator existingOriginalFileIterator = std::find_if(originalFiles.cbegin(), originalFiles.cend(), [originalFile](const std::string & currentOriginalFile) {
			return Utilities::areStringsEqualIgnoreCase(originalFile, currentOriginalFile);
		});

		if(existingOriginalFileIterator != originalFiles.cend()) {
			spdlog::warn("Skipping duplicate installed mod info original file.");
			continue;
		}

		originalFiles.emplace_back(originalFile);
	}

	std::vector<std::string> modFiles;

	for(rapidjson::Value::ConstValueIterator i = modFilesValue.Begin(); i != modFilesValue.End(); ++i) {
		std::string modFile(Utilities::trimString((*i).GetString()));

		if(modFile.empty()) {
			spdlog::warn("Skipping invalid empty installed mod info mod file.");
			continue;
		}

		std::vector<std::string>::const_iterator existingModFileIterator = std::find_if(modFiles.cbegin(), modFiles.cend(), [modFile](const std::string & currentModFile) {
			return Utilities::areStringsEqualIgnoreCase(modFile, currentModFile);
		});

		if(existingModFileIterator != modFiles.cend()) {
			spdlog::warn("Skipping duplicate installed mod info mod file.");
			continue;
		}

		modFiles.emplace_back(modFile);
	}

	return std::unique_ptr<InstalledModInfo>(new InstalledModInfo(modID, modName, modVersion, optionalInstalledTimestamp.value(), originalFiles, modFiles));
}

std::unique_ptr<InstalledModInfo> InstalledModInfo::loadFrom(const GameVersion & gameVersion) {
	if(!gameVersion.isConfigured()) {
		return nullptr;
	}

	return loadFrom(Utilities::joinPaths(gameVersion.getGamePath(), DEFAULT_FILE_NAME));
}

std::unique_ptr<InstalledModInfo> InstalledModInfo::loadFrom(const std::string & filePath) {
	if(filePath.empty()) {
		return nullptr;
	}

	std::string fileExtension(Utilities::getFileExtension(filePath));

	if(fileExtension.empty()) {
		return nullptr;
	}
	else if(Utilities::areStringsEqualIgnoreCase(fileExtension, "json")) {
		return loadFromJSON(filePath);
	}

	return nullptr;
}

std::unique_ptr<InstalledModInfo> InstalledModInfo::loadFromJSON(const std::string & filePath) {
	if(filePath.empty() || !std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		return nullptr;
	}

	std::ifstream fileStream(filePath);

	if(!fileStream.is_open()) {
		return nullptr;
	}

	rapidjson::Document modsValue;
	rapidjson::IStreamWrapper fileStreamWrapper(fileStream);
	if(modsValue.ParseStream(fileStreamWrapper).HasParseError()) {
		return nullptr;
	}

	fileStream.close();

	std::unique_ptr<InstalledModInfo> installedModInfo(parseFrom(modsValue));

	if(!InstalledModInfo::isValid(installedModInfo.get())) {
		spdlog::error("Failed to parse installed mod info from JSON file: '{}'.", filePath);
		return nullptr;
	}

	return installedModInfo;
}

bool InstalledModInfo::saveTo(const GameVersion & gameVersion) {
	if(!gameVersion.isConfigured()) {
		return false;
	}

	return saveTo(Utilities::joinPaths(gameVersion.getGamePath(), DEFAULT_FILE_NAME));
}

bool InstalledModInfo::saveTo(const std::string & filePath, bool overwrite) const {
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

bool InstalledModInfo::saveToJSON(const std::string & filePath, bool overwrite) const {
	if(!overwrite && std::filesystem::exists(std::filesystem::path(filePath))) {
		spdlog::warn("File '{}' already exists, use overwrite to force write.", filePath);
		return false;
	}

	std::ofstream fileStream(filePath);

	if(!fileStream.is_open()) {
		return false;
	}

	rapidjson::Document installedModInfo(toJSON());

	rapidjson::OStreamWrapper fileStreamWrapper(fileStream);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> fileStreamWriter(fileStreamWrapper);
	fileStreamWriter.SetIndent('\t', 1);
	installedModInfo.Accept(fileStreamWriter);
	fileStream.close();

	return true;
}

bool InstalledModInfo::isValid() const {
	for(const std::string & originalFile : m_originalFiles) {
		if(originalFile.empty()) {
			return false;
		}
	}

	for(const std::string & modFile : m_modFiles) {
		if(modFile.empty()) {
			return false;
		}
	}

	return true;
}

bool InstalledModInfo::isValid(const InstalledModInfo * i) {
	return i != nullptr && i->isValid();
}

bool InstalledModInfo::operator == (const InstalledModInfo & i) const {
	return m_installedTimestamp == i.m_installedTimestamp &&
		   Utilities::areStringsEqual(m_modID, i.m_modID) &&
		   Utilities::areStringsEqual(m_modName, i.m_modName) &&
		   Utilities::areStringsEqual(m_modVersion, i.m_modVersion) &&
		   m_originalFiles == i.m_originalFiles &&
		   m_modFiles == i.m_modFiles;
}

bool InstalledModInfo::operator != (const InstalledModInfo & i) const {
	return !operator == (i);
}
