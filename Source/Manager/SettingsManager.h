#ifndef _SETTINGS_MANAGER_H_
#define _SETTINGS_MANAGER_H_

#include "Game/GameType.h"

#include <Dimension.h>
#include <Point2D.h>
#include <Singleton/Singleton.h>

#include <rapidjson/document.h>

#include <chrono>
#include <optional>
#include <string>

class ArgumentParser;

class SettingsManager final : public Singleton<SettingsManager> {
public:
	SettingsManager();
	virtual ~SettingsManager();

	void reset();

	rapidjson::Document toJSON() const;
	bool parseFrom(const rapidjson::Value & settingsDocument);

	bool load(const ArgumentParser * arguments = nullptr, bool autoCreate = true);
	bool save(bool overwrite = true) const;
	bool loadFrom(const std::string & filePath, bool autoCreate = true);
	bool saveTo(const std::string & filePath, bool overwrite = true) const;

	static const std::string FILE_TYPE;
	static const std::string FILE_FORMAT_VERSION;
	static const std::string DEFAULT_SETTINGS_FILE_PATH;
	static const std::string DEFAULT_MODS_LIST_FILE_PATH;
	static const std::string DEFAULT_STANDALONE_MODS_LIST_FILE_PATH;
	static const std::string DEFAULT_FAVOURITE_MODS_LIST_FILE_PATH;
	static const std::string DEFAULT_GAME_VERSIONS_LIST_FILE_PATH;
	static const std::string DEFAULT_GAME_DOWNLOADS_LIST_FILE_PATH;
	static const std::string DEFAULT_GAME_SYMLINK_NAME;
	static const bool DEFAULT_LOCAL_MODE;
	static const std::string DEFAULT_MODS_DIRECTORY_PATH;
	static const std::string DEFAULT_MODS_SYMLINK_NAME;
	static const std::string DEFAULT_MOD_PACKAGE_DOWNLOADS_DIRECTORY_PATH;
	static const std::string DEFAULT_MOD_IMAGES_DIRECTORY_PATH;
	static const std::string DEFAULT_MOD_SOURCE_FILES_DIRECTORY_PATH;
	static const std::string DEFAULT_MAPS_DIRECTORY_PATH;
	static const std::string DEFAULT_MAPS_SYMLINK_NAME;
	static const std::string DEFAULT_DOWNLOADS_DIRECTORY_PATH;
	static const std::string DEFAULT_DOWNLOAD_CACHE_FILE_NAME;
	static const std::string DEFAULT_MOD_DOWNLOADS_DIRECTORY_NAME;
	static const std::string DEFAULT_MAP_DOWNLOADS_DIRECTORY_NAME;
	static const std::string DEFAULT_DOSBOX_DOWNLOADS_DIRECTORY_NAME;
	static const std::string DEFAULT_GAME_DOWNLOADS_DIRECTORY_NAME;
	static const std::string DEFAULT_GROUP_DOWNLOADS_DIRECTORY_NAME;
	static const std::string DEFAULT_DATA_DIRECTORY_PATH;
	static const std::string DEFAULT_APP_TEMP_DIRECTORY_PATH;
	static const std::string DEFAULT_GAME_TEMP_DIRECTORY_NAME;
	static const std::string DEFAULT_TEMP_SYMLINK_NAME;
	static const std::string DEFAULT_CACHE_DIRECTORY_PATH;
	static const std::string DEFAULT_DOSBOX_ARGUMENTS;
	static const bool DEFAULT_DOSBOX_SHOW_CONSOLE;
	static const bool DEFAULT_DOSBOX_AUTO_EXIT;
	static const std::string DEFAULT_DOSBOX_DATA_DIRECTORY_NAME;
	static const GameType DEFAULT_GAME_TYPE;
	static const std::string DEFAULT_PREFERRED_GAME_VERSION_ID;
	static const std::string DEFAULT_DOSBOX_VERSIONS_LIST_FILE_PATH;
	static const std::string DEFAULT_DOSBOX_DOWNLOADS_LIST_FILE_PATH;
	static const std::string DEFAULT_PREFERRED_DOSBOX_VERSION_ID;
	static const std::string DEFAULT_DOSBOX_SERVER_IP_ADDRESS;
	static const uint16_t DEFAULT_DOSBOX_LOCAL_SERVER_PORT;
	static const uint16_t DEFAULT_DOSBOX_REMOTE_SERVER_PORT;
	static const std::string DEFAULT_TIME_ZONE_DATA_DIRECTORY_NAME;
	static const std::string DEFAULT_CURL_DATA_DIRECTORY_NAME;
	static const std::chrono::seconds DEFAULT_CONNECTION_TIMEOUT;
	static const std::chrono::seconds DEFAULT_NETWORK_TIMEOUT;
	static const std::chrono::seconds DEFAULT_TRANSFER_TIMEOUT;
	static const bool DEFAULT_VERBOSE_REQUEST_LOGGING;
	static const std::string DEFAULT_API_BASE_URL;
	static const std::string DEFAULT_REMOTE_MODS_LIST_FILE_NAME;
	static const std::string DEFAULT_REMOTE_DOSBOX_VERSIONS_LIST_FILE_NAME;
	static const std::string DEFAULT_REMOTE_GAMES_LIST_FILE_NAME;
	static const std::string DEFAULT_REMOTE_DOWNLOADS_DIRECTORY_NAME;
	static const std::string DEFAULT_REMOTE_MOD_DOWNLOADS_DIRECTORY_NAME;
	static const std::string DEFAULT_REMOTE_MAP_DOWNLOADS_DIRECTORY_NAME;
	static const std::string DEFAULT_REMOTE_DOSBOX_DOWNLOADS_DIRECTORY_NAME;
	static const std::string DEFAULT_REMOTE_GAME_DOWNLOADS_DIRECTORY_NAME;
	static const bool DEFAULT_SEGMENT_ANALYTICS_ENABLED;
	static const std::string DEFAULT_SEGMENT_ANALYTICS_DATA_FILE_NAME;
	static const bool DEFAULT_DOWNLOAD_THROTTLING_ENABLED;
	static const std::chrono::minutes DEFAULT_MOD_LIST_UPDATE_FREQUENCY;
	static const std::chrono::minutes DEFAULT_DOSBOX_DOWNLOAD_LIST_UPDATE_FREQUENCY;
	static const std::chrono::minutes DEFAULT_GAME_DOWNLOAD_LIST_UPDATE_FREQUENCY;
	static const std::chrono::minutes DEFAULT_CACERT_UPDATE_FREQUENCY;
	static const std::chrono::minutes DEFAULT_TIME_ZONE_DATA_UPDATE_FREQUENCY;
	static const Point2D DEFAULT_WINDOW_POSITION;
	static const Dimension DEFAULT_WINDOW_SIZE;
	static const Dimension MINIMUM_WINDOW_SIZE;

