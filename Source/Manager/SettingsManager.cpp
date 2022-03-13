#include "SettingsManager.h"

#include "Arguments/ArgumentParser.h"
#include "Game/GameVersion.h"
#include "ModManager.h"
#include "Utilities/StringUtilities.h"

#include <fmt/core.h>
#include <magic_enum.hpp>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>

#include <filesystem>
#include <fstream>
#include <optional>
#include <string_view>

static constexpr const char * SYMLINK_NAME = "symlinkName";
static constexpr const char * LIST_FILE_PATH = "listFilePath";
static constexpr const char * DIRECTORY_PATH = "directoryPath";
static constexpr const char * EXECUTABLE_FILE_NAME = "executableFileName";

static constexpr const char * FILE_FORMAT_VERSION_PROPERTY_NAME = "version";
static constexpr const char * GAME_TYPE_PROPERTY_NAME = "gameType";
static constexpr const char * DATA_DIRECTORY_PATH_PROPERTY_NAME = "dataDirectoryPath";
static constexpr const char * VERBOSE_PROPERTY_NAME = "verbose";

static constexpr const char * GAME_VERSIONS_CATEGORY_NAME = "gameVersions";
static constexpr const char * GAME_VERSIONS_LIST_FILE_PATH_PROPERTY_NAME = LIST_FILE_PATH;
static constexpr const char * PREFERRED_GAME_VERSION_PROPERTY_NAME = "preferred";

static constexpr const char * MODS_CATEGORY_NAME = "mods";
static constexpr const char * MODS_LIST_FILE_PATH_PROPERTY_NAME = LIST_FILE_PATH;
static constexpr const char * FAVOURITE_MODS_LIST_FILE_PATH_PROPERTY_NAME = "favouritesListFilePath";
static constexpr const char * MODS_DIRECTORY_PATH_PROPERTY_NAME = DIRECTORY_PATH;
static constexpr const char * MODS_SYMLINK_NAME_PROPERTY_NAME = SYMLINK_NAME;
static constexpr const char * MOD_DOWNLOADS_DIRECTORY_PATH_PROPERTY_NAME = "downloadsDirectoryPath";
static constexpr const char * MOD_IMAGES_DIRECTORY_PATH_PROPERTY_NAME = "imagesDirectoryPath";
static constexpr const char * MOD_SOURCE_FILES_DIRECTORY_PATH_PROPERTY_NAME = "sourceFilesDirectoryPath";

static constexpr const char * MAPS_CATEGORY_NAME = "maps";
static constexpr const char * MAPS_DIRECTORY_PATH_PROPERTY_NAME = DIRECTORY_PATH;
static constexpr const char * MAPS_SYMLINK_NAME_PROPERTY_NAME = SYMLINK_NAME;

static constexpr const char * DOSBOX_CATEGORY_NAME = "dosbox";
static constexpr const char * DOSBOX_DIRECTORY_PATH_PROPERTY_NAME = DIRECTORY_PATH;
static constexpr const char * DOSBOX_EXECUTABLE_FILE_NAME_PROPERTY_NAME = EXECUTABLE_FILE_NAME;
static constexpr const char * DOSBOX_ARGUMENTS_PROPERTY_NAME = "arguments";
static constexpr const char * DOSBOX_DATA_DIRECTORY_NAME_PROPERTY_NAME = "dataDirectoryName";

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

