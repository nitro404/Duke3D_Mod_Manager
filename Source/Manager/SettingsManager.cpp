#include "SettingsManager.h"

#include "Game/GameVersion.h"
#include "ModManager.h"

#include <Arguments/ArgumentParser.h>
#include <Utilities/StringUtilities.h>

#include <magic_enum.hpp>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <optional>
#include <string_view>

using namespace std::chrono_literals;

static constexpr const char * SYMLINK_NAME = "symlinkName";
static constexpr const char * LIST_FILE_PATH = "listFilePath";
static constexpr const char * DIRECTORY_PATH = "directoryPath";
static constexpr const char * DATA_DIRECTORY_NAME = "dataDirectoryName";
static constexpr const char * EXECUTABLE_FILE_NAME = "executableFileName";

static constexpr const char * FILE_FORMAT_VERSION_PROPERTY_NAME = "version";
static constexpr const char * GAME_TYPE_PROPERTY_NAME = "gameType";
static constexpr const char * DATA_DIRECTORY_PATH_PROPERTY_NAME = "dataDirectoryPath";
static constexpr const char * TEMP_DIRECTORY_PATH_PROPERTY_NAME = "tempDirectoryPath";
static constexpr const char * TEMP_SYMLINK_NAME_PROPERTY_NAME = "tempSymlinkName";
static constexpr const char * GAME_SYMLINK_NAME_PROPERTY_NAME = "gameSymlinkName";
static constexpr const char * LOCAL_MODE_PROPERTY_NAME = "localMode";

static constexpr const char * GAME_VERSIONS_CATEGORY_NAME = "gameVersions";
static constexpr const char * GAME_VERSIONS_LIST_FILE_PATH_PROPERTY_NAME = LIST_FILE_PATH;
static constexpr const char * PREFERRED_GAME_VERSION_PROPERTY_NAME = "preferred";

static constexpr const char * MODS_CATEGORY_NAME = "mods";
static constexpr const char * MODS_LIST_FILE_PATH_PROPERTY_NAME = LIST_FILE_PATH;
static constexpr const char * FAVOURITE_MODS_LIST_FILE_PATH_PROPERTY_NAME = "favouritesListFilePath";
static constexpr const char * MODS_DIRECTORY_PATH_PROPERTY_NAME = DIRECTORY_PATH;
static constexpr const char * MODS_SYMLINK_NAME_PROPERTY_NAME = SYMLINK_NAME;
static constexpr const char * MOD_PACKAGE_DOWNLOADS_DIRECTORY_PATH_PROPERTY_NAME = "packageDownloadsDirectoryPath";
static constexpr const char * MOD_IMAGES_DIRECTORY_PATH_PROPERTY_NAME = "imagesDirectoryPath";
static constexpr const char * MOD_SOURCE_FILES_DIRECTORY_PATH_PROPERTY_NAME = "sourceFilesDirectoryPath";

static constexpr const char * MAPS_CATEGORY_NAME = "maps";
static constexpr const char * MAPS_DIRECTORY_PATH_PROPERTY_NAME = DIRECTORY_PATH;
static constexpr const char * MAPS_SYMLINK_NAME_PROPERTY_NAME = SYMLINK_NAME;

static constexpr const char * DOWNLOADS_CATEGORY_NAME = "downloads";
static constexpr const char * DOWNLOADS_DIRECTORY_PATH_PROPERTY_NAME = DIRECTORY_PATH;
static constexpr const char * DOWNLOAD_CACHE_FILE_NAME_PROPERTY_NAME = "cacheFileName";
static constexpr const char * MOD_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME = "mods";
static constexpr const char * MAP_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME = "maps";
static constexpr const char * GAME_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME = "games";

static constexpr const char * CACHE_CATEGORY_NAME = "cache";
static constexpr const char * CACHE_DIRECTORY_PATH_PROPERTY_NAME = DIRECTORY_PATH;

static constexpr const char * DOSBOX_CATEGORY_NAME = "dosbox";
static constexpr const char * DOSBOX_DIRECTORY_PATH_PROPERTY_NAME = DIRECTORY_PATH;
static constexpr const char * DOSBOX_EXECUTABLE_FILE_NAME_PROPERTY_NAME = EXECUTABLE_FILE_NAME;
static constexpr const char * DOSBOX_ARGUMENTS_PROPERTY_NAME = "arguments";
static constexpr const char * DOSBOX_DATA_DIRECTORY_NAME_PROPERTY_NAME = DATA_DIRECTORY_NAME;

static constexpr const char * DOSBOX_SCRIPTS_CATEGORY_NAME = "scriptFileNames";
static constexpr const char * DOSBOX_GAME_SCRIPT_FILE_NAME_PROPERTY_NAME = "game";
static constexpr const char * DOSBOX_SETUP_SCRIPT_FILE_NAME_PROPERTY_NAME = "setup";
static constexpr const char * DOSBOX_CLIENT_SCRIPT_FILE_NAME_PROPERTY_NAME = "client";
static constexpr const char * DOSBOX_SERVER_SCRIPT_FILE_NAME_PROPERTY_NAME = "server";

static constexpr const char * DOSBOX_NETWORKING_CATEGORY_NAME = "networking";
static constexpr const char * DOSBOX_SERVER_IP_ADDRESS_PROPERTY_NAME = "serverIPAddress";

static constexpr const char * DOSBOX_NETWORKING_PORTS_CATEGORY_NAME = "ports";
static constexpr const char * DOSBOX_LOCAL_SERVER_PORT_PROPERTY_NAME = "local";
static constexpr const char * DOSBOX_REMOTE_SERVER_PORT_PROPERTY_NAME = "remote";

static constexpr const char * TIME_ZONE_CATEGORY_NAME = "timeZone";
static constexpr const char * TIME_ZONE_DATA_DIRECTORY_NAME_PROPERTY_NAME = DATA_DIRECTORY_NAME;

static constexpr const char * CURL_CATEGORY_NAME = "curl";
static constexpr const char * CURL_DATA_DIRECTORY_NAME_PROPERTY_NAME = DATA_DIRECTORY_NAME;
static constexpr const char * CURL_CERTIFICATE_AUTHORITY_STORE_FILE_NAME_PROPERTY_NAME = "certificateAuthorityStoreFileName";
static constexpr const char * CURL_CONNECTION_TIMEOUT_PROPERTY_NAME = "connectionTimeout";
static constexpr const char * CURL_NETWORK_TIMEOUT_PROPERTY_NAME = "networkTimeout";

static constexpr const char * API_CATEGORY_NAME = "api";
static constexpr const char * API_BASE_URL_PROPERTY_NAME = "baseURL";
static constexpr const char * REMOTE_MOD_LIST_FILE_NAME_PROPERTY_NAME = "remoteModListFileName";
static constexpr const char * REMOTE_GAME_LIST_FILE_NAME_PROPERTY_NAME = "remoteGameListFileName";
static constexpr const char * REMOTE_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME = "remoteDownloadsDirectoryName";
static constexpr const char * REMOTE_MOD_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME = "remoteModDownloadsDirectoryName";
static constexpr const char * REMOTE_MAP_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME = "remoteMapDownloadsDirectoryName";
static constexpr const char * REMOTE_GAME_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME = "remoteGameDownloadsDirectoryName";

static constexpr const char * ANALYTICS_CATEGORY_NAME = "analytics";
static constexpr const char * SEGMENT_ANALYTICS_CATEGORY_NAME = "segment";
static constexpr const char * SEGMENT_ANALYTICS_ENABLED_PROPERTY_NAME = "enabled";
static constexpr const char * SEGMENT_ANALYTICS_DATA_FILE_NAME_PROPERTY_NAME = "dataFileName";

