#include "ModManager.h"

#include "Download/DownloadManager.h"
#include "Environment.h"
#include "Game/GameLocator.h"
#include "Game/GameVersion.h"
#include "Game/GameVersionCollection.h"
#include "Group/Group.h"
#include "Group/GroupUtilities.h"
#include "Manager/ModMatch.h"
#include "Mod/Mod.h"
#include "Mod/ModAuthorInformation.h"
#include "Mod/ModCollection.h"
#include "Mod/ModDownload.h"
#include "Mod/ModFile.h"
#include "Mod/ModGameVersion.h"
#include "Mod/ModImage.h"
#include "Mod/ModScreenshot.h"
#include "Mod/ModVersion.h"
#include "Mod/ModVersionType.h"
#include "Mod/FavouriteModCollection.h"
#include "Mod/OrganizedModCollection.h"
#include "SettingsManager.h"
#include "Project.h"

#include <Analytics/Segment/SegmentAnalytics.h>
#include <Arguments/ArgumentParser.h>
#include <Location/GeoLocationService.h>
#include <Network/HTTPRequest.h>
#include <Network/HTTPResponse.h>
#include <Network/HTTPService.h>
#include <Script/Script.h>
#include <Script/ScriptArguments.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/NumberUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/Utilities.h>
#include <Zip/ZipArchive.h>

#include <date/tz.h>
#include <fmt/core.h>
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <array>
#include <conio.h>
#include <errno.h>
#include <filesystem>
#include <iostream>
#include <optional>
#include <regex>
#include <sstream>

using namespace std::chrono_literals;

const GameType ModManager::DEFAULT_GAME_TYPE = GameType::Game;
const std::string ModManager::DEFAULT_PREFERRED_GAME_VERSION(GameVersion::ORIGINAL_ATOMIC_EDITION.getName());
const std::string ModManager::HTTP_USER_AGENT("DukeNukem3DModManager/" + APPLICATION_VERSION);

ModManager::ModManager()
	: Application()
	, m_initialized(false)
	, m_localMode(false)
	, m_demoRecordingEnabled(false)
	, m_settings(std::make_unique<SettingsManager>())
	, m_httpService(std::make_unique<HTTPService>())
	, m_gameType(ModManager::DEFAULT_GAME_TYPE)
	, m_selectedModVersionIndex(0)
	, m_selectedModVersionTypeIndex(0)
	, m_gameVersions(std::make_shared<GameVersionCollection>())
	, m_mods(std::make_shared<ModCollection>())
	, m_favouriteMods(std::make_shared<FavouriteModCollection>())
	, m_organizedMods(std::make_shared<OrganizedModCollection>(m_mods, m_favouriteMods, m_gameVersions))
	, m_cli(std::make_unique<ModManager::CLI>(this)) {
	assignPlatformFactories();
}

ModManager::~ModManager() {
	SegmentAnalytics::destroyInstance();
}

bool ModManager::isInitialized() const {
	return m_initialized;
}

