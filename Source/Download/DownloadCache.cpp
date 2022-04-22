#include "DownloadCache.h"

#include "CachedFile.h"
#include "CachedPackageFile.h"
#include "Mod/ModDownload.h"
#include "Mod/ModFile.h"
#include "Mod/ModGameVersion.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <fmt/core.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>

#include <array>
#include <filesystem>
#include <fstream>

static constexpr const char * JSON_DOWNLOAD_CACHE_ID_COUNTER_PROPERTY_NAME = "idCounter";
static constexpr const char * JSON_DOWNLOAD_CACHE_MOD_LIST_PROPERTY_NAME = "modList";
static constexpr const char * JSON_DOWNLOAD_CACHE_PACKAGES_PROPERTY_NAME = "packages";
static const std::array<std::string_view, 3> JSON_CACHED_PACKAGE_FILE_PROPERTY_NAMES = {
	JSON_DOWNLOAD_CACHE_ID_COUNTER_PROPERTY_NAME,
	JSON_DOWNLOAD_CACHE_MOD_LIST_PROPERTY_NAME,
	JSON_DOWNLOAD_CACHE_PACKAGES_PROPERTY_NAME
};

DownloadCache::DownloadCache()
	: m_cachedFileIDCounter(1) { }

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
		m_cachedModListFile = createCachedFile(fileName, fileSize, sha1, eTag);
	}
	else {
		m_cachedModListFile->setFileName(fileName);
		m_cachedModListFile->setFileSize(fileSize);
		m_cachedModListFile->setSHA1(sha1);
		m_cachedModListFile->setETag(eTag);
	}

	return true;
}

size_t DownloadCache::numberOfCachedPackageFiles() const {
	return m_cachedPackageFiles.size();
}

bool DownloadCache::hasCachedPackageFileWithName(const std::string & fileName) const {
	return m_cachedPackageFiles.find(fileName) != m_cachedPackageFiles.end();
}

bool DownloadCache::hasCachedPackageFile(const ModDownload * modDownload) const {
	if(!ModDownload::isValid(modDownload)) {
		return false;
	}

	return m_cachedPackageFiles.find(modDownload->getFileName()) != m_cachedPackageFiles.end();
}

std::shared_ptr<CachedPackageFile> DownloadCache::getCachedPackageFile(const ModDownload * modDownload) const {
	if(!ModDownload::isValid(modDownload)) {
		return false;
	}

	std::map<std::string, std::shared_ptr<CachedPackageFile>>::const_iterator cachedPackageFile(m_cachedPackageFiles.find(modDownload->getFileName()));

	if(cachedPackageFile == m_cachedPackageFiles.end()) {
		return nullptr;
	}

	return cachedPackageFile->second;
}

bool DownloadCache::hasCachedFile(const ModFile * modFile) const {
	return getCachedFile(modFile) != nullptr;
}

std::shared_ptr<CachedFile> DownloadCache::getCachedFile(const ModFile * modFile) const {
	if(!ModFile::isValid(modFile)) {
		return nullptr;
	}

	const ModGameVersion * modGameVersion = modFile->getParentModGameVersion();

	if(modGameVersion == nullptr) {
		return nullptr;
	}

	std::shared_ptr<ModDownload> modDownload(modGameVersion->getDownload());

	if(modDownload == nullptr) {
		return nullptr;
	}

	std::shared_ptr<CachedPackageFile> cachedPackageFile(getCachedPackageFile(modDownload.get()));

	if(cachedPackageFile == nullptr) {
		return nullptr;
	}

	return cachedPackageFile->getCachedFileWithName(modFile->getFileName());
}

bool DownloadCache::updateCachedPackageFile(const ModDownload * modDownload, const ModGameVersion * modGameVersion, uint64_t fileSize, const std::string & eTag) {
	if(!ModDownload::isValid(modDownload) || !ModGameVersion::isValid(modGameVersion) || modDownload->getParentMod() != modGameVersion->getParentMod() || eTag.empty()) {
		fmt::print("Failed to update cached package file, invalid arguments provided.\n");
		return false;
	}

	bool isNewCachedPackageFile = false;
	std::shared_ptr<CachedPackageFile> cachedPackageFile(getCachedPackageFile(modDownload));

	if(cachedPackageFile == nullptr) {
		cachedPackageFile = createCachedPackageFile(modDownload->getFileName(), fileSize, modDownload->getSHA1(), eTag);
		isNewCachedPackageFile = true;
	}
	else {
		cachedPackageFile->setFileName(modDownload->getFileName());
		cachedPackageFile->setFileSize(fileSize);
		cachedPackageFile->setSHA1(modDownload->getSHA1());
		cachedPackageFile->setETag(eTag);
	}

	if(modGameVersion == nullptr) {
		fmt::print("Failed to update cached package file '{}', could not find corresponding mod game version.\n", modDownload->getFileName());
		return false;
	}

	std::shared_ptr<CachedFile> cachedFile;
	std::shared_ptr<ModFile> modFile;

	for(size_t i = 0; i < modGameVersion->numberOfFiles(); i++) {
		modFile = modGameVersion->getFile(i);

		if(modGameVersion->isEDuke32() && modFile->getType() != "zip" && modFile->getType() != "grp") {
			continue;
		}

		if(!isNewCachedPackageFile) {
			cachedFile = cachedPackageFile->getCachedFileWithName(modFile->getFileName());
		}

		if(cachedFile == nullptr) {
			cachedPackageFile->addCachedFile(createCachedFile(modFile->getFileName(), 0, modFile->getSHA1(), Utilities::emptyString));
		}
		else {
			m_cachedModListFile->setFileName(modFile->getFileName());
			m_cachedModListFile->setSHA1(modFile->getSHA1());
		}
	}

	if(isNewCachedPackageFile) {
		m_cachedPackageFiles[modFile->getFileName()] = cachedPackageFile;
	}

	return true;
}

