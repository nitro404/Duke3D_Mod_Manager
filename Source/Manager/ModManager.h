#ifndef _MOD_MANAGER_H_
#define _MOD_MANAGER_H_

#include "Game/GameType.h"
#include "Mod/OrganizedModCollection.h"

#include <Analytics/Segment/SegmentAnalytics.h>
#include <Application/Application.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class ArgumentParser;
class DownloadManager;
class FavouriteModCollection;
class GameManager;
class GameVersion;
class GameVersionCollection;
class HTTPService;
class Mod;
class ModAuthorInformation;
class ModCollection;
class ModGameVersion;
class ModMatch;
class ModVersion;
class ModVersionType;
class Script;
class ScriptArguments;
class SettingsManager;

class ModManager final : public Application {
public:
	ModManager();
	virtual ~ModManager();

	bool isInitialized() const;
	bool initialize(int argc, char * argv[], bool start = true);
	bool uninitialize();

	void run();

	bool isUsingLocalMode() const;
	std::shared_ptr<SettingsManager> getSettings() const;
	std::shared_ptr<HTTPService> getHTTPService() const;
	std::shared_ptr<OrganizedModCollection> getOrganizedMods() const;
	std::string getModsListFilePath() const;
	std::string getModsDirectoryPath() const;
	std::string getMapsDirectoryPath() const;

	GameType getGameType() const;
	bool setGameType(const std::string & gameType);
	void setGameType(GameType gameType);

	bool hasPreferredGameVersion() const;
	std::shared_ptr<GameVersion> getPreferredGameVersion() const;
	bool setPreferredGameVersion(const std::string & gameVersionName);
	bool setPreferredGameVersion(std::shared_ptr<GameVersion> gameVersion);

	std::shared_ptr<GameVersionCollection> getGameVersions() const;

	const std::string & getDOSBoxServerIPAddress() const;
	void setDOSBoxServerIPAddress(const std::string & ipAddress);

	std::shared_ptr<Mod> getSelectedMod() const;
	std::shared_ptr<ModVersion> getSelectedModVersion() const;
	std::shared_ptr<ModVersionType> getSelectedModVersionType() const;
	std::optional<std::string> getSelectedModName() const;
	size_t getSelectedModVersionIndex() const;
	size_t getSelectedModVersionTypeIndex() const;
	bool setSelectedMod(const std::string & name);
	bool setSelectedMod(std::shared_ptr<Mod> mod);
	bool setSelectedModVersionIndex(size_t modVersionIndex);
	bool setSelectedModVersionTypeIndex(size_t modVersionTypeIndex);
	bool selectRandomMod(bool selectPreferredVersion, bool selectFirstVersionType);
	bool selectRandomGameVersion();
	bool selectRandomTeam();
	bool selectRandomAuthor();
	static std::vector<ModMatch> searchForMod(const std::vector<std::shared_ptr<Mod>> & mods, const std::string & query);
	size_t searchForAndSelectGameVersion(const std::string & query, std::vector<std::shared_ptr<GameVersion>> * matches = nullptr);
	size_t searchForAndSelectTeam(const std::string & query, std::vector<std::shared_ptr<ModAuthorInformation>> * matches = nullptr);
	size_t searchForAndSelectAuthor(const std::string & query, std::vector<std::shared_ptr<ModAuthorInformation>> * matches = nullptr);
	void clearSelectedMod();

	bool runSelectedMod();

	static const GameType DEFAULT_GAME_TYPE;
	static const std::string DEFAULT_PREFERRED_GAME_VERSION;
	static const std::string HTTP_USER_AGENT;

private:
	class CLI final {
	public:
		CLI(ModManager * modManager);
		~CLI();

		void runMenu();
		bool updateFilterTypePrompt(const std::string & args = "");
		std::optional<OrganizedModCollection::FilterType> promptForFilterType(const std::string & args = "") const;
		bool runSortPrompt(const std::string & args = "");
		bool updateGameTypePrompt(const std::string & args = "");
		std::optional<GameType> promptForGameType(const std::string & args = "") const;
		bool updatePreferredGameVersionPrompt(const std::string & args = "");
		std::shared_ptr<GameVersion> promptForPreferredGameVersion(const std::string & args = "") const;
		std::optional<std::pair<std::shared_ptr<GameVersion>, std::shared_ptr<ModGameVersion>>> runUnsupportedModVersionTypePrompt(std::shared_ptr<ModVersionType> modVersionType, const std::string & promptMessage = "") const;
		bool updateIPAddressPrompt(const std::string & args = "");
		std::string promptForIPAddress(const std::string & args = "") const;
		bool updatePortPrompt(const std::string & args = "");
		std::optional<uint16_t> promptForPort(const std::string & args = "") const;
		bool runSelectRandomModPrompt(bool selectPreferredVersion, bool selectFirstVersionType);
		bool runSearchPrompt(const std::string & args = "");
		bool updateSelectedModVersionPrompt();
		bool updateSelectedModVersionTypePrompt();