const char * SettingsManager::FILE_FORMAT_VERSION = "1.0.0";
const char * SettingsManager::DEFAULT_SETTINGS_FILE_PATH = "Duke Nukem 3D Mod Manager.json";
const char * SettingsManager::DEFAULT_MODS_LIST_FILE_PATH = "Duke Nukem 3D Mod List.xml";
const char * SettingsManager::DEFAULT_FAVOURITE_MODS_LIST_FILE_PATH = "Duke Nukem 3D Favourite Mods.json";
const char * SettingsManager::DEFAULT_GAME_VERSIONS_LIST_FILE_PATH = "Duke Nukem 3D Game Versions.json";
const char * SettingsManager::DEFAULT_GAME_SYMLINK_NAME = "Game";
const bool SettingsManager::DEFAULT_LOCAL_MODE = false;
const char * SettingsManager::DEFAULT_MODS_DIRECTORY_PATH = "Mods";
const char * SettingsManager::DEFAULT_MODS_SYMLINK_NAME = "Mods";
const char * SettingsManager::DEFAULT_MOD_PACKAGE_DOWNLOADS_DIRECTORY_PATH = "";
const char * SettingsManager::DEFAULT_MOD_IMAGES_DIRECTORY_PATH = "";
const char * SettingsManager::DEFAULT_MOD_SOURCE_FILES_DIRECTORY_PATH = "";
const char * SettingsManager::DEFAULT_MAPS_DIRECTORY_PATH = "";
const char * SettingsManager::DEFAULT_MAPS_SYMLINK_NAME = "Maps";
const char * SettingsManager::DEFAULT_DOWNLOADS_DIRECTORY_PATH = "Downloads";
const char * SettingsManager::DEFAULT_DOWNLOAD_CACHE_FILE_NAME = "Cache.json";
const char * SettingsManager::DEFAULT_MOD_DOWNLOADS_DIRECTORY_NAME = "Mods";
const char * SettingsManager::DEFAULT_MAP_DOWNLOADS_DIRECTORY_NAME = "Maps";
const char * SettingsManager::DEFAULT_GAME_DOWNLOADS_DIRECTORY_NAME = "Games";
const char * SettingsManager::DEFAULT_DATA_DIRECTORY_PATH = "Data";
const char * SettingsManager::DEFAULT_TEMP_DIRECTORY_PATH = "Temp";
const char * SettingsManager::DEFAULT_TEMP_SYMLINK_NAME = "Temp";
const char * SettingsManager::DEFAULT_CACHE_DIRECTORY_PATH = "Cache";
const char * SettingsManager::DEFAULT_DOSBOX_EXECUTABLE_FILE_NAME = "dosbox.exe";
const char * SettingsManager::DEFAULT_DOSBOX_DIRECTORY_PATH = "DOSBox";
const char * SettingsManager::DEFAULT_DOSBOX_ARGUMENTS = "-noconsole";
const char * SettingsManager::DEFAULT_DOSBOX_DATA_DIRECTORY_NAME = "DOSBox";
const char * SettingsManager::DEFAULT_DOSBOX_GAME_SCRIPT_FILE_NAME = "duke3d.conf.in";
const char * SettingsManager::DEFAULT_DOSBOX_SETUP_SCRIPT_FILE_NAME = "duke3d_setup.conf.in";
const char * SettingsManager::DEFAULT_DOSBOX_CLIENT_SCRIPT_FILE_NAME = "duke3d_client.conf.in";
const char * SettingsManager::DEFAULT_DOSBOX_SERVER_SCRIPT_FILE_NAME = "duke3d_server.conf.in";
const GameType SettingsManager::DEFAULT_GAME_TYPE = ModManager::DEFAULT_GAME_TYPE;
const std::string SettingsManager::DEFAULT_PREFERRED_GAME_VERSION(ModManager::DEFAULT_PREFERRED_GAME_VERSION);
const char * SettingsManager::DEFAULT_DOSBOX_SERVER_IP_ADDRESS = "127.0.0.1";
const uint16_t SettingsManager::DEFAULT_DOSBOX_LOCAL_SERVER_PORT = 31337;
const uint16_t SettingsManager::DEFAULT_DOSBOX_REMOTE_SERVER_PORT = 31337;
const char * SettingsManager::DEFAULT_TIME_ZONE_DATA_DIRECTORY_NAME = "Time Zone";
const char * SettingsManager::DEFAULT_CURL_DATA_DIRECTORY_NAME = "cURL";
const char * SettingsManager::DEFAULT_CERTIFICATE_AUTHORITY_STORE_FILE_NAME = "cacert.pem";
const std::chrono::seconds SettingsManager::DEFAULT_CONNECTION_TIMEOUT = 30s;
const std::chrono::seconds SettingsManager::DEFAULT_NETWORK_TIMEOUT = 1min;
const char * SettingsManager::DEFAULT_API_BASE_URL = "http://duke3dmods.com";
const char * SettingsManager::DEFAULT_REMOTE_MODS_LIST_FILE_NAME = "duke3d_mods.xml";
const char * SettingsManager::DEFAULT_REMOTE_GAMES_LIST_FILE_NAME = "duke3d_games.json";
const char * SettingsManager::DEFAULT_REMOTE_DOWNLOADS_DIRECTORY_NAME = "downloads";
const char * SettingsManager::DEFAULT_REMOTE_MOD_DOWNLOADS_DIRECTORY_NAME = "mods";
const char * SettingsManager::DEFAULT_REMOTE_MAP_DOWNLOADS_DIRECTORY_NAME = "maps";
const char * SettingsManager::DEFAULT_REMOTE_GAME_DOWNLOADS_DIRECTORY_NAME = "games";
const bool SettingsManager::DEFAULT_SEGMENT_ANALYTICS_ENABLED = true;
const char * SettingsManager::DEFAULT_SEGMENT_ANALYTICS_DATA_FILE_NAME = "Segment Analytics.json";

static bool assignStringSetting(std::string & setting, const rapidjson::Value & categoryValue, const std::string & propertyName) {
	if(propertyName.empty() || !categoryValue.IsObject() || !categoryValue.HasMember(propertyName.c_str())) {
		return false;
	}

	const rapidjson::Value & settingValue = categoryValue[propertyName.c_str()];

	if(!settingValue.IsString()) {
		return false;
	}

	setting = settingValue.GetString();

	return true;
}

static bool assignBooleanSetting(bool & setting, const rapidjson::Value & categoryValue, const std::string & propertyName) {
	if(propertyName.empty() || !categoryValue.IsObject() || !categoryValue.HasMember(propertyName.c_str())) {
		return false;
	}

	const rapidjson::Value & settingValue = categoryValue[propertyName.c_str()];

	if(!settingValue.IsBool()) {
		return false;
	}

	setting = settingValue.GetBool();

	return true;
}

SettingsManager::SettingsManager()
	: modsListFilePath(DEFAULT_MODS_LIST_FILE_PATH)
	, favouriteModsListFilePath(DEFAULT_FAVOURITE_MODS_LIST_FILE_PATH)
	, gameVersionsListFilePath(DEFAULT_GAME_VERSIONS_LIST_FILE_PATH)
	, gameSymlinkName(DEFAULT_GAME_SYMLINK_NAME)
	, localMode(DEFAULT_LOCAL_MODE)
	, modsDirectoryPath(DEFAULT_MODS_DIRECTORY_PATH)
	, modsSymlinkName(DEFAULT_MODS_SYMLINK_NAME)
	, modPackageDownloadsDirectoryPath(DEFAULT_MOD_PACKAGE_DOWNLOADS_DIRECTORY_PATH)
	, modImagesDirectoryPath(DEFAULT_MOD_IMAGES_DIRECTORY_PATH)
	, modSourceFilesDirectoryPath(DEFAULT_MOD_SOURCE_FILES_DIRECTORY_PATH)
	, mapsDirectoryPath(DEFAULT_MAPS_DIRECTORY_PATH)
	, mapsSymlinkName(DEFAULT_MAPS_SYMLINK_NAME)
	, downloadsDirectoryPath(DEFAULT_DOWNLOADS_DIRECTORY_PATH)
	, downloadCacheFileName(DEFAULT_DOWNLOAD_CACHE_FILE_NAME)
	, modDownloadsDirectoryName(DEFAULT_MOD_DOWNLOADS_DIRECTORY_NAME)
	, mapDownloadsDirectoryName(DEFAULT_MAP_DOWNLOADS_DIRECTORY_NAME)
	, gameDownloadsDirectoryName(DEFAULT_GAME_DOWNLOADS_DIRECTORY_NAME)
	, dataDirectoryPath(DEFAULT_DATA_DIRECTORY_PATH)
	, tempDirectoryPath(DEFAULT_TEMP_DIRECTORY_PATH)
	, tempSymlinkName(DEFAULT_TEMP_SYMLINK_NAME)
	, cacheDirectoryPath(DEFAULT_CACHE_DIRECTORY_PATH)
	, dosboxExecutableFileName(DEFAULT_DOSBOX_EXECUTABLE_FILE_NAME)
	, dosboxDirectoryPath(DEFAULT_DOSBOX_DIRECTORY_PATH)
	, dosboxArguments(DEFAULT_DOSBOX_ARGUMENTS)
	, dosboxDataDirectoryName(DEFAULT_DOSBOX_DATA_DIRECTORY_NAME)
	, dosboxGameScriptFileName(DEFAULT_DOSBOX_GAME_SCRIPT_FILE_NAME)
	, dosboxSetupScriptFileName(DEFAULT_DOSBOX_SETUP_SCRIPT_FILE_NAME)
	, dosboxClientScriptFileName(DEFAULT_DOSBOX_CLIENT_SCRIPT_FILE_NAME)
	, dosboxServerScriptFileName(DEFAULT_DOSBOX_SERVER_SCRIPT_FILE_NAME)
	, dosboxServerIPAddress(DEFAULT_DOSBOX_SERVER_IP_ADDRESS)
	, gameType(DEFAULT_GAME_TYPE)
	, preferredGameVersion(DEFAULT_PREFERRED_GAME_VERSION)
	, dosboxLocalServerPort(DEFAULT_DOSBOX_LOCAL_SERVER_PORT)
	, dosboxRemoteServerPort(DEFAULT_DOSBOX_REMOTE_SERVER_PORT)
	, timeZoneDataDirectoryName(DEFAULT_TIME_ZONE_DATA_DIRECTORY_NAME)
	, curlDataDirectoryName(DEFAULT_CURL_DATA_DIRECTORY_NAME)
	, certificateAuthorityStoreFileName(DEFAULT_CERTIFICATE_AUTHORITY_STORE_FILE_NAME)
	, connectionTimeout(DEFAULT_CONNECTION_TIMEOUT)
	, networkTimeout(DEFAULT_NETWORK_TIMEOUT)
	, apiBaseURL(DEFAULT_API_BASE_URL)
	, remoteModsListFileName(DEFAULT_REMOTE_MODS_LIST_FILE_NAME)
	, remoteGamesListFileName(DEFAULT_REMOTE_GAMES_LIST_FILE_NAME)
	, remoteDownloadsDirectoryName(DEFAULT_REMOTE_DOWNLOADS_DIRECTORY_NAME)
	, remoteModDownloadsDirectoryName(DEFAULT_REMOTE_MOD_DOWNLOADS_DIRECTORY_NAME)
	, remoteMapDownloadsDirectoryName(DEFAULT_REMOTE_MAP_DOWNLOADS_DIRECTORY_NAME)
	, remoteGameDownloadsDirectoryName(DEFAULT_REMOTE_GAME_DOWNLOADS_DIRECTORY_NAME)
	, segmentAnalyticsEnabled(DEFAULT_SEGMENT_ANALYTICS_ENABLED)
	, segmentAnalyticsDataFileName(DEFAULT_SEGMENT_ANALYTICS_DATA_FILE_NAME) { }