const char * SettingsManager::FILE_FORMAT_VERSION = "1.0.0";
const char * SettingsManager::DEFAULT_SETTINGS_FILE_PATH = "Duke Nukem 3D Mod Manager.json";
const char * SettingsManager::DEFAULT_MODS_LIST_FILE_PATH = "Duke Nukem 3D Mod List.xml";
const char * SettingsManager::DEFAULT_FAVOURITE_MODS_LIST_FILE_PATH = "Duke Nukem 3D Favourite Mods.json";
const char * SettingsManager::DEFAULT_GAME_VERSIONS_LIST_FILE_PATH = "Duke Nukem 3D Game Versions.json";
const char * SettingsManager::DEFAULT_MODS_DIRECTORY_PATH = "Mods";
const char * SettingsManager::DEFAULT_MODS_SYMLINK_NAME = "Mods";
const char * SettingsManager::DEFAULT_MOD_DOWNLOADS_DIRECTORY_PATH = "";
const char * SettingsManager::DEFAULT_MOD_IMAGES_DIRECTORY_PATH = "";
const char * SettingsManager::DEFAULT_MOD_SOURCE_FILES_DIRECTORY_PATH = "";
const char * SettingsManager::DEFAULT_MAPS_DIRECTORY_PATH = "";
const char * SettingsManager::DEFAULT_MAPS_SYMLINK_NAME = "Maps";
const char * SettingsManager::DEFAULT_DATA_DIRECTORY_PATH = "Data";
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
const bool SettingsManager::DEFAULT_VERBOSE = false;

SettingsManager::SettingsManager()
	: modsListFilePath(DEFAULT_MODS_LIST_FILE_PATH)
	, favouriteModsListFilePath(DEFAULT_FAVOURITE_MODS_LIST_FILE_PATH)
	, gameVersionsListFilePath(DEFAULT_GAME_VERSIONS_LIST_FILE_PATH)
	, modsDirectoryPath(DEFAULT_MODS_DIRECTORY_PATH)
	, modsSymlinkName(DEFAULT_MODS_SYMLINK_NAME)
	, modDownloadsDirectoryPath(DEFAULT_MOD_DOWNLOADS_DIRECTORY_PATH)
	, modImagesDirectoryPath(DEFAULT_MOD_IMAGES_DIRECTORY_PATH)
	, modSourceFilesDirectoryPath(DEFAULT_MOD_SOURCE_FILES_DIRECTORY_PATH)
	, mapsDirectoryPath(DEFAULT_MAPS_DIRECTORY_PATH)
	, mapsSymlinkName(DEFAULT_MAPS_SYMLINK_NAME)
	, dataDirectoryPath(DEFAULT_DATA_DIRECTORY_PATH)
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
	, verbose(DEFAULT_VERBOSE) { }

SettingsManager::SettingsManager(SettingsManager && s) noexcept
	: modsListFilePath(std::move(s.modsListFilePath))
	, favouriteModsListFilePath(std::move(s.favouriteModsListFilePath))
	, gameVersionsListFilePath(std::move(s.gameVersionsListFilePath))
	, modsDirectoryPath(std::move(s.modsDirectoryPath))
	, modsSymlinkName(std::move(s.modsSymlinkName))
	, modDownloadsDirectoryPath(std::move(s.modDownloadsDirectoryPath))
	, modSourceFilesDirectoryPath(std::move(s.modSourceFilesDirectoryPath))
	, mapsDirectoryPath(std::move(s.mapsDirectoryPath))
	, mapsSymlinkName(std::move(s.mapsSymlinkName))
	, dataDirectoryPath(std::move(s.dataDirectoryPath))
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
	, verbose(s.verbose) { }

SettingsManager::SettingsManager(const SettingsManager & s)
	: modsListFilePath(s.modsListFilePath)
	, favouriteModsListFilePath(s.favouriteModsListFilePath)
	, gameVersionsListFilePath(s.gameVersionsListFilePath)
	, modsDirectoryPath(s.modsDirectoryPath)
	, modsSymlinkName(s.modsSymlinkName)
	, modDownloadsDirectoryPath(s.modDownloadsDirectoryPath)
	, modSourceFilesDirectoryPath(s.modSourceFilesDirectoryPath)
	, mapsDirectoryPath(s.mapsDirectoryPath)
	, mapsSymlinkName(s.mapsSymlinkName)
	, dataDirectoryPath(s.dataDirectoryPath)
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
	, verbose(s.verbose) { }