bool ModManager::initialize(int argc, char * argv[], bool start) {
	if(m_initialized) {
		return true;
	}

	bool localModeSet = false;

	if(argc != 0) {
		m_arguments = std::make_shared<ArgumentParser>(argc, argv);

		if(m_arguments->hasArgument("?")) {
			displayArgumentHelp();
			return false;
		}

		if(m_arguments->hasArgument("version")) {
			fmt::print("{}\n", APPLICATION_VERSION);
			return false;
		}

		if(m_arguments->hasArgument("local")) {
			m_localMode = true;
			localModeSet = true;
		}

		if(m_arguments->hasArgument("r")) {
			m_demoRecordingEnabled = true;
		}
	}

	m_settings->load(m_arguments.get());

	if(m_settings->localMode && !localModeSet) {
		m_localMode = true;
	}

	date::set_install(Utilities::joinPaths(m_settings->dataDirectoryPath, m_settings->timeZoneDataDirectoryName));

	HTTPConfiguration configuration = {
		Utilities::joinPaths(m_settings->dataDirectoryPath, m_settings->curlDataDirectoryName, m_settings->certificateAuthorityStoreFileName),
		m_settings->apiBaseURL,
		m_settings->connectionTimeout,
		m_settings->networkTimeout
	};

	if(!m_httpService->initialize(configuration)) {
		spdlog::error("Failed to initialize HTTP service!");
		return false;
	}

	m_httpService->setUserAgent(HTTP_USER_AGENT);

	GeoLocationService * geoLocationService = GeoLocationService::getInstance();

	if(!geoLocationService->initialize(m_httpService, FREE_GEO_IP_API_KEY)) {
		spdlog::error("Failed to initialize geo location service!");
		return false;
	}

	SegmentAnalytics * segmentAnalytics = SegmentAnalytics::getInstance();

	if(m_settings->segmentAnalyticsEnabled) {
		SegmentAnalytics::Configuration configuration;
		configuration.writeKey = SEGMENT_ANALYTICS_WRITE_KEY;
		configuration.includeIPAddress = false;
		configuration.includeGeoLocation = true;
		configuration.dataStorageFilePath = Utilities::joinPaths(m_settings->cacheDirectoryPath, m_settings->segmentAnalyticsDataFileName);
		configuration.httpService = m_httpService;
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

	GameLocator * gameLocator = GameLocator::getInstance();

	if(gameLocator->locateGames()) {
		spdlog::info("Located {} Duke Nukem 3D game install{}.", gameLocator->numberOfGamePaths(), gameLocator->numberOfGamePaths() == 1 ? "" : "s");
	}

	if(!m_localMode) {
		m_downloadManager = std::make_unique<DownloadManager>(m_httpService, m_settings);

		if(!m_downloadManager->initialize()) {
			spdlog::error("Failed to initialize download manager!");
			return false;
		}
	}

	m_gameType = m_settings->gameType;

	bool saveGameVersions = false;
	bool gameVersionsLoaded = m_gameVersions->loadFrom(m_settings->gameVersionsListFilePath);

	if(!gameVersionsLoaded || m_gameVersions->numberOfGameVersions() == 0) {
		if(!gameVersionsLoaded) {
			spdlog::warn("Missing or invalid game versions configuration file '{}', using default values.", m_settings->gameVersionsListFilePath);
		}
		else if(m_gameVersions->numberOfGameVersions() == 0) {
			spdlog::warn("Empty game versions configuration file '{}', using default values.", m_settings->gameVersionsListFilePath);
		}

		// use default game version configurations
		m_gameVersions->setDefaultGameVersions();
	}

	m_gameVersions->addMissingDefaultGameVersions();

	m_preferredGameVersion = m_gameVersions->getGameVersion(m_settings->preferredGameVersion);

	if(m_preferredGameVersion == nullptr) {
		m_preferredGameVersion = m_gameVersions->getGameVersion(0);
		m_settings->preferredGameVersion = m_preferredGameVersion->getName();

		spdlog::warn("Game configuration for game version '{}' is missing, changing preferred game version to '{}.", m_settings->preferredGameVersion, m_preferredGameVersion->getName());
	}

	if(!m_mods->loadFrom(getModsListFilePath())) {
		spdlog::error("Failed to load mod list '{}'!", getModsListFilePath());
		return false;
	}

	if(m_mods->numberOfMods() == 0) {
		spdlog::error("No mods loaded!");
		return false;
	}

	if(!m_mods->checkGameVersions(*m_gameVersions)) {
		spdlog::error("Found at least one invalid or missing game version.");
		return false;
	}

	spdlog::info("Loaded {} mod{} from '{}'.", m_mods->numberOfMods(), m_mods->numberOfMods() == 1 ? "" : "s", getModsListFilePath());

	m_favouriteMods->loadFrom(m_settings->favouriteModsListFilePath);
	m_favouriteMods->checkForMissingFavouriteMods(*m_mods.get());

	if(m_favouriteMods->numberOfFavourites() != 0) {
		spdlog::info("Loaded {} favourite mod{} from '{}'.", m_favouriteMods->numberOfFavourites(), m_favouriteMods->numberOfFavourites() == 1 ? "" : "s", m_settings->favouriteModsListFilePath);
	}

	m_organizedMods->organize();

	m_initialized = true;

	checkForMissingExecutables();

	if(m_localMode) {
		checkAllModsForMissingFiles();

		checkForUnlinkedModFiles();
	}

	std::map<std::string, std::any> properties;
	properties["sessionNumber"] = segmentAnalytics->getSessionNumber();
	properties["environment"] = Utilities::toCapitalCase(APPLICATION_ENVIRONMENT);
	properties["gameType"] = Utilities::toCapitalCase(magic_enum::enum_name(m_settings->gameType));
	properties["preferredGameVersion"] = m_settings->preferredGameVersion;
	properties["numberOfDownloadedMods"] = m_downloadManager != nullptr ? m_downloadManager->numberOfDownloadedMods() : 0;
	properties["dosBoxArguments"] = m_settings->dosboxArguments;
	properties["dosBoxServerIPAddress"] = m_settings->dosboxServerIPAddress;
	properties["dosBoxServerLocalPort"] = m_settings->dosboxLocalServerPort;
	properties["dosBoxServerRemotePort"] = m_settings->dosboxRemoteServerPort;
	properties["apiBaseURL"] = m_settings->apiBaseURL;
	properties["connectionTimeout"] = m_settings->connectionTimeout.count();
	properties["networkTimeout"] = m_settings->networkTimeout.count();

	segmentAnalytics->track("Application Initialized", properties);

	if(!handleArguments(m_arguments.get(), start)) {
		return false;
	}

	return true;
}

bool ModManager::uninitialize() {
	if(!m_initialized) {
		return false;
	}

	SegmentAnalytics * segmentAnalytics = SegmentAnalytics::getInstance();
	segmentAnalytics->onApplicationClosed();
	segmentAnalytics->flush(3s);

	m_selectedMod.reset();
	m_organizedMods->setModCollection(nullptr);
	m_organizedMods->setFavouriteModCollection(nullptr);
	m_organizedMods->setGameVersionCollection(nullptr);
	m_favouriteMods->clearFavourites();
	m_mods->clearMods();

	m_settings->save(m_arguments.get());
	m_gameVersions->saveTo(m_settings->gameVersionsListFilePath);

	if(m_arguments != nullptr) {
		m_arguments.reset();
	}

	m_initialized = false;

	return true;
}

void ModManager::run() {
	if(!m_initialized) {
		return;
	}

	m_cli->runMenu();
}

bool ModManager::isUsingLocalMode() const {
	return m_localMode;
}

std::shared_ptr<SettingsManager> ModManager::getSettings() const {
	return m_settings;
}

std::shared_ptr<HTTPService> ModManager::getHTTPService() const {
	return m_httpService;
}

std::shared_ptr<OrganizedModCollection> ModManager::getOrganizedMods() const {
	return m_organizedMods;
}

std::string ModManager::getModsListFilePath() const {
	if(m_localMode) {
		if(m_settings == nullptr) {
			return Utilities::emptyString;
		}

		return m_settings->modsListFilePath;
	}

	if(m_downloadManager == nullptr) {
		return Utilities::emptyString;
	}

	return m_downloadManager->getCachedModListFilePath();
}

std::string ModManager::getModsDirectoryPath() const {
	if(m_settings == nullptr) {
		return Utilities::emptyString;
	}

	if(m_localMode) {
		return m_settings->modsDirectoryPath;
	}

	if(m_downloadManager == nullptr) {
		return Utilities::emptyString;
	}

	return m_downloadManager->getDownloadedModsDirectoryPath();
}

std::string ModManager::getMapsDirectoryPath() const {
	if(m_settings == nullptr) {
		return Utilities::emptyString;
	}

	if(m_localMode) {
		return m_settings->mapsDirectoryPath;
	}

	if(m_downloadManager == nullptr) {
		return Utilities::emptyString;
	}

	return m_downloadManager->getDownloadedMapsDirectoryPath();
}

GameType ModManager::getGameType() const {
	return m_gameType;
}

bool ModManager::setGameType(const std::string & gameTypeName) {
	std::optional<GameType> gameTypeOptional = magic_enum::enum_cast<GameType>(Utilities::toPascalCase(gameTypeName));

	if(gameTypeOptional.has_value()) {
		m_gameType = gameTypeOptional.value();

		return true;
	}

	return false;
}

void ModManager::setGameType(GameType gameType) {
	m_gameType = gameType;

	m_settings->gameType = m_gameType;
}

bool ModManager::hasPreferredGameVersion() const {
	return m_preferredGameVersion != nullptr;
}

std::shared_ptr<GameVersion> ModManager::getPreferredGameVersion() const {
	return m_preferredGameVersion;
}

bool ModManager::setPreferredGameVersion(const std::string & gameVersionName) {
	if(gameVersionName.empty()) {
		return false;
	}

	return setPreferredGameVersion(m_gameVersions->getGameVersion(gameVersionName));
}

bool ModManager::setPreferredGameVersion(std::shared_ptr<GameVersion> gameVersion) {
	if(gameVersion == nullptr || !gameVersion->isValid()) {
		return false;
	}

	m_preferredGameVersion = gameVersion;
	m_settings->preferredGameVersion = m_preferredGameVersion->getName();

	return true;
}

std::shared_ptr<GameVersionCollection> ModManager::getGameVersions() const {
	return m_gameVersions;
}

const std::string & ModManager::getDOSBoxServerIPAddress() const {
	return m_settings->dosboxServerIPAddress;
}

void ModManager::setDOSBoxServerIPAddress(const std::string & ipAddress) {
	m_settings->dosboxServerIPAddress = Utilities::trimString(ipAddress);
}

std::shared_ptr<Mod> ModManager::getSelectedMod() const {
	return m_selectedMod;
}

std::shared_ptr<ModVersion> ModManager::getSelectedModVersion() const {
	if(m_selectedMod == nullptr || m_selectedModVersionIndex >= m_selectedMod->numberOfVersions()) {
		return nullptr;
	}

	return m_selectedMod->getVersion(m_selectedModVersionIndex);
}

std::shared_ptr<ModVersionType> ModManager::getSelectedModVersionType() const {
	if(m_selectedMod == nullptr) {
		return nullptr;
	}

	std::shared_ptr<ModVersion> selectedModVersion(getSelectedModVersion());

	if(selectedModVersion == nullptr || m_selectedModVersionTypeIndex >= selectedModVersion->numberOfTypes()) {
		return nullptr;
	}

	return selectedModVersion->getType(m_selectedModVersionTypeIndex);
}

std::optional<std::string> ModManager::getSelectedModName() const {
	if(m_selectedMod == nullptr) {
		return {};
	}

	return m_selectedMod->getName();
}

size_t ModManager::getSelectedModVersionIndex() const {
	return m_selectedModVersionIndex;
}

size_t ModManager::getSelectedModVersionTypeIndex() const {
	return m_selectedModVersionTypeIndex;
}

bool ModManager::setSelectedMod(const std::string & name) {
	return setSelectedMod(m_mods->getMod(name));
}

bool ModManager::setSelectedMod(std::shared_ptr<Mod> mod) {
	if(mod != nullptr && !mod->isValid()) {
		return false;
	}

	if(m_selectedMod != mod) {
		m_selectedMod = mod;
		m_selectedModVersionIndex = std::numeric_limits<size_t>::max();
		m_selectedModVersionTypeIndex = std::numeric_limits<size_t>::max();
	}

	return true;
}

bool ModManager::setSelectedModVersionIndex(size_t modVersionIndex) {
	if(modVersionIndex == std::numeric_limits<size_t>::max()) {
		m_selectedModVersionIndex = std::numeric_limits<size_t>::max();
		m_selectedModVersionTypeIndex = std::numeric_limits<size_t>::max();

		return true;
	}

	if(!Mod::isValid(m_selectedMod.get())) {
		return false;
	}

	size_t newModVersionIndex = std::numeric_limits<size_t>::max();

	if(modVersionIndex < m_selectedMod->numberOfVersions()) {
		newModVersionIndex = modVersionIndex;
	}

	if(newModVersionIndex != m_selectedModVersionIndex) {
		m_selectedModVersionIndex = newModVersionIndex;
		m_selectedModVersionTypeIndex = std::numeric_limits<size_t>::max();
	}

	return true;
}

bool ModManager::setSelectedModVersionTypeIndex(size_t modVersionTypeIndex) {
	if(modVersionTypeIndex == std::numeric_limits<size_t>::max()) {
		m_selectedModVersionTypeIndex = std::numeric_limits<size_t>::max();

		return true;
	}

	if(!Mod::isValid(m_selectedMod.get()) || m_selectedModVersionIndex >= m_selectedMod->numberOfVersions()) {
		return false;
	}

	size_t newModVersionTypeIndex = std::numeric_limits<size_t>::max();
	std::shared_ptr<ModVersion> modVersion = m_selectedMod->getVersion(m_selectedModVersionIndex);

	if(modVersionTypeIndex < modVersion->numberOfTypes()) {
		newModVersionTypeIndex = modVersionTypeIndex;
	}

	m_selectedModVersionTypeIndex = newModVersionTypeIndex;

	return true;
}

bool ModManager::selectRandomMod(bool selectPreferredVersion, bool selectFirstVersionType) {
	if(!m_initialized || m_organizedMods->numberOfMods() == 0) {
		return false;
	}

	setSelectedMod(m_organizedMods->getMod(Utilities::randomInteger(0, m_organizedMods->numberOfMods() - 1)));

	if(selectPreferredVersion) {
		setSelectedModVersionIndex(m_selectedMod->indexOfPreferredVersion());

		if(selectFirstVersionType) {
			setSelectedModVersionTypeIndex(0);
		}
	}

	return true;
}

bool ModManager::selectRandomGameVersion() {
	if(!m_initialized || m_organizedMods->numberOfGameVersions() == 0) {
		return false;
	}

	return m_organizedMods->setSelectedGameVersion(Utilities::randomInteger(0, m_organizedMods->numberOfGameVersions() - 1));
}

bool ModManager::selectRandomTeam() {
	if(!m_initialized || m_organizedMods->numberOfTeams() == 0) {
		return false;
	}

	return m_organizedMods->setSelectedTeam(Utilities::randomInteger(0, m_organizedMods->numberOfTeams() - 1));
}

bool ModManager::selectRandomAuthor() {
	if(!m_initialized || m_organizedMods->numberOfAuthors() == 0) {
		return false;
	}

	return m_organizedMods->setSelectedAuthor(Utilities::randomInteger(0, m_organizedMods->numberOfAuthors() - 1));
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

		if(!Mod::isValid(mod.get())) {
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
	m_selectedMod = nullptr;
	m_selectedModVersionIndex = std::numeric_limits<size_t>::max();
	m_selectedModVersionTypeIndex = std::numeric_limits<size_t>::max();
}

bool ModManager::runSelectedMod() {
	if(!m_initialized) {
		return false;
	}

	SegmentAnalytics * segmentAnalytics = SegmentAnalytics::getInstance();

	std::shared_ptr<GameVersion> selectedGameVersion;

	if(m_arguments != nullptr && m_arguments->hasArgument("game") && !m_arguments->getFirstValue("game").empty()) {
		std::string gameVersionName(m_arguments->getFirstValue("game"));

		selectedGameVersion = m_gameVersions->getGameVersion(gameVersionName);

		if(selectedGameVersion == nullptr) {
			spdlog::error("Could not find game version override for '{}'.", gameVersionName);
			return false;
		}
	}
	else {
		if(m_preferredGameVersion == nullptr) {
			spdlog::error("No preferred game version selected.");
			return false;
		}

		selectedGameVersion = m_preferredGameVersion;
	}

	if(!selectedGameVersion->isValid()) {
		spdlog::error("Invalid preferred game version.");
		return false;
	}

	if(!selectedGameVersion->isConfigured()) {
		spdlog::error("Preferred game version is not configured.");
		return false;
	}

	std::shared_ptr<ModVersion> selectedModVersion;
	std::shared_ptr<ModVersionType> selectedModVersionType;
	std::shared_ptr<ModGameVersion> selectedModGameVersion;

	if(m_selectedMod != nullptr) {
		if(checkModForMissingFiles(*m_selectedMod) != 0) {
			spdlog::error("Mod is missing files, aborting execution.");
			return false;
		}

		if(m_selectedModVersionIndex == std::numeric_limits<size_t>::max()) {
			setSelectedModVersionIndex(0);

			if(m_selectedMod->numberOfVersions() > 1) {
				if(!m_cli->updateSelectedModVersionPrompt()) {
					return false;
				}
			}
		}

		selectedModVersion = m_selectedMod->getVersion(m_selectedModVersionIndex);

		if(m_selectedModVersionTypeIndex == std::numeric_limits<size_t>::max()) {
			setSelectedModVersionTypeIndex(0);

			if (selectedModVersion->numberOfTypes() > 1) {
				if (!m_cli->updateSelectedModVersionTypePrompt()) {
					return false;
				}
			}
		}

		selectedModVersionType = selectedModVersion->getType(m_selectedModVersionTypeIndex);

		std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(selectedGameVersion->getCompatibleModGameVersions(selectedModVersionType->getGameVersions()));

		if(compatibleModGameVersions.empty()) {
			std::string additionalPromptMessage(fmt::format("{} is not supported on {}.", m_selectedMod->getFullName(m_selectedModVersionIndex, m_selectedModVersionTypeIndex), selectedGameVersion->getName()));

			std::optional<std::pair<std::shared_ptr<GameVersion>, std::shared_ptr<ModGameVersion>>> optionalGameAndModGameVersionSelection(m_cli->runUnsupportedModVersionTypePrompt(selectedModVersionType, additionalPromptMessage));

			if(!optionalGameAndModGameVersionSelection.has_value()) {
				spdlog::error("Alternative game version not selected, aborting launch.");

				return false;
			}

			spdlog::info("Using game version '{}' since '{}' is not supported on '{}'.", optionalGameAndModGameVersionSelection.value().first->getName(), m_selectedMod->getFullName(m_selectedModVersionIndex, m_selectedModVersionTypeIndex), selectedGameVersion->getName());

			selectedGameVersion = optionalGameAndModGameVersionSelection.value().first;
			selectedModGameVersion = optionalGameAndModGameVersionSelection.value().second;
		}
		else {
			selectedModGameVersion = compatibleModGameVersions[0];
		}
	}

	bool shouldConfigureTemporaryDirectory = false;

	if(selectedGameVersion->doesRequireCombinedGroup()) {
		shouldConfigureTemporaryDirectory = true;
	}

	if(!m_localMode && selectedModGameVersion != nullptr) {
		if(!m_downloadManager->downloadModGameVersion(selectedModGameVersion.get(), m_gameVersions.get())) {
			spdlog::error("Aborting launch of '{}' mod!", selectedModGameVersion->getFullName());
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

	scriptArgs.addArgument("GROUPFLAG", selectedGameVersion->getGroupFileArgumentFlag());
	scriptArgs.addArgument("CONFLAG", selectedGameVersion->getConFileArgumentFlag());
	scriptArgs.addArgument("MAPFLAG", selectedGameVersion->getMapFileArgumentFlag());

	if(selectedGameVersion->hasDefFileArgumentFlag()) {
		scriptArgs.addArgument("DEFFLAG", selectedGameVersion->getDefFileArgumentFlag().value());
	}

	scriptArgs.addArgument("GAMEDIR", m_settings->gameSymlinkName);
	scriptArgs.addArgument("MAPSDIR", m_settings->mapsSymlinkName);
	scriptArgs.addArgument("MODSDIR", m_settings->modsSymlinkName);

	if(shouldConfigureTemporaryDirectory) {
		scriptArgs.addArgument("TEMPDIR", m_settings->tempSymlinkName);
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

		std::optional<std::string> defFileName(selectedModGameVersion->getFirstFileNameOfType("def"));

		if(defFileName.has_value()) {
			scriptArgs.addArgument("DEF", *defFileName);
		}
	}

	if(m_gameType == GameType::Client) {
		scriptArgs.addArgument("IP", m_settings->dosboxServerIPAddress);
		scriptArgs.addArgument("PORT", std::to_string(m_settings->dosboxRemoteServerPort));
	}
	else if(m_gameType == GameType::Server) {
		scriptArgs.addArgument("PORT", std::to_string(m_settings->dosboxLocalServerPort));
	}

	if(!selectedGameVersion->doesRequireDOSBox() && (m_gameType == GameType::Client || m_gameType == GameType::Server)) {
		spdlog::info("Network settings are only supported when running in DOSBox, ignoring {} game type setting.", Utilities::toCapitalCase(magic_enum::enum_name(m_gameType)));
	}

	bool customMod = false;
	std::string customMap;

	std::string command(generateCommand(selectedModGameVersion, selectedGameVersion, scriptArgs, combinedGroupFileName, &customMod, &customMap));

	if(command.empty()) {
		spdlog::error("Failed to generate command.");
		return false;
	}

	if(!createSymlinks(*selectedGameVersion, shouldConfigureTemporaryDirectory)) {
		return false;
	}

	if(shouldConfigureTemporaryDirectory && !createTemporaryDirectory()) {
		return false;
	}

	std::string combinedGroupFilePath;
	std::unique_ptr<Group> combinedGroup;

	if(m_selectedMod != nullptr && (selectedGameVersion->doesRequireCombinedGroup() || !m_demoRecordingEnabled)) {
		if(!m_demoRecordingEnabled) {
			ModManager::renameFilesWithSuffixTo("DMO", "DMO_", selectedGameVersion->getGamePath());
		}

		if(selectedGameVersion->doesRequireCombinedGroup()) {
			std::string dukeNukemGroupPath(Utilities::joinPaths(selectedGameVersion->getGamePath(), "DUKE3D.GRP"));
			combinedGroup = std::make_unique<Group>(dukeNukemGroupPath);

			if(!combinedGroup->load()) {
				spdlog::error("Failed to load Duke Nukem 3D group for creation of combined group from file path: '{}'.", dukeNukemGroupPath);
				return false;
			}

			combinedGroupFilePath = Utilities::joinPaths(m_settings->tempDirectoryPath, combinedGroupFileName);
			combinedGroup->setFilePath(combinedGroupFilePath);
		}

		std::vector<std::shared_ptr<ModFile>> modFiles(selectedModGameVersion->getFilesOfType("grp"));

		for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = modFiles.begin(); i != modFiles.end(); ++i) {
			std::string modGroupPath(Utilities::joinPaths(getModsDirectoryPath(), m_gameVersions->getGameVersion(selectedModGameVersion->getGameVersion())->getModDirectoryName(), (*i)->getFileName()));
			std::unique_ptr<Group> modGroup(std::make_unique<Group>(modGroupPath));

			if(modGroup->load()) {
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
				spdlog::error("Failed to load mod group from file path: '{}'.", modGroupPath);
				return false;
			}
		}
	}

	if(combinedGroup != nullptr) {
		if(combinedGroup->save(true)) {
			spdlog::info("Saved combined group to file: '{}'.", combinedGroupFilePath);
		}
		else {
			spdlog::error("Failed to write combined group to file: '{}'.", combinedGroupFilePath);
			return false;
		}

		combinedGroup.reset();
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

	if(customMod) {
		spdlog::info("Running custom mod in {} mode{}.", Utilities::toCapitalCase(magic_enum::enum_name(m_gameType)), customMapMessage);

		segmentAnalytics->track("Running Custom Mod", properties);
	}
	else if(m_selectedMod != nullptr) {
		std::string fullModName(m_selectedMod->getFullName(m_selectedModVersionIndex, m_selectedModVersionTypeIndex));
		std::string gameTypeName(Utilities::toCapitalCase(magic_enum::enum_name(m_gameType)));
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

	spdlog::info("Executing Command: {}", command);

	system(command.c_str());

	if(!combinedGroupFilePath.empty()) {
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

	if(m_selectedMod != nullptr && !m_demoRecordingEnabled) {
		ModManager::deleteFilesWithSuffix("DMO", selectedGameVersion->getGamePath());
		ModManager::renameFilesWithSuffixTo("DMO_", "DMO", selectedGameVersion->getGamePath());
	}

	removeSymlinks(*selectedGameVersion);

	return true;
}

std::string ModManager::generateCommand(std::shared_ptr<ModGameVersion> modGameVersion, std::shared_ptr<GameVersion> selectedGameVersion, ScriptArguments & scriptArgs, std::string_view combinedGroupFileName, bool * customMod, std::string * customMap) const {
	static const std::regex respawnModeRegExp("[123x]");

	if(!m_initialized) {
		spdlog::error("Mod manager not initialized.");
		return {};
	}

	if(!selectedGameVersion->isConfigured()) {
		spdlog::error("Invalid or unconfigured game version.");
		return {};
	}

	if(m_settings->dataDirectoryPath.empty()) {
		spdlog::error("Empty data path.");
		return {};
	}

	if(selectedGameVersion->doesRequireDOSBox() && m_settings->dosboxDirectoryPath.empty()) {
		spdlog::error("Empty DOSBox path.");
		return {};
	}

	if(m_settings->gameSymlinkName.empty()) {
		spdlog::error("Empty game directory symbolic link name.");
		return {};
	}

	if(m_settings->modsSymlinkName.empty()) {
		spdlog::error("Empty mods directory symbolic link name.");
		return {};
	}

	if(!GameVersionCollection::isValid(m_gameVersions.get())) {
		spdlog::error("Invalid game version collection.");
		return {};
	}

	std::shared_ptr<GameVersion> targetGameVersion;

	if(modGameVersion != nullptr) {
		if(!modGameVersion->isValid()) {
			spdlog::error("Invalid mod game version.");
			return {};
		}

		if(!Utilities::areStringsEqualIgnoreCase(modGameVersion->getGameVersion(), selectedGameVersion->getName()) && !selectedGameVersion->hasCompatibleGameVersion(modGameVersion->getGameVersion())) {
			spdlog::error("Game version '{}' is not compatible with '{}'.", selectedGameVersion->getName(), modGameVersion->getGameVersion());
			return {};
		}

		targetGameVersion = m_gameVersions->getGameVersion(modGameVersion->getGameVersion());

		if(targetGameVersion == nullptr) {
			spdlog::error("Missing game configuration for '{}'.", modGameVersion->getGameVersion());
			return {};
		}
	}
	else {
		targetGameVersion = selectedGameVersion;
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
	std::string customConFile;
	std::string customDefFile;

	if(m_arguments != nullptr) {
		if(m_arguments->hasArgument("g") ||
		   m_arguments->hasArgument("x") ||
		   m_arguments->hasArgument("h")) {
			customGroupFiles = m_arguments->getValues("g");

			if(!customGroupFiles.empty()) {
				customConFile = m_arguments->getFirstValue("x");
				customDefFile = m_arguments->getFirstValue("h");

				for(std::vector<std::string>::const_iterator i = customGroupFiles.begin(); i != customGroupFiles.end(); ++i) {
					scriptArgs.addArgument("GROUP", *i);
				}

				scriptArgs.addArgument("CON", customConFile);
				scriptArgs.addArgument("DEF", customDefFile);

				if(customMod != nullptr) {
					*customMod = true;
				}
			}
		}
	}

	if(modGameVersion != nullptr || !customGroupFiles.empty()) {
		std::string modPath(Utilities::joinPaths(m_settings->modsSymlinkName, targetGameVersion->getModDirectoryName()));

		if(!selectedGameVersion->hasLocalWorkingDirectory()) {
			modPath = Utilities::joinPaths(m_settings->gameSymlinkName, modPath);
		}

		std::string conFileName;

		if(!customGroupFiles.empty()) {
			conFileName = customConFile;
		}
		else {
			std::shared_ptr<ModFile> conFile(modGameVersion->getFirstFileOfType("con"));

			if(conFile != nullptr) {
				conFileName = conFile->getFileName();
			}
		}

		if(!customGroupFiles.empty()) {
			for(std::vector<std::string>::const_iterator i = customGroupFiles.begin(); i != customGroupFiles.end(); ++i) {
				command << " " << selectedGameVersion->getGroupFileArgumentFlag() << Utilities::joinPaths(modPath, *i);
			}
		}
		else if(!combinedGroupFileName.empty()) {
			command << " " << selectedGameVersion->getGroupFileArgumentFlag() << Utilities::joinPaths(m_settings->tempSymlinkName, combinedGroupFileName);
		}
		else {
			std::vector<std::shared_ptr<ModFile>> groupFiles(modGameVersion->getFilesOfType("grp"));

			for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = groupFiles.begin(); i != groupFiles.end(); ++i) {
				command << " " << selectedGameVersion->getGroupFileArgumentFlag() << Utilities::joinPaths(modPath, (*i)->getFileName());
			}
		}

		if(!customGroupFiles.empty() || modGameVersion->isEDuke32()) {
			if(customGroupFiles.empty()) {
				std::vector<std::shared_ptr<ModFile>> zipFiles(modGameVersion->getFilesOfType("zip"));

				for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = zipFiles.begin(); i != zipFiles.end(); ++i) {
					command << " " << selectedGameVersion->getGroupFileArgumentFlag() << Utilities::joinPaths(modPath, (*i)->getFileName());
				}
			}

			std::string defFileName;

			if(!customGroupFiles.empty()) {
				defFileName = customDefFile;
			}
			else {
				std::shared_ptr<ModFile> defFile(modGameVersion->getFirstFileOfType("def"));

				if(defFile != nullptr) {
					defFileName = defFile->getFileName();
				}
			}

			if(!defFileName.empty()) {
				if(!selectedGameVersion->hasDefFileArgumentFlag()) {
					spdlog::error("Game version '{}' does not have a def file argument flag specified in its configuration.", selectedGameVersion->getName());
					return {};
				}

				command << " " << selectedGameVersion->getDefFileArgumentFlag().value() << defFileName;
			}
		}

		if(!conFileName.empty()) {
			command << " " << selectedGameVersion->getConFileArgumentFlag() << (selectedGameVersion->hasRelativeConFilePath() ? conFileName : Utilities::joinPaths(modPath, conFileName));
		}
	}

	if(m_arguments != nullptr) {
		if(m_arguments->hasArgument("map")) {
			if(m_settings->mapsSymlinkName.empty()) {
				spdlog::error("Maps directory symbolic link name is empty.");
				return {};
			}

			std::string userMap(m_arguments->getFirstValue("map"));

			if(!userMap.empty()) {
				scriptArgs.addArgument("MAP", userMap);

				if(customMap != nullptr) {
					*customMap = userMap;
				}

				command << " " << selectedGameVersion->getMapFileArgumentFlag();

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
						command << Utilities::joinPaths(m_settings->mapsSymlinkName, userMap);
					}
					else {
						spdlog::error("Map '{}' does not exist in game or maps directories.", userMap);

						command << userMap;
					}
				}
			}
		}

		if(m_arguments->hasArgument("v")) {
			std::string episodeNumberData(m_arguments->getFirstValue("v"));
			std::optional<uint8_t> optionalEpisodeNumber(Utilities::parseUnsignedByte(episodeNumberData));

			if(optionalEpisodeNumber.has_value() && optionalEpisodeNumber.value() >= 1) {
				command << " " << selectedGameVersion->getEpisodeArgumentFlag() << std::to_string(optionalEpisodeNumber.value());
			}
			else {
				spdlog::warn("Invalid episode number: '{}'.", episodeNumberData);
			}
		}

		if(m_arguments->hasArgument("l")) {
			std::string levelNumberData(m_arguments->getFirstValue("l"));
			std::optional<uint8_t> optionalLevelNumber(Utilities::parseUnsignedByte(levelNumberData));

			if(optionalLevelNumber.has_value() && optionalLevelNumber.value() >= 1 && optionalLevelNumber.value() <= 11) {
				command << " " << selectedGameVersion->getLevelArgumentFlag() << std::to_string(optionalLevelNumber.value());
			}
			else {
				spdlog::warn("Invalid level number: '{}'.", levelNumberData);
			}
		}

		if(m_arguments->hasArgument("s")) {
			std::string skillNumberData(m_arguments->getFirstValue("s"));
			std::optional<uint8_t> optionalSkillNumber(Utilities::parseUnsignedByte(skillNumberData));

			if(optionalSkillNumber.has_value() && optionalSkillNumber.value() >= 1 && optionalSkillNumber.value() <= 4) {
				command << " " << selectedGameVersion->getSkillArgumentFlag() << std::to_string(optionalSkillNumber.value());
			}
			else {
				spdlog::warn("Invalid skill number: '{}'.", skillNumberData);
			}
		}

		if(m_demoRecordingEnabled) {
			command << " " << selectedGameVersion->getRecordDemoArgumentFlag();
		}

		if(m_arguments->hasArgument("d")) {
			std::string demoFileName(m_arguments->getFirstValue("d"));

			if(!demoFileName.empty() && selectedGameVersion->hasPlayDemoArgumentFlag()) {
				command << " " << selectedGameVersion->getPlayDemoArgumentFlag().value() << demoFileName;
			}
		}

		if(m_arguments->hasArgument("t")) {
			std::string respawnMode(m_arguments->getFirstValue("t"));

			if(!respawnMode.empty() && std::regex_match(respawnMode, respawnModeRegExp)) {
				command << " " << selectedGameVersion->getRespawnModeArgumentFlag() << respawnMode;
			}
		}

		if(m_arguments->hasArgument("u")) {
			std::string weaponSwitchOrder(m_arguments->getFirstValue("u"));

			if(!weaponSwitchOrder.empty() && weaponSwitchOrder.find_first_not_of("0123456789") == std::string::npos) {
				command << " " << selectedGameVersion->getWeaponSwitchOrderArgumentFlag() << weaponSwitchOrder;
			}
		}

		if(m_arguments->hasArgument("m")) {
			command << " " << selectedGameVersion->getDisableMonstersArgumentFlag();
		}

		if(m_arguments->hasArgument("ns")) {
			command << " " << selectedGameVersion->getDisableSoundArgumentFlag();
		}

		if(m_arguments->hasArgument("nm")) {
			command << " " << selectedGameVersion->getDisableMusicArgumentFlag();
		}
	}

	if(selectedGameVersion->doesRequireDOSBox()) {
		Script dosboxScript;
		std::string dosboxScriptFilePath;

		scriptArgs.addArgument("COMMAND", executableName + command.str());

		std::string dosboxDataDirectoryPath(Utilities::joinPaths(m_settings->dataDirectoryPath, m_settings->dosboxDataDirectoryName));

		switch(m_gameType) {
			case GameType::Game: {
				dosboxScriptFilePath = Utilities::joinPaths(dosboxDataDirectoryPath, m_settings->dosboxGameScriptFileName);
				break;
			}
			case GameType::Setup: {
				dosboxScriptFilePath = Utilities::joinPaths(dosboxDataDirectoryPath, m_settings->dosboxSetupScriptFileName);
				break;
			}
			case GameType::Client: {
				dosboxScriptFilePath = Utilities::joinPaths(dosboxDataDirectoryPath, m_settings->dosboxClientScriptFileName);
				break;
			}
			case GameType::Server: {
				dosboxScriptFilePath = Utilities::joinPaths(dosboxDataDirectoryPath, m_settings->dosboxServerScriptFileName);
				break;
			}
		}

		if(!dosboxScript.readFrom(dosboxScriptFilePath)) {
			spdlog::error("Failed to load DOSBox script file: '{}'.", dosboxScriptFilePath);
			return {};
		}

		return generateDOSBoxCommand(dosboxScript, scriptArgs, Utilities::joinPaths(m_settings->dosboxDirectoryPath, m_settings->dosboxExecutableFileName), m_settings->dosboxArguments);
	}

	return Utilities::joinPaths("\"" + selectedGameVersion->getGamePath(), executableName) + "\"" + command.str();
}

std::string ModManager::generateDOSBoxCommand(const Script & script, const ScriptArguments & arguments, const std::string & dosboxPath, const std::string & dosboxArguments) const {
	static const std::regex unescapedQuotesRegExp("(?:^\"|([^\\\\])\")");

	if(dosboxPath.empty()) {
		return std::string();
	}

	std::stringstream command;

	command << fmt::format("CALL \"{}\" {} ", dosboxPath, dosboxArguments);

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

bool ModManager::handleArguments(const ArgumentParser * args, bool start) {
	if(args != nullptr) {
		if(args->hasArgument("hash-new")) {
			updateAllFileHashes(true, true);
			return true;
		}

		if(args->hasArgument("hash-all")) {
			updateAllFileHashes(true, false);
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
							spdlog::warn("Invalid IP Address entered in arguments: '{}'.", ipAddress);

							m_cli->updateIPAddressPrompt();
						}
						else {
							m_settings->dosboxServerIPAddress = ipAddress;
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
							spdlog::warn("Invalid {} Server Port entered in arguments: '{}'.", m_gameType == GameType::Server ? "Local" : "Remote", portData);

							m_cli->updatePortPrompt();
						}
						else {
							if(m_gameType == GameType::Server) {
								m_settings->dosboxLocalServerPort = port;
							}
							else {
								m_settings->dosboxRemoteServerPort = port;
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
			if(args->hasArgument("random") || args->hasArgument("g") || args->hasArgument("x") || args->hasArgument("n")) {
				spdlog::error("Redundant arguments specified, please specify either search OR random OR n OR (x AND/OR g).");
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

				runSelectedMod();

				return true;
			}
			else {
				if(modMatches.size() > 20) {
					fmt::print("Found {} matches, please refine your search query.\n", modMatches.size());
				}
				else {
					fmt::print("Found {} matches:\n", modMatches.size());

					for(size_t i = 0; i < modMatches.size(); i++) {
						printSpacing(Utilities::unsignedLongLength(modMatches.size()) - Utilities::unsignedLongLength(i + 1));

						fmt::print("{}. {}\n", i + 1, modMatches[i].toString());
					}

					fmt::print("\n");
					fmt::print("Please refine your search query.\n");
				}

				return false;
			}
		}
		else if(args->hasArgument("random")) {
			if(args->hasArgument("g") || args->hasArgument("x") || args->hasArgument("n")) {
				spdlog::error("Redundant arguments specified, please specify either search OR random OR n OR (x AND/OR g).");
				return false;
			}

			selectRandomMod(true, true);

			fmt::print("Selected random mod: '{}'\n", m_selectedMod->getFullName(m_selectedModVersionIndex, m_selectedModVersionTypeIndex));
			fmt::print("\n");

			runSelectedMod();

			return true;
		}
		else if(args->hasArgument("n")) {
			if(args->hasArgument("g") || args->hasArgument("x")) {
				spdlog::error("Redundant arguments specified, please specify either search OR random OR n OR (x AND/OR g).");
				return false;
			}

			clearSelectedMod();
			runSelectedMod();

			return true;
		}
		else if(args->hasArgument("g") || args->hasArgument("x")) {
			runSelectedMod();

			return true;
		}
	}

	if(start) {
		run();
	}

	return true;
}

size_t ModManager::checkForUnlinkedModFiles() const {
	if(!m_initialized) {
		return 0;
	}

	size_t numberOfUnlinkedModFiles = 0;

	for(size_t i = 0; i < m_gameVersions->numberOfGameVersions(); i++) {
		numberOfUnlinkedModFiles += checkForUnlinkedModFilesForGameVersion(*m_gameVersions->getGameVersion(i));
	}

	return numberOfUnlinkedModFiles;
}

size_t ModManager::checkForUnlinkedModFilesForGameVersion(const GameVersion & gameVersion) const {
	if(!m_initialized || !gameVersion.isValid()) {
		return 0;
	}

	std::filesystem::path gameModsPath(std::filesystem::path(Utilities::joinPaths(m_settings->modsDirectoryPath, gameVersion.getModDirectoryName())));

	if(!std::filesystem::is_directory(gameModsPath)) {
		return 0;
	}

	std::map<std::string, std::vector<std::shared_ptr<ModFile>>> linkedModFiles;

	for(const std::filesystem::directory_entry& e : std::filesystem::directory_iterator(gameModsPath)) {
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
	if(!m_initialized ||
	   mod.numberOfVersions() == 0 ||
	   (versionIndex >= mod.numberOfVersions() && versionIndex != std::numeric_limits<size_t>::max())) {
		return 0;
	}

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
				gameVersion = m_gameVersions->getGameVersion(modGameVersion->getGameVersion());

				if(!GameVersion::isValid(gameVersion.get())) {
					spdlog::warn("Skipping checking invalid '{}' mod game version '{}', invalid game configuration.", mod.getFullName(i, j), modGameVersion->getGameVersion());
					continue;
				}

				gameModsPath = Utilities::joinPaths(m_settings->modsDirectoryPath, gameVersion->getModDirectoryName());

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
	size_t numberOfMissingExecutables = 0;

	std::string fullDOSBoxExecutablePath(Utilities::joinPaths(m_settings->dosboxDirectoryPath, m_settings->dosboxExecutableFileName));

	if(!std::filesystem::is_regular_file(std::filesystem::path(fullDOSBoxExecutablePath))) {
		numberOfMissingExecutables++;

		spdlog::warn("Missing DOSBox executable: '{}'.", fullDOSBoxExecutablePath);
	}

	numberOfMissingExecutables += m_gameVersions->checkForMissingExecutables();

	return numberOfMissingExecutables++;
}

size_t ModManager::updateAllFileHashes(bool save, bool skipHashedFiles) {
	if(!m_initialized) {
		return 0;
	}

	if(m_settings->modPackageDownloadsDirectoryPath.empty()) {
		spdlog::warn("Mod package downloads directory path not set, cannot hash some download files!");
	}

	if(m_settings->modSourceFilesDirectoryPath.empty()) {
		spdlog::warn("Mod source files directory path not set, cannot hash some download files!");
	}

	if(m_settings->modImagesDirectoryPath.empty()) {
		spdlog::warn("Mod images directory path not set, cannot hash screenshots or images!");
	}

	spdlog::info("Updating {} file hashes...", skipHashedFiles ? "unhashed" : "all");

	size_t numberOfFileHashesUpdated = 0;

	for(size_t i = 0; i < m_mods->numberOfMods(); i++) {
		numberOfFileHashesUpdated += updateModHashes(*m_mods->getMod(i), skipHashedFiles);
	}

	if(numberOfFileHashesUpdated != 0) {
		spdlog::info("Updated {} file hash{}.", numberOfFileHashesUpdated, numberOfFileHashesUpdated == 1 ? "" : "es");
	}
	else {
		spdlog::info("No file hashes updated.");
	}

	if(save) {
		if(m_mods->saveTo(m_settings->modsListFilePath)) {
			spdlog::info("Saved updated mod list to file: '{}'.", m_settings->modsListFilePath);
		}
		else {
			spdlog::error("Failed to save updated mod list to file: '{}'!", m_settings->modsListFilePath);
		}
	}

	return numberOfFileHashesUpdated;
}

size_t ModManager::updateModHashes(Mod & mod, bool skipHashedFiles, std::optional<size_t> versionIndex, std::optional<size_t> versionTypeIndex) {
	if(!m_initialized ||
	   mod.numberOfVersions() == 0 ||
	   (versionIndex >= mod.numberOfVersions() && versionIndex != std::numeric_limits<size_t>::max())) {
		return 0;
	}

	size_t numberOfFileHashesUpdated = 0;
	std::shared_ptr<ModDownload> modDownload;
	std::shared_ptr<ModScreenshot> modScreenshot;
	std::shared_ptr<ModImage> modImage;
	std::shared_ptr<ModVersion> modVersion;
	std::shared_ptr<ModVersionType> modVersionType;
	std::shared_ptr<ModGameVersion> modGameVersion;
	std::shared_ptr<ModFile> modFile;
	std::shared_ptr<GameVersion> gameVersion;
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
	std::optional<std::string> optionalSHA1;

	spdlog::info("Updating '{}' mod hashes...", mod.getName());

	// hash mod downloads
	for(size_t i = 0; i < mod.numberOfDownloads(); i++) {
		modDownload = mod.getDownload(i);

		if(!ModDownload::isValid(modDownload.get())) {
			spdlog::warn("Skipping hash of invalid download file #{} for mod '{}'.", i + 1, mod.getName());
			continue;
		}

		if(skipHashedFiles && !modDownload->getSHA1().empty()) {
			continue;
		}

		if(modDownload->isModManagerFiles()) {
			if(m_settings->modPackageDownloadsDirectoryPath.empty()) {
				continue;
			}

			gameVersion = m_gameVersions->getGameVersion(modDownload->getGameVersion());

			if(gameVersion == nullptr) {
				spdlog::warn("Could not find game configuration for game version '{}', skipping hash of download file: '{}'.", modDownload->getGameVersion(), modDownload->getFileName());
				continue;
			}

			downloadFilePath = Utilities::joinPaths(m_settings->modPackageDownloadsDirectoryPath, Utilities::toLowerCase(gameVersion->getModDirectoryName()), modDownload->getFileName());
		}
		else {
			if(m_settings->modSourceFilesDirectoryPath.empty()) {
				continue;
			}

			downloadFilePath = Utilities::joinPaths(m_settings->modSourceFilesDirectoryPath, Utilities::getSafeDirectoryName(mod.getName()));

			if(!modDownload->getVersion().empty()) {
				downloadFilePath = Utilities::joinPaths(downloadFilePath, Utilities::getSafeDirectoryName(modDownload->getVersion()));
			}

			downloadFilePath = Utilities::joinPaths(downloadFilePath, modDownload->getFileName());
		}

		if(!std::filesystem::is_regular_file(std::filesystem::path(downloadFilePath))) {
			spdlog::warn("Skipping hash of missing '{}' mod download file: '{}'.", mod.getName(), downloadFilePath);
			continue;
		}

		optionalSHA1 = Utilities::getFileSHA1Hash(downloadFilePath);

		if(!optionalSHA1.has_value()) {
			spdlog::error("Failed to hash mod '{}' download file '{}'.", mod.getName(), modFile->getFileName());
			continue;
		}

		if(modDownload->getSHA1() != optionalSHA1.value()) {
			spdlog::info("Updating mod '{}' download file '{}' SHA1 hash from '{}' to '{}'.", mod.getName(), modDownload->getFileName(), modDownload->getSHA1(), optionalSHA1.value());

			modDownload->setSHA1(optionalSHA1.value());

			numberOfFileHashesUpdated++;
		}
	}

	// hash mod screenshots
	if(!m_settings->modImagesDirectoryPath.empty()) {
		for(size_t i = 0; i < mod.numberOfScreenshots(); i++) {
			modScreenshot = mod.getScreenshot(i);

			if(!ModScreenshot::isValid(modScreenshot.get())) {
				spdlog::warn("Skipping hash of invalid screenshot file #{} for mod '{}'.", i + 1, mod.getName());
				continue;
			}

			if(skipHashedFiles && !modScreenshot->getSHA1().empty()) {
				continue;
			}

			screenshotFilePath = Utilities::joinPaths(m_settings->modImagesDirectoryPath, mod.getID(), "screenshots", "lg", modScreenshot->getFileName());

			if(!std::filesystem::is_regular_file(std::filesystem::path(screenshotFilePath))) {
				spdlog::warn("Skipping hash of missing '{}' mod screenshot file: '{}'.", mod.getName(), screenshotFilePath);
				continue;
			}

			optionalSHA1 = Utilities::getFileSHA1Hash(screenshotFilePath);

			if(!optionalSHA1.has_value()) {
				spdlog::error("Failed to hash mod '{}' screenshot file '{}'.", mod.getName(), modFile->getFileName());
				continue;
			}

			if(modScreenshot->getSHA1() != optionalSHA1.value()) {
				spdlog::info("Updating mod '{}' screenshot file '{}' SHA1 hash from '{}' to '{}'.", mod.getName(), modScreenshot->getFileName(), modScreenshot->getSHA1(), optionalSHA1.value());

				modScreenshot->setSHA1(optionalSHA1.value());

				numberOfFileHashesUpdated++;
			}
		}
	}

	// hash mod images
	if(!m_settings->modImagesDirectoryPath.empty()) {
		for(size_t i = 0; i < mod.numberOfImages(); i++) {
			modImage = mod.getImage(i);

			if(!ModImage::isValid(modImage.get())) {
				spdlog::warn("Skipping hash of invalid image file #{} for mod '{}'.", i + 1, mod.getName());
				continue;
			}

			if(skipHashedFiles && !modImage->getSHA1().empty()) {
				continue;
			}

			imageFilePath = Utilities::joinPaths(m_settings->modImagesDirectoryPath, mod.getID());

			if(!modImage->getSubfolder().empty()) {
				imageFilePath = Utilities::joinPaths(imageFilePath, modImage->getSubfolder());
			}

			imageFilePath = Utilities::joinPaths(imageFilePath, modImage->getFileName());

			if(!std::filesystem::is_regular_file(std::filesystem::path(imageFilePath))) {
				spdlog::warn("Skipping hash of missing '{}' mod image file: '{}'.", mod.getName(), imageFilePath);
				continue;
			}

			optionalSHA1 = Utilities::getFileSHA1Hash(imageFilePath);

			if(!optionalSHA1.has_value()) {
				spdlog::error("Failed to hash mod '{}' image file '{}'.", mod.getName(), modFile->getFileName());
				continue;
			}

			if(modImage->getSHA1() != optionalSHA1.value()) {
				spdlog::info("Updating mod '{}' image file '{}' SHA1 hash from '{}' to '{}'.", mod.getName(), modImage->getFileName(), modImage->getSHA1(), optionalSHA1.value());

				modImage->setSHA1(optionalSHA1.value());

				numberOfFileHashesUpdated++;
			}
		}
	}

	// hash mod files
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
				gameVersion = m_gameVersions->getGameVersion(modGameVersion->getGameVersion());

				if(!GameVersion::isValid(gameVersion.get())) {
					spdlog::warn("Mod '{}' game version #{} is not valid, skipping hashing of mod files.", mod.getFullName(i, j), k + 1);
					continue;
				}

				gameModsPath = Utilities::joinPaths(m_settings->modsDirectoryPath, gameVersion->getModDirectoryName());

				if(!std::filesystem::is_directory(gameModsPath)) {
					spdlog::warn("Mod '{}' '{}' game version directory '{}' does not exist or is not a valid directory, skipping hashing of mod files.", mod.getFullName(i, j), gameVersion->getName(), gameModsPath);
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

							group = std::make_unique<Group>(groupFilePath);

							if(!group->load()) {
								group.reset();

								spdlog::error("Failed to open mod group file '{}'.", groupFilePath);
							}
						}
					}
				}

				for(size_t l = 0; l < modGameVersion->numberOfFiles(); l++) {
					modFile = modGameVersion->getFile(l);

					if(skipHashedFiles && !modFile->getSHA1().empty()) {
						continue;
					}

					optionalSHA1.reset();

					// eDuke32 mod files can be read straight out of the group or zip file, and are not stored separately
					if(modGameVersion->isEDuke32() && modFile->getType() != "zip" && modFile->getType() != "grp") {
						if(!zipArchiveFilePath.empty()) {
							if(zipArchive == nullptr) {
								spdlog::error("Skipping hash of mod file '{}' since zip archive could not be opened.", zipArchiveFilePath);
								continue;
							}

							std::weak_ptr<ZipArchive::Entry> zipArchiveEntry(zipArchive->getEntry(modFile->getFileName(), false));

							if(zipArchiveEntry.expired()) {
								spdlog::error("Mod file '{}' not found in zip file '{}'.", modFile->getFileName(), zipArchiveFilePath);
								continue;
							}

							std::unique_ptr<ByteBuffer> zipArchiveEntryData(zipArchiveEntry.lock()->getData());

							if(zipArchiveEntryData == nullptr) {
								spdlog::error("Failed to read zip entry '{}' from zip file '{}' into memory.", zipArchiveEntry.lock()->getName(), zipArchiveFilePath);
								continue;
							}

							optionalSHA1 = zipArchiveEntryData->getSHA1();
						}
						else if(!groupFilePath.empty()) {
							if(group == nullptr) {
								spdlog::error("Skipping hash of mod file '{}' since group could not be opened.", groupFilePath);
								continue;
							}

							groupFile = group->getFile(modFile->getFileName());

							if(groupFile == nullptr) {
								spdlog::error("Mod file '{}' not found in group file '{}'.", modFile->getFileName(), groupFilePath);
								continue;
							}

							optionalSHA1 = groupFile->getData().getSHA1();
						}
					}
					else {
						modFilePath = Utilities::joinPaths(gameModsPath, modFile->getFileName());

						if(!std::filesystem::is_regular_file(std::filesystem::path(modFilePath))) {
							spdlog::warn("Skipping hash of missing '{}' mod file: '{}'.", mod.getFullName(i, j), modFilePath);
							continue;
						}

						optionalSHA1 = Utilities::getFileSHA1Hash(modFilePath);
					}

					if(!optionalSHA1.has_value()) {
						spdlog::error("Failed to hash '{}' mod file '{}'.", mod.getFullName(i, j), modFile->getFileName());
						continue;
					}

					if(modFile->getSHA1() != optionalSHA1.value()) {
						spdlog::info("Updating '{}' mod file '{}' SHA1 hash from '{}' to '{}'.", mod.getFullName(i, j), modFile->getFileName(), modFile->getSHA1(), optionalSHA1.value());

						modFile->setSHA1(optionalSHA1.value());

						numberOfFileHashesUpdated++;
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

	return numberOfFileHashesUpdated;
}

void ModManager::displayArgumentHelp() {
	fmt::print("Duke Nukem 3D Mod Manager version {} arguments:\n", APPLICATION_VERSION);
	fmt::print(" -f \"Custom Settings.json\" - specifies an alternate settings file to use.\n");
	fmt::print(" --type Game/Setup/Client/Server - specifies game type, default: Game.\n");
	fmt::print(" --game \"Game Version\" - specifies the game version to run.\n");
	fmt::print(" --ip 127.0.0.1 - specifies host ip address if running in client mode.\n");
	fmt::print(" --port 1337 - Specifies server port when running in client or server mode.\n");
	fmt::print(" -g MOD.GRP - manually specifies a group or zip file to use. Can be specified multiple times.\n");
	fmt::print(" -x MOD.CON - manually specifies a game con file to use.\n");
	fmt::print(" -h MOD.DEF - manually specifies a game def file to use.\n");
	fmt::print(" --map ZOO.MAP - manually specifies a user map file to load.\n");
	fmt::print(" --search \"Full Mod Name\" - searches for and selects the mod with a full or partially matching name, and optional version / type.\n");
	fmt::print(" --random - randomly selects a mod to run.\n");
	fmt::print(" -n - runs normal Duke Nukem 3D without any mods.\n");
	fmt::print(" -v # - selects an episode (1-4+).\n");
	fmt::print(" -l # - selects a level (1-11).\n");
	fmt::print(" -s # - selects a skill level (1-4).\n");
	fmt::print(" -r - enables demo recording.\n");
	fmt::print(" -d DEMO3.DMO - plays back the specified demo file.\n");
	fmt::print(" -t # - respawn mode: 1 = monsters, 2 = items, 3 = inventory, x = all.\n");
	fmt::print(" -u 8675309241 - set preferred weapon switch order, as a string of 10 digits.\n");
	fmt::print(" -m disable monsters.\n");
	fmt::print(" --ns disable sound.\n");
	fmt::print(" --nm disable music.\n");
	fmt::print(" --local - runs the mod manager in local mode.\n");
	fmt::print(" -- <args> - specify arguments to pass through to the target game executable when executing.\n");
	fmt::print(" --hash-new - updates unhashed SHA1 file hashes (developer use only!).\n");
	fmt::print(" --hash-all - updates all SHA1 file hashes (developer use only!).\n");
	fmt::print(" --version - displays the application version.\n");
	fmt::print(" -? - displays this help message.\n");
}

bool ModManager::createTemporaryDirectory() {
	if(m_settings->tempDirectoryPath.empty()) {
		spdlog::error("Missing temp directory path setting.");
		return false;
	}

	std::error_code errorCode;
	std::filesystem::path tempDirectoryPath(m_settings->tempDirectoryPath);

	if(!std::filesystem::is_directory(tempDirectoryPath)) {
		std::filesystem::create_directories(tempDirectoryPath, errorCode);

		if(errorCode) {
			spdlog::error("Failed to create mod manager temporary directory structure '{}': {}", tempDirectoryPath.string(), errorCode.message());
			return false;
		}

		spdlog::debug("Created mod manager temporary directory structure: '{}'.", tempDirectoryPath.string());
	}

	return true;
}

bool ModManager::areSymlinkSettingsValid() const {
	return !m_settings->gameSymlinkName.empty() &&
		   !m_settings->tempSymlinkName.empty() &&
		   !m_settings->modsSymlinkName.empty() &&
		   !m_settings->mapsSymlinkName.empty();
}

bool ModManager::createSymlink(const std::string & symlinkTarget, const std::string & symlinkName, const std::string & symlinkDestinationDirectory) const {
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
			spdlog::error("Failed to remove existing '{}' symlink, unexpected file system entry type.");
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

bool ModManager::createSymlinks(const GameVersion & gameVersion, bool createTempSymlink) {
	if(!gameVersion.isConfigured() || !areSymlinkSettingsValid() || (createTempSymlink && m_settings->tempDirectoryPath.empty())) {
		return false;
	}

	bool result = true;

	result &= createSymlink(gameVersion.getGamePath(), m_settings->gameSymlinkName, std::filesystem::current_path().string());

	if(createTempSymlink) {
		result &= createSymlink(m_settings->tempDirectoryPath, m_settings->tempSymlinkName, gameVersion.getGamePath());
	}

	result &= createSymlink(getModsDirectoryPath(), m_settings->modsSymlinkName, gameVersion.getGamePath());

	std::string mapsDirectoryPath(getMapsDirectoryPath());

	if(!mapsDirectoryPath.empty()) {
		result &= createSymlink(mapsDirectoryPath, m_settings->mapsSymlinkName, gameVersion.getGamePath());
	}

	return result;
}

bool ModManager::removeSymlinks(const GameVersion & gameVersion) {
	if(!gameVersion.isConfigured() || !areSymlinkSettingsValid()) {
		return false;
	}

	bool result = true;

	result &= removeSymlink(m_settings->gameSymlinkName, std::filesystem::current_path().string());

	result &= removeSymlink(m_settings->tempSymlinkName, gameVersion.getGamePath());

	result &= removeSymlink(m_settings->modsSymlinkName, gameVersion.getGamePath());

	std::string mapsDirectoryPath(getMapsDirectoryPath());

	if(!mapsDirectoryPath.empty()) {
		result &= removeSymlink(m_settings->mapsSymlinkName, gameVersion.getGamePath());
	}

	return result;
}

size_t ModManager::deleteFilesWithSuffix(const std::string & suffix, const std::string & path) {
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

void ModManager::printSpacing(size_t length) {
	for(size_t i = 0; i < length; i++) {
		fmt::print(" ");
	}
}