SettingsManager::SettingsManager(SettingsManager && s) noexcept
	: modsListFilePath(std::move(s.modsListFilePath))
	, favouriteModsListFilePath(std::move(s.favouriteModsListFilePath))
	, gameVersionsListFilePath(std::move(s.gameVersionsListFilePath))
	, gameSymlinkName(std::move(s.gameSymlinkName))
	, localMode(s.localMode)
	, modsDirectoryPath(std::move(s.modsDirectoryPath))
	, modsSymlinkName(std::move(s.modsSymlinkName))
	, modPackageDownloadsDirectoryPath(std::move(s.modPackageDownloadsDirectoryPath))
	, modSourceFilesDirectoryPath(std::move(s.modSourceFilesDirectoryPath))
	, mapsDirectoryPath(std::move(s.mapsDirectoryPath))
	, mapsSymlinkName(std::move(s.mapsSymlinkName))
	, downloadsDirectoryPath(std::move(s.downloadsDirectoryPath))
	, downloadCacheFileName(std::move(s.downloadCacheFileName))
	, modDownloadsDirectoryName(std::move(s.modDownloadsDirectoryName))
	, mapDownloadsDirectoryName(std::move(s.mapDownloadsDirectoryName))
	, gameDownloadsDirectoryName(std::move(s.gameDownloadsDirectoryName))
	, dataDirectoryPath(std::move(s.dataDirectoryPath))
	, tempDirectoryPath(std::move(s.tempDirectoryPath))
	, tempSymlinkName(std::move(s.tempSymlinkName))
	, cacheDirectoryPath(std::move(s.cacheDirectoryPath))
	, dosboxExecutableFileName(std::move(s.dosboxExecutableFileName))
	, dosboxDirectoryPath(std::move(s.dosboxDirectoryPath))
	, dosboxArguments(std::move(s.dosboxArguments))
	, dosboxDataDirectoryName(std::move(s.dosboxDataDirectoryName))
	, dosboxGameScriptFileName(std::move(s.dosboxGameScriptFileName))
	, dosboxSetupScriptFileName(std::move(s.dosboxSetupScriptFileName))
	, dosboxClientScriptFileName(std::move(s.dosboxClientScriptFileName))
	, dosboxServerScriptFileName(std::move(s.dosboxServerScriptFileName))
	, dosboxServerIPAddress(std::move(s.dosboxServerIPAddress))
	, gameType(s.gameType)
	, preferredGameVersion(std::move(s.preferredGameVersion))
	, dosboxLocalServerPort(s.dosboxLocalServerPort)
	, dosboxRemoteServerPort(s.dosboxRemoteServerPort)
	, timeZoneDataDirectoryName(std::move(s.timeZoneDataDirectoryName))
	, curlDataDirectoryName(std::move(s.curlDataDirectoryName))
	, certificateAuthorityStoreFileName(std::move(s.certificateAuthorityStoreFileName))
	, connectionTimeout(s.connectionTimeout)
	, networkTimeout(s.networkTimeout)
	, apiBaseURL(std::move(s.apiBaseURL))
	, remoteModsListFileName(std::move(s.remoteModsListFileName))
	, remoteGamesListFileName(std::move(s.remoteGamesListFileName))
	, remoteDownloadsDirectoryName(std::move(s.remoteDownloadsDirectoryName))
	, remoteModDownloadsDirectoryName(std::move(s.remoteModDownloadsDirectoryName))
	, remoteMapDownloadsDirectoryName(std::move(s.remoteMapDownloadsDirectoryName))
	, remoteGameDownloadsDirectoryName(std::move(s.remoteGameDownloadsDirectoryName))
	, segmentAnalyticsEnabled(s.segmentAnalyticsEnabled)
	, segmentAnalyticsDataFileName(std::move(s.segmentAnalyticsDataFileName)) { }

SettingsManager::SettingsManager(const SettingsManager & s)
	: modsListFilePath(s.modsListFilePath)
	, favouriteModsListFilePath(s.favouriteModsListFilePath)
	, gameVersionsListFilePath(s.gameVersionsListFilePath)
	, gameSymlinkName(s.gameSymlinkName)
	, localMode(s.localMode)
	, modsDirectoryPath(s.modsDirectoryPath)
	, modsSymlinkName(s.modsSymlinkName)
	, modPackageDownloadsDirectoryPath(s.modPackageDownloadsDirectoryPath)
	, modSourceFilesDirectoryPath(s.modSourceFilesDirectoryPath)
	, mapsDirectoryPath(s.mapsDirectoryPath)
	, mapsSymlinkName(s.mapsSymlinkName)
	, downloadsDirectoryPath(s.downloadsDirectoryPath)
	, downloadCacheFileName(s.downloadCacheFileName)
	, modDownloadsDirectoryName(s.modDownloadsDirectoryName)
	, mapDownloadsDirectoryName(s.mapDownloadsDirectoryName)
	, gameDownloadsDirectoryName(s.gameDownloadsDirectoryName)
	, dataDirectoryPath(s.dataDirectoryPath)
	, tempDirectoryPath(s.tempDirectoryPath)
	, tempSymlinkName(s.tempSymlinkName)
	, cacheDirectoryPath(s.cacheDirectoryPath)
	, dosboxExecutableFileName(s.dosboxExecutableFileName)
	, dosboxDirectoryPath(s.dosboxDirectoryPath)
	, dosboxArguments(s.dosboxArguments)
	, dosboxDataDirectoryName(s.dosboxDataDirectoryName)
	, dosboxGameScriptFileName(s.dosboxGameScriptFileName)
	, dosboxSetupScriptFileName(s.dosboxSetupScriptFileName)
	, dosboxClientScriptFileName(s.dosboxClientScriptFileName)
	, dosboxServerScriptFileName(s.dosboxServerScriptFileName)
	, dosboxServerIPAddress(s.dosboxServerIPAddress)
	, gameType(s.gameType)
	, preferredGameVersion(s.preferredGameVersion)
	, dosboxLocalServerPort(s.dosboxLocalServerPort)
	, dosboxRemoteServerPort(s.dosboxRemoteServerPort)
	, timeZoneDataDirectoryName(s.timeZoneDataDirectoryName)
	, curlDataDirectoryName(s.curlDataDirectoryName)
	, certificateAuthorityStoreFileName(s.certificateAuthorityStoreFileName)
	, connectionTimeout(s.connectionTimeout)
	, networkTimeout(s.networkTimeout)
	, apiBaseURL(s.apiBaseURL)
	, remoteModsListFileName(s.remoteModsListFileName)
	, remoteGamesListFileName(s.remoteGamesListFileName)
	, remoteDownloadsDirectoryName(s.remoteDownloadsDirectoryName)
	, remoteModDownloadsDirectoryName(s.remoteModDownloadsDirectoryName)
	, remoteMapDownloadsDirectoryName(s.remoteMapDownloadsDirectoryName)
	, remoteGameDownloadsDirectoryName(s.remoteGameDownloadsDirectoryName)
	, segmentAnalyticsEnabled(s.segmentAnalyticsEnabled)
	, segmentAnalyticsDataFileName(s.segmentAnalyticsDataFileName) { }

