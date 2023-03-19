#include "ModManager.h"

#include "DOSBox/DOSBoxManager.h"
#include "DOSBox/DOSBoxVersion.h"
#include "Download/DownloadManager.h"
#include "Environment.h"
#include "Game/GameLocator.h"
#include "Game/GameManager.h"
#include "Game/GameVersion.h"
#include "Game/GameVersionCollection.h"
#include "Group/Group.h"
#include "Group/GroupUtilities.h"
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
#include "SettingsManager.h"
#include "Project.h"

#include <Analytics/Segment/SegmentAnalytics.h>
#include <Archive/ArchiveFactoryRegistry.h>
#include <Archive/Zip/ZipArchive.h>
#include <Arguments/ArgumentParser.h>
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
#include <Utilities/StringUtilities.h>
#include <Utilities/Utilities.h>

#include <fmt/core.h>
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <array>
#include <conio.h>
#include <errno.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <regex>
#include <sstream>

using namespace std::chrono_literals;

static constexpr uint8_t NUMBER_OF_INITIALIZATION_STEPS = 15;

const GameType ModManager::DEFAULT_GAME_TYPE = GameType::Game;
const std::string ModManager::DEFAULT_PREFERRED_DOSBOX_VERSION(DOSBoxVersion::DOSBOX.getName());
const std::string ModManager::DEFAULT_PREFERRED_GAME_VERSION(GameVersion::ORIGINAL_ATOMIC_EDITION.getName());
const std::string ModManager::HTTP_USER_AGENT("DukeNukem3DModManager/" + APPLICATION_VERSION);
const std::string ModManager::DEFAULT_BACKUP_FILE_RENAME_SUFFIX("_");

ModManager::ModManager()
	: Application()
	, m_initialized(false)
	, m_initializing(false)
	, m_localMode(false)
	, m_demoRecordingEnabled(false)
	, m_gameType(ModManager::DEFAULT_GAME_TYPE)
	, m_selectedModVersionIndex(std::numeric_limits<size_t>::max())
	, m_selectedModVersionTypeIndex(std::numeric_limits<size_t>::max())
	, m_selectedModGameVersionIndex(std::numeric_limits<size_t>::max())
	, m_dosboxManager(std::make_shared<DOSBoxManager>())
	, m_gameManager(std::make_shared<GameManager>())
	, m_mods(std::make_shared<ModCollection>())
	, m_favouriteMods(std::make_shared<FavouriteModCollection>())
	, m_organizedMods(std::make_shared<OrganizedModCollection>(m_mods, m_favouriteMods, m_gameManager->getGameVersions()))
	, m_initializationStep(0) {
	assignPlatformFactories();

	FactoryRegistry::getInstance().setFactory<SettingsManager>([]() {
		return std::make_unique<SettingsManager>();
	});

	m_selectedModChangedConnection = m_organizedMods->selectedModChanged.connect(std::bind(&ModManager::onSelectedModChanged, this, std::placeholders::_1));
	m_selectedFavouriteModChangedConnection = m_organizedMods->selectedFavouriteModChanged.connect(std::bind(&ModManager::onSelectedFavouriteModChanged, this, std::placeholders::_1));
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

		if(m_arguments->hasArgument("?") || m_arguments->hasArgument("help")) {
			displayArgumentHelp();
			initialized();
			return true;
		}

		if(m_arguments->hasArgument("version")) {
			fmt::print("{}\n", APPLICATION_VERSION);
			initialized();
			return true;
		}

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

	settings->load(m_arguments.get());

	if(settings->localMode && !localModeSet) {
		m_localMode = true;
	}

	m_organizedMods->setLocalMode(m_localMode);

	bool updateFileInfo = m_localMode && m_arguments != nullptr && (m_arguments->hasArgument("update-new") || m_arguments->hasArgument("update-all"));

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

	if(!notifyInitializationProgress("Creating DOSBox Template Script Files")) {
		return false;
	}

	createDOSBoxTemplateScriptFiles();

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

	m_preferredDOSBoxVersion = dosboxVersions->getDOSBoxVersionWithName(settings->preferredDOSBoxVersion);

	if(m_preferredDOSBoxVersion == nullptr) {
		m_preferredDOSBoxVersion = dosboxVersions->getDOSBoxVersion(0);
		settings->preferredDOSBoxVersion = m_preferredDOSBoxVersion->getName();

		spdlog::warn("DOSBox configuration for game version '{}' is missing, changing preferred game version to '{}.", settings->preferredDOSBoxVersion, m_preferredDOSBoxVersion->getName());
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

	m_preferredGameVersion = gameVersions->getGameVersionWithName(settings->preferredGameVersion);

	if(m_preferredGameVersion == nullptr) {
		m_preferredGameVersion = gameVersions->getGameVersion(0);
		settings->preferredGameVersion = m_preferredGameVersion->getName();

		spdlog::warn("Game configuration for game version '{}' is missing, changing preferred game version to '{}.", settings->preferredGameVersion, m_preferredGameVersion->getName());
	}

	m_gameVersionCollectionSizeChangedConnection = gameVersions->sizeChanged.connect(std::bind(&ModManager::onGameVersionCollectionSizeChanged, this, std::placeholders::_1));
	m_gameVersionCollectionItemModifiedConnection = gameVersions->itemModified.connect(std::bind(&ModManager::onGameVersionCollectionItemModified, this, std::placeholders::_1, std::placeholders::_2));

	if(!notifyInitializationProgress("Loading Mod List")) {
		return false;
	}

	if(!m_mods->loadFrom(getModsListFilePath(), updateFileInfo)) {
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

	if(!notifyInitializationProgress("Loading Favourite Mod List")) {
		return false;
	}

	m_favouriteMods->loadFrom(settings->favouriteModsListFilePath);
	m_favouriteMods->checkForMissingFavouriteMods(*m_mods.get());

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

	std::chrono::milliseconds initializationDuration(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - initializeSteadyStartTimePoint));

	spdlog::info("{} initialized successfully after {} milliseconds.", APPLICATION_NAME, initializationDuration.count());

	std::map<std::string, std::any> properties;
	properties["sessionNumber"] = segmentAnalytics->getSessionNumber();
	properties["initializationDuration"] = initializationDuration.count();
	properties["environment"] = Utilities::toCapitalCase(APPLICATION_ENVIRONMENT);
	properties["gameType"] = Utilities::toCapitalCase(magic_enum::enum_name(settings->gameType));
	properties["preferredGameVersion"] = settings->preferredGameVersion;
	properties["numberOfDownloadedMods"] = m_downloadManager != nullptr ? m_downloadManager->numberOfDownloadedMods() : 0;
	properties["dosboxArguments"] = settings->dosboxArguments;
	properties["dosboxServerIPAddress"] = settings->dosboxServerIPAddress;
	properties["dosboxServerLocalPort"] = settings->dosboxLocalServerPort;
	properties["dosboxServerRemotePort"] = settings->dosboxRemoteServerPort;
	properties["apiBaseURL"] = settings->apiBaseURL;
	properties["connectionTimeout"] = settings->connectionTimeout.count();
	properties["networkTimeout"] = settings->networkTimeout.count();
	properties["transferTimeout"] = settings->transferTimeout.count();

	segmentAnalytics->track("Application Initialized", properties);

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

bool ModManager::initializeAndWait(int argc, char * argv[]) {
	std::future<bool> initializeFuture(initializeAsync(argc, argv));

	if(!initializeFuture.valid()) {
		return false;
	}

	initializeFuture.wait();

	return initializeFuture.get();
}

bool ModManager::initializeAndWait(std::shared_ptr<ArgumentParser> arguments) {
	std::future<bool> initializeFuture(initializeAsync(arguments));

	if(!initializeFuture.valid()) {
		return false;
	}

	initializeFuture.wait();

	return initializeFuture.get();
}

bool ModManager::uninitialize() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	SettingsManager * settings = SettingsManager::getInstance();

	settings->save(m_arguments.get());
	m_favouriteMods->saveTo(settings->favouriteModsListFilePath);
	getDOSBoxVersions()->saveTo(settings->dosboxVersionsListFilePath);
	getGameVersions()->saveTo(settings->gameVersionsListFilePath);

	SegmentAnalytics * segmentAnalytics = SegmentAnalytics::getInstance();
	segmentAnalytics->onApplicationClosed();
	segmentAnalytics->flush(3s);

	m_selectedMod.reset();
	m_organizedMods->setModCollection(nullptr);
	m_organizedMods->setFavouriteModCollection(nullptr);
	m_organizedMods->setGameVersionCollection(nullptr);
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

std::shared_ptr<FavouriteModCollection> ModManager::getFavouriteMods() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_favouriteMods;
}

std::shared_ptr<OrganizedModCollection> ModManager::getOrganizedMods() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_organizedMods;
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

	if(m_gameType != gameType) {
		m_gameType = gameType;

		SettingsManager::getInstance()->gameType = m_gameType;

		gameTypeChanged(m_gameType);
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
		std::string dosboxVersionName(m_arguments->getFirstValue("dosbox"));

		selectedDOSBoxVersion = getDOSBoxVersions()->getDOSBoxVersionWithName(dosboxVersionName);

		if(selectedDOSBoxVersion == nullptr) {
			spdlog::error("Could not find DOSBox version override for '{}'.", dosboxVersionName);
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

bool ModManager::setPreferredDOSBoxVersion(const std::string & dosboxVersionName) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(dosboxVersionName.empty()) {
		return false;
	}

	return setPreferredDOSBoxVersion(getDOSBoxVersions()->getDOSBoxVersionWithName(dosboxVersionName));
}

bool ModManager::setPreferredDOSBoxVersion(std::shared_ptr<DOSBoxVersion> dosboxVersion) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(dosboxVersion == nullptr || !dosboxVersion->isValid()) {
		return false;
	}

	if(m_preferredDOSBoxVersion == nullptr || !Utilities::areStringsEqualIgnoreCase(m_preferredDOSBoxVersion->getName(), dosboxVersion->getName())) {
		m_preferredDOSBoxVersion = dosboxVersion;
		SettingsManager::getInstance()->preferredDOSBoxVersion = m_preferredDOSBoxVersion->getName();

		preferredDOSBoxVersionChanged(m_preferredDOSBoxVersion);
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

	std::shared_ptr<GameVersion> selectedGameVersion;

	if(m_arguments != nullptr && m_arguments->hasArgument("game") && !m_arguments->getFirstValue("game").empty()) {
		std::string gameVersionName(m_arguments->getFirstValue("game"));

		selectedGameVersion = getGameVersions()->getGameVersionWithName(gameVersionName);

		if(selectedGameVersion == nullptr) {
			spdlog::error("Could not find game version override for '{}'.", gameVersionName);
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

bool ModManager::setPreferredGameVersion(const std::string & gameVersionName) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(gameVersionName.empty()) {
		return false;
	}

	return setPreferredGameVersion(getGameVersions()->getGameVersionWithName(gameVersionName));
}

bool ModManager::setPreferredGameVersion(std::shared_ptr<GameVersion> gameVersion) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(gameVersion == nullptr || !gameVersion->isValid()) {
		return false;
	}

	if(m_preferredGameVersion == nullptr || !Utilities::areStringsEqualIgnoreCase(m_preferredGameVersion->getName(), gameVersion->getName())) {
		m_preferredGameVersion = gameVersion;
		SettingsManager::getInstance()->preferredGameVersion = m_preferredGameVersion->getName();

		m_selectedModGameVersionIndex = std::numeric_limits<size_t>::max();
		std::shared_ptr<ModVersionType> selectedModVersionType(getSelectedModVersionType());

		if(selectedModVersionType != nullptr) {
			std::shared_ptr<GameVersion> selectedGameVersion(getSelectedGameVersion());

			if(selectedGameVersion != nullptr) {
				std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(selectedModVersionType->getCompatibleModGameVersions(*selectedGameVersion));

				if(!compatibleModGameVersions.empty()) {
					m_selectedModGameVersionIndex = selectedModVersionType->indexOfGameVersion(compatibleModGameVersions.back()->getGameVersion());
				}
			}
		}

		modSelectionChanged(m_selectedMod, m_selectedModVersionIndex, m_selectedModVersionTypeIndex, m_selectedModGameVersionIndex);
		preferredGameVersionChanged(m_preferredGameVersion);
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

	if(!Utilities::areStringsEqual(settings->dosboxServerIPAddress, formattedIPAddress)) {
		settings->dosboxServerIPAddress = formattedIPAddress;

		dosboxServerIPAddressChanged(settings->dosboxServerIPAddress);
	}
}

uint16_t ModManager::getDOSBoxLocalServerPort() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return SettingsManager::getInstance()->dosboxLocalServerPort;
}

void ModManager::setDOSBoxLocalServerPort(uint16_t port) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->dosboxLocalServerPort != port) {
		settings->dosboxLocalServerPort = port;

		dosboxLocalServerPortChanged(settings->dosboxLocalServerPort);
	}
}

uint16_t ModManager::getDOSBoxRemoteServerPort() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return SettingsManager::getInstance()->dosboxRemoteServerPort;
}

