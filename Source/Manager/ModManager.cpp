#include "ModManager.h"

#include "Download/DownloadManager.h"
#include "Game/GameVersion.h"
#include "Game/GameVersionCollection.h"
#include "Group/Group.h"
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

#include <fmt/core.h>
#include <magic_enum.hpp>

#include <array>
#include <conio.h>
#include <errno.h>
#include <filesystem>
#include <iostream>
#include <optional>
#include <regex>
#include <sstream>

const GameType ModManager::DEFAULT_GAME_TYPE = GameType::Game;
const std::string ModManager::DEFAULT_PREFERRED_GAME_VERSION(GameVersion::ATOMIC_EDITION.getName());
const std::string ModManager::HTTP_USER_AGENT("DukeNukem3DModManager/" + APPLICATION_VERSION);

ModManager::ModManager()
	: Application()
	, m_initialized(false)
	, m_verbose(false)
	, m_localMode(false)
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
#if _DEBUG
	m_verbose = true;
#endif // _DEBUG
}

ModManager::~ModManager() { }

bool ModManager::isInitialized() const {
	return m_initialized;
}

bool ModManager::initialize(int argc, char * argv[], bool start) {
	return initialize(&ArgumentParser(argc, argv), start);
}

bool ModManager::initialize(const ArgumentParser * args, bool start) {
	if(m_initialized) {
		return true;
	}

	if(args != nullptr) {
		m_arguments = std::make_shared<ArgumentParser>(*args);

		if(m_arguments->hasArgument("?")) {
			displayArgumentHelp();
			return false;
		}

		if(m_arguments->hasArgument("verbose")) {
			m_verbose = true;
		}

		if(m_arguments->hasArgument("local")) {
			m_localMode = true;
		}
	}

	m_settings->load(m_arguments.get());

	if(m_settings->verbose) {
		m_verbose = true;
	}

	HTTPConfiguration configuration = {
		Utilities::joinPaths(m_settings->dataDirectoryPath, m_settings->curlDataDirectoryName, m_settings->certificateAuthorityStoreFileName),
		m_settings->apiBaseURL,
		m_settings->connectionTimeout,
		m_settings->networkTimeout
	};

	if(!m_httpService->initialize(configuration)) {
		fmt::print("Failed to initialize HTTP service!\n");
		return false;
	}

	m_httpService->setUserAgent(HTTP_USER_AGENT);

	GeoLocationService * geoLocationService = GeoLocationService::getInstance();

	if(!geoLocationService->initialize(m_httpService, "a96b0340-b491-11ec-a512-43b18b43a434")) {
		fmt::print("Failed to initialize geo location service!\n");
		return false;
	}

	if(!m_localMode) {
		m_downloadManager = std::make_unique<DownloadManager>(m_httpService, m_settings);

		if(!m_downloadManager->initialize()) {
			fmt::print("Failed to initialize download manager!\n");
			return false;
		}
	}

	m_gameType = m_settings->gameType;

	bool saveGameVersions = false;
	bool gameVersionsLoaded = m_gameVersions->loadFrom(m_settings->gameVersionsListFilePath);

	if(!gameVersionsLoaded || m_gameVersions->numberOfGameVersions() == 0) {
		if(!gameVersionsLoaded) {
			fmt::print("Missing or invalid game versions configuration file '{}', using default values.\n", m_settings->gameVersionsListFilePath);
		}
		else if(m_gameVersions->numberOfGameVersions() == 0) {
			fmt::print("Empty game versions configuration file '{}', using default values.\n", m_settings->gameVersionsListFilePath);
		}

		// use default game version configurations
		m_gameVersions->setDefaultGameVersions();
	}

	m_gameVersions->addMissingDefaultGameVersions();

	m_preferredGameVersion = m_gameVersions->getGameVersion(m_settings->preferredGameVersion);

	if(m_preferredGameVersion == nullptr) {
		m_preferredGameVersion = m_gameVersions->getGameVersion(0);
		m_settings->preferredGameVersion = m_preferredGameVersion->getName();

		fmt::print("Game configuration for game version '{}' is missing, changing preferred game version to '{}.\n", m_settings->preferredGameVersion, m_preferredGameVersion->getName());
	}

	if(!m_mods->loadFrom(getModsListFilePath())) {
		fmt::print("Failed to load mod list '{}'!\n", getModsListFilePath());
		return false;
	}

	if(m_mods->numberOfMods() == 0) {
		fmt::print("No mods loaded!\n");
		return false;
	}

	if(!m_mods->checkGameVersions(*m_gameVersions)) {
		fmt::print("Found at least one invalid or missing game version.\n");
		return false;
	}

	fmt::print("Loaded {} mod{} from '{}'.\n", m_mods->numberOfMods(), m_mods->numberOfMods() == 1 ? "" : "s", getModsListFilePath());

	m_favouriteMods->loadFrom(m_settings->favouriteModsListFilePath);
	m_favouriteMods->checkForMissingFavouriteMods(*m_mods.get());

	if(m_favouriteMods->numberOfFavourites() != 0) {
		fmt::print("Loaded {} favourite mod{} from '{}'.\n", m_favouriteMods->numberOfFavourites(), m_favouriteMods->numberOfFavourites() == 1 ? "" : "s", m_settings->favouriteModsListFilePath);
	}

	m_organizedMods->organize();

	m_initialized = true;

	checkForMissingExecutables();

	if(m_localMode) {
		checkAllModsForMissingFiles();

		checkForUnlinkedModFiles();
	}

	if(!handleArguments(m_arguments.get(), start)) {
		return false;
	}

	return true;
}

