#include "GameAddonDiscImageDownloader.h"

#include "Manager/SettingsManager.h"

#include <Analytics/Segment/SegmentAnalytics.h>
#include <Archive/ArchiveFactoryRegistry.h>
#include <Network/HTTPService.h>
#include <Utilities/CDIOUtilities.h>
#include <Utilities/FileUtilities.h>

#include <filesystem>
#include <vector>

GameAddonDiscImageDownloader::GameAddonDiscImageDownloader() { }

std::string GameAddonDiscImageDownloader::getGameAddonDownloadURL(GameAddon gameAddon) {
	switch(gameAddon) {
		case GameAddon::DukeCaribbeanLifesABeach:
			// https://archive.org/details/dukecaribbeanlifesabeachusa
			return "https://archive.org/download/dukecaribbeanlifesabeachusa/Duke%20Caribbean%20-%20Life%27s%20a%20Beach%20%28USA%29.zip";
		case GameAddon::DukeItOutInDC:
			// https://archive.org/details/dukeitoutindcusa
			return "https://archive.org/download/dukeitoutindcusa/Duke%20It%20Out%20in%20D.C.%20%28USA%29.zip";
		case GameAddon::NuclearWinter:
			// https://archive.org/details/dukenuclearwinterusa
			return "https://archive.org/download/dukenuclearwinterusa/Duke%20-%20Nuclear%20Winter%20%28USA%29.zip";
	}

	return "";
}

std::unique_ptr<Archive> GameAddonDiscImageDownloader::downloadGameAddonArchive(GameAddon gameAddon) {
	SettingsManager * settings = SettingsManager::getInstance();
	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->isInitialized()) {
		return nullptr;
	}

	std::string gameAddonDiscImageArchiveDownloadURL(getGameAddonDownloadURL(gameAddon));

	if(gameAddonDiscImageArchiveDownloadURL.empty()) {
		spdlog::error("No game addon URL available for '{}' game addon.", getGameAddonName(gameAddon));
		return nullptr;
	}

	// TODO: signal status change?

	spdlog::info("Downloading '{}' game addon disc image archive from: '{}'...", getGameAddonName(gameAddon), gameAddonDiscImageArchiveDownloadURL);

	std::shared_ptr<HTTPRequest> request(httpService->createRequest(HTTPRequest::Method::Get, gameAddonDiscImageArchiveDownloadURL));

	std::shared_ptr<HTTPResponse> response(httpService->sendRequestAndWait(request));

	if(response == nullptr || response->isFailure()) {
		spdlog::error("Failed to download '{}' game addon disc image archive with error: {}", getGameAddonName(gameAddon), response != nullptr ? response->getErrorMessage() : "Invalid request.");
		return nullptr;
	}
	else if(response->isFailureStatusCode()) {
		std::string statusCodeName(HTTPUtilities::getStatusCodeName(response->getStatusCode()));
		spdlog::error("Failed to download '{}' game addon disc image archive ({}{})!", getGameAddonName(gameAddon), response->getStatusCode(), statusCodeName.empty() ? "" : " " + statusCodeName);
		return nullptr;
	}

	spdlog::info("'{}' game addon disc image archive downloaded successfully after {} ms.", getGameAddonName(gameAddon), response->getRequestDuration().value().count());

	std::unique_ptr<Archive> gameAddonDiscImageArchive(ArchiveFactoryRegistry::getInstance()->createArchiveFrom(response->transferBody()));

	if(gameAddonDiscImageArchive == nullptr) {
		spdlog::error("Failed to create archive handle from downloaded '{}' game addon disc image archive file.", getGameAddonName(gameAddon));
		return nullptr;
	}

	if(settings->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["gameAddonID"] = getGameAddonID(gameAddon);
		properties["gameAddonName"] = getGameAddonName(gameAddon);
		properties["fileName"] = Utilities::getFileName(gameAddonDiscImageArchiveDownloadURL);
		properties["fileSize"] = response->getBody()->getSize();
		properties["eTag"] = response->getETag();
		properties["transferDurationMs"] = response->getRequestDuration().value().count();

		SegmentAnalytics::getInstance()->track("Game Addon Disc Image Archive Downloaded", properties);
	}

	return gameAddonDiscImageArchive;
}

std::pair<std::unique_ptr<ISO9660::FS>, std::string> GameAddonDiscImageDownloader::downloadGameAddonDiscImage(GameAddon gameAddon) {
	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->appTempDirectoryPath.empty()) {
		spdlog::error("Missing application temporary directory path setting.");
		return {};
	}

	std::unique_ptr<Archive> gameAddonDiscImageArchive(downloadGameAddonArchive(gameAddon));

	if(gameAddonDiscImageArchive == nullptr) {
		spdlog::error("Failed to obtain '{}' game addon disc image archive.", getGameAddonName(gameAddon));
		return {};
	}

	std::string gameAddonDiscImageTempDirectoryPath(Utilities::joinPaths(settings->appTempDirectoryPath, getGameAddonID(gameAddon)));

	std::vector<std::shared_ptr<ArchiveEntry>> gameAddonDiscImageFiles(gameAddonDiscImageArchive->getEntriesWithExtensions({ "ISO", "CUE", "MDS", "IMG", "NRG" }));

	if(gameAddonDiscImageFiles.empty()) {
		spdlog::warn("No '{}' game addon disc images found inside of archive.", getGameAddonName(gameAddon));
		return {};
	}
	else if(gameAddonDiscImageFiles.size() != 1) {
		spdlog::warn("Found multiple '{}' game addon disc images inside of archive, using first one.", getGameAddonName(gameAddon));
	}

	std::string gameAddonDiscImageFilePath(Utilities::joinPaths(gameAddonDiscImageTempDirectoryPath, gameAddonDiscImageFiles.front()->getPath()));

	if(!std::filesystem::is_directory(std::filesystem::path(gameAddonDiscImageTempDirectoryPath))) {
		std::error_code errorCode;
		std::filesystem::create_directories(std::filesystem::path(gameAddonDiscImageTempDirectoryPath), errorCode);

		if(errorCode) {
			spdlog::error("Failed to create '{}' game addon disc image temporary directory structure '{}' with error: {}.", getGameAddonName(gameAddon), gameAddonDiscImageTempDirectoryPath, errorCode.message());
			return {};
		}
		else {
			spdlog::info("Created '{}' game addon disc image temporary directory structure: '{}'.", getGameAddonName(gameAddon), gameAddonDiscImageTempDirectoryPath);
		}
	}

	if(gameAddonDiscImageArchive->extractAllEntries(gameAddonDiscImageTempDirectoryPath) == 0) {
		spdlog::error("Failed to extract all game addon disc image archive entries.");
		return {};
	}

	gameAddonDiscImageFiles.clear();
	gameAddonDiscImageArchive.reset();

	std::unique_ptr<ISO9660::FS> gameAddonDiscImage(CDIOUtilities::readDiscImage(gameAddonDiscImageFilePath));

	if(gameAddonDiscImage == nullptr) {
		// TODO: error
		return {};
	}

	// TODO: stuff - temp files on file system, need to be removed..
	
	std::error_code errorCode;
	std::filesystem::remove(std::filesystem::path(gameAddonDiscImageTempDirectoryPath), errorCode);

	if(errorCode) {
		spdlog::error("Failed to delete '{}' game addon disc image temporary directory '{}' with error: {}", getGameAddonName(gameAddon), gameAddonDiscImageTempDirectoryPath, errorCode.message());
	}

	// TODO:
	return std::make_pair(std::move(gameAddonDiscImage), gameAddonDiscImageTempDirectoryPath);
}