void ModManager::setDOSBoxRemoteServerPort(uint16_t port) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	SettingsManager * settings = SettingsManager::getInstance();

	if(settings->dosboxRemoteServerPort != port) {
		settings->dosboxRemoteServerPort = port;

		dosboxRemoteServerPortChanged(settings->dosboxRemoteServerPort);
	}
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

bool ModManager::setSelectedModByName(const std::string & name) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return setSelectedMod(m_mods->getMod(name));
}

bool ModManager::setSelectedMod(std::shared_ptr<Mod> mod) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!Mod::isValid(mod.get(), true)) {
		m_selectedMod = nullptr;
		m_selectedModVersionIndex = std::numeric_limits<size_t>::max();
		m_selectedModVersionTypeIndex = std::numeric_limits<size_t>::max();
		m_selectedModGameVersionIndex = std::numeric_limits<size_t>::max();

		modSelectionChanged(m_selectedMod, m_selectedModVersionIndex, m_selectedModVersionTypeIndex, m_selectedModGameVersionIndex);

		return false;
	}

	if(m_selectedMod != mod) {
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
					std::shared_ptr<GameVersion> selectedGameVersion(getSelectedGameVersion());

					if(selectedGameVersion != nullptr) {
						std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(selectedModVersionType->getCompatibleModGameVersions(*selectedGameVersion));

						if(!compatibleModGameVersions.empty()) {
							m_selectedModGameVersionIndex = selectedModVersionType->indexOfGameVersion(compatibleModGameVersions.back()->getGameVersion());
						}
					}
				}
			}
		}

		modSelectionChanged(m_selectedMod, m_selectedModVersionIndex, m_selectedModVersionTypeIndex, m_selectedModGameVersionIndex);
	}

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
			std::shared_ptr<GameVersion> selectedGameVersion(getSelectedGameVersion());

			if(selectedGameVersion != nullptr) {
				std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(selectedModVersionType->getCompatibleModGameVersions(*selectedGameVersion));

				if(!compatibleModGameVersions.empty()) {
					m_selectedModGameVersionIndex = selectedModVersionType->indexOfGameVersion(compatibleModGameVersions.back()->getGameVersion());
				}
			}
		}
	}

	modSelectionChanged(m_selectedMod, m_selectedModVersionIndex, m_selectedModVersionTypeIndex, m_selectedModGameVersionIndex);

	return true;
}

bool ModManager::setSelectedMod(const ModIdentifier & modIdentifier) {
	if(!modIdentifier.isValid()) {
		return false;
	}

	std::shared_ptr<Mod> selectedMod(m_mods->getModWithName(modIdentifier.getName()));

	if(selectedMod == nullptr) {
		return false;
	}

	m_selectedMod = selectedMod;

	m_selectedModVersionIndex = m_selectedMod->indexOfVersion(modIdentifier.getVersion());

	if(m_selectedModVersionIndex == std::numeric_limits<size_t>::max()) {
		m_selectedModVersionIndex = m_selectedMod->indexOfPreferredVersion();
	}

	std::shared_ptr<ModVersion> selectedModVersion(m_selectedMod->getVersion(m_selectedModVersionIndex));

	m_selectedModVersionTypeIndex = selectedModVersion->indexOfType(modIdentifier.getVersionType());

	if(m_selectedModVersionTypeIndex == std::numeric_limits<size_t>::max()) {
		m_selectedModVersionTypeIndex = m_selectedMod->indexOfDefaultVersionType();
	}
	
	std::shared_ptr<ModVersionType> selectedModVersionType(selectedModVersion->getType(m_selectedModVersionTypeIndex));

	std::shared_ptr<GameVersion> selectedGameVersion(getSelectedGameVersion());

	if(selectedGameVersion != nullptr) {
		std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(selectedModVersionType->getCompatibleModGameVersions(*selectedGameVersion));

		if(!compatibleModGameVersions.empty()) {
			m_selectedModGameVersionIndex = selectedModVersionType->indexOfGameVersion(compatibleModGameVersions.back()->getGameVersion());
		}
	}

	modSelectionChanged(m_selectedMod, m_selectedModVersionIndex, m_selectedModVersionTypeIndex, m_selectedModGameVersionIndex);

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
			modSelectionChanged(m_selectedMod, m_selectedModVersionIndex, m_selectedModVersionTypeIndex, m_selectedModGameVersionIndex);
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

	if(m_selectedModVersionIndex != newModVersionIndex) {
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
			std::shared_ptr<GameVersion> selectedGameVersion(getSelectedGameVersion());

			if(selectedGameVersion != nullptr) {
				std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(selectedModVersionType->getCompatibleModGameVersions(*selectedGameVersion));

				if(!compatibleModGameVersions.empty()) {
					m_selectedModGameVersionIndex = selectedModVersionType->indexOfGameVersion(compatibleModGameVersions.back()->getGameVersion());
				}
			}
		}

		modSelectionChanged(m_selectedMod, m_selectedModVersionIndex, m_selectedModVersionTypeIndex, m_selectedModGameVersionIndex);
	}

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
			modSelectionChanged(m_selectedMod, m_selectedModVersionIndex, m_selectedModVersionTypeIndex, m_selectedModGameVersionIndex);
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

	if(m_selectedModVersionTypeIndex != newModVersionTypeIndex) {
		m_selectedModVersionTypeIndex = newModVersionTypeIndex;
		m_selectedModGameVersionIndex = std::numeric_limits<size_t>::max();

		if(m_selectedModVersionTypeIndex != std::numeric_limits<size_t>::max()) {
			std::shared_ptr<ModVersionType> selectedModVersionType(selectedModVersion->getType(m_selectedModVersionTypeIndex));
			std::shared_ptr<GameVersion> selectedGameVersion(getSelectedGameVersion());

			if(selectedGameVersion != nullptr) {
				std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(selectedModVersionType->getCompatibleModGameVersions(*selectedGameVersion));

				if(!compatibleModGameVersions.empty()) {
					m_selectedModGameVersionIndex = selectedModVersionType->indexOfGameVersion(compatibleModGameVersions.back()->getGameVersion());
				}
			}
		}

		modSelectionChanged(m_selectedMod, m_selectedModVersionIndex, m_selectedModVersionTypeIndex, m_selectedModGameVersionIndex);
	}

	return true;
}

bool ModManager::setSelectedModGameVersionIndex(size_t modGameVersionIndex) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(modGameVersionIndex == std::numeric_limits<size_t>::max()) {
		bool modGameVersionChanged = m_selectedModGameVersionIndex != std::numeric_limits<size_t>::max();

		m_selectedModGameVersionIndex = std::numeric_limits<size_t>::max();

		if(modGameVersionChanged) {
			modSelectionChanged(m_selectedMod, m_selectedModVersionIndex, m_selectedModVersionTypeIndex, m_selectedModGameVersionIndex);
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

	if(m_selectedModGameVersionIndex != newModGameVersionIndex) {
		m_selectedModGameVersionIndex = newModGameVersionIndex;

		modSelectionChanged(m_selectedMod, m_selectedModVersionIndex, m_selectedModVersionTypeIndex, m_selectedModGameVersionIndex);
	}

	return true;
}

bool ModManager::selectRandomMod(bool selectPreferredVersion, bool selectFirstVersionType) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	return m_organizedMods->selectRandomMod();
}

bool ModManager::selectRandomGameVersion() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	return m_organizedMods->selectRandomGameVersion();
}

bool ModManager::selectRandomTeam() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	return m_organizedMods->selectRandomTeam();
}

bool ModManager::selectRandomAuthor() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return false;
	}

	return m_organizedMods->selectRandomAuthor();
}

