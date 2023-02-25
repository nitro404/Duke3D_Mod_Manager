#include "SettingsManager.h"

#include "Game/GameVersion.h"
#include "ModManager.h"

#include <Arguments/ArgumentParser.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/TimeUtilities.h>

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

static constexpr const char * FILE_FORMAT_VERSION_PROPERTY_NAME = "fileFormatVersion";
static constexpr const char * GAME_TYPE_PROPERTY_NAME = "gameType";
static constexpr const char * DATA_DIRECTORY_PATH_PROPERTY_NAME = "dataDirectoryPath";
static constexpr const char * APP_TEMP_DIRECTORY_PATH_PROPERTY_NAME = "appTempDirectoryPath";
static constexpr const char * GAME_TEMP_DIRECTORY_NAME_PROPERTY_NAME = "gameTempDirectoryName";
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
static constexpr const char * DOSBOX_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME = "dosbox";
static constexpr const char * GAME_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME = "games";

static constexpr const char * CACHE_CATEGORY_NAME = "cache";
static constexpr const char * CACHE_DIRECTORY_PATH_PROPERTY_NAME = DIRECTORY_PATH;

static constexpr const char * DOSBOX_CATEGORY_NAME = "dosbox";
static constexpr const char * DOSBOX_VERSIONS_LIST_FILE_PATH_PROPERTY_NAME = LIST_FILE_PATH;
static constexpr const char * PREFERRED_DOSBOX_VERSION_PROPERTY_NAME = "preferred";
static constexpr const char * DOSBOX_ARGUMENTS_PROPERTY_NAME = "arguments";
static constexpr const char * DOSBOX_DATA_DIRECTORY_NAME_PROPERTY_NAME = DATA_DIRECTORY_NAME;

static constexpr const char * DOSBOX_NETWORKING_CATEGORY_NAME = "networking";
static constexpr const char * DOSBOX_SERVER_IP_ADDRESS_PROPERTY_NAME = "serverIPAddress";

static constexpr const char * DOSBOX_NETWORKING_PORTS_CATEGORY_NAME = "ports";
static constexpr const char * DOSBOX_LOCAL_SERVER_PORT_PROPERTY_NAME = "local";
static constexpr const char * DOSBOX_REMOTE_SERVER_PORT_PROPERTY_NAME = "remote";

static constexpr const char * TIME_ZONE_CATEGORY_NAME = "timeZone";
static constexpr const char * TIME_ZONE_DATA_DIRECTORY_NAME_PROPERTY_NAME = DATA_DIRECTORY_NAME;

static constexpr const char * CURL_CATEGORY_NAME = "curl";
static constexpr const char * CURL_DATA_DIRECTORY_NAME_PROPERTY_NAME = DATA_DIRECTORY_NAME;
static constexpr const char * CURL_CONNECTION_TIMEOUT_PROPERTY_NAME = "connectionTimeout";
static constexpr const char * CURL_NETWORK_TIMEOUT_PROPERTY_NAME = "networkTimeout";
static constexpr const char * CURL_TRANSFER_TIMEOUT_PROPERTY_NAME = "transferTimeout";
static constexpr const char * CURL_VERBOSE_REQUEST_LOGGING_PROPERTY_NAME = "verboseRequestLogging";

static constexpr const char * API_CATEGORY_NAME = "api";
static constexpr const char * API_BASE_URL_PROPERTY_NAME = "baseURL";
static constexpr const char * REMOTE_MOD_LIST_FILE_NAME_PROPERTY_NAME = "remoteModListFileName";
static constexpr const char * REMOTE_DOSBOX_VERSIONS_LIST_FILE_NAME_PROPERTY_NAME = "remoteDOSBoxVersionListFileName";
static constexpr const char * REMOTE_GAME_LIST_FILE_NAME_PROPERTY_NAME = "remoteGameListFileName";
static constexpr const char * REMOTE_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME = "remoteDownloadsDirectoryName";
static constexpr const char * REMOTE_MOD_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME = "remoteModDownloadsDirectoryName";
static constexpr const char * REMOTE_MAP_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME = "remoteMapDownloadsDirectoryName";
static constexpr const char * REMOTE_DOSBOX_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME = "remoteDOSBoxDownloadsDirectoryName";
static constexpr const char * REMOTE_GAME_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME = "remoteGameDownloadsDirectoryName";

static constexpr const char * ANALYTICS_CATEGORY_NAME = "analytics";
static constexpr const char * SEGMENT_ANALYTICS_CATEGORY_NAME = "segment";
static constexpr const char * SEGMENT_ANALYTICS_ENABLED_PROPERTY_NAME = "enabled";
static constexpr const char * SEGMENT_ANALYTICS_DATA_FILE_NAME_PROPERTY_NAME = "dataFileName";

static constexpr const char * FILE_ETAGS_PROPERTY_NAME = "fileETags";

static constexpr const char * DOWNLOAD_THROTTLING_CATEGORY_NAME = "downloadThrottling";
static constexpr const char * DOWNLOAD_THROTTLING_ENABLED_PROPERTY_NAME = "enabled";
static constexpr const char * MOD_LIST_LAST_DOWNLOADED_PROPERTY_NAME = "modListLastDownloaded";
static constexpr const char * MOD_LIST_UPDATE_FREQUENCY_PROPERTY_NAME = "modListUpdateFrequency";
static constexpr const char * DOSBOX_DOWNLOAD_LIST_LAST_DOWNLOADED_PROPERTY_NAME = "dosboxDownloadListLastDownloaded";
static constexpr const char * DOSBOX_DOWNLOAD_LIST_UPDATE_FREQUENCY_PROPERTY_NAME = "dosboxDownloadListUpdateFrequency";
static constexpr const char * GAME_DOWNLOAD_LIST_LAST_DOWNLOADED_PROPERTY_NAME = "gameDownloadListLastDownloaded";
static constexpr const char * GAME_DOWNLOAD_LIST_UPDATE_FREQUENCY_PROPERTY_NAME = "gameDownloadListUpdateFrequency";
static constexpr const char * CACERT_LAST_DOWNLOADED_PROPERTY_NAME = "cacertLastDownloaded";
static constexpr const char * CACERT_UPDATE_FREQUENCY_PROPERTY_NAME = "cacertUpdateFrequency";
static constexpr const char * TIME_ZONE_DATA_LAST_DOWNLOADED_PROPERTY_NAME = "timeZoneDataLastDownloaded";
static constexpr const char * TIME_ZONE_DATA_UPDATE_FREQUENCY_PROPERTY_NAME = "timeZoneDataUpdateFrequency";