	std::string modsListFilePath;
	std::string standAloneModsListFilePath;
	std::string favouriteModsListFilePath;
	std::string gameVersionsListFilePath;
	std::string gameDownloadsListFilePath;
	std::string gameSymlinkName;
	bool localMode;
	std::string modsDirectoryPath;
	std::string modsSymlinkName;
	std::string modPackageDownloadsDirectoryPath;
	std::string modImagesDirectoryPath;
	std::string modSourceFilesDirectoryPath;
	std::string mapsDirectoryPath;
	std::string mapsSymlinkName;
	std::string downloadsDirectoryPath;
	std::string downloadCacheFileName;
	std::string modDownloadsDirectoryName;
	std::string mapDownloadsDirectoryName;
	std::string dosboxDownloadsDirectoryName;
	std::string gameDownloadsDirectoryName;
	std::string groupDownloadsDirectoryName;
	std::string dataDirectoryPath;
	std::string appTempDirectoryPath;
	std::string gameTempDirectoryName;
	std::string tempSymlinkName;
	std::string cacheDirectoryPath;
	std::string dosboxArguments;
	bool dosboxShowConsole;
	bool dosboxAutoExit;
	std::string dosboxDataDirectoryName;
	GameType gameType;
	std::string preferredGameVersionID;
	std::string dosboxVersionsListFilePath;
	std::string dosboxDownloadsListFilePath;
	std::string preferredDOSBoxVersionID;
	std::string dosboxServerIPAddress;
	uint16_t dosboxLocalServerPort;
	uint16_t dosboxRemoteServerPort;
	std::string timeZoneDataDirectoryName;
	std::string curlDataDirectoryName;
	std::chrono::seconds connectionTimeout;
	std::chrono::seconds networkTimeout;
	std::chrono::seconds transferTimeout;
	bool verboseRequestLogging;
	std::string apiBaseURL;
	std::string remoteModsListFileName;
	std::string remoteDOSBoxVersionsListFileName;
	std::string remoteGamesListFileName;
	std::string remoteDownloadsDirectoryName;
	std::string remoteModDownloadsDirectoryName;
	std::string remoteMapDownloadsDirectoryName;
	std::string remoteDOSBoxDownloadsDirectoryName;
	std::string remoteGameDownloadsDirectoryName;
	bool segmentAnalyticsEnabled;
	std::string segmentAnalyticsDataFileName;
	bool downloadThrottlingEnabled;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> modListLastDownloadedTimestamp;
	std::chrono::minutes modListUpdateFrequency;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> dosboxDownloadListLastDownloadedTimestamp;
	std::chrono::minutes dosboxDownloadListUpdateFrequency;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> gameDownloadListLastDownloadedTimestamp;
	std::chrono::minutes gameDownloadListUpdateFrequency;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> cacertLastDownloadedTimestamp;
	std::chrono::minutes cacertUpdateFrequency;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> timeZoneDataLastDownloadedTimestamp;
	std::chrono::minutes timeZoneDataUpdateFrequency;
	Point2D windowPosition;
	Dimension windowSize;

	std::map<std::string, std::string> fileETags;

private:
	SettingsManager(const SettingsManager &) = delete;
	const SettingsManager & operator = (const SettingsManager &) = delete;

	std::string m_filePath;
};

#endif // _SETTINGS_MANAGER_H_
