#include "DownloadCache.h"

#include "CachedFile.h"
#include "CachedPackageFile.h"
#include "Game/GameVersion.h"
#include "Mod/ModDownload.h"
#include "Mod/ModFile.h"
#include "Mod/ModGameVersion.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <spdlog/spdlog.h>

#include <array>
#include <filesystem>
#include <fstream>

static constexpr const char * JSON_DOWNLOAD_CACHE_FILE_TYPE_PROPERTY_NAME = "fileType";
static constexpr const char * JSON_DOWNLOAD_CACHE_FILE_FORMAT_VERSION_PROPERTY_NAME = "fileFormatVersion";
static constexpr const char * JSON_DOWNLOAD_CACHE_MOD_LIST_PROPERTY_NAME = "modList";
static constexpr const char * JSON_DOWNLOAD_CACHE_PACKAGES_PROPERTY_NAME = "packages";
static const std::array<std::string_view, 4> JSON_DOWNLOAD_CACHE_PROPERTY_NAMES = {
	JSON_DOWNLOAD_CACHE_FILE_TYPE_PROPERTY_NAME,
	JSON_DOWNLOAD_CACHE_FILE_FORMAT_VERSION_PROPERTY_NAME,
	JSON_DOWNLOAD_CACHE_MOD_LIST_PROPERTY_NAME,
	JSON_DOWNLOAD_CACHE_PACKAGES_PROPERTY_NAME
};

const std::string DownloadCache::FILE_TYPE = "Download Cache";
const uint32_t DownloadCache::FILE_FORMAT_VERSION = 1;

DownloadCache::DownloadCache() = default;

DownloadCache::DownloadCache(DownloadCache && downloadCache) noexcept
	: m_cachedModListFile(std::move(downloadCache.m_cachedModListFile))
	, m_cachedPackageFiles(std::move(downloadCache.m_cachedPackageFiles)) { }

const DownloadCache & DownloadCache::operator = (DownloadCache && downloadCache) noexcept {
	if(this != &downloadCache) {
		m_cachedModListFile = std::move(downloadCache.m_cachedModListFile);
		m_cachedPackageFiles = std::move(downloadCache.m_cachedPackageFiles);
	}

	return *this;
}

DownloadCache::~DownloadCache() = default;

bool DownloadCache::hasCachedModListFile() const {
	return m_cachedModListFile != nullptr;
}

std::shared_ptr<CachedFile> DownloadCache::getCachedModListFile() const {
	return m_cachedModListFile;
}

bool DownloadCache::updateCachedModListFile(const std::string & fileName, uint64_t fileSize, const std::string & sha1, const std::string & eTag) {
	if(fileName.empty() || sha1.empty() || eTag.empty()) {
		return false;
	}

	if(m_cachedModListFile == nullptr) {
		m_cachedModListFile = createCachedFile(fileName, fileSize, sha1, eTag, std::chrono::system_clock::now());
	}
	else {
		m_cachedModListFile->setFileName(fileName);
		m_cachedModListFile->setFileSize(fileSize);
		m_cachedModListFile->setSHA1(sha1);
		m_cachedModListFile->setETag(eTag);
		m_cachedModListFile->setDownloadedTimePoint(std::chrono::system_clock::now());
	}

	return true;
}

size_t DownloadCache::numberOfCachedPackageFiles() const {
	return m_cachedPackageFiles.size();
}

bool DownloadCache::hasCachedPackageFileWithName(const std::string & fileName) const {
	return m_cachedPackageFiles.find(fileName) != m_cachedPackageFiles.end();
}

bool DownloadCache::hasCachedPackageFile(const ModDownload & modDownload) const {
	if(!modDownload.isValid()) {
		return false;
	}

	return m_cachedPackageFiles.find(modDownload.getFileName()) != m_cachedPackageFiles.end();
}

std::shared_ptr<CachedPackageFile> DownloadCache::getCachedPackageFile(const ModDownload & modDownload) const {
	if(!modDownload.isValid()) {
		return nullptr;
	}

	std::map<std::string, std::shared_ptr<CachedPackageFile>>::const_iterator cachedPackageFile(m_cachedPackageFiles.find(modDownload.getFileName()));

	if(cachedPackageFile == m_cachedPackageFiles.end()) {
		return nullptr;
	}

	return cachedPackageFile->second;
}

bool DownloadCache::hasCachedFile(const ModFile & modFile) const {
	return getCachedFile(modFile) != nullptr;
}

