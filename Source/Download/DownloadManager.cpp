#include "DownloadManager.h"

#include "CachedFile.h"
#include "CachedPackageFile.h"
#include "DownloadCache.h"
#include "Game/GameVersion.h"
#include "Game/GameVersionCollection.h"
#include "Manager/SettingsManager.h"
#include "Mod/ModDownload.h"
#include "Mod/Mod.h"
#include "Mod/ModFile.h"
#include "Mod/ModGameVersion.h"
#include "Mod/ModVersion.h"
#include "Mod/ModVersionType.h"

#include <Analytics/Segment/SegmentAnalytics.h>
#include <Archive/Zip/ZipArchive.h>
#include <Network/HTTPService.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>

#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>

using namespace std::chrono_literals;

DownloadManager::DownloadManager(std::shared_ptr<SettingsManager> settings)
	: m_initialized(false)
	, m_settings(settings)
	, m_downloadCache(std::make_unique<DownloadCache>()) { }

DownloadManager::~DownloadManager() = default;

bool DownloadManager::isInitialized() const {
	return m_initialized;
}

bool DownloadManager::initialize() {
	if(m_initialized) {
		return true;
	}

	if(!HTTPService::getInstance()->isInitialized()) {
		spdlog::error("Failed to initialize download manager, HTTP service not initialized!");
		return false;
	}

	if(m_settings == nullptr) {
		spdlog::error("Failed to initialize download manager, invalid settings manager provided.");
		return false;
	}

	if(!createRequiredDirectories()) {
		spdlog::error("Failed to create required download manager directories!");
		return false;
	}

	loadDownloadCache();

	if(!downloadModList()) {
		return false;
	}

	m_initialized = true;

	return true;
}

bool DownloadManager::uninitialize() {
	if(!m_initialized) {
		return false;
	}

	saveDownloadCache();

	m_initialized = false;

	return true;
}

size_t DownloadManager::numberOfDownloadedMods() const {
	return m_downloadCache->numberOfCachedPackageFiles();
}

std::string DownloadManager::getDownloadCacheFilePath() const {
	if(m_settings == nullptr) {
		return {};
	}

	if(m_settings->downloadsDirectoryPath.empty()) {
		spdlog::error("Missing downloads directory path setting.");
		return {};
	}

	if(m_settings->downloadCacheFileName.empty()) {
		spdlog::error("Missing download cache file name setting.");
		return {};
	}

	return Utilities::joinPaths(m_settings->downloadsDirectoryPath, m_settings->downloadCacheFileName);
}

std::string DownloadManager::getCachedModListFilePath() const {
	if(m_settings == nullptr) {
		return {};
	}

	if(m_settings->downloadsDirectoryPath.empty()) {
		spdlog::error("Missing downloads directory path setting.");
		return {};
	}

	if(m_settings->modDownloadsDirectoryName.empty()) {
		spdlog::error("Missing mod downloads directory name setting.");
		return {};
	}

	if(m_settings->remoteModsListFileName.empty()) {
		spdlog::error("Missing remote mods list file name setting.");
		return {};
	}

	return Utilities::joinPaths(m_settings->downloadsDirectoryPath, m_settings->modDownloadsDirectoryName, m_settings->remoteModsListFileName);
}

std::string DownloadManager::getDownloadedModsDirectoryPath() const {
	if(m_settings == nullptr) {
		return {};
	}

	if(m_settings->downloadsDirectoryPath.empty()) {
		spdlog::error("Missing downloads directory path setting.");
		return {};
	}

	if(m_settings->modDownloadsDirectoryName.empty()) {
		spdlog::error("Missing mod downloads directory name setting.");
		return {};
	}

	return Utilities::joinPaths(m_settings->downloadsDirectoryPath, m_settings->modDownloadsDirectoryName);
}

std::string DownloadManager::getDownloadedMapsDirectoryPath() const {
	if(m_settings == nullptr) {
		return {};
	}

	if(m_settings->downloadsDirectoryPath.empty()) {
		spdlog::error("Missing downloads directory path setting.");
		return {};
	}

	if(m_settings->mapDownloadsDirectoryName.empty()) {
		spdlog::error("Missing map downloads directory name setting.");
		return {};
	}

	return Utilities::joinPaths(m_settings->downloadsDirectoryPath, m_settings->mapDownloadsDirectoryName);
}

