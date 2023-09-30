#include "ModManager.h"

#include "DOSBox/DOSBoxManager.h"
#include "DOSBox/DOSBoxVersion.h"
#include "DOSBox/Configuration/DOSBoxConfiguration.h"
#include "Download/CachedPackageFile.h"
#include "Download/DownloadCache.h"
#include "Download/DownloadManager.h"
#include "Environment.h"
#include "Game/GameLocator.h"
#include "Game/GameManager.h"
#include "Game/GameVersion.h"
#include "Game/GameVersionCollection.h"
#include "Game/File/GameFileFactoryRegistry.h"
#include "Game/File/Art/Art.h"
#include "Game/File/Group/GroupUtilities.h"
#include "Game/File/Group/GRP/GroupGRP.h"
#include "Game/File/Map/Map.h"
#include "InstalledModInfo.h"
#include "Manager/ModMatch.h"
#include "Mod/Mod.h"
#include "Mod/ModAuthorInformation.h"
#include "Mod/ModCollection.h"
#include "Mod/ModDownload.h"
#include "Mod/ModFile.h"
#include "Mod/ModGameVersion.h"
#include "Mod/ModIdentifier.h"
#include "Mod/ModImage.h"
#include "Mod/ModScreenshot.h"
#include "Mod/ModVersion.h"
#include "Mod/ModVersionType.h"
#include "Mod/FavouriteModCollection.h"
#include "Mod/OrganizedModCollection.h"
#include "Mod/StandAloneMod.h"
#include "Mod/StandAloneModCollection.h"
#include "SettingsManager.h"
#include "Project.h"

#include <Analytics/Segment/SegmentAnalytics.h>
#include <Archive/ArchiveFactoryRegistry.h>
#include <Archive/Zip/ZipArchive.h>
#include <Arguments/ArgumentParser.h>
#include <LibraryInformation.h>
#include <Location/GeoLocationService.h>
#include <Network/HTTPRequest.h>
#include <Network/HTTPResponse.h>
#include <Network/HTTPService.h>
#include <Platform/ProcessCreator.h>
#include <Platform/TimeZoneDataManager.h>
#include <Script/Script.h>
#include <Script/ScriptArguments.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/NumberUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/TimeUtilities.h>
#include <Utilities/TinyXML2Utilities.h>
#include <Utilities/Utilities.h>

#include <fmt/core.h>
#include <jdksmidi/version.h>
#include <magic_enum.hpp>
#include <sndfile.h>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <array>
#include <conio.h>
#include <errno.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <regex>
#include <sstream>

using namespace std::chrono_literals;

static constexpr uint8_t NUMBER_OF_INITIALIZATION_STEPS = 16;

const GameType ModManager::DEFAULT_GAME_TYPE = GameType::Game;
const std::string ModManager::DEFAULT_PREFERRED_DOSBOX_VERSION_ID(DOSBoxVersion::DOSBOX.getID());
const std::string ModManager::DEFAULT_PREFERRED_GAME_VERSION_ID(GameVersion::ORIGINAL_ATOMIC_EDITION.getID());
const std::string ModManager::HTTP_USER_AGENT("DukeNukem3DModManager/" + APPLICATION_VERSION);
const std::string ModManager::DEFAULT_BACKUP_FILE_RENAME_SUFFIX("_");
const std::string ModManager::GENERAL_DOSBOX_CONFIGURATION_FILE_NAME("general." + DOSBoxConfiguration::FILE_EXTENSION);

const DOSBoxConfiguration ModManager::DEFAULT_GENERAL_DOSBOX_CONFIGURATION({
	DOSBoxConfiguration::Section("dosbox", {
		DOSBoxConfiguration::Section::Entry("machine", "vesa_nolfb"),
		DOSBoxConfiguration::Section::Entry("memsize", "63"),
	})
});

static std::string modFilesToFileNameList(const std::vector<std::shared_ptr<ModFile>> & modFiles) {
	std::stringstream modFileNames;

	for(const std::shared_ptr<ModFile> & modFile : modFiles) {
		if(modFileNames.tellp() != 0) {
			modFileNames << ", ";
		}

		modFileNames << modFile->getFileName();
	}

	return modFileNames.str();
}

static std::string filePathsToFileNameList(const std::vector<std::string> & filePaths) {
	std::stringstream fileNames;

	for(const std::string & filePath : filePaths) {
		if(fileNames.tellp() != 0) {
			fileNames << ", ";
		}

		fileNames << Utilities::getFileName(filePath);
	}

	return fileNames.str();
}

ModManager::ModManager()
	: Application()
	, m_initialized(false)
	, m_initializing(false)
	, m_localMode(SettingsManager::DEFAULT_LOCAL_MODE)
	, m_shouldRunSelectedMod(false)
	, m_demoRecordingEnabled(false)
	, m_gameType(ModManager::DEFAULT_GAME_TYPE)
	, m_selectedModVersionIndex(std::numeric_limits<size_t>::max())
	, m_selectedModVersionTypeIndex(std::numeric_limits<size_t>::max())
	, m_selectedModGameVersionIndex(std::numeric_limits<size_t>::max())
	, m_dosboxManager(std::make_shared<DOSBoxManager>())
	, m_gameManager(std::make_shared<GameManager>())
	, m_generalDOSBoxConfiguration(std::make_shared<DOSBoxConfiguration>(DEFAULT_GENERAL_DOSBOX_CONFIGURATION))
	, m_mods(std::make_shared<ModCollection>())
	, m_standAloneMods(std::make_shared<StandAloneModCollection>())
	, m_favouriteMods(std::make_shared<FavouriteModCollection>())
	, m_organizedMods(std::make_shared<OrganizedModCollection>(m_mods, m_favouriteMods, m_gameManager->getGameVersions()))
	, m_initializationStep(0) {
	assignPlatformFactories();

	FactoryRegistry & factoryRegistry = FactoryRegistry::getInstance();

	factoryRegistry.setFactory<SettingsManager>([]() {
		return std::make_unique<SettingsManager>();
	});

	factoryRegistry.setFactory<GameFileFactoryRegistry>([]() {
		return std::make_unique<GameFileFactoryRegistry>();
	});

	m_selectedModChangedConnection = m_organizedMods->selectedModChanged.connect(std::bind(&ModManager::onSelectedModChanged, this, std::placeholders::_1));
	m_selectedFavouriteModChangedConnection = m_organizedMods->selectedFavouriteModChanged.connect(std::bind(&ModManager::onSelectedFavouriteModChanged, this, std::placeholders::_1));

	LibraryInformation * libraryInformation = LibraryInformation::getInstance();
	libraryInformation->addLibrary("JDKSMIDI", jdksmidi::LibraryVersion);

	std::string_view libSndFileVersion(sf_version_string());
	size_t versionStartIndex = libSndFileVersion.find_first_of("0123456789");
	if(versionStartIndex == std::string::npos) {
		versionStartIndex = 0;
	}
	libraryInformation->addLibrary("libsndfile", std::string(libSndFileVersion.substr(versionStartIndex, libSndFileVersion.length() - versionStartIndex)));
}

ModManager::~ModManager() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_selectedModChangedConnection.disconnect();
	m_selectedFavouriteModChangedConnection.disconnect();
	m_dosboxVersionCollectionSizeChangedConnection.disconnect();
	m_dosboxVersionCollectionItemModifiedConnection.disconnect();
	m_gameVersionCollectionSizeChangedConnection.disconnect();
	m_gameVersionCollectionItemModifiedConnection.disconnect();

	SegmentAnalytics::destroyInstance();
}

bool ModManager::isInitialized() const {
	return m_initialized;
}

bool ModManager::isInitializing() const {
	return m_initializing;
}

uint8_t ModManager::numberOfInitializationSteps() const {
	return NUMBER_OF_INITIALIZATION_STEPS;
}

bool ModManager::notifyInitializationProgress(const std::string & description) {
	if(!initializationProgress(m_initializationStep++, NUMBER_OF_INITIALIZATION_STEPS, description)) {
		initializationCancelled();

		return false;
	}

	return true;
}

void ModManager::notifyModSelectionChanged() {
	modSelectionChanged(m_selectedMod, m_selectedModVersionIndex, m_selectedModVersionTypeIndex, m_selectedModGameVersionIndex);

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		if(m_selectedMod != nullptr) {
			std::map<std::string, std::any> properties;
			properties["fullModName"] = m_selectedMod->getFullName(m_selectedModVersionIndex, m_selectedModVersionTypeIndex);
			properties["modID"] = m_selectedMod->getID();
			properties["modName"] = m_selectedMod->getName();

			if(m_selectedModVersionIndex != std::numeric_limits<size_t>::max()) {
				properties["modVersion"] = getSelectedModVersion()->getVersion();

				if(m_selectedModVersionTypeIndex != std::numeric_limits<size_t>::max()) {
					properties["modVersionType"] = getSelectedModVersionType()->getType();
				}
			}

			SegmentAnalytics::getInstance()->track("Mod Selection Changed", properties);
		}
	}
}

std::future<bool> ModManager::initializeAsync(int argc, char * argv[]) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);

	if(m_initialized || m_initializing) {
		return {};
	}

	std::shared_ptr<ArgumentParser> arguments;

	if(argc != 0) {
		arguments = std::make_shared<ArgumentParser>(argc, argv);
	}

	lock.unlock();

	return initializeAsync(arguments);
}

std::future<bool> ModManager::initializeAsync(std::shared_ptr<ArgumentParser> arguments) {
	return std::async(std::launch::async, &ModManager::initialize, std::ref(*this), arguments);
}

bool ModManager::initialize(std::shared_ptr<ArgumentParser> arguments) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_initialized || m_initializing) {
		return false;
	}

	std::chrono::time_point<std::chrono::steady_clock> initializeSteadyStartTimePoint(std::chrono::steady_clock::now());
	m_initializing = true;
	m_initializationStep = 0;

	if(!notifyInitializationProgress("Parsing Arguments")) {
		return false;
	}

	bool localModeSet = false;

	if(arguments != nullptr) {
		m_arguments = arguments;

		if(m_arguments->hasArgument("local")) {
			m_localMode = true;
			localModeSet = true;
		}

		if(m_arguments->hasArgument("r") || m_arguments->hasArgument("record")) {
			m_demoRecordingEnabled = true;
		}
	}

	if(!notifyInitializationProgress("Loading Settings")) {
		return false;
	}

	SettingsManager * settings = SettingsManager::getInstance();

	if(!settings->isLoaded()) {
		settings->load(m_arguments.get());
	}

	if(settings->localMode && !localModeSet) {
		m_localMode = true;
	}

	m_organizedMods->setLocalMode(m_localMode);
	m_dosboxManager->setLocalMode(m_localMode);
	m_gameManager->setLocalMode(m_localMode);

	bool skipFileInfoValidation = m_localMode && m_arguments != nullptr && (m_arguments->hasArgument("skip-file-info-validation") || m_arguments->hasArgument("update-new") || m_arguments->hasArgument("update-all"));

	if(!notifyInitializationProgress("Initializing HTTP Service")) {
		return false;
	}

	HTTPConfiguration configuration = {
		Utilities::joinPaths(settings->dataDirectoryPath, settings->curlDataDirectoryName),
		settings->apiBaseURL,
		settings->connectionTimeout,
		settings->networkTimeout,
		settings->transferTimeout
	};

	HTTPService * httpService = HTTPService::getInstance();

	if(!httpService->initialize(configuration)) {
		spdlog::error("Failed to initialize HTTP service!");
		initializationFailed();
		return false;
	}

	httpService->setUserAgent(HTTP_USER_AGENT);
	httpService->setVerboseLoggingEnabled(settings->verboseRequestLogging);

	if(!settings->downloadThrottlingEnabled || !settings->cacertLastDownloadedTimestamp.has_value() || std::chrono::system_clock::now() - settings->cacertLastDownloadedTimestamp.value() > settings->cacertUpdateFrequency) {
		if(httpService->updateCertificateAuthorityCertificateAndWait()) {
			settings->cacertLastDownloadedTimestamp = std::chrono::system_clock::now();
			settings->save();
		}
	}

	if(!notifyInitializationProgress("Initializing Time Zone Data Manager")) {
		return false;
	}

	bool timeZoneDataUpdated = false;
	bool shouldUpdateTimeZoneData = !settings->downloadThrottlingEnabled || !settings->timeZoneDataLastDownloadedTimestamp.has_value() || std::chrono::system_clock::now() - settings->timeZoneDataLastDownloadedTimestamp.value() > settings->timeZoneDataUpdateFrequency;

	if(!TimeZoneDataManager::getInstance()->initialize(Utilities::joinPaths(settings->dataDirectoryPath, settings->timeZoneDataDirectoryName), settings->fileETags, shouldUpdateTimeZoneData, false, &timeZoneDataUpdated)) {
		spdlog::error("Failed to initialize time zone data manager!");
		initializationFailed();
		return false;
	}

	if(timeZoneDataUpdated) {
		settings->timeZoneDataLastDownloadedTimestamp = std::chrono::system_clock::now();
		settings->save();
	}

	if(!notifyInitializationProgress("Initializing Geo Location Service")) {
		return false;
	}

	GeoLocationService * geoLocationService = GeoLocationService::getInstance();

	if(!geoLocationService->initialize(FREE_GEO_IP_API_KEY)) {
		spdlog::error("Failed to initialize geo location service!");
		initializationFailed();
		return false;
	}

	if(!notifyInitializationProgress("Initializing Segment Analytics")) {
		return false;
	}

	SegmentAnalytics * segmentAnalytics = SegmentAnalytics::getInstance();

	if(settings->segmentAnalyticsEnabled) {
		SegmentAnalytics::Configuration configuration;
		configuration.writeKey = SEGMENT_ANALYTICS_WRITE_KEY;
		configuration.includeIPAddress = false;
		configuration.includeGeoLocation = true;
		configuration.dataStorageFilePath = Utilities::joinPaths(settings->cacheDirectoryPath, settings->segmentAnalyticsDataFileName);
		configuration.applicationName = "Duke Nukem 3D Mod Manager";
		configuration.applicationVersion = APPLICATION_VERSION;
		configuration.applicationBuild = APPLICATION_COMMIT_HASH;
		configuration.applicationPackageName = Utilities::emptyString;
		configuration.userAgent = HTTP_USER_AGENT;

		if(segmentAnalytics->initialize(configuration)) {
			if(segmentAnalytics->start()) {
				spdlog::debug("Segment analytics initialized and started successfully!");
			}
			else {
				spdlog::error("Failed to start Segment analytics!");
			}
		}
		else {
			spdlog::error("Failed to initialize Segment analytics!");
		}
	}

	if(!notifyInitializationProgress("Creating DOSBox Command Script Files")) {
		return false;
	}

	createDOSBoxTemplateCommandScriptFiles();

	if(!notifyInitializationProgress("Locating Existing Duke Nukem 3D Game Installations")) {
		return false;
	}

	GameLocator * gameLocator = GameLocator::getInstance();

	if(gameLocator->locateGames()) {
		spdlog::info("Located {} Duke Nukem 3D game install{}.", gameLocator->numberOfGamePaths(), gameLocator->numberOfGamePaths() == 1 ? "" : "s");
	}

	if(!notifyInitializationProgress("Initializing Mod Download Manager")) {
		return false;
	}

	if(!m_localMode) {
		m_downloadManager = std::make_shared<DownloadManager>();

		if(!m_downloadManager->initialize()) {
			spdlog::error("Failed to initialize download manager!");
			initializationFailed();
			return false;
		}

		m_organizedMods->setDownloadManager(m_downloadManager);
	}

	m_gameType = settings->gameType;

	if(!notifyInitializationProgress("Initializing DOSBox Manager")) {
		return false;
	}

	if(!m_dosboxManager->initialize()) {
		spdlog::error("Failed to initialize DOSBox manager!");
		initializationFailed();
		return false;
	}

	std::shared_ptr<DOSBoxVersionCollection> dosboxVersions(getDOSBoxVersions());

	m_preferredDOSBoxVersion = dosboxVersions->getDOSBoxVersionWithID(settings->preferredDOSBoxVersionID);

	if(m_preferredDOSBoxVersion == nullptr) {
		m_preferredDOSBoxVersion = dosboxVersions->getDOSBoxVersion(0);

		spdlog::warn("DOSBox configuration for version with ID '{}' is missing, changing preferred DOSBox version to '{}'.", settings->preferredDOSBoxVersionID, m_preferredDOSBoxVersion->getLongName());

		settings->preferredDOSBoxVersionID = m_preferredDOSBoxVersion->getID();
	}

	m_dosboxVersionCollectionSizeChangedConnection = dosboxVersions->sizeChanged.connect(std::bind(&ModManager::onDOSBoxVersionCollectionSizeChanged, this, std::placeholders::_1));
	m_dosboxVersionCollectionItemModifiedConnection = dosboxVersions->itemModified.connect(std::bind(&ModManager::onDOSBoxVersionCollectionItemModified, this, std::placeholders::_1, std::placeholders::_2));

	if(!notifyInitializationProgress("Initializing Game Manager")) {
		return false;
	}

	if(!m_gameManager->initialize()) {
		spdlog::error("Failed to initialize game manager!");
		initializationFailed();
		return false;
	}

	std::shared_ptr<GameVersionCollection> gameVersions(getGameVersions());

	m_preferredGameVersion = gameVersions->getGameVersionWithID(settings->preferredGameVersionID);

	if(m_preferredGameVersion == nullptr) {
		m_preferredGameVersion = gameVersions->getGameVersion(0);

		spdlog::warn("Game configuration for game version '{}' is missing, changing preferred game version to '{}'.", getGameVersions()->getLongNameOfGameVersionWithID(settings->preferredGameVersionID), m_preferredGameVersion->getLongName());

		settings->preferredGameVersionID = m_preferredGameVersion->getID();
	}

	m_gameManager->updateGroupFileSymlinks();

	m_gameVersionCollectionSizeChangedConnection = gameVersions->sizeChanged.connect(std::bind(&ModManager::onGameVersionCollectionSizeChanged, this, std::placeholders::_1));
	m_gameVersionCollectionItemModifiedConnection = gameVersions->itemModified.connect(std::bind(&ModManager::onGameVersionCollectionItemModified, this, std::placeholders::_1, std::placeholders::_2));

	std::string generalDOSBoxConfigurationFilePath(getGeneralDOSBoxConfigurationFilePath());

	if(generalDOSBoxConfigurationFilePath.empty()) {
		spdlog::error("Failed to load general DOSBox configuration file!");
		initializationFailed();
		return false;
	}

	m_generalDOSBoxConfiguration->setFilePath(generalDOSBoxConfigurationFilePath);

	if(std::filesystem::is_regular_file(std::filesystem::path(generalDOSBoxConfigurationFilePath))) {
		std::unique_ptr<DOSBoxConfiguration> generalDOSBoxConfiguration(DOSBoxConfiguration::loadFrom(generalDOSBoxConfigurationFilePath));

		if(generalDOSBoxConfiguration == nullptr) {
			spdlog::error("Failed to load general DOSBox configuration from file: '{}'.", generalDOSBoxConfigurationFilePath);
			initializationFailed();
			return false;
		}

		*m_generalDOSBoxConfiguration = std::move(*generalDOSBoxConfiguration);
		m_generalDOSBoxConfiguration->setModified(false);

		size_t sectionCount = m_generalDOSBoxConfiguration->numberOfSections();
		size_t totalEntryCount = m_generalDOSBoxConfiguration->totalNumberOfEntries();

		spdlog::info("Loaded general DOSBox configuration file with {} section{} and {} {}.", sectionCount, sectionCount == 1 ? "" : "s", totalEntryCount, totalEntryCount == 1 ? "entry" : "entries");
	}

	if(!notifyInitializationProgress("Loading Mod List")) {
		return false;
	}

	if(!m_mods->loadFrom(getModsListFilePath(), getGameVersions().get(), skipFileInfoValidation)) {
		spdlog::error("Failed to load mod list '{}'!", getModsListFilePath());
		initializationFailed();
		return false;
	}

	if(m_mods->numberOfMods() == 0) {
		spdlog::error("No mods loaded!");
		initializationFailed();
		return false;
	}

	if(!m_mods->checkGameVersions(*getGameVersions())) {
		spdlog::error("Found at least one invalid or missing game version.");
		initializationFailed();
		return false;
	}

	spdlog::info("Loaded {} mod{} from '{}'.", m_mods->numberOfMods(), m_mods->numberOfMods() == 1 ? "" : "s", getModsListFilePath());

	if(!notifyInitializationProgress("Loading Installed Stand-Alone Mod List")) {
		return false;
	}

	m_standAloneMods->loadFrom(settings->standAloneModsListFilePath);

	if(m_standAloneMods->numberOfStandAloneMods() != 0) {
		spdlog::info("Loaded {} installed stand-alone mod configuration{} from '{}'.", m_standAloneMods->numberOfStandAloneMods(), m_standAloneMods->numberOfStandAloneMods() == 1 ? "" : "s", settings->standAloneModsListFilePath);
	}

	if(!notifyInitializationProgress("Loading Favourite Mod List")) {
		return false;
	}

	m_favouriteMods->loadFrom(settings->favouriteModsListFilePath);
	m_favouriteMods->checkForMissingFavouriteMods(*m_mods);

	if(m_favouriteMods->numberOfFavourites() != 0) {
		spdlog::info("Loaded {} favourite mod{} from '{}'.", m_favouriteMods->numberOfFavourites(), m_favouriteMods->numberOfFavourites() == 1 ? "" : "s", settings->favouriteModsListFilePath);
	}

	if(!notifyInitializationProgress("Organizing Mods")) {
		return false;
	}

	m_organizedMods->organize();

	std::filesystem::path mapsDirectoryPath(getMapsDirectoryPath());

	if(!mapsDirectoryPath.empty() && !std::filesystem::is_directory(mapsDirectoryPath)) {
		std::error_code errorCode;

		std::filesystem::create_directories(mapsDirectoryPath, errorCode);

		if(errorCode) {
			spdlog::warn("Failed to create maps directory structure '{}'.", mapsDirectoryPath.string(), errorCode.message());
		}
		else {
			spdlog::info("Created maps directory structure: '{}'.", mapsDirectoryPath.string());
		}
	}

	m_initialized = true;

	if(!notifyInitializationProgress("Checking for Missing Files")) {
		return false;
	}

	checkForMissingExecutables();

	if(m_localMode) {
		checkAllModsForMissingFiles();

		checkForUnlinkedModFiles();
	}

	clearApplicationTemporaryDirectory();

	std::chrono::milliseconds initializationDuration(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - initializeSteadyStartTimePoint));

	spdlog::info("{} initialized successfully after {} milliseconds.", APPLICATION_NAME, initializationDuration.count());

	if(settings->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["sessionNumber"] = segmentAnalytics->getSessionNumber();
		properties["initializationDuration"] = initializationDuration.count();
		properties["environment"] = Utilities::toCapitalCase(APPLICATION_ENVIRONMENT);
		properties["gameType"] = Utilities::toCapitalCase(magic_enum::enum_name(settings->gameType));
		properties["demoRecordingEnabled"] = m_demoRecordingEnabled;
		properties["preferredDOSBoxVersionID"] = settings->preferredDOSBoxVersionID;
		properties["preferredGameVersionID"] = settings->preferredGameVersionID;
		properties["numberMods"] = m_mods->numberOfMods();
		properties["numberOfFavouriteMods"] = m_favouriteMods->numberOfFavourites();
		properties["numberOfDownloadedMods"] = m_downloadManager != nullptr ? m_downloadManager->numberOfDownloadedMods() : 0;
		properties["numberOfInstalledStandAloneMods"] = m_standAloneMods->numberOfStandAloneMods();
		properties["dosboxShowConsole"] = settings->dosboxShowConsole;
		properties["dosboxFullscreen"] = settings->dosboxFullscreen;
		properties["dosboxAutoExit"] = settings->dosboxAutoExit;
		properties["dosboxArguments"] = settings->dosboxArguments;
		properties["dosboxServerIPAddressChanged"] = !Utilities::areStringsEqual(settings->dosboxServerIPAddress, SettingsManager::DEFAULT_DOSBOX_SERVER_IP_ADDRESS);
		properties["dosboxServerLocalPortChanged"] = settings->dosboxLocalServerPort != SettingsManager::DEFAULT_DOSBOX_LOCAL_SERVER_PORT;
		properties["dosboxServerRemotePortChanged"] = settings->dosboxRemoteServerPort != SettingsManager::DEFAULT_DOSBOX_REMOTE_SERVER_PORT;
		properties["windowPosition"] = settings->windowPosition.toString();
		properties["windowSize"] = settings->windowSize.toString();
		properties["apiBaseURL"] = settings->apiBaseURL;
		properties["connectionTimeoutSeconds"] = settings->connectionTimeout.count();
		properties["networkTimeoutSeconds"] = settings->networkTimeout.count();
		properties["transferTimeoutSeconds"] = settings->transferTimeout.count();
		properties["downloadThrottlingEnabled"] = settings->downloadThrottlingEnabled;
		properties["verboseRequestLogging"] = settings->verboseRequestLogging;
		properties["modListUpdateFrequencyMinutes"] = settings->modListUpdateFrequency.count();
		properties["dosboxDownloadListUpdateFrequencyMinutes"] = settings->dosboxDownloadListUpdateFrequency.count();
		properties["gameDownloadListUpdateFrequencyMinutes"] = settings->gameDownloadListUpdateFrequency.count();
		properties["cacertUpdateFrequencyMinutes"] = settings->cacertUpdateFrequency.count();
		properties["timeZoneDataUpdateFrequencyMinutes"] = settings->timeZoneDataUpdateFrequency.count();

		if(settings->modListLastDownloadedTimestamp.has_value()) {
			properties["modListLastDownloaded"] = Utilities::timePointToString(settings->modListLastDownloadedTimestamp.value(), Utilities::TimeFormat::ISO8601);
		}

		if(settings->dosboxDownloadListLastDownloadedTimestamp.has_value()) {
			properties["dosboxDownloadListLastDownloaded"] = Utilities::timePointToString(settings->dosboxDownloadListLastDownloadedTimestamp.value(), Utilities::TimeFormat::ISO8601);
		}

		if(settings->gameDownloadListLastDownloadedTimestamp.has_value()) {
			properties["gameDownloadListLastDownloaded"] = Utilities::timePointToString(settings->gameDownloadListLastDownloadedTimestamp.value(), Utilities::TimeFormat::ISO8601);
		}

		if(settings->cacertLastDownloadedTimestamp.has_value()) {
			properties["cacertLastDownloaded"] = Utilities::timePointToString(settings->cacertLastDownloadedTimestamp.value(), Utilities::TimeFormat::ISO8601);
		}

		if(settings->timeZoneDataLastDownloadedTimestamp.has_value()) {
			properties["timeZoneDataLastDownloaded"] = Utilities::timePointToString(settings->timeZoneDataLastDownloadedTimestamp.value(), Utilities::TimeFormat::ISO8601);
		}

		if(m_arguments != nullptr) {
			properties["arguments"] = m_arguments->toString();
		}

		segmentAnalytics->track("Application Initialized", properties);
	}

	if(!handleArguments(m_arguments.get())) {
		initializationFailed();
		return false;
	}

	if(!notifyInitializationProgress("Initialization Complete")) {
		return false;
	}

	initialized();

	return true;
}