std::shared_ptr<CachedFile> DownloadCache::getCachedFile(const ModFile & modFile) const {
	if(!modFile.isValid()) {
		return nullptr;
	}

	const ModGameVersion * modGameVersion = modFile.getParentModGameVersion();

	if(modGameVersion == nullptr) {
		return nullptr;
	}

	std::shared_ptr<ModDownload> modDownload(modGameVersion->getDownload());

	if(modDownload == nullptr) {
		return nullptr;
	}

	std::shared_ptr<CachedPackageFile> cachedPackageFile(getCachedPackageFile(*modDownload));

	if(cachedPackageFile == nullptr) {
		return nullptr;
	}

	return cachedPackageFile->getCachedFileWithName(modFile.getFileName());
}

bool DownloadCache::updateCachedPackageFile(const ModDownload & modDownload, const ModGameVersion & modGameVersion, const GameVersion & gameVersion, uint64_t fileSize, const std::string & eTag) {
	if(!modDownload.isValid()) {
		spdlog::error("Failed to update cached package file, mod download is invalid.");
		return false;
	}

	if(!modGameVersion.isValid()) {
		spdlog::error("Failed to update cached package file, mod game version is invalid.");
		return false;
	}

	if(modDownload.getParentMod() != modGameVersion.getParentMod()) {
		spdlog::error("Failed to update cached package file, mod download does not share the same parent mod as the mod game version.");
		return false;
	}

	if(eTag.empty()) {
		spdlog::error("Failed to update cached package file, ETag is empty.");
		return false;
	}

	if(!Utilities::areStringsEqualIgnoreCase(modGameVersion.getGameVersionID(), gameVersion.getID())) {
		spdlog::error("Failed to update cached package file, mod game version ID does not match provided game version ID.");
		return false;
	}

	bool isNewCachedPackageFile = false;
	std::shared_ptr<CachedPackageFile> cachedPackageFile(getCachedPackageFile(modDownload));

	if(cachedPackageFile == nullptr) {
		cachedPackageFile = createCachedPackageFile(modDownload.getFileName(), fileSize, modDownload.getSHA1(), eTag, std::chrono::system_clock::now());
		isNewCachedPackageFile = true;
	}
	else {
		cachedPackageFile->setFileName(modDownload.getFileName());
		cachedPackageFile->setFileSize(fileSize);
		cachedPackageFile->setSHA1(modDownload.getSHA1());
		cachedPackageFile->setETag(eTag);
		cachedPackageFile->setDownloadedTimePoint(std::chrono::system_clock::now());
	}

	std::shared_ptr<CachedFile> cachedFile;
	std::shared_ptr<ModFile> modFile;

	for(size_t i = 0; i < modGameVersion.numberOfFiles(); i++) {
		modFile = modGameVersion.getFile(i);

		if(gameVersion.areScriptFilesReadFromGroup() && modFile->getType() != "zip" && modFile->getType() != "grp") {
			continue;
		}

		if(!isNewCachedPackageFile) {
			cachedFile = cachedPackageFile->getCachedFileWithName(modFile->getFileName());
		}

		if(cachedFile == nullptr) {
			cachedPackageFile->addCachedFile(createCachedFile(modFile->getFileName(), modFile->getFileSize(), modFile->getSHA1(), Utilities::emptyString));
		}
		else {
			cachedFile->setFileName(modFile->getFileName());
			cachedFile->setFileSize(modFile->getFileSize());
			cachedFile->setSHA1(modFile->getSHA1());
		}
	}

	if(isNewCachedPackageFile) {
		m_cachedPackageFiles[modDownload.getFileName()] = cachedPackageFile;
	}

	return true;
}

void DownloadCache::removeCachedPackageFile(const std::string & modPackageDownloadFileName) {
	if(modPackageDownloadFileName.empty()) {
		return;
	}

	m_cachedPackageFiles.erase(modPackageDownloadFileName);
}

void DownloadCache::removeCachedPackageFile(const CachedPackageFile & cachedPackageFile) {
	removeCachedPackageFile(cachedPackageFile.getFileName());
}

void DownloadCache::removeCachedPackageFile(const ModDownload & modDownload) {
	if(!modDownload.isValid()) {
		return;
	}

	removeCachedPackageFile(modDownload.getFileName());
}

void DownloadCache::clearCachedPackageFiles() {
	m_cachedPackageFiles.clear();
}

std::unique_ptr<CachedFile> DownloadCache::createCachedFile(const std::string & fileName, uint64_t fileSize, const std::string & sha1, const std::string & eTag, std::optional<std::chrono::time_point<std::chrono::system_clock>> downloadedTimePoint) {
	return std::make_unique<CachedFile>(fileName, fileSize, sha1, eTag, downloadedTimePoint);
}

std::unique_ptr<CachedPackageFile> DownloadCache::createCachedPackageFile(const std::string & fileName, uint64_t fileSize, const std::string & sha1, const std::string & eTag, std::optional<std::chrono::time_point<std::chrono::system_clock>> downloadedTimePoint) {
	return std::make_unique<CachedPackageFile>(fileName, fileSize, sha1, eTag, downloadedTimePoint);
}