static constexpr const char * WINDOW_CATEGORY_NAME = "window";

static constexpr const char * WINDOW_POSITION_PROPERTY_NAME = "position";
static constexpr const char * WINDOW_SIZE_PROPERTY_NAME = "size";

const std::string SettingsManager::FILE_FORMAT_VERSION("1.0.0");
const std::string SettingsManager::DEFAULT_SETTINGS_FILE_PATH("Duke Nukem 3D Mod Manager Settings.json");
const std::string SettingsManager::DEFAULT_MODS_LIST_FILE_PATH("Duke Nukem 3D Mod List.xml");
const std::string SettingsManager::DEFAULT_FAVOURITE_MODS_LIST_FILE_PATH("Duke Nukem 3D Favourite Mods.json");
const std::string SettingsManager::DEFAULT_GAME_VERSIONS_LIST_FILE_PATH("Duke Nukem 3D Game Versions.json");
const std::string SettingsManager::DEFAULT_GAME_SYMLINK_NAME("Game");
const bool SettingsManager::DEFAULT_LOCAL_MODE = false;
const std::string SettingsManager::DEFAULT_MODS_DIRECTORY_PATH("Mods");
const std::string SettingsManager::DEFAULT_MODS_SYMLINK_NAME("Mods");
const std::string SettingsManager::DEFAULT_MOD_PACKAGE_DOWNLOADS_DIRECTORY_PATH("");
const std::string SettingsManager::DEFAULT_MOD_IMAGES_DIRECTORY_PATH("");
const std::string SettingsManager::DEFAULT_MOD_SOURCE_FILES_DIRECTORY_PATH("");
const std::string SettingsManager::DEFAULT_MAPS_DIRECTORY_PATH("");
const std::string SettingsManager::DEFAULT_MAPS_SYMLINK_NAME("Maps");
const std::string SettingsManager::DEFAULT_DOWNLOADS_DIRECTORY_PATH("Downloads");
const std::string SettingsManager::DEFAULT_DOWNLOAD_CACHE_FILE_NAME("Download Cache.json");
const std::string SettingsManager::DEFAULT_MOD_DOWNLOADS_DIRECTORY_NAME("Mods");
const std::string SettingsManager::DEFAULT_MAP_DOWNLOADS_DIRECTORY_NAME("Maps");
const std::string SettingsManager::DEFAULT_DOSBOX_DOWNLOADS_DIRECTORY_NAME("DOSBox");
const std::string SettingsManager::DEFAULT_GAME_DOWNLOADS_DIRECTORY_NAME("Games");
const std::string SettingsManager::DEFAULT_DATA_DIRECTORY_PATH("Data");
const std::string SettingsManager::DEFAULT_APP_TEMP_DIRECTORY_PATH("Temp");
const std::string SettingsManager::DEFAULT_GAME_TEMP_DIRECTORY_NAME("Temp");
const std::string SettingsManager::DEFAULT_TEMP_SYMLINK_NAME("Temp");
const std::string SettingsManager::DEFAULT_CACHE_DIRECTORY_PATH("Cache");
const std::string SettingsManager::DEFAULT_DOSBOX_ARGUMENTS("-noconsole");
const std::string SettingsManager::DEFAULT_DOSBOX_DATA_DIRECTORY_NAME("DOSBox");
const GameType SettingsManager::DEFAULT_GAME_TYPE = ModManager::DEFAULT_GAME_TYPE;
const std::string SettingsManager::DEFAULT_PREFERRED_GAME_VERSION(ModManager::DEFAULT_PREFERRED_GAME_VERSION);
const std::string SettingsManager::DEFAULT_DOSBOX_VERSIONS_LIST_FILE_PATH("DOSBox Versions.json");
const std::string SettingsManager::DEFAULT_PREFERRED_DOSBOX_VERSION(ModManager::DEFAULT_PREFERRED_DOSBOX_VERSION);
const std::string SettingsManager::DEFAULT_DOSBOX_SERVER_IP_ADDRESS("127.0.0.1");
const uint16_t SettingsManager::DEFAULT_DOSBOX_LOCAL_SERVER_PORT = 31337;
const uint16_t SettingsManager::DEFAULT_DOSBOX_REMOTE_SERVER_PORT = 31337;
const std::string SettingsManager::DEFAULT_TIME_ZONE_DATA_DIRECTORY_NAME("Time Zone");
const std::string SettingsManager::DEFAULT_CURL_DATA_DIRECTORY_NAME("cURL");
const std::chrono::seconds SettingsManager::DEFAULT_CONNECTION_TIMEOUT = 15s;
const std::chrono::seconds SettingsManager::DEFAULT_NETWORK_TIMEOUT = 30s;
const std::chrono::seconds SettingsManager::DEFAULT_TRANSFER_TIMEOUT = 0s;
const bool SettingsManager::DEFAULT_VERBOSE_REQUEST_LOGGING = false;
const std::string SettingsManager::DEFAULT_API_BASE_URL("http://duke3dmods.com");
const std::string SettingsManager::DEFAULT_REMOTE_MODS_LIST_FILE_NAME("duke3d_mods.xml");
const std::string SettingsManager::DEFAULT_REMOTE_DOSBOX_VERSIONS_LIST_FILE_NAME("dosbox_versions.json");
const std::string SettingsManager::DEFAULT_REMOTE_GAMES_LIST_FILE_NAME("duke3d_games.json");
const std::string SettingsManager::DEFAULT_REMOTE_DOWNLOADS_DIRECTORY_NAME("downloads");
const std::string SettingsManager::DEFAULT_REMOTE_MOD_DOWNLOADS_DIRECTORY_NAME("mods");
const std::string SettingsManager::DEFAULT_REMOTE_MAP_DOWNLOADS_DIRECTORY_NAME("maps");
const std::string SettingsManager::DEFAULT_REMOTE_DOSBOX_DOWNLOADS_DIRECTORY_NAME("dosbox");
const std::string SettingsManager::DEFAULT_REMOTE_GAME_DOWNLOADS_DIRECTORY_NAME("games");
const bool SettingsManager::DEFAULT_SEGMENT_ANALYTICS_ENABLED = true;
const std::string SettingsManager::DEFAULT_SEGMENT_ANALYTICS_DATA_FILE_NAME("Segment Analytics Cache.json");
const bool SettingsManager::DEFAULT_DOWNLOAD_THROTTLING_ENABLED = true;
const std::chrono::minutes SettingsManager::DEFAULT_MOD_LIST_UPDATE_FREQUENCY = std::chrono::minutes(30);
const std::chrono::minutes SettingsManager::DEFAULT_DOSBOX_DOWNLOAD_LIST_UPDATE_FREQUENCY = std::chrono::minutes(120);
const std::chrono::minutes SettingsManager::DEFAULT_GAME_DOWNLOAD_LIST_UPDATE_FREQUENCY = std::chrono::minutes(60);
const std::chrono::minutes SettingsManager::DEFAULT_CACERT_UPDATE_FREQUENCY = std::chrono::hours(2 * 24 * 7); // 2 weeks
const std::chrono::minutes SettingsManager::DEFAULT_TIME_ZONE_DATA_UPDATE_FREQUENCY = std::chrono::hours(1 * 24 * 7); // 1 week
const Point SettingsManager::DEFAULT_WINDOW_POSITION(-1, -1);
const Dimension SettingsManager::DEFAULT_WINDOW_SIZE(1024, 768);
const Dimension SettingsManager::MINIMUM_WINDOW_SIZE(800, 600);

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