bool ModManager::uninitialize() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	SettingsManager * settings = SettingsManager::getInstance();

	settings->save(m_arguments.get());

	if(m_standAloneMods->saveTo(settings->standAloneModsListFilePath)) {
		spdlog::info("Installed stand-alone mod list saved to file: '{}'.", settings->standAloneModsListFilePath);
	}
	else {
		spdlog::error("Failed to save Installed stand-alone mod list to file: '{}'.", settings->standAloneModsListFilePath);
	}

	if(m_favouriteMods->saveTo(settings->favouriteModsListFilePath)) {
		spdlog::info("Favourite mods list saved to file: '{}'.", settings->favouriteModsListFilePath);
	}
	else {
		spdlog::error("Failed to save favourite mods list to file: '{}'.", settings->favouriteModsListFilePath);
	}

	if(getDOSBoxVersions()->saveTo(settings->dosboxVersionsListFilePath)) {
		spdlog::info("DOSBox version configurations saved to file: '{}'.", settings->dosboxVersionsListFilePath);
	}
	else {
		spdlog::error("Failed to save DOSBox version configurations to file: '{}'.", settings->dosboxVersionsListFilePath);
	}

	if(getGameVersions()->saveTo(settings->gameVersionsListFilePath)) {
		spdlog::info("Game version configurations saved to file: '{}'.", settings->gameVersionsListFilePath);
	}
	else {
		spdlog::error("Failed to save game version configurations to file: '{}'.", settings->gameVersionsListFilePath);
	}

	SegmentAnalytics * segmentAnalytics = SegmentAnalytics::getInstance();
	segmentAnalytics->onApplicationClosed();
	segmentAnalytics->flush(3s);

	m_selectedMod.reset();
	m_organizedMods->setModCollection(nullptr);
	m_organizedMods->setFavouriteModCollection(nullptr);
	m_organizedMods->setGameVersionCollection(nullptr);
	m_standAloneMods->clearStandAloneMods();
	m_favouriteMods->clearFavourites();
	m_mods->clearMods();

	if(m_arguments != nullptr) {
		m_arguments.reset();
	}

	m_initialized = false;

	return true;
}

bool ModManager::isUsingLocalMode() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_localMode;
}

std::shared_ptr<ModCollection> ModManager::getMods() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_mods;
}

std::shared_ptr<StandAloneModCollection> ModManager::getStandAloneMods() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_standAloneMods;
}

std::shared_ptr<FavouriteModCollection> ModManager::getFavouriteMods() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_favouriteMods;
}

std::shared_ptr<OrganizedModCollection> ModManager::getOrganizedMods() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_organizedMods;
}

std::shared_ptr<DOSBoxConfiguration> ModManager::getGeneralDOSBoxConfiguration() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_generalDOSBoxConfiguration;
}

std::string ModManager::getModsListFilePath() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_localMode) {
		return SettingsManager::getInstance()->modsListFilePath;
	}

	if(m_downloadManager == nullptr) {
		return Utilities::emptyString;
	}

	return m_downloadManager->getCachedModListFilePath();
}

std::string ModManager::getModsDirectoryPath() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_localMode) {
		return SettingsManager::getInstance()->modsDirectoryPath;
	}

	if(m_downloadManager == nullptr) {
		return Utilities::emptyString;
	}

	return m_downloadManager->getDownloadedModsDirectoryPath();
}

std::string ModManager::getMapsDirectoryPath() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_localMode) {
		return SettingsManager::getInstance()->mapsDirectoryPath;
	}

	if(m_downloadManager == nullptr) {
		return Utilities::emptyString;
	}

	return m_downloadManager->getDownloadedMapsDirectoryPath();
}


std::string ModManager::getGeneralDOSBoxConfigurationFilePath() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::string dosboxConfigurationsDirectoryPath(getDOSBoxConfigurationsDirectoryPath());

	if(dosboxConfigurationsDirectoryPath.empty()) {
		return {};
	}

	return Utilities::joinPaths(dosboxConfigurationsDirectoryPath, GENERAL_DOSBOX_CONFIGURATION_FILE_NAME);
}

std::string ModManager::getDOSBoxConfigurationsDirectoryPath() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->dataDirectoryPath.empty()) {
		spdlog::error("Missing data directory path setting.");
		return {};
	}

	if(settings->dosboxDataDirectoryName.empty()) {
		spdlog::error("Missing DOSBox data directory name setting.");
		return {};
	}

	if(settings->dosboxConfigurationsDirectoryName.empty()) {
		spdlog::error("Missing DOSBox configurations directory name setting.");
		return {};
	}

	return Utilities::joinPaths(settings->dataDirectoryPath, settings->dosboxDataDirectoryName, settings->dosboxConfigurationsDirectoryName);
}

GameType ModManager::getGameType() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_gameType;
}

bool ModManager::setGameType(const std::string & gameTypeName) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::optional<GameType> gameTypeOptional = magic_enum::enum_cast<GameType>(Utilities::toPascalCase(gameTypeName));

	if(gameTypeOptional.has_value()) {
		setGameType(gameTypeOptional.value());

		return true;
	}

	return false;
}

void ModManager::setGameType(GameType gameType) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_gameType == gameType) {
		return;
	}

	GameType previousGameType = m_gameType;

	m_gameType = gameType;

	SettingsManager::getInstance()->gameType = m_gameType;

	gameTypeChanged(m_gameType);

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["previousGameType"] = magic_enum::enum_name(previousGameType);
		properties["newGameType"] = magic_enum::enum_name(m_gameType);

		SegmentAnalytics::getInstance()->track("Game Type Changed", properties);
	}
}

bool ModManager::hasPreferredDOSBoxVersion() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_preferredDOSBoxVersion != nullptr;
}

std::shared_ptr<DOSBoxVersion> ModManager::getPreferredDOSBoxVersion() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_preferredDOSBoxVersion;
}

std::shared_ptr<DOSBoxVersion> ModManager::getSelectedDOSBoxVersion() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::shared_ptr<DOSBoxVersion> selectedDOSBoxVersion;

	if(m_arguments != nullptr && m_arguments->hasArgument("dosbox") && !m_arguments->getFirstValue("dosbox").empty()) {
		std::string dosboxVersionID(m_arguments->getFirstValue("dosbox"));

		selectedDOSBoxVersion = getDOSBoxVersions()->getDOSBoxVersionWithID(dosboxVersionID);

		if(selectedDOSBoxVersion == nullptr) {
			spdlog::error("Could not find DOSBox version override for '{}'.", dosboxVersionID);
		}
	}

	if(selectedDOSBoxVersion == nullptr) {
		if(m_preferredDOSBoxVersion == nullptr) {
			spdlog::error("No preferred DOSBox version selected.");
			return nullptr;
		}

		selectedDOSBoxVersion = m_preferredDOSBoxVersion;
	}

	if(!DOSBoxVersion::isValid(selectedDOSBoxVersion.get())) {
		spdlog::error("Invalid selected DOSBox version.");
		return false;
	}

	return selectedDOSBoxVersion;
}

bool ModManager::setPreferredDOSBoxVersionByID(const std::string & dosboxVersionID) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(dosboxVersionID.empty()) {
		return false;
	}

	return setPreferredDOSBoxVersion(getDOSBoxVersions()->getDOSBoxVersionWithID(dosboxVersionID));
}

bool ModManager::setPreferredDOSBoxVersion(std::shared_ptr<DOSBoxVersion> dosboxVersion) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(dosboxVersion == nullptr || !dosboxVersion->isValid()) {
		return false;
	}

	if(m_preferredDOSBoxVersion != nullptr && Utilities::areStringsEqualIgnoreCase(m_preferredDOSBoxVersion->getID(), dosboxVersion->getID())) {
		return true;
	}

	std::shared_ptr<DOSBoxVersion> previousPreferredDOSBoxVersion(m_preferredDOSBoxVersion);

	m_preferredDOSBoxVersion = dosboxVersion;
	SettingsManager::getInstance()->preferredDOSBoxVersionID = m_preferredDOSBoxVersion->getID();

	preferredDOSBoxVersionChanged(m_preferredDOSBoxVersion);

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		if(previousPreferredDOSBoxVersion != nullptr) {
			properties["previousDOSBoxVersionID"] = previousPreferredDOSBoxVersion->getID();
		}

		properties["newDOSBoxVersionID"] = m_preferredDOSBoxVersion->getID();

		SegmentAnalytics::getInstance()->track("Preferred DOSBox Version Changed", properties);
	}

	return true;
}

bool ModManager::hasPreferredGameVersion() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_preferredGameVersion != nullptr;
}

std::shared_ptr<GameVersion> ModManager::getPreferredGameVersion() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_preferredGameVersion;
}

std::shared_ptr<GameVersion> ModManager::getSelectedGameVersion() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::shared_ptr<ModGameVersion> selectedModGameVersion(getSelectedModGameVersion());

	if(selectedModGameVersion != nullptr && selectedModGameVersion->isStandAlone()) {
		std::shared_ptr<StandAloneMod> installedStandAloneMod(m_standAloneMods->getStandAloneMod(*selectedModGameVersion));

		if(installedStandAloneMod != nullptr) {
			return installedStandAloneMod;
		}

		return selectedModGameVersion->getStandAloneGameVersion();
	}

	std::shared_ptr<GameVersion> selectedGameVersion;

	if(m_arguments != nullptr && m_arguments->hasArgument("game") && !m_arguments->getFirstValue("game").empty()) {
		std::string gameVersionID(m_arguments->getFirstValue("game"));

		selectedGameVersion = getGameVersions()->getGameVersionWithID(gameVersionID);

		if(selectedGameVersion == nullptr) {
			spdlog::error("Could not find game version override for '{}'.", getGameVersions()->getLongNameOfGameVersionWithID(gameVersionID));
		}
	}

	if(selectedGameVersion == nullptr) {
		if(m_preferredGameVersion == nullptr) {
			spdlog::error("No preferred game version selected.");
			return nullptr;
		}

		selectedGameVersion = m_preferredGameVersion;
	}

	if(!GameVersion::isValid(selectedGameVersion.get())) {
		spdlog::error("Invalid selected game version.");
		return false;
	}

	return selectedGameVersion;
}

bool ModManager::setPreferredGameVersionByID(const std::string & gameVersionID) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(gameVersionID.empty()) {
		return false;
	}

	return setPreferredGameVersion(getGameVersions()->getGameVersionWithID(gameVersionID));
}

bool ModManager::setPreferredGameVersion(std::shared_ptr<GameVersion> gameVersion) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(gameVersion == nullptr || !gameVersion->isValid()) {
		return false;
	}

	if(m_preferredGameVersion != nullptr && Utilities::areStringsEqualIgnoreCase(m_preferredGameVersion->getID(), gameVersion->getID())) {
		return true;
	}

	std::shared_ptr<GameVersion> previousPreferredGameVersion(m_preferredGameVersion);

	m_preferredGameVersion = gameVersion;
	SettingsManager::getInstance()->preferredGameVersionID = m_preferredGameVersion->getID();

	m_selectedModGameVersionIndex = std::numeric_limits<size_t>::max();
	std::shared_ptr<ModVersionType> selectedModVersionType(getSelectedModVersionType());

	if(selectedModVersionType != nullptr) {
		if(selectedModVersionType->isStandAlone()) {
			m_selectedModGameVersionIndex = 0;
		}
		else {
			std::shared_ptr<GameVersion> selectedGameVersion(getSelectedGameVersion());

			if(selectedGameVersion != nullptr) {
				std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(selectedModVersionType->getCompatibleModGameVersions(*selectedGameVersion));

				if(!compatibleModGameVersions.empty()) {
					m_selectedModGameVersionIndex = selectedModVersionType->indexOfGameVersionWithID(compatibleModGameVersions.back()->getGameVersionID());
				}
			}
		}
	}

	notifyModSelectionChanged();
	preferredGameVersionChanged(m_preferredGameVersion);

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		if(previousPreferredGameVersion != nullptr) {
			properties["previousGameVersionID"] = previousPreferredGameVersion->getID();
		}

		properties["newGameVersionID"] = m_preferredGameVersion->getID();

		SegmentAnalytics::getInstance()->track("Preferred Game Version Changed", properties);
	}

	return true;
}

std::shared_ptr<DOSBoxManager> ModManager::getDOSBoxManager() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_dosboxManager;
}

std::shared_ptr<DOSBoxVersionCollection> ModManager::getDOSBoxVersions() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_dosboxManager->getDOSBoxVersions();
}

std::shared_ptr<GameVersionCollection> ModManager::getGameVersions() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_gameManager->getGameVersions();
}

std::shared_ptr<GameManager> ModManager::getGameManager() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_gameManager;
}

std::shared_ptr<DownloadManager> ModManager::getDownloadManager() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_downloadManager;
}

const std::string & ModManager::getDOSBoxServerIPAddress() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return SettingsManager::getInstance()->dosboxServerIPAddress;
}

void ModManager::setDOSBoxServerIPAddress(const std::string & ipAddress) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	SettingsManager * settings = SettingsManager::getInstance();

	std::string formattedIPAddress(Utilities::trimString(ipAddress));

	if(Utilities::areStringsEqual(settings->dosboxServerIPAddress, formattedIPAddress)) {
		return;
	}

	settings->dosboxServerIPAddress = formattedIPAddress;

	dosboxServerIPAddressChanged(settings->dosboxServerIPAddress);
}

uint16_t ModManager::getDOSBoxLocalServerPort() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return SettingsManager::getInstance()->dosboxLocalServerPort;
}

void ModManager::setDOSBoxLocalServerPort(uint16_t port) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->dosboxLocalServerPort == port) {
		return;
	}

	settings->dosboxLocalServerPort = port;

	dosboxLocalServerPortChanged(settings->dosboxLocalServerPort);
}

uint16_t ModManager::getDOSBoxRemoteServerPort() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return SettingsManager::getInstance()->dosboxRemoteServerPort;
}

void ModManager::setDOSBoxRemoteServerPort(uint16_t port) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->dosboxRemoteServerPort == port) {
		return;
	}

	settings->dosboxRemoteServerPort = port;

	dosboxRemoteServerPortChanged(settings->dosboxRemoteServerPort);
}

bool ModManager::hasModSelected() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_selectedMod != nullptr;
}

bool ModManager::hasModVersionSelected() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_selectedModVersionIndex != std::numeric_limits<size_t>::max();
}

bool ModManager::hasModVersionTypeSelected() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_selectedModVersionTypeIndex != std::numeric_limits<size_t>::max();
}

bool ModManager::hasModGameVersionSelected() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_selectedModGameVersionIndex != std::numeric_limits<size_t>::max();
}

std::shared_ptr<Mod> ModManager::getSelectedMod() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_selectedMod;
}

std::shared_ptr<ModVersion> ModManager::getSelectedModVersion() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_selectedMod == nullptr || m_selectedModVersionIndex >= m_selectedMod->numberOfVersions()) {
		return nullptr;
	}

	return m_selectedMod->getVersion(m_selectedModVersionIndex);
}

std::shared_ptr<ModVersionType> ModManager::getSelectedModVersionType() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::shared_ptr<ModVersion> selectedModVersion(getSelectedModVersion());

	if(selectedModVersion == nullptr || m_selectedModVersionTypeIndex >= selectedModVersion->numberOfTypes()) {
		return nullptr;
	}

	return selectedModVersion->getType(m_selectedModVersionTypeIndex);
}

std::shared_ptr<ModGameVersion> ModManager::getSelectedModGameVersion() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::shared_ptr<ModVersionType> selectedModVersionType(getSelectedModVersionType());

	if(selectedModVersionType == nullptr || m_selectedModGameVersionIndex >= selectedModVersionType->numberOfGameVersions()) {
		return nullptr;
	}

	return selectedModVersionType->getGameVersion(m_selectedModGameVersionIndex);
}

std::optional<std::string> ModManager::getSelectedModName() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_selectedMod == nullptr) {
		return {};
	}

	return m_selectedMod->getName();
}

size_t ModManager::getSelectedModVersionIndex() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_selectedModVersionIndex;
}

size_t ModManager::getSelectedModVersionTypeIndex() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_selectedModVersionTypeIndex;
}

size_t ModManager::getSelectedModGameVersionIndex() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_selectedModGameVersionIndex;
}

bool ModManager::setSelectedModByID(const std::string & id) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return setSelectedMod(m_mods->getModWithID(id));
}

bool ModManager::setSelectedMod(std::shared_ptr<Mod> mod) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!Mod::isValid(mod.get(), true)) {
		m_selectedMod = nullptr;
		m_selectedModVersionIndex = std::numeric_limits<size_t>::max();
		m_selectedModVersionTypeIndex = std::numeric_limits<size_t>::max();
		m_selectedModGameVersionIndex = std::numeric_limits<size_t>::max();

		notifyModSelectionChanged();

		return false;
	}

	if(m_selectedMod == mod) {
		return true;
	}

	m_selectedMod = mod;
	m_selectedModVersionIndex = std::numeric_limits<size_t>::max();
	m_selectedModVersionTypeIndex = std::numeric_limits<size_t>::max();
	m_selectedModGameVersionIndex = std::numeric_limits<size_t>::max();

	if(m_selectedMod != nullptr) {
		if(m_selectedMod->numberOfVersions() == 1) {
			m_selectedModVersionIndex = 0;
		}
		else {
			m_selectedModVersionIndex = m_selectedMod->indexOfPreferredVersion();
		}

		if(m_selectedModVersionIndex != std::numeric_limits<size_t>::max()) {
			std::shared_ptr<ModVersion> selectedModVersion(m_selectedMod->getVersion(m_selectedModVersionIndex));

			if(selectedModVersion->numberOfTypes() == 1) {
				m_selectedModVersionTypeIndex = 0;
			}
			else {
				m_selectedModVersionTypeIndex = m_selectedMod->indexOfDefaultVersionType();
			}

			if(m_selectedModVersionTypeIndex != std::numeric_limits<size_t>::max()) {
				std::shared_ptr<ModVersionType> selectedModVersionType(selectedModVersion->getType(m_selectedModVersionTypeIndex));

				if(selectedModVersionType->isStandAlone()) {
					m_selectedModGameVersionIndex = 0;
				}
				else {
					std::shared_ptr<GameVersion> selectedGameVersion(getSelectedGameVersion());

					if(selectedGameVersion != nullptr) {
						std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(selectedModVersionType->getCompatibleModGameVersions(*selectedGameVersion));

						if(!compatibleModGameVersions.empty()) {
							m_selectedModGameVersionIndex = selectedModVersionType->indexOfGameVersionWithID(compatibleModGameVersions.back()->getGameVersionID());
						}
					}
				}
			}
		}
	}

	notifyModSelectionChanged();

	return true;
}

bool ModManager::setSelectedModFromMatch(const ModMatch & modMatch) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!modMatch.isValid()) {
		return false;
	}

	m_selectedMod = modMatch.getMod();
	m_selectedModVersionTypeIndex = modMatch.getModVersionTypeIndex();
	m_selectedModVersionIndex = modMatch.getModVersionIndex();
	m_selectedModGameVersionIndex = std::numeric_limits<size_t>::max();

	if(m_selectedModVersionIndex == std::numeric_limits<size_t>::max()) {
		if(m_selectedMod->numberOfVersions() == 1) {
			m_selectedModVersionIndex = 0;
		}
		else {
			m_selectedModVersionIndex = m_selectedMod->indexOfPreferredVersion();
		}
	}

	if(m_selectedModVersionIndex != std::numeric_limits<size_t>::max()) {
		std::shared_ptr<ModVersion> selectedModVersion(m_selectedMod->getVersion(m_selectedModVersionIndex));

		if(selectedModVersion->numberOfTypes() == 1) {
			m_selectedModVersionTypeIndex = 0;
		}
		else {
			m_selectedModVersionTypeIndex = m_selectedMod->indexOfDefaultVersionType();
		}

		if(m_selectedModVersionTypeIndex != std::numeric_limits<size_t>::max()) {
			std::shared_ptr<ModVersionType> selectedModVersionType(selectedModVersion->getType(m_selectedModVersionTypeIndex));

			if(selectedModVersionType->isStandAlone()) {
				m_selectedModGameVersionIndex = 0;
			}
			else {
				std::shared_ptr<GameVersion> selectedGameVersion(getSelectedGameVersion());

				if(selectedGameVersion != nullptr) {
					std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(selectedModVersionType->getCompatibleModGameVersions(*selectedGameVersion));

					if(!compatibleModGameVersions.empty()) {
						m_selectedModGameVersionIndex = selectedModVersionType->indexOfGameVersionWithID(compatibleModGameVersions.back()->getGameVersionID());
					}
				}
			}
		}
	}

	notifyModSelectionChanged();

	return true;
}

bool ModManager::setSelectedMod(const ModIdentifier & modIdentifier) {
	if(!modIdentifier.isValid()) {
		return false;
	}

	std::shared_ptr<Mod> selectedMod(m_mods->getModWithName(modIdentifier.getName()));

	m_selectedMod = selectedMod;
	m_selectedModVersionIndex = std::numeric_limits<size_t>::max();
	m_selectedModVersionTypeIndex = std::numeric_limits<size_t>::max();
	m_selectedModGameVersionIndex = std::numeric_limits<size_t>::max();

	if(selectedMod == nullptr) {
		return false;
	}

	if(modIdentifier.hasVersion()) {
		m_selectedModVersionIndex = m_selectedMod->indexOfVersion(modIdentifier.getVersion().value());
	}

	if(m_selectedModVersionIndex == std::numeric_limits<size_t>::max()) {
		if(m_selectedMod->numberOfVersions() == 1) {
			m_selectedModVersionIndex = 0;
		}
		else {
			m_selectedModVersionIndex = m_selectedMod->indexOfPreferredVersion();
		}
	}

	if(m_selectedModVersionIndex != std::numeric_limits<size_t>::max()) {
		std::shared_ptr<ModVersion> selectedModVersion(m_selectedMod->getVersion(m_selectedModVersionIndex));

		if(modIdentifier.hasVersionType()) {
			m_selectedModVersionTypeIndex = selectedModVersion->indexOfType(modIdentifier.getVersionType().value());
		}

		if(selectedModVersion->numberOfTypes() == 1) {
			m_selectedModVersionTypeIndex = 0;
		}
		else {
			m_selectedModVersionTypeIndex = m_selectedMod->indexOfDefaultVersionType();
		}

		if(m_selectedModVersionTypeIndex != std::numeric_limits<size_t>::max()) {
			std::shared_ptr<ModVersionType> selectedModVersionType(selectedModVersion->getType(m_selectedModVersionTypeIndex));

			if(selectedModVersionType->isStandAlone()) {
				m_selectedModGameVersionIndex = 0;
			}
			else {
				std::shared_ptr<GameVersion> selectedGameVersion(getSelectedGameVersion());

				if(selectedGameVersion != nullptr) {
					std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(selectedModVersionType->getCompatibleModGameVersions(*selectedGameVersion));

					if(!compatibleModGameVersions.empty()) {
						m_selectedModGameVersionIndex = selectedModVersionType->indexOfGameVersionWithID(compatibleModGameVersions.back()->getGameVersionID());
					}
				}
			}
		}
	}

	notifyModSelectionChanged();

	return true;
}

bool ModManager::setSelectedMod(const std::string & modID, const std::string & modVersion, const std::string & modVersionType) {
	if(modID.empty()) {
		return false;
	}

	std::shared_ptr<Mod> selectedMod(m_mods->getModWithID(modID));

	if(selectedMod == nullptr) {
		return false;
	}

	size_t selectedModVersionIndex = selectedMod->indexOfVersion(modVersion);

	if(selectedModVersionIndex == std::numeric_limits<size_t>::max()) {
		return false;
	}

	size_t selectedModVersionTypeIndex = selectedMod->getVersion(selectedModVersionIndex)->indexOfType(modVersionType);

	if(selectedModVersionTypeIndex == std::numeric_limits<size_t>::max()) {
		return false;
	}

	m_selectedMod = selectedMod;
	m_selectedModVersionIndex = selectedModVersionIndex;
	m_selectedModVersionTypeIndex = selectedModVersionTypeIndex;
	m_selectedModGameVersionIndex = std::numeric_limits<size_t>::max();

	std::shared_ptr<ModVersionType> selectedModVersionType(m_selectedMod->getVersion(m_selectedModVersionIndex)->getType(m_selectedModVersionTypeIndex));

	if(selectedModVersionType->isStandAlone()) {
		m_selectedModGameVersionIndex = 0;
	}
	else {
		std::shared_ptr<GameVersion> selectedGameVersion(getSelectedGameVersion());

		if(selectedGameVersion != nullptr) {
			std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(selectedModVersionType->getCompatibleModGameVersions(*selectedGameVersion));

			if(!compatibleModGameVersions.empty()) {
				m_selectedModGameVersionIndex = selectedModVersionType->indexOfGameVersionWithID(compatibleModGameVersions.back()->getGameVersionID());
			}
		}
	}

	notifyModSelectionChanged();

	return true;
}

bool ModManager::setSelectedModVersionIndex(size_t modVersionIndex) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(modVersionIndex == std::numeric_limits<size_t>::max()) {
		bool didModSelectionChange = m_selectedModVersionIndex != std::numeric_limits<size_t>::max() ||
									 m_selectedModVersionTypeIndex != std::numeric_limits<size_t>::max() ||
									 m_selectedModGameVersionIndex != std::numeric_limits<size_t>::max();

		m_selectedModVersionIndex = std::numeric_limits<size_t>::max();
		m_selectedModVersionTypeIndex = std::numeric_limits<size_t>::max();
		m_selectedModGameVersionIndex = std::numeric_limits<size_t>::max();

		if(didModSelectionChange) {
			notifyModSelectionChanged();
		}

		return true;
	}

	if(!Mod::isValid(m_selectedMod.get(), true)) {
		return false;
	}

	size_t newModVersionIndex = std::numeric_limits<size_t>::max();

	if(modVersionIndex < m_selectedMod->numberOfVersions()) {
		newModVersionIndex = modVersionIndex;
	}

	if(m_selectedModVersionIndex == newModVersionIndex) {
		return true;
	}

	m_selectedModVersionIndex = newModVersionIndex;
	m_selectedModVersionTypeIndex = std::numeric_limits<size_t>::max();
	m_selectedModGameVersionIndex = std::numeric_limits<size_t>::max();
	std::shared_ptr<ModVersion> selectedModVersion(m_selectedMod->getVersion(m_selectedModVersionIndex));

	if(selectedModVersion->numberOfTypes() == 1) {
		m_selectedModVersionTypeIndex = 0;
	}
	else {
		m_selectedModVersionTypeIndex = m_selectedMod->indexOfDefaultVersionType();
	}

	if(m_selectedModVersionTypeIndex != std::numeric_limits<size_t>::max()) {
		std::shared_ptr<ModVersionType> selectedModVersionType(selectedModVersion->getType(m_selectedModVersionTypeIndex));

		if(selectedModVersionType->isStandAlone()) {
			m_selectedModGameVersionIndex = 0;
		}
		else {
			std::shared_ptr<GameVersion> selectedGameVersion(getSelectedGameVersion());

			if(selectedGameVersion != nullptr) {
				std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(selectedModVersionType->getCompatibleModGameVersions(*selectedGameVersion));

				if(!compatibleModGameVersions.empty()) {
					m_selectedModGameVersionIndex = selectedModVersionType->indexOfGameVersionWithID(compatibleModGameVersions.back()->getGameVersionID());
				}
			}
		}
	}

	notifyModSelectionChanged();

	return true;
}