std::vector<ModMatch> ModManager::searchForMod(const std::vector<std::shared_ptr<Mod>> & mods, const std::string & query) {
	std::string formattedQuery(Utilities::toLowerCase(Utilities::trimString(query)));

	if(formattedQuery.empty()) {
		return {};
	}

	SegmentAnalytics * segmentAnalytics = SegmentAnalytics::getInstance();

	std::map<std::string, std::any> properties;
	properties["query"] = formattedQuery;

	segmentAnalytics->track("Mod Search", properties);

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

	for(size_t i = 0; i < mods.size(); i++) {
		mod = mods[i];

		if(!Mod::isValid(mod.get(), true)) {
			continue;
		}

		formattedModName = Utilities::toLowerCase(mod->getName());
		modNameMatches = formattedQuery == formattedModName;

		if(modNameMatches && mod->numberOfVersions() == 1 && mod->getVersion(0)->numberOfTypes() == 1) {
			matches.clear();
			matches.emplace_back(mod, mod->getVersion(0), mod->getVersion(0)->getType(0), i, 0, 0);

			return matches;
		}

		modNamePartiallyMatches = formattedModName.find(formattedQuery) != std::string::npos;

		for(size_t j = 0; j < mod->numberOfVersions(); j++) {
			modVersion = mod->getVersion(j);
			formattedModVersion = Utilities::toLowerCase(modVersion->getFullName());
			modVersionMatches = formattedQuery == formattedModVersion;

			if((modVersionMatches && modVersion->numberOfTypes() == 1) ||
			   (modNameMatches && modVersion->getVersion().empty() && modVersion->numberOfTypes() == 1)) {
				matches.clear();
				matches.emplace_back(mod, modVersion, modVersion->getType(0), i, j, 0);

				return matches;
			}

			modVersionPartiallyMatches = formattedModVersion.find(formattedQuery) != std::string::npos;

			for(size_t k = 0; k < modVersion->numberOfTypes(); k++) {
				modVersionType = modVersion->getType(k);
				formattedModVersionType = Utilities::toLowerCase(modVersionType->getFullName());
				modVersionTypeMatches = formattedQuery == formattedModVersionType;

				if(modVersionTypeMatches) {
					matches.clear();
					matches.emplace_back(mod, modVersion, modVersionType, i, j, k);

					return matches;
				}

				modVersionTypePartiallyMatches = formattedModVersionType.find(formattedQuery) != std::string::npos;

				if(modVersionTypePartiallyMatches && !modNamePartiallyMatches && !modVersionPartiallyMatches) {
					matches.emplace_back(mod, modVersion, modVersionType, i, j, k);
				}
			}

			if(modVersionPartiallyMatches && !modNamePartiallyMatches) {
				if(modVersion->numberOfTypes() == 1) {
					matches.emplace_back(mod, modVersion, modVersion->getType(0), i, j, 0);
				}
				else {
					matches.emplace_back(mod, modVersion, i, j);
				}
			}
		}

		if(modNamePartiallyMatches) {
			if(mod->numberOfVersions() == 1) {
				if(mod->getVersion(0)->numberOfTypes() == 1) {
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

	return matches;
}

size_t ModManager::searchForAndSelectGameVersion(const std::string & query, std::vector<std::shared_ptr<GameVersion>> * matches) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return std::numeric_limits<size_t>::max();
	}

	std::string formattedQuery(Utilities::toLowerCase(Utilities::trimString(query)));

	if(formattedQuery.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	SegmentAnalytics * segmentAnalytics = SegmentAnalytics::getInstance();

	std::map<std::string, std::any> properties;
	properties["query"] = formattedQuery;

	segmentAnalytics->track("Game Version Search", properties);

	std::string gameVersionName;
	size_t matchingGameVersionIndex = std::numeric_limits<size_t>::max();
	size_t numberOfMatches = 0;

	for(size_t i = 0; i < m_organizedMods->numberOfGameVersions(); i++) {
		gameVersionName = Utilities::toLowerCase(m_organizedMods->getGameVersion(i)->getName());

		if(gameVersionName == formattedQuery) {
			matchingGameVersionIndex = i;
			numberOfMatches = 1;

			if(matches != nullptr) {
				matches->clear();
				matches->push_back(m_organizedMods->getGameVersion(i));
			}

			break;
		}

		if(gameVersionName.find(formattedQuery) != std::string::npos) {
			if(matchingGameVersionIndex == std::numeric_limits<size_t>::max()) {
				matchingGameVersionIndex = i;
			}

			if(matches != nullptr) {
				matches->push_back(m_organizedMods->getGameVersion(i));
			}

			numberOfMatches++;
		}
	}

	if(numberOfMatches == 1) {
		m_organizedMods->setSelectedGameVersion(matchingGameVersionIndex);
	}

	return numberOfMatches;
}

size_t ModManager::searchForAndSelectTeam(const std::string & query, std::vector<std::shared_ptr<ModAuthorInformation>> * matches) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return std::numeric_limits<size_t>::max();
	}

	std::string formattedQuery(Utilities::toLowerCase(Utilities::trimString(query)));

	if(formattedQuery.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	SegmentAnalytics * segmentAnalytics = SegmentAnalytics::getInstance();

	std::map<std::string, std::any> properties;
	properties["query"] = formattedQuery;

	segmentAnalytics->track("Team Search", properties);

	std::string teamName;
	size_t matchingTeamIndex = std::numeric_limits<size_t>::max();
	size_t numberOfMatches = 0;

	for(size_t i = 0; i < m_organizedMods->numberOfTeams(); i++) {
		teamName = Utilities::toLowerCase(m_organizedMods->getTeamInfo(i)->getName());

		if(teamName == formattedQuery) {
			matchingTeamIndex = i;
			numberOfMatches = 1;

			if(matches != nullptr) {
				matches->clear();
				matches->push_back(m_organizedMods->getTeamInfo(i));
			}

			break;
		}

		if(teamName.find(formattedQuery) != std::string::npos) {
			if(matchingTeamIndex == std::numeric_limits<size_t>::max()) {
				matchingTeamIndex = i;
			}

			if(matches != nullptr) {
				matches->push_back(m_organizedMods->getTeamInfo(i));
			}

			numberOfMatches++;
		}
	}

	if(numberOfMatches == 1) {
		m_organizedMods->setSelectedTeam(matchingTeamIndex);
	}

	return numberOfMatches;
}

size_t ModManager::searchForAndSelectAuthor(const std::string & query, std::vector<std::shared_ptr<ModAuthorInformation>> * matches) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return std::numeric_limits<size_t>::max();
	}

	std::string formattedQuery(Utilities::toLowerCase(Utilities::trimString(query)));

	if(formattedQuery.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	SegmentAnalytics * segmentAnalytics = SegmentAnalytics::getInstance();

	std::map<std::string, std::any> properties;
	properties["query"] = formattedQuery;

	segmentAnalytics->track("Author Search", properties);

	std::string authorName;
	size_t matchingAuthorIndex = std::numeric_limits<size_t>::max();
	size_t numberOfMatches = 0;

	for(size_t i = 0; i < m_organizedMods->numberOfAuthors(); i++) {
		authorName = Utilities::toLowerCase(m_organizedMods->getAuthorInfo(i)->getName());

		if(authorName == formattedQuery) {
			matchingAuthorIndex = i;
			numberOfMatches = 1;

			if(matches != nullptr) {
				matches->clear();
				matches->push_back(m_organizedMods->getAuthorInfo(i));
			}

			break;
		}

		if(authorName.find(formattedQuery) != std::string::npos) {
			if(matchingAuthorIndex == std::numeric_limits<size_t>::max()) {
				matchingAuthorIndex = i;
			}

			if(matches != nullptr) {
				matches->push_back(m_organizedMods->getAuthorInfo(i));
			}

			numberOfMatches++;
		}
	}

	if(numberOfMatches == 1) {
		m_organizedMods->setSelectedAuthor(matchingAuthorIndex);
	}

	return numberOfMatches;
}

void ModManager::clearSelectedMod() {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	bool selectedModChanged = m_selectedMod != nullptr;

	m_selectedMod = nullptr;
	m_selectedModVersionIndex = std::numeric_limits<size_t>::max();
	m_selectedModVersionTypeIndex = std::numeric_limits<size_t>::max();

	if(selectedModChanged) {
		modSelectionChanged(m_selectedMod, m_selectedModVersionIndex, m_selectedModVersionTypeIndex, m_selectedModGameVersionIndex);
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

	std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(selectedGameVersion->getCompatibleModGameVersions(selectedModVersionType->getGameVersions()));

	return !compatibleModGameVersions.empty();
}

std::future<bool> ModManager::runSelectedModAsync(std::shared_ptr<GameVersion> alternateGameVersion, std::shared_ptr<ModGameVersion> alternateModGameVersion) {
	return std::async(std::launch::async, &ModManager::runSelectedMod, std::ref(*this), alternateGameVersion, alternateModGameVersion);
}

bool ModManager::runSelectedModAndWait(std::shared_ptr<GameVersion> alternateGameVersion, std::shared_ptr<ModGameVersion> alternateModGameVersion) {
	std::future<bool> runSelectedModFuture(runSelectedModAsync(alternateGameVersion, alternateModGameVersion));

	if(!runSelectedModFuture.valid()) {
		return false;
	}

	runSelectedModFuture.wait();

	return runSelectedModFuture.get();
}

bool ModManager::runSelectedMod(std::shared_ptr<GameVersion> alternateGameVersion, std::shared_ptr<ModGameVersion> alternateModGameVersion) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized || m_gameProcess != nullptr) {
		notifyLaunchError("Mod manager not initialized or game process already running.");
		return false;
	}

	SettingsManager * settings = SettingsManager::getInstance();

	SegmentAnalytics * segmentAnalytics = SegmentAnalytics::getInstance();

	std::shared_ptr<GameVersion> selectedGameVersion(alternateGameVersion != nullptr ? alternateGameVersion : getSelectedGameVersion());

	if(selectedGameVersion == nullptr) {
		notifyLaunchError("No game version selected.");
		return false;
	}

	if(!selectedGameVersion->isConfigured()) {
		notifyLaunchError(fmt::format("Game version '{}' is not configured.", selectedGameVersion->getName()));
		return false;
	}

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

		std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(selectedGameVersion->getCompatibleModGameVersions(selectedModVersionType->getGameVersions()));

		if(compatibleModGameVersions.empty()) {
			notifyLaunchError(fmt::format("{} is not supported on {}.", m_selectedMod->getFullName(m_selectedModVersionIndex, m_selectedModVersionTypeIndex), selectedGameVersion->getName()));
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

	bool shouldConfigureApplicationTemporaryDirectory = selectedGameVersion->doesRequireCombinedGroup() && areSymlinksSupported() && selectedGameVersion->doesSupportSubdirectories();
	bool shouldConfigureGameTemporaryDirectory = !areSymlinksSupported() && selectedGameVersion->doesSupportSubdirectories();

	std::string temporaryDirectoryName;

	if(shouldConfigureGameTemporaryDirectory) {
		temporaryDirectoryName = settings->gameTempDirectoryName;
	}
	else if(shouldConfigureApplicationTemporaryDirectory) {
		temporaryDirectoryName = settings->tempSymlinkName;
	}

	if(!m_localMode && selectedModGameVersion != nullptr) {
		if(!m_downloadManager->downloadModGameVersion(selectedModGameVersion.get(), getGameVersions().get())) {
			notifyLaunchError(fmt::format("Aborting launch of '{}' mod!", selectedModGameVersion->getFullName()));
			return false;
		}
	}

	std::string combinedGroupFileName;
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

	if(selectedModGameVersion != nullptr) {
		std::optional<std::string> conFileName(selectedModGameVersion->getFirstFileNameOfType("con"));

		if(conFileName.has_value()) {
			scriptArgs.addArgument("CON", *conFileName);
		}

		std::vector<std::string> groupFileNames(selectedModGameVersion->getFileNamesOfType("grp"));

		if(selectedGameVersion->doesRequireCombinedGroup()) {
			combinedGroupFileName = Utilities::replaceFileExtension(groupFileNames[0], "CMB");
			scriptArgs.addArgument("COMBINEDGROUP", combinedGroupFileName);
		}
		else {
			for(const std::string & groupFileName : groupFileNames) {
				scriptArgs.addArgument("GROUP", groupFileName);
			}
		}

		std::vector<std::string> zipFileNames(selectedModGameVersion->getFileNamesOfType("zip"));

		for(const std::string & zipFileName : zipFileNames) {
			scriptArgs.addArgument("GROUP", zipFileName);
		}

		std::vector<std::string> defFileNames(selectedModGameVersion->getFileNamesOfType("def"));

		for(const std::string & defFileName : defFileNames) {
			scriptArgs.addArgument("DEF", defFileName);
		}
	}

	if(m_gameType == GameType::Client) {
		scriptArgs.addArgument("IP", settings->dosboxServerIPAddress);
		scriptArgs.addArgument("PORT", std::to_string(settings->dosboxRemoteServerPort));
	}
	else if(m_gameType == GameType::Server) {
		scriptArgs.addArgument("PORT", std::to_string(settings->dosboxLocalServerPort));
	}

	if(settings->dosboxAutoExit) {
		scriptArgs.addArgument("EXIT", "EXIT");
	}

	if(!selectedGameVersion->doesRequireDOSBox() && (m_gameType == GameType::Client || m_gameType == GameType::Server)) {
		spdlog::info("Network settings are only supported when running in DOSBox, ignoring {} game type setting.", Utilities::toCapitalCase(magic_enum::enum_name(m_gameType)));
	}

	bool customMod = false;
	std::string customMap;
	std::shared_ptr<GameVersion> customTargetGameVersion;
	std::vector<std::string> customGroupFileNames;

	std::string command(generateCommand(selectedModGameVersion, selectedGameVersion, scriptArgs, combinedGroupFileName, &customMod, &customMap, &customTargetGameVersion, &customGroupFileNames));

	if(command.empty()) {
		notifyLaunchError(fmt::format("Failed to generate command."));
		return false;
	}

	std::vector<std::string> temporaryCopiedFilePaths;

	if(shouldConfigureApplicationTemporaryDirectory && !createApplicationTemporaryDirectory()) {
		notifyLaunchError("Failed to create application temporary directory.");
		return false;
	}

	if(!createSymlinksOrCopyTemporaryFiles(*getGameVersions(), *selectedGameVersion, selectedModGameVersion.get(), customMap, shouldConfigureApplicationTemporaryDirectory, &temporaryCopiedFilePaths)) {
		notifyLaunchError("Failed to create symbolic links or copy temporary mod files.");
		return false;
	}

	std::string combinedGroupFilePath;
	std::unique_ptr<Group> combinedGroup;

	if(m_selectedMod != nullptr && !selectedGameVersion->doesRequireGroupFileExtraction() && (selectedGameVersion->doesRequireCombinedGroup() || !m_demoRecordingEnabled)) {
		if(!m_demoRecordingEnabled) {
			ModManager::renameFilesWithSuffixTo("DMO", "DMO" + DEFAULT_BACKUP_FILE_RENAME_SUFFIX, selectedGameVersion->getGamePath());
		}

		if(selectedGameVersion->doesRequireCombinedGroup()) {
			std::string dukeNukemGroupPath(Utilities::joinPaths(selectedGameVersion->getGamePath(), Group::DUKE_NUKEM_3D_GROUP_FILE_NAME));
			combinedGroup = Group::loadFrom(dukeNukemGroupPath);

			if(combinedGroup == nullptr) {
				notifyLaunchError(fmt::format("Failed to load Duke Nukem 3D group for creation of combined group from file path: '{}'.", dukeNukemGroupPath));
				return false;
			}

			if(shouldConfigureApplicationTemporaryDirectory) {
				combinedGroupFilePath = Utilities::joinPaths(settings->appTempDirectoryPath, combinedGroupFileName);
			}
			else if(shouldConfigureGameTemporaryDirectory) {
				combinedGroupFilePath = Utilities::joinPaths(selectedGameVersion->getGamePath(), settings->gameTempDirectoryName, combinedGroupFileName);
			}
			else {
				combinedGroupFilePath = Utilities::joinPaths(selectedGameVersion->getGamePath(), combinedGroupFileName);
			}

			combinedGroup->setFilePath(combinedGroupFilePath);
		}

		std::vector<std::shared_ptr<ModFile>> modFiles(selectedModGameVersion->getFilesOfType("grp"));

		for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = modFiles.begin(); i != modFiles.end(); ++i) {
			std::string modGroupPath(Utilities::joinPaths(getModsDirectoryPath(), getGameVersions()->getGameVersionWithName(selectedModGameVersion->getGameVersion())->getModDirectoryName(), (*i)->getFileName()));
			std::unique_ptr<Group> modGroup(Group::loadFrom(modGroupPath));

			if(modGroup != nullptr) {
				if(!m_demoRecordingEnabled) {
					size_t numberOfDemosExtracted = modGroup->extractAllFilesWithExtension("DMO", selectedGameVersion->getGamePath());

					spdlog::info("Extracted {} demo{} from group file '{}' to game directory '{}'.", numberOfDemosExtracted, numberOfDemosExtracted == 1 ? "" : "s", modGroup->getFilePath(), selectedGameVersion->getGamePath());
				}

				if(combinedGroup != nullptr) {
					for(size_t j = 0; j < modGroup->numberOfFiles(); j++) {
						combinedGroup->addFile(*modGroup->getFile(j), true);
					}
				}
			}
			else if(selectedGameVersion->doesRequireCombinedGroup()) {
				notifyLaunchError(fmt::format("Failed to load mod group from file path: '{}'.", modGroupPath));
				return false;
			}
		}
	}

	if(combinedGroup != nullptr) {
		if(combinedGroup->save(true)) {
			spdlog::info("Saved combined group to file: '{}'.", combinedGroupFilePath);
		}
		else {
			notifyLaunchError(fmt::format("Failed to write combined group to file: '{}'.", combinedGroupFilePath));
			return false;
		}

		combinedGroup.reset();
	}

	std::shared_ptr<InstalledModInfo> installedModInfo;

	if(m_selectedMod != nullptr && selectedGameVersion->doesRequireGroupFileExtraction()) {
		installedModInfo = std::shared_ptr<InstalledModInfo>(extractModFilesToGameDirectory(*selectedModGameVersion, *selectedGameVersion, *customTargetGameVersion, customGroupFileNames).release());

		if(installedModInfo == nullptr) {
			notifyLaunchError(fmt::format("Failed to extract mod files to '{}' game directory.", selectedGameVersion->getName()));
			return false;
		}
	}

	std::string customMapMessage;

	if(!customMap.empty()) {
		customMapMessage = fmt::format(" with custom map '{}'", customMap);
	}

	std::map<std::string, std::any> properties;
	properties["gameVersion"] = selectedGameVersion->getName();
	properties["command"] = command;

	if(!customMap.empty()) {
		properties["customMap"] = customMap;
	}

	if(m_arguments != nullptr && m_arguments->hasPassthroughArguments()) {
		properties["arguments"] = m_arguments->getPassthroughArguments().value();
	}

	std::string gameTypeName(Utilities::toCapitalCase(magic_enum::enum_name(m_gameType)));

	if(customMod) {
		spdlog::info("Running custom mod in {} mode{}.", Utilities::toCapitalCase(magic_enum::enum_name(m_gameType)), customMapMessage);

		segmentAnalytics->track("Running Custom Mod", properties);
	}
	else if(m_selectedMod != nullptr) {
		std::string fullModName(m_selectedMod->getFullName(m_selectedModVersionIndex, m_selectedModVersionTypeIndex));
		spdlog::info("Running '{}' version of mod '{}' in {} mode{}.", selectedModGameVersion->getGameVersion(), fullModName, gameTypeName, customMapMessage);

		properties["modName"] = fullModName;
		properties["modGameVersion"] = selectedModGameVersion->getGameVersion();
		properties["gameType"] = gameTypeName;

		segmentAnalytics->track("Running Mod", properties);
	}
	else {
		segmentAnalytics->track("Running Game", properties);
	}

	segmentAnalytics->flush();

	std::shared_ptr<DOSBoxVersion> selectedDOSBoxVersion(getSelectedDOSBoxVersion());

	if(!selectedDOSBoxVersion->isConfigured()) {
		notifyLaunchError(fmt::format("Selected DOSBox version '{}' is not configured.", selectedDOSBoxVersion->getName()));
		return false;
	}

	std::string workingDirectory(selectedGameVersion->doesRequireDOSBox() ? selectedDOSBoxVersion->getDirectoryPath() : selectedGameVersion->getGamePath());

	spdlog::info("Using working directory: '{}'.", workingDirectory);
	spdlog::info("Executing command: {}", command);

	m_gameProcess = ProcessCreator::getInstance()->createProcess(command, workingDirectory);

	m_gameProcess->terminated.connect([this, gameTypeName](uint64_t nativeExitCode, bool forceTerminated) {
		spdlog::info("{} process {} with code: '{}'.", gameTypeName, forceTerminated ? "force terminated" : "exited", nativeExitCode);

		gameProcessTerminated(nativeExitCode, forceTerminated);
	});

	m_gameProcess->wait();

	if(installedModInfo != nullptr) {
		if(!removeModFilesFromGameDirectory(*selectedGameVersion, *installedModInfo)) {
			spdlog::error("Failed to remove '{}' mod files from '{}' game directory.", selectedModVersionType->getFullName(), selectedGameVersion->getName());
		}
	}

	if(!combinedGroupFilePath.empty() && !shouldConfigureGameTemporaryDirectory) {
		std::filesystem::path filePath(combinedGroupFilePath);

		if(std::filesystem::is_regular_file(filePath)) {
			spdlog::info("Deleting temporary combined group file: '{}'.", combinedGroupFilePath);

			std::error_code errorCode;
			std::filesystem::remove(filePath, errorCode);

			if(errorCode) {
				spdlog::error("Failed to delete temporary combined group file '{}': {}", combinedGroupFilePath, errorCode.message());
			}
		}
	}

	if(m_selectedMod != nullptr && !selectedGameVersion->doesRequireGroupFileExtraction() && !m_demoRecordingEnabled) {
		ModManager::deleteFilesWithSuffix("DMO", selectedGameVersion->getGamePath());
		ModManager::renameFilesWithSuffixTo("DMO" + DEFAULT_BACKUP_FILE_RENAME_SUFFIX, "DMO", selectedGameVersion->getGamePath());
	}

	removeSymlinksOrTemporaryFiles(*selectedGameVersion, &temporaryCopiedFilePaths);

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

std::unique_ptr<InstalledModInfo> ModManager::extractModFilesToGameDirectory(const ModGameVersion & modGameVersion, const GameVersion & selectedGameVersion, const GameVersion & targetGameVersion, const std::vector<std::string> & customGroupFileNames) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	struct FilePathComparator {
	public:
		bool operator () (const std::string & filePathA, const std::string & filePathB) const {
			return std::lexicographical_compare(filePathA.begin(), filePathA.end(), filePathB.begin(), filePathB.end(), [](unsigned char a, unsigned char b) {
				return std::tolower(a) < std::tolower(b);
			});
		}
	};

	if(!modGameVersion.isValid(true) || !targetGameVersion.isValid() || !removeModFilesFromGameDirectory(selectedGameVersion)) {
		return nullptr;
	}

	SettingsManager * settings = SettingsManager::getInstance();

	std::map<std::string, std::shared_ptr<ByteBuffer>, FilePathComparator> modFiles;
	std::string modPath(Utilities::joinPaths(settings->modsDirectoryPath, targetGameVersion.getModDirectoryName()));

	if(!customGroupFileNames.empty()) {
		for(const std::string & customGroupFileName : customGroupFileNames) {
			std::unique_ptr<Group> customGroup(Group::loadFrom(customGroupFileName));

			if(!Group::isValid(customGroup.get())) {
				spdlog::error("Failed to open custom group file: '{}'.", customGroupFileName);
				return nullptr;
			}

			std::shared_ptr<GroupFile> currentGroupFile;

			for(size_t i = 0; i < customGroup->numberOfFiles(); i++) {
				currentGroupFile = customGroup->getFile(i);

				modFiles[currentGroupFile->getFileName()] = std::make_shared<ByteBuffer>(currentGroupFile->getData());
			}
		}
	}
	else {
		std::string currentModFilePath;
		std::shared_ptr<ModFile> currentModFile;

		for(size_t i = 0; i < modGameVersion.numberOfFiles(); i++) {
			currentModFile = modGameVersion.getFile(i);
			currentModFilePath = Utilities::joinPaths(modPath, currentModFile->getFileName());

			if(Utilities::areStringsEqualIgnoreCase(currentModFile->getType(), "grp")) {
				std::unique_ptr<Group> modGroup(Group::loadFrom(currentModFilePath));

				if(!Group::isValid(modGroup.get())) {
					spdlog::error("Failed to open mod group file: '{}'.", currentModFilePath);
					return nullptr;
				}

				std::shared_ptr<GroupFile> currentGroupFile;

				for(size_t j = 0; j < modGroup->numberOfFiles(); j++) {
					currentGroupFile = modGroup->getFile(j);

					modFiles[currentGroupFile->getFileName()] = std::make_shared<ByteBuffer>(currentGroupFile->getData());
				}
			}
			else if(Utilities::areStringsEqualIgnoreCase(currentModFile->getType(), "zip")) {
				std::unique_ptr<Archive> modFilesArchive(ArchiveFactoryRegistry::getInstance()->readArchiveFrom(currentModFilePath));

				if(modFilesArchive == nullptr) {
					spdlog::error("Failed to open mod files archive: '{}'.", currentModFilePath);
					return nullptr;
				}

				std::vector<std::shared_ptr<ArchiveEntry>> modFilesArchiveEntries(modFilesArchive->getEntries());

				for(const std::shared_ptr<ArchiveEntry> & modFilesArchiveEntry : modFilesArchiveEntries) {
					if(modFilesArchiveEntry == nullptr || !modFilesArchiveEntry->isFile()) {
						continue;
					}

					modFiles[modFilesArchiveEntry->getPath()] = std::shared_ptr<ByteBuffer>(modFilesArchiveEntry->getData().release());
				}
			}
		}
	}

	if(modFiles.empty()) {
		spdlog::debug("No mod files to extract to '{}' game directory.", selectedGameVersion.getName());
		return nullptr;
	}

	std::vector<std::string> originalFilePaths;
	std::vector<std::string> renamedOriginalFilePaths;
	std::vector<std::string> modFilePaths;

	for(auto modFilesIterator = modFiles.cbegin(); modFilesIterator != modFiles.cend(); ++modFilesIterator) {
		modFilePaths.push_back(modFilesIterator->first);

		if(std::filesystem::is_regular_file(std::filesystem::path(Utilities::joinPaths(selectedGameVersion.getGamePath(), modFilesIterator->first)))) {
			if(std::filesystem::is_regular_file(std::filesystem::path(Utilities::joinPaths(selectedGameVersion.getGamePath(), modFilesIterator->first) + InstalledModInfo::DEFAULT_FILE_NAME))) {
				spdlog::error("Cannot temporarily rename original '{}' file, original backup file already exists at path: '{}'. Please manually restore or remove this file.", selectedGameVersion.getName(), Utilities::joinPaths(selectedGameVersion.getGamePath(), modFilesIterator->first) + DEFAULT_BACKUP_FILE_RENAME_SUFFIX);
				return nullptr;
			}

			originalFilePaths.push_back(modFilesIterator->first);
		}
	}

	for(const std::string & originalFilePath : originalFilePaths) {
		std::string relativeRenamedFilePath(originalFilePath + DEFAULT_BACKUP_FILE_RENAME_SUFFIX);
		std::string fullOriginalFilePath(Utilities::joinPaths(selectedGameVersion.getGamePath(), originalFilePath));
		std::string fullRenamedFilePath(Utilities::joinPaths(selectedGameVersion.getGamePath(), relativeRenamedFilePath));

		spdlog::debug("Renaming '{}' file '{}' to '{}'.", selectedGameVersion.getName(), originalFilePath, relativeRenamedFilePath);

		std::error_code errorCode;
		std::filesystem::rename(std::filesystem::path(fullOriginalFilePath), std::filesystem::path(fullRenamedFilePath), errorCode);

		if(!errorCode) {
			renamedOriginalFilePaths.push_back(originalFilePath);
		}
		else {
			spdlog::error("Failed to rename '{}' file '{}' to '{}': {}", selectedGameVersion.getName(), originalFilePath, relativeRenamedFilePath, errorCode.message());
		}
	}

	std::unique_ptr<InstalledModInfo> installedModInfo(std::make_unique<InstalledModInfo>(*modGameVersion.getParentModVersion(), originalFilePaths, modFilePaths));

	if(!installedModInfo->saveTo(selectedGameVersion)) {
		spdlog::error("Failed to save installed mod info to '{}' game directory.", selectedGameVersion.getName());

		return nullptr;
	}

	spdlog::info("Saved installed mod info to file '{}' in '{}' game directory.", InstalledModInfo::DEFAULT_FILE_NAME, selectedGameVersion.getName());

	size_t fileNumber = 1;

	for(auto modFilesIterator = modFiles.cbegin(); modFilesIterator != modFiles.cend(); ++modFilesIterator, fileNumber++) {
		std::string fullModFilePath(Utilities::joinPaths(selectedGameVersion.getGamePath(), modFilesIterator->first));

		if(!modFilesIterator->second->writeTo(fullModFilePath)) {
			spdlog::error("Failed to write mod file #{} of {} ('{}') to '{}' game directory.", fileNumber, modFiles.size(), modFilesIterator->first, selectedGameVersion.getName());
			continue;
		}

		spdlog::info("Extracted mod file #{} of {} ('{}') to '{}' game directory.", fileNumber, modFiles.size(), modFilesIterator->first, selectedGameVersion.getName());
	}

	return installedModInfo;
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

	if(!gameVersion.isConfigured() || !installedModInfo.isValid() || !std::filesystem::is_directory(std::filesystem::path(gameVersion.getGamePath()))) {
		return false;
	}

	spdlog::info("Removing {} '{}' mod files from '{}' game directory...", installedModInfo.numberOfModFiles(), installedModInfo.getFullModName(), gameVersion.getName());

	for(size_t i = 0; i < installedModInfo.numberOfModFiles(); i++) {
		std::string relativeModFilePath(installedModInfo.getModFile(i));
		std::string fullModFilePath(Utilities::joinPaths(gameVersion.getGamePath(), relativeModFilePath));
		bool modFileExists = std::filesystem::is_regular_file(std::filesystem::path(fullModFilePath));

		if(!modFileExists) {
			spdlog::warn("Mod file #{} of {} ('{}') no longer exists in '{}' game directory.", i + 1, installedModInfo.numberOfModFiles(), relativeModFilePath, gameVersion.getName());
			continue;
		}

		std::error_code errorCode;
		std::filesystem::remove(std::filesystem::path(fullModFilePath), errorCode);

		if(errorCode) {
			spdlog::error("Failed to remove mod file #{} of {} ('{}') from '{}' game directory: {}", i + 1, installedModInfo.numberOfModFiles(), relativeModFilePath, gameVersion.getName(), errorCode.message());
			continue;
		}

		spdlog::debug("Removed mod file #{} of {} ('{}') from '{}' game directory.", i + 1, installedModInfo.numberOfModFiles(), relativeModFilePath, gameVersion.getName());
	}

	spdlog::info("Renaming {} '{}' backed up game files back to their original names...", installedModInfo.numberOfOriginalFiles(), gameVersion.getName());

	for(size_t i = 0; i < installedModInfo.numberOfOriginalFiles(); i++) {
		std::string relativeOriginalFilePath(installedModInfo.getOriginalFile(i));
		std::string relativeRenamedFilePath(relativeOriginalFilePath + DEFAULT_BACKUP_FILE_RENAME_SUFFIX);
		std::string fullOriginalFilePath(Utilities::joinPaths(gameVersion.getGamePath(), relativeOriginalFilePath));
		std::string fullRenamedFilePath(Utilities::joinPaths(gameVersion.getGamePath(), relativeRenamedFilePath));
		bool originalFileExists = std::filesystem::is_regular_file(std::filesystem::path(fullOriginalFilePath));
		bool renamedFileExists = std::filesystem::is_regular_file(std::filesystem::path(fullRenamedFilePath));

		if(!renamedFileExists) {
			spdlog::error("Renamed backup file #{} of {} ('{}') no longer exists in '{}' game directory.", i + 1, installedModInfo.numberOfOriginalFiles(), relativeRenamedFilePath, gameVersion.getName());
			continue;
		}

		if(originalFileExists) {
			spdlog::error("Original file #{} of {} ('{}') already exists in '{}' game directory, it will be replaced.");

			std::error_code errorCode;
			std::filesystem::remove(std::filesystem::path(fullOriginalFilePath), errorCode);

			if(errorCode) {
				spdlog::error("Failed to remove original file ('{}') from '{}' game directory: {}", relativeOriginalFilePath, gameVersion.getName(), errorCode.message());
				continue;
			}
		}

		std::error_code errorCode;
		std::filesystem::rename(std::filesystem::path(fullRenamedFilePath), std::filesystem::path(fullOriginalFilePath), errorCode);

		if(errorCode) {
			spdlog::error("Failed to rename original backed up '{}' game file #{} of {} from '{}' to '{}': {}", gameVersion.getName(), i + 1, installedModInfo.numberOfOriginalFiles(), relativeRenamedFilePath, relativeOriginalFilePath, errorCode.message());
			continue;
		}

		spdlog::info("Renamed original backed up '{}' game file #{} of {} from '{}' to '{}'.", gameVersion.getName(), i + 1, installedModInfo.numberOfOriginalFiles(), relativeRenamedFilePath, relativeOriginalFilePath);
	}

	std::filesystem::path installedModInfoFilePath(Utilities::joinPaths(gameVersion.getGamePath(), InstalledModInfo::DEFAULT_FILE_NAME));

	if(!std::filesystem::exists(installedModInfoFilePath)) {
		spdlog::warn("Installed mod info file '{}' no longer exists in '{}' game directory.", InstalledModInfo::DEFAULT_FILE_NAME, gameVersion.getGamePath());
		return true;
	}

	std::error_code errorCode;
	std::filesystem::remove(installedModInfoFilePath, errorCode);

	if(errorCode) {
		spdlog::error("Failed to remove installed mod info file '{}' from '{}' game directory: {}", InstalledModInfo::DEFAULT_FILE_NAME, gameVersion.getName(), errorCode.message());
		return true;
	}

	spdlog::info("Removed installed mod info file '{}' from '{}' game directory.", InstalledModInfo::DEFAULT_FILE_NAME, gameVersion.getName());

	return true;
}