bool DownloadManager::createRequiredDirectories() {
	if(m_settings == nullptr) {
		return false;
	}

	if(m_settings->downloadsDirectoryPath.empty()) {
		spdlog::error("Missing downloads directory path setting.");
		return false;
	}

	std::error_code errorCode;
	std::filesystem::path downloadsDirectoryPath(m_settings->downloadsDirectoryPath);

	if(!std::filesystem::is_directory(downloadsDirectoryPath)) {
		std::filesystem::create_directories(downloadsDirectoryPath, errorCode);

		if(errorCode) {
			spdlog::error("Failed to create downloads directory '{}': {}", downloadsDirectoryPath.string(), errorCode.message());
			return false;
		}

		spdlog::debug("Created downloads directory: '{}'.", downloadsDirectoryPath.string());
	}

	std::array<std::filesystem::path, 3> downloadSubdirectories = {
		Utilities::joinPaths(m_settings->downloadsDirectoryPath, m_settings->modDownloadsDirectoryName),
		Utilities::joinPaths(m_settings->downloadsDirectoryPath, m_settings->mapDownloadsDirectoryName),
		Utilities::joinPaths(m_settings->downloadsDirectoryPath, m_settings->gameDownloadsDirectoryName)
	};

	for(const std::filesystem::path & subdirectory : downloadSubdirectories) {
		if(!std::filesystem::is_directory(subdirectory)) {
			std::filesystem::create_directory(subdirectory, errorCode);

			if(errorCode) {
				spdlog::error("Failed to create downloads sub-directory '{}': {}", subdirectory.string(), errorCode.message());
				return false;
			}

			spdlog::debug("Created downloads sub-directory: '{}'.", subdirectory.string());
		}
	}

	return true;
}

bool DownloadManager::loadDownloadCache() {
	return m_downloadCache->loadFrom(getDownloadCacheFilePath());
}

bool DownloadManager::saveDownloadCache() const {
	return m_downloadCache->saveTo(getDownloadCacheFilePath());
}

bool DownloadManager::downloadModList(bool force) {
	if(m_settings == nullptr) {
		return false;
	}

	HTTPService * httpService = HTTPService::getInstance();

	std::shared_ptr<CachedFile> cachedModListFile(m_downloadCache->getCachedModListFile());

	std::string modListFileName(m_settings->remoteModsListFileName);
	std::string modListLocalFilePath(Utilities::joinPaths(getDownloadedModsDirectoryPath(), modListFileName));
	std::string modListRemoteFilePath(Utilities::joinPaths(m_settings->remoteDownloadsDirectoryName, m_settings->remoteModDownloadsDirectoryName, modListFileName));
	std::string modListURL(Utilities::joinPaths(httpService->getBaseURL(), modListRemoteFilePath));

	spdlog::info("Downloading Duke Nukem 3D mod list from: '{}'...", modListURL);

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, modListRemoteFilePath));

	if(!force && cachedModListFile != nullptr) {
		request->setIfNoneMatchETag(cachedModListFile->getETag());
	}

	std::future<std::shared_ptr<HTTPResponse>> futureResponse(httpService->sendRequest(request));

	if(!futureResponse.valid()) {
		spdlog::error("Failed to create Duke Nukem 3D mod list file HTTP request!");
		return false;
	}

	futureResponse.wait();

	std::shared_ptr<HTTPResponse> response(futureResponse.get());

	if(response->isFailure()) {
		spdlog::error("Failed to download Duke Nukem 3D mod list with error: {}", response->getErrorMessage());
		return false;
	}

	if(response->getStatusCode() == magic_enum::enum_integer(HTTPStatusCode::NotModified)) {
		spdlog::info("Duke Nukem 3D mod list is already up to date!");
		return true;
	}
	else if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to download Duke Nukem 3D mod list ({}{})!", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return false;
	}

	spdlog::info("Duke Nukem 3D mod list downloaded successfully after {} ms to file: '{}'.", response->getRequestDuration().value().count(), modListLocalFilePath);

	response->getBody()->writeTo(modListLocalFilePath, true);

	std::string modListSHA1(response->getBody()->getSHA1());
	m_downloadCache->updateCachedModListFile(modListFileName, response->getBody()->getSize(), modListSHA1, response->getETag());

	saveDownloadCache();

	SegmentAnalytics * segmentAnalytics = SegmentAnalytics::getInstance();

	std::map<std::string, std::any> properties;
	properties["fileName"] = modListFileName;
	properties["fileSize"] = response->getBody()->getSize();
	properties["sha1"] = modListSHA1;
	properties["eTag"] = response->getETag();
	properties["transferDuration"] = response->getRequestDuration().value().count();

	segmentAnalytics->track("Mod List Downloaded", properties);

	return true;
}