bool ModManager::setSelectedModVersionTypeIndex(size_t modVersionTypeIndex) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(modVersionTypeIndex == std::numeric_limits<size_t>::max()) {
		bool modVersionTypeChanged = m_selectedModVersionTypeIndex != std::numeric_limits<size_t>::max() ||
									 m_selectedModGameVersionIndex != std::numeric_limits<size_t>::max();

		m_selectedModVersionTypeIndex = std::numeric_limits<size_t>::max();
		m_selectedModGameVersionIndex = std::numeric_limits<size_t>::max();

		if(modVersionTypeChanged) {
			notifyModSelectionChanged();
		}

		return true;
	}

	if(!Mod::isValid(m_selectedMod.get(), true) || m_selectedModVersionIndex >= m_selectedMod->numberOfVersions()) {
		return false;
	}

	size_t newModVersionTypeIndex = std::numeric_limits<size_t>::max();
	std::shared_ptr<ModVersion> selectedModVersion(m_selectedMod->getVersion(m_selectedModVersionIndex));

	if(modVersionTypeIndex < selectedModVersion->numberOfTypes()) {
		newModVersionTypeIndex = modVersionTypeIndex;
	}

	if(m_selectedModVersionTypeIndex == newModVersionTypeIndex) {
		return true;
	}

	m_selectedModVersionTypeIndex = newModVersionTypeIndex;
	m_selectedModGameVersionIndex = std::numeric_limits<size_t>::max();

	if(m_selectedModVersionTypeIndex != std::numeric_limits<size_t>::max()) {
		std::shared_ptr<ModVersionType> selectedModVersionType(selectedModVersion->getType(m_selectedModVersionTypeIndex));

		if(selectedModVersionType->isStandAlone()) {
			m_selectedModGameVersionIndex = 0;
		}
		else {
			std::shared_ptr<GameVersion> selectedGameVersion(getSelectedGameVersion());

			if(selectedGameVersion != nullptr) {
				std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(selectedModVersionType->getCompatibleModGameVersions(*selectedGameVersion));

				if(!compatibleModGameVersions.empty()) {
					m_selectedModGameVersionIndex = selectedModVersionType->indexOfGameVersionWithID(compatibleModGameVersions.back()->getGameVersionID());
				}
			}
		}
	}

	notifyModSelectionChanged();

	return true;
}

bool ModManager::setSelectedModGameVersionIndex(size_t modGameVersionIndex) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(modGameVersionIndex == std::numeric_limits<size_t>::max()) {
		bool modGameVersionChanged = m_selectedModGameVersionIndex != std::numeric_limits<size_t>::max();

		m_selectedModGameVersionIndex = std::numeric_limits<size_t>::max();

		if(modGameVersionChanged) {
			notifyModSelectionChanged();
		}

		return true;
	}

	if(!Mod::isValid(m_selectedMod.get(), true) || m_selectedModVersionIndex >= m_selectedMod->numberOfVersions()) {
		return false;
	}

	std::shared_ptr<ModVersion> selectedModVersion(m_selectedMod->getVersion(m_selectedModVersionIndex));

	if(m_selectedModVersionTypeIndex >= selectedModVersion->numberOfTypes()) {
		return false;
	}

	std::shared_ptr<ModVersionType> selectedModVersionType(selectedModVersion->getType(m_selectedModVersionTypeIndex));
	std::shared_ptr<GameVersion> selectedGameVersion(getSelectedGameVersion());
	size_t newModGameVersionIndex = std::numeric_limits<size_t>::max();

	if(modGameVersionIndex < selectedModVersionType->numberOfGameVersions() && selectedGameVersion != nullptr && selectedModVersionType->isGameVersionCompatible(*selectedGameVersion)) {
		newModGameVersionIndex = modGameVersionIndex;
	}

	if(m_selectedModGameVersionIndex == newModGameVersionIndex) {
		return true;
	}

	m_selectedModGameVersionIndex = newModGameVersionIndex;

	notifyModSelectionChanged();

	return true;
}

bool ModManager::selectRandomMod(bool selectPreferredVersion, bool selectFirstVersionType) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	if(!m_organizedMods->selectRandomMod()) {
		return false;
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		SegmentAnalytics::getInstance()->track("Random Mod Selected");
	}

	return true;
}

bool ModManager::selectRandomGameVersion() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	if(!m_organizedMods->selectRandomGameVersion()) {
		return false;
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		SegmentAnalytics::getInstance()->track("Random Game Selected");
	}

	return true;
}

bool ModManager::selectRandomTeam() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	if(!m_organizedMods->selectRandomTeam()) {
		return false;
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		SegmentAnalytics::getInstance()->track("Random Team Selected");
	}

	return true;
}

bool ModManager::selectRandomAuthor() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	if(!m_organizedMods->selectRandomAuthor()) {
		return false;
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		SegmentAnalytics::getInstance()->track("Random Author Selected");
	}

	return true;
}

std::vector<ModMatch> ModManager::searchForMod(const std::vector<std::shared_ptr<Mod>> & mods, const std::string & query, bool autoPopulateVersion, bool autoPopulateVersionType) {
	std::string formattedQuery(Utilities::toLowerCase(Utilities::trimString(query)));

	if(formattedQuery.empty()) {
		return {};
	}

	std::vector<ModMatch> matches;
	std::shared_ptr<Mod> mod;
	std::shared_ptr<ModVersion> modVersion;
	std::shared_ptr<ModVersionType> modVersionType;
	std::string formattedModName;
	std::string formattedModVersion;
	std::string formattedModVersionType;
	bool modNameMatches = false;
	bool modNamePartiallyMatches = false;
	bool modVersionMatches = false;
	bool modVersionPartiallyMatches = false;
	bool modVersionTypeMatches = false;
	bool modVersionTypePartiallyMatches = false;
	bool exactMatchFound = false;

	for(size_t i = 0; i < mods.size(); i++) {
		mod = mods[i];

		if(!Mod::isValid(mod.get(), true)) {
			continue;
		}

		formattedModName = Utilities::toLowerCase(mod->getName());
		modNameMatches = formattedQuery == formattedModName;

		if(modNameMatches && mod->numberOfVersions() == 1 && mod->getVersion(0)->numberOfTypes() == 1) {
			matches.clear();

			if(autoPopulateVersion) {
				if(autoPopulateVersionType) {
					matches.emplace_back(mod, mod->getVersion(0), mod->getVersion(0)->getType(0), i, 0, 0);
				}
				else {
					matches.emplace_back(mod, mod->getVersion(0), i, 0);
				}
			}
			else {
				matches.emplace_back(mod, i);
			}

			exactMatchFound = true;
			break;
		}

		modNamePartiallyMatches = formattedModName.find(formattedQuery) != std::string::npos;

		for(size_t j = 0; j < mod->numberOfVersions(); j++) {
			modVersion = mod->getVersion(j);
			formattedModVersion = Utilities::toLowerCase(modVersion->getFullName());
			modVersionMatches = formattedQuery == formattedModVersion;

			if((modVersionMatches && modVersion->numberOfTypes() == 1) ||
			   (modNameMatches && modVersion->getVersion().empty() && modVersion->numberOfTypes() == 1)) {
				matches.clear();

				if(autoPopulateVersionType) {
					matches.emplace_back(mod, modVersion, modVersion->getType(0), i, j, 0);
				}
				else {
					matches.emplace_back(mod, modVersion, i, j);
				}

				exactMatchFound = true;
				break;
			}

			modVersionPartiallyMatches = formattedModVersion.find(formattedQuery) != std::string::npos;

			for(size_t k = 0; k < modVersion->numberOfTypes(); k++) {
				modVersionType = modVersion->getType(k);
				formattedModVersionType = Utilities::toLowerCase(modVersionType->getFullName());
				modVersionTypeMatches = formattedQuery == formattedModVersionType;

				if(modVersionTypeMatches) {
					matches.clear();
					matches.emplace_back(mod, modVersion, modVersionType, i, j, k);

					exactMatchFound = true;
					break;
				}

				modVersionTypePartiallyMatches = formattedModVersionType.find(formattedQuery) != std::string::npos;

				if(modVersionTypePartiallyMatches && !modNamePartiallyMatches && !modVersionPartiallyMatches) {
					matches.emplace_back(mod, modVersion, modVersionType, i, j, k);
				}
			}

			if(exactMatchFound) {
				break;
			}

			if(modVersionPartiallyMatches && !modNamePartiallyMatches) {
				if(modVersion->numberOfTypes() == 1 && autoPopulateVersionType) {
					matches.emplace_back(mod, modVersion, modVersion->getType(0), i, j, 0);
				}
				else {
					matches.emplace_back(mod, modVersion, i, j);
				}
			}
		}

		if(exactMatchFound) {
			break;
		}

		if(modNamePartiallyMatches) {
			if(mod->numberOfVersions() == 1 && autoPopulateVersion) {
				if(mod->getVersion(0)->numberOfTypes() == 1 && autoPopulateVersionType) {
					matches.emplace_back(mod, mod->getVersion(0), mod->getVersion(0)->getType(0), i, 0, 0);
				}
				else {
					matches.emplace_back(mod, mod->getVersion(0), i, 0);
				}
			}
			else {
				matches.emplace_back(mod, i);
			}
		}
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["query"] = formattedQuery;
		properties["exactMatch"] = exactMatchFound;
		properties["numberOfMatches"] = matches.size();

		if(matches.size() == 1) {
			properties["match"] = matches[0].toString();
		}

		SegmentAnalytics::getInstance()->track("Mod Search", properties);
	}

	return matches;
}

std::vector<std::shared_ptr<ModIdentifier>> ModManager::searchForFavouriteMod(const std::vector<std::shared_ptr<ModIdentifier>> & favouriteMods, const std::string & query) {
	std::string formattedQuery(Utilities::toLowerCase(Utilities::trimString(query)));

	if(formattedQuery.empty()) {
		return {};
	}

	std::string favouriteModName;
	std::vector<std::shared_ptr<ModIdentifier>> matchingFavouriteMods;
	bool exactMatchFound = false;

	for(size_t i = 0; i < favouriteMods.size(); i++) {
		favouriteModName = Utilities::toLowerCase(favouriteMods[i]->toString());

		if(favouriteModName == formattedQuery) {
			exactMatchFound = true;
			matchingFavouriteMods.clear();
			matchingFavouriteMods.push_back(favouriteMods[i]);

			break;
		}

		if(favouriteModName.find(formattedQuery) != std::string::npos) {
			matchingFavouriteMods.push_back(favouriteMods[i]);
		}
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["query"] = formattedQuery;
		properties["exactMatch"] = exactMatchFound;
		properties["numberOfMatches"] = matchingFavouriteMods.size();

		SegmentAnalytics::getInstance()->track("Favourite Mod Search", properties);
	}

	return matchingFavouriteMods;
}

std::vector<std::shared_ptr<GameVersion>> ModManager::searchForGameVersion(const std::vector<std::shared_ptr<GameVersion>> & gameVersions, const std::string & query) {
	std::string formattedQuery(Utilities::toLowerCase(Utilities::trimString(query)));

	if(formattedQuery.empty()) {
		return {};
	}

	std::string gameVersionName;
	std::vector<std::shared_ptr<GameVersion>> matchingGameVersions;
	bool exactMatchFound = false;

	for(size_t i = 0; i < gameVersions.size(); i++) {
		gameVersionName = Utilities::toLowerCase(gameVersions[i]->getLongName());

		if(gameVersionName == formattedQuery) {
			exactMatchFound = true;
			matchingGameVersions.clear();
			matchingGameVersions.push_back(gameVersions[i]);

			break;
		}

		if(gameVersionName.find(formattedQuery) != std::string::npos) {
			matchingGameVersions.push_back(gameVersions[i]);

			continue;
		}

		gameVersionName = Utilities::toLowerCase(gameVersions[i]->getShortName());

		if(gameVersionName == formattedQuery) {
			exactMatchFound = true;
			matchingGameVersions.clear();
			matchingGameVersions.push_back(gameVersions[i]);

			break;
		}

		if(gameVersionName.find(formattedQuery) != std::string::npos) {
			matchingGameVersions.push_back(gameVersions[i]);

			continue;
		}
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["query"] = formattedQuery;
		properties["exactMatch"] = exactMatchFound;
		properties["numberOfMatches"] = matchingGameVersions.size();

		SegmentAnalytics::getInstance()->track("Game Version Search", properties);
	}

	return matchingGameVersions;
}

std::vector<std::shared_ptr<ModAuthorInformation>> ModManager::searchForAuthor(const std::vector<std::shared_ptr<ModAuthorInformation>> & authors, const std::string & query) {
	std::string formattedQuery(Utilities::toLowerCase(Utilities::trimString(query)));

	if(formattedQuery.empty()) {
		return {};
	}

	std::string authorName;
	std::vector<std::shared_ptr<ModAuthorInformation>> matchingAuthors;
	bool exactMatchFound = false;

	for(size_t i = 0; i < authors.size(); i++) {
		authorName = Utilities::toLowerCase(authors[i]->getName());

		if(authorName == formattedQuery) {
			exactMatchFound = true;
			matchingAuthors.clear();
			matchingAuthors.push_back(authors[i]);

			break;
		}

		if(authorName.find(formattedQuery) != std::string::npos) {
			matchingAuthors.push_back(authors[i]);
		}
	}

	if(SettingsManager::getInstance()->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["query"] = formattedQuery;
		properties["exactMatch"] = exactMatchFound;
		properties["numberOfMatches"] = matchingAuthors.size();

		SegmentAnalytics::getInstance()->track("Author Search", properties);
	}

	return matchingAuthors;
}

void ModManager::clearSelectedMod() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	bool selectedModChanged = m_selectedMod != nullptr;

	m_selectedMod = nullptr;
	m_selectedModVersionIndex = std::numeric_limits<size_t>::max();
	m_selectedModVersionTypeIndex = std::numeric_limits<size_t>::max();

	if(selectedModChanged) {
		notifyModSelectionChanged();
	}
}

bool ModManager::isModSupportedOnSelectedGameVersion() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_selectedMod == nullptr || m_selectedModVersionIndex == std::numeric_limits<size_t>::max() || m_selectedModVersionTypeIndex == std::numeric_limits<size_t>::max()) {
		return false;
	}

	std::shared_ptr<GameVersion> selectedGameVersion(getSelectedGameVersion());

	if(selectedGameVersion == nullptr) {
		return false;
	}

	std::shared_ptr<ModVersionType> selectedModVersionType(m_selectedMod->getVersion(m_selectedModVersionIndex)->getType(m_selectedModVersionTypeIndex));

	if(selectedModVersionType->isStandAlone()) {
		return false;
	}

	std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(selectedGameVersion->getCompatibleModGameVersions(selectedModVersionType->getGameVersions()));

	return !compatibleModGameVersions.empty();
}

bool ModManager::installStandAloneMod(const ModGameVersion & standAloneModGameVersion, const std::string & destinationDirectoryPath, bool removeArchivePackage) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!standAloneModGameVersion.isValid(true) || !standAloneModGameVersion.isStandAlone()) {
		spdlog::error("Cannot install invalid stand-alone mod game version.");
		return false;
	}

	if(m_standAloneMods->hasStandAloneMod(standAloneModGameVersion)) {
		spdlog::error("Stand-alone '{}' mod already installed.", standAloneModGameVersion.getParentModVersion()->getFullName());
		return false;
	}

	std::shared_ptr<ModDownload> modDownload(standAloneModGameVersion.getDownload());

	if(modDownload == nullptr) {
		spdlog::error("Stand-alone '{}' mod has no corresponding download.", standAloneModGameVersion.getParentModVersion()->getFullName());
		return false;
	}

	if(!m_localMode && !m_downloadManager->downloadModGameVersion(standAloneModGameVersion, *m_mods, *getGameVersions())) {
		spdlog::error("Failed to download stand-alone '{}' mod!", standAloneModGameVersion.getParentModVersion()->getFullName());
		return false;
	}

	SettingsManager * settings = SettingsManager::getInstance();
	std::string standAloneModArchiveFilePath;

	if(m_localMode) {
		if(settings->modPackageDownloadsDirectoryPath.empty()) {
			spdlog::error("Local mod package downloads directory path setting is not set.");
			return false;
		}

		standAloneModArchiveFilePath = settings->modPackageDownloadsDirectoryPath;
	}
	else {
		standAloneModArchiveFilePath = getModsDirectoryPath();
	}

	standAloneModArchiveFilePath = Utilities::joinPaths(standAloneModArchiveFilePath, GameVersion::STANDALONE_DIRECTORY_NAME, modDownload->getFileName());

	if(!std::filesystem::is_regular_file(std::filesystem::path(standAloneModArchiveFilePath))) {
		spdlog::error("Stand-alone '{}' mod download archive package file does not exist at path: '{}'.", standAloneModGameVersion.getParentModVersion()->getFullName(), standAloneModArchiveFilePath);
		return false;
	}

	std::unique_ptr<Archive> standAloneModArchive(ArchiveFactoryRegistry::getInstance()->readArchiveFrom(standAloneModArchiveFilePath));

	if(standAloneModArchive == nullptr) {
		spdlog::error("Failed to load '{}' mod download archive package file: '{}'.", standAloneModGameVersion.getParentModVersion()->getFullName(), standAloneModArchiveFilePath);
		return false;
	}

	size_t numberOfFiles = standAloneModArchive->numberOfFiles();
	std::shared_ptr<StandAloneMod> standAloneMod(std::make_shared<StandAloneMod>(standAloneModGameVersion, destinationDirectoryPath));

	if(!m_standAloneMods->addStandAloneMod(standAloneMod)) {
		spdlog::error("Failed to add installed stand-alone '{}' mod to collection.");
		return false;
	}

	if(standAloneModArchive->extractAllEntries(destinationDirectoryPath, true) == 0) {
		spdlog::error("Failed to extract stand-alone '{}' mod files archive package to '{}'.", standAloneModGameVersion.getParentModVersion()->getFullName(), destinationDirectoryPath);
		m_standAloneMods->removeStandAloneMod(standAloneModGameVersion);
		return false;
	}

	standAloneModArchive.reset();

	if(m_standAloneMods->saveTo(settings->standAloneModsListFilePath)) {
		spdlog::info("Installed stand-alone mod list saved to file: '{}'.", settings->standAloneModsListFilePath);
	}
	else {
		spdlog::error("Failed to save Installed stand-alone mod list to file: '{}'.", settings->standAloneModsListFilePath);
	}

	if(removeArchivePackage && !m_localMode) {
		spdlog::info("Deleting downloaded '{}' mod archive package file: '{}'...", standAloneModGameVersion.getParentModVersion()->getFullName(), standAloneModArchiveFilePath);

		std::error_code errorCode;
		std::filesystem::remove(std::filesystem::path(standAloneModArchiveFilePath), errorCode);

		if(errorCode) {
			spdlog::error("Failed to delete temporary stand-alone '{}' mod archive package file '{}' with error: {}", standAloneModGameVersion.getParentModVersion()->getFullName(), standAloneModArchiveFilePath, errorCode.message());
		}
		else {
			spdlog::info("Removing stand-alone mod '{}' archive package entry from cached download list.", standAloneModGameVersion.getParentModVersion()->getFullName());

			m_downloadManager->getDownloadCache()->removeCachedPackageFile(*modDownload);
			m_downloadManager->saveDownloadCache();
		}
	}

	if(settings->segmentAnalyticsEnabled) {
		std::map<std::string, std::any> properties;
		properties["modID"] = standAloneMod->getID();
		properties["modName"] = standAloneMod->getLongName();
		properties["modBaseGame"] = standAloneMod->getBase();
		properties["version"] = standAloneMod->getVersion();
		properties["gameExecutableName"] = standAloneMod->getGameExecutableName();

		if(standAloneMod->hasSetupExecutableName()) {
			properties["setupExecutableName"] = standAloneMod->getSetupExecutableName();
		}

		properties["archiveFileName"] = modDownload->getFileName();
		properties["archiveFileSize"] = modDownload->getFileSize();
		properties["sha1"] = modDownload->getSHA1();
		properties["numberOfFiles"] = numberOfFiles;

		SegmentAnalytics::getInstance()->track("Stand-Alone Mod Installed", properties);
	}

	return true;
}

bool ModManager::uninstallModGameVersion(const ModGameVersion & modGameVersion) {
	if(!modGameVersion.isValid(true)) {
		return false;
	}

	SettingsManager * settings = SettingsManager::getInstance();

	if(!m_localMode) {
		std::shared_ptr<DownloadManager> downloadManager(m_downloadManager);

		if(downloadManager->isModGameVersionDownloaded(modGameVersion)) {
			std::shared_ptr<CachedPackageFile> cachedModPackageFile(m_downloadManager->getDownloadCache()->getCachedPackageFile(*modGameVersion.getDownload()));

			if(downloadManager->uninstallModGameVersion(modGameVersion, *getGameVersions())) {
				if(!modGameVersion.isStandAlone()) {
					if(settings->segmentAnalyticsEnabled) {
						std::map<std::string, std::any> properties;
						properties["fullModName"] = modGameVersion.getParentModVersionType()->getFullName();
						properties["modID"] = modGameVersion.getParentMod()->getID();
						properties["modName"] = modGameVersion.getParentMod()->getName();
						properties["modVersion"] = modGameVersion.getParentModVersion()->getVersion();
						properties["modVersionType"] = modGameVersion.getParentModVersionType()->getType();
						properties["modGameVersionID"] = modGameVersion.getGameVersionID();

						if(cachedModPackageFile != nullptr) {
							properties["packageFileName"] = cachedModPackageFile->getFileName();
							properties["packageFileSize"] = cachedModPackageFile->getFileSize();
							properties["sha1"] =  cachedModPackageFile->getSHA1();

							if(cachedModPackageFile->hasETag()) {
								properties["eTag"] = cachedModPackageFile->getETag();
							}

							if(cachedModPackageFile->hasDownloadedTimePoint()) {
								properties["downloadedAt"] = Utilities::timePointToString(cachedModPackageFile->getDownloadedTimePoint().value(), Utilities::TimeFormat::ISO8601);
							}
						}

						SegmentAnalytics::getInstance()->track("Mod Uninstalled", properties);
					}
				}
			}
			else {
				spdlog::warn("Failed to uninstall '{}' mod files.", modGameVersion.getFullName(true));
			}
		}
	}

	if(modGameVersion.isStandAlone()) {
		std::shared_ptr<StandAloneMod> standAloneMod(m_standAloneMods->getStandAloneMod(modGameVersion));

		if(standAloneMod != nullptr && standAloneMod->isConfigured()) {
			spdlog::info("Deleting stand-alone '{}' mod game directory: '{}'...", modGameVersion.getParentModVersion()->getFullName(), standAloneMod->getGamePath());

			if(std::filesystem::is_directory(std::filesystem::path(standAloneMod->getGamePath()))) {
				std::error_code errorCode;
				std::filesystem::remove_all(std::filesystem::path(standAloneMod->getGamePath()), errorCode);

				if(errorCode) {
					spdlog::error("Failed to delete stand-alone '{}' mod game directory '{}' with error: {}", modGameVersion.getParentModVersion()->getFullName(), standAloneMod->getGamePath(), errorCode.message());
				}
			}
			else {
				spdlog::warn("Stand-alone '{}' mod game directory '{}' no longer exists.", modGameVersion.getParentModVersion()->getFullName(), standAloneMod->getGamePath());
			}

			if(m_standAloneMods->removeStandAloneMod(*standAloneMod)) {
				spdlog::info("Removing installed stand-alone '{}' mod application configuration entry.", modGameVersion.getParentModVersion()->getFullName());
			}

			if(m_standAloneMods->saveTo(settings->standAloneModsListFilePath)) {
				spdlog::info("Installed stand-alone mod list saved to file: '{}'.", settings->standAloneModsListFilePath);
			}
			else {
				spdlog::error("Failed to save Installed stand-alone mod list to file: '{}'.", settings->standAloneModsListFilePath);
			}
		}

		if(settings->segmentAnalyticsEnabled) {
			std::map<std::string, std::any> properties;
			properties["modID"] = standAloneMod->getID();
			properties["modName"] = standAloneMod->getLongName();
			properties["version"] = standAloneMod->getVersion();

			if(standAloneMod->hasInstalledTimePoint()) {
				properties["installedAt"] = Utilities::timePointToString(standAloneMod->getInstalledTimePoint().value(), Utilities::TimeFormat::ISO8601);
			}

			if(standAloneMod->hasLastPlayedTimePoint()) {
				properties["lastPlayedAt"] = Utilities::timePointToString(standAloneMod->getLastPlayedTimePoint().value(), Utilities::TimeFormat::ISO8601);
			}

			SegmentAnalytics::getInstance()->track("Stand-Alone Mod Uninstalled", properties);
		}
	}

	return true;
}

bool ModManager::shouldRunSelectedMod() const {
	return m_shouldRunSelectedMod;
}

std::future<bool> ModManager::runSelectedModAsync(std::shared_ptr<GameVersion> alternateGameVersion, std::shared_ptr<ModGameVersion> alternateModGameVersion) {
	return std::async(std::launch::async, &ModManager::runSelectedMod, std::ref(*this), alternateGameVersion, alternateModGameVersion);
}