std::string ModManager::generateCommand(std::shared_ptr<ModGameVersion> modGameVersion, std::shared_ptr<GameVersion> selectedGameVersion, ScriptArguments & scriptArgs, std::string_view combinedGroupFileName, bool * customMod, std::string * customMap, std::shared_ptr<GameVersion> * customTargetGameVersion, std::vector<std::string> * customGroupFileNames) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	static const std::regex respawnModeRegExp("[123x]");

	if(!m_initialized) {
		spdlog::error("Mod manager not initialized.");
		return {};
	}

	SettingsManager * settings = SettingsManager::getInstance();

	if(!selectedGameVersion->isConfigured()) {
		spdlog::error("Invalid or unconfigured game version.");
		return {};
	}

	if(settings->dataDirectoryPath.empty()) {
		spdlog::error("Empty data path.");
		return {};
	}

	std::shared_ptr<DOSBoxVersion> selectedDOSBoxVersion(getSelectedDOSBoxVersion());

	if(selectedGameVersion->doesRequireDOSBox() && !DOSBoxVersion::isConfigured(selectedDOSBoxVersion.get())) {
		spdlog::error("Selected DOSBox version '{}' is not configured.", selectedDOSBoxVersion != nullptr ? selectedDOSBoxVersion->getName() : "N/A");
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

	if(!GameVersionCollection::isValid(getGameVersions().get())) {
		spdlog::error("Invalid game version collection.");
		return {};
	}

	std::shared_ptr<GameVersion> targetGameVersion;

	if(modGameVersion != nullptr) {
		if(!modGameVersion->isValid(true)) {
			spdlog::error("Invalid mod game version.");
			return {};
		}

		if(!Utilities::areStringsEqualIgnoreCase(modGameVersion->getGameVersion(), selectedGameVersion->getName()) && !selectedGameVersion->hasCompatibleGameVersionWithName(modGameVersion->getGameVersion())) {
			spdlog::error("Game version '{}' is not compatible with '{}'.", selectedGameVersion->getName(), modGameVersion->getGameVersion());
			return {};
		}

		targetGameVersion = getGameVersions()->getGameVersionWithName(modGameVersion->getGameVersion());

		if(targetGameVersion == nullptr) {
			spdlog::error("Missing game configuration for '{}'.", modGameVersion->getGameVersion());
			return {};
		}
	}
	else {
		targetGameVersion = selectedGameVersion;
	}

	if(customTargetGameVersion != nullptr) {
		*customTargetGameVersion = targetGameVersion;
	}

	std::string executableName;

	if(m_gameType == GameType::Game) {
		executableName = selectedGameVersion->getGameExecutableName();
	}
	else {
		if(!selectedGameVersion->hasSetupExecutableName()) {
			spdlog::error("Game version '{}' does not have a setup executable.", selectedGameVersion->getName());
			return {};
		}

		executableName = selectedGameVersion->getSetupExecutableName().value();
	}

	std::stringstream command;

	std::vector<std::string> customGroupFiles;
	std::vector<std::string> customConFiles;
	std::vector<std::string> customDefFiles;

	if(m_arguments != nullptr) {
		if((m_arguments->hasArgument("g") || m_arguments->hasArgument("group")) ||
		   (m_arguments->hasArgument("x") || m_arguments->hasArgument("con")) ||
		   (m_arguments->hasArgument("h") || m_arguments->hasArgument("def"))) {
			customGroupFiles = m_arguments->getValues("g");

			if(customGroupFiles.empty()) {
				customGroupFiles = m_arguments->getValues("group");
			}

			if(!customGroupFiles.empty()) {
				customConFiles = m_arguments->getValues("x");
				customDefFiles = m_arguments->getValues("h");

				if(customConFiles.empty()) {
					m_arguments->getValues("con");
				}

				if(customDefFiles.empty()) {
					m_arguments->getValues("def");
				}

				for(std::vector<std::string>::const_iterator i = customGroupFiles.begin(); i != customGroupFiles.end(); ++i) {
					scriptArgs.addArgument("GROUP", *i);
				}

				for(std::vector<std::string>::const_iterator i = customConFiles.begin(); i != customConFiles.end(); ++i) {
					scriptArgs.addArgument("CON", *i);
				}

				for(std::vector<std::string>::const_iterator i = customDefFiles.begin(); i != customDefFiles.end(); ++i) {
					scriptArgs.addArgument("DEF", *i);
				}

				if(customMod != nullptr) {
					*customMod = true;
				}

				if(customGroupFileNames != nullptr) {
					*customGroupFileNames = customGroupFiles;
				}
			}
		}
	}

	if(modGameVersion != nullptr || !customGroupFiles.empty()) {
		if(!selectedGameVersion->doesRequireGroupFileExtraction()) {
			std::string modPath;

			if(areSymlinksSupported() && selectedGameVersion->doesSupportSubdirectories()) {
				modPath = Utilities::joinPaths(settings->modsSymlinkName, targetGameVersion->getModDirectoryName());
			}
			else if(selectedGameVersion->doesSupportSubdirectories()) {
				modPath = settings->gameTempDirectoryName;
			}

			std::vector<std::string> conFileNames;

			if(!customGroupFiles.empty()) {
				conFileNames = customConFiles;
			}
			else {
				std::vector<std::shared_ptr<ModFile>> conFiles(modGameVersion->getFilesOfType("con"));

				for(const std::shared_ptr<ModFile> & conFile : conFiles) {
					conFileNames.push_back(conFile->getFileName());
				}
			}

			if(!selectedGameVersion->hasGroupFileArgumentFlag()) {
				spdlog::error("Game version '{}' does not have a group file argument flag specified in its configuration.", selectedGameVersion->getName());
				return {};
			}

			if(!conFileNames.empty()) {
				if(!selectedGameVersion->hasConFileArgumentFlag()) {
					spdlog::error("Game version '{}' does not have a con file argument flag specified in its configuration.", selectedGameVersion->getName());
					return {};
				}
				else if(conFileNames.size() > 1 && !selectedGameVersion->hasExtraConFileArgumentFlag()) {
					spdlog::error("Multiple con files specified, but game version '{}' does not have an extra con file argument flag specified in its configuration.", selectedGameVersion->getName());
					return {};
				}

				for(std::vector<std::string>::const_iterator i = conFileNames.cbegin(); i != conFileNames.cend(); ++i) {
					const std::string & conFileName = *i;

					command << " ";

					if(i == conFileNames.begin()) {
						command << selectedGameVersion->getConFileArgumentFlag().value();
					}
					else {
						command << selectedGameVersion->getExtraConFileArgumentFlag().value();
					}

					command << (selectedGameVersion->hasRelativeConFilePath() ? conFileName : Utilities::joinPaths(modPath, conFileName));
				}
			}

			if(!customGroupFiles.empty()) {
				for(std::vector<std::string>::const_iterator i = customGroupFiles.begin(); i != customGroupFiles.end(); ++i) {
					const std::string & groupFileName = *i;

					command << " " << selectedGameVersion->getGroupFileArgumentFlag().value() << Utilities::joinPaths(modPath, groupFileName);
				}
			}
			else if(!combinedGroupFileName.empty()) {
				std::string combinedGroupFilePath;

				if(areSymlinksSupported() && selectedGameVersion->doesSupportSubdirectories()) {
					combinedGroupFilePath = Utilities::joinPaths(settings->tempSymlinkName, combinedGroupFileName);
				}
				else if(selectedGameVersion->doesSupportSubdirectories()) {
					combinedGroupFilePath = Utilities::joinPaths(settings->gameTempDirectoryName, combinedGroupFileName);
				}
				else {
					combinedGroupFilePath = combinedGroupFileName;
				}

				command << " " << selectedGameVersion->getGroupFileArgumentFlag().value() << Utilities::joinPaths(settings->tempSymlinkName, combinedGroupFileName);
			}
			else {
				std::vector<std::shared_ptr<ModFile>> groupFiles(modGameVersion->getFilesOfType("grp"));

				for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = groupFiles.begin(); i != groupFiles.end(); ++i) {
					command << " " << selectedGameVersion->getGroupFileArgumentFlag().value() << Utilities::joinPaths(modPath, (*i)->getFileName());
				}
			}

			if(!customGroupFiles.empty() || modGameVersion->isEDuke32()) {
				if(customGroupFiles.empty()) {
					std::vector<std::shared_ptr<ModFile>> zipFiles(modGameVersion->getFilesOfType("zip"));

					for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = zipFiles.begin(); i != zipFiles.end(); ++i) {
						command << " " << selectedGameVersion->getGroupFileArgumentFlag().value() << Utilities::joinPaths(modPath, (*i)->getFileName());
					}
				}

				std::vector<std::string> defFileNames;

				if(!customGroupFiles.empty()) {
					defFileNames = customDefFiles;
				}
				else {
					std::vector<std::shared_ptr<ModFile>> defFiles(modGameVersion->getFilesOfType("def"));

					for(const std::shared_ptr<ModFile> & defFile : defFiles) {
						defFileNames.push_back(defFile->getFileName());
					}
				}

				if(!defFileNames.empty()) {
					if(!selectedGameVersion->hasDefFileArgumentFlag()) {
						spdlog::error("Game version '{}' does not have a def file argument flag specified in its configuration.", selectedGameVersion->getName());
						return {};
					}
					else if(defFileNames.size() > 1 && !selectedGameVersion->hasExtraDefFileArgumentFlag()) {
						spdlog::error("Multiple def files specified, but game version '{}' does not have an extra def file argument flag specified in its configuration.", selectedGameVersion->getName());
						return {};
					}

					for(std::vector<std::string>::const_iterator i = defFileNames.cbegin(); i != defFileNames.cend(); ++i) {
						const std::string & defFileName = *i;

						command << " ";

						if(i == defFileNames.begin()) {
							command << selectedGameVersion->getDefFileArgumentFlag().value();
						}
						else {
							command << selectedGameVersion->getExtraDefFileArgumentFlag().value();
						}

						command << defFileName;
					}
				}
			}
		}
	}

	if(m_arguments != nullptr) {
		if(m_arguments->hasArgument("map")) {
			if(settings->mapsSymlinkName.empty()) {
				spdlog::error("Maps directory symbolic link name is empty.");
				return {};
			}

			std::string userMap(m_arguments->getFirstValue("map"));

			if(!userMap.empty()) {
				if(selectedGameVersion->hasMapFileArgumentFlag()) {
					scriptArgs.addArgument("MAP", userMap);

					if(customMap != nullptr) {
						*customMap = userMap;
					}

					command << " " << selectedGameVersion->getMapFileArgumentFlag().value();

					if(std::filesystem::is_regular_file(std::filesystem::path(Utilities::joinPaths(selectedGameVersion->getGamePath(), userMap)))) {
						command << userMap;
					}
					else {
						std::string mapsDirectoryPath(getMapsDirectoryPath());

						if(mapsDirectoryPath.empty()) {
							spdlog::error("Maps directory path is empty.");
							return {};
						}

						if(std::filesystem::is_regular_file(std::filesystem::path(Utilities::joinPaths(mapsDirectoryPath, userMap)))) {
							if(areSymlinksSupported() && selectedGameVersion->doesSupportSubdirectories()) {
								command << Utilities::joinPaths(settings->mapsSymlinkName, userMap);
							}
							else if(selectedGameVersion->doesSupportSubdirectories()) {
								command << Utilities::joinPaths(settings->gameTempDirectoryName, userMap);
							}
							else {
								command << userMap;
							}
						}
						else {
							spdlog::error("Map '{}' does not exist in game or maps directories.", userMap);

							command << userMap;
						}
					}
				}
				else {
					spdlog::warn("Game version '{}' does not have a map file argument flag specified in its configuration.", selectedGameVersion->getName());
				}
			}
		}

		if(m_arguments->hasArgument("v") || m_arguments->hasArgument("episode")) {
			std::string episodeNumberData(m_arguments->getFirstValue("v"));

			if(episodeNumberData.empty()) {
				episodeNumberData = m_arguments->getFirstValue("episode");
			}

			std::optional<uint8_t> optionalEpisodeNumber(Utilities::parseUnsignedByte(episodeNumberData));

			if(optionalEpisodeNumber.has_value() && optionalEpisodeNumber.value() >= 1) {
				command << " " << selectedGameVersion->getEpisodeArgumentFlag() << std::to_string(optionalEpisodeNumber.value());
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
				command << " " << selectedGameVersion->getLevelArgumentFlag() << std::to_string(optionalLevelNumber.value());
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
				command << " " << selectedGameVersion->getSkillArgumentFlag() << std::to_string(optionalSkillNumber.value() - 1 + selectedGameVersion->getSkillStartValue());
			}
			else {
				spdlog::warn("Invalid skill number: '{}'.", skillNumberData);
			}
		}

		if(m_demoRecordingEnabled) {
			command << " " << selectedGameVersion->getRecordDemoArgumentFlag();
		}

		if(m_arguments->hasArgument("d") || m_arguments->hasArgument("demo")) {
			std::string demoFileName(m_arguments->getFirstValue("d"));

			if(demoFileName.empty()) {
				demoFileName = m_arguments->getFirstValue("demo");
			}

			if(!demoFileName.empty()) {
				if(selectedGameVersion->hasPlayDemoArgumentFlag()) {
					command << " " << selectedGameVersion->getPlayDemoArgumentFlag().value() << demoFileName;
				}
				else {
					spdlog::warn("Game version '{}' does not have a play demo argument flag specified in its configuration.", selectedGameVersion->getName());
				}
			}
		}

		if(m_arguments->hasArgument("t") || m_arguments->hasArgument("respawn")) {
			std::string respawnMode(m_arguments->getFirstValue("t"));

			if(respawnMode.empty()) {
				respawnMode = m_arguments->getFirstValue("respawn");
			}

			if(!respawnMode.empty() && std::regex_match(respawnMode, respawnModeRegExp)) {
				if(selectedGameVersion->hasRespawnModeArgumentFlag()) {
					command << " " << selectedGameVersion->getRespawnModeArgumentFlag().value() << respawnMode;
				}
				else {
					spdlog::warn("Game version '{}' does not have a respawn mode argument flag specified in its configuration.", selectedGameVersion->getName());
				}
			}
		}

		if(m_arguments->hasArgument("u") || m_arguments->hasArgument("weaponswitch")) {
			std::string weaponSwitchOrder(m_arguments->getFirstValue("u"));

			if(weaponSwitchOrder.empty()) {
				weaponSwitchOrder = m_arguments->getFirstValue("weaponswitch");
			}

			if(!weaponSwitchOrder.empty() && weaponSwitchOrder.find_first_not_of("0123456789") == std::string::npos) {
				if(selectedGameVersion->hasWeaponSwitchOrderArgumentFlag()) {
					command << " " << selectedGameVersion->getWeaponSwitchOrderArgumentFlag().value() << weaponSwitchOrder;
				}
				else {
					spdlog::warn("Game version '{}' does not have a weapon switch order argument flag specified in its configuration.", selectedGameVersion->getName());
				}
			}
		}

		if(m_arguments->hasArgument("m") || m_arguments->hasArgument("nomonsters")) {
			if(selectedGameVersion->hasDisableMonstersArgumentFlag()) {
				command << " " << selectedGameVersion->getDisableMonstersArgumentFlag().value();
			}
			else {
				spdlog::warn("Game version '{}' does not have a disable monsters argument flag specified in its configuration.", selectedGameVersion->getName());
			}
		}

		if(m_arguments->hasArgument("ns") || m_arguments->hasArgument("nosound")) {
			if(selectedGameVersion->hasDisableSoundArgumentFlag()) {
				command << " " << selectedGameVersion->getDisableSoundArgumentFlag().value();
			}
			else {
				spdlog::warn("Game version '{}' does not have a disable sound argument flag specified in its configuration.", selectedGameVersion->getName());
			}
		}

		if(m_arguments->hasArgument("nm") || m_arguments->hasArgument("nomusic")) {
			if(selectedGameVersion->hasDisableMusicArgumentFlag()) {
				command << " " << selectedGameVersion->getDisableMusicArgumentFlag().value();
			}
			else {
				spdlog::warn("Game version '{}' does not have a disable music argument flag specified in its configuration.", selectedGameVersion->getName());
			}
		}
	}

	if(selectedGameVersion->doesRequireDOSBox()) {
		std::shared_ptr<DOSBoxVersion> selectedDOSBoxVersion(getSelectedDOSBoxVersion());

		if(selectedDOSBoxVersion == nullptr) {
			spdlog::error("No DOSBox version selected.");
			return {};
		}

		if(!selectedDOSBoxVersion->isConfigured()) {
			spdlog::error("Selected DOSBox version '{}' is not configured.", selectedGameVersion->getName());
			return {};
		}

		Script dosboxScript;

		scriptArgs.addArgument("COMMAND", executableName + command.str());

		std::string dosboxDataDirectoryPath(Utilities::joinPaths(settings->dataDirectoryPath, settings->dosboxDataDirectoryName));
		std::string dosboxTemplateScriptFilePath(Utilities::joinPaths(dosboxDataDirectoryPath, getDOSBoxTemplateScriptFileName(m_gameType)));

		if(!dosboxScript.readFrom(dosboxTemplateScriptFilePath)) {
			spdlog::error("Failed to load DOSBox template script file: '{}'.", dosboxTemplateScriptFilePath);
			return {};
		}

		return generateDOSBoxCommand(dosboxScript, scriptArgs, *selectedDOSBoxVersion, settings->dosboxArguments);
	}

	return "\"" +  Utilities::joinPaths(selectedGameVersion->getGamePath(), executableName) + "\"" + command.str();
}