void DownloadCache::removeCachedPackageFile(const std::string & modPackageDownloadFileName) {
	if(modPackageDownloadFileName.empty()) {
		return;
	}

	m_cachedPackageFiles.erase(modPackageDownloadFileName);
}

void DownloadCache::removeCachedPackageFile(const CachedPackageFile * cachedPackageFile) {
	if(cachedPackageFile == nullptr) {
		return;
	}

	removeCachedPackageFile(cachedPackageFile->getFileName());
}

void DownloadCache::removeCachedPackageFile(const ModDownload * modDownload) {
	if(!ModDownload::isValid(modDownload)) {
		return;
	}

	removeCachedPackageFile(modDownload->getFileName());
}

void DownloadCache::clearCachedPackageFiles() {
	m_cachedPackageFiles.clear();
}

std::unique_ptr<CachedFile> DownloadCache::createCachedFile(const std::string & fileName, uint64_t fileSize, const std::string & sha1, const std::string & eTag) {
	return std::make_unique<CachedFile>(m_cachedFileIDCounter++, fileName, fileSize, sha1, eTag);
}

std::unique_ptr<CachedPackageFile> DownloadCache::createCachedPackageFile(const std::string & fileName, uint64_t fileSize, const std::string & sha1, const std::string & eTag) {
	return std::make_unique<CachedPackageFile>(m_cachedFileIDCounter++, fileName, fileSize, sha1, eTag);
}

