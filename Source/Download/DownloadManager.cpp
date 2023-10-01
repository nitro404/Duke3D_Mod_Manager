#include "DownloadManager.h"

#include "CachedFile.h"
#include "CachedPackageFile.h"
#include "DownloadCache.h"
#include "Game/GameVersion.h"
#include "Game/GameVersionCollection.h"
#include "Manager/SettingsManager.h"
#include "Mod/ModDownload.h"
#include "Mod/Mod.h"
#include "Mod/ModCollection.h"
#include "Mod/ModFile.h"
#include "Mod/ModGameVersion.h"
#include "Mod/ModVersion.h"
#include "Mod/ModVersionType.h"

#include <Analytics/Segment/SegmentAnalytics.h>
#include <Archive/Zip/ZipArchive.h>
#include <Network/HTTPService.h>
#include <Platform/DeviceInformationBridge.h>
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

DownloadManager::DownloadManager()
	: m_initialized(false)
	, m_downloadCache(std::make_unique<DownloadCache>()) { }

DownloadManager::DownloadManager(DownloadManager && downloadManager) noexcept
	: m_initialized(downloadManager.m_initialized)
	, m_downloadCache(std::move(downloadManager.m_downloadCache)) { }

const DownloadManager & DownloadManager::operator = (DownloadManager && downloadManager) noexcept {
	if(this != &downloadManager) {
		m_initialized = downloadManager.m_initialized;
		m_downloadCache = std::move(downloadManager.m_downloadCache);
	}

	return *this;
}

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

	if(!createRequiredDirectories()) {
		spdlog::error("Failed to create required download manager directories!");
		return false;
	}

	loadDownloadCache();

	if(shouldUpdateModList() && !downloadModList()) {
		if(isModListDownloaded()) {
			spdlog::info("Failed to download latest mod list, using last downloaded mod list instead.");
		}
		else {
			return false;
		}
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
	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->downloadsDirectoryPath.empty()) {
		spdlog::error("Missing downloads directory path setting.");
		return {};
	}

	if(settings->downloadCacheFileName.empty()) {
		spdlog::error("Missing download cache file name setting.");
		return {};
	}

	return Utilities::joinPaths(settings->downloadsDirectoryPath, settings->downloadCacheFileName);
}

std::string DownloadManager::getCachedModListFilePath() const {
	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->downloadsDirectoryPath.empty()) {
		spdlog::error("Missing downloads directory path setting.");
		return {};
	}

	if(settings->modDownloadsDirectoryName.empty()) {
		spdlog::error("Missing mod downloads directory name setting.");
		return {};
	}

	if(settings->remoteModsListFileName.empty()) {
		spdlog::error("Missing remote mods list file name setting.");
		return {};
	}

	return Utilities::joinPaths(settings->downloadsDirectoryPath, settings->modDownloadsDirectoryName, settings->remoteModsListFileName);
}

std::string DownloadManager::getDownloadedModsDirectoryPath() const {
	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->downloadsDirectoryPath.empty()) {
		spdlog::error("Missing downloads directory path setting.");
		return {};
	}

	if(settings->modDownloadsDirectoryName.empty()) {
		spdlog::error("Missing mod downloads directory name setting.");
		return {};
	}

	return Utilities::joinPaths(settings->downloadsDirectoryPath, settings->modDownloadsDirectoryName);
}

std::string DownloadManager::getDownloadedMapsDirectoryPath() const {
	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->downloadsDirectoryPath.empty()) {
		spdlog::error("Missing downloads directory path setting.");
		return {};
	}

	if(settings->mapDownloadsDirectoryName.empty()) {
		spdlog::error("Missing map downloads directory name setting.");
		return {};
	}

	return Utilities::joinPaths(settings->downloadsDirectoryPath, settings->mapDownloadsDirectoryName);
}