std::string ModManager::generateDOSBoxCommand(const Script & script, const ScriptArguments & arguments, const DOSBoxVersion & dosboxVersion, const std::string & dosboxArguments) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	static const std::regex unescapedQuotesRegExp("(?:^\"|([^\\\\])\")");

	if(!dosboxVersion.isConfigured()) {
		return std::string();
	}

	std::stringstream command;

	command << fmt::format("\"{}\" {} ", Utilities::joinPaths(dosboxVersion.getDirectoryPath(), dosboxVersion.getExecutableName()), dosboxArguments);

	std::string line;
	std::string formattedLine;

	for(size_t i = 0; i < script.numberOfCommands(); i++) {
		line = arguments.applyArguments(*script.getCommand(i));

		formattedLine.clear();
		std::regex_replace(std::back_inserter(formattedLine), line.begin(), line.end(), unescapedQuotesRegExp, "$1\\\"");

		if(!formattedLine.empty()) {
			if(command.tellp() != 0) {
				command << " ";
			}

			command << fmt::format("-c \"{}\"", formattedLine);
		}
	}

	return Utilities::trimString(command.str());
}

bool ModManager::handleArguments(const ArgumentParser * args) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	SettingsManager * settings = SettingsManager::getInstance();

	if(args != nullptr) {
		if(args->hasArgument("update-new")) {
			updateFileInfoForAllMods(true, true);
		}

		if(args->hasArgument("update-all")) {
			updateFileInfoForAllMods(true, false);
			return true;
		}

		if(args->hasArgument("type")) {
			std::optional<GameType> newGameTypeOptional(magic_enum::enum_cast<GameType>(Utilities::toPascalCase(args->getFirstValue("type"))));

			if(newGameTypeOptional.has_value()) {
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
			else {
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
		}

		if(args->hasArgument("search")) {
			if(args->hasArgument("random") || (args->hasArgument("g") || args->hasArgument("group")) || (args->hasArgument("x") || args->hasArgument("con")) || (args->hasArgument("n") || args->hasArgument("normal"))) {
				spdlog::error("Redundant arguments specified, please specify either search OR random OR n/normal OR (x/con AND/OR g/group).");
				return false;
			}

			std::vector<ModMatch> modMatches(searchForMod(m_mods->getMods(), args->getFirstValue("search")));

			if(modMatches.empty()) {
				spdlog::error("No matches found for specified search query.");
				return false;
			}
			else if(modMatches.size() == 1) {
				const ModMatch & modMatch = modMatches[0];

				fmt::print("Selected {} from search query: '{}'.\n", Utilities::toCapitalCase(magic_enum::enum_name(modMatch.getMatchType())), modMatch.toString());

				setSelectedMod(modMatch.getMod());
				setSelectedModVersionIndex(modMatch.getModVersionIndex());
				setSelectedModVersionTypeIndex(modMatch.getModVersionTypeIndex());

				runSelectedModAndWait();

				return true;
			}
			else {
				if(modMatches.size() > 20) {
					fmt::print("Found {} matches, please refine your search query.\n", modMatches.size());
				}
				else {
					fmt::print("Found {} matches:\n", modMatches.size());

					for(size_t i = 0; i < modMatches.size(); i++) {
						size_t spacingLength = Utilities::unsignedLongLength(modMatches.size()) - Utilities::unsignedLongLength(i + 1);

						for(size_t i = 0; i < spacingLength; i++) {
							fmt::print(" ");
						}

						fmt::print("{}. {}\n", i + 1, modMatches[i].toString());
					}

					fmt::print("\n");
					fmt::print("Please refine your search query.\n");
				}

				return false;
			}
		}
		else if(args->hasArgument("random")) {
			if((args->hasArgument("g") || args->hasArgument("group")) || (args->hasArgument("x") || args->hasArgument("con")) || (args->hasArgument("n") || args->hasArgument("normal"))) {
				spdlog::error("Redundant arguments specified, please specify either search OR random OR n/normal OR (x/con AND/OR g/group).");
				return false;
			}

			selectRandomMod(true, true);

			spdlog::info("Selected random mod: '{}'\n", m_selectedMod->getFullName(m_selectedModVersionIndex, m_selectedModVersionTypeIndex));
			fmt::print("\n");

			runSelectedModAndWait();

			return true;
		}
		else if(args->hasArgument("n") || args->hasArgument("normal")) {
			if((args->hasArgument("g") || args->hasArgument("group")) || (args->hasArgument("x") || args->hasArgument("con"))) {
				spdlog::error("Redundant arguments specified, please specify either search OR random OR n/normal OR (x/con AND/OR g/group).");
				return false;
			}

			clearSelectedMod();
			runSelectedModAndWait();

			return true;
		}
		else if((args->hasArgument("g") || args->hasArgument("group")) || (args->hasArgument("x") || args->hasArgument("con"))) {
			runSelectedModAndWait();

			return true;
		}
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
				modGameVersion = modVersionType->getGameVersion(gameVersion.getName());

				if(modGameVersion == nullptr) {
					continue;
				}

				for(size_t l = 0; l < modGameVersion->numberOfFiles(); l++) {
					modFile = modGameVersion->getFile(l);
					fileName = modFile->getFileName();

					if(linkedModFiles.find(fileName) != linkedModFiles.end() && linkedModFiles[fileName].size() != 0) {
						// eDuke32 mod files can be read straight out of the group or zip file, and are not stored separately
						if(modGameVersion->isEDuke32() && modFile->getType() != "zip" && modFile->getType() != "grp") {
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

			spdlog::warn("Unlinked '{}' file {}: '{}' for '{}'.", gameVersion.getName(), numberOfUnlinkedModFiles, i->first, gameVersion.getName());
		}
	}

	if(numberOfUnlinkedModFiles != 0) {
		spdlog::warn("Found {} unlinked '{}' mod file{} in '{}' mods directory.", numberOfUnlinkedModFiles, gameVersion.getName(), numberOfUnlinkedModFiles == 1 ? "" : "s", gameVersion.getName());
	}

	bool multipleLinkedFile = false;
	size_t numberOfMultipleLinkedModFiles = 0;

	for(std::map<std::string, std::vector<std::shared_ptr<ModFile>>>::const_iterator i = linkedModFiles.begin(); i != linkedModFiles.end(); ++i) {
		if(i->second.size() > 1) {
			for(std::vector<std::shared_ptr<ModFile>>::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
				multipleLinkedFile = false;

				for(std::vector<std::shared_ptr<ModFile>>::const_iterator k = j + 1; k != i->second.end(); ++k) {
					if((*j)->getParentMod() != (*k)->getParentMod()) {
						multipleLinkedFile = true;

						break;
					}
					else if((*j)->getParentModVersion() != (*k)->getParentModVersion()) {
						if(!(*j)->isShared() || !(*k)->isShared()) {
							multipleLinkedFile = true;

							break;
						}
					}
				}

				if(multipleLinkedFile) {
					spdlog::warn("'{}' Mod file '{}' linked {} times.", gameVersion.getName(), i->first, i->second.size());

					numberOfMultipleLinkedModFiles++;
				}
			}
		}
	}

	if(numberOfMultipleLinkedModFiles != 0) {
		spdlog::warn("Found {} multiple linked '{}' mod file{} in '{}' mods directory. If a mod file is linked intentionally multiple times within the same game version, it must have its shared property set to true.", numberOfMultipleLinkedModFiles, gameVersion.getName(), numberOfMultipleLinkedModFiles == 1 ? "" : "s", gameVersion.getName());
	}

	return numberOfUnlinkedModFiles;
}

size_t ModManager::checkModForMissingFiles(const std::string & modName, std::optional<size_t> versionIndex, std::optional<size_t> versionTypeIndex) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(!m_initialized) {
		return 0;
	}

	std::shared_ptr<Mod> mod = m_mods->getMod(modName);

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
				gameVersion = getGameVersions()->getGameVersionWithName(modGameVersion->getGameVersion());

				if(!GameVersion::isValid(gameVersion.get())) {
					spdlog::warn("Skipping checking invalid '{}' mod game version '{}', invalid game configuration.", mod.getFullName(i, j), modGameVersion->getGameVersion());
					continue;
				}

				gameModsPath = Utilities::joinPaths(settings->modsDirectoryPath, gameVersion->getModDirectoryName());

				if(!std::filesystem::is_directory(gameModsPath)) {
					spdlog::warn("Skipping checking '{}' mod game version '{}', base directory is missing or not a valid directory: '{}'.", mod.getFullName(i, j), gameVersion->getName(), gameModsPath);
					continue;
				}

				for(size_t l = 0; l < modGameVersion->numberOfFiles(); l++) {
					modFile = modGameVersion->getFile(l);
					modFilePath = Utilities::joinPaths(gameModsPath, modFile->getFileName());

					if(!std::filesystem::is_regular_file(std::filesystem::path(modFilePath))) {
						// skip eDuke 32 mod files which are contained within the main zip file
						if(modGameVersion->isEDuke32() && modFile->getType() != "zip" && modFile->getType() != "grp") {
							continue;
						}

						spdlog::warn("Mod '{}' is missing {} {} file: '{}'.", mod.getFullName(i, j), modGameVersion->getGameVersion(), modFile->getType(), modFile->getFileName());

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
		spdlog::info("'{}' mod file info already populated!", mod.getName());
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

	spdlog::info("Updating '{}' mod file info...", mod.getName());

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

			std::shared_ptr<GameVersion> gameVersion(getGameVersions()->getGameVersionWithName(modDownload->getGameVersion()));

			if(gameVersion == nullptr) {
				spdlog::warn("Could not find game configuration for game version '{}', skipping update of download file info: '{}'.", modDownload->getGameVersion(), modDownload->getFileName());
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
				std::shared_ptr<GameVersion> gameVersion(getGameVersions()->getGameVersionWithName(modGameVersion->getGameVersion()));

				if(!GameVersion::isValid(gameVersion.get())) {
					spdlog::warn("Mod '{}' game version #{} is not valid, skipping update of mod files info.", mod.getFullName(i, j), k + 1);
					continue;
				}

				gameModsPath = Utilities::joinPaths(settings->modsDirectoryPath, gameVersion->getModDirectoryName());

				if(!std::filesystem::is_directory(gameModsPath)) {
					spdlog::warn("Mod '{}' '{}' game version directory '{}' does not exist or is not a valid directory, skipping update of mod files info.", mod.getFullName(i, j), gameVersion->getName(), gameModsPath);
					continue;
				}

				if(modGameVersion->isEDuke32()) {
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

							group = Group::loadFrom(groupFilePath);

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

					// eDuke32 mod files can be read straight out of the group or zip file, and are not stored separately
					if(modGameVersion->isEDuke32() && modFile->getType() != "zip" && modFile->getType() != "grp") {
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

std::string ModManager::getArgumentHelpInfo() {
	std::stringstream argumentHelpStream;

	argumentHelpStream << APPLICATION_NAME << " version " << APPLICATION_VERSION << " arguments:\n";
	argumentHelpStream << " --file \"Settings.json\" - specifies an alternate settings file to use.\n";
	argumentHelpStream << " -f \"File.json\" - alias for 'file'.\n";
	argumentHelpStream << " --type Game/Setup/Client/Server - specifies game type, default: Game.\n";
	argumentHelpStream << " --dosbox \"DOSBox Version\" - specifies the DOSBox version to use.\n";
	argumentHelpStream << " --game \"Game Version\" - specifies the game version to run.\n";
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
	argumentHelpStream << " --normal - runs normal Duke Nukem 3D without any mods.\n";
	argumentHelpStream << " -n - alias for 'normal'.\n";
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
	argumentHelpStream << " --version - displays the application version.\n";
	argumentHelpStream << " --help - displays this help message.\n";
	argumentHelpStream << " -? - alias for 'help'.\n";

	return argumentHelpStream.str();
}

void ModManager::displayArgumentHelp() {
	fmt::print(getArgumentHelpInfo());
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

bool ModManager::areSymlinksSupported() const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	static const std::string TEST_SYMLINK_NAME("TestSymlink");

	static std::optional<bool> s_symlinksSupported;

	if(!s_symlinksSupported.has_value()) {
		s_symlinksSupported = createSymlink(std::filesystem::current_path().string(), TEST_SYMLINK_NAME, "");

		removeSymlink(TEST_SYMLINK_NAME, "");

		spdlog::debug("Symbolic link creation is {}.", s_symlinksSupported.value() ? "supported" : "unsupported");
	}

	return s_symlinksSupported.value();
}

bool ModManager::createSymlink(const std::string & symlinkTarget, const std::string & symlinkName, const std::string & symlinkDestinationDirectory) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

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

bool ModManager::removeSymlink(const std::string & symlinkName, const std::string & symlinkDestinationDirectory) const {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

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

bool ModManager::createSymlinksOrCopyTemporaryFiles(const GameVersionCollection & gameVersions, const GameVersion & gameVersion, const ModGameVersion * selectedModGameVersion, const std::string & customMap, bool createTempSymlink, std::vector<std::string> * temporaryCopiedFilePaths) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	SettingsManager * settings = SettingsManager::getInstance();

	if(!gameVersion.isConfigured() || !areSymlinkSettingsValid() || (createTempSymlink && settings->appTempDirectoryPath.empty())) {
		return false;
	}

	std::string mapsDirectoryPath(getMapsDirectoryPath());

	if(areSymlinksSupported() && gameVersion.doesSupportSubdirectories()) {
		bool result = true;

		result &= createSymlink(gameVersion.getGamePath(), settings->gameSymlinkName, std::filesystem::current_path().string());

		if(createTempSymlink) {
			result &= createSymlink(settings->appTempDirectoryPath, settings->tempSymlinkName, gameVersion.getGamePath());
		}

		result &= createSymlink(getModsDirectoryPath(), settings->modsSymlinkName, gameVersion.getGamePath());

		if(!mapsDirectoryPath.empty()) {
			result &= createSymlink(mapsDirectoryPath, settings->mapsSymlinkName, gameVersion.getGamePath());
		}

		return result;
	}

	if(gameVersion.doesSupportSubdirectories() && !createGameTemporaryDirectory(gameVersion)) {
		return false;
	}

	std::string fileDestinationDirectoryPath;

	if(gameVersion.doesSupportSubdirectories()) {
		fileDestinationDirectoryPath = Utilities::joinPaths(gameVersion.getGamePath(), settings->gameTempDirectoryName);
	}
	else {
		fileDestinationDirectoryPath = gameVersion.getGamePath();
	}

	if(selectedModGameVersion != nullptr && !gameVersion.doesRequireGroupFileExtraction()) {
		for(size_t i = 0; i < selectedModGameVersion->numberOfFiles(); i++) {
			std::shared_ptr<ModFile> modFile(selectedModGameVersion->getFile(i));

			if(Utilities::areStringsEqualIgnoreCase(modFile->getFileExtension(), "grp") && gameVersion.doesRequireCombinedGroup()) {
				continue;
			}

			std::string modFileFileName(modFile->getFileName());
			std::string modFileSourceFilePath(Utilities::joinPaths(getModsDirectoryPath(), gameVersions.getGameVersionWithName(selectedModGameVersion->getGameVersion())->getModDirectoryName(), modFileFileName));
			std::string modFileDestinationFilePath(Utilities::joinPaths(fileDestinationDirectoryPath, modFileFileName));

			std::error_code errorCode;
			std::filesystem::copy_file(std::filesystem::path(modFileSourceFilePath), std::filesystem::path(modFileDestinationFilePath), errorCode);

			if(errorCode) {
				spdlog::error("Failed to copy '{}' mod file '{}' from '{}' to directory '{}': {}", selectedModGameVersion->getFullName(), modFileFileName, Utilities::getFilePath(modFileSourceFilePath), fileDestinationDirectoryPath, errorCode.message());
				return false;
			}

			if(temporaryCopiedFilePaths != nullptr) {
				temporaryCopiedFilePaths->push_back(modFileDestinationFilePath);
			}

			spdlog::debug("Copied '{}' mod file '{}' to directory '{}'.", selectedModGameVersion->getFullName(), modFileFileName, fileDestinationDirectoryPath);
		}
	}

	if(!mapsDirectoryPath.empty() && !customMap.empty()) {
		std::string customMapSourceFilePath(Utilities::joinPaths(mapsDirectoryPath, customMap));
		std::string customMapDestinationFilePath(Utilities::joinPaths(fileDestinationDirectoryPath, customMap));

		std::error_code errorCode;
		std::filesystem::copy_file(std::filesystem::path(customMapSourceFilePath), std::filesystem::path(customMapDestinationFilePath), errorCode);

		if(errorCode) {
			spdlog::error("Failed to copy map file '{}' to directory '{}': {}", customMap, fileDestinationDirectoryPath, errorCode.message());
			return false;
		}

		if(temporaryCopiedFilePaths != nullptr) {
			temporaryCopiedFilePaths->push_back(customMapDestinationFilePath);
		}

		spdlog::debug("Copied map file '{}' to directory '{}'.", customMap, fileDestinationDirectoryPath);
	}

	return true;
}

bool ModManager::removeSymlinksOrTemporaryFiles(const GameVersion & gameVersion, const std::vector<std::string> * temporaryCopiedFilePaths) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	SettingsManager * settings = SettingsManager::getInstance();

	if(!gameVersion.isConfigured() || !areSymlinkSettingsValid()) {
		return false;
	}

	if(areSymlinksSupported() && gameVersion.doesSupportSubdirectories()) {
		bool result = true;

		result &= removeSymlink(settings->gameSymlinkName, std::filesystem::current_path().string());

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

	if(temporaryCopiedFilePaths != nullptr) {
		for(const std::string & filePath : *temporaryCopiedFilePaths) {
			if(!std::filesystem::is_regular_file(filePath)) {
				spdlog::info("Temporary copied file '{}' is not a regular file, or has already been deleted.", filePath);
				continue;
			}

			std::error_code errorCode;
			std::filesystem::remove(std::filesystem::path(filePath), errorCode);

			if(errorCode) {
				spdlog::error("Failed to delete temporary copied file '{}': {}", filePath, errorCode.message());
				continue;
			}

			spdlog::info("Deleted temporary copied file '{}'.", filePath);
		}
	}

	return true;
}

size_t ModManager::deleteFilesWithSuffix(const std::string & suffix, const std::string & path) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(suffix.empty()) {
		return 0;
	}

	std::filesystem::path directoryPath(path.empty() ? std::filesystem::current_path() : std::filesystem::path(path));
	size_t numberOfFilesDeleted = 0;

	for(const std::filesystem::directory_entry & e : std::filesystem::directory_iterator(directoryPath)) {
		if(e.is_regular_file() && Utilities::areStringsEqualIgnoreCase(Utilities::getFileExtension(e.path().string()), suffix)) {
			spdlog::info("Deleting file: '{}'.", e.path().string());

			std::error_code errorCode;
			std::filesystem::remove(e.path(), errorCode);

			if(errorCode) {
				spdlog::error("Failed to delete file '{}': {}", e.path().string(), errorCode.message());
				continue;
			}

			numberOfFilesDeleted++;
		}
	}

	return numberOfFilesDeleted;
}

size_t ModManager::renameFilesWithSuffixTo(const std::string & fromSuffix, const std::string & toSuffix, const std::string & path) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(fromSuffix.empty() || toSuffix.empty()) {
		return 0;
	}

	std::filesystem::path directoryPath(path.empty() ? std::filesystem::current_path() : std::filesystem::path(path));
	std::string newFilePath;
	size_t numberOfFilesRenamed = 0;

	for(const std::filesystem::directory_entry & e : std::filesystem::directory_iterator(directoryPath)) {
		if(e.is_regular_file() && Utilities::areStringsEqualIgnoreCase(Utilities::getFileExtension(e.path().string()), fromSuffix)) {
			newFilePath = Utilities::replaceFileExtension(e.path().string(), toSuffix);

			spdlog::info("Renaming file: '{}' > '{}'.", e.path().string(), newFilePath);

			std::error_code error;
			std::filesystem::rename(e.path(), newFilePath, error);

			if(error) {
				spdlog::error("Failed to rename file '{}' to '{}': {}", e.path().string(), newFilePath, error.message());
				continue;
			}

			numberOfFilesRenamed++;
		}
	}

	return numberOfFilesRenamed;
}

size_t ModManager::createDOSBoxTemplateScriptFiles(bool overwrite) {
	SettingsManager* settings = SettingsManager::getInstance();

	return createDOSBoxTemplateScriptFiles(Utilities::joinPaths(settings->dataDirectoryPath, settings->dosboxDataDirectoryName), overwrite);
}

size_t ModManager::createDOSBoxTemplateScriptFiles(const std::string & directoryPath, bool overwrite) {
	size_t numberOfDOSBoxTemplateScriptFilesCreated = 0;

	for(GameType gameType : magic_enum::enum_values<GameType>()) {
		if(createDOSBoxTemplateScriptFile(gameType, directoryPath, overwrite)) {
			numberOfDOSBoxTemplateScriptFilesCreated++;
		}
	}

	return numberOfDOSBoxTemplateScriptFilesCreated;
}

bool ModManager::createDOSBoxTemplateScriptFile(GameType gameType, const std::string & directoryPath, bool overwrite) {
	if(!directoryPath.empty()) {
		std::filesystem::path outputDirectoryPath(directoryPath);

		if(!std::filesystem::is_directory(outputDirectoryPath)) {
			std::error_code errorCode;
			std::filesystem::create_directories(outputDirectoryPath, errorCode);

			if(errorCode) {
				spdlog::error("Cannot create '{}' DOSBox template script file, output directory '{}' creation failed: {}", magic_enum::enum_name(gameType), directoryPath, errorCode.message());
				return false;
			}
		}
	}

	std::string templateScriptFileName(getDOSBoxTemplateScriptFileName(gameType));
	std::string templateScriptFilePath(Utilities::joinPaths(directoryPath, templateScriptFileName));

	if(std::filesystem::is_regular_file(std::filesystem::path(templateScriptFilePath)) && !overwrite) {
		spdlog::debug("'{}' DOSBox template script already exists at '{}', specify overwrite to replace.", magic_enum::enum_name(gameType), templateScriptFilePath);
		return false;
	}

	std::string templateScriptFileData(generateDOSBoxTemplateScriptFileData(gameType));

	std::ofstream fileStream(templateScriptFilePath);

	if(!fileStream.is_open()) {
		return false;
	}

	fileStream.write(reinterpret_cast<const char *>(templateScriptFileData.data()), templateScriptFileData.size());

	fileStream.close();

	spdlog::info("Created '{}' DOSBox template file script '{}' in directory '{}'.", magic_enum::enum_name(gameType), templateScriptFileName, directoryPath);

	return true;
}

std::string ModManager::getDOSBoxTemplateScriptFileName(GameType gameType) {
	return Utilities::toLowerCase(magic_enum::enum_name(gameType)) + ".conf.in";
}

std::string ModManager::generateDOSBoxTemplateScriptFileData(GameType gameType) {
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

	if(m_preferredDOSBoxVersion != nullptr && !dosboxVersions->hasDOSBoxVersion(*m_preferredDOSBoxVersion.get())) {
		SettingsManager * settings = SettingsManager::getInstance();

		if(dosboxVersions->numberOfDOSBoxVersions() == 0) {
			m_preferredDOSBoxVersion = nullptr;
			settings->preferredDOSBoxVersion.clear();
		}
		else {
			m_preferredDOSBoxVersion = dosboxVersions->getDOSBoxVersion(0);
			settings->preferredDOSBoxVersion = m_preferredDOSBoxVersion->getName();
		}
	}
}

void ModManager::onDOSBoxVersionCollectionItemModified(DOSBoxVersionCollection & dosboxVersionCollection, DOSBoxVersion & dosboxVersion) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_preferredDOSBoxVersion.get() == &dosboxVersion) {
		SettingsManager::getInstance()->preferredDOSBoxVersion = dosboxVersion.getName();
	}
}

void ModManager::onGameVersionCollectionSizeChanged(GameVersionCollection & gameVersionCollection) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_preferredGameVersion != nullptr && !getGameVersions()->hasGameVersion(*m_preferredGameVersion.get())) {
		SettingsManager * settings = SettingsManager::getInstance();

		if(getGameVersions()->numberOfGameVersions() == 0) {
			m_preferredGameVersion = nullptr;
			settings->preferredGameVersion.clear();
		}
		else {
			m_preferredGameVersion = getGameVersions()->getGameVersion(0);
			settings->preferredGameVersion = m_preferredGameVersion->getName();
		}
	}
}

void ModManager::onGameVersionCollectionItemModified(GameVersionCollection & gameVersionCollection, GameVersion & gameVersion) {
	std::lock_guard<std::recursive_mutex> lock(m_mutex);

	if(m_preferredGameVersion.get() == &gameVersion) {
		SettingsManager::getInstance()->preferredGameVersion = gameVersion.getName();
	}
}