bool ModManager::runSelectedMod(std::shared_ptr<GameVersion> alternateGameVersion, std::shared_ptr<ModGameVersion> alternateModGameVersion) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::chrono::time_point<std::chrono::steady_clock> runStartTimePoint(std::chrono::steady_clock::now());

	if(!m_initialized) {
		notifyLaunchError("Mod manager not initialized");
		return false;
	}
	else if(m_gameProcess != nullptr) {
		notifyLaunchError("Game process already running.");
		return false;
	}

	m_shouldRunSelectedMod = false;

	SettingsManager * settings = SettingsManager::getInstance();

	std::shared_ptr<GameVersion> selectedGameVersion(getSelectedGameVersion());

	if(alternateGameVersion != nullptr) {
		if(selectedGameVersion != nullptr && selectedGameVersion->isStandAlone()) {
			spdlog::warn("Ignoring alternate game version provided to run selected mod command since selected game version is stand-alone.");
		}
		else {
			selectedGameVersion = alternateGameVersion;
		}
	}

	if(selectedGameVersion == nullptr) {
		notifyLaunchError("No game version selected.");
		return false;
	}

	if(!selectedGameVersion->isConfigured()) {
		notifyLaunchError(fmt::format("Game version '{}' is not configured.", selectedGameVersion->getLongName()));
		return false;
	}

	std::shared_ptr<DOSBoxVersion> selectedDOSBoxVersion(getSelectedDOSBoxVersion());

	if(selectedGameVersion->doesRequireDOSBox() && !selectedDOSBoxVersion->isConfigured()) {
		notifyLaunchError(fmt::format("Selected DOSBox version '{}' is not configured.", selectedDOSBoxVersion->getLongName()));
		return false;
	}

	bool standAlone = selectedGameVersion->isStandAlone();
	std::shared_ptr<ModVersion> selectedModVersion;
	std::shared_ptr<ModVersionType> selectedModVersionType;
	std::shared_ptr<ModGameVersion> selectedModGameVersion;

	if(m_selectedMod != nullptr) {
		if(checkModForMissingFiles(*m_selectedMod) != 0) {
			notifyLaunchError("Mod is missing files, aborting execution.");
			return false;
		}

		if(m_selectedModVersionIndex == std::numeric_limits<size_t>::max()) {
			if(m_selectedModVersionIndex == std::numeric_limits<size_t>::max()) {
				notifyLaunchError("No mod version selected.");
				return false;
			}
		}

		selectedModVersion = m_selectedMod->getVersion(m_selectedModVersionIndex);

		if(m_selectedModVersionTypeIndex == std::numeric_limits<size_t>::max()) {
			if(m_selectedModVersionTypeIndex == std::numeric_limits<size_t>::max()) {
				notifyLaunchError("No mod version type selected.");
				return false;
			}
		}

		selectedModVersionType = selectedModVersion->getType(m_selectedModVersionTypeIndex);

		if(m_selectedModGameVersionIndex != std::numeric_limits<size_t>::max()) {
			selectedModGameVersion = selectedModVersionType->getGameVersion(m_selectedModGameVersionIndex);
		}

		if(selectedModGameVersion != nullptr && selectedModGameVersion->isStandAlone()) {
			if(!standAlone) {
				notifyLaunchError("Selected game version is stand-alone, but selected mod game version is not.");
				return false;
			}

			if(alternateModGameVersion != nullptr) {
				spdlog::warn("Ignoring alternate mod game version provided to run selected mod command since selected mod game version is stand-alone.");
			}
		}
		else {
			std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(selectedGameVersion->getCompatibleModGameVersions(selectedModVersionType->getGameVersions()));

			if(compatibleModGameVersions.empty()) {
				notifyLaunchError(fmt::format("{} is not supported on {}.", m_selectedMod->getFullName(m_selectedModVersionIndex, m_selectedModVersionTypeIndex), selectedGameVersion->getLongName()));
				return false;
			}

			if(alternateModGameVersion != nullptr) {
				if(alternateModGameVersion->getParentModVersionType() != selectedModVersionType.get()) {
					notifyLaunchError(fmt::format("Provided alternate game version does not belong to '{}'.", m_selectedMod->getFullName(m_selectedModVersionIndex, m_selectedModVersionTypeIndex)));
					return false;
				}

				selectedModGameVersion = alternateModGameVersion;
			}
			else {
				if(m_selectedModGameVersionIndex != std::numeric_limits<size_t>::max()) {
					selectedModGameVersion = selectedModVersionType->getGameVersion(m_selectedModGameVersionIndex);
				}
				else {
					spdlog::warn("No mod game version selected, auto-selecting default version.");

					selectedModGameVersion = compatibleModGameVersions.back();
				}
			}
		}
	}

	std::vector<std::string> customConFilePaths;
	std::vector<std::string> customDefFilePaths;
	std::vector<std::string> customGroupFilePaths;
	std::vector<std::shared_ptr<ModFile>> modConFiles;
	std::vector<std::shared_ptr<ModFile>> modDefFiles;
	std::vector<std::shared_ptr<ModFile>> modGroupFiles;
	std::vector<std::shared_ptr<ModFile>> modDependencyGroupFiles;
	std::vector<std::string> relativeModConFilePaths;
	std::vector<std::string> relativeModDefFilePaths;
	std::vector<std::string> relativeModGroupFilePaths;
	std::vector<std::string> relativeModDependencyGroupFilePaths;
	std::vector<std::string> absoluteModConFilePaths;
	std::vector<std::string> absoluteModDefFilePaths;
	std::vector<std::string> absoluteModGroupFilePaths;
	std::vector<std::string> absoluteModDependencyGroupFilePaths;
	std::vector<std::string> allRelativeConFilePaths;
	std::vector<std::string> allRelativeDefFilePaths;
	std::vector<std::string> allRelativeGroupFilePaths;
	std::vector<std::string> allAbsoluteConFilePaths;
	std::vector<std::string> allAbsoluteDefFilePaths;
	std::vector<std::string> allAbsoluteGroupFilePaths;
	std::string relativeCustomFilesBaseDirectoryPath;
	std::string relativeModFilesBaseDirectoryPath;
	std::string absoluteModFilesBaseDirectoryPath;
	bool customMod = false;

	if(m_arguments != nullptr) {
		customConFilePaths = m_arguments->getValues("x", "con");
		customDefFilePaths = m_arguments->getValues("h", "def");
		customGroupFilePaths = m_arguments->getValues("g", "group");

		customMod = !customConFilePaths.empty() ||
					!customDefFilePaths.empty() ||
					!customGroupFilePaths.empty();
	}

	if(!standAlone && selectedModGameVersion != nullptr) {
		std::shared_ptr<GameVersion> targetGameVersion(getGameVersions()->getGameVersionWithID(selectedModGameVersion->getGameVersionID()));
		const std::string & modDirectoryName(targetGameVersion->getModDirectoryName());
		absoluteModFilesBaseDirectoryPath = Utilities::joinPaths(getModsDirectoryPath(), modDirectoryName);

		if(selectedGameVersion->doesSupportSubdirectories()) {
			if(Utilities::areSymlinksSupported()) {
				relativeModFilesBaseDirectoryPath = Utilities::joinPaths(settings->modsSymlinkName, modDirectoryName);
			}
			else {
				relativeModFilesBaseDirectoryPath = settings->gameTempDirectoryName;
			}
		}

		modDependencyGroupFiles = m_mods->getModDependencyGroupFiles(*selectedModGameVersion, *targetGameVersion, getGameVersions().get(), true, true);

		modConFiles = selectedModGameVersion->getFilesOfType("con");
		modDefFiles = selectedModGameVersion->getFilesOfType("def");

		if(selectedGameVersion->areZipArchiveGroupsSupported()) {
			modGroupFiles = selectedModGameVersion->getFilesOfType("grp", "zip");
		}
		else {
			modGroupFiles = selectedModGameVersion->getFilesOfType("grp");
		}

		for(const std::shared_ptr<ModFile> & modConFile : modConFiles) {
			if(selectedGameVersion->areScriptFilesReadFromGroup()) {
				relativeModConFilePaths.push_back(modConFile->getFileName());
			}
			else {
				absoluteModConFilePaths.push_back(Utilities::joinPaths(absoluteModFilesBaseDirectoryPath, modConFile->getFileName()));
				relativeModConFilePaths.push_back(Utilities::joinPaths(relativeModFilesBaseDirectoryPath, modConFile->getFileName()));
			}
		}

		for(const std::shared_ptr<ModFile> & modDefFile : modDefFiles) {
			if(selectedGameVersion->areScriptFilesReadFromGroup()) {
				relativeModDefFilePaths.push_back(modDefFile->getFileName());
			}
			else {
				absoluteModDefFilePaths.push_back(Utilities::joinPaths(absoluteModFilesBaseDirectoryPath, modDefFile->getFileName()));
				relativeModDefFilePaths.push_back(Utilities::joinPaths(relativeModFilesBaseDirectoryPath, modDefFile->getFileName()));
			}
		}

		for(const std::shared_ptr<ModFile> & modGroupFile : modGroupFiles) {
			relativeModGroupFilePaths.push_back(Utilities::joinPaths(relativeModFilesBaseDirectoryPath, modGroupFile->getFileName()));
			absoluteModGroupFilePaths.push_back(Utilities::joinPaths(absoluteModFilesBaseDirectoryPath, modGroupFile->getFileName()));
		}

		for(const std::shared_ptr<ModFile> & modDependencyGroupFile : modDependencyGroupFiles) {
			std::shared_ptr<GameVersion> modDependencyGameVersion(getGameVersions()->getGameVersionWithID(modDependencyGroupFile->getParentModGameVersion()->getGameVersionID()));
			const std::string & modDependencyDirectoryName(modDependencyGameVersion->getModDirectoryName());
			std::string absoluteModDependencyFilesBaseDirectoryPath(Utilities::joinPaths(getModsDirectoryPath(), modDependencyDirectoryName));
			std::string relativeModDependencyFilesBaseDirectoryPath;

			if(selectedGameVersion->doesSupportSubdirectories()) {
				if(Utilities::areSymlinksSupported()) {
					relativeModDependencyFilesBaseDirectoryPath = Utilities::joinPaths(settings->modsSymlinkName, modDependencyDirectoryName);
				}
				else {
					relativeModDependencyFilesBaseDirectoryPath = settings->gameTempDirectoryName;
				}
			}

			relativeModDependencyGroupFilePaths.push_back(Utilities::joinPaths(relativeModDependencyFilesBaseDirectoryPath, modDependencyGroupFile->getFileName()));
			absoluteModDependencyGroupFilePaths.push_back(Utilities::joinPaths(absoluteModDependencyFilesBaseDirectoryPath, modDependencyGroupFile->getFileName()));
		}

		allRelativeConFilePaths = relativeModConFilePaths;
		allRelativeDefFilePaths = relativeModDefFilePaths;
		allRelativeGroupFilePaths = relativeModDependencyGroupFilePaths;
		allAbsoluteConFilePaths = absoluteModConFilePaths;
		allAbsoluteDefFilePaths = absoluteModDefFilePaths;
		allAbsoluteGroupFilePaths = absoluteModDependencyGroupFilePaths;

		for(const std::string & relativeModGroupFilePath : relativeModGroupFilePaths) {
			allRelativeGroupFilePaths.push_back(relativeModGroupFilePath);
		}

		for(const std::string & absoluteModGroupFilePath : absoluteModGroupFilePaths) {
			allAbsoluteGroupFilePaths.push_back(absoluteModGroupFilePath);
		}
	}

	if(selectedGameVersion->doesSupportSubdirectories()) {
		if(Utilities::areSymlinksSupported()) {
			relativeCustomFilesBaseDirectoryPath = settings->appSymlinkName;
		}
		else {
			relativeCustomFilesBaseDirectoryPath = settings->gameTempDirectoryName;
		}
	}

	if(!customConFilePaths.empty()) {
		allRelativeConFilePaths.clear();

		for(const std::string & customConFilePath : customConFilePaths) {
			if(relativeCustomFilesBaseDirectoryPath.empty()) {
				allRelativeConFilePaths.push_back(std::string(Utilities::getFileName(customConFilePath)));
			}
			else {
				allRelativeConFilePaths.push_back(Utilities::joinPaths(relativeCustomFilesBaseDirectoryPath, customConFilePath));
			}

			allAbsoluteConFilePaths.push_back(customConFilePath);
		}
	}

	if(!customDefFilePaths.empty()) {
		allRelativeDefFilePaths.clear();

		for(const std::string & customDefFilePath : customDefFilePaths) {
			if(relativeCustomFilesBaseDirectoryPath.empty()) {
				allRelativeDefFilePaths.push_back(std::string(Utilities::getFileName(customDefFilePath)));
			}
			else {
				allRelativeDefFilePaths.push_back(Utilities::joinPaths(relativeCustomFilesBaseDirectoryPath, customDefFilePath));
			}

			allAbsoluteDefFilePaths.push_back(customDefFilePath);
		}
	}

	for(const std::string & customGroupFilePath : customGroupFilePaths) {
		if(relativeCustomFilesBaseDirectoryPath.empty()) {
			allRelativeGroupFilePaths.push_back(std::string(Utilities::getFileName(customGroupFilePath)));
		}
		else {
			allRelativeGroupFilePaths.push_back(Utilities::joinPaths(relativeCustomFilesBaseDirectoryPath, customGroupFilePath));
		}

		allAbsoluteGroupFilePaths.push_back(customGroupFilePath);
	}

	bool doesRequireCombinedGroup = !standAlone && !selectedGameVersion->doesRequireGroupFileExtraction() && (selectedGameVersion->doesRequireOriginalGameFiles() || selectedGameVersion->doesGroupCountExceedMaximumGroupFilesLimit(modDependencyGroupFiles.size() + modGroupFiles.size() + customGroupFilePaths.size()));
	bool doesRequireCombinedZip = doesRequireCombinedGroup && (((selectedModGameVersion != nullptr && selectedModGameVersion->hasFileOfType("zip")) || std::find_if(modDependencyGroupFiles.cbegin(), modDependencyGroupFiles.cend(), [](const std::shared_ptr<ModFile> & dependencyModFile) { return Utilities::areStringsEqualIgnoreCase(dependencyModFile->getType(), "zip"); }) != modDependencyGroupFiles.cend() || std::find_if(customGroupFilePaths.cbegin(), customGroupFilePaths.cend(), [](const std::string & groupFile) { return Utilities::hasFileExtension(groupFile, "zip"); }) != customGroupFilePaths.cend()));
	bool shouldSymlinkToCombinedGroup = doesRequireCombinedGroup && Utilities::areSymlinksSupported() && selectedGameVersion->doesSupportSubdirectories();
	bool shouldConfigureApplicationTemporaryDirectory = shouldSymlinkToCombinedGroup || selectedGameVersion->doesRequireDOSBox();
	bool shouldConfigureGameTemporaryDirectory = !Utilities::areSymlinksSupported() && selectedGameVersion->doesSupportSubdirectories();

	std::string combinedGroupFileName;
	std::string absoluteCombinedGroupFilePath;
	std::string relativeCombinedGroupFilePath;
	std::unique_ptr<Group> combinedGroup;
	std::unique_ptr<ZipArchive> combinedZip;

	struct FileNameComparator {
	public:
		bool operator () (const std::string & fileNameA, const std::string & fileNameB) const {
			return std::lexicographical_compare(fileNameA.begin(), fileNameA.end(), fileNameB.begin(), fileNameB.end(), [](unsigned char a, unsigned char b) {
				return std::toupper(a) < std::toupper(b);
			});
		}
	};

	std::map<std::string, std::unique_ptr<ByteBuffer>, FileNameComparator> demoFiles;

	if(doesRequireCombinedGroup || (!m_demoRecordingEnabled && settings->demoExtractionEnabled)) {
		if(doesRequireCombinedGroup) {
			if(doesRequireCombinedZip) {
				if(!selectedGameVersion->areZipArchiveGroupsSupported()) {
					notifyLaunchError(fmt::format("Zip archive group files are not supported by '{}'.", selectedGameVersion->getLongName()));

					return false;
				}

				combinedGroupFileName = fmt::format("{}-Combined.zip", Utilities::getFileNameNoExtension(modGroupFiles.back()->getFileName()));

				spdlog::info("Generating combined zip archive file '{}'...", combinedGroupFileName);
			}
			else {
				combinedGroupFileName = Utilities::replaceFileExtension(modGroupFiles.back()->getFileName(), "CMB");

				spdlog::info("Generating combined group file '{}'...", combinedGroupFileName);
			}

			if(shouldSymlinkToCombinedGroup) {
				absoluteCombinedGroupFilePath = Utilities::joinPaths(settings->appTempDirectoryPath, combinedGroupFileName);
				relativeCombinedGroupFilePath = Utilities::joinPaths(settings->tempSymlinkName, combinedGroupFileName);
			}
			else if(shouldConfigureGameTemporaryDirectory) {
				absoluteCombinedGroupFilePath = Utilities::joinPaths(selectedGameVersion->getGamePath(), settings->gameTempDirectoryName, combinedGroupFileName);
				relativeCombinedGroupFilePath = Utilities::joinPaths(settings->gameTempDirectoryName, combinedGroupFileName);
			}
			else {
				absoluteCombinedGroupFilePath = Utilities::joinPaths(selectedGameVersion->getGamePath(), combinedGroupFileName);
				relativeCombinedGroupFilePath = combinedGroupFileName;
			}

			std::shared_ptr<GameVersion> dukeNukemGroupGameVersion;
			std::unique_ptr<GroupGRP> dukeNukemGroup;

			if(selectedGameVersion->doesRequireOriginalGameFiles()) {
				dukeNukemGroupGameVersion = m_gameManager->getGroupGameVersion(selectedGameVersion->getID());
				std::string dukeNukemGroupPath(m_gameManager->getGroupFilePath(selectedGameVersion->getID()));

				if(dukeNukemGroupGameVersion == nullptr || dukeNukemGroupPath.empty()) {
					return false;
				}

				dukeNukemGroup = GroupGRP::loadFrom(dukeNukemGroupPath);

				if(dukeNukemGroup == nullptr) {
					notifyLaunchError(fmt::format("Failed to load '{}' group for creation of combined group from file path: '{}'.", dukeNukemGroupGameVersion->getLongName(), dukeNukemGroupPath));
					return false;
				}
			}

			if(doesRequireCombinedZip) {
				combinedZip = ZipArchive::createNew(absoluteCombinedGroupFilePath);

				if(selectedGameVersion->doesRequireOriginalGameFiles()) {
					std::shared_ptr<GroupFile> groupFile;

					for(size_t i = 0; i < dukeNukemGroup->numberOfFiles(); i++) {
						groupFile = dukeNukemGroup->getFile(i);
						combinedZip->addData(groupFile->transferData(), groupFile->getFileName(), true);
					}

					spdlog::info("Added {} original '{}' game file{} to combined zip archive file.", dukeNukemGroup->numberOfFiles(), dukeNukemGroup->numberOfFiles() == 1 ? "" : "s", dukeNukemGroupGameVersion->getLongName());

					dukeNukemGroup.reset();
				}
			}
			else {
				if(selectedGameVersion->doesRequireOriginalGameFiles()) {
					combinedGroup = std::move(dukeNukemGroup);

					spdlog::info("Added {} original '{}' game file{} to combined group file.", combinedGroup->numberOfFiles(), combinedGroup->numberOfFiles() == 1 ? "" : "s", dukeNukemGroupGameVersion->getLongName());
				}
				else {
					combinedGroup = std::make_unique<GroupGRP>();
				}

				combinedGroup->setFilePath(absoluteCombinedGroupFilePath);
			}
		}

		for(const std::string & absoluteGroupFilePath : allAbsoluteGroupFilePaths) {
			if(Utilities::hasFileExtension(absoluteGroupFilePath, "zip")) {
				std::unique_ptr<ZipArchive> modZip(ZipArchive::readFrom(absoluteGroupFilePath));

				if(modZip == nullptr) {
					notifyLaunchError(fmt::format("Failed to load zip archive from file path: '{}'.", absoluteGroupFilePath));

					return false;
				}

				size_t addedFileCount = 0;
				std::shared_ptr<ArchiveEntry> modZipEntry;

				for(size_t i = 0; i < modZip->numberOfEntries(); i++) {
					modZipEntry = modZip->getEntry(i);

					if(modZipEntry == nullptr || !modZipEntry->isFile()) {
						continue;
					}

					std::unique_ptr<ByteBuffer> modZipEntryData(modZipEntry->getData());

					if(settings->demoExtractionEnabled && Utilities::hasFileExtension(modZipEntry->getPath(), "dmo")) {
						demoFiles.emplace(modZipEntry->getPath(), std::make_unique<ByteBuffer>(*modZipEntryData));
					}

					if(combinedZip != nullptr) {
						combinedZip->addData(std::move(modZipEntryData), modZipEntry->getPath(), true);
						addedFileCount++;
					}
				}

				spdlog::info("Added {} file{} from '{}' to combined zip archive file.", addedFileCount, addedFileCount == 1 ? "" : "s", Utilities::getFileName(absoluteGroupFilePath));
			}
			else {
				std::unique_ptr<Group> modGroup(GroupGRP::loadFrom(absoluteGroupFilePath));

				if(modGroup == nullptr) {
					notifyLaunchError(fmt::format("Failed to load group from file path: '{}'.", absoluteGroupFilePath));

					return false;
				}

				std::shared_ptr<GroupFile> groupFile;

				for(size_t i = 0; i < modGroup->numberOfFiles(); i++) {
					groupFile = modGroup->getFile(i);

					if(settings->demoExtractionEnabled && Utilities::hasFileExtension(groupFile->getFileName(), "dmo")) {
						demoFiles.emplace(groupFile->getFileName(), std::make_unique<ByteBuffer>(groupFile->getData()));
					}

					if(combinedZip != nullptr) {
						combinedZip->addData(groupFile->transferData(), groupFile->getFileName(), true);
					}
					else if(combinedGroup != nullptr) {
						combinedGroup->addFile(std::make_unique<GroupFile>(groupFile->getFileName(), groupFile->transferData()), true);
					}
				}

				spdlog::info("Added {} file{} from '{}' to combined {} file.", modGroup->numberOfFiles(), modGroup->numberOfFiles() == 1 ? "" : "s", Utilities::getFileName(absoluteGroupFilePath), combinedZip != nullptr ? "zip archive" : "group");
			}
		}
	}

	std::string customMap;
	std::string relativeCustomMapFilePath;
	std::string absoluteCustomMapFilePath;

	if(m_arguments != nullptr) {
		customMap = m_arguments->getFirstValue("map");

		if(!customMap.empty()) {
			std::string relativeCustomApplicationBaseDirectoryPath;
			std::string relativeCustomMapsBaseDirectoryPath;

			if(selectedGameVersion->doesSupportSubdirectories()) {
				if(Utilities::areSymlinksSupported()) {
					relativeCustomApplicationBaseDirectoryPath = settings->appSymlinkName;
					relativeCustomMapsBaseDirectoryPath = settings->mapsSymlinkName;
				}
				else {
					relativeCustomApplicationBaseDirectoryPath = settings->gameTempDirectoryName;
					relativeCustomMapsBaseDirectoryPath = settings->gameTempDirectoryName;
				}
			}

			if(std::filesystem::is_regular_file(std::filesystem::path(customMap))) {
				relativeCustomMapFilePath = Utilities::joinPaths(relativeCustomApplicationBaseDirectoryPath, customMap);
				absoluteCustomMapFilePath = customMap;
			}
			else {
				std::string mapsDirectoryPath(getMapsDirectoryPath());

				if(std::filesystem::is_regular_file(Utilities::joinPaths(mapsDirectoryPath, customMap))) {
					relativeCustomMapFilePath = Utilities::joinPaths(relativeCustomMapsBaseDirectoryPath, customMap);
					absoluteCustomMapFilePath = Utilities::joinPaths(mapsDirectoryPath, customMap);
				}
				else {
					spdlog::error("Map '{}' does not exist in application or maps directories.", customMap);
				}
			}
		}
	}

	std::string temporaryDirectoryName;

	if(shouldConfigureGameTemporaryDirectory) {
		temporaryDirectoryName = settings->gameTempDirectoryName;
	}
	else if(shouldConfigureApplicationTemporaryDirectory) {
		temporaryDirectoryName = settings->tempSymlinkName;
	}

	if(!m_localMode && !standAlone && selectedModGameVersion != nullptr && !m_downloadManager->downloadModGameVersion(*selectedModGameVersion, *m_mods, *getGameVersions(), true, true)) {
		notifyLaunchError(fmt::format("Failed to download '{}' game version of '{}' mod!", getGameVersions()->getLongNameOfGameVersionWithID(selectedModGameVersion->getGameVersionID()), selectedModGameVersion->getFullName(false)));
		return false;
	}

	ScriptArguments scriptArgs;

	if(m_arguments != nullptr && m_arguments->hasPassthroughArguments()) {
		scriptArgs.addArgument("ARGUMENTS", m_arguments->getPassthroughArguments().value());
	}

	scriptArgs.addArgument("GAMEPATH", selectedGameVersion->getGamePath());
	scriptArgs.addArgument("DUKE3D", selectedGameVersion->getGameExecutableName());

	if(selectedGameVersion->hasSetupExecutableName()) {
		scriptArgs.addArgument("SETUP", selectedGameVersion->getSetupExecutableName().value());
	}

	if(selectedGameVersion->hasGroupFileArgumentFlag()) {
		scriptArgs.addArgument("GROUPFLAG", selectedGameVersion->getGroupFileArgumentFlag().value());
	}

	if(selectedGameVersion->hasConFileArgumentFlag()) {
		scriptArgs.addArgument("CONFLAG", selectedGameVersion->getConFileArgumentFlag().value());
	}

	if(selectedGameVersion->hasMapFileArgumentFlag()) {
		scriptArgs.addArgument("MAPFLAG", selectedGameVersion->getMapFileArgumentFlag().value());
	}

	if(selectedGameVersion->hasDefFileArgumentFlag()) {
		scriptArgs.addArgument("DEFFLAG", selectedGameVersion->getDefFileArgumentFlag().value());
	}

	if(shouldConfigureGameTemporaryDirectory) {
		scriptArgs.addArgument("MAPSDIR", settings->gameTempDirectoryName);
		scriptArgs.addArgument("MODSDIR", settings->gameTempDirectoryName);
	}
	else if(shouldConfigureApplicationTemporaryDirectory) {
		scriptArgs.addArgument("GAMEDIR", settings->gameSymlinkName);
		scriptArgs.addArgument("MAPSDIR", settings->mapsSymlinkName);
		scriptArgs.addArgument("MODSDIR", settings->modsSymlinkName);
	}

	if(!temporaryDirectoryName.empty()) {
		scriptArgs.addArgument("TEMPDIR", temporaryDirectoryName);
	}

	scriptArgs.addArgument("MAP", relativeCustomMapFilePath);

	if(selectedModGameVersion != nullptr && !standAlone) {
		for(const std::string & relativeConFilePath : allRelativeConFilePaths) {
			scriptArgs.addArgument("CON", relativeConFilePath);
		}

		if(doesRequireCombinedGroup) {
			scriptArgs.addArgument("COMBINEDGROUP", relativeCombinedGroupFilePath);
		}
		else {
			for(const std::string & relativeGroupFilePath : allRelativeGroupFilePaths) {
				if(!selectedGameVersion->areZipArchiveGroupsSupported() && Utilities::hasFileExtension(relativeGroupFilePath, "zip")) {
					continue;
				}

				scriptArgs.addArgument("GROUP", relativeGroupFilePath);
			}
		}

		for(const std::string & relativeDefFilePath : allRelativeDefFilePaths) {
			scriptArgs.addArgument("DEF", relativeDefFilePath);
		}
	}

	if(m_gameType == GameType::Client) {
		scriptArgs.addArgument("IP", settings->dosboxServerIPAddress);
		scriptArgs.addArgument("PORT", std::to_string(settings->dosboxRemoteServerPort));
	}
	else if(m_gameType == GameType::Server) {
		scriptArgs.addArgument("PORT", std::to_string(settings->dosboxLocalServerPort));
	}

	if(settings->dosboxFullscreen) {
		scriptArgs.addArgument("FULLSCREEN", "FULLSCREEN");
	}

	if(settings->dosboxAutoExit) {
		scriptArgs.addArgument("EXIT", "EXIT");
	}

	std::unique_ptr<DOSBoxConfiguration> combinedDOSBoxConfiguration;
	std::string combinedDOSBoxConfigurationFilePath;

	if(selectedGameVersion->doesRequireDOSBox()) {
		combinedDOSBoxConfigurationFilePath = Utilities::joinPaths(settings->appTempDirectoryPath, DOSBoxConfiguration::DEFAULT_FILE_NAME);
		combinedDOSBoxConfiguration = std::make_unique<DOSBoxConfiguration>(combinedDOSBoxConfigurationFilePath);

		if(!combinedDOSBoxConfiguration->mergeWith(*m_generalDOSBoxConfiguration)) {
			spdlog::error("Failed to merge general DOSBox configuration.", selectedDOSBoxVersion->getLongName());
		}

		if(!combinedDOSBoxConfiguration->mergeWith(*selectedDOSBoxVersion->getDOSBoxConfiguration())) {
			spdlog::error("Failed to merge '{}' configuration.", selectedDOSBoxVersion->getLongName());
		}

		if(!combinedDOSBoxConfiguration->mergeWith(*selectedGameVersion->getDOSBoxConfiguration())) {
			spdlog::error("Failed to merge '{}' configuration.", selectedGameVersion->getLongName());
		}

		if(combinedDOSBoxConfiguration->isNotEmpty()) {
			size_t sectionCount = combinedDOSBoxConfiguration->numberOfSections();
			size_t totalEntryCount = combinedDOSBoxConfiguration->totalNumberOfEntries();

			spdlog::debug("Using custom combined DOSBox configuration with {} section{} and {} {}.", sectionCount, sectionCount == 1 ? "" : "s", totalEntryCount, totalEntryCount == 1 ? "entry" : "entries");
		}
	}
	else if(m_gameType == GameType::Client || m_gameType == GameType::Server) {
		spdlog::info("Network settings are only supported when running in DOSBox, ignoring {} game type setting.", Utilities::toCapitalCase(magic_enum::enum_name(m_gameType)));
	}

	std::chrono::time_point<std::chrono::steady_clock> commandGenerationStartTimePoint(std::chrono::steady_clock::now());

	std::string command(generateCommand(selectedGameVersion, allRelativeConFilePaths, allRelativeDefFilePaths, allRelativeGroupFilePaths, scriptArgs, relativeCombinedGroupFilePath, combinedDOSBoxConfigurationFilePath, relativeCustomMapFilePath));


	if(command.empty()) {
		notifyLaunchError("Failed to generate launch command.");
		return false;
	}

	std::chrono::microseconds commandGenerationDuration(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - commandGenerationStartTimePoint));

	spdlog::trace("Generated command string after {} us.", commandGenerationDuration.count());

	if(!removeModFilesFromGameDirectory(*selectedGameVersion)) {
		notifyLaunchError(fmt::format("Failed to remove existing mod files from '{}' game directory.", selectedGameVersion->getLongName()));
		return false;
	}

	if(shouldConfigureApplicationTemporaryDirectory && !createApplicationTemporaryDirectory()) {
		notifyLaunchError("Failed to create application temporary directory.");
		return false;
	}

	if(combinedDOSBoxConfiguration != nullptr) {
		if(combinedDOSBoxConfiguration->save()) {
			spdlog::info("Saved custom combined DOSBox configuration to file: '{}'.", combinedDOSBoxConfiguration->getFilePath());
		}
		else {
			spdlog::error("Failed to save custom combined DOSBox configuration to file: '{}'.", combinedDOSBoxConfiguration->getFilePath());
		}

		combinedDOSBoxConfiguration.reset();
	}

	std::unique_ptr<InstalledModInfo> installedModInfo(std::make_unique<InstalledModInfo>(selectedModVersion.get()));

	if(!createSymlinksOrCopyTemporaryFiles(*selectedGameVersion, allAbsoluteConFilePaths, allAbsoluteDefFilePaths, allAbsoluteGroupFilePaths, absoluteCustomMapFilePath, doesRequireCombinedGroup, shouldConfigureApplicationTemporaryDirectory, installedModInfo.get())) {
		if(!removeModFilesFromGameDirectory(*selectedGameVersion, *installedModInfo)) {
			spdlog::error("Failed to remove '{}' mod files from '{}' game directory.", selectedModVersionType->getFullName(), selectedGameVersion->getLongName());
		}

		notifyLaunchError("Failed to create symbolic links or copy temporary mod files.");

		return false;
	}

	if(!selectedGameVersion->doesRequireGroupFileExtraction() && !m_demoRecordingEnabled) {
		std::vector<std::string> originalRenamedDemoFilePaths(ModManager::renameFilesWithSuffixTo("DMO", "DMO" + DEFAULT_BACKUP_FILE_RENAME_SUFFIX, selectedGameVersion->getGamePath()));

		for(const std::string & originalRenamedDemoFilePath : originalRenamedDemoFilePaths) {
			installedModInfo->addOriginalFile(originalRenamedDemoFilePath);
		}

		if(settings->demoExtractionEnabled) {
			size_t numberOfDemoFilesWritten = 0;
			size_t demoFileNumber = 1;

			for(std::map<std::string, std::unique_ptr<ByteBuffer>, FileNameComparator>::const_iterator i = demoFiles.cbegin(); i != demoFiles.cend(); ++i) {
				if(i->second->writeTo(Utilities::joinPaths(selectedGameVersion->getGamePath(), i->first))) {
					numberOfDemoFilesWritten++;
					installedModInfo->addModFile(i->first);

					spdlog::debug("Wrote demo #{}/{} '{}' to game directory '{}'.", demoFileNumber++, demoFiles.size(), i->first, selectedGameVersion->getGamePath());
				}
				else {
					spdlog::warn("Failed to write '{}' demo file to game directory '{}'.", i->first, selectedGameVersion->getGamePath());
				}
			}

			demoFiles.clear();

			spdlog::info("Wrote {} demo{} to game directory '{}'.", numberOfDemoFilesWritten, numberOfDemoFilesWritten == 1 ? "" : "s", selectedGameVersion->getGamePath());
		}
	}

	if(combinedGroup != nullptr || combinedZip != nullptr) {
		bool combinedGroupOrZipArchiveSaved = false;

		if(combinedZip != nullptr) {
			combinedGroupOrZipArchiveSaved = combinedZip->save();
		}
		else {
			combinedGroupOrZipArchiveSaved = combinedGroup->save(true);
		}

		if(combinedGroupOrZipArchiveSaved) {
			if(!shouldSymlinkToCombinedGroup) {
				std::error_code errorCode;
				std::filesystem::path relativeCombinedGroupFilePath(std::filesystem::relative(std::filesystem::path(absoluteCombinedGroupFilePath), std::filesystem::path(selectedGameVersion->getGamePath()), errorCode));

				if(!errorCode) {
					installedModInfo->addModFile(relativeCombinedGroupFilePath.string());
				}
				else {
					spdlog::warn("Failed to relativize combined group file path: '{}' against base path: '{}'.", absoluteCombinedGroupFilePath, selectedGameVersion->getGamePath());
				}
			}

			spdlog::info("Saved combined group to file: '{}'.", absoluteCombinedGroupFilePath);
		}
		else {
			if(!removeModFilesFromGameDirectory(*selectedGameVersion, *installedModInfo)) {
				spdlog::error("Failed to remove '{}' mod files from '{}' game directory.", selectedModVersionType->getFullName(), selectedGameVersion->getLongName());
			}

			notifyLaunchError(fmt::format("Failed to write combined group to file: '{}'.", absoluteCombinedGroupFilePath));

			return false;
		}

		combinedGroup.reset();
		combinedZip.reset();
	}

	if(m_selectedMod != nullptr && selectedGameVersion->doesRequireGroupFileExtraction()) {
		if(!extractModFilesToGameDirectory(*selectedModGameVersion, *selectedGameVersion, *selectedGameVersion, installedModInfo.get(), allAbsoluteGroupFilePaths)) {
			if(!removeModFilesFromGameDirectory(*selectedGameVersion, *installedModInfo)) {
				spdlog::error("Failed to remove '{}' mod files from '{}' game directory.", selectedModVersionType->getFullName(), selectedGameVersion->getLongName());
			}

			notifyLaunchError(fmt::format("Failed to extract mod files to '{}' game directory.", selectedGameVersion->getLongName()));

			return false;
		}
	}

	if(installedModInfo != nullptr && !installedModInfo->isEmpty()) {
		if(installedModInfo->saveTo(*selectedGameVersion)) {
			spdlog::info("Saved installed mod info to file '{}' in '{}' game directory.", InstalledModInfo::DEFAULT_FILE_NAME, selectedGameVersion->getLongName());
		}
		else {
			spdlog::warn("Failed to save installed mod info to '{}' game directory. Closing the application while the game is still running may result in your game being left in a bad state.", selectedGameVersion->getLongName());
		}
	}

	std::string customMapMessage;

	if(!relativeCustomMapFilePath.empty()) {
		customMapMessage = fmt::format(" with custom map '{}'", Utilities::getFileName(relativeCustomMapFilePath));
	}

	std::chrono::milliseconds runDelayDuration(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - runStartTimePoint));

	std::map<std::string, std::any> properties;

	if(settings->segmentAnalyticsEnabled) {
		properties["gameVersion"] = selectedGameVersion->getID();
		properties["command"] = command;
		properties["commandGenerationDurationUs"] = commandGenerationDuration.count();

		if(selectedGameVersion->doesRequireDOSBox()) {
			properties["dosboxVersion"] = selectedDOSBoxVersion->getID();
		}

		if(!customMap.empty()) {
			properties["customMap"] = customMap;
		}

		if(m_arguments != nullptr && m_arguments->hasPassthroughArguments()) {
			properties["arguments"] = m_arguments->getPassthroughArguments().value();
		}

		properties["runDelayDurationMs"] = runDelayDuration.count();

		if(!customConFilePaths.empty()) {
			properties["customConFiles"] = filePathsToFileNameList(customConFilePaths);
		}

		if(!customDefFilePaths.empty()) {
			properties["customDefFiles"] = filePathsToFileNameList(customDefFilePaths);
		}

		if(!customGroupFilePaths.empty()) {
			properties["customGroupFiles"] = filePathsToFileNameList(customGroupFilePaths);
		}
	}

	std::string gameTypeName(Utilities::toCapitalCase(magic_enum::enum_name(m_gameType)));

	SegmentAnalytics * segmentAnalytics = SegmentAnalytics::getInstance();

	if(customMod && m_selectedMod == nullptr) {
		spdlog::info("Running custom mod in {} mode{}.", Utilities::toCapitalCase(magic_enum::enum_name(m_gameType)), customMapMessage);

		if(settings->segmentAnalyticsEnabled) {
			segmentAnalytics->track("Running Custom Mod", properties);
		}
	}
	else if(m_selectedMod != nullptr) {
		std::string fullModName(m_selectedMod->getFullName(m_selectedModVersionIndex, m_selectedModVersionTypeIndex));

		if(standAlone) {
			spdlog::info("Running '{}' stand-alone mod in {} mode{}.", fullModName, gameTypeName, customMapMessage);
		}
		else {
			spdlog::info("Running '{}' version of mod '{}' in {} mode{}.", getGameVersions()->getLongNameOfGameVersionWithID(selectedModGameVersion->getGameVersionID()), fullModName, gameTypeName, customMapMessage);
		}

		if(settings->segmentAnalyticsEnabled) {
			properties["modID"] = m_selectedMod->getID();
			properties["modName"] = m_selectedMod->getName();
			properties["modVersion"] = selectedModVersion->getVersion();
			properties["modVersionType"] = selectedModVersionType->getType();
			properties["modFullName"] = fullModName;
			properties["modGameVersionID"] = selectedModGameVersion->getGameVersionID();
			properties["preferredVersion"] = m_selectedMod->getPreferredVersionName();
			properties["defaultVersionType"] = m_selectedMod->getDefaultVersionType();
			properties["initialReleaseDate"] = m_selectedMod->getInitialReleaseDateAsString();
			properties["latestReleaseDate"] = m_selectedMod->getLatestReleaseDateAsString();
			properties["hasDependencies"] = m_selectedMod->hasDependencies();
			properties["officialExpansion"] = m_selectedMod->isOfficialExpansion();
			properties["hasTeam"] = m_selectedMod->hasTeam();
			properties["standAlone"] = standAlone;
			properties["gameType"] = gameTypeName;
			properties["numberOfVersions"] = m_selectedMod->numberOfVersions();
			properties["numberOfDownloads"] = m_selectedMod->numberOfDownloads();
			properties["numberOfScreenshots"] = m_selectedMod->numberOfScreenshots();
			properties["numberOfImages"] = m_selectedMod->numberOfImages();
			properties["numberOfVideos"] = m_selectedMod->numberOfVideos();
			properties["numberOfNotes"] = m_selectedMod->numberOfNotes();
			properties["numberOfRelatedMods"] = m_selectedMod->numberOfRelatedMods();
			properties["numberOfSimilarMods"] = m_selectedMod->numberOfSimilarMods();

			if(m_selectedMod->hasAlias()) {
				properties["modAlias"] = m_selectedMod->getAlias();
			}

			if(!modConFiles.empty()) {
				properties["modConFiles"] = modFilesToFileNameList(modConFiles);
			}

			if(!modDefFiles.empty()) {
				properties["modDefFiles"] = modFilesToFileNameList(modDefFiles);
			}

			if(!modGroupFiles.empty()) {
				properties["modGroupFiles"] = modFilesToFileNameList(modGroupFiles);
			}

			if(!modDependencyGroupFiles.empty()) {
				properties["modDependencyGroupFiles"] = modFilesToFileNameList(modDependencyGroupFiles);
			}

			segmentAnalytics->track("Running Mod", properties);
		}
	}
	else {
		if(settings->segmentAnalyticsEnabled) {
			segmentAnalytics->track("Running Game", properties);
		}
	}

	segmentAnalytics->flush();

	std::string workingDirectory(selectedGameVersion->doesRequireDOSBox() ? selectedDOSBoxVersion->getDirectoryPath() : selectedGameVersion->getGamePath());

	spdlog::info("Using working directory: '{}'.", workingDirectory);
	spdlog::info("Executing command: {}", command);

	m_gameProcess = ProcessCreator::getInstance()->createProcess(command, workingDirectory);

	if(m_gameProcess == nullptr) {
		if(installedModInfo != nullptr && !installedModInfo->isEmpty() && !removeModFilesFromGameDirectory(*selectedGameVersion, *installedModInfo)) {
			spdlog::error("Failed to remove '{}' mod files from '{}' game directory.", selectedModVersionType->getFullName(), selectedGameVersion->getLongName());
		}

		notifyLaunchError("Failed to create game process, check console for details.");

		return false;
	}

	boost::signals2::connection terminatedConnection(m_gameProcess->terminated.connect([this, gameTypeName](uint64_t nativeExitCode, bool forceTerminated) {
		spdlog::info("{} process {} with code: '{}'.", gameTypeName, forceTerminated ? "force terminated" : "exited", nativeExitCode);

		gameProcessTerminated(nativeExitCode, forceTerminated);
	}));

	selectedGameVersion->updateLastPlayedTimePoint();

	if(selectedGameVersion->doesRequireDOSBox()) {
		selectedDOSBoxVersion->updateLastLaunchedTimePoint();
	}

	launched();

	m_gameProcess->wait();
	terminatedConnection.disconnect();

	if(installedModInfo != nullptr && !installedModInfo->isEmpty()) {
		if(!removeModFilesFromGameDirectory(*selectedGameVersion, *installedModInfo)) {
			spdlog::error("Failed to remove '{}' mod files from '{}' game directory.", selectedModVersionType->getFullName(), selectedGameVersion->getLongName());
		}
	}

	if(!absoluteCombinedGroupFilePath.empty() && shouldSymlinkToCombinedGroup) {
		std::filesystem::path filePath(absoluteCombinedGroupFilePath);

		if(std::filesystem::is_regular_file(filePath)) {
			spdlog::info("Deleting temporary combined group file: '{}'.", absoluteCombinedGroupFilePath);

			std::error_code errorCode;
			std::filesystem::remove(filePath, errorCode);

			if(errorCode) {
				spdlog::error("Failed to delete temporary combined group file '{}': {}", absoluteCombinedGroupFilePath, errorCode.message());
			}
		}
	}

	if(!combinedDOSBoxConfigurationFilePath.empty()) {
		std::filesystem::path filePath(combinedDOSBoxConfigurationFilePath);

		if(std::filesystem::is_regular_file(filePath)) {
			spdlog::info("Deleting generated combined DOSBox configuration file: '{}'.", combinedDOSBoxConfigurationFilePath);

			std::error_code errorCode;
			std::filesystem::remove(filePath, errorCode);

			if(errorCode) {
				spdlog::error("Failed to delete generated combined DOSBox configuration file '{}': {}", combinedDOSBoxConfigurationFilePath, errorCode.message());
			}
		}
	}

	removeSymlinks(*selectedGameVersion);

	m_gameProcess.reset();

	return true;
}