bool ModManager::uninitialize() {
	if(!m_initialized) {
		return false;
	}

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

	std::shared_ptr<GameVersion> selectedGameVersion;

	if(m_arguments != nullptr && m_arguments->hasArgument("v") && !m_arguments->getFirstValue("v").empty()) {
		std::string gameVersionName(m_arguments->getFirstValue("v"));

		selectedGameVersion = m_gameVersions->getGameVersion(gameVersionName);

		if(selectedGameVersion == nullptr) {
			fmt::print("Could not find game version override for '{}'.\n", gameVersionName);
			return {};
		}
	}
	else {
		if(m_preferredGameVersion == nullptr) {
			fmt::print("No preferred game version selected.\n");
			return false;
		}

		selectedGameVersion = m_preferredGameVersion;
	}

	if(!selectedGameVersion->isValid()) {
		fmt::print("Invalid preferred game version.\n");
		return false;
	}

	if(!selectedGameVersion->isConfigured()) {
		fmt::print("Preferred game version is not configured.\n");
		return false;
	}

	std::shared_ptr<ModVersion> selectedModVersion;
	std::shared_ptr<ModVersionType> selectedModVersionType;
	std::shared_ptr<ModGameVersion> selectedModGameVersion;

	if(m_selectedMod != nullptr) {
		if(checkModForMissingFiles(*m_selectedMod) != 0) {
			fmt::print("Mod is missing files, aborting execution.\n");
			return false;
		}

		if(m_selectedModVersionIndex == std::numeric_limits<size_t>::max()) {
			setSelectedModVersionIndex(0);

			if(m_selectedMod->numberOfVersions() > 1) {
				if(!m_cli->updateSelectedModVersionPrompt()) {
					return false;
				}

				fmt::print("\n");
			}
		}

		selectedModVersion = m_selectedMod->getVersion(m_selectedModVersionIndex);

		if(m_selectedModVersionTypeIndex == std::numeric_limits<size_t>::max()) {
			setSelectedModVersionTypeIndex(0);

			if (selectedModVersion->numberOfTypes() > 1) {
				if (!m_cli->updateSelectedModVersionTypePrompt()) {
					return false;
				}

				fmt::print("\n");
			}
		}

		selectedModVersionType = selectedModVersion->getType(m_selectedModVersionTypeIndex);

		std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(selectedGameVersion->getCompatibleModGameVersions(selectedModVersionType->getGameVersions()));

		if(compatibleModGameVersions.empty()) {
			std::string additionalPromptMessage(fmt::format("{} is not supported on {}.", m_selectedMod->getFullName(m_selectedModVersionIndex, m_selectedModVersionTypeIndex), selectedGameVersion->getName()));

			std::optional<std::pair<std::shared_ptr<GameVersion>, std::shared_ptr<ModGameVersion>>> optionalGameAndModGameVersionSelection(m_cli->runUnsupportedModVersionTypePrompt(selectedModVersionType, additionalPromptMessage));

			if(!optionalGameAndModGameVersionSelection.has_value()) {
				fmt::print("\nAlternative game version not selected, aborting launch.\n");

				return false;
			}

			fmt::print("Using game version '{}' since '{}' is not supported on '{}'.\n\n", optionalGameAndModGameVersionSelection.value().first->getName(), m_selectedMod->getFullName(m_selectedModVersionIndex, m_selectedModVersionTypeIndex), selectedGameVersion->getName());

			selectedGameVersion = optionalGameAndModGameVersionSelection.value().first;
			selectedModGameVersion = optionalGameAndModGameVersionSelection.value().second;
		}
		else {
			selectedModGameVersion = compatibleModGameVersions[0];
		}
	}

	if(!m_localMode && selectedModGameVersion != nullptr) {
		if(!m_downloadManager->downloadModGameVersion(selectedModGameVersion.get(), m_gameVersions.get())) {
			fmt::print("Aborting launch of '{}' mod!\n", selectedModGameVersion->getFullName());
			return false;
		}

		fmt::print("\n");
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

	scriptArgs.addArgument("GROUPFLAG", selectedGameVersion->getGroupFileArgumentFlag());
	scriptArgs.addArgument("CONFLAG", selectedGameVersion->getConFileArgumentFlag());
	scriptArgs.addArgument("MAPFLAG", selectedGameVersion->getMapFileArgumentFlag());

	if(selectedGameVersion->hasDefFileArgumentFlag()) {
		scriptArgs.addArgument("DEFFLAG", selectedGameVersion->getDefFileArgumentFlag().value());
	}

	scriptArgs.addArgument("MODSDIR", m_settings->modsSymlinkName);
	scriptArgs.addArgument("MAPSDIR", m_settings->mapsSymlinkName);

	if(selectedModGameVersion != nullptr) {
		std::optional<std::string> conFileName(selectedModGameVersion->getFirstFileNameOfType("con"));

		if(conFileName.has_value()) {
			scriptArgs.addArgument("CON", *conFileName);
		}

		std::vector<std::string> groupFileNames = selectedModGameVersion->getFileNamesOfType("grp");

		for(const std::string & groupFileName : groupFileNames) {
			scriptArgs.addArgument("GROUP", groupFileName);
		}

		std::vector<std::string> zipFileNames = selectedModGameVersion->getFileNamesOfType("zip");

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

	if(!selectedGameVersion->doesRequiresDOSBox() && (m_gameType == GameType::Client || m_gameType == GameType::Server)) {
		fmt::print("Network settings are only supported when running in DOSBox, ignoring {} game type setting.\n\n", Utilities::toCapitalCase(std::string(magic_enum::enum_name(m_gameType))));
		fmt::print("\n");
	}

	bool customMod = false;
	std::string customMap;

	std::string command(generateCommand(selectedModGameVersion, selectedGameVersion, scriptArgs, &customMod, &customMap));

	if(command.empty()) {
		fmt::print("Failed to generate command.\n");
		return false;
	}

	if(!createSymlinks(*selectedGameVersion, m_verbose)) {
		return false;
	}

	if(m_verbose) {
		fmt::print("\n");
	}

	if(m_selectedMod != nullptr) {
		if(ModManager::renameFilesWithSuffixTo("DMO", "DMO_", selectedGameVersion->getGamePath(), m_verbose) != 0) {
			fmt::print("\n");
		}

		std::vector<std::shared_ptr<ModFile>> modFiles(selectedModGameVersion->getFilesOfType("grp"));

		for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = modFiles.begin(); i != modFiles.end(); ++i) {
			std::unique_ptr<Group> group(std::make_unique<Group>(Utilities::joinPaths(getModsDirectoryPath(), m_gameVersions->getGameVersion(selectedModGameVersion->getGameVersion())->getModDirectoryName(), (*i)->getFileName())));

			if(group->load()) {
				size_t numberOfDemosExtracted = group->extractAllFilesWithExtension("DMO", selectedGameVersion->getGamePath());

				fmt::print("Extracted {} demo{} from group file '{}' to game directory '{}'.\n\n", numberOfDemosExtracted, numberOfDemosExtracted == 1 ? "" : "s", group->getFilePath(), selectedGameVersion->getGamePath());
			}
		}
	}

	std::string customMapMessage;

	if(!customMap.empty()) {
		customMapMessage = fmt::format(" with custom map '{}'", customMap);
	}

	if(customMod) {
		fmt::print("Running custom mod in {} mode{}.\n", Utilities::toCapitalCase(std::string(magic_enum::enum_name(m_gameType))), customMapMessage);
	}
	else if(m_selectedMod != nullptr) {
		fmt::print("Running '{}' version of mod '{}' in {} mode{}.\n", selectedModGameVersion->getGameVersion(), m_selectedMod->getFullName(m_selectedModVersionIndex, m_selectedModVersionTypeIndex), Utilities::toCapitalCase(std::string(magic_enum::enum_name(m_gameType))), customMapMessage);
	}

	fmt::print("\n");

	if(m_verbose) {
		fmt::print("Executing Command: {}\n\n", command);
	}

	system(command.c_str());

	if(m_selectedMod != nullptr) {
		if(ModManager::deleteFilesWithSuffix("DMO", selectedGameVersion->getGamePath(), m_verbose) != 0) {
			fmt::print("\n");
		}

		if(ModManager::renameFilesWithSuffixTo("DMO_", "DMO", selectedGameVersion->getGamePath(), m_verbose) != 0) {
			fmt::print("\n");
		}
	}

	removeSymlinks(*selectedGameVersion, m_verbose);

	return true;
}

std::string ModManager::generateCommand(std::shared_ptr<ModGameVersion> modGameVersion, std::shared_ptr<GameVersion> gameVersion, ScriptArguments & scriptArgs, bool * customMod, std::string * customMap) const {
	if(!m_initialized) {
		fmt::print("Mod manager not initialized.\n");
		return {};
	}

	if(!gameVersion->isConfigured()) {
		fmt::print("Invalid or unconfigured game version.\n");
		return {};
	}

	if(m_settings->dataDirectoryPath.empty()) {
		fmt::print("Empty data path.\n");
		return {};
	}

	if(gameVersion->doesRequiresDOSBox() && m_settings->dosboxDirectoryPath.empty()) {
		fmt::print("Empty DOSBox path.\n");
		return {};
	}

	if(m_settings->modsSymlinkName.empty()) {
		fmt::print("Empty mods directory symbolic link name.\n");
		return {};
	}

	if(!GameVersionCollection::isValid(m_gameVersions.get())) {
		fmt::print("Invalid game version collection.\n");
		return {};
	}

	std::shared_ptr<GameVersion> targetGameVersion;

	if(modGameVersion != nullptr) {
		if(!modGameVersion->isValid()) {
			fmt::print("Invalid mod game version.\n");
			return {};
		}

		if(!Utilities::areStringsEqualIgnoreCase(modGameVersion->getGameVersion(), gameVersion->getName()) && !gameVersion->hasCompatibleGameVersion(modGameVersion->getGameVersion())) {
			fmt::print("Game version '{}' is not compatible with '{}'.\n", gameVersion->getName(), modGameVersion->getGameVersion());
			return {};
		}

		targetGameVersion = m_gameVersions->getGameVersion(modGameVersion->getGameVersion());

		if(targetGameVersion == nullptr) {
			fmt::print("Missing game configuration for '{}'.\n", modGameVersion->getGameVersion());
			return {};
		}
	}
	else {
		targetGameVersion = gameVersion;
	}

	std::string executableName;

	if(m_gameType == GameType::Setup) {
		if(!gameVersion->hasSetupExecutableName()) {
			fmt::print("Game version '{}' does not have a setup executable.\n", gameVersion->getName());
			return {};
		}

		executableName = gameVersion->getSetupExecutableName().value();
	}
	else {
		executableName = gameVersion->getGameExecutableName();
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
				command << " " << gameVersion->getGroupFileArgumentFlag() << Utilities::joinPaths(modPath, *i);
			}
		}
		else {
			std::vector<std::shared_ptr<ModFile>> groupFiles(modGameVersion->getFilesOfType("grp"));

			for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = groupFiles.begin(); i != groupFiles.end(); ++i) {
				command << " " << gameVersion->getGroupFileArgumentFlag() << Utilities::joinPaths(modPath, (*i)->getFileName());
			}
		}

		if(!customGroupFiles.empty() || modGameVersion->isEDuke32()) {
			if(customGroupFiles.empty()) {
				std::vector<std::shared_ptr<ModFile>> zipFiles(modGameVersion->getFilesOfType("zip"));

				for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = zipFiles.begin(); i != zipFiles.end(); ++i) {
					command << " " << gameVersion->getGroupFileArgumentFlag() << Utilities::joinPaths(modPath, (*i)->getFileName());
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
				if(!gameVersion->hasDefFileArgumentFlag()) {
					fmt::print("Game version '{}' does not have a def file argument flag specified in its configuration.\n", gameVersion->getName());
					return {};
				}

				command << " " << gameVersion->getDefFileArgumentFlag().value() << defFileName;
			}
		}

		if(!conFileName.empty()) {
			command << " " << gameVersion->getConFileArgumentFlag() << (gameVersion->hasRelativeConFilePath() ? conFileName : Utilities::joinPaths(modPath, conFileName));
		}
	}

	if(m_arguments != nullptr) {
		if(m_arguments->hasArgument("map")) {
			if(m_settings->mapsSymlinkName.empty()) {
				fmt::print("Maps directory symbolic link name is empty.\n");
				return {};
			}

			std::string userMap(m_arguments->getFirstValue("map"));

			if(!userMap.empty()) {
				scriptArgs.addArgument("MAP", userMap);

				if(customMap != nullptr) {
					*customMap = userMap;
				}

				command << " " << gameVersion->getMapFileArgumentFlag();

				if(std::filesystem::is_regular_file(std::filesystem::path(Utilities::joinPaths(gameVersion->getGamePath(), userMap)))) {
					command << userMap;
				}
				else {
					std::string mapsDirectoryPath(getMapsDirectoryPath());

					if(mapsDirectoryPath.empty()) {
						fmt::print("Maps directory path is empty.\n");
						return {};
					}

					if(std::filesystem::is_regular_file(std::filesystem::path(Utilities::joinPaths(mapsDirectoryPath, userMap)))) {
						command << Utilities::joinPaths(m_settings->mapsSymlinkName, userMap);
					}
					else {
						fmt::print("Map '{}' does not exist in game or maps directories.\n", userMap);

						command << userMap;
					}
				}
			}
		}

		if(m_arguments->hasPassthroughArguments()) {
			command << " " << m_arguments->getPassthroughArguments().value();
		}
	}

	if(gameVersion->doesRequiresDOSBox()) {
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
			fmt::print("Failed to load DOSBox script file: '{}'.\n", dosboxScriptFilePath);
			return {};
		}

		return generateDOSBoxCommand(dosboxScript, scriptArgs, Utilities::joinPaths(m_settings->dosboxDirectoryPath, m_settings->dosboxExecutableFileName), m_settings->dosboxArguments);
	}

	return Utilities::joinPaths("\"" + gameVersion->getGamePath(), executableName) + "\"" + command.str();
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

		if(args->hasArgument("t")) {
			std::optional<GameType> newGameTypeOptional(magic_enum::enum_cast<GameType>(Utilities::toPascalCase(args->getFirstValue("t"))));

			if(newGameTypeOptional.has_value()) {
				m_gameType = newGameTypeOptional.value();

				fmt::print("Setting game type to: '{}'.\n", Utilities::toCapitalCase(std::string(magic_enum::enum_name(m_gameType))));

				if(m_gameType == GameType::Client) {
					std::string ipAddress;

					if(args->hasArgument("ip")) {
						ipAddress = Utilities::trimString(args->getFirstValue("ip"));

						bool error = ipAddress.empty() || ipAddress.find_first_of(" \t") != std::string::npos;

						if(error) {
							fmt::print("\nInvalid IP Address entered in arguments: '{}'.\n\n", ipAddress);

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
							fmt::print("\nInvalid {} Server Port entered in arguments: '{}'.\n\n", m_gameType == GameType::Server ? "Local" : "Remote", portData);

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

						gameTypesStream << Utilities::toCapitalCase(std::string(gameTypeNames[i]));
					}

					gameTypes = gameTypesStream.str();
				}

				fmt::print("Invalid game type, please specify one of the following: {}.\n", gameTypes);
				return false;
			}
		}

		if(args->hasArgument("s")) {
			if(args->hasArgument("r") || args->hasArgument("g") || args->hasArgument("x") || args->hasArgument("n")) {
				fmt::print("Redundant arguments specified, please specify either s OR r OR n OR (x AND/OR g).\n");
				return false;
			}

			std::vector<ModMatch> modMatches(searchForMod(m_mods->getMods(), args->getFirstValue("s")));

			if(modMatches.empty()) {
				fmt::print("No matches found for specified search query.\n\n");
				return false;
			}
			else if(modMatches.size() == 1) {
				const ModMatch & modMatch = modMatches[0];

				fmt::print("Selected {} from search query: '{}'.\n", Utilities::toCapitalCase(std::string(magic_enum::enum_name(modMatch.getMatchType()))), modMatch.toString());

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
		else if(args->hasArgument("r")) {
			if(args->hasArgument("g") || args->hasArgument("x") || args->hasArgument("n")) {
				fmt::print("Redundant arguments specified, please specify either s OR r OR n OR (x AND/OR g).\n");
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
				fmt::print("Redundant arguments specified, please specify either s OR r OR n OR (x AND/OR g).\n");
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

			fmt::print("Unlinked '{}' file {}: '{}' for '{}'.\n", gameVersion.getName(), numberOfUnlinkedModFiles, i->first, gameVersion.getName());
		}
	}

	if(numberOfUnlinkedModFiles != 0) {
		fmt::print("Found {} unlinked '{}' mod file{} in '{}' mods directory.\n", numberOfUnlinkedModFiles, gameVersion.getName(), numberOfUnlinkedModFiles == 1 ? "" : "s", gameVersion.getName());
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
					fmt::print("'{}' Mod file '{}' linked {} times.\n", gameVersion.getName(), i->first, i->second.size());

					numberOfMultipleLinkedModFiles++;
				}
			}
		}
	}

	if(numberOfMultipleLinkedModFiles != 0) {
		fmt::print("Found {} multiple linked '{}' mod file{} in '{}' mods directory. If a mod file is linked intentionally multiple times within the same game version, it must have its shared property set to true.\n", numberOfMultipleLinkedModFiles, gameVersion.getName(), numberOfMultipleLinkedModFiles == 1 ? "" : "s", gameVersion.getName());
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
					fmt::print("Skipping checking invalid '{}' mod game version '{}', invalid game configuration.\n", mod.getFullName(i, j), modGameVersion->getGameVersion());
					continue;
				}

				gameModsPath = Utilities::joinPaths(m_settings->modsDirectoryPath, gameVersion->getModDirectoryName());

				if(!std::filesystem::is_directory(gameModsPath)) {
					fmt::print("Skipping checking '{}' mod game version '{}', base directory is missing or not a valid directory: '{}'.\n", mod.getFullName(i, j), gameVersion->getName(), gameModsPath);
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

						fmt::print("Mod '{}' is missing {} {} file: '{}'.\n", mod.getFullName(i, j), modGameVersion->getGameVersion(), modFile->getType(), modFile->getFileName());

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
		fmt::print("Found {} missing mod file{} in mods directory.\n", numberOfMissingModFiles, numberOfMissingModFiles == 1 ? "" : "s");
	}

	return numberOfMissingModFiles;
}

