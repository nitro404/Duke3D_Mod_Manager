#ifndef _MOD_MANAGER_H_
#define _MOD_MANAGER_H_

#include "Game/GameType.h"
#include "Mod/OrganizedModCollection.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class ArgumentParser;
class FavouriteModCollection;
class GameVersion;
class GameVersionCollection;
class Mod;
class ModAuthorInformation;
class ModCollection;
class ModGameVersion;
class ModMatch;
class ModVersion;
class ModVersionType;
class ScriptArguments;
class SettingsManager;

class ModManager final {
public:
	ModManager();
	~ModManager();

	bool isInitialized() const;
	bool initialize(int argc, char * argv[], bool start = true);
	bool initialize(const ArgumentParser * args = nullptr, bool start = true);
	bool uninitialize();

	void run();

	std::shared_ptr<SettingsManager> getSettings() const;
	std::shared_ptr<OrganizedModCollection> getOrganizedMods() const;

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

private:
	class CLI {
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
	};

	bool handleArguments(const ArgumentParser * args, bool start);
	std::string generateCommand(std::shared_ptr<ModGameVersion> modGameVersion, std::shared_ptr<GameVersion> gameVersion, ScriptArguments & scriptArgs, bool * customMod = nullptr, std::string * customMap = nullptr) const;
	size_t checkForUnlinkedModFiles() const;
	size_t checkForUnlinkedModFilesForGameVersion(const GameVersion & gameVersion) const;
	size_t checkModForMissingFiles(const std::string & modName, std::optional<size_t> versionIndex = {}, std::optional<size_t> versionTypeIndex = {}) const;
	size_t checkModForMissingFiles(const Mod & mod, std::optional<size_t> versionIndex = {}, std::optional<size_t> versionTypeIndex = {}) const;
	size_t checkAllModsForMissingFiles() const;
	size_t checkForMissingExecutables() const;
	size_t updateAllFileHashes(bool save = true, bool skipHashedFiles = true);
	size_t updateModHashes(Mod & mod, bool skipHashedFiles = true, std::optional<size_t> versionIndex = {}, std::optional<size_t> versionTypeIndex = {});
	bool createSymlink(const std::string & symlinkName, const std::string & symlinkTarget, const std::string & symlinkDestinationDirectory, bool verbose = true) const;
	bool removeSymlink(const std::string & symlinkName, const std::string & symlinkDestinationDirectory, bool verbose = true) const;
	bool createSymlinks(const GameVersion & gameVersion, bool verbose = true);
	bool removeSymlinks(const GameVersion & gameVersion, bool verbose = true);
	size_t deleteFilesWithSuffix(const std::string & suffix, const std::string & path = "", bool verbose = false);
	size_t renameFilesWithSuffixTo(const std::string & fromSuffix, const std::string & toSuffix, const std::string & path = "", bool verbose = false);
	static void displayArgumentHelp();
	static void printSpacing(size_t length);

	bool m_initialized;
	bool m_verbose;
	std::shared_ptr<ArgumentParser> m_arguments;
	std::shared_ptr<SettingsManager> m_settings;
	GameType m_gameType;
	std::shared_ptr<GameVersion> m_preferredGameVersion;
	std::shared_ptr<Mod> m_selectedMod;
	size_t m_selectedModVersionIndex;
	size_t m_selectedModVersionTypeIndex;
	std::shared_ptr<GameVersionCollection> m_gameVersions;
	std::shared_ptr<ModCollection> m_mods;
	std::shared_ptr<FavouriteModCollection> m_favouriteMods;
	std::shared_ptr<OrganizedModCollection> m_organizedMods;
	std::unique_ptr<CLI> m_cli;
};

#endif // _MOD_MANAGER_H_