bool ModManager::isGameProcessRunning() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_gameProcess != nullptr && m_gameProcess->isRunning();
}

std::shared_ptr<Process> ModManager::getGameProcess() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_gameProcess;
}

bool ModManager::terminateGameProcess() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_gameProcess == nullptr || !m_gameProcess->isRunning()) {
		return false;
	}

	m_gameProcess->terminate();
	m_gameProcess.reset();

	return true;
}

void ModManager::notifyLaunchError(const std::string & errorMessage) {
	spdlog::error(errorMessage);
	launchError(errorMessage);
}

bool ModManager::areModFilesPresentInGameDirectory(const GameVersion & gameVersion) {
	if(!gameVersion.isConfigured()) {
		return false;
	}

	return areModFilesPresentInGameDirectory(gameVersion.getGamePath());
}

bool ModManager::areModFilesPresentInGameDirectory(const std::string & gamePath) {
	return !gamePath.empty() &&
		   std::filesystem::is_regular_file(std::filesystem::path(Utilities::joinPaths(gamePath, InstalledModInfo::DEFAULT_FILE_NAME)));
}

bool ModManager::extractModFilesToGameDirectory(const ModGameVersion & modGameVersion, const GameVersion & selectedGameVersion, const GameVersion & targetGameVersion, InstalledModInfo * installedModInfo, const std::vector<std::string> & groupFilePaths) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	struct FilePathComparator {
	public:
		bool operator () (const std::string & filePathA, const std::string & filePathB) const {
			return std::lexicographical_compare(filePathA.begin(), filePathA.end(), filePathB.begin(), filePathB.end(), [](unsigned char a, unsigned char b) {
				return std::tolower(a) < std::tolower(b);
			});
		}
	};

	if(!modGameVersion.isValid(true) || !targetGameVersion.isValid()) {
		return false;
	}

	SettingsManager * settings = SettingsManager::getInstance();

	std::map<std::string, std::unique_ptr<ByteBuffer>, FilePathComparator> modFiles;
	std::string modPath(Utilities::joinPaths(settings->modsDirectoryPath, targetGameVersion.getModDirectoryName()));

	for(const std::string & groupFilePath : groupFilePaths) {
		if(Utilities::hasFileExtension(groupFilePath, "zip")) {
			std::unique_ptr<ZipArchive> zipArchive(ZipArchive::readFrom(groupFilePath));

			if(zipArchive == nullptr) {
				spdlog::error("Failed to open zip archive file: '{}'.", groupFilePath);
				return false;
			}

			std::vector<std::shared_ptr<ArchiveEntry>> zipArchiveEntries(zipArchive->getEntries());

			for(const std::shared_ptr<ArchiveEntry> & zipArchiveEntry : zipArchiveEntries) {
				if(zipArchiveEntry == nullptr || !zipArchiveEntry->isFile()) {
					continue;
				}

				modFiles[zipArchiveEntry->getName()] = zipArchiveEntry->getData();
			}
		}
		else {
			std::unique_ptr<Group> group(GroupGRP::loadFrom(groupFilePath));

			if(!Group::isValid(group.get())) {
				spdlog::error("Failed to open group file: '{}'.", groupFilePath);
				return false;
			}

			std::shared_ptr<GroupFile> currentGroupFile;

			for(size_t i = 0; i < group->numberOfFiles(); i++) {
				currentGroupFile = group->getFile(i);

				modFiles[currentGroupFile->getFileName()] = currentGroupFile->transferData();
			}
		}
	}

	if(modFiles.empty()) {
		spdlog::debug("No mod files to extract to '{}' game directory.", selectedGameVersion.getLongName());
		return true;
	}

	std::vector<std::string> originalFilePaths;

	for(auto modFilesIterator = modFiles.cbegin(); modFilesIterator != modFiles.cend(); ++modFilesIterator) {
		if(std::filesystem::is_regular_file(std::filesystem::path(Utilities::joinPaths(selectedGameVersion.getGamePath(), modFilesIterator->first)))) {
			if(std::filesystem::is_regular_file(std::filesystem::path(Utilities::joinPaths(selectedGameVersion.getGamePath(), modFilesIterator->first) + DEFAULT_BACKUP_FILE_RENAME_SUFFIX))) {
				spdlog::error("Cannot temporarily rename original '{}' file, original backup file already exists at path: '{}'. Please manually restore or remove this file.", selectedGameVersion.getLongName(), Utilities::joinPaths(selectedGameVersion.getGamePath(), modFilesIterator->first) + DEFAULT_BACKUP_FILE_RENAME_SUFFIX);
				return false;
			}

			originalFilePaths.push_back(modFilesIterator->first);
		}
	}

	for(const std::string & originalFilePath : originalFilePaths) {
		std::string relativeRenamedFilePath(originalFilePath + DEFAULT_BACKUP_FILE_RENAME_SUFFIX);
		std::string fullOriginalFilePath(Utilities::joinPaths(selectedGameVersion.getGamePath(), originalFilePath));
		std::string fullRenamedFilePath(Utilities::joinPaths(selectedGameVersion.getGamePath(), relativeRenamedFilePath));

		spdlog::debug("Renaming '{}' file '{}' to '{}'.", selectedGameVersion.getLongName(), originalFilePath, relativeRenamedFilePath);

		std::error_code errorCode;
		std::filesystem::rename(std::filesystem::path(fullOriginalFilePath), std::filesystem::path(fullRenamedFilePath), errorCode);

		if(!errorCode) {
			if(installedModInfo != nullptr) {
				installedModInfo->addOriginalFile(originalFilePath);
			}
		}
		else {
			spdlog::error("Failed to rename '{}' file '{}' to '{}': {}", selectedGameVersion.getLongName(), originalFilePath, relativeRenamedFilePath, errorCode.message());
		}
	}

	size_t fileNumber = 1;

	bool isRunningNonBetaModOnBetaGameVersion = Utilities::areStringsEqualIgnoreCase(selectedGameVersion.getID(), GameVersion::ORIGINAL_BETA_VERSION.getID()) &&
												!Utilities::areStringsEqualIgnoreCase(modGameVersion.getGameVersionID(), GameVersion::ORIGINAL_BETA_VERSION.getID());

	for(auto modFilesIterator = modFiles.cbegin(); modFilesIterator != modFiles.cend(); ++modFilesIterator, fileNumber++) {
		if(isRunningNonBetaModOnBetaGameVersion && Utilities::hasFileExtension(modFilesIterator->first, "DMO")) {
			spdlog::info("Skipping extraction of '{}' demo file '{}' into '{}' game directory.", modGameVersion.getFullName(true), modFilesIterator->first, selectedGameVersion.getLongName());
			continue;
		}

		std::string fullModFilePath(Utilities::joinPaths(selectedGameVersion.getGamePath(), modFilesIterator->first));

		if(!modFilesIterator->second->writeTo(fullModFilePath)) {
			spdlog::error("Failed to write mod file #{} of {} ('{}') to '{}' game directory.", fileNumber, modFiles.size(), modFilesIterator->first, selectedGameVersion.getLongName());
			continue;
		}

		if(installedModInfo != nullptr) {
			installedModInfo->addModFile(modFilesIterator->first);
		}

		spdlog::info("Extracted mod file #{} of {} ('{}') to '{}' game directory.", fileNumber, modFiles.size(), modFilesIterator->first, selectedGameVersion.getLongName());
	}

	return true;
}

bool ModManager::removeModFilesFromGameDirectory(const GameVersion & gameVersion) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!gameVersion.isConfigured()) {
		return false;
	}

	if(!areModFilesPresentInGameDirectory(gameVersion)) {
		return true;
	}

	std::unique_ptr<InstalledModInfo> installedModInfo(InstalledModInfo::loadFrom(gameVersion));

	if(installedModInfo == nullptr) {
		return false;
	}

	return removeModFilesFromGameDirectory(gameVersion, *installedModInfo);
}

bool ModManager::removeModFilesFromGameDirectory(const GameVersion & gameVersion, const InstalledModInfo & installedModInfo) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(installedModInfo.isEmpty()) {
		return true;
	}

	if(!gameVersion.isConfigured() || !installedModInfo.isValid() || !std::filesystem::is_directory(std::filesystem::path(gameVersion.getGamePath()))) {
		return false;
	}

	if(installedModInfo.numberOfModFiles() != 0) {
		spdlog::info("Removing {} '{}' mod file{} from '{}' game directory...", installedModInfo.numberOfModFiles(), installedModInfo.getFullModName(), installedModInfo.numberOfModFiles() == 1 ? "" : "s", gameVersion.getLongName());

		for(size_t i = 0; i < installedModInfo.numberOfModFiles(); i++) {
			std::string relativeModFilePath(installedModInfo.getModFile(i));
			std::string fullModFilePath(Utilities::joinPaths(gameVersion.getGamePath(), relativeModFilePath));
			bool modFileExists = std::filesystem::is_regular_file(std::filesystem::path(fullModFilePath));

			if(!modFileExists) {
				spdlog::warn("Mod file #{} of {} ('{}') no longer exists in '{}' game directory.", i + 1, installedModInfo.numberOfModFiles(), relativeModFilePath, gameVersion.getLongName());
				continue;
			}

			std::error_code errorCode;
			std::filesystem::remove(std::filesystem::path(fullModFilePath), errorCode);

			if(errorCode) {
				spdlog::error("Failed to remove mod file #{} of {} ('{}') from '{}' game directory: {}", i + 1, installedModInfo.numberOfModFiles(), relativeModFilePath, gameVersion.getLongName(), errorCode.message());
				continue;
			}

			spdlog::debug("Removed mod file #{} of {} ('{}') from '{}' game directory.", i + 1, installedModInfo.numberOfModFiles(), relativeModFilePath, gameVersion.getLongName());
		}
	}

	if(installedModInfo.numberOfOriginalFiles() != 0) {
		spdlog::info("Renaming {0} '{1}' backed up game file{2} back to {3} original name{2}...", installedModInfo.numberOfOriginalFiles(), gameVersion.getLongName(), installedModInfo.numberOfOriginalFiles() == 1 ? "" : "s", installedModInfo.numberOfOriginalFiles() == 1 ? "its" : "their");

		for(size_t i = 0; i < installedModInfo.numberOfOriginalFiles(); i++) {
			std::string relativeOriginalFilePath(installedModInfo.getOriginalFile(i));
			std::string relativeRenamedFilePath(relativeOriginalFilePath + DEFAULT_BACKUP_FILE_RENAME_SUFFIX);
			std::string fullOriginalFilePath(Utilities::joinPaths(gameVersion.getGamePath(), relativeOriginalFilePath));
			std::string fullRenamedFilePath(Utilities::joinPaths(gameVersion.getGamePath(), relativeRenamedFilePath));
			bool originalFileExists = std::filesystem::is_regular_file(std::filesystem::path(fullOriginalFilePath));
			bool renamedFileExists = std::filesystem::is_regular_file(std::filesystem::path(fullRenamedFilePath));

			if(!renamedFileExists) {
				spdlog::error("Renamed backup file #{} of {} ('{}') no longer exists in '{}' game directory.", i + 1, installedModInfo.numberOfOriginalFiles(), relativeRenamedFilePath, gameVersion.getLongName());
				continue;
			}

			if(originalFileExists) {
				spdlog::error("Original file #{} of {} ('{}') already exists in '{}' game directory, it will be replaced.", i + 1, installedModInfo.numberOfOriginalFiles(), relativeOriginalFilePath, gameVersion.getLongName());

				std::error_code errorCode;
				std::filesystem::remove(std::filesystem::path(fullOriginalFilePath), errorCode);

				if(errorCode) {
					spdlog::error("Failed to remove original file ('{}') from '{}' game directory: {}", relativeOriginalFilePath, gameVersion.getLongName(), errorCode.message());
					continue;
				}
			}

			std::error_code errorCode;
			std::filesystem::rename(std::filesystem::path(fullRenamedFilePath), std::filesystem::path(fullOriginalFilePath), errorCode);

			if(errorCode) {
				spdlog::error("Failed to rename original backed up '{}' game file #{} of {} from '{}' to '{}': {}", gameVersion.getLongName(), i + 1, installedModInfo.numberOfOriginalFiles(), relativeRenamedFilePath, relativeOriginalFilePath, errorCode.message());
				continue;
			}

			spdlog::info("Renamed original backed up '{}' game file #{} of {} from '{}' to '{}'.", gameVersion.getLongName(), i + 1, installedModInfo.numberOfOriginalFiles(), relativeRenamedFilePath, relativeOriginalFilePath);
		}
	}

	std::filesystem::path installedModInfoFilePath(Utilities::joinPaths(gameVersion.getGamePath(), InstalledModInfo::DEFAULT_FILE_NAME));

	if(!std::filesystem::exists(installedModInfoFilePath)) {
		spdlog::warn("Installed mod info file '{}' no longer exists in '{}' game directory.", InstalledModInfo::DEFAULT_FILE_NAME, gameVersion.getGamePath());
		return true;
	}

	std::error_code errorCode;
	std::filesystem::remove(installedModInfoFilePath, errorCode);

	if(errorCode) {
		spdlog::error("Failed to remove installed mod info file '{}' from '{}' game directory: {}", InstalledModInfo::DEFAULT_FILE_NAME, gameVersion.getLongName(), errorCode.message());
		return true;
	}

	spdlog::info("Removed installed mod info file '{}' from '{}' game directory.", InstalledModInfo::DEFAULT_FILE_NAME, gameVersion.getLongName());

	return true;
}