SettingsManager & SettingsManager::operator = (SettingsManager && s) noexcept {
	if(this != &s) {
		modsListFilePath = std::move(s.modsListFilePath);
		favouriteModsListFilePath = std::move(s.favouriteModsListFilePath);
		gameVersionsListFilePath = std::move(s.gameVersionsListFilePath);
		modsDirectoryPath = std::move(s.modsDirectoryPath);
		modsSymlinkName = std::move(s.modsSymlinkName);
		modDownloadsDirectoryPath = std::move(s.modDownloadsDirectoryPath);
		modImagesDirectoryPath = std::move(s.modImagesDirectoryPath);
		modSourceFilesDirectoryPath = std::move(s.modSourceFilesDirectoryPath);
		mapsDirectoryPath = std::move(s.mapsDirectoryPath);
		mapsSymlinkName = std::move(s.mapsSymlinkName);
		dataDirectoryPath = std::move(s.dataDirectoryPath);
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
		verbose = s.verbose;
	}

	return *this;
}

SettingsManager & SettingsManager::operator = (const SettingsManager & s) {
	modsListFilePath = s.modsListFilePath;
	favouriteModsListFilePath = s.favouriteModsListFilePath;
	gameVersionsListFilePath = s.gameVersionsListFilePath;
	modsDirectoryPath = s.modsDirectoryPath;
	modsSymlinkName = s.modsSymlinkName;
	modDownloadsDirectoryPath = s.modDownloadsDirectoryPath;
	modImagesDirectoryPath = s.modImagesDirectoryPath;
	modSourceFilesDirectoryPath = s.modSourceFilesDirectoryPath;
	mapsDirectoryPath = s.mapsDirectoryPath;
	mapsSymlinkName = s.mapsSymlinkName;
	dataDirectoryPath = s.dataDirectoryPath;
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
	verbose = s.verbose;

	return *this;
}

SettingsManager::~SettingsManager() = default;

void SettingsManager::reset() {
	modsListFilePath = DEFAULT_MODS_LIST_FILE_PATH;
	favouriteModsListFilePath = DEFAULT_FAVOURITE_MODS_LIST_FILE_PATH;
	gameVersionsListFilePath = DEFAULT_GAME_VERSIONS_LIST_FILE_PATH;
	modsDirectoryPath = DEFAULT_MODS_DIRECTORY_PATH;
	modsSymlinkName = DEFAULT_MODS_SYMLINK_NAME;
	modDownloadsDirectoryPath = DEFAULT_MOD_DOWNLOADS_DIRECTORY_PATH;
	modImagesDirectoryPath = DEFAULT_MOD_IMAGES_DIRECTORY_PATH;
	modSourceFilesDirectoryPath = DEFAULT_MOD_SOURCE_FILES_DIRECTORY_PATH;
	mapsDirectoryPath = DEFAULT_MAPS_DIRECTORY_PATH;
	mapsSymlinkName = DEFAULT_MAPS_SYMLINK_NAME;
	dataDirectoryPath = DEFAULT_DATA_DIRECTORY_PATH;
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
	verbose = DEFAULT_VERBOSE;
}