size_t ModManager::checkForMissingExecutables() const {
	size_t numberOfMissingExecutables = 0;

	std::string fullDOSBoxExecutablePath(Utilities::joinPaths(m_settings->dosboxDirectoryPath, m_settings->dosboxExecutableFileName));

	if(!std::filesystem::is_regular_file(std::filesystem::path(fullDOSBoxExecutablePath))) {
		numberOfMissingExecutables++;

		fmt::print("Missing DOSBox executable: '{}'.\n", fullDOSBoxExecutablePath);
	}

	numberOfMissingExecutables += m_gameVersions->checkForMissingExecutables();

	return numberOfMissingExecutables++;
}

size_t ModManager::updateAllFileHashes(bool save, bool skipHashedFiles) {
	if(!m_initialized) {
		return 0;
	}

	if(m_settings->modPackageDownloadsDirectoryPath.empty()) {
		fmt::print("Mod package downloads directory path not set, cannot hash some download files!\n");
	}

	if(m_settings->modSourceFilesDirectoryPath.empty()) {
		fmt::print("Mod source files directory path not set, cannot hash some download files!\n");
	}

	if(m_settings->modImagesDirectoryPath.empty()) {
		fmt::print("Mod images directory path not set, cannot hash screenshots or images!\n");
	}

	fmt::print("Updating {} file hashes...\n", skipHashedFiles ? "unhashed" : "all");

	size_t numberOfFileHashesUpdated = 0;

	for(size_t i = 0; i < m_mods->numberOfMods(); i++) {
		numberOfFileHashesUpdated += updateModHashes(*m_mods->getMod(i), skipHashedFiles);
	}

	if(numberOfFileHashesUpdated != 0) {
		fmt::print("Updated {} file hash{}.\n", numberOfFileHashesUpdated, numberOfFileHashesUpdated == 1 ? "" : "es");
	}
	else {
		fmt::print("No file hashes updated.\n");
	}

	if(save) {
		if(m_mods->saveTo(m_settings->modsListFilePath)) {
			fmt::print("Saved updated mod list to file: '{}'.\n", m_settings->modsListFilePath);
		}
		else {
			fmt::print("Failed to save updated mod list to file: '{}'!\n", m_settings->modsListFilePath);
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

	fmt::print("Updating '{}' mod hashes...\n", mod.getName());

	// hash mod downloads
	for(size_t i = 0; i < mod.numberOfDownloads(); i++) {
		modDownload = mod.getDownload(i);

		if(!ModDownload::isValid(modDownload.get())) {
			fmt::print("Skipping hash of invalid download file #{} for mod '{}'.\n", i + 1, mod.getName());
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
				fmt::print("Could not find game configuration for game version '{}', skipping hash of download file: '{}'.\n", modDownload->getGameVersion(), modDownload->getFileName());
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
			fmt::print("Skipping hash of missing '{}' mod download file: '{}'.\n", mod.getName(), downloadFilePath);
			continue;
		}

		optionalSHA1 = Utilities::getFileSHA1Hash(downloadFilePath);

		if(!optionalSHA1.has_value()) {
			fmt::print("Failed to hash mod '{}' download file '{}'.\n", mod.getName(), modFile->getFileName());
			continue;
		}

		if(modDownload->getSHA1() != optionalSHA1.value()) {
			fmt::print("Updating mod '{}' download file '{}' SHA1 hash from '{}' to '{}'.\n", mod.getName(), modDownload->getFileName(), modDownload->getSHA1(), optionalSHA1.value());

			modDownload->setSHA1(optionalSHA1.value());

			numberOfFileHashesUpdated++;
		}
	}

	// hash mod screenshots
	if(!m_settings->modImagesDirectoryPath.empty()) {
		for(size_t i = 0; i < mod.numberOfScreenshots(); i++) {
			modScreenshot = mod.getScreenshot(i);

			if(!ModScreenshot::isValid(modScreenshot.get())) {
				fmt::print("Skipping hash of invalid screenshot file #{} for mod '{}'.\n", i + 1, mod.getName());
				continue;
			}

			if(skipHashedFiles && !modScreenshot->getSHA1().empty()) {
				continue;
			}

			screenshotFilePath = Utilities::joinPaths(m_settings->modImagesDirectoryPath, mod.getID(), "screenshots", "lg", modScreenshot->getFileName());

			if(!std::filesystem::is_regular_file(std::filesystem::path(screenshotFilePath))) {
				fmt::print("Skipping hash of missing '{}' mod screenshot file: '{}'.\n", mod.getName(), screenshotFilePath);
				continue;
			}

			optionalSHA1 = Utilities::getFileSHA1Hash(screenshotFilePath);

			if(!optionalSHA1.has_value()) {
				fmt::print("Failed to hash mod '{}' screenshot file '{}'.\n", mod.getName(), modFile->getFileName());
				continue;
			}

			if(modScreenshot->getSHA1() != optionalSHA1.value()) {
				fmt::print("Updating mod '{}' screenshot file '{}' SHA1 hash from '{}' to '{}'.\n", mod.getName(), modScreenshot->getFileName(), modScreenshot->getSHA1(), optionalSHA1.value());

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
				fmt::print("Skipping hash of invalid image file #{} for mod '{}'.\n", i + 1, mod.getName());
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
				fmt::print("Skipping hash of missing '{}' mod image file: '{}'.\n", mod.getName(), imageFilePath);
				continue;
			}

			optionalSHA1 = Utilities::getFileSHA1Hash(imageFilePath);

			if(!optionalSHA1.has_value()) {
				fmt::print("Failed to hash mod '{}' image file '{}'.\n", mod.getName(), modFile->getFileName());
				continue;
			}

			if(modImage->getSHA1() != optionalSHA1.value()) {
				fmt::print("Updating mod '{}' image file '{}' SHA1 hash from '{}' to '{}'.\n", mod.getName(), modImage->getFileName(), modImage->getSHA1(), optionalSHA1.value());

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
					fmt::print("Mod '{}' game version #{} is not valid, skipping hashing of mod files.\n", mod.getFullName(i, j), k + 1);
					continue;
				}

				gameModsPath = Utilities::joinPaths(m_settings->modsDirectoryPath, gameVersion->getModDirectoryName());

				if(!std::filesystem::is_directory(gameModsPath)) {
					fmt::print("Mod '{}' '{}' game version directory '{}' does not exist or is not a valid directory, skipping hashing of mod files.\n", mod.getFullName(i, j), gameVersion->getName(), gameModsPath);
					continue;
				}

				if(modGameVersion->isEDuke32()) {
					std::shared_ptr<ModFile> modZipFile(modGameVersion->getFirstFileOfType("zip"));

					if(modZipFile != nullptr) {
						zipArchiveFilePath = Utilities::joinPaths(gameModsPath, modZipFile->getFileName());
						zipArchive = ZipArchive::readFrom(zipArchiveFilePath, Utilities::emptyString, true);

						if(zipArchive != nullptr) {
							fmt::print("Opened '{}' zip file '{}'.\n", modVersionType->getFullName(), zipArchiveFilePath);
						}
					}
					else {
						std::shared_ptr<ModFile> modGroupFile(modGameVersion->getFirstFileOfType("grp"));

						if(modGroupFile != nullptr) {
							groupFilePath = Utilities::joinPaths(gameModsPath, modGroupFile->getFileName());

							group = std::make_unique<Group>(groupFilePath);

							if(!group->load()) {
								group.reset();

								fmt::print("Failed to open mod group file '{}'.\n", groupFilePath);
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

					if(modGameVersion->isEDuke32() && modFile->getType() != "zip" && modFile->getType() != "grp") {
						if(!zipArchiveFilePath.empty()) {
							if(zipArchive == nullptr) {
								fmt::print("Skipping hash of mod file '{}' since zip archive could not be opened.\n", zipArchiveFilePath);
								continue;
							}

							std::weak_ptr<ZipArchive::Entry> zipArchiveEntry(zipArchive->getEntry(modFile->getFileName(), false));

							if(zipArchiveEntry.expired()) {
								fmt::print("Mod file '{}' not found in zip file '{}'.\n", modFile->getFileName(), zipArchiveFilePath);
								continue;
							}

							std::unique_ptr<ByteBuffer> zipArchiveEntryData(zipArchiveEntry.lock()->getData());

							if(zipArchiveEntryData == nullptr) {
								fmt::print("Failed to read zip entry '{}' from zip file '{}' into memory.\n", zipArchiveEntry.lock()->getName(), zipArchiveFilePath);
								continue;
							}

							optionalSHA1 = zipArchiveEntryData->getSHA1();
						}
						else if(!groupFilePath.empty()) {
							if(group == nullptr) {
								fmt::print("Skipping hash of mod file '{}' since group could not be opened.\n", groupFilePath);
								continue;
							}

							groupFile = group->getFile(modFile->getFileName());

							if(groupFile == nullptr) {
								fmt::print("Mod file '{}' not found in group file '{}'.\n", modFile->getFileName(), groupFilePath);
								continue;
							}

							optionalSHA1 = groupFile->getData().getSHA1();
						}
					}
					else {
						modFilePath = Utilities::joinPaths(gameModsPath, modFile->getFileName());

						if(!std::filesystem::is_regular_file(std::filesystem::path(modFilePath))) {
							fmt::print("Skipping hash of missing '{}' mod file: '{}'.\n", mod.getFullName(i, j), modFilePath);
							continue;
						}

						optionalSHA1 = Utilities::getFileSHA1Hash(modFilePath);
					}

					if(!optionalSHA1.has_value()) {
						fmt::print("Failed to hash '{}' mod file '{}'.\n", mod.getFullName(i, j), modFile->getFileName());
						continue;
					}

					if(modFile->getSHA1() != optionalSHA1.value()) {
						fmt::print("Updating '{}' mod file '{}' SHA1 hash from '{}' to '{}'.\n", mod.getFullName(i, j), modFile->getFileName(), modFile->getSHA1(), optionalSHA1.value());

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
	fmt::print(" -t Game/Setup/Client/Server - specifies game type, default: Game.\n");
	fmt::print(" -v \"Game Version\" - specifies the game version to run.\n");
	fmt::print(" --ip 127.0.0.1 - specifies host ip address if running in client mode.\n");
	fmt::print(" --port 1337 - Specifies server port when running in client or server mode.\n");
	fmt::print(" -g MOD.GRP - manually specifies a group or zip file to use. Can be specified multiple times.\n");
	fmt::print(" -x MOD.CON - manually specifies a game con file to use.\n");
	fmt::print(" -h MOD.DEF - manually specifies a game def file to use.\n");
	fmt::print(" --map ZOO.MAP - manually specifies a user map file to load.\n");
	fmt::print(" -s \"Full Mod Name\" - searches for and selects the mod with a full or partially matching name, and optional version / type.\n");
	fmt::print(" -r - randomly selects a mod to run.\n");
	fmt::print(" -n - runs normal Duke Nukem 3D without any mods.\n");
	fmt::print(" --local - runs the mod manager in local mode.\n");
	fmt::print(" -- <args> - specify arguments to pass through to the target game executable when executing.\n");
	fmt::print(" --hash-new - updates unhashed SHA1 file hashes (developer use only!).\n");
	fmt::print(" --hash-all - updates all SHA1 file hashes (developer use only!).\n");
	fmt::print(" --verbose - enables verbose debug output.\n");
	fmt::print(" -? - displays this help message.\n");
}

bool ModManager::createSymlink(const std::string & symlinkTarget, const std::string & symlinkName, const std::string & symlinkDestinationDirectory, bool verbose) const {
	if(symlinkName.empty() ||
	   symlinkTarget.empty()) {
		fmt::print("Failed to create symlink, invalid arguments provided.\n");
		return false;
	}

	std::filesystem::path symlinkTargetPath(symlinkTarget);

	if(!std::filesystem::is_directory(symlinkTargetPath)) {
		fmt::print("Failed to create '{}' symlink, target path '{}' does not exist or is not a valid directory.\n", symlinkName, symlinkTarget);
		return false;
	}

	std::string symlinkDestination(Utilities::joinPaths(symlinkDestinationDirectory.empty() ? std::filesystem::current_path().string() : symlinkDestinationDirectory, symlinkName));
	std::filesystem::path symlinkDestinationPath(symlinkDestination);

	if(std::filesystem::exists(symlinkDestinationPath)) {
		if(!std::filesystem::is_symlink(symlinkDestinationPath)) {
			fmt::print("Failed to remove existing '{}' symlink, unexpected file system entry type.\n");
			return false;
		}

		if(verbose) {
			fmt::print("Removing existing symlink: '{}'.\n", symlinkDestinationPath.string());
		}

		std::error_code errorCode;
		std::filesystem::remove(symlinkDestinationPath, errorCode);

		if(errorCode) {
			fmt::print("Failed to remove existing symlink '{}': {}\n", symlinkDestinationPath.string(), errorCode.message());
			return false;
		}
	}

	if(verbose) {
		fmt::print("Creating symlink '{}' to target '{}'.\n", symlinkDestination, symlinkTarget);
	}

	std::error_code errorCode;
	std::filesystem::create_directory_symlink(symlinkTarget, symlinkDestinationPath, errorCode);

	if(errorCode) {
		fmt::print("Failed to create symlink '{}' to target '{}': {}\n", symlinkDestination, symlinkTarget, errorCode.message());
		return false;
	}

	return true;
}

bool ModManager::removeSymlink(const std::string & symlinkName, const std::string & symlinkDestinationDirectory, bool verbose) const {
	if(symlinkName.empty()) {
		fmt::print("Failed to remove symlink, invalid arguments provided.\n");
		return false;
	}

	std::string symlinkDestination(Utilities::joinPaths(symlinkDestinationDirectory.empty() ? std::filesystem::current_path().string() : symlinkDestinationDirectory, symlinkName));
	std::filesystem::path symlinkDestinationPath(symlinkDestination);

	if(std::filesystem::exists(symlinkDestinationPath)) {
		if(!std::filesystem::is_symlink(symlinkDestinationPath)) {
			fmt::print("Failed to remove '{}' symlink, unexpected file system entry type.\n");
			return false;
		}

		if(verbose) {
			fmt::print("Removing symlink: '{}'.\n", symlinkDestination);
		}

		std::error_code errorCode;
		std::filesystem::remove(symlinkDestinationPath, errorCode);

		if(errorCode) {
			fmt::print("Failed to remove symlink '{}': {}\n", symlinkDestination, errorCode.message());
			return false;
		}
	}

	return true;
}

bool ModManager::createSymlinks(const GameVersion & gameVersion, bool verbose) {
	if(!gameVersion.isConfigured()) {
		return false;
	}

	bool result = true;

	result &= createSymlink(getModsDirectoryPath(), m_settings->modsSymlinkName, gameVersion.getGamePath(), verbose);

	std::string mapsDirectoryPath(getMapsDirectoryPath());

	if(!mapsDirectoryPath.empty()) {
		result &= createSymlink(mapsDirectoryPath, m_settings->mapsSymlinkName, gameVersion.getGamePath(), verbose);
	}

	return result;
}

bool ModManager::removeSymlinks(const GameVersion & gameVersion, bool verbose) {
	if(!gameVersion.isConfigured()) {
		return false;
	}

	bool result = true;

	result &= removeSymlink(m_settings->modsSymlinkName, gameVersion.getGamePath(), verbose);

	std::string mapsDirectoryPath(getMapsDirectoryPath());

	if(!mapsDirectoryPath.empty()) {
		result &= removeSymlink(m_settings->mapsSymlinkName, gameVersion.getGamePath(), verbose);
	}

	return result;
}

size_t ModManager::deleteFilesWithSuffix(const std::string & suffix, const std::string & path, bool verbose) {
	if(suffix.empty()) {
		return 0;
	}

	std::filesystem::path directoryPath(path.empty() ? std::filesystem::current_path() : std::filesystem::path(path));
	size_t numberOfFilesDeleted = 0;

	for(const std::filesystem::directory_entry & e : std::filesystem::directory_iterator(directoryPath)) {
		if(e.is_regular_file() && Utilities::areStringsEqualIgnoreCase(Utilities::getFileExtension(e.path().string()), suffix)) {

			if(verbose) {
				fmt::print("Deleting file: '{}'.\n", e.path().string());
			}

			std::error_code errorCode;
			std::filesystem::remove(e.path(), errorCode);

			if(errorCode) {
				fmt::print("Failed to delete file '{}': {}\n", e.path().string(), errorCode.message());
				continue;
			}

			numberOfFilesDeleted++;
		}
	}

	return numberOfFilesDeleted;
}

size_t ModManager::renameFilesWithSuffixTo(const std::string & fromSuffix, const std::string & toSuffix, const std::string & path, bool verbose) {
	if(fromSuffix.empty() || toSuffix.empty()) {
		return 0;
	}

	std::filesystem::path directoryPath(path.empty() ? std::filesystem::current_path() : std::filesystem::path(path));
	std::string newFilePath;
	size_t numberOfFilesRenamed = 0;

	for(const std::filesystem::directory_entry & e : std::filesystem::directory_iterator(directoryPath)) {
		if(e.is_regular_file() && Utilities::areStringsEqualIgnoreCase(Utilities::getFileExtension(e.path().string()), fromSuffix)) {
			newFilePath = Utilities::replaceFileExtension(e.path().string(), toSuffix);

			if(verbose) {
				fmt::print("Renaming file: '{}' > '{}'.\n", e.path().string(), newFilePath);
			}

			std::error_code error;
			std::filesystem::rename(e.path(), newFilePath, error);

			if(error) {
				fmt::print("Failed to rename file '{}' to '{}': {}\n", e.path().string(), newFilePath, error.message());
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