static bool assignOptionalTimePointSetting(std::optional<std::chrono::time_point<std::chrono::system_clock>> & setting, const rapidjson::Value & categoryValue, const std::string & propertyName) {
	if(propertyName.empty() || !categoryValue.IsObject() || !categoryValue.HasMember(propertyName.c_str())) {
		return false;
	}

	const rapidjson::Value & settingValue = categoryValue[propertyName.c_str()];

	if(!settingValue.IsString()) {
		return false;
	}

	setting = Utilities::parseTimePointFromString(settingValue.GetString());

	return setting.has_value();
}

template <typename T>
static bool assignChronoSetting(T & setting, const rapidjson::Value & categoryValue, const std::string & propertyName) {
	if(propertyName.empty() || !categoryValue.IsObject() || !categoryValue.HasMember(propertyName.c_str())) {
		return false;
	}

	const rapidjson::Value & settingValue = categoryValue[propertyName.c_str()];

	if(!settingValue.IsUint64()) {
		return false;
	}

	setting = T(settingValue.GetUint64());

	return true;
}

static bool assignPointSetting(Point & setting, const rapidjson::Value & categoryValue, const std::string & propertyName) {
	if(propertyName.empty() || !categoryValue.IsObject() || !categoryValue.HasMember(propertyName.c_str())) {
		return false;
	}

	const rapidjson::Value & settingValue = categoryValue[propertyName.c_str()];

	std::optional<Point> optionalPoint(Point::parseFrom(settingValue));

	if(!optionalPoint.has_value()) {
		return false;
	}

	setting = std::move(optionalPoint.value());

	return true;
}