bool DownloadManager::createRequiredDirectories() {
	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->downloadsDirectoryPath.empty()) {
		spdlog::error("Missing downloads directory path setting.");
		return false;
	}

	std::error_code errorCode;
	std::filesystem::path downloadsDirectoryPath(settings->downloadsDirectoryPath);

	if(!std::filesystem::is_directory(downloadsDirectoryPath)) {
		std::filesystem::create_directories(downloadsDirectoryPath, errorCode);

		if(errorCode) {
			spdlog::error("Failed to create downloads directory '{}': {}", downloadsDirectoryPath.string(), errorCode.message());
			return false;
		}

		spdlog::debug("Created downloads directory: '{}'.", downloadsDirectoryPath.string());
	}

	std::array<std::filesystem::path, 3> downloadSubdirectories = {
		Utilities::joinPaths(settings->downloadsDirectoryPath, settings->modDownloadsDirectoryName),
		Utilities::joinPaths(settings->downloadsDirectoryPath, settings->mapDownloadsDirectoryName),
		Utilities::joinPaths(settings->downloadsDirectoryPath, settings->gameDownloadsDirectoryName)
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

bool DownloadManager::isModListDownloaded() const {
	SettingsManager * settings = SettingsManager::getInstance();

	return std::filesystem::is_regular_file(std::filesystem::path(Utilities::joinPaths(getDownloadedModsDirectoryPath(), settings->remoteModsListFileName)));
}

bool DownloadManager::shouldUpdateModList() const {
	SettingsManager * settings = SettingsManager::getInstance();

	if(!settings->downloadThrottlingEnabled || !settings->modListLastDownloadedTimestamp.has_value() || !isModListDownloaded()) {
		return true;
	}

	return std::chrono::system_clock::now() - settings->modListLastDownloadedTimestamp.value() > settings->modListUpdateFrequency;
}

bool DownloadManager::isModDownloaded(const Mod & mod, const ModCollection * mods, const GameVersionCollection * gameVersions, bool checkDependencies, bool allowCompatibleGameVersions) const {
	if(!mod.isValid(true)) {
		return false;
	}

	for(size_t i = 0; i < mod.numberOfVersions(); i++) {
		if(isModVersionDownloaded(*mod.getVersion(i), mods, gameVersions, checkDependencies, allowCompatibleGameVersions)) {
			return true;
		}
	}

	return false;
}

bool DownloadManager::isModVersionDownloaded(const ModVersion & modVersion, const ModCollection * mods, const GameVersionCollection * gameVersions, bool checkDependencies, bool allowCompatibleGameVersions) const {
	if(!modVersion.isValid(true)) {
		return false;
	}

	for(size_t i = 0; i < modVersion.numberOfTypes(); i++) {
		if(isModVersionTypeDownloaded(*modVersion.getType(i), mods, gameVersions, checkDependencies, allowCompatibleGameVersions)) {
			return true;
		}
	}

	return false;
}

bool DownloadManager::isModVersionTypeDownloaded(const ModVersionType & modVersionType, const ModCollection * mods, const GameVersionCollection * gameVersions, bool checkDependencies, bool allowCompatibleGameVersions) const {
	if(!modVersionType.isValid(true)) {
		return false;
	}

	for(size_t i = 0; i < modVersionType.numberOfGameVersions(); i++) {
		if(isModGameVersionDownloaded(*modVersionType.getGameVersion(i), mods, gameVersions, checkDependencies, allowCompatibleGameVersions)) {
			return true;
		}
	}

	return false;
}

bool DownloadManager::isModGameVersionDownloaded(const ModGameVersion & modGameVersion, const ModCollection * mods, const GameVersionCollection * gameVersions, bool checkDependencies, bool allowCompatibleGameVersions) const {
	if(!modGameVersion.isValid(true)) {
		return false;
	}

	bool modGameVersionDownloaded = false;
	std::shared_ptr<ModDownload> modDownload(modGameVersion.getDownload());

	if(modDownload == nullptr) {
		if(!allowCompatibleGameVersions || gameVersions == nullptr) {
			spdlog::error("Failed to obtain download for mod game version: '{}'. Is your mod collection data correct?", modGameVersion.getFullName(true));
			return false;
		}
	}
	else if(m_downloadCache->hasCachedPackageFile(*modDownload)) {
		modGameVersionDownloaded = true;
	}

	if(!modGameVersionDownloaded) {
		if(!allowCompatibleGameVersions || !GameVersionCollection::isValid(gameVersions)) {
			return false;
		}

		std::shared_ptr<GameVersion> gameVersion(gameVersions->getGameVersionWithID(modGameVersion.getGameVersionID()));

		if(gameVersion == nullptr) {
			return false;
		}

		bool compatibleModGameVersionDownloaded = false;
		std::vector<std::shared_ptr<GameVersion>> compatibleGameVersions(gameVersion->getCompatibleGameVersions(*gameVersions));
		std::shared_ptr<ModGameVersion> compatibleModGameVersion;

		for(std::vector<std::shared_ptr<GameVersion>>::const_reverse_iterator i = compatibleGameVersions.crbegin(); i != compatibleGameVersions.crend(); ++i) {
			compatibleModGameVersion = modGameVersion.getParentModVersionType()->getGameVersionWithID((*i)->getID());

			if(compatibleModGameVersion == nullptr) {
				continue;
			}

			if(isModGameVersionDownloaded(*compatibleModGameVersion, mods, gameVersions, checkDependencies, true)) {
				compatibleModGameVersionDownloaded = true;
				break;
			}
		}

		if(!compatibleModGameVersionDownloaded) {
			return false;
		}
	}

	if(!checkDependencies) {
		return true;
	}

	if(!ModCollection::isValid(mods, gameVersions, true)) {
		return false;
	}

	std::vector<std::shared_ptr<ModGameVersion>> modDependencyGameVersions(mods->getModDependencyGameVersions(modGameVersion));

	for(const std::shared_ptr<ModGameVersion> & dependencyModGameVersion : modDependencyGameVersions) {
		if(!isModGameVersionDownloaded(*dependencyModGameVersion, mods, gameVersions, true, allowCompatibleGameVersions)) {
			return false;
		}
	}

	return true;
}

DownloadCache * DownloadManager::getDownloadCache() const {
	return m_downloadCache.get();
}

bool DownloadManager::downloadModList(bool force) {
	if(!DeviceInformationBridge::getInstance()->isConnectedToInternet()) {
		return isModListDownloaded();
	}

	SettingsManager * settings = SettingsManager::getInstance();

	HTTPService * httpService = HTTPService::getInstance();

	std::shared_ptr<CachedFile> cachedModListFile(m_downloadCache->getCachedModListFile());

	std::string modListFileName(settings->remoteModsListFileName);
	std::string modListLocalFilePath(Utilities::joinPaths(getDownloadedModsDirectoryPath(), modListFileName));
	std::string modListRemoteFilePath(Utilities::joinPaths(settings->remoteDownloadsDirectoryName, settings->remoteModDownloadsDirectoryName, modListFileName));
	std::string modListURL(Utilities::joinPaths(httpService->getBaseURL(), modListRemoteFilePath));

	spdlog::info("Downloading Duke Nukem 3D mod list from: '{}'...", modListURL);

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, modListRemoteFilePath));

	if(!force && cachedModListFile != nullptr) {
		request->setIfNoneMatchETag(cachedModListFile->getETag());
	}

	std::shared_ptr<HTTPResponse> response(httpService->sendRequestAndWait(request));

	if(response == nullptr || response->isFailure()) {
		spdlog::error("Failed to download Duke Nukem 3D mod list with error: {}", response != nullptr ? response->getErrorMessage() : "Invalid request.");
		return false;
	}

	if(response->getStatusCode() == magic_enum::enum_integer(HTTPStatusCode::NotModified)) {
		spdlog::info("Duke Nukem 3D mod list is already up to date!");

		settings->modListLastDownloadedTimestamp = std::chrono::system_clock::now();

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

	settings->modListLastDownloadedTimestamp = std::chrono::system_clock::now();

	if(settings->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["fileName"] = modListFileName;
		properties["fileSize"] = response->getBody()->getSize();
		properties["sha1"] = modListSHA1;
		properties["eTag"] = response->getETag();
		properties["transferDurationMs"] = response->getRequestDuration().value().count();
		properties["forced"] = force;

		SegmentAnalytics::getInstance()->track("Mod List Downloaded", properties);
	}

	return true;
}