std::string ModManager::generateCommand(std::shared_ptr<GameVersion> gameVersion, const std::vector<std::string> & relativeConFilePaths, const std::vector<std::string> & relativeDefFilePaths, const std::vector<std::string> & relativeGroupFilePaths, ScriptArguments & scriptArgs, std::string_view relativeCombinedGroupFilePath, std::string_view combinedDOSBoxConfigurationFilePath, std::string_view relativeCustomMapFilePath) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	static const std::regex respawnModeRegExp("[123x]");

	if(!m_initialized) {
		spdlog::error("Mod manager not initialized.");
		return {};
	}

	SettingsManager * settings = SettingsManager::getInstance();

	if(!gameVersion->isConfigured()) {
		spdlog::error("Invalid or unconfigured game version.");
		return {};
	}

	if(settings->dataDirectoryPath.empty()) {
		spdlog::error("Empty data path.");
		return {};
	}

	std::shared_ptr<DOSBoxVersion> selectedDOSBoxVersion(getSelectedDOSBoxVersion());

	if(gameVersion->doesRequireDOSBox() && !DOSBoxVersion::isConfigured(selectedDOSBoxVersion.get())) {
		spdlog::error("Selected DOSBox version '{}' is not configured.", selectedDOSBoxVersion != nullptr ? selectedDOSBoxVersion->getLongName() : "N/A");
		return {};
	}

	if(settings->gameSymlinkName.empty()) {
		spdlog::error("Empty game directory symbolic link name.");
		return {};
	}

	if(settings->modsSymlinkName.empty()) {
		spdlog::error("Empty mods directory symbolic link name.");
		return {};
	}

	if(settings->gameTempDirectoryName.empty() || settings->gameTempDirectoryName.find_first_of("/\\") != std::string::npos) {
		spdlog::error("Empty or invalid game temporary directory name, must not be empty and not contain any path separators.");
		return {};
	}

	std::string executableName;

	if(m_gameType == GameType::Game) {
		executableName = gameVersion->getGameExecutableName();
	}
	else {
		if(!gameVersion->hasSetupExecutableName()) {
			spdlog::error("Game version '{}' does not have a setup executable.", gameVersion->getLongName());
			return {};
		}

		executableName = gameVersion->getSetupExecutableName().value();
	}

	std::stringstream command;

	if(m_arguments != nullptr) {
		if(m_arguments->hasArgument("v") || m_arguments->hasArgument("episode")) {
			std::string episodeNumberData(m_arguments->getFirstValue("v"));

			if(episodeNumberData.empty()) {
				episodeNumberData = m_arguments->getFirstValue("episode");
			}

			std::optional<uint8_t> optionalEpisodeNumber(Utilities::parseUnsignedByte(episodeNumberData));

			if(optionalEpisodeNumber.has_value() && optionalEpisodeNumber.value() >= 1) {
				command << " " << gameVersion->getEpisodeArgumentFlag() << std::to_string(optionalEpisodeNumber.value());
			}
			else {
				spdlog::warn("Invalid episode number: '{}'.", episodeNumberData);
			}
		}

		if(m_arguments->hasArgument("l") || m_arguments->hasArgument("level")) {
			std::string levelNumberData(m_arguments->getFirstValue("l"));

			if(levelNumberData.empty()) {
				levelNumberData = m_arguments->getFirstValue("level");
			}

			std::optional<uint8_t> optionalLevelNumber(Utilities::parseUnsignedByte(levelNumberData));

			if(optionalLevelNumber.has_value() && optionalLevelNumber.value() >= 1 && optionalLevelNumber.value() <= 11) {
				command << " " << gameVersion->getLevelArgumentFlag() << std::to_string(optionalLevelNumber.value());
			}
			else {
				spdlog::warn("Invalid level number: '{}'.", levelNumberData);
			}
		}

		if(m_arguments->hasArgument("s") || m_arguments->hasArgument("skill")) {
			std::string skillNumberData(m_arguments->getFirstValue("s"));

			if(skillNumberData.empty()) {
				skillNumberData = m_arguments->getFirstValue("skill");
			}

			std::optional<uint8_t> optionalSkillNumber(Utilities::parseUnsignedByte(skillNumberData));

			if(optionalSkillNumber.has_value() && optionalSkillNumber.value() >= 1 && optionalSkillNumber.value() <= 4) {
				command << " " << gameVersion->getSkillArgumentFlag() << std::to_string(optionalSkillNumber.value() - 1 + gameVersion->getSkillStartValue());
			}
			else {
				spdlog::warn("Invalid skill number: '{}'.", skillNumberData);
			}
		}

		if(m_demoRecordingEnabled) {
			command << " " << gameVersion->getRecordDemoArgumentFlag();
		}

		if(m_arguments->hasArgument("d") || m_arguments->hasArgument("demo")) {
			std::string demoFileName(m_arguments->getFirstValue("d"));

			if(demoFileName.empty()) {
				demoFileName = m_arguments->getFirstValue("demo");
			}

			if(!demoFileName.empty()) {
				if(gameVersion->hasPlayDemoArgumentFlag()) {
					command << " " << gameVersion->getPlayDemoArgumentFlag().value() << demoFileName;
				}
				else {
					spdlog::warn("Game version '{}' does not have a play demo argument flag specified in its configuration.", gameVersion->getLongName());
				}
			}
		}

		if(m_arguments->hasArgument("t") || m_arguments->hasArgument("respawn")) {
			std::string respawnMode(m_arguments->getFirstValue("t"));

			if(respawnMode.empty()) {
				respawnMode = m_arguments->getFirstValue("respawn");
			}

			if(!respawnMode.empty() && std::regex_match(respawnMode, respawnModeRegExp)) {
				if(gameVersion->hasRespawnModeArgumentFlag()) {
					command << " " << gameVersion->getRespawnModeArgumentFlag().value() << respawnMode;
				}
				else {
					spdlog::warn("Game version '{}' does not have a respawn mode argument flag specified in its configuration.", gameVersion->getLongName());
				}
			}
		}

		if(m_arguments->hasArgument("u") || m_arguments->hasArgument("weaponswitch")) {
			std::string weaponSwitchOrder(m_arguments->getFirstValue("u"));

			if(weaponSwitchOrder.empty()) {
				weaponSwitchOrder = m_arguments->getFirstValue("weaponswitch");
			}

			if(!weaponSwitchOrder.empty() && weaponSwitchOrder.find_first_not_of("0123456789") == std::string::npos) {
				if(gameVersion->hasWeaponSwitchOrderArgumentFlag()) {
					command << " " << gameVersion->getWeaponSwitchOrderArgumentFlag().value() << weaponSwitchOrder;
				}
				else {
					spdlog::warn("Game version '{}' does not have a weapon switch order argument flag specified in its configuration.", gameVersion->getLongName());
				}
			}
		}

		if(m_arguments->hasArgument("m") || m_arguments->hasArgument("nomonsters")) {
			if(gameVersion->hasDisableMonstersArgumentFlag()) {
				command << " " << gameVersion->getDisableMonstersArgumentFlag().value();
			}
			else {
				spdlog::warn("Game version '{}' does not have a disable monsters argument flag specified in its configuration.", gameVersion->getLongName());
			}
		}

		if(m_arguments->hasArgument("ns") || m_arguments->hasArgument("nosound")) {
			if(gameVersion->hasDisableSoundArgumentFlag()) {
				command << " " << gameVersion->getDisableSoundArgumentFlag().value();
			}
			else {
				spdlog::warn("Game version '{}' does not have a disable sound argument flag specified in its configuration.", gameVersion->getLongName());
			}
		}

		if(m_arguments->hasArgument("nm") || m_arguments->hasArgument("nomusic")) {
			if(gameVersion->hasDisableMusicArgumentFlag()) {
				command << " " << gameVersion->getDisableMusicArgumentFlag().value();
			}
			else {
				spdlog::warn("Game version '{}' does not have a disable music argument flag specified in its configuration.", gameVersion->getLongName());
			}
		}
	}

	if(gameVersion->hasLaunchArguments()) {
		command << " " << gameVersion->getLaunchArguments();
	}

	if(!relativeCustomMapFilePath.empty()) {
		if(settings->mapsSymlinkName.empty()) {
			spdlog::error("Maps directory symbolic link name is empty.");
			return {};
		}

		if(gameVersion->hasMapFileArgumentFlag()) {
			command << " " << gameVersion->getMapFileArgumentFlag().value() << relativeCustomMapFilePath;
		}
		else {
			spdlog::warn("Game version '{}' does not have a map file argument flag specified in its configuration.", gameVersion->getLongName());
		}
	}

	if(!gameVersion->doesRequireGroupFileExtraction()) {
		if(!gameVersion->hasGroupFileArgumentFlag()) {
			spdlog::error("Game version '{}' does not have a group file argument flag specified in its configuration.", gameVersion->getLongName());
			return {};
		}

		if(!relativeConFilePaths.empty()) {
			if(!gameVersion->hasConFileArgumentFlag()) {
				spdlog::error("Game version '{}' does not have a con file argument flag specified in its configuration.", gameVersion->getLongName());
				return {};
			}
			else if(relativeConFilePaths.size() > 1 && !gameVersion->hasExtraConFileArgumentFlag()) {
				spdlog::error("Multiple con files specified, but game version '{}' does not have an extra con file argument flag specified in its configuration.", gameVersion->getLongName());
				return {};
			}

			for(std::vector<std::string>::const_iterator i = relativeConFilePaths.cbegin(); i != relativeConFilePaths.cend(); ++i) {
				const std::string & relativeConFilePath = *i;

				command << " ";

				if(i == relativeConFilePaths.begin()) {
					command << gameVersion->getConFileArgumentFlag().value();
				}
				else {
					command << gameVersion->getExtraConFileArgumentFlag().value();
				}

				command << relativeConFilePath;
			}
		}

		if(!relativeDefFilePaths.empty()) {
			if(!gameVersion->hasDefFileArgumentFlag()) {
				spdlog::error("Game version '{}' does not have a def file argument flag specified in its configuration.", gameVersion->getLongName());
				return {};
			}
			else if(relativeDefFilePaths.size() > 1 && !gameVersion->hasExtraDefFileArgumentFlag()) {
				spdlog::error("Multiple def files specified, but game version '{}' does not have an extra def file argument flag specified in its configuration.", gameVersion->getLongName());
				return {};
			}

			for(std::vector<std::string>::const_iterator i = relativeDefFilePaths.cbegin(); i != relativeDefFilePaths.cend(); ++i) {
				const std::string & relativeDefFilePath = *i;

				command << " ";

				if(i == relativeDefFilePaths.begin()) {
					command << gameVersion->getDefFileArgumentFlag().value();
				}
				else {
					command << gameVersion->getExtraDefFileArgumentFlag().value();
				}

				command << relativeDefFilePath;
			}
		}

		if(!relativeCombinedGroupFilePath.empty()) {
			command << " " << gameVersion->getGroupFileArgumentFlag().value() << relativeCombinedGroupFilePath;
		}
		else {
			for(const std::string & relativeGroupFilePath : relativeGroupFilePaths) {
				command << " " << gameVersion->getGroupFileArgumentFlag().value() << relativeGroupFilePath;
			}
		}
	}

	if(gameVersion->doesRequireDOSBox()) {
		std::shared_ptr<DOSBoxVersion> selectedDOSBoxVersion(getSelectedDOSBoxVersion());

		if(selectedDOSBoxVersion == nullptr) {
			spdlog::error("No DOSBox version selected.");
			return {};
		}

		if(!selectedDOSBoxVersion->isConfigured()) {
			spdlog::error("Selected DOSBox version '{}' is not configured.", selectedDOSBoxVersion->getLongName());
			return {};
		}

		Script dosboxScript;

		scriptArgs.addArgument("COMMAND", executableName + command.str());

		std::string dosboxTemplateScriptFilePath(getDOSBoxCommandScriptFilePath(m_gameType));

		if(!dosboxScript.readFrom(dosboxTemplateScriptFilePath)) {
			spdlog::error("Failed to load DOSBox command script file: '{}'.", dosboxTemplateScriptFilePath);
			return {};
		}

		return generateDOSBoxCommand(dosboxScript, scriptArgs, *selectedDOSBoxVersion, settings->dosboxArguments, settings->dosboxShowConsole, settings->dosboxFullscreen, combinedDOSBoxConfigurationFilePath);
	}

	return "\"" +  Utilities::joinPaths(gameVersion->getGamePath(), executableName) + "\"" + command.str();
}

std::string ModManager::generateDOSBoxCommand(const Script & script, const ScriptArguments & arguments, const DOSBoxVersion & dosboxVersion, const std::string & dosboxArguments, bool showConsole, bool fullscreen, std::string_view combinedDOSBoxConfigurationFilePath) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	static const std::regex unescapedQuotesRegExp("(?:^\"|([^\\\\])\")");

	if(!dosboxVersion.isConfigured()) {
		return std::string();
	}

	std::stringstream command;

	command << fmt::format("\"{}\"", Utilities::joinPaths(dosboxVersion.getDirectoryPath(), dosboxVersion.getExecutableName()));

	if(!combinedDOSBoxConfigurationFilePath.empty() && std::filesystem::is_regular_file(std::filesystem::path(combinedDOSBoxConfigurationFilePath))) {
		command << " -conf \"";
		command << combinedDOSBoxConfigurationFilePath;
		command << "\"";
	}

	if(!showConsole) {
		command << " -noconsole";
	}

	if(fullscreen) {
		command << " -fullscreen";
	}

	if(dosboxVersion.hasLaunchArguments()) {
		command << " " << dosboxVersion.getLaunchArguments();
	}

	if(!dosboxArguments.empty()) {
		command << " " << dosboxArguments;
	}

	std::string line;
	std::string formattedLine;

	for(size_t i = 0; i < script.numberOfCommands(); i++) {
		line = arguments.applyArguments(*script.getCommand(i));

		formattedLine.clear();
		std::regex_replace(std::back_inserter(formattedLine), line.begin(), line.end(), unescapedQuotesRegExp, "$1\\\"");

		if(!formattedLine.empty()) {
			command << fmt::format(" -c \"{}\"", formattedLine);
		}
	}

	return Utilities::trimString(command.str());
}

bool ModManager::handleArguments(const ArgumentParser * args) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(args == nullptr) {
		return true;
	}

	SettingsManager * settings = SettingsManager::getInstance();


	if(args->hasArgument("update-new")) {
		updateFileInfoForAllMods(true, true);
	}
	else if(args->hasArgument("update-all")) {
		updateFileInfoForAllMods(true, false);
	}

	if(args->hasArgument("test-parsing")) {
		testParsing();
	}

	if(args->hasArgument("type")) {
		std::optional<GameType> newGameTypeOptional(magic_enum::enum_cast<GameType>(Utilities::toPascalCase(args->getFirstValue("type"))));

		if(!newGameTypeOptional.has_value()) {
			static std::string gameTypes;

			if(gameTypes.empty()) {
				std::stringstream gameTypesStream;
				constexpr auto & gameTypeNames = magic_enum::enum_names<GameType>();

				for(size_t i = 0; i < gameTypeNames.size(); i++) {
					if(gameTypesStream.tellp() != 0) {
						gameTypesStream << ", ";
					}

					gameTypesStream << Utilities::toCapitalCase(gameTypeNames[i]);
				}

				gameTypes = gameTypesStream.str();
			}

			spdlog::error("Invalid game type, please specify one of the following: {}.", gameTypes);

			return false;
		}

		m_gameType = newGameTypeOptional.value();

		spdlog::info("Setting game type to: '{}'.", Utilities::toCapitalCase(magic_enum::enum_name(m_gameType)));

		if(m_gameType == GameType::Client) {
			std::string ipAddress;

			if(args->hasArgument("ip")) {
				ipAddress = Utilities::trimString(args->getFirstValue("ip"));

				bool error = ipAddress.empty() || ipAddress.find_first_of(" \t") != std::string::npos;

				if(error) {
					spdlog::error("Invalid IP Address entered in arguments: '{}'.", ipAddress);
					return false;
				}
				else {
					settings->dosboxServerIPAddress = ipAddress;
				}
			}
		}

		if(m_gameType == GameType::Client || m_gameType == GameType::Server) {
			if(args->hasArgument("port")) {
				std::string portData(Utilities::trimString(args->getFirstValue("port")));
				bool error = false;
				uint16_t port = static_cast<uint16_t>(Utilities::parseUnsignedInteger(portData, &error));

				if(port == 0) {
					error = true;
				}

				if(error) {
					spdlog::error("Invalid {} Server Port entered in arguments: '{}'.", m_gameType == GameType::Server ? "Local" : "Remote", portData);
					return false;
				}
				else {
					if(m_gameType == GameType::Server) {
						settings->dosboxLocalServerPort = port;
					}
					else {
						settings->dosboxRemoteServerPort = port;
					}
				}
			}
		}
	}

	if(args->hasArgument("search")) {
		if(args->hasArgument("random")) {
			spdlog::error("Redundant arguments specified, please specify either search or random.");
			return false;
		}

		std::vector<ModMatch> modMatches(searchForMod(m_mods->getMods(), args->getFirstValue("search"), true, true));

		if(modMatches.empty()) {
			spdlog::error("No matches found for specified search query.");
			return false;
		}
		else if(modMatches.size() == 1) {
			const ModMatch & modMatch = modMatches[0];

			spdlog::info("Selected {} from search query: '{}'.", Utilities::toCapitalCase(magic_enum::enum_name(modMatch.getMatchType())), modMatch.toString());

			setSelectedMod(modMatch.getMod());
			setSelectedModVersionIndex(modMatch.getModVersionIndex());
			setSelectedModVersionTypeIndex(modMatch.getModVersionTypeIndex());

			if(args->hasArgument("start")) {
				m_shouldRunSelectedMod = true;
			}

			return true;
		}
		else {
			if(modMatches.size() > 20) {
				spdlog::info("Found {} matches, please refine your search query.", modMatches.size());
			}
			else {
				std::stringstream matchesStringStream;

				matchesStringStream << fmt::format("Found {} matches:", modMatches.size()) << std::endl;

				for(size_t i = 0; i < modMatches.size(); i++) {
					size_t spacingLength = Utilities::unsignedLongLength(modMatches.size()) - Utilities::unsignedLongLength(i + 1);

					for(size_t i = 0; i < spacingLength; i++) {
						matchesStringStream << ' ';
					}

					matchesStringStream << fmt::format("{}. {}", i + 1, modMatches[i].toString()) << std::endl;
				}

				matchesStringStream << std::endl;
				matchesStringStream << "Please refine your search query.";
			}

			return false;
		}
	}
	else if(args->hasArgument("random")) {
		selectRandomMod(true, true);

		spdlog::info("Selected random mod: '{}'", m_selectedMod->getFullName(m_selectedModVersionIndex, m_selectedModVersionTypeIndex));

		if(args->hasArgument("start")) {
			m_shouldRunSelectedMod = true;
		}

		return true;
	}
	else if(args->hasArgument("start")) {
		m_shouldRunSelectedMod = true;

		return true;
	}

	return true;
}

size_t ModManager::checkForUnlinkedModFiles() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return 0;
	}

	size_t numberOfUnlinkedModFiles = 0;

	for(size_t i = 0; i < getGameVersions()->numberOfGameVersions(); i++) {
		numberOfUnlinkedModFiles += checkForUnlinkedModFilesForGameVersion(*getGameVersions()->getGameVersion(i));
	}

	return numberOfUnlinkedModFiles;
}

size_t ModManager::checkForUnlinkedModFilesForGameVersion(const GameVersion & gameVersion) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized || !gameVersion.isValid()) {
		return 0;
	}

	SettingsManager * settings = SettingsManager::getInstance();

	std::filesystem::path gameModsPath(std::filesystem::path(Utilities::joinPaths(settings->modsDirectoryPath, gameVersion.getModDirectoryName())));

	if(!std::filesystem::is_directory(gameModsPath)) {
		return 0;
	}

	std::map<std::string, std::vector<std::shared_ptr<ModFile>>> linkedModFiles;

	for(const std::filesystem::directory_entry & e : std::filesystem::directory_iterator(gameModsPath)) {
		if(e.is_regular_file()) {
			linkedModFiles[std::string(Utilities::getFileName(e.path().string()))] = std::vector<std::shared_ptr<ModFile>>();
		}
	}

	std::shared_ptr<Mod> mod;
	std::shared_ptr<ModVersion> modVersion;
	std::shared_ptr<ModVersionType> modVersionType;
	std::shared_ptr<ModGameVersion> modGameVersion;
	std::shared_ptr<ModFile> modFile;
	std::string fileName;

	for(size_t i = 0; i < m_mods->numberOfMods(); i++) {
		mod = m_mods->getMod(i);

		for(size_t j = 0; j < mod->numberOfVersions(); j++) {
			modVersion = mod->getVersion(j);

			for(size_t k = 0; k < modVersion->numberOfTypes(); k++) {
				modVersionType = modVersion->getType(k);
				modGameVersion = modVersionType->getGameVersionWithID(gameVersion.getID());

				if(modGameVersion == nullptr) {
					continue;
				}

				for(size_t l = 0; l < modGameVersion->numberOfFiles(); l++) {
					modFile = modGameVersion->getFile(l);
					fileName = modFile->getFileName();

					if(linkedModFiles.find(fileName) != linkedModFiles.end() && linkedModFiles[fileName].size() != 0) {
						if(gameVersion.areScriptFilesReadFromGroup() && modFile->getType() != "zip" && modFile->getType() != "grp") {
							continue;
						}
					}

					linkedModFiles[fileName].push_back(modFile);
				}
			}
		}
	}

	size_t numberOfUnlinkedModFiles = 0;

	for(std::map<std::string, std::vector<std::shared_ptr<ModFile>>>::const_iterator i = linkedModFiles.begin(); i != linkedModFiles.end(); ++i) {
		if(i->second.size() == 0) {
			numberOfUnlinkedModFiles++;

			spdlog::warn("Unlinked '{}' file {}: '{}' for '{}'.", gameVersion.getLongName(), numberOfUnlinkedModFiles, i->first, gameVersion.getLongName());
		}
	}

	if(numberOfUnlinkedModFiles != 0) {
		spdlog::warn("Found {} unlinked '{}' mod file{} in '{}' mods directory.", numberOfUnlinkedModFiles, gameVersion.getLongName(), numberOfUnlinkedModFiles == 1 ? "" : "s", gameVersion.getLongName());
	}

	bool multipleLinkedFile = false;
	size_t numberOfMultipleLinkedModFiles = 0;

	for(std::map<std::string, std::vector<std::shared_ptr<ModFile>>>::const_iterator i = linkedModFiles.begin(); i != linkedModFiles.end(); ++i) {
		if(i->second.size() > 1) {
			for(std::vector<std::shared_ptr<ModFile>>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
				multipleLinkedFile = false;

				for(std::vector<std::shared_ptr<ModFile>>::const_iterator k = j + 1; k != i->second.end(); ++k) {
					if((*j)->getParentMod() != (*k)->getParentMod()) {
						if(!(*j)->getParentModVersionType()->isDependencyOf(*(*k)->getParentModVersionType(), m_mods.get(), true) &&
						   !(*k)->getParentModVersionType()->isDependencyOf(*(*j)->getParentModVersionType(), m_mods.get(), true)) {
							multipleLinkedFile = true;

							break;
						}
					}
					else if((*j)->getParentModVersion() != (*k)->getParentModVersion()) {
						if(!(*j)->isShared() || !(*k)->isShared()) {
							multipleLinkedFile = true;

							break;
						}
					}
				}

				if(multipleLinkedFile) {
					spdlog::warn("'{}' Mod file '{}' linked {} times.", gameVersion.getLongName(), i->first, i->second.size());

					numberOfMultipleLinkedModFiles++;
				}
			}
		}
	}

	if(numberOfMultipleLinkedModFiles != 0) {
		spdlog::warn("Found {} multiple linked '{}' mod file{} in '{}' mods directory. If a mod file is linked intentionally multiple times within the same game version, it must have its shared property set to true.", numberOfMultipleLinkedModFiles, gameVersion.getLongName(), numberOfMultipleLinkedModFiles == 1 ? "" : "s", gameVersion.getLongName());
	}

	return numberOfUnlinkedModFiles;
}

size_t ModManager::checkModForMissingFiles(const std::string & modID, std::optional<size_t> versionIndex, std::optional<size_t> versionTypeIndex) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return 0;
	}

	std::shared_ptr<Mod> mod = m_mods->getModWithID(modID);

	if(mod == nullptr) {
		return 0;
	}

	return checkModForMissingFiles(*mod, versionIndex, versionTypeIndex);
}

size_t ModManager::checkModForMissingFiles(const Mod & mod, std::optional<size_t> versionIndex, std::optional<size_t> versionTypeIndex) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized ||
	   mod.numberOfVersions() == 0 ||
	   (versionIndex >= mod.numberOfVersions() && versionIndex != std::numeric_limits<size_t>::max())) {
		return 0;
	}

	SettingsManager * settings = SettingsManager::getInstance();

	size_t numberOfMissingFiles = 0;
	std::shared_ptr<ModVersion> modVersion;
	std::shared_ptr<ModVersionType> modVersionType;
	std::shared_ptr<ModGameVersion> modGameVersion;
	std::shared_ptr<ModFile> modFile;
	std::shared_ptr<GameVersion> gameVersion;
	std::string modFilePath;
	std::string gameModsPath;

	for(size_t i = (versionIndex.has_value() ? versionIndex.value() : 0); i < (versionIndex.has_value() ? versionIndex.value() + 1 : mod.numberOfVersions()); i++) {
		if(i >= mod.numberOfVersions()) {
			break;
		}

		modVersion = mod.getVersion(i);

		for(size_t j = (versionTypeIndex.has_value() ? versionTypeIndex.value() : 0); j < (versionTypeIndex.has_value() ? versionTypeIndex.value() + 1 : modVersion->numberOfTypes()); j++) {
			if(j >= modVersion->numberOfTypes()) {
				break;
			}

			modVersionType = modVersion->getType(j);

			for(size_t k = 0; k < modVersionType->numberOfGameVersions(); k++) {
				modGameVersion = modVersionType->getGameVersion(k);

				if(modGameVersion->isStandAlone()) {
					continue;
				}

				gameVersion = getGameVersions()->getGameVersionWithID(modGameVersion->getGameVersionID());

				if(!GameVersion::isValid(gameVersion.get())) {
					spdlog::warn("Skipping checking invalid '{}' mod game version '{}', invalid game configuration.", mod.getFullName(i, j), getGameVersions()->getLongNameOfGameVersionWithID(modGameVersion->getGameVersionID()));
					continue;
				}

				gameModsPath = Utilities::joinPaths(settings->modsDirectoryPath, gameVersion->getModDirectoryName());

				if(!std::filesystem::is_directory(gameModsPath)) {
					spdlog::warn("Skipping checking '{}' mod game version '{}', base directory is missing or not a valid directory: '{}'.", mod.getFullName(i, j), gameVersion->getLongName(), gameModsPath);
					continue;
				}

				for(size_t l = 0; l < modGameVersion->numberOfFiles(); l++) {
					modFile = modGameVersion->getFile(l);
					modFilePath = Utilities::joinPaths(gameModsPath, modFile->getFileName());

					if(!std::filesystem::is_regular_file(std::filesystem::path(modFilePath))) {
						if(gameVersion->areScriptFilesReadFromGroup() && modFile->getType() != "zip" && modFile->getType() != "grp") {
							continue;
						}

						spdlog::warn("Mod '{}' is missing {} {} file: '{}'.", mod.getFullName(i, j), getGameVersions()->getLongNameOfGameVersionWithID(modGameVersion->getGameVersionID()), modFile->getType(), modFile->getFileName());

						numberOfMissingFiles++;
					}
				}
			}
		}
	}

	return numberOfMissingFiles;
}