SettingsManager & SettingsManager::operator = (SettingsManager && s) noexcept {
	if(this != &s) {
		modsListFilePath = std::move(s.modsListFilePath);
		favouriteModsListFilePath = std::move(s.favouriteModsListFilePath);
		gameVersionsListFilePath = std::move(s.gameVersionsListFilePath);
		gameSymlinkName = std::move(s.gameSymlinkName);
		localMode = s.localMode;
		modsDirectoryPath = std::move(s.modsDirectoryPath);
		modsSymlinkName = std::move(s.modsSymlinkName);
		modPackageDownloadsDirectoryPath = std::move(s.modPackageDownloadsDirectoryPath);
		modImagesDirectoryPath = std::move(s.modImagesDirectoryPath);
		modSourceFilesDirectoryPath = std::move(s.modSourceFilesDirectoryPath);
		mapsDirectoryPath = std::move(s.mapsDirectoryPath);
		mapsSymlinkName = std::move(s.mapsSymlinkName);
		downloadsDirectoryPath = std::move(s.downloadsDirectoryPath);
		downloadCacheFileName = std::move(s.downloadCacheFileName);
		modDownloadsDirectoryName = std::move(s.modDownloadsDirectoryName);
		mapDownloadsDirectoryName = std::move(s.mapDownloadsDirectoryName);
		gameDownloadsDirectoryName = std::move(s.gameDownloadsDirectoryName);
		dataDirectoryPath = std::move(s.dataDirectoryPath);
		tempDirectoryPath = std::move(s.tempDirectoryPath);
		tempSymlinkName = std::move(s.tempSymlinkName);
		cacheDirectoryPath = std::move(s.cacheDirectoryPath);
		dosboxExecutableFileName = std::move(s.dosboxExecutableFileName);
		dosboxDirectoryPath = std::move(s.dosboxDirectoryPath);
		dosboxArguments = std::move(s.dosboxArguments);
		dosboxDataDirectoryName = std::move(s.dosboxDataDirectoryName);
		dosboxGameScriptFileName = std::move(s.dosboxGameScriptFileName);
		dosboxSetupScriptFileName = std::move(s.dosboxSetupScriptFileName);
		dosboxClientScriptFileName = std::move(s.dosboxClientScriptFileName);
		dosboxServerScriptFileName = std::move(s.dosboxServerScriptFileName);
		dosboxServerIPAddress = std::move(s.dosboxServerIPAddress);
		gameType = s.gameType;
		preferredGameVersion = std::move(s.preferredGameVersion);
		dosboxLocalServerPort = s.dosboxLocalServerPort;
		dosboxRemoteServerPort = s.dosboxRemoteServerPort;
		timeZoneDataDirectoryName = std::move(s.timeZoneDataDirectoryName);
		curlDataDirectoryName = std::move(s.curlDataDirectoryName);
		certificateAuthorityStoreFileName = std::move(s.certificateAuthorityStoreFileName);
		connectionTimeout = s.connectionTimeout;
		networkTimeout = s.networkTimeout;
		apiBaseURL = std::move(s.apiBaseURL);
		remoteModsListFileName = std::move(s.remoteModsListFileName);
		remoteGamesListFileName = std::move(s.remoteGamesListFileName);
		remoteDownloadsDirectoryName = std::move(s.remoteDownloadsDirectoryName);
		remoteModDownloadsDirectoryName = std::move(s.remoteModDownloadsDirectoryName);
		remoteMapDownloadsDirectoryName = std::move(s.remoteMapDownloadsDirectoryName);
		remoteGameDownloadsDirectoryName = std::move(s.remoteGameDownloadsDirectoryName);
		segmentAnalyticsEnabled = s.segmentAnalyticsEnabled;
		segmentAnalyticsDataFileName = std::move(s.segmentAnalyticsDataFileName);
	}

	return *this;
}

SettingsManager & SettingsManager::operator = (const SettingsManager & s) {
	modsListFilePath = s.modsListFilePath;
	favouriteModsListFilePath = s.favouriteModsListFilePath;
	gameVersionsListFilePath = s.gameVersionsListFilePath;
	gameSymlinkName = s.gameSymlinkName;
	localMode = s.localMode;
	modsDirectoryPath = s.modsDirectoryPath;
	modsSymlinkName = s.modsSymlinkName;
	modPackageDownloadsDirectoryPath = s.modPackageDownloadsDirectoryPath;
	modImagesDirectoryPath = s.modImagesDirectoryPath;
	modSourceFilesDirectoryPath = s.modSourceFilesDirectoryPath;
	mapsDirectoryPath = s.mapsDirectoryPath;
	mapsSymlinkName = s.mapsSymlinkName;
	downloadsDirectoryPath = s.downloadsDirectoryPath;
	downloadCacheFileName = s.downloadCacheFileName;
	modDownloadsDirectoryName = s.modDownloadsDirectoryName;
	mapDownloadsDirectoryName = s.mapDownloadsDirectoryName;
	gameDownloadsDirectoryName = s.gameDownloadsDirectoryName;
	dataDirectoryPath = s.dataDirectoryPath;
	tempDirectoryPath = s.tempDirectoryPath;
	tempSymlinkName = s.tempSymlinkName;
	cacheDirectoryPath = s.cacheDirectoryPath;
	dosboxExecutableFileName = s.dosboxExecutableFileName;
	dosboxDirectoryPath = s.dosboxDirectoryPath;
	dosboxArguments = s.dosboxArguments;
	dosboxDataDirectoryName = s.dosboxDataDirectoryName;
	dosboxGameScriptFileName = s.dosboxGameScriptFileName;
	dosboxSetupScriptFileName = s.dosboxSetupScriptFileName;
	dosboxClientScriptFileName = s.dosboxClientScriptFileName;
	dosboxServerScriptFileName = s.dosboxServerScriptFileName;
	dosboxServerIPAddress = s.dosboxServerIPAddress;
	gameType = s.gameType;
	preferredGameVersion = s.preferredGameVersion;
	dosboxLocalServerPort = s.dosboxLocalServerPort;
	dosboxRemoteServerPort = s.dosboxRemoteServerPort;
	timeZoneDataDirectoryName = s.timeZoneDataDirectoryName;
	curlDataDirectoryName = s.curlDataDirectoryName;
	certificateAuthorityStoreFileName = s.certificateAuthorityStoreFileName;
	connectionTimeout = s.connectionTimeout;
	networkTimeout = s.networkTimeout;
	apiBaseURL = s.apiBaseURL;
	remoteModsListFileName = s.remoteModsListFileName;
	remoteGamesListFileName = s.remoteGamesListFileName;
	remoteDownloadsDirectoryName = s.remoteDownloadsDirectoryName;
	remoteModDownloadsDirectoryName =s.remoteModDownloadsDirectoryName;
	remoteMapDownloadsDirectoryName = s.remoteMapDownloadsDirectoryName;
	remoteGameDownloadsDirectoryName = s.remoteGameDownloadsDirectoryName;
	segmentAnalyticsEnabled = s.segmentAnalyticsEnabled;
	segmentAnalyticsDataFileName = s.segmentAnalyticsDataFileName;

	return *this;
}

SettingsManager::~SettingsManager() = default;

void SettingsManager::reset() {
	modsListFilePath = DEFAULT_MODS_LIST_FILE_PATH;
	favouriteModsListFilePath = DEFAULT_FAVOURITE_MODS_LIST_FILE_PATH;
	gameVersionsListFilePath = DEFAULT_GAME_VERSIONS_LIST_FILE_PATH;
	gameSymlinkName = DEFAULT_GAME_SYMLINK_NAME;
	localMode = DEFAULT_LOCAL_MODE;
	modsDirectoryPath = DEFAULT_MODS_DIRECTORY_PATH;
	modsSymlinkName = DEFAULT_MODS_SYMLINK_NAME;
	modPackageDownloadsDirectoryPath = DEFAULT_MOD_PACKAGE_DOWNLOADS_DIRECTORY_PATH;
	modImagesDirectoryPath = DEFAULT_MOD_IMAGES_DIRECTORY_PATH;
	modSourceFilesDirectoryPath = DEFAULT_MOD_SOURCE_FILES_DIRECTORY_PATH;
	mapsDirectoryPath = DEFAULT_MAPS_DIRECTORY_PATH;
	mapsSymlinkName = DEFAULT_MAPS_SYMLINK_NAME;
	downloadsDirectoryPath = DEFAULT_DOWNLOADS_DIRECTORY_PATH;
	downloadCacheFileName = DEFAULT_DOWNLOAD_CACHE_FILE_NAME;
	modDownloadsDirectoryName = DEFAULT_MOD_DOWNLOADS_DIRECTORY_NAME;
	mapDownloadsDirectoryName = DEFAULT_MAP_DOWNLOADS_DIRECTORY_NAME;
	gameDownloadsDirectoryName = DEFAULT_GAME_DOWNLOADS_DIRECTORY_NAME;
	dataDirectoryPath = DEFAULT_DATA_DIRECTORY_PATH;
	tempDirectoryPath = DEFAULT_TEMP_DIRECTORY_PATH;
	tempSymlinkName = DEFAULT_TEMP_SYMLINK_NAME;
	cacheDirectoryPath = DEFAULT_CACHE_DIRECTORY_PATH;
	dosboxExecutableFileName = DEFAULT_DOSBOX_EXECUTABLE_FILE_NAME;
	dosboxDirectoryPath = DEFAULT_DOSBOX_DIRECTORY_PATH;
	dosboxArguments = DEFAULT_DOSBOX_ARGUMENTS;
	dosboxDataDirectoryName = DEFAULT_DOSBOX_DATA_DIRECTORY_NAME;
	dosboxGameScriptFileName = DEFAULT_DOSBOX_GAME_SCRIPT_FILE_NAME;
	dosboxSetupScriptFileName = DEFAULT_DOSBOX_SETUP_SCRIPT_FILE_NAME;
	dosboxClientScriptFileName = DEFAULT_DOSBOX_CLIENT_SCRIPT_FILE_NAME;
	dosboxServerScriptFileName = DEFAULT_DOSBOX_SERVER_SCRIPT_FILE_NAME;
	dosboxServerIPAddress = DEFAULT_DOSBOX_SERVER_IP_ADDRESS;
	gameType = DEFAULT_GAME_TYPE;
	preferredGameVersion = DEFAULT_PREFERRED_GAME_VERSION;
	dosboxLocalServerPort = DEFAULT_DOSBOX_LOCAL_SERVER_PORT;
	dosboxRemoteServerPort = DEFAULT_DOSBOX_REMOTE_SERVER_PORT;
	timeZoneDataDirectoryName = DEFAULT_TIME_ZONE_DATA_DIRECTORY_NAME;
	curlDataDirectoryName = DEFAULT_CURL_DATA_DIRECTORY_NAME;
	certificateAuthorityStoreFileName = DEFAULT_CERTIFICATE_AUTHORITY_STORE_FILE_NAME;
	connectionTimeout = DEFAULT_CONNECTION_TIMEOUT;
	networkTimeout = DEFAULT_NETWORK_TIMEOUT;
	apiBaseURL = DEFAULT_API_BASE_URL;
	remoteModsListFileName = DEFAULT_REMOTE_MODS_LIST_FILE_NAME;
	remoteGamesListFileName = DEFAULT_REMOTE_GAMES_LIST_FILE_NAME;
	remoteDownloadsDirectoryName = DEFAULT_REMOTE_DOWNLOADS_DIRECTORY_NAME;
	remoteModDownloadsDirectoryName = DEFAULT_REMOTE_MOD_DOWNLOADS_DIRECTORY_NAME;
	remoteMapDownloadsDirectoryName = DEFAULT_REMOTE_MAP_DOWNLOADS_DIRECTORY_NAME;
	remoteGameDownloadsDirectoryName = DEFAULT_REMOTE_GAME_DOWNLOADS_DIRECTORY_NAME;
	segmentAnalyticsEnabled = DEFAULT_SEGMENT_ANALYTICS_ENABLED;
	segmentAnalyticsDataFileName = DEFAULT_SEGMENT_ANALYTICS_DATA_FILE_NAME;
}