rapidjson::Document DownloadCache::toJSON() const {
	rapidjson::Document downloadCacheDocument(rapidjson::kObjectType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = downloadCacheDocument.GetAllocator();

	rapidjson::Value fileTypeValue(FILE_TYPE.c_str(), allocator);
	downloadCacheDocument.AddMember(rapidjson::StringRef(JSON_DOWNLOAD_CACHE_FILE_TYPE_PROPERTY_NAME), fileTypeValue, allocator);

	downloadCacheDocument.AddMember(rapidjson::StringRef(JSON_DOWNLOAD_CACHE_FILE_FORMAT_VERSION_PROPERTY_NAME), rapidjson::Value(FILE_FORMAT_VERSION), allocator);

	if(m_cachedModListFile != nullptr) {
		rapidjson::Value modListValue(m_cachedModListFile->toJSON(allocator));
		downloadCacheDocument.AddMember(rapidjson::StringRef(JSON_DOWNLOAD_CACHE_MOD_LIST_PROPERTY_NAME), modListValue, allocator);
	}

	rapidjson::Value packagesValue(rapidjson::kArrayType);
	packagesValue.Reserve(m_cachedPackageFiles.size(), allocator);

	for(std::map<std::string, std::shared_ptr<CachedPackageFile>>::const_iterator i = m_cachedPackageFiles.begin(); i != m_cachedPackageFiles.end(); ++i) {
		packagesValue.PushBack(i->second->toJSON(allocator), allocator);
	}

	downloadCacheDocument.AddMember(rapidjson::StringRef(JSON_DOWNLOAD_CACHE_PACKAGES_PROPERTY_NAME), packagesValue, allocator);

	return downloadCacheDocument;
}

std::unique_ptr<DownloadCache> DownloadCache::parseFrom(const rapidjson::Value & downloadCacheValue) {
	if(!downloadCacheValue.IsObject()) {
		spdlog::error("Invalid download cache type: '{}', expected 'object'.", Utilities::typeToString(downloadCacheValue.GetType()));
		return nullptr;
	}

	// check for unhandled download cache properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = downloadCacheValue.MemberBegin(); i != downloadCacheValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_DOWNLOAD_CACHE_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::warn("Download cache has unexpected property '{}'.", i->name.GetString());
		}
	}

	if(downloadCacheValue.HasMember(JSON_DOWNLOAD_CACHE_FILE_TYPE_PROPERTY_NAME)) {
		const rapidjson::Value & fileTypeValue = downloadCacheValue[JSON_DOWNLOAD_CACHE_FILE_TYPE_PROPERTY_NAME];

		if(!fileTypeValue.IsString()) {
			spdlog::error("Invalid download cache file type type: '{}', expected: 'string'.", Utilities::typeToString(fileTypeValue.GetType()));
			return nullptr;
		}

		if(!Utilities::areStringsEqualIgnoreCase(fileTypeValue.GetString(), FILE_TYPE)) {
			spdlog::error("Incorrect download cache file type: '{}', expected: '{}'.", fileTypeValue.GetString(), FILE_TYPE);
			return nullptr;
		}
	}
	else {
		spdlog::warn("Download cache JSON data is missing file type, and may fail to load correctly!");
	}

	if(downloadCacheValue.HasMember(JSON_DOWNLOAD_CACHE_FILE_FORMAT_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & fileFormatVersionValue = downloadCacheValue[JSON_DOWNLOAD_CACHE_FILE_FORMAT_VERSION_PROPERTY_NAME];

		if(!fileFormatVersionValue.IsUint()) {
			spdlog::error("Invalid download cache file format version type: '{}', expected unsigned integer 'number'.", Utilities::typeToString(fileFormatVersionValue.GetType()));
			return nullptr;
		}

		if(fileFormatVersionValue.GetUint() != FILE_FORMAT_VERSION) {
			spdlog::error("Unsupported download cache file format version: {}, only version {} is supported.", fileFormatVersionValue.GetUint(), FILE_FORMAT_VERSION);
			return nullptr;
		}
	}
	else {
		spdlog::warn("Download cache JSON data is missing file format version, and may fail to load correctly!");
	}

	std::unique_ptr<DownloadCache> newDownloadCache(std::make_unique<DownloadCache>());

	// parse download cache mod list file cache information
	if(downloadCacheValue.HasMember(JSON_DOWNLOAD_CACHE_MOD_LIST_PROPERTY_NAME)) {
		const rapidjson::Value & modListValue = downloadCacheValue[JSON_DOWNLOAD_CACHE_MOD_LIST_PROPERTY_NAME];

		newDownloadCache->m_cachedModListFile = CachedFile::parseFrom(modListValue);

		if(!CachedFile::isValid(newDownloadCache->m_cachedModListFile.get())) {
			spdlog::error("Failed to parse download cache mod list cached file.");
			return nullptr;
		}

		if(newDownloadCache->m_cachedModListFile->getETag().empty()) {
			spdlog::error("Failed to parse download cache mod list cached file due to missing ETag value.");
			return nullptr;
		}
	}

	// parse download cache packages cache information property
	if(!downloadCacheValue.HasMember(JSON_DOWNLOAD_CACHE_PACKAGES_PROPERTY_NAME)) {
		spdlog::error("Download cache is missing '{}' property.", JSON_DOWNLOAD_CACHE_PACKAGES_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & packagesValue = downloadCacheValue[JSON_DOWNLOAD_CACHE_PACKAGES_PROPERTY_NAME];

	if(!packagesValue.IsArray()) {
		spdlog::error("Invalid download cache '{}' type: '{}', expected 'array'.", JSON_DOWNLOAD_CACHE_PACKAGES_PROPERTY_NAME, Utilities::typeToString(packagesValue.GetType()));
		return nullptr;
	}

	if(!packagesValue.Empty()) {
		std::unique_ptr<CachedPackageFile> newCachedPackageFile;

		for(rapidjson::Value::ConstValueIterator i = packagesValue.Begin(); i != packagesValue.End(); ++i) {
			newCachedPackageFile = std::move(CachedPackageFile::parseFrom(*i));

			if(!CachedPackageFile::isValid(newCachedPackageFile.get())) {
				spdlog::error("Failed to parse download cache package cached file #{}!", newDownloadCache->m_cachedPackageFiles.size() + 1);
				return nullptr;
			}

			if(newDownloadCache->hasCachedPackageFileWithName(newCachedPackageFile->getFileName())) {
				spdlog::error("Encountered duplicate download cache package cached file #{}.", newDownloadCache->m_cachedPackageFiles.size() + 1);
				return nullptr;
			}

			std::string cachedPackageFileName(newCachedPackageFile->getFileName());

			newDownloadCache->m_cachedPackageFiles[cachedPackageFileName] = std::move(newCachedPackageFile);
		}
	}

	return newDownloadCache;
}

bool DownloadCache::loadFrom(const std::string & filePath) {
	if(filePath.empty()) {
		spdlog::error("Cannot load from empty download cache file path.");
		return false;
	}

	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		return false;
	}

	std::ifstream fileStream(filePath);

	if(!fileStream.is_open()) {
		spdlog::error("Failed to open download cache file for reading!");
		return false;
	}

	rapidjson::Document downloadCacheValue;
	rapidjson::IStreamWrapper fileStreamWrapper(fileStream);
	if(downloadCacheValue.ParseStream(fileStreamWrapper).HasParseError()) {
		spdlog::error("Invalid download cache JSON file data!");
		return false;
	}

	fileStream.close();

	std::unique_ptr<DownloadCache> newDownloadCache(parseFrom(downloadCacheValue));

	if(!DownloadCache::isValid(newDownloadCache.get())) {
		spdlog::error("Failed to parse download cache from JSON file '{}'.", filePath);
		return false;
	}

	m_cachedModListFile = std::move(newDownloadCache->m_cachedModListFile);
	m_cachedPackageFiles = std::move(newDownloadCache->m_cachedPackageFiles);

#if _DEBUG
	spdlog::debug("Successfully loaded download cache from file: '{}'.", filePath);
#endif // _DEBUG

	return true;
}

bool DownloadCache::saveTo(const std::string & filePath) const {
	if(filePath.empty()) {
		spdlog::error("Cannot save to empty download cache file path.");
		return false;
	}

	std::ofstream fileStream(filePath);

	if(!fileStream.is_open()) {
		spdlog::error("Failed to open download cache file for writing.");
		return false;
	}

	rapidjson::Document downloadCache(toJSON());

	rapidjson::OStreamWrapper fileStreamWrapper(fileStream);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> fileStreamWriter(fileStreamWrapper);
	fileStreamWriter.SetIndent('\t', 1);
	downloadCache.Accept(fileStreamWriter);
	fileStream.close();

	spdlog::info("Saved download cache to file: '{}'.", filePath);

	return true;
}

bool DownloadCache::isValid() const {
	if(m_cachedModListFile != nullptr && !m_cachedModListFile->isValid()) {
		return false;
	}

	for(std::map<std::string, std::shared_ptr<CachedPackageFile>>::const_iterator i = m_cachedPackageFiles.begin(); i != m_cachedPackageFiles.end(); ++i) {
		if(!i->second->isValid()) {
			return false;
		}
	}

	return true;
}

bool DownloadCache::isValid(const DownloadCache * d) {
	return d != nullptr && d->isValid();
}