size_t ModManager::checkAllModsForMissingFiles() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return 0;
	}

	size_t numberOfMissingModFiles = 0;

	for(size_t i = 0; i < m_mods->numberOfMods(); i++) {
		numberOfMissingModFiles += checkModForMissingFiles(*m_mods->getMod(i));
	}

	if(numberOfMissingModFiles != 0) {
		spdlog::warn("Found {} missing mod file{} in mods directory.", numberOfMissingModFiles, numberOfMissingModFiles == 1 ? "" : "s");
	}

	return numberOfMissingModFiles;
}

size_t ModManager::checkForMissingExecutables() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	size_t numberOfMissingExecutables = 0;

	numberOfMissingExecutables += getDOSBoxVersions()->checkForMissingExecutables();
	numberOfMissingExecutables += getGameVersions()->checkForMissingExecutables();
	numberOfMissingExecutables += m_standAloneMods->checkForMissingExecutables();

	return numberOfMissingExecutables++;
}

size_t ModManager::updateFileInfoForAllMods(bool save, bool skipPopulatedFiles) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized || !m_localMode) {
		return 0;
	}

	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->modPackageDownloadsDirectoryPath.empty()) {
		spdlog::warn("Mod package downloads directory path not set, cannot update info for some download files!");
	}

	if(settings->modSourceFilesDirectoryPath.empty()) {
		spdlog::warn("Mod source files directory path not set, cannot update file info for some download files!");
	}

	if(settings->modImagesDirectoryPath.empty()) {
		spdlog::warn("Mod images directory path not set, cannot update screenshot or image file info!");
	}

	spdlog::info("Updating info for {} files...", skipPopulatedFiles ? "new" : "all");

	size_t numberOfFilesUpdated = 0;

	for(size_t i = 0; i < m_mods->numberOfMods(); i++) {
		numberOfFilesUpdated += updateModFileInfo(*m_mods->getMod(i), skipPopulatedFiles);
	}

	if(numberOfFilesUpdated != 0) {
		spdlog::info("Updated info for {} mod file{}.", numberOfFilesUpdated, numberOfFilesUpdated == 1 ? "" : "s");
	}
	else {
		spdlog::info("No file info updated.");
	}

	if(save) {
		if(m_mods->saveTo(settings->modsListFilePath, true)) {
			spdlog::info("Saved updated mod list to file: '{}'.", settings->modsListFilePath);
		}
		else {
			spdlog::error("Failed to save updated mod list to file: '{}'!", settings->modsListFilePath);
		}
	}

	return numberOfFilesUpdated;
}

size_t ModManager::updateModFileInfo(Mod & mod, bool skipPopulatedFiles, std::optional<size_t> versionIndex, std::optional<size_t> versionTypeIndex) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized ||
	   !m_localMode ||
	   mod.numberOfVersions() == 0 ||
	   (versionIndex >= mod.numberOfVersions() && versionIndex != std::numeric_limits<size_t>::max())) {
		return 0;
	}

	if(skipPopulatedFiles && mod.isValid()) {
		spdlog::info("Mod #{}/{} ('{}') file info already populated!", m_mods->indexOfMod(mod) + 1, m_mods->numberOfMods(), mod.getName());
		return 0;
	}

	SettingsManager * settings = SettingsManager::getInstance();

	size_t numberOfFilesUpdated = 0;
	std::string downloadFilePath;
	std::string screenshotFilePath;
	std::string imageFilePath;
	std::string modFilePath;
	std::string gameModsPath;
	std::string groupFilePath;
	std::unique_ptr<Group> group;
	std::shared_ptr<GroupFile> groupFile;
	std::string zipArchiveFilePath;
	std::unique_ptr<ZipArchive> zipArchive;

	spdlog::info("Updating mod #{}/{} ('{}') file info...", m_mods->indexOfMod(mod) + 1, m_mods->numberOfMods(), mod.getName());

	// update mod downloads file info
	for(size_t i = 0; i < mod.numberOfDownloads(); i++) {
		std::shared_ptr<ModDownload> modDownload(mod.getDownload(i));

		if(!ModDownload::isValid(modDownload.get(), true)) {
			spdlog::warn("Skipping info update of invalid download file #{} for mod '{}'.", i + 1, mod.getName());
			continue;
		}

		if(skipPopulatedFiles && !modDownload->getSHA1().empty() && modDownload->getFileSize() != 0) {
			continue;
		}

		if(modDownload->isModManagerFiles()) {
			if(settings->modPackageDownloadsDirectoryPath.empty()) {
				continue;
			}

			std::shared_ptr<GameVersion> gameVersion(getGameVersions()->getGameVersionWithID(modDownload->getGameVersionID()));

			if(gameVersion == nullptr) {
				spdlog::warn("Could not find game configuration for game version '{}', skipping update of download file info: '{}'.", getGameVersions()->getLongNameOfGameVersionWithID(modDownload->getGameVersionID()), modDownload->getFileName());
				continue;
			}

			downloadFilePath = Utilities::joinPaths(settings->modPackageDownloadsDirectoryPath, Utilities::toLowerCase(gameVersion->getModDirectoryName()), modDownload->getFileName());
		}
		else {
			if(settings->modSourceFilesDirectoryPath.empty()) {
				continue;
			}

			std::string downloadFileBasePath(Utilities::joinPaths(settings->modSourceFilesDirectoryPath, Utilities::getSafeDirectoryName(mod.getName())));

			downloadFilePath = Utilities::joinPaths(downloadFileBasePath, Utilities::getSafeDirectoryName(modDownload->getVersion()), modDownload->getFileName());

			if(modDownload->getVersion().empty() && !std::filesystem::is_regular_file(std::filesystem::path(downloadFilePath))) {
				downloadFilePath = Utilities::joinPaths(downloadFileBasePath, "Full", modDownload->getFileName());
			}

			if(!modDownload->getVersionType().empty() && !std::filesystem::is_regular_file(std::filesystem::path(downloadFilePath))) {
				downloadFilePath = Utilities::joinPaths(downloadFileBasePath, modDownload->getVersionType(), modDownload->getFileName());
			}

			if(!modDownload->getSpecial().empty() && !std::filesystem::is_regular_file(std::filesystem::path(downloadFilePath))) {
				downloadFilePath = Utilities::joinPaths(downloadFileBasePath, modDownload->getSpecial(), modDownload->getFileName());
			}
		}

		if(!std::filesystem::is_regular_file(std::filesystem::path(downloadFilePath))) {
			spdlog::warn("Skipping update of missing '{}' mod download file info: '{}'.", mod.getName(), downloadFilePath);
			continue;
		}

		bool fileSHA1Updated = false;

		if(!skipPopulatedFiles || modDownload->getSHA1().empty()) {
			std::string fileSHA1(Utilities::getFileSHA1Hash(downloadFilePath));

			if(fileSHA1.empty()) {
				spdlog::error("Failed to hash mod '{}' download file '{}'.", mod.getName(), modDownload->getFileName());
				continue;
			}

			if(modDownload->getSHA1() != fileSHA1) {
				spdlog::info("Updating mod '{}' download file '{}' SHA1 hash from '{}' to '{}'.", mod.getName(), modDownload->getFileName(), modDownload->getSHA1(), fileSHA1);

				modDownload->setSHA1(fileSHA1);

				fileSHA1Updated = true;
			}
		}

		bool fileSizeUpdated = false;

		if(!skipPopulatedFiles || modDownload->getFileSize() == 0) {
			std::error_code errorCode;
			uint64_t fileSize = std::filesystem::file_size(std::filesystem::path(downloadFilePath), errorCode);

			if(errorCode) {
				spdlog::error("Failed to obtain mod '{}' download file '{}' size: {}", mod.getName(), modDownload->getFileName(), errorCode.message());
				continue;
			}

			if(modDownload->getFileSize() != fileSize) {
				spdlog::info("Updating mod '{}' download file '{}' size from {} to {} bytes.", mod.getName(), modDownload->getFileName(), modDownload->getFileSize(), fileSize);

				modDownload->setFileSize(fileSize);

				fileSizeUpdated = true;
			}
		}

		if(fileSHA1Updated || fileSizeUpdated) {
			numberOfFilesUpdated++;
		}
	}

	// update mod screenshots file info
	if(!settings->modImagesDirectoryPath.empty()) {
		for(size_t i = 0; i < mod.numberOfScreenshots(); i++) {
			std::shared_ptr<ModScreenshot> modScreenshot(mod.getScreenshot(i));

			if(!ModScreenshot::isValid(modScreenshot.get(), true)) {
				spdlog::warn("Skipping info update of invalid screenshot file #{} for mod '{}'.", i + 1, mod.getName());
				continue;
			}

			if(skipPopulatedFiles && !modScreenshot->getSHA1().empty() && modScreenshot->getFileSize() != 0) {
				continue;
			}

			screenshotFilePath = Utilities::joinPaths(settings->modImagesDirectoryPath, mod.getID(), "screenshots", "lg", modScreenshot->getFileName());

			if(!std::filesystem::is_regular_file(std::filesystem::path(screenshotFilePath))) {
				spdlog::warn("Skipping update of missing '{}' mod screenshot file info: '{}'.", mod.getName(), screenshotFilePath);
				continue;
			}

			bool fileSHA1Updated = false;

			if(!skipPopulatedFiles || modScreenshot->getSHA1().empty()) {
				std::string fileSHA1(Utilities::getFileSHA1Hash(screenshotFilePath));

				if(fileSHA1.empty()) {
					spdlog::error("Failed to hash mod '{}' screenshot file '{}'.", mod.getName(), modScreenshot->getFileName());
					continue;
				}

				if(modScreenshot->getSHA1() != fileSHA1) {
					spdlog::info("Updating mod '{}' screenshot file '{}' SHA1 hash from '{}' to '{}'.", mod.getName(), modScreenshot->getFileName(), modScreenshot->getSHA1(), fileSHA1);

					modScreenshot->setSHA1(fileSHA1);

					fileSHA1Updated = true;
				}
			}

			bool fileSizeUpdated = false;

			if(!skipPopulatedFiles || modScreenshot->getFileSize() == 0) {
				std::error_code errorCode;
				uint64_t fileSize = std::filesystem::file_size(std::filesystem::path(screenshotFilePath), errorCode);

				if(errorCode) {
					spdlog::error("Failed to obtain mod '{}' screenshot file '{}' size: {}", mod.getName(), modScreenshot->getFileName(), errorCode.message());
					continue;
				}

				if(modScreenshot->getFileSize() != fileSize) {
					spdlog::info("Updating mod '{}' screenshot file '{}' size from {} to {} bytes.", mod.getName(), modScreenshot->getFileName(), modScreenshot->getFileSize(), fileSize);

					modScreenshot->setFileSize(fileSize);

					fileSizeUpdated = true;
				}
			}

			if(fileSHA1Updated || fileSizeUpdated) {
				numberOfFilesUpdated++;
			}
		}
	}

	// update mod images file info
	if(!settings->modImagesDirectoryPath.empty()) {
		for(size_t i = 0; i < mod.numberOfImages(); i++) {
			std::shared_ptr<ModImage> modImage(mod.getImage(i));

			if(!ModImage::isValid(modImage.get(), true)) {
				spdlog::warn("Skipping info update of invalid image file #{} for mod '{}'.", i + 1, mod.getName());
				continue;
			}

			if(skipPopulatedFiles && !modImage->getSHA1().empty() && modImage->getFileSize() != 0) {
				continue;
			}

			imageFilePath = Utilities::joinPaths(settings->modImagesDirectoryPath, mod.getID());

			if(!modImage->getSubfolder().empty()) {
				imageFilePath = Utilities::joinPaths(imageFilePath, modImage->getSubfolder());
			}

			imageFilePath = Utilities::joinPaths(imageFilePath, modImage->getFileName());

			if(!std::filesystem::is_regular_file(std::filesystem::path(imageFilePath))) {
				spdlog::warn("Skipping update of missing '{}' mod image file info: '{}'.", mod.getName(), imageFilePath);
				continue;
			}

			bool fileSHA1Updated = false;

			if(!skipPopulatedFiles || modImage->getSHA1().empty()) {
				std::string fileSHA1(Utilities::getFileSHA1Hash(imageFilePath));

				if(fileSHA1.empty()) {
					spdlog::error("Failed to hash mod '{}' image file '{}'.", mod.getName(), modImage->getFileName());
					continue;
				}

				if(modImage->getSHA1() != fileSHA1) {
					spdlog::info("Updating mod '{}' image file '{}' SHA1 hash from '{}' to '{}'.", mod.getName(), modImage->getFileName(), modImage->getSHA1(), fileSHA1);

					modImage->setSHA1(fileSHA1);

					fileSHA1Updated = true;
				}
			}

			bool fileSizeUpdated = false;

			if(!skipPopulatedFiles || modImage->getFileSize() == 0) {
				std::error_code errorCode;
				uint64_t fileSize = std::filesystem::file_size(std::filesystem::path(imageFilePath), errorCode);

				if(errorCode) {
					spdlog::error("Failed to obtain mod '{}' image file '{}' size: {}", mod.getName(), modImage->getFileName(), errorCode.message());
					continue;
				}

				if(modImage->getFileSize() != fileSize) {
					spdlog::info("Updating mod '{}' image file '{}' size from {} to {} bytes.", mod.getName(), modImage->getFileName(), modImage->getFileSize(), fileSize);

					modImage->setFileSize(fileSize);

					fileSizeUpdated = true;
				}
			}

			if(fileSHA1Updated || fileSizeUpdated) {
				numberOfFilesUpdated++;
			}
		}
	}

	// update mod files info
	for(size_t i = (versionIndex.has_value() ? versionIndex.value() : 0); i < (versionIndex.has_value() ? versionIndex.value() + 1 : mod.numberOfVersions()); i++) {
		if(i >= mod.numberOfVersions()) {
			break;
		}

		std::shared_ptr<ModVersion> modVersion(mod.getVersion(i));

		for(size_t j = (versionTypeIndex.has_value() ? versionTypeIndex.value() : 0); j < (versionTypeIndex.has_value() ? versionTypeIndex.value() + 1 : modVersion->numberOfTypes()); j++) {
			if(j >= modVersion->numberOfTypes()) {
				break;
			}

			std::shared_ptr<ModVersionType> modVersionType(modVersion->getType(j));

			for(size_t k = 0; k < modVersionType->numberOfGameVersions(); k++) {
				std::shared_ptr<ModGameVersion> modGameVersion(modVersionType->getGameVersion(k));
				std::shared_ptr<GameVersion> gameVersion(getGameVersions()->getGameVersionWithID(modGameVersion->getGameVersionID()));

				if(!GameVersion::isValid(gameVersion.get())) {
					spdlog::warn("Mod '{}' game version #{} is not valid, skipping update of mod files info.", mod.getFullName(i, j), k + 1);
					continue;
				}

				gameModsPath = Utilities::joinPaths(settings->modsDirectoryPath, gameVersion->getModDirectoryName());

				if(!std::filesystem::is_directory(gameModsPath)) {
					spdlog::warn("Mod '{}' '{}' game version directory '{}' does not exist or is not a valid directory, skipping update of mod files info.", mod.getFullName(i, j), gameVersion->getLongName(), gameModsPath);
					continue;
				}

				if(gameVersion->areZipArchiveGroupsSupported()) {
					std::shared_ptr<ModFile> modZipFile(modGameVersion->getFirstFileOfType("zip"));

					if(modZipFile != nullptr) {
						zipArchiveFilePath = Utilities::joinPaths(gameModsPath, modZipFile->getFileName());
						zipArchive = ZipArchive::readFrom(zipArchiveFilePath, Utilities::emptyString, true);

						if(zipArchive != nullptr) {
							spdlog::info("Opened '{}' zip file '{}'.", modVersionType->getFullName(), zipArchiveFilePath);
						}
					}
					else {
						std::shared_ptr<ModFile> modGroupFile(modGameVersion->getFirstFileOfType("grp"));

						if(modGroupFile != nullptr) {
							groupFilePath = Utilities::joinPaths(gameModsPath, modGroupFile->getFileName());

							group = GroupGRP::loadFrom(groupFilePath);

							if(group == nullptr) {
								spdlog::error("Failed to open mod group file '{}'.", groupFilePath);
							}
						}
					}
				}

				for(size_t l = 0; l < modGameVersion->numberOfFiles(); l++) {
					std::shared_ptr<ModFile> modFile(modGameVersion->getFile(l));

					if(skipPopulatedFiles && !modFile->getSHA1().empty() && modFile->getFileSize() != 0) {
						continue;
					}

					std::string fileSHA1;
					uint64_t fileSize = 0;

					if(gameVersion->areScriptFilesReadFromGroup() && modFile->getType() != "zip" && modFile->getType() != "grp") {
						if(!zipArchiveFilePath.empty()) {
							if(zipArchive == nullptr) {
								spdlog::error("Skipping update of mod file '{}' info since zip archive could not be opened.", zipArchiveFilePath);
								continue;
							}

							std::shared_ptr<ArchiveEntry> zipArchiveEntry(zipArchive->getEntry(modFile->getFileName(), false));

							if(zipArchiveEntry == nullptr) {
								spdlog::error("Mod file '{}' not found in zip file '{}'.", modFile->getFileName(), zipArchiveFilePath);
								continue;
							}

							if(!zipArchiveEntry->isFile()) {
								spdlog::error("Mod file '{}' located in zip file '{}' is not a file.", modFile->getFileName(), zipArchiveFilePath);
								continue;
							}

							if(!skipPopulatedFiles || modFile->getSHA1().empty()) {
								std::unique_ptr<ByteBuffer> zipArchiveEntryData(zipArchiveEntry->getData());

								if(zipArchiveEntryData == nullptr) {
									spdlog::error("Failed to read zip entry '{}' from zip file '{}' into memory.", zipArchiveEntry->getName(), zipArchiveFilePath);
									continue;
								}

								fileSHA1 = zipArchiveEntryData->getSHA1();
							}

							if(!skipPopulatedFiles || modFile->getFileSize() == 0) {
								fileSize = zipArchiveEntry->getUncompressedSize();

								if(fileSize == 0) {
									fileSize = zipArchiveEntry->getData()->getSize();
								}
							}
						}
						else if(!groupFilePath.empty()) {
							if(group == nullptr) {
								spdlog::error("Skipping update of mod file '{}' info since group could not be opened.", groupFilePath);
								continue;
							}

							groupFile = group->getFileWithName(modFile->getFileName());

							if(groupFile == nullptr) {
								spdlog::error("Mod file '{}' not found in group file '{}'.", modFile->getFileName(), groupFilePath);
								continue;
							}

							if(!skipPopulatedFiles || modFile->getSHA1().empty()) {
								fileSHA1 = groupFile->getData().getSHA1();
							}

							if(!skipPopulatedFiles || modFile->getFileSize() == 0) {
								fileSize = groupFile->getSize();
							}
						}
					}
					else {
						modFilePath = Utilities::joinPaths(gameModsPath, modFile->getFileName());

						if(!std::filesystem::is_regular_file(std::filesystem::path(modFilePath))) {
							spdlog::warn("Skipping update of missing '{}' mod file info: '{}'.", mod.getFullName(i, j), modFilePath);
							continue;
						}

						if(!skipPopulatedFiles || modFile->getSHA1().empty()) {
							fileSHA1 = Utilities::getFileSHA1Hash(modFilePath);
						}

						if(!skipPopulatedFiles || modFile->getFileSize() == 0) {
							std::error_code errorCode;
							fileSize = std::filesystem::file_size(std::filesystem::path(modFilePath), errorCode);

							if(errorCode) {
								spdlog::error("Failed to obtain '{}' mod file '{}' size: {}", mod.getName(), modFile->getFileName(), errorCode.message());
								continue;
							}
						}
					}

					bool fileSHA1Updated = false;
					bool fileSizeUpdated = false;

					if(!skipPopulatedFiles || modFile->getSHA1().empty()) {
						if(!fileSHA1.empty()) {
							if(modFile->getSHA1() != fileSHA1) {
								spdlog::info("Updating '{}' mod file '{}' SHA1 hash from '{}' to '{}'.", mod.getFullName(i, j), modFile->getFileName(), modFile->getSHA1(), fileSHA1);

								modFile->setSHA1(fileSHA1);

								fileSHA1Updated = true;
							}
						}
						else {
							spdlog::error("Failed to hash '{}' mod file '{}'.", mod.getFullName(i, j), modFile->getFileName());
						}
					}

					if(!skipPopulatedFiles || modFile->getFileSize() == 0) {
						if(fileSize != 0) {
							if(modFile->getFileSize() != fileSize) {
								spdlog::info("Updating '{}' mod file '{}' size from {} to {} bytes.", mod.getFullName(i, j), modFile->getFileName(), modFile->getFileSize(), fileSize);

								modFile->setFileSize(fileSize);

								fileSizeUpdated = true;
							}
						}
						else {
							spdlog::error("Failed to obtain '{}' mod file '{}' size.", mod.getFullName(i, j), modFile->getFileName());
						}
					}

					if(fileSHA1Updated || fileSizeUpdated) {
						numberOfFilesUpdated++;
					}
				}

				if(zipArchive != nullptr) {
					zipArchive.reset();
				}
				else if(group != nullptr) {
					group.reset();
				}
			}
		}
	}

	return numberOfFilesUpdated;
}

bool ModManager::testParsing() {
	std::string_view modListFilePath(SettingsManager::getInstance()->modsListFilePath);

	if(!std::filesystem::is_regular_file(std::filesystem::path(modListFilePath))) {
		spdlog::error("Local mod list file does not exist!");
		return false;
	}

	std::ifstream fileStream(modListFilePath, std::ios::binary | std::ios::ate);

	if(!fileStream.is_open()) {
		spdlog::error("Failed to read local mod list file data!");
		return false;
	}

	size_t size = fileStream.tellg();

	std::string modCollectionXMLData(size, '\0');

	fileStream.seekg(0, std::ios::beg);
	fileStream.read(modCollectionXMLData.data(), size);
	fileStream.close();

	tinyxml2::XMLDocument modCollectionXMLDocument;

	if(modCollectionXMLDocument.Parse(modCollectionXMLData.data(), modCollectionXMLData.size()) != tinyxml2::XML_SUCCESS) {
		spdlog::error("Failed to parse XML mod list file data!");
		return nullptr;
	}

	std::unique_ptr<ModCollection> xmlModCollection(ModCollection::parseFrom(modCollectionXMLDocument.RootElement()));

	if(!ModCollection::isValid(xmlModCollection.get())) {
		spdlog::error("Failed to parse mod collection from XML file: '{}'.", modListFilePath);
		return false;
	}

	tinyxml2::XMLDocument xmlModsDocument;
	xmlModsDocument.InsertFirstChild(xmlModsDocument.NewDeclaration());
	xmlModsDocument.InsertEndChild(xmlModCollection->toXML(&xmlModsDocument));
	std::string parsedXMLModCollectionString(Utilities::documentToString(&xmlModsDocument));

	if(Utilities::areStringsEqual(modCollectionXMLData, parsedXMLModCollectionString, true, true)) {
		spdlog::info("Original and parsed mod collection XML data match!");
	}
	else {
		spdlog::warn("Original mod collection XML data & parsed XML data do not match! Writing data to temporary file for comparison.");

		std::ofstream fileStream("Parsed Duke Nukem 3D Mod List.xml", std::ios::binary);

		if(!fileStream.is_open()) {
			return false;
		}

		fileStream.write(parsedXMLModCollectionString.data(), parsedXMLModCollectionString.size());

		fileStream.close();
	}

	parsedXMLModCollectionString = "";

	rapidjson::Document jsonModsDocument(xmlModCollection->toJSON());

	std::unique_ptr<ModCollection> jsonModCollection(ModCollection::parseFrom(jsonModsDocument));

	if(!ModCollection::isValid(jsonModCollection.get())) {
		spdlog::error("Failed to parse mod collection from JSON data.", modListFilePath);
		return false;
	}

	if(!jsonModCollection->copyHiddenPropertiesFrom(*xmlModCollection)) {
		spdlog::warn("Failed to copy hidden properties from original XML mod collection to converted JSON mod collection.");
	}

	xmlModCollection.reset();

	tinyxml2::XMLDocument xmlFromJSONModsDocument;
	xmlFromJSONModsDocument.InsertFirstChild(xmlFromJSONModsDocument.NewDeclaration());
	xmlFromJSONModsDocument.InsertEndChild(jsonModCollection->toXML(&xmlFromJSONModsDocument));
	std::string parsedXMLFromJSONModCollectionString(Utilities::documentToString(&xmlFromJSONModsDocument));

	if(Utilities::areStringsEqual(modCollectionXMLData, parsedXMLFromJSONModCollectionString, true, true)) {
		spdlog::info("Original and parsed from JSON mod collection XML data match!");
	}
	else {
		spdlog::warn("Original mod collection XML data & parsed from JSON XML data do not match! Writing data to temporary files for comparison.");

		std::ofstream xmlFileStream("Parsed from JSON Duke Nukem 3D Mod List.xml", std::ios::binary);

		if(!xmlFileStream.is_open()) {
			return false;
		}

		xmlFileStream.write(parsedXMLFromJSONModCollectionString.data(), parsedXMLFromJSONModCollectionString.size());

		xmlFileStream.close();

		modCollectionXMLData = "";
		parsedXMLFromJSONModCollectionString = "";

		std::string jsonModsData(Utilities::valueToString(jsonModsDocument));

		std::ofstream jsonFileStream("Converted Duke Nukem 3D Mod List.json", std::ios::binary);

		if(!jsonFileStream.is_open()) {
			return false;
		}

		jsonFileStream.write(jsonModsData.data(), jsonModsData.size());

		jsonFileStream.close();
	}

	return true;
}