bool DownloadManager::downloadModGameVersion(ModGameVersion * modGameVersion, GameVersionCollection * gameVersions, bool force) {
	if(!m_initialized) {
		return false;
	}

	HTTPService * httpService = HTTPService::getInstance();

	if(!ModGameVersion::isValid(modGameVersion)) {
		spdlog::error("Failed to download mod, invalid mod game version provided!");
		return false;
	}

	if(!GameVersionCollection::isValid(gameVersions)) {
		spdlog::error("Failed to download mod, invalid game version collection provided!");
		return false;
	}

	std::shared_ptr<GameVersion> gameVersion(gameVersions->getGameVersion(modGameVersion->getGameVersion()));

	if(gameVersion == nullptr) {
		spdlog::error("Failed to download '{}' mod, could not find '{}' game version! Is your game version configuration file missing information?", modGameVersion->getParentModVersionType()->getFullName(), modGameVersion->getGameVersion());
		return false;
	}

	std::shared_ptr<ModDownload> modDownload(modGameVersion->getDownload());

	if(modDownload == nullptr) {
		spdlog::error("Failed to obtain download for mod game version: '{}'. Is your mod collection data correct?", modGameVersion->getFullName());
		return false;
	}

	std::shared_ptr<CachedPackageFile> cachedModPackageFile(m_downloadCache->getCachedPackageFile(modDownload.get()));

	std::string modDownloadLocalBasePath(Utilities::joinPaths(m_settings->downloadsDirectoryPath, m_settings->modDownloadsDirectoryName, gameVersion->getModDirectoryName()));
	std::string modDownloadLocalFilePath(Utilities::joinPaths(modDownloadLocalBasePath, modDownload->getFileName()));
	std::string modDownloadRemoteFilePath(Utilities::joinPaths(m_settings->remoteDownloadsDirectoryName, m_settings->remoteModDownloadsDirectoryName, modDownload->getSubfolder(), Utilities::toLowerCase(gameVersion->getModDirectoryName()), modDownload->getFileName()));
	std::string modDownloadURL(Utilities::joinPaths(httpService->getBaseURL(), modDownloadRemoteFilePath));

	spdlog::info("Downloading '{}' mod from: '{}'...", modGameVersion->getFullName(), modDownloadURL);

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, modDownloadRemoteFilePath));

	request->setNetworkTimeout(30min);

	if(!force && cachedModPackageFile != nullptr) {
		request->setIfNoneMatchETag(cachedModPackageFile->getETag());
	}

	std::future<std::shared_ptr<HTTPResponse>> futureResponse(httpService->sendRequest(request));

	if(!futureResponse.valid()) {
		spdlog::error("Failed to create '{}' mod package file HTTP request!", modGameVersion->getFullName());
		return false;
	}

	futureResponse.wait();

	std::shared_ptr<HTTPResponse> response(futureResponse.get());

	if(response->isFailure()) {
		spdlog::error("Failed to download '{}' mod package file with error: {}", modGameVersion->getFullName(), response->getErrorMessage());
		return false;
	}

	if(response->getStatusCode() == magic_enum::enum_integer(HTTPStatusCode::NotModified)) {
		spdlog::info("Mod '{}' is already up to date!", modGameVersion->getFullName());
		return true;
	}
	else if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to download '{}' mod package file ({}{})!", modGameVersion->getFullName(), response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return false;
	}

	spdlog::info("Successfully downloaded '{}' mod package file '{}' after {} ms, verifying file integrity using SHA1 hash...", modGameVersion->getFullName(), modDownload->getFileName(), response->getRequestDuration().value().count());

	std::string modPackageFileSHA1(response->getBody()->getSHA1());
	if(modPackageFileSHA1 != modDownload->getSHA1()) {
		spdlog::error("Failed to download '{}' mod package file '{}' due to data corruption, SHA1 hash check failed!", modGameVersion->getFullName(), modDownload->getFileName());
		return false;
	}

	spdlog::debug("Mod '{}' package file '{}' file integrity verified!", modGameVersion->getFullName(), modDownload->getFileName());

	std::unique_ptr<ZipArchive> modDownloadZipArchive(ZipArchive::createFrom(response->transferBody(), Utilities::emptyString, true));

	if(modDownloadZipArchive == nullptr) {
		spdlog::error("Failed to create zip archive handle from mod '{}' package file '{}'!", modGameVersion->getFullName(), modDownload->getFileName());
		return false;
	}

	// verify mod file SHA1 hashes
	std::shared_ptr<ModFile> modZipFile(modGameVersion->getFirstFileOfType("zip"));

	if(modZipFile != nullptr) {
		std::weak_ptr<ArchiveEntry> modFileZipEntry(modDownloadZipArchive->getEntry(modZipFile->getFileName()));

		if(modFileZipEntry.expired()) {
			spdlog::error("Failed to download '{}' mod package file '{}', mod zip file '{}' not found!", modGameVersion->getFullName(), modDownload->getFileName(), modZipFile->getFileName());
			return false;
		}

		std::unique_ptr<ByteBuffer> modFileZipEntryData(modFileZipEntry.lock()->getData());

		if(modFileZipEntryData == nullptr) {
			spdlog::error("Failed to download '{}' mod package file '{}', could not retrieve zip entry file '{}' data!", modGameVersion->getFullName(), modDownload->getFileName(), modZipFile->getFileName());
			return false;
		}

		if(!Utilities::areStringsEqualIgnoreCase(modFileZipEntryData->getSHA1(), modZipFile->getSHA1())) {
			spdlog::error("Failed to download '{}' mod package file '{}', zip entry file '{}' SHA1 hash validation failed!", modGameVersion->getFullName(), modDownload->getFileName(), modZipFile->getFileName());
			return false;
		}
	}
	else {
		std::shared_ptr<ModFile> modFile;
		std::weak_ptr<ArchiveEntry> modFileEntry;

		for(size_t i = 0; i < modGameVersion->numberOfFiles(); i++) {
			modFile = modGameVersion->getFile(i);

			// eDuke32 mod files can be read straight out of the group or zip file, and are not stored separately
			if(modGameVersion->isEDuke32() && modFile->getType() != "zip" && modFile->getType() != "grp") {
				continue;
			}

			modFileEntry = modDownloadZipArchive->getEntry(modFile->getFileName());

			if(modFileEntry.expired()) {
				spdlog::error("Failed to download '{}' mod package file '{}', mod file '{}' not found!", modGameVersion->getFullName(), modDownload->getFileName(), modFile->getFileName());
				return false;
			}

			std::unique_ptr<ByteBuffer> modFileEntryData(modFileEntry.lock()->getData());

			if(modFileEntryData == nullptr) {
				spdlog::error("Failed to download '{}' mod package file '{}', could not retrieve zip entry file '{}' data!", modGameVersion->getFullName(), modDownload->getFileName(), modFile->getFileName());
				return false;
			}

			if(!Utilities::areStringsEqualIgnoreCase(modFileEntryData->getSHA1(), modFile->getSHA1())) {
				spdlog::error("Failed to download '{}' mod package file '{}', zip entry file '{}' SHA1 hash validation failed!", modGameVersion->getFullName(), modDownload->getFileName(), modFile->getFileName());
				return false;
			}
		}
	}

	if(!modDownloadZipArchive->extractAllEntries(modDownloadLocalBasePath, true)) {
		spdlog::error("Failed to extract '{}' mod package file '{}' contents to directory: '{}'.", modGameVersion->getFullName(), modDownload->getFileName(), modDownloadLocalBasePath);
		return false;
	}

	m_downloadCache->updateCachedPackageFile(modDownload.get(), modGameVersion, modDownloadZipArchive->getCompressedSize(), response->getETag());

	saveDownloadCache();

	SegmentAnalytics * segmentAnalytics = SegmentAnalytics::getInstance();

	std::map<std::string, std::any> properties;
	properties["modID"] = modGameVersion->getParentMod()->getID();
	properties["modName"] = modGameVersion->getParentMod()->getName();
	properties["modVersion"] = modGameVersion->getParentModVersion()->getVersion();
	properties["modVersionType"] = modGameVersion->getParentModVersionType()->getType();
	properties["fullModName"] = modGameVersion->getFullName();
	properties["gameVersion"] = modGameVersion->getGameVersion();
	properties["fileName"] = modDownload->getFileName();
	properties["fileSize"] = response->getBody()->getSize();
	properties["numberOfFiles"] = modDownloadZipArchive->numberOfFiles();
	properties["sha1"] = modPackageFileSHA1;
	properties["eTag"] = response->getETag();
	properties["transferDuration"] = response->getRequestDuration().value().count();

	segmentAnalytics->track("Mod Downloaded", properties);

	return true;
}