bool DownloadManager::downloadModGameVersion(const ModGameVersion & modGameVersion, const ModCollection & mods, const GameVersionCollection & gameVersions, bool downloadDependencies, bool allowCompatibleGameVersions, bool force) {
	if(!m_initialized) {
		return false;
	}

	if(!modGameVersion.isValid()) {
		spdlog::error("Failed to download mod, invalid mod game version provided!");
		return false;
	}

	SettingsManager * settings = SettingsManager::getInstance();
	HTTPService * httpService = HTTPService::getInstance();

	bool standAlone = modGameVersion.isStandAlone();
	std::shared_ptr<GameVersion> gameVersion;

	if(standAlone) {
		gameVersion = modGameVersion.getStandAloneGameVersion();
	}
	else {
		if(!gameVersions.isValid()) {
			spdlog::error("Failed to download mod, invalid game version collection provided!");
			return false;
		}

		gameVersion = gameVersions.getGameVersionWithID(modGameVersion.getGameVersionID());
	}

	if(gameVersion == nullptr) {
		spdlog::error("Failed to download '{}' mod, could not find game version with ID '{}'! Is your game version configuration file missing information?", modGameVersion.getParentModVersionType()->getFullName(), modGameVersion.getGameVersionID());
		return false;
	}

	std::shared_ptr<ModDownload> modDownload(modGameVersion.getDownload());

	if(modDownload == nullptr) {
		spdlog::error("Failed to obtain download for mod game version: '{}'. Is your mod collection data correct?", modGameVersion.getFullName(true));
		return false;
	}

	std::vector<std::shared_ptr<ModGameVersion>> modDependencyGameVersions;

	if(downloadDependencies) {
		if(!mods.isValid(&gameVersions, true)) {
			return false;
		}

		modDependencyGameVersions = mods.getModDependencyGameVersions(modGameVersion, &gameVersions, allowCompatibleGameVersions);
	}

	std::shared_ptr<CachedPackageFile> cachedModPackageFile(m_downloadCache->getCachedPackageFile(*modDownload));

	if(!DeviceInformationBridge::getInstance()->isConnectedToInternet()) {
		if(cachedModPackageFile == nullptr) {
			return false;
		}

		for(const std::shared_ptr<ModGameVersion> & dependencyModGameVersion : modDependencyGameVersions) {
			if(!downloadModGameVersion(*dependencyModGameVersion, mods, gameVersions, true, allowCompatibleGameVersions, force)) {
				return false;
			}
		}

		return true;
	}

	std::string modDownloadLocalBasePath(Utilities::joinPaths(settings->downloadsDirectoryPath, settings->modDownloadsDirectoryName, gameVersion->getModDirectoryName()));
	std::string modPackageDownloadLocalFilePath(Utilities::joinPaths(modDownloadLocalBasePath, modDownload->getFileName()));
	std::string modDownloadRemoteFilePath(Utilities::joinPaths(settings->remoteDownloadsDirectoryName, settings->remoteModDownloadsDirectoryName, modDownload->getSubfolder(), Utilities::toLowerCase(gameVersion->getModDirectoryName()), modDownload->getFileName()));
	std::string modDownloadURL(Utilities::joinPaths(httpService->getBaseURL(), modDownloadRemoteFilePath));

	spdlog::info("Downloading '{}' mod from: '{}'...", modGameVersion.getFullName(true), modDownloadURL);

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, modDownloadRemoteFilePath));

	request->setNetworkTimeout(15s);

	if(!force && cachedModPackageFile != nullptr) {
		request->setIfNoneMatchETag(cachedModPackageFile->getETag());
	}

	std::shared_ptr<HTTPResponse> response(httpService->sendRequestAndWait(request));

	if(response == nullptr || response->isFailure()) {
		spdlog::error("Failed to download '{}' mod package file with error: {}", modGameVersion.getFullName(true), response != nullptr ? response->getErrorMessage() : "Invalid request.");
		return false;
	}
	else if(response->getStatusCode() == magic_enum::enum_integer(HTTPStatusCode::NotModified)) {
		spdlog::info("Mod '{}' is already up to date!", modGameVersion.getFullName(true));

		for(const std::shared_ptr<ModGameVersion> & dependencyModGameVersion : modDependencyGameVersions) {
			if(!downloadModGameVersion(*dependencyModGameVersion, mods, gameVersions, true, allowCompatibleGameVersions, force)) {
				return false;
			}
		}

		return true;
	}
	else if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to download '{}' mod package file ({}{})!", modGameVersion.getFullName(true), response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return false;
	}

	spdlog::info("Successfully downloaded '{}' mod package file '{}' after {} ms, verifying file integrity using SHA1 hash...", modGameVersion.getFullName(true), modDownload->getFileName(), response->getRequestDuration().value().count());

	std::string modPackageFileSHA1(response->getBody()->getSHA1());
	if(modPackageFileSHA1 != modDownload->getSHA1()) {
		spdlog::error("Failed to download '{}' mod package file '{}' due to data corruption, SHA1 hash check failed!", modGameVersion.getFullName(true), modDownload->getFileName());
		return false;
	}

	spdlog::debug("Mod '{}' package file '{}' file integrity verified!", modGameVersion.getFullName(true), modDownload->getFileName());

	if(standAlone) {
		spdlog::info("Mod '{}' package file saved to '{}' mod download directory.", modGameVersion.getFullName(true), gameVersion->getModDirectoryName());

		response->getBody()->writeTo(modPackageDownloadLocalFilePath, true);
	}

	std::unique_ptr<ZipArchive> modDownloadZipArchive(ZipArchive::createFrom(response->transferBody(), Utilities::emptyString, true));

	if(modDownloadZipArchive == nullptr) {
		spdlog::error("Failed to create zip archive handle from mod '{}' package file '{}'!", modGameVersion.getFullName(true), modDownload->getFileName());
		return false;
	}

	// verify mod file SHA1 hashes
	std::shared_ptr<ModFile> modZipFile(modGameVersion.getFirstFileOfType("zip"));

	if(modZipFile != nullptr && !standAlone) {
		std::weak_ptr<ArchiveEntry> modFileZipEntry(modDownloadZipArchive->getEntry(modZipFile->getFileName()));

		if(modFileZipEntry.expired()) {
			spdlog::error("Failed to download '{}' mod package file '{}', mod zip file '{}' not found!", modGameVersion.getFullName(true), modDownload->getFileName(), modZipFile->getFileName());
			return false;
		}

		std::unique_ptr<ByteBuffer> modFileZipEntryData(modFileZipEntry.lock()->getData());

		if(modFileZipEntryData == nullptr) {
			spdlog::error("Failed to download '{}' mod package file '{}', could not retrieve zip entry file '{}' data!", modGameVersion.getFullName(true), modDownload->getFileName(), modZipFile->getFileName());
			return false;
		}

		if(!Utilities::areStringsEqualIgnoreCase(modFileZipEntryData->getSHA1(), modZipFile->getSHA1())) {
			spdlog::error("Failed to download '{}' mod package file '{}', zip entry file '{}' SHA1 hash validation failed!", modGameVersion.getFullName(true), modDownload->getFileName(), modZipFile->getFileName());
			return false;
		}
	}
	else {
		std::shared_ptr<ModFile> modFile;
		std::weak_ptr<ArchiveEntry> modFileEntry;

		for(size_t i = 0; i < modGameVersion.numberOfFiles(); i++) {
			modFile = modGameVersion.getFile(i);

			// eDuke32 mod files can be read straight out of the group or zip file, and are not stored separately
			if(gameVersion->areZipArchiveGroupsSupported() && modFile->getType() != "zip") {
				continue;
			}

			// 0 size files don't really have a hash, skip them or else checks will fail
			if(modFile->getFileSize() == 0) {
				continue;
			}

			// zero size files don't really have a hash, so skip them or else checks will fail
			if(modFile->getFileSize() == 0) {
				continue;
			}

			modFileEntry = modDownloadZipArchive->getEntry(modFile->getFileName());

			if(modFileEntry.expired()) {
				spdlog::error("Failed to download '{}' mod package file '{}', mod file '{}' not found!", modGameVersion.getFullName(true), modDownload->getFileName(), modFile->getFileName());
				return false;
			}

			std::unique_ptr<ByteBuffer> modFileEntryData(modFileEntry.lock()->getData());

			if(modFileEntryData == nullptr) {
				spdlog::error("Failed to download '{}' mod package file '{}', could not retrieve zip entry file '{}' data!", modGameVersion.getFullName(true), modDownload->getFileName(), modFile->getFileName());
				return false;
			}
			else if(!Utilities::areStringsEqualIgnoreCase(modFileEntryData->getSHA1(), modFile->getSHA1())) {
				spdlog::error("Failed to download '{}' mod package file '{}', zip entry file '{}' SHA1 hash validation failed!", modGameVersion.getFullName(true), modDownload->getFileName(), modFile->getFileName());
				return false;
			}
		}
	}

	if(!standAlone && !modDownloadZipArchive->extractAllEntries(modDownloadLocalBasePath, true)) {
		spdlog::error("Failed to extract '{}' mod package file '{}' contents to directory: '{}'.", modGameVersion.getFullName(true), modDownload->getFileName(), modDownloadLocalBasePath);
		return false;
	}

	m_downloadCache->updateCachedPackageFile(*modDownload, modGameVersion, *gameVersion, modDownloadZipArchive->getCompressedSize(), response->getETag());

	size_t numberOfFiles = modDownloadZipArchive->numberOfFiles();

	modDownloadZipArchive.reset();

	saveDownloadCache();

	if(settings->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["modID"] = modGameVersion.getParentMod()->getID();
		properties["modName"] = modGameVersion.getParentMod()->getName();
		properties["modVersion"] = modGameVersion.getParentModVersion()->getVersion();
		properties["modVersionType"] = modGameVersion.getParentModVersionType()->getType();
		properties["fullModName"] = modGameVersion.getFullName(false);
		properties["fileName"] = modDownload->getFileName();
		properties["fileSize"] = response->getBody()->getSize();
		properties["numberOfFiles"] = numberOfFiles;
		properties["sha1"] = modPackageFileSHA1;
		properties["eTag"] = response->getETag();
		properties["transferDurationMs"] = response->getRequestDuration().value().count();
		properties["standAlone"] = standAlone;

		if(!standAlone) {
			properties["modGameVersion"] = modGameVersion.getGameVersionID();
		}

		SegmentAnalytics::getInstance()->track("Mod Downloaded", properties);
	}

	for(const std::shared_ptr<ModGameVersion> & dependencyModGameVersion : modDependencyGameVersions) {
		if(!downloadModGameVersion(*dependencyModGameVersion, mods, gameVersions, true, allowCompatibleGameVersions, force)) {
			return false;
		}
	}

	return true;
}