static bool assignDimensionSetting(Dimension & setting, const rapidjson::Value & categoryValue, const std::string & propertyName) {
	if(propertyName.empty() || !categoryValue.IsObject() || !categoryValue.HasMember(propertyName.c_str())) {
		return false;
	}

	const rapidjson::Value & settingValue = categoryValue[propertyName.c_str()];

	std::optional<Dimension> optionalDimension(Dimension::parseFrom(settingValue));

	if(!optionalDimension.has_value()) {
		return false;
	}

	setting = std::move(optionalDimension.value());

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
	, appTempDirectoryPath(DEFAULT_APP_TEMP_DIRECTORY_PATH)
	, gameTempDirectoryName(DEFAULT_GAME_TEMP_DIRECTORY_NAME)
	, tempSymlinkName(DEFAULT_TEMP_SYMLINK_NAME)
	, cacheDirectoryPath(DEFAULT_CACHE_DIRECTORY_PATH)
	, dosboxArguments(DEFAULT_DOSBOX_ARGUMENTS)
	, dosboxDataDirectoryName(DEFAULT_DOSBOX_DATA_DIRECTORY_NAME)
	, dosboxServerIPAddress(DEFAULT_DOSBOX_SERVER_IP_ADDRESS)
	, gameType(DEFAULT_GAME_TYPE)
	, preferredGameVersion(DEFAULT_PREFERRED_GAME_VERSION)
	, dosboxVersionsListFilePath(DEFAULT_DOSBOX_VERSIONS_LIST_FILE_PATH)
	, preferredDOSBoxVersion(DEFAULT_PREFERRED_DOSBOX_VERSION)
	, dosboxLocalServerPort(DEFAULT_DOSBOX_LOCAL_SERVER_PORT)
	, dosboxRemoteServerPort(DEFAULT_DOSBOX_REMOTE_SERVER_PORT)
	, timeZoneDataDirectoryName(DEFAULT_TIME_ZONE_DATA_DIRECTORY_NAME)
	, curlDataDirectoryName(DEFAULT_CURL_DATA_DIRECTORY_NAME)
	, connectionTimeout(DEFAULT_CONNECTION_TIMEOUT)
	, networkTimeout(DEFAULT_NETWORK_TIMEOUT)
	, transferTimeout(DEFAULT_TRANSFER_TIMEOUT)
	, verboseRequestLogging(DEFAULT_VERBOSE_REQUEST_LOGGING)
	, apiBaseURL(DEFAULT_API_BASE_URL)
	, remoteModsListFileName(DEFAULT_REMOTE_MODS_LIST_FILE_NAME)
	, remoteGamesListFileName(DEFAULT_REMOTE_GAMES_LIST_FILE_NAME)
	, remoteDownloadsDirectoryName(DEFAULT_REMOTE_DOWNLOADS_DIRECTORY_NAME)
	, remoteModDownloadsDirectoryName(DEFAULT_REMOTE_MOD_DOWNLOADS_DIRECTORY_NAME)
	, remoteMapDownloadsDirectoryName(DEFAULT_REMOTE_MAP_DOWNLOADS_DIRECTORY_NAME)
	, remoteGameDownloadsDirectoryName(DEFAULT_REMOTE_GAME_DOWNLOADS_DIRECTORY_NAME)
	, segmentAnalyticsEnabled(DEFAULT_SEGMENT_ANALYTICS_ENABLED)
	, downloadThrottlingEnabled(DEFAULT_DOWNLOAD_THROTTLING_ENABLED)
	, modListUpdateFrequency(DEFAULT_MOD_LIST_UPDATE_FREQUENCY)
	, gameDownloadListUpdateFrequency(DEFAULT_GAME_DOWNLOAD_LIST_UPDATE_FREQUENCY)
	, cacertUpdateFrequency(DEFAULT_CACERT_UPDATE_FREQUENCY)
	, timeZoneDataUpdateFrequency(DEFAULT_TIME_ZONE_DATA_UPDATE_FREQUENCY)
	, segmentAnalyticsDataFileName(DEFAULT_SEGMENT_ANALYTICS_DATA_FILE_NAME)
	, windowPosition(DEFAULT_WINDOW_POSITION)
	, windowSize(DEFAULT_WINDOW_SIZE) { }

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
	dosboxDownloadsDirectoryName = DEFAULT_DOSBOX_DOWNLOADS_DIRECTORY_NAME;
	gameDownloadsDirectoryName = DEFAULT_GAME_DOWNLOADS_DIRECTORY_NAME;
	dataDirectoryPath = DEFAULT_DATA_DIRECTORY_PATH;
	appTempDirectoryPath = DEFAULT_APP_TEMP_DIRECTORY_PATH;
	gameTempDirectoryName = DEFAULT_GAME_TEMP_DIRECTORY_NAME;
	tempSymlinkName = DEFAULT_TEMP_SYMLINK_NAME;
	cacheDirectoryPath = DEFAULT_CACHE_DIRECTORY_PATH;
	dosboxArguments = DEFAULT_DOSBOX_ARGUMENTS;
	dosboxDataDirectoryName = DEFAULT_DOSBOX_DATA_DIRECTORY_NAME;
	dosboxServerIPAddress = DEFAULT_DOSBOX_SERVER_IP_ADDRESS;
	gameType = DEFAULT_GAME_TYPE;
	preferredGameVersion = DEFAULT_PREFERRED_GAME_VERSION;
	dosboxVersionsListFilePath = DEFAULT_DOSBOX_VERSIONS_LIST_FILE_PATH;
	preferredDOSBoxVersion = DEFAULT_PREFERRED_DOSBOX_VERSION;
	dosboxLocalServerPort = DEFAULT_DOSBOX_LOCAL_SERVER_PORT;
	dosboxRemoteServerPort = DEFAULT_DOSBOX_REMOTE_SERVER_PORT;
	timeZoneDataDirectoryName = DEFAULT_TIME_ZONE_DATA_DIRECTORY_NAME;
	curlDataDirectoryName = DEFAULT_CURL_DATA_DIRECTORY_NAME;
	connectionTimeout = DEFAULT_CONNECTION_TIMEOUT;
	networkTimeout = DEFAULT_NETWORK_TIMEOUT;
	transferTimeout = DEFAULT_TRANSFER_TIMEOUT;
	verboseRequestLogging = DEFAULT_VERBOSE_REQUEST_LOGGING;
	apiBaseURL = DEFAULT_API_BASE_URL;
	remoteModsListFileName = DEFAULT_REMOTE_MODS_LIST_FILE_NAME;
	remoteDOSBoxVersionsListFileName = DEFAULT_REMOTE_DOSBOX_VERSIONS_LIST_FILE_NAME;
	remoteGamesListFileName = DEFAULT_REMOTE_GAMES_LIST_FILE_NAME;
	remoteDownloadsDirectoryName = DEFAULT_REMOTE_DOWNLOADS_DIRECTORY_NAME;
	remoteModDownloadsDirectoryName = DEFAULT_REMOTE_MOD_DOWNLOADS_DIRECTORY_NAME;
	remoteMapDownloadsDirectoryName = DEFAULT_REMOTE_MAP_DOWNLOADS_DIRECTORY_NAME;
	remoteDOSBoxDownloadsDirectoryName = DEFAULT_REMOTE_DOSBOX_DOWNLOADS_DIRECTORY_NAME;
	remoteGameDownloadsDirectoryName = DEFAULT_REMOTE_GAME_DOWNLOADS_DIRECTORY_NAME;
	segmentAnalyticsEnabled = DEFAULT_SEGMENT_ANALYTICS_ENABLED;
	segmentAnalyticsDataFileName = DEFAULT_SEGMENT_ANALYTICS_DATA_FILE_NAME;
	downloadThrottlingEnabled = DEFAULT_DOWNLOAD_THROTTLING_ENABLED;
	modListLastDownloadedTimestamp.reset();
	modListUpdateFrequency = DEFAULT_MOD_LIST_UPDATE_FREQUENCY;
	dosboxDownloadListLastDownloadedTimestamp.reset();
	dosboxDownloadListUpdateFrequency = DEFAULT_DOSBOX_DOWNLOAD_LIST_UPDATE_FREQUENCY;
	gameDownloadListLastDownloadedTimestamp.reset();
	gameDownloadListUpdateFrequency = DEFAULT_GAME_DOWNLOAD_LIST_UPDATE_FREQUENCY;
	cacertLastDownloadedTimestamp.reset();
	cacertUpdateFrequency = DEFAULT_CACERT_UPDATE_FREQUENCY;
	timeZoneDataLastDownloadedTimestamp.reset();
	timeZoneDataUpdateFrequency = DEFAULT_TIME_ZONE_DATA_UPDATE_FREQUENCY;
	windowPosition = DEFAULT_WINDOW_POSITION;
	windowSize = DEFAULT_WINDOW_SIZE;
	fileETags.clear();
}