rapidjson::Document SettingsManager::toJSON() const {
	rapidjson::Document settingsDocument(rapidjson::kObjectType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = settingsDocument.GetAllocator();

	rapidjson::Value fileFormatVersionValue(FILE_FORMAT_VERSION, allocator);
	settingsDocument.AddMember(rapidjson::StringRef(FILE_FORMAT_VERSION_PROPERTY_NAME), fileFormatVersionValue, allocator);
	rapidjson::Value gameTypeValue(Utilities::toCapitalCase(std::string(magic_enum::enum_name(gameType))).c_str(), allocator);
	settingsDocument.AddMember(rapidjson::StringRef(GAME_TYPE_PROPERTY_NAME), gameTypeValue, allocator);
	rapidjson::Value dataDirectoryPathValue(dataDirectoryPath.c_str(), allocator);
	settingsDocument.AddMember(rapidjson::StringRef(DATA_DIRECTORY_PATH_PROPERTY_NAME), dataDirectoryPathValue, allocator);
	settingsDocument.AddMember(rapidjson::StringRef(VERBOSE_PROPERTY_NAME), rapidjson::Value(verbose), allocator);

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

	if(!modDownloadsDirectoryPath.empty()) {
		rapidjson::Value modDownloadsDirectoryPathValue(modDownloadsDirectoryPath.c_str(), allocator);
		modsCategoryValue.AddMember(rapidjson::StringRef(MOD_DOWNLOADS_DIRECTORY_PATH_PROPERTY_NAME), modDownloadsDirectoryPathValue, allocator);
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

	return settingsDocument;
}

bool SettingsManager::parseFrom(const rapidjson::Value & settingsDocument) {
	if(!settingsDocument.IsObject()) {
		fmt::print("Invalid settings value, expected object.\n");
		return false;
	}

	if(settingsDocument.HasMember(FILE_FORMAT_VERSION_PROPERTY_NAME)) {
		if(!settingsDocument[FILE_FORMAT_VERSION_PROPERTY_NAME].IsString()) {
			fmt::print("Invalid settings file version: '{}', expected: '{}'.\n", settingsDocument[FILE_FORMAT_VERSION_PROPERTY_NAME].GetString(), FILE_FORMAT_VERSION);
			return false;
		}

		std::optional<std::uint8_t> optionalVersionComparison(Utilities::compareVersions(settingsDocument[FILE_FORMAT_VERSION_PROPERTY_NAME].GetString(), FILE_FORMAT_VERSION));

		if(!optionalVersionComparison.has_value()) {
			fmt::print("Invalid settings file version: '{}'.\n", settingsDocument[FILE_FORMAT_VERSION_PROPERTY_NAME].GetString());
			return false;
		}

		if(*optionalVersionComparison != 0) {
			fmt::print("Unsupported settings file version: '{}', only version '{}' is supported.\n", settingsDocument[FILE_FORMAT_VERSION_PROPERTY_NAME].GetString(), FILE_FORMAT_VERSION);
			return false;
		}
	}
	else {
		fmt::print("Settings file is missing version, and may fail to load correctly!\n");
	}

	if(settingsDocument.HasMember(GAME_TYPE_PROPERTY_NAME) && settingsDocument[GAME_TYPE_PROPERTY_NAME].IsString()) {
		std::optional<GameType> gameTypeOptional = magic_enum::enum_cast<GameType>(Utilities::toPascalCase(settingsDocument[GAME_TYPE_PROPERTY_NAME].GetString()));

		if(gameTypeOptional.has_value()) {
			gameType = gameTypeOptional.value();
		}
	}

	if(settingsDocument.HasMember(DATA_DIRECTORY_PATH_PROPERTY_NAME) && settingsDocument[DATA_DIRECTORY_PATH_PROPERTY_NAME].IsString()) {
		dataDirectoryPath = settingsDocument[DATA_DIRECTORY_PATH_PROPERTY_NAME].GetString();
	}

	if(settingsDocument.HasMember(VERBOSE_PROPERTY_NAME) && settingsDocument[VERBOSE_PROPERTY_NAME].IsBool()) {
		verbose = settingsDocument[VERBOSE_PROPERTY_NAME].GetBool();
	}

	if(settingsDocument.HasMember(GAME_VERSIONS_CATEGORY_NAME) && settingsDocument[GAME_VERSIONS_CATEGORY_NAME].IsObject()) {
		const rapidjson::Value & gameVersionsCategoryValue = settingsDocument[GAME_VERSIONS_CATEGORY_NAME];

		if(gameVersionsCategoryValue.HasMember(GAME_VERSIONS_LIST_FILE_PATH_PROPERTY_NAME) && gameVersionsCategoryValue[GAME_VERSIONS_LIST_FILE_PATH_PROPERTY_NAME].IsString()) {
			gameVersionsListFilePath = gameVersionsCategoryValue[GAME_VERSIONS_LIST_FILE_PATH_PROPERTY_NAME].GetString();
		}

		if(gameVersionsCategoryValue.HasMember(PREFERRED_GAME_VERSION_PROPERTY_NAME) && gameVersionsCategoryValue[PREFERRED_GAME_VERSION_PROPERTY_NAME].IsString()) {
			preferredGameVersion = gameVersionsCategoryValue[PREFERRED_GAME_VERSION_PROPERTY_NAME].GetString();
		}
	}

	if(settingsDocument.HasMember(MODS_CATEGORY_NAME) && settingsDocument[MODS_CATEGORY_NAME].IsObject()) {
		const rapidjson::Value & modsCategoryValue = settingsDocument[MODS_CATEGORY_NAME];

		if(modsCategoryValue.HasMember(MODS_LIST_FILE_PATH_PROPERTY_NAME) && modsCategoryValue[MODS_LIST_FILE_PATH_PROPERTY_NAME].IsString()) {
			modsListFilePath = modsCategoryValue[MODS_LIST_FILE_PATH_PROPERTY_NAME].GetString();
		}

		if(modsCategoryValue.HasMember(FAVOURITE_MODS_LIST_FILE_PATH_PROPERTY_NAME) && modsCategoryValue[FAVOURITE_MODS_LIST_FILE_PATH_PROPERTY_NAME].IsString()) {
			favouriteModsListFilePath = modsCategoryValue[FAVOURITE_MODS_LIST_FILE_PATH_PROPERTY_NAME].GetString();
		}

		if(modsCategoryValue.HasMember(MODS_DIRECTORY_PATH_PROPERTY_NAME) && modsCategoryValue[MODS_DIRECTORY_PATH_PROPERTY_NAME].IsString()) {
			modsDirectoryPath = modsCategoryValue[MODS_DIRECTORY_PATH_PROPERTY_NAME].GetString();
		}

		if(modsCategoryValue.HasMember(MODS_SYMLINK_NAME_PROPERTY_NAME) && modsCategoryValue[MODS_SYMLINK_NAME_PROPERTY_NAME].IsString()) {
			modsSymlinkName = modsCategoryValue[MODS_SYMLINK_NAME_PROPERTY_NAME].GetString();
		}

		if(modsCategoryValue.HasMember(MOD_DOWNLOADS_DIRECTORY_PATH_PROPERTY_NAME) && modsCategoryValue[MOD_DOWNLOADS_DIRECTORY_PATH_PROPERTY_NAME].IsString()) {
			modDownloadsDirectoryPath = modsCategoryValue[MOD_DOWNLOADS_DIRECTORY_PATH_PROPERTY_NAME].GetString();
		}

		if(modsCategoryValue.HasMember(MOD_IMAGES_DIRECTORY_PATH_PROPERTY_NAME) && modsCategoryValue[MOD_IMAGES_DIRECTORY_PATH_PROPERTY_NAME].IsString()) {
			modImagesDirectoryPath = modsCategoryValue[MOD_IMAGES_DIRECTORY_PATH_PROPERTY_NAME].GetString();
		}

		if(modsCategoryValue.HasMember(MOD_SOURCE_FILES_DIRECTORY_PATH_PROPERTY_NAME) && modsCategoryValue[MOD_SOURCE_FILES_DIRECTORY_PATH_PROPERTY_NAME].IsString()) {
			modSourceFilesDirectoryPath = modsCategoryValue[MOD_SOURCE_FILES_DIRECTORY_PATH_PROPERTY_NAME].GetString();
		}
	}

	if(settingsDocument.HasMember(MAPS_CATEGORY_NAME) && settingsDocument[MAPS_CATEGORY_NAME].IsObject()) {
		const rapidjson::Value & mapsCategoryValue = settingsDocument[MAPS_CATEGORY_NAME];

		if(mapsCategoryValue.HasMember(MAPS_DIRECTORY_PATH_PROPERTY_NAME) && mapsCategoryValue[MAPS_DIRECTORY_PATH_PROPERTY_NAME].IsString()) {
			mapsDirectoryPath = mapsCategoryValue[MAPS_DIRECTORY_PATH_PROPERTY_NAME].GetString();
		}

		if(mapsCategoryValue.HasMember(MAPS_SYMLINK_NAME_PROPERTY_NAME) && mapsCategoryValue[MAPS_SYMLINK_NAME_PROPERTY_NAME].IsString()) {
			mapsSymlinkName = mapsCategoryValue[MAPS_SYMLINK_NAME_PROPERTY_NAME].GetString();
		}
	}

	if(settingsDocument.HasMember(DOSBOX_CATEGORY_NAME) && settingsDocument[DOSBOX_CATEGORY_NAME].IsObject()) {
		const rapidjson::Value & dosboxCategoryValue = settingsDocument[DOSBOX_CATEGORY_NAME];

		if(dosboxCategoryValue.HasMember(DOSBOX_DIRECTORY_PATH_PROPERTY_NAME) && dosboxCategoryValue[DOSBOX_DIRECTORY_PATH_PROPERTY_NAME].IsString()) {
			dosboxDirectoryPath = dosboxCategoryValue[DOSBOX_DIRECTORY_PATH_PROPERTY_NAME].GetString();
		}

		if(dosboxCategoryValue.HasMember(DOSBOX_EXECUTABLE_FILE_NAME_PROPERTY_NAME) && dosboxCategoryValue[DOSBOX_EXECUTABLE_FILE_NAME_PROPERTY_NAME].IsString()) {
			dosboxExecutableFileName = dosboxCategoryValue[DOSBOX_EXECUTABLE_FILE_NAME_PROPERTY_NAME].GetString();
		}

		if(dosboxCategoryValue.HasMember(DOSBOX_ARGUMENTS_PROPERTY_NAME) && dosboxCategoryValue[DOSBOX_ARGUMENTS_PROPERTY_NAME].IsString()) {
			dosboxArguments = dosboxCategoryValue[DOSBOX_ARGUMENTS_PROPERTY_NAME].GetString();
		}

		if(dosboxCategoryValue.HasMember(DOSBOX_DATA_DIRECTORY_NAME_PROPERTY_NAME) && dosboxCategoryValue[DOSBOX_DATA_DIRECTORY_NAME_PROPERTY_NAME].IsString()) {
			dosboxDataDirectoryName = dosboxCategoryValue[DOSBOX_DATA_DIRECTORY_NAME_PROPERTY_NAME].GetString();
		}

		if(dosboxCategoryValue.HasMember(DOSBOX_SCRIPTS_CATEGORY_NAME) && dosboxCategoryValue[DOSBOX_SCRIPTS_CATEGORY_NAME].IsObject()) {
			const rapidjson::Value & dosboxScriptsCategoryValue = dosboxCategoryValue[DOSBOX_SCRIPTS_CATEGORY_NAME];

			if(dosboxScriptsCategoryValue.HasMember(DOSBOX_GAME_SCRIPT_FILE_NAME_PROPERTY_NAME) && dosboxScriptsCategoryValue[DOSBOX_GAME_SCRIPT_FILE_NAME_PROPERTY_NAME].IsString()) {
				dosboxGameScriptFileName = dosboxScriptsCategoryValue[DOSBOX_GAME_SCRIPT_FILE_NAME_PROPERTY_NAME].GetString();
			}

			if(dosboxScriptsCategoryValue.HasMember(DOSBOX_SETUP_SCRIPT_FILE_NAME_PROPERTY_NAME) && dosboxScriptsCategoryValue[DOSBOX_SETUP_SCRIPT_FILE_NAME_PROPERTY_NAME].IsString()) {
				dosboxSetupScriptFileName = dosboxScriptsCategoryValue[DOSBOX_SETUP_SCRIPT_FILE_NAME_PROPERTY_NAME].GetString();
			}

			if(dosboxScriptsCategoryValue.HasMember(DOSBOX_CLIENT_SCRIPT_FILE_NAME_PROPERTY_NAME) && dosboxScriptsCategoryValue[DOSBOX_CLIENT_SCRIPT_FILE_NAME_PROPERTY_NAME].IsString()) {
				dosboxClientScriptFileName = dosboxScriptsCategoryValue[DOSBOX_CLIENT_SCRIPT_FILE_NAME_PROPERTY_NAME].GetString();
			}

			if(dosboxScriptsCategoryValue.HasMember(DOSBOX_SERVER_SCRIPT_FILE_NAME_PROPERTY_NAME) && dosboxScriptsCategoryValue[DOSBOX_SERVER_SCRIPT_FILE_NAME_PROPERTY_NAME].IsString()) {
				dosboxServerScriptFileName = dosboxScriptsCategoryValue[DOSBOX_SERVER_SCRIPT_FILE_NAME_PROPERTY_NAME].GetString();
			}
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

	return true;
}

bool SettingsManager::load(const ArgumentParser * arguments) {
	if(arguments != nullptr) {
		std::string alternateSettingsFileName(arguments->getFirstValue("f"));

		if(!alternateSettingsFileName.empty()) {
			fmt::print("Loading settings from alternate file: '{}'...\n", alternateSettingsFileName);

			bool loadedSettings = loadFrom(alternateSettingsFileName);

			if(!loadedSettings) {
				fmt::print("Failed to load settings from alt settings file: '{}'.\n", alternateSettingsFileName);
			}

			return loadedSettings;
		}
	}

	return loadFrom(DEFAULT_SETTINGS_FILE_PATH);
}

bool SettingsManager::save(const ArgumentParser * arguments, bool overwrite) const {
	if(arguments != nullptr) {
		std::string alternateSettingsFileName(arguments->getFirstValue("f"));

		if(!alternateSettingsFileName.empty()) {
			fmt::print("Saving settings to alternate file: '{}'...\n", alternateSettingsFileName);

			bool savedSettings = saveTo(alternateSettingsFileName, overwrite);

			if(!savedSettings) {
				fmt::print("Failed to save settings to alternate file: '{}'.\n", alternateSettingsFileName);
			}

			return savedSettings;
		}
	}

	return saveTo(DEFAULT_SETTINGS_FILE_PATH, overwrite);
}

bool SettingsManager::loadFrom(const std::string & filePath) {
	if(filePath.empty() || !std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		return false;
	}

	std::ifstream fileStream(filePath);

	if(!fileStream.is_open()) {
		return false;
	}

	rapidjson::Document settings;
	rapidjson::IStreamWrapper fileStreamWrapper(fileStream);
	if(settings.ParseStream(fileStreamWrapper).HasParseError()) {
		return false;
	}

	fileStream.close();

	if(settings.IsNull()) {
		fmt::print("Failed to parse settings file JSON data!\n");
		return false;
	}

	if(!parseFrom(settings)) {
		fmt::print("Failed to parse settings from file '{}!\n", filePath);
		return false;
	}

	fmt::print("Settings successfully loaded from file '{}'.\n", filePath);

	return true;
}

bool SettingsManager::saveTo(const std::string & filePath, bool overwrite) const {
	if (!overwrite && std::filesystem::exists(std::filesystem::path(filePath))) {
		fmt::print("File '{}' already exists, use overwrite to force write.\n", filePath);
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

	fmt::print("Settings successfully saved to file '{}'.\n", filePath);

	return true;
}