rapidjson::Document SettingsManager::toJSON() const {
	rapidjson::Document settingsDocument(rapidjson::kObjectType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = settingsDocument.GetAllocator();

	rapidjson::Value fileFormatVersionValue(FILE_FORMAT_VERSION, allocator);
	settingsDocument.AddMember(rapidjson::StringRef(FILE_FORMAT_VERSION_PROPERTY_NAME), fileFormatVersionValue, allocator);
	rapidjson::Value gameTypeValue(Utilities::toCapitalCase(magic_enum::enum_name(gameType)).c_str(), allocator);
	settingsDocument.AddMember(rapidjson::StringRef(GAME_TYPE_PROPERTY_NAME), gameTypeValue, allocator);
	rapidjson::Value dataDirectoryPathValue(dataDirectoryPath.c_str(), allocator);
	settingsDocument.AddMember(rapidjson::StringRef(DATA_DIRECTORY_PATH_PROPERTY_NAME), dataDirectoryPathValue, allocator);
	rapidjson::Value tempDirectoryPathValue(tempDirectoryPath.c_str(), allocator);
	settingsDocument.AddMember(rapidjson::StringRef(TEMP_DIRECTORY_PATH_PROPERTY_NAME), tempDirectoryPathValue, allocator);
	rapidjson::Value tempSymlinkNameValue(tempSymlinkName.c_str(), allocator);
	settingsDocument.AddMember(rapidjson::StringRef(TEMP_SYMLINK_NAME_PROPERTY_NAME), tempSymlinkNameValue, allocator);
	rapidjson::Value gameSymlinkNameValue(gameSymlinkName.c_str(), allocator);
	settingsDocument.AddMember(rapidjson::StringRef(GAME_SYMLINK_NAME_PROPERTY_NAME), gameSymlinkNameValue, allocator);
	settingsDocument.AddMember(rapidjson::StringRef(LOCAL_MODE_PROPERTY_NAME), rapidjson::Value(localMode), allocator);

	rapidjson::Value gameVersionsCategoryValue(rapidjson::kObjectType);

	rapidjson::Value gameVersionsListFilePathValue(gameVersionsListFilePath.c_str(), allocator);
	gameVersionsCategoryValue.AddMember(rapidjson::StringRef(GAME_VERSIONS_LIST_FILE_PATH_PROPERTY_NAME), gameVersionsListFilePathValue, allocator);
	rapidjson::Value preferredGameVersionValue(preferredGameVersion.c_str(), allocator);
	gameVersionsCategoryValue.AddMember(rapidjson::StringRef(PREFERRED_GAME_VERSION_PROPERTY_NAME), preferredGameVersionValue, allocator);

	settingsDocument.AddMember(rapidjson::StringRef(GAME_VERSIONS_CATEGORY_NAME), gameVersionsCategoryValue, allocator);

	rapidjson::Value modsCategoryValue(rapidjson::kObjectType);

	rapidjson::Value modsListFilePathValue(modsListFilePath.c_str(), allocator);
	modsCategoryValue.AddMember(rapidjson::StringRef(MODS_LIST_FILE_PATH_PROPERTY_NAME), modsListFilePathValue, allocator);
	rapidjson::Value favouriteModsListFilePathValue(favouriteModsListFilePath.c_str(), allocator);
	modsCategoryValue.AddMember(rapidjson::StringRef(FAVOURITE_MODS_LIST_FILE_PATH_PROPERTY_NAME), favouriteModsListFilePathValue, allocator);
	rapidjson::Value modsDirectoryPathValue(modsDirectoryPath.c_str(), allocator);
	modsCategoryValue.AddMember(rapidjson::StringRef(MODS_DIRECTORY_PATH_PROPERTY_NAME), modsDirectoryPathValue, allocator);
	rapidjson::Value modsSymlinkNameValue(modsSymlinkName.c_str(), allocator);
	modsCategoryValue.AddMember(rapidjson::StringRef(MODS_SYMLINK_NAME_PROPERTY_NAME), modsSymlinkNameValue, allocator);

	if(!modPackageDownloadsDirectoryPath.empty()) {
		rapidjson::Value modPackageDownloadsDirectoryPathValue(modPackageDownloadsDirectoryPath.c_str(), allocator);
		modsCategoryValue.AddMember(rapidjson::StringRef(MOD_PACKAGE_DOWNLOADS_DIRECTORY_PATH_PROPERTY_NAME), modPackageDownloadsDirectoryPathValue, allocator);
	}

	if(!modImagesDirectoryPath.empty()) {
		rapidjson::Value modImagesDirectoryPathValue(modImagesDirectoryPath.c_str(), allocator);
		modsCategoryValue.AddMember(rapidjson::StringRef(MOD_IMAGES_DIRECTORY_PATH_PROPERTY_NAME), modImagesDirectoryPathValue, allocator);
	}

	if(!modSourceFilesDirectoryPath.empty()) {
		rapidjson::Value modSourceFilesDirectoryPathValue(modSourceFilesDirectoryPath.c_str(), allocator);
		modsCategoryValue.AddMember(rapidjson::StringRef(MOD_SOURCE_FILES_DIRECTORY_PATH_PROPERTY_NAME), modSourceFilesDirectoryPathValue, allocator);
	}

	settingsDocument.AddMember(rapidjson::StringRef(MODS_CATEGORY_NAME), modsCategoryValue, allocator);

	rapidjson::Value mapsCategoryValue(rapidjson::kObjectType);

	rapidjson::Value mapsDirectoryPathValue(mapsDirectoryPath.c_str(), allocator);
	mapsCategoryValue.AddMember(rapidjson::StringRef(MAPS_DIRECTORY_PATH_PROPERTY_NAME), mapsDirectoryPathValue, allocator);
	rapidjson::Value mapsSymlinkNameValue(mapsSymlinkName.c_str(), allocator);
	mapsCategoryValue.AddMember(rapidjson::StringRef(MAPS_SYMLINK_NAME_PROPERTY_NAME), mapsSymlinkNameValue, allocator);

	settingsDocument.AddMember(rapidjson::StringRef(MAPS_CATEGORY_NAME), mapsCategoryValue, allocator);

	rapidjson::Value downloadsCategoryValue(rapidjson::kObjectType);

	rapidjson::Value downloadsDirectoryPathValue(downloadsDirectoryPath.c_str(), allocator);
	downloadsCategoryValue.AddMember(rapidjson::StringRef(DOWNLOADS_DIRECTORY_PATH_PROPERTY_NAME), downloadsDirectoryPathValue, allocator);
	rapidjson::Value downloadCacheFileNameValue(downloadCacheFileName.c_str(), allocator);
	downloadsCategoryValue.AddMember(rapidjson::StringRef(DOWNLOAD_CACHE_FILE_NAME_PROPERTY_NAME), downloadCacheFileNameValue, allocator);
	rapidjson::Value modDownloadsDirectoryNameValue(modDownloadsDirectoryName.c_str(), allocator);
	downloadsCategoryValue.AddMember(rapidjson::StringRef(MOD_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME), modDownloadsDirectoryNameValue, allocator);
	rapidjson::Value mapDownloadsDirectoryNameValue(mapDownloadsDirectoryName.c_str(), allocator);
	downloadsCategoryValue.AddMember(rapidjson::StringRef(MAP_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME), mapDownloadsDirectoryNameValue, allocator);
	rapidjson::Value gameDownloadsDirectoryNameValue(gameDownloadsDirectoryName.c_str(), allocator);
	downloadsCategoryValue.AddMember(rapidjson::StringRef(GAME_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME), gameDownloadsDirectoryNameValue, allocator);

	settingsDocument.AddMember(rapidjson::StringRef(DOWNLOADS_CATEGORY_NAME), downloadsCategoryValue, allocator);

	rapidjson::Value cacheCategoryValue(rapidjson::kObjectType);

	rapidjson::Value cacheDirectoryPathValue(cacheDirectoryPath.c_str(), allocator);
	cacheCategoryValue.AddMember(rapidjson::StringRef(CACHE_DIRECTORY_PATH_PROPERTY_NAME), cacheDirectoryPathValue, allocator);

	settingsDocument.AddMember(rapidjson::StringRef(CACHE_CATEGORY_NAME), cacheCategoryValue, allocator);

	rapidjson::Value dosboxCategoryValue(rapidjson::kObjectType);

	rapidjson::Value dosboxDirectoryPathValue(dosboxDirectoryPath.c_str(), allocator);
	dosboxCategoryValue.AddMember(rapidjson::StringRef(DOSBOX_DIRECTORY_PATH_PROPERTY_NAME), dosboxDirectoryPathValue, allocator);
	rapidjson::Value dosboxExecutableFileNameValue(dosboxExecutableFileName.c_str(), allocator);
	dosboxCategoryValue.AddMember(rapidjson::StringRef(DOSBOX_EXECUTABLE_FILE_NAME_PROPERTY_NAME), dosboxExecutableFileNameValue, allocator);
	rapidjson::Value dosboxArgumentsValue(dosboxArguments.c_str(), allocator);
	dosboxCategoryValue.AddMember(rapidjson::StringRef(DOSBOX_ARGUMENTS_PROPERTY_NAME), dosboxArgumentsValue, allocator);
	rapidjson::Value dosboxDataDirectroryNameValue(dosboxDataDirectoryName.c_str(), allocator);
	dosboxCategoryValue.AddMember(rapidjson::StringRef(DOSBOX_DATA_DIRECTORY_NAME_PROPERTY_NAME), dosboxDataDirectroryNameValue, allocator);

	rapidjson::Value dosboxScriptsCategoryValue(rapidjson::kObjectType);

	rapidjson::Value dosboxGameScriptFileNameValue(dosboxGameScriptFileName.c_str(), allocator);
	dosboxScriptsCategoryValue.AddMember(rapidjson::StringRef(DOSBOX_GAME_SCRIPT_FILE_NAME_PROPERTY_NAME), dosboxGameScriptFileNameValue, allocator);
	rapidjson::Value dosboxSetupScriptFileNameValue(dosboxSetupScriptFileName.c_str(), allocator);
	dosboxScriptsCategoryValue.AddMember(rapidjson::StringRef(DOSBOX_SETUP_SCRIPT_FILE_NAME_PROPERTY_NAME), dosboxSetupScriptFileNameValue, allocator);
	rapidjson::Value dosboxClientScriptFileNameValue(dosboxClientScriptFileName.c_str(), allocator);
	dosboxScriptsCategoryValue.AddMember(rapidjson::StringRef(DOSBOX_CLIENT_SCRIPT_FILE_NAME_PROPERTY_NAME), dosboxClientScriptFileNameValue, allocator);
	rapidjson::Value dosboxServerScriptFileNameValue(dosboxServerScriptFileName.c_str(), allocator);
	dosboxScriptsCategoryValue.AddMember(rapidjson::StringRef(DOSBOX_SERVER_SCRIPT_FILE_NAME_PROPERTY_NAME), dosboxServerScriptFileNameValue, allocator);

	dosboxCategoryValue.AddMember(rapidjson::StringRef(DOSBOX_SCRIPTS_CATEGORY_NAME), dosboxScriptsCategoryValue, allocator);

	rapidjson::Value dosboxNetworkingCategoryValue(rapidjson::kObjectType);

	rapidjson::Value dosboxServerIPAddressValue(dosboxServerIPAddress.c_str(), allocator);
	dosboxNetworkingCategoryValue.AddMember(rapidjson::StringRef(DOSBOX_SERVER_IP_ADDRESS_PROPERTY_NAME), dosboxServerIPAddressValue, allocator);

	rapidjson::Value dosboxNetworkingPortsCategoryValue(rapidjson::kObjectType);

	dosboxNetworkingPortsCategoryValue.AddMember(rapidjson::StringRef(DOSBOX_LOCAL_SERVER_PORT_PROPERTY_NAME), rapidjson::Value(dosboxLocalServerPort), allocator);
	dosboxNetworkingPortsCategoryValue.AddMember(rapidjson::StringRef(DOSBOX_REMOTE_SERVER_PORT_PROPERTY_NAME), rapidjson::Value(dosboxRemoteServerPort), allocator);

	dosboxNetworkingCategoryValue.AddMember(rapidjson::StringRef(DOSBOX_NETWORKING_PORTS_CATEGORY_NAME), dosboxNetworkingPortsCategoryValue, allocator);

	dosboxCategoryValue.AddMember(rapidjson::StringRef(DOSBOX_NETWORKING_CATEGORY_NAME), dosboxNetworkingCategoryValue, allocator);

	settingsDocument.AddMember(rapidjson::StringRef(DOSBOX_CATEGORY_NAME), dosboxCategoryValue, allocator);

	rapidjson::Value timeZoneCategoryValue(rapidjson::kObjectType);

	rapidjson::Value timeZoneDataDirectoryNameValue(timeZoneDataDirectoryName.c_str(), allocator);
	timeZoneCategoryValue.AddMember(rapidjson::StringRef(TIME_ZONE_DATA_DIRECTORY_NAME_PROPERTY_NAME), timeZoneDataDirectoryNameValue, allocator);

	settingsDocument.AddMember(rapidjson::StringRef(TIME_ZONE_CATEGORY_NAME), timeZoneCategoryValue, allocator);

	rapidjson::Value curlCategoryValue(rapidjson::kObjectType);

	rapidjson::Value curlDataDirectoryNameValue(curlDataDirectoryName.c_str(), allocator);
	curlCategoryValue.AddMember(rapidjson::StringRef(CURL_DATA_DIRECTORY_NAME_PROPERTY_NAME), curlDataDirectoryNameValue, allocator);
	rapidjson::Value curlCertificateAuthorityStoreFilePathValue(certificateAuthorityStoreFileName.c_str(), allocator);
	curlCategoryValue.AddMember(rapidjson::StringRef(CURL_CERTIFICATE_AUTHORITY_STORE_FILE_NAME_PROPERTY_NAME), curlCertificateAuthorityStoreFilePathValue, allocator);
	curlCategoryValue.AddMember(rapidjson::StringRef(CURL_CONNECTION_TIMEOUT_PROPERTY_NAME), rapidjson::Value(connectionTimeout.count()), allocator);
	curlCategoryValue.AddMember(rapidjson::StringRef(CURL_NETWORK_TIMEOUT_PROPERTY_NAME), rapidjson::Value(networkTimeout.count()), allocator);

	settingsDocument.AddMember(rapidjson::StringRef(CURL_CATEGORY_NAME), curlCategoryValue, allocator);

	rapidjson::Value apiCategoryValue(rapidjson::kObjectType);

	rapidjson::Value apiBaseURLValue(apiBaseURL.c_str(), allocator);
	apiCategoryValue.AddMember(rapidjson::StringRef(API_BASE_URL_PROPERTY_NAME), apiBaseURLValue, allocator);
	rapidjson::Value remoteModsListFileNameValue(remoteModsListFileName.c_str(), allocator);
	apiCategoryValue.AddMember(rapidjson::StringRef(REMOTE_MOD_LIST_FILE_NAME_PROPERTY_NAME), remoteModsListFileNameValue, allocator);
	rapidjson::Value remoteGamesListFileNameValue(remoteGamesListFileName.c_str(), allocator);
	apiCategoryValue.AddMember(rapidjson::StringRef(REMOTE_GAME_LIST_FILE_NAME_PROPERTY_NAME), remoteGamesListFileNameValue, allocator);
	rapidjson::Value remoteDownloadsDirectoryNameValue(remoteDownloadsDirectoryName.c_str(), allocator);
	apiCategoryValue.AddMember(rapidjson::StringRef(REMOTE_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME), remoteDownloadsDirectoryNameValue, allocator);
	rapidjson::Value remoteModDownloadsDirectoryNameValue(remoteModDownloadsDirectoryName.c_str(), allocator);
	apiCategoryValue.AddMember(rapidjson::StringRef(REMOTE_MOD_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME), remoteModDownloadsDirectoryNameValue, allocator);
	rapidjson::Value remoteMapDownloadsDirectoryNameValue(remoteMapDownloadsDirectoryName.c_str(), allocator);
	apiCategoryValue.AddMember(rapidjson::StringRef(REMOTE_MAP_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME), remoteMapDownloadsDirectoryNameValue, allocator);
	rapidjson::Value remoteGameDownloadsDirectoryNameValue(remoteGameDownloadsDirectoryName.c_str(), allocator);
	apiCategoryValue.AddMember(rapidjson::StringRef(REMOTE_GAME_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME), remoteGameDownloadsDirectoryNameValue, allocator);

	settingsDocument.AddMember(rapidjson::StringRef(API_CATEGORY_NAME), apiCategoryValue, allocator);

	rapidjson::Value analyticsCategoryValue(rapidjson::kObjectType);

	rapidjson::Value segmentAnalyticsCategoryValue(rapidjson::kObjectType);

	segmentAnalyticsCategoryValue.AddMember(rapidjson::StringRef(SEGMENT_ANALYTICS_ENABLED_PROPERTY_NAME), rapidjson::Value(segmentAnalyticsEnabled), allocator);
	rapidjson::Value segmentAnalyticsDataFileNameValue(segmentAnalyticsDataFileName.c_str(), allocator);
	segmentAnalyticsCategoryValue.AddMember(rapidjson::StringRef(SEGMENT_ANALYTICS_DATA_FILE_NAME_PROPERTY_NAME), segmentAnalyticsDataFileNameValue, allocator);

	analyticsCategoryValue.AddMember(rapidjson::StringRef(SEGMENT_ANALYTICS_CATEGORY_NAME), segmentAnalyticsCategoryValue, allocator);

	settingsDocument.AddMember(rapidjson::StringRef(ANALYTICS_CATEGORY_NAME), analyticsCategoryValue, allocator);

	return settingsDocument;
}

bool SettingsManager::parseFrom(const rapidjson::Value & settingsDocument) {
	if(!settingsDocument.IsObject()) {
		spdlog::error("Invalid settings value, expected object.");
		return false;
	}

	if(settingsDocument.HasMember(FILE_FORMAT_VERSION_PROPERTY_NAME)) {
		if(!settingsDocument[FILE_FORMAT_VERSION_PROPERTY_NAME].IsString()) {
			spdlog::error("Invalid settings file version: '{}', expected: '{}'.", settingsDocument[FILE_FORMAT_VERSION_PROPERTY_NAME].GetString(), FILE_FORMAT_VERSION);
			return false;
		}

		std::optional<std::uint8_t> optionalVersionComparison(Utilities::compareVersions(settingsDocument[FILE_FORMAT_VERSION_PROPERTY_NAME].GetString(), FILE_FORMAT_VERSION));

		if(!optionalVersionComparison.has_value()) {
			spdlog::error("Invalid settings file version: '{}'.", settingsDocument[FILE_FORMAT_VERSION_PROPERTY_NAME].GetString());
			return false;
		}

		if(*optionalVersionComparison != 0) {
			spdlog::error("Unsupported settings file version: '{}', only version '{}' is supported.", settingsDocument[FILE_FORMAT_VERSION_PROPERTY_NAME].GetString(), FILE_FORMAT_VERSION);
			return false;
		}
	}
	else {
		spdlog::warn("Settings file is missing version, and may fail to load correctly!");
	}

	if(settingsDocument.HasMember(GAME_TYPE_PROPERTY_NAME) && settingsDocument[GAME_TYPE_PROPERTY_NAME].IsString()) {
		std::optional<GameType> gameTypeOptional = magic_enum::enum_cast<GameType>(Utilities::toPascalCase(settingsDocument[GAME_TYPE_PROPERTY_NAME].GetString()));

		if(gameTypeOptional.has_value()) {
			gameType = gameTypeOptional.value();
		}
	}

	assignStringSetting(dataDirectoryPath, settingsDocument, DATA_DIRECTORY_PATH_PROPERTY_NAME);
	assignStringSetting(tempDirectoryPath, settingsDocument, TEMP_DIRECTORY_PATH_PROPERTY_NAME);
	assignStringSetting(tempSymlinkName, settingsDocument, TEMP_SYMLINK_NAME_PROPERTY_NAME);
	assignStringSetting(gameSymlinkName, settingsDocument, GAME_SYMLINK_NAME_PROPERTY_NAME);
	assignBooleanSetting(localMode, settingsDocument, LOCAL_MODE_PROPERTY_NAME);

	if(settingsDocument.HasMember(GAME_VERSIONS_CATEGORY_NAME) && settingsDocument[GAME_VERSIONS_CATEGORY_NAME].IsObject()) {
		const rapidjson::Value & gameVersionsCategoryValue = settingsDocument[GAME_VERSIONS_CATEGORY_NAME];

		assignStringSetting(gameVersionsListFilePath, gameVersionsCategoryValue, GAME_VERSIONS_LIST_FILE_PATH_PROPERTY_NAME);
		assignStringSetting(preferredGameVersion, gameVersionsCategoryValue, PREFERRED_GAME_VERSION_PROPERTY_NAME);
	}

	if(settingsDocument.HasMember(MODS_CATEGORY_NAME) && settingsDocument[MODS_CATEGORY_NAME].IsObject()) {
		const rapidjson::Value & modsCategoryValue = settingsDocument[MODS_CATEGORY_NAME];

		assignStringSetting(modsListFilePath, modsCategoryValue, MODS_LIST_FILE_PATH_PROPERTY_NAME);
		assignStringSetting(favouriteModsListFilePath, modsCategoryValue, FAVOURITE_MODS_LIST_FILE_PATH_PROPERTY_NAME);
		assignStringSetting(modsDirectoryPath, modsCategoryValue, MODS_DIRECTORY_PATH_PROPERTY_NAME);
		assignStringSetting(modsSymlinkName, modsCategoryValue, MODS_SYMLINK_NAME_PROPERTY_NAME);
		assignStringSetting(modPackageDownloadsDirectoryPath, modsCategoryValue, MOD_PACKAGE_DOWNLOADS_DIRECTORY_PATH_PROPERTY_NAME);
		assignStringSetting(modImagesDirectoryPath, modsCategoryValue, MOD_IMAGES_DIRECTORY_PATH_PROPERTY_NAME);
		assignStringSetting(modSourceFilesDirectoryPath, modsCategoryValue, MOD_SOURCE_FILES_DIRECTORY_PATH_PROPERTY_NAME);
	}

	if(settingsDocument.HasMember(MAPS_CATEGORY_NAME) && settingsDocument[MAPS_CATEGORY_NAME].IsObject()) {
		const rapidjson::Value & mapsCategoryValue = settingsDocument[MAPS_CATEGORY_NAME];

		assignStringSetting(mapsDirectoryPath, mapsCategoryValue, MAPS_DIRECTORY_PATH_PROPERTY_NAME);
		assignStringSetting(mapsSymlinkName, mapsCategoryValue, MAPS_SYMLINK_NAME_PROPERTY_NAME);
	}

	if(settingsDocument.HasMember(DOWNLOADS_CATEGORY_NAME) && settingsDocument[DOWNLOADS_CATEGORY_NAME].IsObject()) {
		const rapidjson::Value & downloadsCategoryValue = settingsDocument[DOWNLOADS_CATEGORY_NAME];

		assignStringSetting(downloadsDirectoryPath, downloadsCategoryValue, DOWNLOADS_DIRECTORY_PATH_PROPERTY_NAME);
		assignStringSetting(downloadCacheFileName, downloadsCategoryValue, DOWNLOAD_CACHE_FILE_NAME_PROPERTY_NAME);
		assignStringSetting(modDownloadsDirectoryName, downloadsCategoryValue, MOD_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME);
		assignStringSetting(mapDownloadsDirectoryName, downloadsCategoryValue, MAP_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME);
		assignStringSetting(gameDownloadsDirectoryName, downloadsCategoryValue, GAME_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME);
	}

	if(settingsDocument.HasMember(CACHE_CATEGORY_NAME) && settingsDocument[CACHE_CATEGORY_NAME].IsObject()) {
		const rapidjson::Value & cacheCategoryValue = settingsDocument[CACHE_CATEGORY_NAME];

		assignStringSetting(cacheDirectoryPath, cacheCategoryValue, CACHE_DIRECTORY_PATH_PROPERTY_NAME);
	}

	if(settingsDocument.HasMember(DOSBOX_CATEGORY_NAME) && settingsDocument[DOSBOX_CATEGORY_NAME].IsObject()) {
		const rapidjson::Value & dosboxCategoryValue = settingsDocument[DOSBOX_CATEGORY_NAME];

		assignStringSetting(dosboxDirectoryPath, dosboxCategoryValue, DOSBOX_DIRECTORY_PATH_PROPERTY_NAME);
		assignStringSetting(dosboxExecutableFileName, dosboxCategoryValue, DOSBOX_EXECUTABLE_FILE_NAME_PROPERTY_NAME);
		assignStringSetting(dosboxArguments, dosboxCategoryValue, DOSBOX_ARGUMENTS_PROPERTY_NAME);
		assignStringSetting(dosboxDataDirectoryName, dosboxCategoryValue, DOSBOX_DATA_DIRECTORY_NAME_PROPERTY_NAME);

		if(dosboxCategoryValue.HasMember(DOSBOX_SCRIPTS_CATEGORY_NAME) && dosboxCategoryValue[DOSBOX_SCRIPTS_CATEGORY_NAME].IsObject()) {
			const rapidjson::Value & dosboxScriptsCategoryValue = dosboxCategoryValue[DOSBOX_SCRIPTS_CATEGORY_NAME];

			assignStringSetting(dosboxGameScriptFileName, dosboxScriptsCategoryValue, DOSBOX_GAME_SCRIPT_FILE_NAME_PROPERTY_NAME);
			assignStringSetting(dosboxSetupScriptFileName, dosboxScriptsCategoryValue, DOSBOX_SETUP_SCRIPT_FILE_NAME_PROPERTY_NAME);
			assignStringSetting(dosboxClientScriptFileName, dosboxScriptsCategoryValue, DOSBOX_CLIENT_SCRIPT_FILE_NAME_PROPERTY_NAME);
			assignStringSetting(dosboxServerScriptFileName, dosboxScriptsCategoryValue, DOSBOX_SERVER_SCRIPT_FILE_NAME_PROPERTY_NAME);
		}

		if(dosboxCategoryValue.HasMember(DOSBOX_NETWORKING_CATEGORY_NAME) && dosboxCategoryValue[DOSBOX_NETWORKING_CATEGORY_NAME].IsObject()) {
			const rapidjson::Value & dosboxNetworkingCategoryValue = dosboxCategoryValue[DOSBOX_NETWORKING_CATEGORY_NAME];

			if(dosboxNetworkingCategoryValue.HasMember(DOSBOX_SERVER_IP_ADDRESS_PROPERTY_NAME) && dosboxNetworkingCategoryValue[DOSBOX_SERVER_IP_ADDRESS_PROPERTY_NAME].IsString()) {
				std::string ipAddress = dosboxNetworkingCategoryValue[DOSBOX_SERVER_IP_ADDRESS_PROPERTY_NAME].GetString();

				if(Utilities::isIPV4Address(ipAddress)) {
					dosboxServerIPAddress = ipAddress;
				}
			}

			if(dosboxNetworkingCategoryValue.HasMember(DOSBOX_NETWORKING_PORTS_CATEGORY_NAME) && dosboxNetworkingCategoryValue[DOSBOX_NETWORKING_PORTS_CATEGORY_NAME].IsObject()) {
				const rapidjson::Value & dosboxNetworkingPortsCategoryValue = dosboxNetworkingCategoryValue[DOSBOX_NETWORKING_PORTS_CATEGORY_NAME];

				if(dosboxNetworkingPortsCategoryValue.HasMember(DOSBOX_LOCAL_SERVER_PORT_PROPERTY_NAME) && dosboxNetworkingPortsCategoryValue[DOSBOX_LOCAL_SERVER_PORT_PROPERTY_NAME].IsInt()) {
					int32_t port = dosboxNetworkingPortsCategoryValue[DOSBOX_LOCAL_SERVER_PORT_PROPERTY_NAME].GetInt();

					if(port >= 1 && port <= std::numeric_limits<uint16_t>::max()) {
						dosboxLocalServerPort = port;
					}
				}

				if(dosboxNetworkingPortsCategoryValue.HasMember(DOSBOX_REMOTE_SERVER_PORT_PROPERTY_NAME) && dosboxNetworkingPortsCategoryValue[DOSBOX_REMOTE_SERVER_PORT_PROPERTY_NAME].IsInt()) {
					int32_t port = dosboxNetworkingPortsCategoryValue[DOSBOX_REMOTE_SERVER_PORT_PROPERTY_NAME].GetInt();

					if(port >= 1 && port <= std::numeric_limits<uint16_t>::max()) {
						dosboxRemoteServerPort = static_cast<uint16_t>(port);
					}
				}
			}
		}
	}

	if(settingsDocument.HasMember(TIME_ZONE_CATEGORY_NAME) && settingsDocument[TIME_ZONE_CATEGORY_NAME].IsObject()) {
		const rapidjson::Value & timeZoneCategoryValue = settingsDocument[TIME_ZONE_CATEGORY_NAME];

		assignStringSetting(timeZoneDataDirectoryName, timeZoneCategoryValue, TIME_ZONE_DATA_DIRECTORY_NAME_PROPERTY_NAME);
	}

	if(settingsDocument.HasMember(CURL_CATEGORY_NAME) && settingsDocument[CURL_CATEGORY_NAME].IsObject()) {
		const rapidjson::Value & curlCategoryValue = settingsDocument[CURL_CATEGORY_NAME];

		assignStringSetting(curlDataDirectoryName, curlCategoryValue, CURL_DATA_DIRECTORY_NAME_PROPERTY_NAME);
		assignStringSetting(certificateAuthorityStoreFileName, curlCategoryValue, CURL_CERTIFICATE_AUTHORITY_STORE_FILE_NAME_PROPERTY_NAME);

		if(curlCategoryValue.HasMember(CURL_CONNECTION_TIMEOUT_PROPERTY_NAME) && curlCategoryValue[CURL_CONNECTION_TIMEOUT_PROPERTY_NAME].IsUint64()) {
			connectionTimeout = std::chrono::seconds(curlCategoryValue[CURL_CONNECTION_TIMEOUT_PROPERTY_NAME].GetUint64());
		}

		if(curlCategoryValue.HasMember(CURL_NETWORK_TIMEOUT_PROPERTY_NAME) && curlCategoryValue[CURL_NETWORK_TIMEOUT_PROPERTY_NAME].IsUint64()) {
			networkTimeout = std::chrono::seconds(curlCategoryValue[CURL_NETWORK_TIMEOUT_PROPERTY_NAME].GetUint64());
		}
	}

	if(settingsDocument.HasMember(API_CATEGORY_NAME) && settingsDocument[API_CATEGORY_NAME].IsObject()) {
		const rapidjson::Value & apiCategoryValue = settingsDocument[API_CATEGORY_NAME];

		assignStringSetting(apiBaseURL, apiCategoryValue, API_BASE_URL_PROPERTY_NAME);
		assignStringSetting(remoteModsListFileName, apiCategoryValue, REMOTE_MOD_LIST_FILE_NAME_PROPERTY_NAME);
		assignStringSetting(remoteGamesListFileName, apiCategoryValue, REMOTE_GAME_LIST_FILE_NAME_PROPERTY_NAME);
		assignStringSetting(remoteDownloadsDirectoryName, apiCategoryValue, REMOTE_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME);
		assignStringSetting(remoteModDownloadsDirectoryName, apiCategoryValue, REMOTE_MOD_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME);
		assignStringSetting(remoteMapDownloadsDirectoryName, apiCategoryValue, REMOTE_MAP_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME);
		assignStringSetting(remoteGameDownloadsDirectoryName, apiCategoryValue, REMOTE_GAME_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME);
	}

	if(settingsDocument.HasMember(ANALYTICS_CATEGORY_NAME) && settingsDocument[ANALYTICS_CATEGORY_NAME].IsObject()) {
		const rapidjson::Value & analyticsCategoryValue = settingsDocument[ANALYTICS_CATEGORY_NAME];

		if(analyticsCategoryValue.HasMember(SEGMENT_ANALYTICS_CATEGORY_NAME) && analyticsCategoryValue[SEGMENT_ANALYTICS_CATEGORY_NAME].IsObject()) {
			const rapidjson::Value & segmentAnalyticsCategoryValue = analyticsCategoryValue[SEGMENT_ANALYTICS_CATEGORY_NAME];

			assignBooleanSetting(segmentAnalyticsEnabled, segmentAnalyticsCategoryValue, SEGMENT_ANALYTICS_ENABLED_PROPERTY_NAME);
			assignStringSetting(segmentAnalyticsDataFileName, segmentAnalyticsCategoryValue, DEFAULT_SEGMENT_ANALYTICS_DATA_FILE_NAME);
		}
	}

	return true;
}

bool SettingsManager::load(const ArgumentParser * arguments, bool autoCreate) {
	if(arguments != nullptr) {
		std::string alternateSettingsFileName(arguments->getFirstValue("f"));

		if(!alternateSettingsFileName.empty()) {
			spdlog::debug("Loading settings from alternate file: '{}'...", alternateSettingsFileName);

			bool loadedSettings = loadFrom(alternateSettingsFileName, autoCreate);

			if(!loadedSettings) {
				spdlog::error("Failed to load settings from alt settings file: '{}'.", alternateSettingsFileName);
			}

			return loadedSettings;
		}
	}

	return loadFrom(DEFAULT_SETTINGS_FILE_PATH, autoCreate);
}

bool SettingsManager::save(const ArgumentParser * arguments, bool overwrite) const {
	if(arguments != nullptr) {
		std::string alternateSettingsFileName(arguments->getFirstValue("f"));

		if(!alternateSettingsFileName.empty()) {
			spdlog::debug("Saving settings to alternate file: '{}'...", alternateSettingsFileName);

			bool savedSettings = saveTo(alternateSettingsFileName, overwrite);

			if(!savedSettings) {
				spdlog::info("Failed to save settings to alternate file: '{}'.", alternateSettingsFileName);
			}

			return savedSettings;
		}
	}

	return saveTo(DEFAULT_SETTINGS_FILE_PATH, overwrite);
}

bool SettingsManager::loadFrom(const std::string & filePath, bool autoCreate) {
	if(filePath.empty()) {
		spdlog::error("Settings file path cannot be empty!");
		return false;
	}

	if(!std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		spdlog::error("Failed to open missing or invalid settings file: '{}'!", filePath);

		if(autoCreate) {
			saveTo(filePath);
		}

		return false;
	}

	std::ifstream fileStream(filePath);

	if(!fileStream.is_open()) {
		spdlog::error("Failed to open settings file '{}' for parsing!", filePath);
		return false;
	}

	rapidjson::Document settings;
	rapidjson::IStreamWrapper fileStreamWrapper(fileStream);
	if(settings.ParseStream(fileStreamWrapper).HasParseError()) {
		spdlog::error("Failed to parse settings file JSON data!");
		return false;
	}

	fileStream.close();

	if(!parseFrom(settings)) {
		spdlog::error("Failed to parse settings from file '{}!", filePath);
		return false;
	}

	spdlog::info("Settings successfully loaded from file '{}'.", filePath);

	return true;
}

bool SettingsManager::saveTo(const std::string & filePath, bool overwrite) const {
	if (!overwrite && std::filesystem::exists(std::filesystem::path(filePath))) {
		spdlog::warn("File '{}' already exists, use overwrite to force write.", filePath);
		return false;
	}

	std::ofstream fileStream(filePath);

	if(!fileStream.is_open()) {
		return false;
	}

	rapidjson::Document settings(toJSON());

	rapidjson::OStreamWrapper fileStreamWrapper(fileStream);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> fileStreamWriter(fileStreamWrapper);
	fileStreamWriter.SetIndent('\t', 1);
	settings.Accept(fileStreamWriter);
	fileStream.close();

	spdlog::info("Settings successfully saved to file '{}'.", filePath);

	return true;
}