rapidjson::Document SettingsManager::toJSON() const {
	rapidjson::Document settingsDocument(rapidjson::kObjectType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = settingsDocument.GetAllocator();

	rapidjson::Value fileFormatVersionValue(FILE_FORMAT_VERSION.c_str(), allocator);
	settingsDocument.AddMember(rapidjson::StringRef(FILE_FORMAT_VERSION_PROPERTY_NAME), fileFormatVersionValue, allocator);
	rapidjson::Value gameTypeValue(Utilities::toCapitalCase(magic_enum::enum_name(gameType)).c_str(), allocator);
	settingsDocument.AddMember(rapidjson::StringRef(GAME_TYPE_PROPERTY_NAME), gameTypeValue, allocator);
	rapidjson::Value dataDirectoryPathValue(dataDirectoryPath.c_str(), allocator);
	settingsDocument.AddMember(rapidjson::StringRef(DATA_DIRECTORY_PATH_PROPERTY_NAME), dataDirectoryPathValue, allocator);
	rapidjson::Value appTempDirectoryPathValue(appTempDirectoryPath.c_str(), allocator);
	settingsDocument.AddMember(rapidjson::StringRef(APP_TEMP_DIRECTORY_PATH_PROPERTY_NAME), appTempDirectoryPathValue, allocator);
	rapidjson::Value gameTempDirectoryNameValue(gameTempDirectoryName.c_str(), allocator);
	settingsDocument.AddMember(rapidjson::StringRef(GAME_TEMP_DIRECTORY_NAME_PROPERTY_NAME), gameTempDirectoryNameValue, allocator);
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
	rapidjson::Value dosboxDownloadsDirectoryNameValue(dosboxDownloadsDirectoryName.c_str(), allocator);
	downloadsCategoryValue.AddMember(rapidjson::StringRef(DOSBOX_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME), dosboxDownloadsDirectoryNameValue, allocator);
	rapidjson::Value gameDownloadsDirectoryNameValue(gameDownloadsDirectoryName.c_str(), allocator);
	downloadsCategoryValue.AddMember(rapidjson::StringRef(GAME_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME), gameDownloadsDirectoryNameValue, allocator);

	settingsDocument.AddMember(rapidjson::StringRef(DOWNLOADS_CATEGORY_NAME), downloadsCategoryValue, allocator);

	rapidjson::Value cacheCategoryValue(rapidjson::kObjectType);

	rapidjson::Value cacheDirectoryPathValue(cacheDirectoryPath.c_str(), allocator);
	cacheCategoryValue.AddMember(rapidjson::StringRef(CACHE_DIRECTORY_PATH_PROPERTY_NAME), cacheDirectoryPathValue, allocator);

	settingsDocument.AddMember(rapidjson::StringRef(CACHE_CATEGORY_NAME), cacheCategoryValue, allocator);

	rapidjson::Value dosboxCategoryValue(rapidjson::kObjectType);

	rapidjson::Value dosboxVersionsListFilePathValue(dosboxVersionsListFilePath.c_str(), allocator);
	dosboxCategoryValue.AddMember(rapidjson::StringRef(DOSBOX_VERSIONS_LIST_FILE_PATH_PROPERTY_NAME), dosboxVersionsListFilePathValue, allocator);
	rapidjson::Value preferredDOSBoxVersionValue(preferredDOSBoxVersion.c_str(), allocator);
	dosboxCategoryValue.AddMember(rapidjson::StringRef(PREFERRED_DOSBOX_VERSION_PROPERTY_NAME), preferredDOSBoxVersionValue, allocator);
	rapidjson::Value dosboxArgumentsValue(dosboxArguments.c_str(), allocator);
	dosboxCategoryValue.AddMember(rapidjson::StringRef(DOSBOX_ARGUMENTS_PROPERTY_NAME), dosboxArgumentsValue, allocator);
	rapidjson::Value dosboxDataDirectroryNameValue(dosboxDataDirectoryName.c_str(), allocator);
	dosboxCategoryValue.AddMember(rapidjson::StringRef(DOSBOX_DATA_DIRECTORY_NAME_PROPERTY_NAME), dosboxDataDirectroryNameValue, allocator);

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
	curlCategoryValue.AddMember(rapidjson::StringRef(CURL_CONNECTION_TIMEOUT_PROPERTY_NAME), rapidjson::Value(connectionTimeout.count()), allocator);
	curlCategoryValue.AddMember(rapidjson::StringRef(CURL_NETWORK_TIMEOUT_PROPERTY_NAME), rapidjson::Value(networkTimeout.count()), allocator);
	curlCategoryValue.AddMember(rapidjson::StringRef(CURL_TRANSFER_TIMEOUT_PROPERTY_NAME), rapidjson::Value(transferTimeout.count()), allocator);
	curlCategoryValue.AddMember(rapidjson::StringRef(CURL_VERBOSE_REQUEST_LOGGING_PROPERTY_NAME), rapidjson::Value(verboseRequestLogging), allocator);

	settingsDocument.AddMember(rapidjson::StringRef(CURL_CATEGORY_NAME), curlCategoryValue, allocator);

	rapidjson::Value apiCategoryValue(rapidjson::kObjectType);

	rapidjson::Value apiBaseURLValue(apiBaseURL.c_str(), allocator);
	apiCategoryValue.AddMember(rapidjson::StringRef(API_BASE_URL_PROPERTY_NAME), apiBaseURLValue, allocator);
	rapidjson::Value remoteModsListFileNameValue(remoteModsListFileName.c_str(), allocator);
	apiCategoryValue.AddMember(rapidjson::StringRef(REMOTE_MOD_LIST_FILE_NAME_PROPERTY_NAME), remoteModsListFileNameValue, allocator);
	rapidjson::Value remoteDOSBoxListFileNameValue(remoteDOSBoxVersionsListFileName.c_str(), allocator);
	apiCategoryValue.AddMember(rapidjson::StringRef(REMOTE_DOSBOX_VERSIONS_LIST_FILE_NAME_PROPERTY_NAME), remoteDOSBoxListFileNameValue, allocator);
	rapidjson::Value remoteGamesListFileNameValue(remoteGamesListFileName.c_str(), allocator);
	apiCategoryValue.AddMember(rapidjson::StringRef(REMOTE_GAME_LIST_FILE_NAME_PROPERTY_NAME), remoteGamesListFileNameValue, allocator);
	rapidjson::Value remoteDownloadsDirectoryNameValue(remoteDownloadsDirectoryName.c_str(), allocator);
	apiCategoryValue.AddMember(rapidjson::StringRef(REMOTE_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME), remoteDownloadsDirectoryNameValue, allocator);
	rapidjson::Value remoteModDownloadsDirectoryNameValue(remoteModDownloadsDirectoryName.c_str(), allocator);
	apiCategoryValue.AddMember(rapidjson::StringRef(REMOTE_MOD_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME), remoteModDownloadsDirectoryNameValue, allocator);
	rapidjson::Value remoteMapDownloadsDirectoryNameValue(remoteMapDownloadsDirectoryName.c_str(), allocator);
	apiCategoryValue.AddMember(rapidjson::StringRef(REMOTE_MAP_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME), remoteMapDownloadsDirectoryNameValue, allocator);
	rapidjson::Value remoteDOSBoxDownloadsDirectoryNameValue(remoteDOSBoxDownloadsDirectoryName.c_str(), allocator);
	apiCategoryValue.AddMember(rapidjson::StringRef(REMOTE_DOSBOX_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME), remoteDOSBoxDownloadsDirectoryNameValue, allocator);
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

	rapidjson::Value downloadThrottlingCategoryValue(rapidjson::kObjectType);

	downloadThrottlingCategoryValue.AddMember(rapidjson::StringRef(DOWNLOAD_THROTTLING_ENABLED_PROPERTY_NAME), rapidjson::Value(downloadThrottlingEnabled), allocator);

	if(modListLastDownloadedTimestamp.has_value()) {
		rapidjson::Value modListLastDownloadedValue(Utilities::timePointToString(modListLastDownloadedTimestamp.value(), Utilities::TimeFormat::ISO8601).c_str(), allocator);
		downloadThrottlingCategoryValue.AddMember(rapidjson::StringRef(MOD_LIST_LAST_DOWNLOADED_PROPERTY_NAME), modListLastDownloadedValue, allocator);
	}

	downloadThrottlingCategoryValue.AddMember(rapidjson::StringRef(MOD_LIST_UPDATE_FREQUENCY_PROPERTY_NAME), rapidjson::Value(modListUpdateFrequency.count()), allocator);

	if(dosboxDownloadListLastDownloadedTimestamp.has_value()) {
		rapidjson::Value dosboxDownloadListLastDownloadedValue(Utilities::timePointToString(dosboxDownloadListLastDownloadedTimestamp.value(), Utilities::TimeFormat::ISO8601).c_str(), allocator);
		downloadThrottlingCategoryValue.AddMember(rapidjson::StringRef(DOSBOX_DOWNLOAD_LIST_LAST_DOWNLOADED_PROPERTY_NAME), dosboxDownloadListLastDownloadedValue, allocator);
	}

	downloadThrottlingCategoryValue.AddMember(rapidjson::StringRef(DOSBOX_DOWNLOAD_LIST_UPDATE_FREQUENCY_PROPERTY_NAME), rapidjson::Value(dosboxDownloadListUpdateFrequency.count()), allocator);

	if(gameDownloadListLastDownloadedTimestamp.has_value()) {
		rapidjson::Value gameDownloadListLastDownloadedValue(Utilities::timePointToString(gameDownloadListLastDownloadedTimestamp.value(), Utilities::TimeFormat::ISO8601).c_str(), allocator);
		downloadThrottlingCategoryValue.AddMember(rapidjson::StringRef(GAME_DOWNLOAD_LIST_LAST_DOWNLOADED_PROPERTY_NAME), gameDownloadListLastDownloadedValue, allocator);
	}

	downloadThrottlingCategoryValue.AddMember(rapidjson::StringRef(GAME_DOWNLOAD_LIST_UPDATE_FREQUENCY_PROPERTY_NAME), rapidjson::Value(gameDownloadListUpdateFrequency.count()), allocator);

	if(cacertLastDownloadedTimestamp.has_value()) {
		rapidjson::Value cacertLastDownloadedValue(Utilities::timePointToString(cacertLastDownloadedTimestamp.value(), Utilities::TimeFormat::ISO8601).c_str(), allocator);
		downloadThrottlingCategoryValue.AddMember(rapidjson::StringRef(CACERT_LAST_DOWNLOADED_PROPERTY_NAME), cacertLastDownloadedValue, allocator);
	}

	downloadThrottlingCategoryValue.AddMember(rapidjson::StringRef(CACERT_UPDATE_FREQUENCY_PROPERTY_NAME), rapidjson::Value(cacertUpdateFrequency.count()), allocator);

	if(timeZoneDataLastDownloadedTimestamp.has_value()) {
		rapidjson::Value timeZoneDataLastDownloadedValue(Utilities::timePointToString(timeZoneDataLastDownloadedTimestamp.value(), Utilities::TimeFormat::ISO8601).c_str(), allocator);
		downloadThrottlingCategoryValue.AddMember(rapidjson::StringRef(TIME_ZONE_DATA_LAST_DOWNLOADED_PROPERTY_NAME), timeZoneDataLastDownloadedValue, allocator);
	}

	downloadThrottlingCategoryValue.AddMember(rapidjson::StringRef(TIME_ZONE_DATA_UPDATE_FREQUENCY_PROPERTY_NAME), rapidjson::Value(timeZoneDataUpdateFrequency.count()), allocator);

	settingsDocument.AddMember(rapidjson::StringRef(DOWNLOAD_THROTTLING_CATEGORY_NAME), downloadThrottlingCategoryValue, allocator);

	rapidjson::Value windowCategoryValue(rapidjson::kObjectType);

	rapidjson::Value windowPositionValue(windowPosition.toJSON(allocator));
	windowCategoryValue.AddMember(rapidjson::StringRef(WINDOW_POSITION_PROPERTY_NAME), windowPositionValue, allocator);
	rapidjson::Value windowSizeValue(windowSize.toJSON(allocator));
	windowCategoryValue.AddMember(rapidjson::StringRef(WINDOW_SIZE_PROPERTY_NAME), windowSizeValue, allocator);

	settingsDocument.AddMember(rapidjson::StringRef(WINDOW_CATEGORY_NAME), windowCategoryValue, allocator);

	rapidjson::Value fileETagsValue(rapidjson::kObjectType);

	for(std::map<std::string, std::string>::const_iterator i = fileETags.begin(); i != fileETags.end(); ++i) {
		rapidjson::Value fileNameValue(i->first.c_str(), allocator);
		rapidjson::Value fileETagValue(i->second.c_str(), allocator);
		fileETagsValue.AddMember(fileNameValue, fileETagValue, allocator);
	}

	settingsDocument.AddMember(rapidjson::StringRef(FILE_ETAGS_PROPERTY_NAME), fileETagsValue, allocator);

	return settingsDocument;
}

bool SettingsManager::parseFrom(const rapidjson::Value & settingsDocument) {
	if(!settingsDocument.IsObject()) {
		spdlog::error("Invalid settings value, expected object.");
		return false;
	}

	if(settingsDocument.HasMember(FILE_FORMAT_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & fileFormatVersionValue = settingsDocument[FILE_FORMAT_VERSION_PROPERTY_NAME];

		if(!fileFormatVersionValue.IsString()) {
			spdlog::error("Invalid settings file format version type: '{}', expected: 'string'.", Utilities::typeToString(fileFormatVersionValue.GetType()));
			return false;
		}

		std::optional<std::uint8_t> optionalVersionComparison(Utilities::compareVersions(fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION));

		if(!optionalVersionComparison.has_value()) {
			spdlog::error("Invalid settings file format version: '{}'.", fileFormatVersionValue.GetString());
			return false;
		}

		if(*optionalVersionComparison != 0) {
			spdlog::error("Unsupported settings file format version: '{}', only version '{}' is supported.", fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION);
			return false;
		}
	}
	else {
		spdlog::warn("Settings file is missing file format version, and may fail to load correctly!");
	}

	if(settingsDocument.HasMember(GAME_TYPE_PROPERTY_NAME) && settingsDocument[GAME_TYPE_PROPERTY_NAME].IsString()) {
		std::optional<GameType> gameTypeOptional = magic_enum::enum_cast<GameType>(Utilities::toPascalCase(settingsDocument[GAME_TYPE_PROPERTY_NAME].GetString()));

		if(gameTypeOptional.has_value()) {
			gameType = gameTypeOptional.value();
		}
	}

	assignStringSetting(dataDirectoryPath, settingsDocument, DATA_DIRECTORY_PATH_PROPERTY_NAME);
	assignStringSetting(appTempDirectoryPath, settingsDocument, APP_TEMP_DIRECTORY_PATH_PROPERTY_NAME);
	assignStringSetting(gameTempDirectoryName, settingsDocument, GAME_TEMP_DIRECTORY_NAME_PROPERTY_NAME);
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
		assignStringSetting(dosboxDownloadsDirectoryName, downloadsCategoryValue, DOSBOX_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME);
		assignStringSetting(gameDownloadsDirectoryName, downloadsCategoryValue, GAME_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME);
	}

	if(settingsDocument.HasMember(CACHE_CATEGORY_NAME) && settingsDocument[CACHE_CATEGORY_NAME].IsObject()) {
		const rapidjson::Value & cacheCategoryValue = settingsDocument[CACHE_CATEGORY_NAME];

		assignStringSetting(cacheDirectoryPath, cacheCategoryValue, CACHE_DIRECTORY_PATH_PROPERTY_NAME);
	}

	if(settingsDocument.HasMember(DOSBOX_CATEGORY_NAME) && settingsDocument[DOSBOX_CATEGORY_NAME].IsObject()) {
		const rapidjson::Value & dosboxCategoryValue = settingsDocument[DOSBOX_CATEGORY_NAME];

		assignStringSetting(dosboxVersionsListFilePath, dosboxCategoryValue, DOSBOX_VERSIONS_LIST_FILE_PATH_PROPERTY_NAME);
		assignStringSetting(preferredDOSBoxVersion, dosboxCategoryValue, PREFERRED_DOSBOX_VERSION_PROPERTY_NAME);
		assignStringSetting(dosboxArguments, dosboxCategoryValue, DOSBOX_ARGUMENTS_PROPERTY_NAME);
		assignStringSetting(dosboxDataDirectoryName, dosboxCategoryValue, DOSBOX_DATA_DIRECTORY_NAME_PROPERTY_NAME);

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

		if(curlCategoryValue.HasMember(CURL_CONNECTION_TIMEOUT_PROPERTY_NAME) && curlCategoryValue[CURL_CONNECTION_TIMEOUT_PROPERTY_NAME].IsUint64()) {
			connectionTimeout = std::chrono::seconds(curlCategoryValue[CURL_CONNECTION_TIMEOUT_PROPERTY_NAME].GetUint64());
		}

		if(curlCategoryValue.HasMember(CURL_NETWORK_TIMEOUT_PROPERTY_NAME) && curlCategoryValue[CURL_NETWORK_TIMEOUT_PROPERTY_NAME].IsUint64()) {
			networkTimeout = std::chrono::seconds(curlCategoryValue[CURL_NETWORK_TIMEOUT_PROPERTY_NAME].GetUint64());
		}

		if(curlCategoryValue.HasMember(CURL_TRANSFER_TIMEOUT_PROPERTY_NAME) && curlCategoryValue[CURL_TRANSFER_TIMEOUT_PROPERTY_NAME].IsUint64()) {
			transferTimeout = std::chrono::seconds(curlCategoryValue[CURL_TRANSFER_TIMEOUT_PROPERTY_NAME].GetUint64());
		}

		if(curlCategoryValue.HasMember(CURL_VERBOSE_REQUEST_LOGGING_PROPERTY_NAME) && curlCategoryValue[CURL_VERBOSE_REQUEST_LOGGING_PROPERTY_NAME].IsBool()) {
			verboseRequestLogging = curlCategoryValue[CURL_VERBOSE_REQUEST_LOGGING_PROPERTY_NAME].GetBool();
		}
	}

	if(settingsDocument.HasMember(API_CATEGORY_NAME) && settingsDocument[API_CATEGORY_NAME].IsObject()) {
		const rapidjson::Value & apiCategoryValue = settingsDocument[API_CATEGORY_NAME];

		assignStringSetting(apiBaseURL, apiCategoryValue, API_BASE_URL_PROPERTY_NAME);
		assignStringSetting(remoteModsListFileName, apiCategoryValue, REMOTE_MOD_LIST_FILE_NAME_PROPERTY_NAME);
		assignStringSetting(remoteDOSBoxVersionsListFileName, apiCategoryValue, REMOTE_DOSBOX_VERSIONS_LIST_FILE_NAME_PROPERTY_NAME);
		assignStringSetting(remoteGamesListFileName, apiCategoryValue, REMOTE_GAME_LIST_FILE_NAME_PROPERTY_NAME);
		assignStringSetting(remoteDownloadsDirectoryName, apiCategoryValue, REMOTE_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME);
		assignStringSetting(remoteModDownloadsDirectoryName, apiCategoryValue, REMOTE_MOD_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME);
		assignStringSetting(remoteMapDownloadsDirectoryName, apiCategoryValue, REMOTE_MAP_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME);
		assignStringSetting(remoteDOSBoxDownloadsDirectoryName, apiCategoryValue, REMOTE_DOSBOX_DOWNLOADS_DIRECTORY_NAME_PROPERTY_NAME);
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

	if(settingsDocument.HasMember(DOWNLOAD_THROTTLING_CATEGORY_NAME) && settingsDocument[DOWNLOAD_THROTTLING_CATEGORY_NAME].IsObject()) {
		const rapidjson::Value & downloadThrottlingValue = settingsDocument[DOWNLOAD_THROTTLING_CATEGORY_NAME];

		assignBooleanSetting(downloadThrottlingEnabled, downloadThrottlingValue, DOWNLOAD_THROTTLING_ENABLED_PROPERTY_NAME);
		assignOptionalTimePointSetting(modListLastDownloadedTimestamp, downloadThrottlingValue, MOD_LIST_LAST_DOWNLOADED_PROPERTY_NAME);
		assignChronoSetting(modListUpdateFrequency, downloadThrottlingValue, MOD_LIST_UPDATE_FREQUENCY_PROPERTY_NAME);
		assignOptionalTimePointSetting(dosboxDownloadListLastDownloadedTimestamp, downloadThrottlingValue, DOSBOX_DOWNLOAD_LIST_LAST_DOWNLOADED_PROPERTY_NAME);
		assignChronoSetting(dosboxDownloadListUpdateFrequency, downloadThrottlingValue, DOSBOX_DOWNLOAD_LIST_UPDATE_FREQUENCY_PROPERTY_NAME);
		assignOptionalTimePointSetting(gameDownloadListLastDownloadedTimestamp, downloadThrottlingValue, GAME_DOWNLOAD_LIST_LAST_DOWNLOADED_PROPERTY_NAME);
		assignChronoSetting(gameDownloadListUpdateFrequency, downloadThrottlingValue, GAME_DOWNLOAD_LIST_UPDATE_FREQUENCY_PROPERTY_NAME);
		assignOptionalTimePointSetting(cacertLastDownloadedTimestamp, downloadThrottlingValue, CACERT_LAST_DOWNLOADED_PROPERTY_NAME);
		assignChronoSetting(cacertUpdateFrequency, downloadThrottlingValue, CACERT_UPDATE_FREQUENCY_PROPERTY_NAME);
		assignOptionalTimePointSetting(timeZoneDataLastDownloadedTimestamp, downloadThrottlingValue, TIME_ZONE_DATA_LAST_DOWNLOADED_PROPERTY_NAME);
		assignChronoSetting(timeZoneDataUpdateFrequency, downloadThrottlingValue, TIME_ZONE_DATA_UPDATE_FREQUENCY_PROPERTY_NAME);
	}

	if(settingsDocument.HasMember(WINDOW_CATEGORY_NAME) && settingsDocument[WINDOW_CATEGORY_NAME].IsObject()) {
		const rapidjson::Value & windowCategoryValue = settingsDocument[WINDOW_CATEGORY_NAME];

		assignPointSetting(windowPosition, windowCategoryValue, WINDOW_POSITION_PROPERTY_NAME);
		assignDimensionSetting(windowSize, windowCategoryValue, WINDOW_SIZE_PROPERTY_NAME);
	}

	if(settingsDocument.HasMember(FILE_ETAGS_PROPERTY_NAME) && settingsDocument[FILE_ETAGS_PROPERTY_NAME].IsObject()) {
		const rapidjson::Value & fileETagsValue = settingsDocument[FILE_ETAGS_PROPERTY_NAME];

		for(rapidjson::Value::ConstMemberIterator i = fileETagsValue.MemberBegin(); i != fileETagsValue.MemberEnd(); ++i) {
			fileETags.emplace(i->name.GetString(), i->value.GetString());
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