	private:
		static size_t getValuePrompt(const std::vector<std::string> & values, const std::string & promptMessage = "", const std::string & args = "");
		static std::string getArguments(const std::string & data);
		static void pause();
		static void clearOutput();

		ModManager * m_modManager;

		CLI(const CLI &) = delete;
		CLI(CLI &&) noexcept = delete;
		const CLI & operator = (const CLI &) = delete;
		const CLI & operator = (CLI &&) noexcept = delete;
	};

	void assignPlatformFactories();
	bool handleArguments(const ArgumentParser * args, bool start);
	std::string generateCommand(std::shared_ptr<ModGameVersion> modGameVersion, std::shared_ptr<GameVersion> selectedGameVersion, ScriptArguments & scriptArgs, std::string_view combinedGroupFileName = "", bool * customMod = nullptr, std::string * customMap = nullptr) const;
	std::string generateDOSBoxCommand(const Script & script, const ScriptArguments & arguments, const std::string & dosboxPath, const  std::string & dosboxArguments) const;
	size_t checkForUnlinkedModFiles() const;
	size_t checkForUnlinkedModFilesForGameVersion(const GameVersion & gameVersion) const;
	size_t checkModForMissingFiles(const std::string & modName, std::optional<size_t> versionIndex = {}, std::optional<size_t> versionTypeIndex = {}) const;
	size_t checkModForMissingFiles(const Mod & mod, std::optional<size_t> versionIndex = {}, std::optional<size_t> versionTypeIndex = {}) const;
	size_t checkAllModsForMissingFiles() const;
	size_t checkForMissingExecutables() const;
	size_t updateAllFileHashes(bool save = true, bool skipHashedFiles = true);
	size_t updateModHashes(Mod & mod, bool skipHashedFiles = true, std::optional<size_t> versionIndex = {}, std::optional<size_t> versionTypeIndex = {});
	bool createTemporaryDirectory();
	bool areSymlinkSettingsValid() const;
	bool createSymlink(const std::string & symlinkName, const std::string & symlinkTarget, const std::string & symlinkDestinationDirectory) const;
	bool removeSymlink(const std::string & symlinkName, const std::string & symlinkDestinationDirectory) const;
	bool createSymlinks(const GameVersion & gameVersion, bool createTempSymlink = true);
	bool removeSymlinks(const GameVersion & gameVersion);
	size_t deleteFilesWithSuffix(const std::string & suffix, const std::string & path = "");
	size_t renameFilesWithSuffixTo(const std::string & fromSuffix, const std::string & toSuffix, const std::string & path = "");
	static void displayArgumentHelp();
	static void printSpacing(size_t length);

	bool m_initialized;
	bool m_localMode;
	bool m_demoRecordingEnabled;
	std::shared_ptr<ArgumentParser> m_arguments;
	std::shared_ptr<SettingsManager> m_settings;
	std::shared_ptr<HTTPService> m_httpService;
	std::unique_ptr<DownloadManager> m_downloadManager;
	GameType m_gameType;
	std::shared_ptr<GameVersion> m_preferredGameVersion;
	std::shared_ptr<Mod> m_selectedMod;
	size_t m_selectedModVersionIndex;
	size_t m_selectedModVersionTypeIndex;
	std::shared_ptr<GameVersionCollection> m_gameVersions;
	std::unique_ptr<GameManager> m_gameManager;
	std::shared_ptr<ModCollection> m_mods;
	std::shared_ptr<FavouriteModCollection> m_favouriteMods;
	std::shared_ptr<OrganizedModCollection> m_organizedMods;
	std::unique_ptr<CLI> m_cli;

	ModManager(const ModManager &) = delete;
	ModManager(ModManager &&) noexcept = delete;
	const ModManager & operator = (const ModManager &) = delete;
	const ModManager & operator = (ModManager &&) noexcept = delete;
};

#endif // _MOD_MANAGER_H_