bool DownloadManager::uninstallModGameVersion(const ModGameVersion & modGameVersion, const GameVersionCollection & gameVersions) {
	if(!m_initialized) {
		return false;
	}

	if(!modGameVersion.isValid()) {
		spdlog::error("Failed to download mod, invalid mod game version provided!");
		return false;
	}

	SettingsManager * settings = SettingsManager::getInstance();
	HTTPService * httpService = HTTPService::getInstance();

	bool standAlone = modGameVersion.isStandAlone();
	std::shared_ptr<GameVersion> gameVersion;

	if(standAlone) {
		gameVersion = modGameVersion.getStandAloneGameVersion();
	}
	else {
		if(!gameVersions.isValid()) {
			spdlog::error("Failed to download mod, invalid game version collection provided!");
			return false;
		}

		gameVersion = gameVersions.getGameVersionWithID(modGameVersion.getGameVersionID());
	}

	if(gameVersion == nullptr) {
		spdlog::error("Failed to download '{}' mod, could not find game version with ID '{}'! Is your game version configuration file missing information?", modGameVersion.getParentModVersionType()->getFullName(), modGameVersion.getGameVersionID());
		return false;
	}

	std::shared_ptr<ModDownload> modDownload(modGameVersion.getDownload());

	if(modDownload == nullptr) {
		spdlog::error("Failed to obtain download for mod game version: '{}'. Is your mod collection data correct?", modGameVersion.getFullName(true));
		return false;
	}

	std::shared_ptr<CachedPackageFile> cachedModPackageDownload(m_downloadCache->getCachedPackageFile(*modDownload));

	if(cachedModPackageDownload == nullptr) {
		spdlog::warn("No cached mod download found for '{}'.", modGameVersion.getFullName(true));
		return false;
	}

	std::string modDownloadLocalBasePath(Utilities::joinPaths(settings->downloadsDirectoryPath, settings->modDownloadsDirectoryName, gameVersion->getModDirectoryName()));
	std::string modPackageDownloadLocalFilePath(Utilities::joinPaths(modDownloadLocalBasePath, modDownload->getFileName()));

	if(standAlone) {
		if(std::filesystem::is_regular_file(std::filesystem::path(modPackageDownloadLocalFilePath))) {
			spdlog::info("Deleting stand-alone '{}' mod archive package file: '{}'...", modGameVersion.getFullName(false), modPackageDownloadLocalFilePath);

			std::error_code errorCode;
			std::filesystem::remove(std::filesystem::path(modPackageDownloadLocalFilePath), errorCode);

			if(errorCode) {
				spdlog::error("Failed to delete '{}' mod package file download '{}' with error: {}", modGameVersion.getFullName(false), modPackageDownloadLocalFilePath, errorCode.message());
				return false;
			}
		}
		else {
			spdlog::warn("Failed to delete '{}' mod package file '{}' because it no longer exists.", modGameVersion.getFullName(true), modPackageDownloadLocalFilePath);
		}
	}
	else {
		std::vector<std::shared_ptr<CachedFile>> cachedModFiles(cachedModPackageDownload->getCachedFiles());

		for(const std::shared_ptr<CachedFile> & cachedModFile : cachedModFiles) {
			std::string cachedModFilePath(Utilities::joinPaths(modDownloadLocalBasePath, cachedModFile->getFileName()));

			if(std::filesystem::is_regular_file(std::filesystem::path(cachedModFilePath))) {
				spdlog::info("Deleting extracted '{}' mod file: '{}'...", modGameVersion.getFullName(true), cachedModFilePath);

				std::error_code errorCode;
				std::filesystem::remove(std::filesystem::path(cachedModFilePath), errorCode);

				if(errorCode) {
					spdlog::error("Failed to delete '{}' mod file download '{}' with error: {}", modGameVersion.getFullName(true), cachedModFilePath, errorCode.message());
					return false;
				}
			}
			else {
				spdlog::warn("Failed to delete '{}' mod file '{}' because it no longer exists.", modGameVersion.getFullName(true), cachedModFilePath);
			}
		}
	}

	m_downloadCache->removeCachedPackageFile(*cachedModPackageDownload);

	saveDownloadCache();

	spdlog::info("Successfully uninstalled '{}' mod download '{}' files.", modGameVersion.getFullName(true), modDownload->getFileName());

	if(settings->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["modID"] = modGameVersion.getParentMod()->getID();
		properties["modName"] = modGameVersion.getParentMod()->getName();
		properties["modVersion"] = modGameVersion.getParentModVersion()->getVersion();
		properties["modVersionType"] = modGameVersion.getParentModVersionType()->getType();
		properties["fullModName"] = modGameVersion.getFullName(false);
		properties["fileName"] = modDownload->getFileName();
		properties["fileSize"] = cachedModPackageDownload->getFileSize();
		properties["numberOfFiles"] = cachedModPackageDownload->numberOfCachedFiles();
		properties["sha1"] = cachedModPackageDownload->getSHA1();
		properties["eTag"] = cachedModPackageDownload->getETag();

		if(!standAlone) {
			properties["modGameVersion"] = modGameVersion.getGameVersionID();
		}

		SegmentAnalytics::getInstance()->track("Mod Uninstalled", properties);
	}

	return true;
}
