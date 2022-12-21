#ifndef _SETTINGS_MANAGER_H_
#define _SETTINGS_MANAGER_H_

#include "Game/GameType.h"

#include <Singleton/Singleton.h>

#include <rapidjson/document.h>

#include <chrono>
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
	bool save(const ArgumentParser * arguments = nullptr, bool overwrite = true) const;
	bool loadFrom(const std::string & filePath, bool autoCreate = true);
	bool saveTo(const std::string & filePath, bool overwrite = true) const;

	static const char * FILE_FORMAT_VERSION;
	static const char * DEFAULT_SETTINGS_FILE_PATH;
	static const char * DEFAULT_MODS_LIST_FILE_PATH;
	static const char * DEFAULT_FAVOURITE_MODS_LIST_FILE_PATH;
	static const char * DEFAULT_GAME_VERSIONS_LIST_FILE_PATH;
	static const char * DEFAULT_GAME_SYMLINK_NAME;
	static const bool DEFAULT_LOCAL_MODE;
	static const char * DEFAULT_MODS_DIRECTORY_PATH;
	static const char * DEFAULT_MODS_SYMLINK_NAME;
	static const char * DEFAULT_MOD_PACKAGE_DOWNLOADS_DIRECTORY_PATH;
	static const char * DEFAULT_MOD_IMAGES_DIRECTORY_PATH;
	static const char * DEFAULT_MOD_SOURCE_FILES_DIRECTORY_PATH;
	static const char * DEFAULT_MAPS_DIRECTORY_PATH;
	static const char * DEFAULT_MAPS_SYMLINK_NAME;
	static const char * DEFAULT_DOWNLOADS_DIRECTORY_PATH;
	static const char * DEFAULT_DOWNLOAD_CACHE_FILE_NAME;
	static const char * DEFAULT_MOD_DOWNLOADS_DIRECTORY_NAME;
	static const char * DEFAULT_MAP_DOWNLOADS_DIRECTORY_NAME;
	static const char * DEFAULT_GAME_DOWNLOADS_DIRECTORY_NAME;
	static const char * DEFAULT_DATA_DIRECTORY_PATH;
	static const char * DEFAULT_APP_TEMP_DIRECTORY_PATH;
	static const char * DEFAULT_GAME_TEMP_DIRECTORY_NAME;
	static const char * DEFAULT_TEMP_SYMLINK_NAME;
	static const char * DEFAULT_CACHE_DIRECTORY_PATH;
	static const char * DEFAULT_DOSBOX_EXECUTABLE_FILE_NAME;
	static const char * DEFAULT_DOSBOX_DIRECTORY_PATH;
	static const char * DEFAULT_DOSBOX_ARGUMENTS;
	static const char * DEFAULT_DOSBOX_DATA_DIRECTORY_NAME;
	static const GameType DEFAULT_GAME_TYPE;
	static const std::string DEFAULT_PREFERRED_GAME_VERSION;
	static const char * DEFAULT_DOSBOX_SERVER_IP_ADDRESS;
	static const uint16_t DEFAULT_DOSBOX_LOCAL_SERVER_PORT;
	static const uint16_t DEFAULT_DOSBOX_REMOTE_SERVER_PORT;
	static const char * DEFAULT_TIME_ZONE_DATA_DIRECTORY_NAME;
	static const char * DEFAULT_CURL_DATA_DIRECTORY_NAME;
	static const std::chrono::seconds DEFAULT_CONNECTION_TIMEOUT;
	static const std::chrono::seconds DEFAULT_NETWORK_TIMEOUT;
	static const char * DEFAULT_API_BASE_URL;
	static const char * DEFAULT_REMOTE_MODS_LIST_FILE_NAME;
	static const char * DEFAULT_REMOTE_GAMES_LIST_FILE_NAME;
	static const char * DEFAULT_REMOTE_DOWNLOADS_DIRECTORY_NAME;
	static const char * DEFAULT_REMOTE_MOD_DOWNLOADS_DIRECTORY_NAME;
	static const char * DEFAULT_REMOTE_MAP_DOWNLOADS_DIRECTORY_NAME;
	static const char * DEFAULT_REMOTE_GAME_DOWNLOADS_DIRECTORY_NAME;
	static const bool DEFAULT_SEGMENT_ANALYTICS_ENABLED;
	static const char * DEFAULT_SEGMENT_ANALYTICS_DATA_FILE_NAME;

	std::string modsListFilePath;
	std::string favouriteModsListFilePath;
	std::string gameVersionsListFilePath;
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
	std::string gameDownloadsDirectoryName;
	std::string dataDirectoryPath;
	std::string appTempDirectoryPath;
	std::string gameTempDirectoryName;
	std::string tempSymlinkName;
	std::string cacheDirectoryPath;
	std::string dosboxExecutableFileName;
	std::string dosboxDirectoryPath;
	std::string dosboxArguments;
	std::string dosboxDataDirectoryName;
	GameType gameType;
	std::string preferredGameVersion;
	std::string dosboxServerIPAddress;
	uint16_t dosboxLocalServerPort;
	uint16_t dosboxRemoteServerPort;
	std::string timeZoneDataDirectoryName;
	std::string curlDataDirectoryName;
	std::chrono::seconds connectionTimeout;
	std::chrono::seconds networkTimeout;
	std::string apiBaseURL;
	std::string remoteModsListFileName;
	std::string remoteGamesListFileName;
	std::string remoteDownloadsDirectoryName;
	std::string remoteModDownloadsDirectoryName;
	std::string remoteMapDownloadsDirectoryName;
	std::string remoteGameDownloadsDirectoryName;
	bool segmentAnalyticsEnabled;
	std::string segmentAnalyticsDataFileName;
	std::map<std::string, std::string> fileETags;

private:
	SettingsManager(const SettingsManager &) = delete;
	SettingsManager(SettingsManager &&) noexcept = delete;
	const SettingsManager & operator = (const SettingsManager &) = delete;
	const SettingsManager & operator = (SettingsManager &&) noexcept = delete;
};

#endif // _SETTINGS_MANAGER_H_