std::string ModManager::getArgumentHelpInfo() {
	std::stringstream argumentHelpStream;

	argumentHelpStream << APPLICATION_NAME << " version " << APPLICATION_VERSION << " arguments:\n";
	argumentHelpStream << " --file \"Settings.json\" - specifies an alternate settings file to use.\n";
	argumentHelpStream << " -f \"File.json\" - alias for 'file'.\n";
	argumentHelpStream << " --type Game/Setup/Client/Server - specifies game type, default: Game.\n";
	argumentHelpStream << " --dosbox \"dosbox_ece\" - specifies the ID of the DOSBox version to use.\n";
	argumentHelpStream << " --game \"atomic\" - specifies the ID of the game version to run.\n";
	argumentHelpStream << " --ip 127.0.0.1 - specifies host ip address if running in client mode.\n";
	argumentHelpStream << " --port 1337 - specifies server port when running in client or server mode.\n";
	argumentHelpStream << " --group MOD.GRP - manually specifies a group or zip file to use. Can be specified multiple times.\n";
	argumentHelpStream << " -g FILE.GRP - alias for 'group'.\n";
	argumentHelpStream << " --con MOD.CON - manually specifies a game con file to use. Can be specified multiple times.\n";
	argumentHelpStream << " -x FILE.CON - alias for 'con'.\n";
	argumentHelpStream << " --def MOD.DEF - manually specifies a def file to use. Can be specified multiple times.\n";
	argumentHelpStream << " -h FILE.DEF - alias for 'def'.\n";
	argumentHelpStream << " --map _ZOO.MAP - manually specifies a user map file to load.\n";
	argumentHelpStream << " --search \"Full Mod Name\" - searches for and selects the mod with a full or partially matching name, and optional version / type.\n";
	argumentHelpStream << " --random - randomly selects a mod to run.\n";
	argumentHelpStream << " --start - launches the game immediately.\n";
	argumentHelpStream << " --episode # - selects an episode (1-4+).\n";
	argumentHelpStream << " -v # - alias for 'episode'\n";
	argumentHelpStream << " --level # - selects a level (1-11).\n";
	argumentHelpStream << " -l # - alias for 'level'.\n";
	argumentHelpStream << " --skill # - selects a skill level (1-4).\n";
	argumentHelpStream << " -s # - alias for 'skill'.\n";
	argumentHelpStream << " --record - enables demo recording.\n";
	argumentHelpStream << " -r - alias for 'record'\n";
	argumentHelpStream << " --demo DEMO3.DMO - plays back the specified demo file.\n";
	argumentHelpStream << " -d FILE.DMO - alias for 'demo'\n";
	argumentHelpStream << " --respawn # - respawn mode: 1 = monsters, 2 = items, 3 = inventory, x = all.\n";
	argumentHelpStream << " -t # - alias for 'respawn'.\n";
	argumentHelpStream << " --weaponswitch 8675309241 - set preferred weapon switch order, as a string of 10 digits.\n";
	argumentHelpStream << " -u 1234567890 - alias for 'switch'.\n";
	argumentHelpStream << " --nomonsters - disable monsters.\n";
	argumentHelpStream << " -m - alias for 'nomonsters'.\n";
	argumentHelpStream << " --nosound - disable sound.\n";
	argumentHelpStream << " --ns - alias for 'nosound'.\n";
	argumentHelpStream << " --nomusic - disable music.\n";
	argumentHelpStream << " --nm - alias for 'nomusic'.\n";
	argumentHelpStream << " --local - runs the mod manager in local mode.\n";
	argumentHelpStream << " -- <args> - specify arguments to pass through to the target game executable when executing.\n";
	argumentHelpStream << " --help - displays this help message.\n";
	argumentHelpStream << " -? - alias for 'help'.\n";

	return argumentHelpStream.str();
}

bool ModManager::createApplicationTemporaryDirectory() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->appTempDirectoryPath.empty()) {
		spdlog::error("Missing application temporary directory path setting.");
		return false;
	}

	std::error_code errorCode;
	std::filesystem::path tempDirectoryPath(settings->appTempDirectoryPath);

	if(!std::filesystem::is_directory(tempDirectoryPath)) {
		std::filesystem::create_directories(tempDirectoryPath, errorCode);

		if(errorCode) {
			spdlog::error("Failed to create application temporary directory structure '{}': {}", tempDirectoryPath.string(), errorCode.message());
			return false;
		}

		spdlog::debug("Created application temporary directory structure: '{}'.", tempDirectoryPath.string());
	}

	return true;
}

void ModManager::clearApplicationTemporaryDirectory() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->appTempDirectoryPath.empty()) {
		return;
	}

	std::filesystem::path tempDirectoryPath(settings->appTempDirectoryPath);

	if(!std::filesystem::is_directory(tempDirectoryPath)) {
		return;
	}

	for(const std::filesystem::directory_entry & entry : std::filesystem::directory_iterator(tempDirectoryPath)) {
		spdlog::info("Deleting leftover temporary {}: '{}'.", entry.is_directory() ? "directory" : "file", entry.path().string());

		std::error_code errorCode;
		std::filesystem::remove(entry.path(), errorCode);

		if(errorCode) {
			spdlog::error("Failed to delete leftover temporary {} '{}': {}", entry.is_directory() ? "directory" : "file", entry.path().string(), errorCode.message());
		}
	}
}

bool ModManager::createGameTemporaryDirectory(const GameVersion & gameVersion) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	SettingsManager * settings = SettingsManager::getInstance();

	if(!gameVersion.isConfigured()) {
		spdlog::error("Failed to create game temporary directory, provided game version is not configured.");
		return false;
	}

	if(settings->gameTempDirectoryName.empty() || settings->gameTempDirectoryName.find_first_of("/\\") != std::string::npos) {
		spdlog::error("Missing or invalid game temporary directory name, must not be empty and not contain any path separators.");
		return false;
	}

	std::string gameTempDirectoryPath(Utilities::joinPaths(gameVersion.getGamePath(), settings->gameTempDirectoryName));

	if(std::filesystem::is_directory(gameTempDirectoryPath)) {
		return true;
	}

	std::error_code errorCode;
	std::filesystem::create_directories(std::filesystem::path(gameTempDirectoryPath), errorCode);

	if(errorCode) {
		spdlog::error("Failed to create game temporary directory structure '{}': {}", gameTempDirectoryPath, errorCode.message());
		return false;
	}

	spdlog::debug("Created game temporary directory structure: '{}'.", gameTempDirectoryPath);

	return true;
}

bool ModManager::removeGameTemporaryDirectory(const GameVersion & gameVersion) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	SettingsManager * settings = SettingsManager::getInstance();

	if(!gameVersion.isConfigured()) {
		spdlog::error("Failed to create game temporary directory, provided game version is not configured.");
		return false;
	}

	if(settings->gameTempDirectoryName.empty() || settings->gameTempDirectoryName.find_first_of("/\\") != std::string::npos) {
		spdlog::error("Missing or invalid game temporary directory name, must not be empty and not contain any path separators.");
		return false;
	}

	std::string gameTempDirectoryPath(Utilities::joinPaths(gameVersion.getGamePath(), settings->gameTempDirectoryName));

	if(!std::filesystem::is_directory(gameTempDirectoryPath)) {
		return true;
	}

	std::error_code errorCode;
	std::filesystem::remove_all(std::filesystem::path(gameTempDirectoryPath), errorCode);

	if(errorCode) {
		spdlog::error("Failed to remove game temporary directory '{}': {}", gameTempDirectoryPath, errorCode.message());
		return false;
	}

	spdlog::debug("Removed game temporary directory: '{}'.", gameTempDirectoryPath);

	return true;
}

bool ModManager::areSymlinkSettingsValid() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	SettingsManager * settings = SettingsManager::getInstance();

	return !settings->gameSymlinkName.empty() &&
		   !settings->tempSymlinkName.empty() &&
		   !settings->modsSymlinkName.empty() &&
		   !settings->mapsSymlinkName.empty();
}

bool ModManager::createSymlink(const std::string & symlinkTarget, const std::string & symlinkName, const std::string & symlinkDestinationDirectory) {
	if(symlinkName.empty() ||
	   symlinkTarget.empty()) {
		spdlog::error("Failed to create symlink, invalid arguments provided.");
		return false;
	}

	std::filesystem::path symlinkTargetPath(symlinkTarget);

	if(!std::filesystem::is_directory(symlinkTargetPath)) {
		spdlog::error("Failed to create '{}' symlink, target path '{}' does not exist or is not a valid directory.", symlinkName, symlinkTarget);
		return false;
	}

	std::string symlinkDestination(Utilities::joinPaths(symlinkDestinationDirectory.empty() ? std::filesystem::current_path().string() : symlinkDestinationDirectory, symlinkName));
	std::filesystem::path symlinkDestinationPath(symlinkDestination);

	if(std::filesystem::exists(symlinkDestinationPath)) {
		if(!std::filesystem::is_symlink(symlinkDestinationPath)) {
			spdlog::error("Failed to remove existing '{}' symlink, unexpected file system entry type.", symlinkName);
			return false;
		}

		spdlog::info("Removing existing symlink: '{}'.", symlinkDestinationPath.string());

		std::error_code errorCode;
		std::filesystem::remove(symlinkDestinationPath, errorCode);

		if(errorCode) {
			spdlog::error("Failed to remove existing symlink '{}': {}", symlinkDestinationPath.string(), errorCode.message());
			return false;
		}
	}

	spdlog::info("Creating symlink '{}' to target '{}'.", symlinkDestination, symlinkTarget);

	std::error_code errorCode;
	std::filesystem::create_directory_symlink(symlinkTarget, symlinkDestinationPath, errorCode);

	if(errorCode) {
		spdlog::error("Failed to create symlink '{}' to target '{}': {}", symlinkDestination, symlinkTarget, errorCode.message());
		return false;
	}

	return true;
}

bool ModManager::removeSymlink(const std::string & symlinkName, const std::string & symlinkDestinationDirectory) {
	if(symlinkName.empty()) {
		spdlog::error("Failed to remove symlink, invalid arguments provided.");
		return false;
	}

	std::string symlinkDestination(Utilities::joinPaths(symlinkDestinationDirectory.empty() ? std::filesystem::current_path().string() : symlinkDestinationDirectory, symlinkName));
	std::filesystem::path symlinkDestinationPath(symlinkDestination);

	if(std::filesystem::exists(symlinkDestinationPath)) {
		if(!std::filesystem::is_symlink(symlinkDestinationPath)) {
			spdlog::error("Failed to remove '{}' symlink, unexpected file system entry type.");
			return false;
		}

		spdlog::info("Removing symlink: '{}'.", symlinkDestination);

		std::error_code errorCode;
		std::filesystem::remove(symlinkDestinationPath, errorCode);

		if(errorCode) {
			spdlog::error("Failed to remove symlink '{}': {}", symlinkDestination, errorCode.message());
			return false;
		}
	}

	return true;
}

bool ModManager::createSymlinksOrCopyTemporaryFiles(const GameVersion & gameVersion, const std::vector<std::string> & conFilePaths, const std::vector<std::string> & defFilePaths, const std::vector<std::string> & groupFilePaths, std::string_view customMapFilePath, bool doesRequireCombinedGroup, bool createTempSymlink, InstalledModInfo * installedModInfo) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	SettingsManager * settings = SettingsManager::getInstance();

	if(!gameVersion.isConfigured() || !areSymlinkSettingsValid() || (createTempSymlink && settings->appTempDirectoryPath.empty())) {
		return false;
	}

	std::string mapsDirectoryPath(getMapsDirectoryPath());

	if(Utilities::areSymlinksSupported() && gameVersion.doesSupportSubdirectories()) {
		bool result = true;

		result &= createSymlink(gameVersion.getGamePath(), settings->gameSymlinkName, std::filesystem::current_path().string());

		if(createTempSymlink) {
			result &= createSymlink(settings->appTempDirectoryPath, settings->tempSymlinkName, gameVersion.getGamePath());
		}

		result &= createSymlink(std::filesystem::current_path().string(), settings->appSymlinkName, gameVersion.getGamePath());

		result &= createSymlink(getModsDirectoryPath(), settings->modsSymlinkName, gameVersion.getGamePath());

		if(!mapsDirectoryPath.empty()) {
			result &= createSymlink(mapsDirectoryPath, settings->mapsSymlinkName, gameVersion.getGamePath());
		}

		return result;
	}

	if(gameVersion.doesSupportSubdirectories() && !createGameTemporaryDirectory(gameVersion)) {
		return false;
	}

	std::string relativeFileDestinationDirectoryPath;
	std::string absoluteFileDestinationDirectoryPath(gameVersion.getGamePath());

	if(gameVersion.doesSupportSubdirectories()) {
		relativeFileDestinationDirectoryPath = settings->gameTempDirectoryName;
		absoluteFileDestinationDirectoryPath = Utilities::joinPaths(absoluteFileDestinationDirectoryPath, relativeFileDestinationDirectoryPath);
	}

	std::vector<std::string> modFilePaths(conFilePaths);

	for(const std::string & defFilePath : defFilePaths) {
		modFilePaths.push_back(defFilePath);
	}

	if(!doesRequireCombinedGroup) {
		for(const std::string & groupFilePath : groupFilePaths) {
			modFilePaths.push_back(groupFilePath);
		}
	}

	if(!gameVersion.doesRequireGroupFileExtraction()) {
		for(const std::string & modFilePath : modFilePaths) {
			std::string_view modFileFileName(Utilities::getFileName(modFilePath));
			std::string relativeModFileDestinationFilePath(Utilities::joinPaths(relativeFileDestinationDirectoryPath, modFileFileName));
			std::string absoluteModFileDestionationFilePath(Utilities::joinPaths(absoluteFileDestinationDirectoryPath, modFileFileName));

			if(std::filesystem::is_regular_file(std::filesystem::path(absoluteModFileDestionationFilePath))) {
				if(std::filesystem::is_regular_file(std::filesystem::path(absoluteModFileDestionationFilePath + DEFAULT_BACKUP_FILE_RENAME_SUFFIX))) {
					spdlog::error("Cannot temporarily rename original '{}' mod file, original backup file already exists at path: '{}'. Please manually restore or remove this file.", gameVersion.getLongName(), absoluteModFileDestionationFilePath + DEFAULT_BACKUP_FILE_RENAME_SUFFIX);
					return false;
				}

				std::error_code errorCode;
				std::filesystem::rename(std::filesystem::path(absoluteModFileDestionationFilePath), std::filesystem::path(absoluteModFileDestionationFilePath + DEFAULT_BACKUP_FILE_RENAME_SUFFIX), errorCode);

				if(errorCode) {
					spdlog::error("Failed to rename '{}' mod file '{}' to '{}': {}", gameVersion.getLongName(), relativeModFileDestinationFilePath, relativeModFileDestinationFilePath + DEFAULT_BACKUP_FILE_RENAME_SUFFIX, errorCode.message());
					return false;
				}

				spdlog::debug("Renamed '{}' mod file '{}' to '{}'.", gameVersion.getLongName(), relativeModFileDestinationFilePath, relativeModFileDestinationFilePath + DEFAULT_BACKUP_FILE_RENAME_SUFFIX);

				if(installedModInfo != nullptr) {
					installedModInfo->addOriginalFile(relativeModFileDestinationFilePath);
				}
			}

			std::error_code errorCode;
			std::filesystem::copy_file(std::filesystem::path(modFilePath), std::filesystem::path(absoluteModFileDestionationFilePath), errorCode);

			if(errorCode) {
				spdlog::error("Failed to copy mod file '{}' from '{}' to directory '{}': {}", modFileFileName, Utilities::getFilePath(modFilePath), absoluteFileDestinationDirectoryPath, errorCode.message());
				return false;
			}

			if(installedModInfo != nullptr) {
				installedModInfo->addModFile(relativeModFileDestinationFilePath);
			}

			spdlog::debug("Copied mod file '{}' to directory '{}'.", modFileFileName, absoluteFileDestinationDirectoryPath);
		}
	}

	if(!customMapFilePath.empty()) {
		std::string_view customMapFileName(Utilities::getFileName(customMapFilePath));
		std::string relativeCustomMapDestinationFilePath(Utilities::joinPaths(relativeFileDestinationDirectoryPath, customMapFileName));
		std::string absoluteCustomMapDestinationFilePath(Utilities::joinPaths(absoluteFileDestinationDirectoryPath, customMapFileName));

		if(std::filesystem::is_regular_file(std::filesystem::path(absoluteCustomMapDestinationFilePath))) {
			if(std::filesystem::is_regular_file(std::filesystem::path(absoluteCustomMapDestinationFilePath + DEFAULT_BACKUP_FILE_RENAME_SUFFIX))) {
				spdlog::error("Cannot temporarily rename original '{}' map file, original backup file already exists at path: '{}'. Please manually restore or remove this file.", gameVersion.getLongName(), absoluteCustomMapDestinationFilePath + DEFAULT_BACKUP_FILE_RENAME_SUFFIX);
				return false;
			}

			std::error_code errorCode;
			std::filesystem::rename(std::filesystem::path(absoluteCustomMapDestinationFilePath), std::filesystem::path(absoluteCustomMapDestinationFilePath + DEFAULT_BACKUP_FILE_RENAME_SUFFIX), errorCode);

			if(errorCode) {
				spdlog::error("Failed to rename '{}' map file '{}' to '{}': {}", gameVersion.getLongName(), relativeCustomMapDestinationFilePath, relativeCustomMapDestinationFilePath + DEFAULT_BACKUP_FILE_RENAME_SUFFIX, errorCode.message());
				return false;
			}

			spdlog::debug("Renamed '{}' map file '{}' to '{}'.", gameVersion.getLongName(), relativeCustomMapDestinationFilePath, relativeCustomMapDestinationFilePath + DEFAULT_BACKUP_FILE_RENAME_SUFFIX);

			if(installedModInfo != nullptr) {
				installedModInfo->addOriginalFile(relativeCustomMapDestinationFilePath);
			}
		}

		std::error_code errorCode;
		std::filesystem::copy_file(std::filesystem::path(customMapFilePath), std::filesystem::path(absoluteCustomMapDestinationFilePath), errorCode);

		if(errorCode) {
			spdlog::error("Failed to copy map file '{}' to directory '{}': {}", customMapFileName, absoluteFileDestinationDirectoryPath, errorCode.message());
			return false;
		}

		if(installedModInfo != nullptr) {
			installedModInfo->addModFile(relativeCustomMapDestinationFilePath);
		}

		spdlog::debug("Copied map file '{}' to directory '{}'.", customMapFileName, absoluteFileDestinationDirectoryPath);
	}

	return true;
}

bool ModManager::removeSymlinks(const GameVersion & gameVersion) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	SettingsManager * settings = SettingsManager::getInstance();

	if(!gameVersion.isConfigured() || !areSymlinkSettingsValid()) {
		return false;
	}

	if(Utilities::areSymlinksSupported() && gameVersion.doesSupportSubdirectories()) {
		bool result = true;

		result &= removeSymlink(settings->gameSymlinkName, std::filesystem::current_path().string());

		result &= removeSymlink(settings->appSymlinkName, gameVersion.getGamePath());

		result &= removeSymlink(settings->tempSymlinkName, gameVersion.getGamePath());

		result &= removeSymlink(settings->modsSymlinkName, gameVersion.getGamePath());

		std::string mapsDirectoryPath(getMapsDirectoryPath());

		if(!mapsDirectoryPath.empty()) {
			result &= removeSymlink(settings->mapsSymlinkName, gameVersion.getGamePath());
		}

		return result;
	}

	if(gameVersion.doesSupportSubdirectories()) {
		return removeGameTemporaryDirectory(gameVersion);
	}

	return true;
}

std::vector<std::string> ModManager::deleteFilesWithSuffix(const std::string & suffix, const std::string & path) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(suffix.empty()) {
		return {};
	}

	std::filesystem::path directoryPath(path.empty() ? std::filesystem::current_path() : std::filesystem::path(path));
	std::vector<std::string> deletedFilePaths;

	for(const std::filesystem::directory_entry & e : std::filesystem::directory_iterator(directoryPath)) {
		if(e.is_regular_file() && Utilities::areStringsEqualIgnoreCase(Utilities::getFileExtension(e.path().string()), suffix)) {
			spdlog::info("Deleting file: '{}'.", e.path().string());

			std::error_code errorCode;
			std::filesystem::remove(e.path(), errorCode);

			if(errorCode) {
				spdlog::error("Failed to delete file '{}': {}", e.path().string(), errorCode.message());
				continue;
			}

			std::filesystem::path relativeDeletedFilePath(std::filesystem::relative(e.path(), directoryPath, errorCode));

			if(errorCode) {
				spdlog::warn("Failed to relativize deleted file path: '{}' against base path: '{}'.", e.path().string(), directoryPath.string());
				continue;
			}

			deletedFilePaths.push_back(relativeDeletedFilePath.string());
		}
	}

	return deletedFilePaths;
}

std::vector<std::string> ModManager::renameFilesWithSuffixTo(const std::string & fromSuffix, const std::string & toSuffix, const std::string & path) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(fromSuffix.empty() || toSuffix.empty()) {
		return {};
	}

	std::filesystem::path directoryPath(path.empty() ? std::filesystem::current_path() : std::filesystem::path(path));
	std::string newFilePath;
	std::vector<std::string> originalRenamedFilePaths;

	for(const std::filesystem::directory_entry & e : std::filesystem::directory_iterator(directoryPath)) {
		if(e.is_regular_file() && Utilities::areStringsEqualIgnoreCase(Utilities::getFileExtension(e.path().string()), fromSuffix)) {
			newFilePath = Utilities::replaceFileExtension(e.path().string(), toSuffix);

			spdlog::info("Renaming file: '{}' > '{}'.", e.path().string(), newFilePath);

			std::error_code errorCode;
			std::filesystem::rename(e.path(), newFilePath, errorCode);

			if(errorCode) {
				spdlog::error("Failed to rename file '{}' to '{}': {}", e.path().string(), newFilePath, errorCode.message());
				continue;
			}

			std::filesystem::path relativeRenamedFilePath(std::filesystem::relative(e.path(), directoryPath, errorCode));

			if(errorCode) {
				spdlog::warn("Failed to relativize renamed file path: '{}' against base path: '{}'.", e.path().string(), directoryPath.string());
				continue;
			}

			originalRenamedFilePaths.push_back(relativeRenamedFilePath.string());
		}
	}

	return originalRenamedFilePaths;
}

size_t ModManager::createDOSBoxTemplateCommandScriptFiles(bool overwrite) {
	SettingsManager* settings = SettingsManager::getInstance();

	return createDOSBoxTemplateCommandScriptFiles(Utilities::joinPaths(settings->dataDirectoryPath, settings->dosboxDataDirectoryName, settings->dosboxCommandScriptsDirectoryName), overwrite);
}

size_t ModManager::createDOSBoxTemplateCommandScriptFiles(const std::string & directoryPath, bool overwrite) {
	size_t numberOfDOSBoxTemplateScriptFilesCreated = 0;

	for(GameType gameType : magic_enum::enum_values<GameType>()) {
		if(createDOSBoxTemplateCommandScriptFile(gameType, directoryPath, overwrite)) {
			numberOfDOSBoxTemplateScriptFilesCreated++;
		}
	}

	return numberOfDOSBoxTemplateScriptFilesCreated;
}

bool ModManager::createDOSBoxTemplateCommandScriptFile(GameType gameType, const std::string & directoryPath, bool overwrite) {
	if(!directoryPath.empty()) {
		std::filesystem::path outputDirectoryPath(directoryPath);

		if(!std::filesystem::is_directory(outputDirectoryPath)) {
			std::error_code errorCode;
			std::filesystem::create_directories(outputDirectoryPath, errorCode);

			if(errorCode) {
				spdlog::error("Cannot create '{}' DOSBox template command script file, output directory '{}' creation failed: {}", magic_enum::enum_name(gameType), directoryPath, errorCode.message());
				return false;
			}
		}
	}

	std::string templateScriptFileName(getDOSBoxCommandScriptFileName(gameType));
	std::string templateScriptFilePath(Utilities::joinPaths(directoryPath, templateScriptFileName));

	if(std::filesystem::is_regular_file(std::filesystem::path(templateScriptFilePath)) && !overwrite) {
		spdlog::debug("'{}' DOSBox template command script already exists at '{}', specify overwrite to replace.", magic_enum::enum_name(gameType), templateScriptFilePath);
		return false;
	}

	std::string templateCommandScriptFileData(generateDOSBoxTemplateCommandScriptFileData(gameType));

	std::ofstream fileStream(templateScriptFilePath);

	if(!fileStream.is_open()) {
		return false;
	}

	fileStream.write(reinterpret_cast<const char *>(templateCommandScriptFileData.data()), templateCommandScriptFileData.size());

	fileStream.close();

	spdlog::info("Created '{}' DOSBox template command file script '{}' in directory '{}'.", magic_enum::enum_name(gameType), templateScriptFileName, directoryPath);

	return true;
}

std::string ModManager::getDOSBoxCommandScriptFileName(GameType gameType) {
	return Utilities::toLowerCase(magic_enum::enum_name(gameType)) + ".cmd.in";
}

std::string ModManager::getDOSBoxCommandScriptFilePath(GameType gameType) {
	SettingsManager * settings = SettingsManager::getInstance();

	return Utilities::joinPaths(settings->dataDirectoryPath, settings->dosboxDataDirectoryName, settings->dosboxCommandScriptsDirectoryName, getDOSBoxCommandScriptFileName(gameType));
}

std::string ModManager::generateDOSBoxTemplateCommandScriptFileData(GameType gameType) {
	std::stringstream templateStream;

	templateStream << "MOUNT C \":GAMEPATH:\"" << std::endl;
	templateStream << "C:" << std::endl;

	if(gameType == GameType::Client || gameType == GameType::Server) {
		templateStream << "IPX ENABLE" << std::endl;

		if(gameType == GameType::Client) {
			templateStream << "IPXNET CONNECT :IP: :PORT:" << std::endl;
		}
		else if(gameType == GameType::Server) {
			templateStream << "IPXNET STARTSERVER :PORT:" << std::endl;
		}
	}

	templateStream << ":COMMAND:" << std::endl;
	templateStream << "<EXIT|EXIT>" << std::endl;

	return templateStream.str();
}

void ModManager::onSelectedModChanged(std::shared_ptr<Mod> mod) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	setSelectedMod(mod);
}

void ModManager::onSelectedFavouriteModChanged(std::shared_ptr<ModIdentifier> favouriteMod) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(favouriteMod == nullptr) {
		clearSelectedMod();
		return;
	}

	setSelectedMod(*favouriteMod);
}

void ModManager::onDOSBoxVersionCollectionSizeChanged(DOSBoxVersionCollection & dosboxVersionCollection) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	std::shared_ptr<DOSBoxVersionCollection> dosboxVersions(getDOSBoxVersions());

	if(m_preferredDOSBoxVersion != nullptr && !dosboxVersions->hasDOSBoxVersion(*m_preferredDOSBoxVersion)) {
		SettingsManager * settings = SettingsManager::getInstance();

		if(dosboxVersions->numberOfDOSBoxVersions() == 0) {
			m_preferredDOSBoxVersion = nullptr;
			settings->preferredDOSBoxVersionID.clear();
		}
		else {
			m_preferredDOSBoxVersion = dosboxVersions->getDOSBoxVersion(0);
			settings->preferredDOSBoxVersionID = m_preferredDOSBoxVersion->getID();
		}
	}
}

void ModManager::onDOSBoxVersionCollectionItemModified(DOSBoxVersionCollection & dosboxVersionCollection, DOSBoxVersion & dosboxVersion) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_preferredDOSBoxVersion.get() == &dosboxVersion) {
		SettingsManager::getInstance()->preferredDOSBoxVersionID = dosboxVersion.getID();
	}
}

void ModManager::onGameVersionCollectionSizeChanged(GameVersionCollection & gameVersionCollection) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_preferredGameVersion != nullptr && !getGameVersions()->hasGameVersion(*m_preferredGameVersion)) {
		SettingsManager * settings = SettingsManager::getInstance();

		if(getGameVersions()->numberOfGameVersions() == 0) {
			m_preferredGameVersion = nullptr;
			settings->preferredGameVersionID.clear();
		}
		else {
			m_preferredGameVersion = getGameVersions()->getGameVersion(0);
			settings->preferredGameVersionID = m_preferredGameVersion->getID();
		}
	}
}

void ModManager::onGameVersionCollectionItemModified(GameVersionCollection & gameVersionCollection, GameVersion & gameVersion) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_preferredGameVersion.get() == &gameVersion) {
		SettingsManager::getInstance()->preferredGameVersionID = gameVersion.getID();
	}
}