rapidjson::Document DownloadCache::toJSON() const {
	rapidjson::Document downloadCacheDocument(rapidjson::kObjectType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = downloadCacheDocument.GetAllocator();

	downloadCacheDocument.AddMember(rapidjson::StringRef(JSON_DOWNLOAD_CACHE_ID_COUNTER_PROPERTY_NAME), rapidjson::Value(m_cachedFileIDCounter), allocator);

	if(m_cachedModListFile != nullptr) {
		rapidjson::Value modListValue(m_cachedModListFile->toJSON(allocator));
		downloadCacheDocument.AddMember(rapidjson::StringRef(JSON_DOWNLOAD_CACHE_MOD_LIST_PROPERTY_NAME), modListValue, allocator);
	}

	rapidjson::Value packagesValue(rapidjson::kArrayType);

	for(std::map<std::string, std::shared_ptr<CachedPackageFile>>::const_iterator i = m_cachedPackageFiles.begin(); i != m_cachedPackageFiles.end(); ++i) {
		packagesValue.PushBack(i->second->toJSON(allocator), allocator);
	}

	downloadCacheDocument.AddMember(rapidjson::StringRef(JSON_DOWNLOAD_CACHE_PACKAGES_PROPERTY_NAME), packagesValue, allocator);

	return downloadCacheDocument;
}

std::unique_ptr<DownloadCache> DownloadCache::parseFrom(const rapidjson::Value & downloadCacheValue) {
	if(!downloadCacheValue.IsObject()) {
		fmt::print("Invalid download cache type: '{}', expected 'object'.\n", Utilities::typeToString(downloadCacheValue.GetType()));
		return nullptr;
	}

	std::unique_ptr<DownloadCache> newDownloadCache(std::make_unique<DownloadCache>());

	// parse download cache ID counter
	if(!downloadCacheValue.HasMember(JSON_DOWNLOAD_CACHE_ID_COUNTER_PROPERTY_NAME)) {
		fmt::print("Download cache is missing '{}' property'.\n", JSON_DOWNLOAD_CACHE_ID_COUNTER_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & idCounterValue = downloadCacheValue[JSON_DOWNLOAD_CACHE_ID_COUNTER_PROPERTY_NAME];

	if(!idCounterValue.IsUint64()) {
		fmt::print("Download cache has an invalid '{}' property type: '{}', expected unsigned integer 'number'.\n", JSON_DOWNLOAD_CACHE_ID_COUNTER_PROPERTY_NAME, Utilities::typeToString(idCounterValue.GetType()));
		return nullptr;
	}

	newDownloadCache->m_cachedFileIDCounter = idCounterValue.GetUint64();

	if(newDownloadCache->m_cachedFileIDCounter == 0) {
		fmt::print("Download cache has an invalid '{}' property value: '{}', expected positive integer greater than 0.\n", JSON_DOWNLOAD_CACHE_ID_COUNTER_PROPERTY_NAME, Utilities::typeToString(idCounterValue.GetType()));
		return nullptr;
	}

	// parse download cache mod list file cache information
	if(downloadCacheValue.HasMember(JSON_DOWNLOAD_CACHE_MOD_LIST_PROPERTY_NAME)) {
		const rapidjson::Value & modListValue = downloadCacheValue[JSON_DOWNLOAD_CACHE_MOD_LIST_PROPERTY_NAME];

		newDownloadCache->m_cachedModListFile = std::shared_ptr<CachedFile>(std::move(CachedFile::parseFrom(modListValue)).release());

		if(!CachedFile::isValid(newDownloadCache->m_cachedModListFile.get())) {
			fmt::print("Failed to parse download cache mod list cached file.\n");
			return nullptr;
		}

		if(newDownloadCache->m_cachedModListFile->getETag().empty()) {
			fmt::print("Failed to parse download cache mod list cached file due to missing ETag value.\n");
			return nullptr;
		}
	}

	// parse download cache packages cache information property
	if(!downloadCacheValue.HasMember(JSON_DOWNLOAD_CACHE_PACKAGES_PROPERTY_NAME)) {
		fmt::print("Download cache is missing '{}' property'.\n", JSON_DOWNLOAD_CACHE_PACKAGES_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & packagesValue = downloadCacheValue[JSON_DOWNLOAD_CACHE_PACKAGES_PROPERTY_NAME];

	if(!packagesValue.IsArray()) {
		fmt::print("Invalid download cache '{}' type: '{}', expected 'array'.\n", JSON_DOWNLOAD_CACHE_PACKAGES_PROPERTY_NAME, Utilities::typeToString(packagesValue.GetType()));
		return nullptr;
	}

	if(!packagesValue.Empty()) {
		std::unique_ptr<CachedPackageFile> newCachedPackageFile;

		for(rapidjson::Value::ConstValueIterator i = packagesValue.Begin(); i != packagesValue.End(); ++i) {
			newCachedPackageFile = std::move(CachedPackageFile::parseFrom(*i));

			if(!CachedPackageFile::isValid(newCachedPackageFile.get())) {
				fmt::print("Failed to parse download cache package cached file #{}!\n", newDownloadCache->m_cachedPackageFiles.size() + 1);
				return nullptr;
			}

			if(newDownloadCache->hasCachedPackageFileWithName(newCachedPackageFile->getFileName())) {
				fmt::print("Encountered duplicate download cache package cached file #{}.\n", newDownloadCache->m_cachedPackageFiles.size() + 1);
				return nullptr;
			}

			std::string cachedPackageFileName(newCachedPackageFile->getFileName());

			newDownloadCache->m_cachedPackageFiles[cachedPackageFileName] = std::shared_ptr<CachedPackageFile>(newCachedPackageFile.release());
		}
	}

	return newDownloadCache;
}

bool DownloadCache::loadFrom(const std::string & filePath) {
	if(filePath.empty()) {
		fmt::print("Cannot load from empty download cache file path.\n");
		return false;
	}

	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		return false;
	}

	std::ifstream fileStream(filePath);

	if(!fileStream.is_open()) {
		fmt::print("Failed to open download cache file for reading!\n");
		return false;
	}

	rapidjson::Document downloadCacheValue;
	rapidjson::IStreamWrapper fileStreamWrapper(fileStream);
	if(downloadCacheValue.ParseStream(fileStreamWrapper).HasParseError()) {
		fmt::print("Invalid download cache JSON file data!\n");
		return false;
	}

	fileStream.close();

	std::unique_ptr<DownloadCache> newDownloadCache(parseFrom(downloadCacheValue));

	if(!DownloadCache::isValid(newDownloadCache.get())) {
		fmt::print("Failed to parse download cache from JSON file '{}'.\n", filePath);
		return false;
	}

	m_cachedFileIDCounter = newDownloadCache->m_cachedFileIDCounter;
	m_cachedModListFile = std::move(newDownloadCache->m_cachedModListFile);
	m_cachedPackageFiles = std::move(newDownloadCache->m_cachedPackageFiles);

#if _DEBUG
	fmt::print("Successfully loaded download cache from file: '{}'.\n", filePath);
#endif // _DEBUG

	return true;
}

bool DownloadCache::saveTo(const std::string & filePath) const {
	if(filePath.empty()) {
		fmt::print("Cannot save to empty download cache file path.\n");
		return false;
	}

	std::ofstream fileStream(filePath);

	if(!fileStream.is_open()) {
		fmt::print("Failed to open download cache file for writing.\n");
		return false;
	}

	rapidjson::Document downloadCache(toJSON());

	rapidjson::OStreamWrapper fileStreamWrapper(fileStream);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> fileStreamWriter(fileStreamWrapper);
	fileStreamWriter.SetIndent('\t', 1);
	downloadCache.Accept(fileStreamWriter);
	fileStream.close();

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

	return m_cachedFileIDCounter > 0;
}

bool DownloadCache::isValid(const DownloadCache * d) {
	return d != nullptr && d->isValid();
}
