#include "DownloadManager.h"

#include "CachedFile.h"
#include "CachedPackageFile.h"
#include "DownloadCache.h"
#include "Game/GameVersion.h"
#include "Game/GameVersionCollection.h"
#include "Manager/SettingsManager.h"
#include "Mod/ModDownload.h"
#include "Mod/ModFile.h"
#include "Mod/ModGameVersion.h"
#include "Mod/ModVersionType.h"

#include <Network/HTTPService.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Zip/ZipArchive.h>

#include <fmt/core.h>
#include <magic_enum.hpp>

#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>

using namespace std::chrono_literals;

DownloadManager::DownloadManager(std::shared_ptr<HTTPService> httpService, std::shared_ptr<SettingsManager> settings)
	: m_initialized(false)
	, m_httpService(httpService)
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

	if(m_httpService == nullptr) {
		fmt::print("Failed to initialize download manager, invalid HTTP service provided.\n");
		return false;
	}

	if(m_settings == nullptr) {
		fmt::print("Failed to initialize download manager, invalid settings manager provided.\n");
		return false;
	}

	if(!ensureRequiredDirectories()) {
		fmt::print("Failed to create required directories!\n");
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

std::string DownloadManager::getDownloadCacheFilePath() const {
	if(m_settings == nullptr) {
		return {};
	}

	if(m_settings->downloadsDirectoryPath.empty()) {
		fmt::print("Missing downloads directory path setting.\n");
		return {};
	}

	if(m_settings->downloadCacheFileName.empty()) {
		fmt::print("Missing download cache file name setting.\n");
		return {};
	}

	return Utilities::joinPaths(m_settings->downloadsDirectoryPath, m_settings->downloadCacheFileName);
}

std::string DownloadManager::getCachedModListFilePath() const {
	if(m_settings == nullptr) {
		return {};
	}

	if(m_settings->downloadsDirectoryPath.empty()) {
		fmt::print("Missing downloads directory path setting.\n");
		return {};
	}

	if(m_settings->modDownloadsDirectoryName.empty()) {
		fmt::print("Missing mod downloads directory name setting.\n");
		return {};
	}

	if(m_settings->remoteModsListFileName.empty()) {
		fmt::print("Missing remote mods list file name setting.\n");
		return {};
	}

	return Utilities::joinPaths(m_settings->downloadsDirectoryPath, m_settings->modDownloadsDirectoryName, m_settings->remoteModsListFileName);
}

std::string DownloadManager::getDownloadedModsDirectoryPath() const {
	if(m_settings == nullptr) {
		return {};
	}

	if(m_settings->downloadsDirectoryPath.empty()) {
		fmt::print("Missing downloads directory path setting.\n");
		return {};
	}

	if(m_settings->modDownloadsDirectoryName.empty()) {
		fmt::print("Missing mod downloads directory name setting.\n");
		return {};
	}

	return Utilities::joinPaths(m_settings->downloadsDirectoryPath, m_settings->modDownloadsDirectoryName);
}

std::string DownloadManager::getDownloadedMapsDirectoryPath() const {
	if(m_settings == nullptr) {
		return {};
	}

	if(m_settings->downloadsDirectoryPath.empty()) {
		fmt::print("Missing downloads directory path setting.\n");
		return {};
	}

	if(m_settings->mapDownloadsDirectoryName.empty()) {
		fmt::print("Missing map downloads directory name setting.\n");
		return {};
	}

	return Utilities::joinPaths(m_settings->downloadsDirectoryPath, m_settings->mapDownloadsDirectoryName);
}

bool DownloadManager::ensureRequiredDirectories() {
	if(m_settings == nullptr) {
		return false;
	}

	if(m_settings->downloadsDirectoryPath.empty()) {
		fmt::print("Missing downloads directory path setting.\n");
		return false;
	}

	std::error_code errorCode;
	std::filesystem::path downloadsDirectoryPath(m_settings->downloadsDirectoryPath);

	if(!std::filesystem::is_directory(downloadsDirectoryPath)) {
		std::filesystem::create_directories(downloadsDirectoryPath, errorCode);

		if(errorCode) {
			fmt::print("Failed to create downloads directory '{}': {}\n", downloadsDirectoryPath.string(), errorCode.message());
			return false;
		}

#if _DEBUG
			fmt::print("Created downloads directory: '{}'.\n", downloadsDirectoryPath.string());
#endif _DEBUG
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
				fmt::print("Failed to create downloads sub-directory '{}': {}\n", subdirectory.string(), errorCode.message());
				return false;
			}

#if _DEBUG
			fmt::print("Created downloads sub-directory: '{}'.\n", subdirectory.string());
#endif _DEBUG
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
	if(m_httpService == nullptr || m_settings == nullptr) {
		return false;
	}

	std::shared_ptr<CachedFile> cachedModListFile(m_downloadCache->getCachedModListFile());

	std::string modListFileName(m_settings->remoteModsListFileName);
	std::string modListLocalFilePath(Utilities::joinPaths(getDownloadedModsDirectoryPath(), modListFileName));
	std::string modListRemoteFilePath(Utilities::joinPaths(m_settings->remoteDownloadsDirectoryName, m_settings->remoteModDownloadsDirectoryName, modListFileName));
	std::string modListURL(Utilities::joinPaths(m_httpService->getBaseURL(), modListRemoteFilePath));

	fmt::print("Downloading Duke Nukem 3D mod list from: '{}'...\n", modListURL);

	std::shared_ptr<HTTPRequest> request(m_httpService->createRequest(HTTPRequest::Method::Get, modListRemoteFilePath));

	if(!force && cachedModListFile != nullptr) {
		request->setIfNoneMatchETag(cachedModListFile->getETag());
	}

	std::future<std::shared_ptr<HTTPResponse>> futureResponse(m_httpService->sendRequest(request));

	if(!futureResponse.valid()) {
		fmt::print("Failed to create Duke Nukem 3D mod list file HTTP request!\n");
		return false;
	}

	futureResponse.wait();

	std::shared_ptr<HTTPResponse> response(futureResponse.get());

	if(response->isFailure()) {
		fmt::print("Failed to download Duke Nukem 3D mod list with error: {}\n", response->getErrorMessage());
		return false;
	}

	if(response->getStatusCode() == magic_enum::enum_integer(HTTPStatusCode::NotModified)) {
		fmt::print("Duke Nukem 3D mod list is already up to date!\n");
		return true;
	}
	else if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		fmt::print("Failed to download Duke Nukem 3D mod list ({}{})!\n", response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return true;
	}

	fmt::print("Duke Nukem 3D mod list downloaded successfully after {} ms to file: '{}'.\n", response->getRequestDuration().value().count(), modListLocalFilePath);

	response->getBody()->writeTo(modListLocalFilePath, true);

	m_downloadCache->updateCachedModListFile(modListFileName, response->getBody()->getSize(), response->getBody()->getSHA1(), response->getETag());

	saveDownloadCache();

	return true;
}

bool DownloadManager::downloadModGameVersion(ModGameVersion * modGameVersion, GameVersionCollection * gameVersions, bool force) {
	if(!m_initialized) {
		return false;
	}

	if(!ModGameVersion::isValid(modGameVersion)) {
		fmt::print("Failed to download mod, invalid mod game version provided!\n");
		return false;
	}

	if(!GameVersionCollection::isValid(gameVersions)) {
		fmt::print("Failed to download mod, invalid game version collection provided!\n");
		return false;
	}

	std::shared_ptr<GameVersion> gameVersion(gameVersions->getGameVersion(modGameVersion->getGameVersion()));

	if(gameVersion == nullptr) {
		fmt::print("Failed to download '{}' mod, could not find '{}' game version! Is your game version configuration file missing information?\n", modGameVersion->getParentModVersionType()->getFullName(), modGameVersion->getGameVersion());
		return false;
	}

	std::shared_ptr<ModDownload> modDownload(modGameVersion->getDownload());

	if(modDownload == nullptr) {
		fmt::print("Failed to obtain download for mod game version: '{}'. Is your mod collection data correct?\n", modGameVersion->getFullName());
		return false;
	}

	std::shared_ptr<CachedPackageFile> cachedModPackageFile(m_downloadCache->getCachedPackageFile(modDownload.get()));

	std::string modDownloadLocalBasePath(Utilities::joinPaths(m_settings->downloadsDirectoryPath, m_settings->modDownloadsDirectoryName, gameVersion->getModDirectoryName()));
	std::string modDownloadLocalFilePath(Utilities::joinPaths(modDownloadLocalBasePath, modDownload->getFileName()));
	std::string modDownloadRemoteFilePath(Utilities::joinPaths(m_settings->remoteDownloadsDirectoryName, m_settings->remoteModDownloadsDirectoryName, modDownload->getSubfolder(), Utilities::toLowerCase(gameVersion->getModDirectoryName()), modDownload->getFileName()));
	std::string modDownloadURL(Utilities::joinPaths(m_httpService->getBaseURL(), modDownloadRemoteFilePath));

	fmt::print("Downloading '{}' mod from: '{}'...\n", modGameVersion->getFullName(), modDownloadURL);

	std::shared_ptr<HTTPRequest> request(m_httpService->createRequest(HTTPRequest::Method::Get, modDownloadRemoteFilePath));

	request->setNetworkTimeout(30min);

	if(!force && cachedModPackageFile != nullptr) {
		request->setIfNoneMatchETag(cachedModPackageFile->getETag());
	}

	std::future<std::shared_ptr<HTTPResponse>> futureResponse(m_httpService->sendRequest(request));

	if(!futureResponse.valid()) {
		fmt::print("Failed to create '{}' mod package file HTTP request!\n", modGameVersion->getFullName());
		return false;
	}

	futureResponse.wait();

	std::shared_ptr<HTTPResponse> response(futureResponse.get());

	if(response->isFailure()) {
		fmt::print("Failed to download '{}' mod package file with error: {}\n", modGameVersion->getFullName(), response->getErrorMessage());
		return false;
	}

	if(response->getStatusCode() == magic_enum::enum_integer(HTTPStatusCode::NotModified)) {
		fmt::print("Mod '{}' is already up to date!\n", modGameVersion->getFullName());
		return true;
	}
	else if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		fmt::print("Failed to download '{}' mod package file ({}{})!\n", modGameVersion->getFullName(), response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return true;
	}

	fmt::print("Successfully downloaded '{}' mod package file '{}' after {} ms, verifying file integrity using SHA1 hash...\n", modGameVersion->getFullName(), modDownload->getFileName(), response->getRequestDuration().value().count());

	if(response->getBody()->getSHA1() != modDownload->getSHA1()) {
		fmt::print("Failed to download '{}' mod package file '{}' due to data corruption, SHA1 hash check failed!\n", modGameVersion->getFullName(), modDownload->getFileName());
		return false;
	}

	fmt::print("Mod '{}' package file '{}' file integrity verified!\n", modGameVersion->getFullName(), modDownload->getFileName());

	std::unique_ptr<ZipArchive> modDownloadZipArchive(ZipArchive::createFrom(response->transferBody(), Utilities::emptyString, true));

	if(modDownloadZipArchive == nullptr) {
		fmt::print("Failed to create zip archive handle from mod '{}' package file '{}'!\n", modGameVersion->getFullName(), modDownload->getFileName());
		return false;
	}

	// verify mod file SHA1 hashes
	std::shared_ptr<ModFile> modZipFile(modGameVersion->getFirstFileOfType("zip"));

	if(modZipFile != nullptr) {
		std::weak_ptr<ZipArchive::Entry> modFileZipEntry(modDownloadZipArchive->getEntry(modZipFile->getFileName()));

		if(modFileZipEntry.expired()) {
			fmt::print("Failed to download '{}' mod package file '{}', mod zip file '{}' not found!\n", modGameVersion->getFullName(), modDownload->getFileName(), modZipFile->getFileName());
			return false;
		}

		std::unique_ptr<ByteBuffer> modFileZipEntryData(modFileZipEntry.lock()->getData());

		if(modFileZipEntryData == nullptr) {
			fmt::print("Failed to download '{}' mod package file '{}', could not retrieve zip entry file '{}' data!\n", modGameVersion->getFullName(), modDownload->getFileName(), modZipFile->getFileName());
			return false;
		}

		if(!Utilities::areStringsEqualIgnoreCase(modFileZipEntryData->getSHA1(), modZipFile->getSHA1())) {
			fmt::print("Failed to download '{}' mod package file '{}', zip entry file '{}' SHA1 hash validation failed!\n", modGameVersion->getFullName(), modDownload->getFileName(), modZipFile->getFileName());
			return false;
		}
	}
	else {
		std::shared_ptr<ModFile> modFile;
		std::weak_ptr<ZipArchive::Entry> modFileEntry;

		for(size_t i = 0; i < modGameVersion->numberOfFiles(); i++) {
			modFile = modGameVersion->getFile(i);
			modFileEntry = modDownloadZipArchive->getEntry(modFile->getFileName());

			if(modFileEntry.expired()) {
				fmt::print("Failed to download '{}' mod package file '{}', mod file '{}' not found!\n", modGameVersion->getFullName(), modDownload->getFileName(), modFile->getFileName());
				return false;
			}

			std::unique_ptr<ByteBuffer> modFileEntryData(modFileEntry.lock()->getData());

			if(modFileEntryData == nullptr) {
				fmt::print("Failed to download '{}' mod package file '{}', could not retrieve zip entry file '{}' data!\n", modGameVersion->getFullName(), modDownload->getFileName(), modFile->getFileName());
				return false;
			}

			if(!Utilities::areStringsEqualIgnoreCase(modFileEntryData->getSHA1(), modFile->getSHA1())) {
				fmt::print("Failed to download '{}' mod package file '{}', zip entry file '{}' SHA1 hash validation failed!\n", modGameVersion->getFullName(), modDownload->getFileName(), modFile->getFileName());
				return false;
			}
		}
	}

	if(!modDownloadZipArchive->extractAllEntries(modDownloadLocalBasePath, true)) {
		fmt::print("Failed to extract '{}' mod package file '{}' contents to directory: '{}'.\n", modGameVersion->getFullName(), modDownload->getFileName(), modDownloadLocalBasePath);
		return false;
	}

	m_downloadCache->updateCachedPackageFile(modDownload.get(), modGameVersion, modDownloadZipArchive->getCompressedSize(), response->getETag());

	saveDownloadCache();

	return true;
}
