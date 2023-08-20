#ifndef _MOD_MANAGER_H_
#define _MOD_MANAGER_H_

#include "DOSBox/DOSBoxVersionCollection.h"
#include "Game/GameType.h"

#include <Analytics/Segment/SegmentAnalytics.h>
#include <Application/Application.h>

#include <boost/signals2.hpp>

#include <cstdint>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class ArgumentParser;
class DOSBoxConfiguration;
class DOSBoxManager;
class DOSBoxVersion;
class DOSBoxVersionCollection;
class DownloadManager;
class FavouriteModCollection;
class GameManager;
class GameVersion;
class GameVersionCollection;
class InstalledModInfo;
class Mod;
class ModAuthorInformation;
class ModCollection;
class ModGameVersion;
class ModIdentifier;
class ModMatch;
class ModVersion;
class ModVersionType;
class OrganizedModCollection;
class Process;
class Script;
class ScriptArguments;
class StandAloneMod;
class StandAloneModCollection;

class ModManager final : public Application {
public:
	ModManager();
	virtual ~ModManager();

	bool isInitialized() const;
	bool isInitializing() const;
	uint8_t numberOfInitializationSteps() const;
	std::future<bool> initializeAsync(int argc = 0, char * argv[] = nullptr);
	std::future<bool> initializeAsync(std::shared_ptr<ArgumentParser> arguments);
	bool uninitialize();

	bool isUsingLocalMode() const;
	std::shared_ptr<ModCollection> getMods() const;
	std::shared_ptr<StandAloneModCollection> getStandAloneMods() const;
	std::shared_ptr<FavouriteModCollection> getFavouriteMods() const;
	std::shared_ptr<OrganizedModCollection> getOrganizedMods() const;
	std::shared_ptr<DOSBoxConfiguration> getGeneralDOSBoxConfiguration() const;
	std::string getModsListFilePath() const;
	std::string getModsDirectoryPath() const;
	std::string getMapsDirectoryPath() const;
	std::string getGeneralDOSBoxConfigurationFilePath() const;
	std::string getDOSBoxConfigurationsDirectoryPath() const;

	GameType getGameType() const;
	bool setGameType(const std::string & gameType);
	void setGameType(GameType gameType);

	bool hasPreferredDOSBoxVersion() const;
	std::shared_ptr<DOSBoxVersion> getPreferredDOSBoxVersion() const;
	std::shared_ptr<DOSBoxVersion> getSelectedDOSBoxVersion() const;
	bool setPreferredDOSBoxVersionByID(const std::string & dosboxVersionID);
	bool setPreferredDOSBoxVersion(std::shared_ptr<DOSBoxVersion> dosboxVersion);

	bool hasPreferredGameVersion() const;
	std::shared_ptr<GameVersion> getPreferredGameVersion() const;
	std::shared_ptr<GameVersion> getSelectedGameVersion() const;
	bool setPreferredGameVersionByID(const std::string & gameVersionID);
	bool setPreferredGameVersion(std::shared_ptr<GameVersion> gameVersion);

	std::shared_ptr<DOSBoxManager> getDOSBoxManager() const;
	std::shared_ptr<DOSBoxVersionCollection> getDOSBoxVersions() const;
	std::shared_ptr<GameVersionCollection> getGameVersions() const;
	std::shared_ptr<GameManager> getGameManager() const;
	std::shared_ptr<DownloadManager> getDownloadManager() const;

	const std::string & getDOSBoxServerIPAddress() const;
	void setDOSBoxServerIPAddress(const std::string & ipAddress);
	uint16_t getDOSBoxLocalServerPort() const;
	void setDOSBoxLocalServerPort(uint16_t port);
	uint16_t getDOSBoxRemoteServerPort() const;
	void setDOSBoxRemoteServerPort(uint16_t port);

	bool hasModSelected() const;
	bool hasModVersionSelected() const;
	bool hasModVersionTypeSelected() const;
	bool hasModGameVersionSelected() const;
	std::shared_ptr<Mod> getSelectedMod() const;
	std::shared_ptr<ModVersion> getSelectedModVersion() const;
	std::shared_ptr<ModVersionType> getSelectedModVersionType() const;
	std::shared_ptr<ModGameVersion> getSelectedModGameVersion() const;
	std::optional<std::string> getSelectedModName() const;
	size_t getSelectedModVersionIndex() const;
	size_t getSelectedModVersionTypeIndex() const;
	size_t getSelectedModGameVersionIndex() const;
	bool setSelectedModByID(const std::string & id);
	bool setSelectedMod(std::shared_ptr<Mod> mod);
	bool setSelectedModFromMatch(const ModMatch & modMatch);
	bool setSelectedMod(const ModIdentifier & modIdentifier);
	bool setSelectedModVersionIndex(size_t modVersionIndex);
	bool setSelectedModVersionTypeIndex(size_t modVersionTypeIndex);
	bool setSelectedModGameVersionIndex(size_t modGameVersionIndex);
	bool selectRandomMod(bool selectPreferredVersion, bool selectFirstVersionType);
	bool selectRandomGameVersion();
	bool selectRandomTeam();
	bool selectRandomAuthor();
	static std::vector<ModMatch> searchForMod(const std::vector<std::shared_ptr<Mod>> & mods, const std::string & query, bool autoPopulateVersion = false, bool autoPopulateVersionType = false);
	static std::vector<std::shared_ptr<ModIdentifier>> searchForFavouriteMod(const std::vector<std::shared_ptr<ModIdentifier>> & favouriteMods, const std::string & query);
	static std::vector<std::shared_ptr<GameVersion>> searchForGameVersion(const std::vector<std::shared_ptr<GameVersion>> & gameVersions, const std::string & query);
	static std::vector<std::shared_ptr<ModAuthorInformation>> searchForAuthor(const std::vector<std::shared_ptr<ModAuthorInformation>> & authors, const std::string & query);
	void clearSelectedMod();

	bool isModSupportedOnSelectedGameVersion();
	bool installStandAloneMod(const ModGameVersion & standAloneModGameVersion, const std::string & destinationDirectoryPath, bool removeArchivePackage = true);
	bool uninstallModGameVersion(const ModGameVersion & modGameVersion);
	std::future<bool> runSelectedModAsync(std::shared_ptr<GameVersion> alternateGameVersion = nullptr, std::shared_ptr<ModGameVersion> alternateModGameVersion = nullptr);
	bool isGameProcessRunning() const;
	std::shared_ptr<Process> getGameProcess() const;
	bool terminateGameProcess();

	static std::string getArgumentHelpInfo();

	boost::signals2::signal<void ()> initialized;
	boost::signals2::signal<void ()> initializationCancelled;
	boost::signals2::signal<void ()> initializationFailed;
	boost::signals2::signal<bool (uint8_t /* initializationStep */, uint8_t /* initializationStepCount */, std::string /* description */)> initializationProgress;
	boost::signals2::signal<void ()> launched;
	boost::signals2::signal<void (std::string)> launchError;
	boost::signals2::signal<void (uint64_t /* nativeExitCode */, bool /* forceTerminated */)> gameProcessTerminated;
	boost::signals2::signal<void (std::shared_ptr<Mod> /* mod */, size_t /* modVersionIndex */, size_t /* modVersionTypeIndex */, size_t /* modGameVersionIndex */)> modSelectionChanged;
	boost::signals2::signal<void (GameType /* gameType */)> gameTypeChanged;
	boost::signals2::signal<void (std::shared_ptr<DOSBoxVersion> /* dosboxVersion */)> preferredDOSBoxVersionChanged;
	boost::signals2::signal<void (std::shared_ptr<GameVersion> /* gameVersion */)> preferredGameVersionChanged;
	boost::signals2::signal<void (std::string /* ipAddress */)> dosboxServerIPAddressChanged;
	boost::signals2::signal<void (uint16_t /* port */)> dosboxLocalServerPortChanged;
	boost::signals2::signal<void (uint16_t /* port */)> dosboxRemoteServerPortChanged;

	static const GameType DEFAULT_GAME_TYPE;
	static const std::string DEFAULT_PREFERRED_DOSBOX_VERSION_ID;
	static const std::string DEFAULT_PREFERRED_GAME_VERSION_ID;
	static const std::string HTTP_USER_AGENT;
	static const std::string DEFAULT_BACKUP_FILE_RENAME_SUFFIX;
	static const std::string GENERAL_DOSBOX_CONFIGURATION_FILE_NAME;
	static const DOSBoxConfiguration DEFAULT_GENERAL_DOSBOX_CONFIGURATION;

private:
	bool initialize(std::shared_ptr<ArgumentParser> arguments);
	bool runSelectedMod(std::shared_ptr<GameVersion> alternateGameVersion = nullptr, std::shared_ptr<ModGameVersion> alternateModGameVersion = nullptr);
	void notifyLaunchError(const std::string & errorMessage);
	bool notifyInitializationProgress(const std::string & description);
	void notifyModSelectionChanged();
	void assignPlatformFactories();
	bool handleArguments(const ArgumentParser * args);
	std::string generateCommand(std::shared_ptr<ModGameVersion> modGameVersion, std::shared_ptr<GameVersion> selectedGameVersion, ScriptArguments & scriptArgs, std::string_view combinedGroupFileName = "", std::string_view combinedDOSBoxConfigurationFilePath = "", bool * customMod = nullptr, std::string * customMap = nullptr, std::shared_ptr<GameVersion> * customTargetGameVersion = nullptr, std::vector<std::string> * customGroupFileNames = nullptr) const;
	std::string generateDOSBoxCommand(const Script & script, const ScriptArguments & arguments, const DOSBoxVersion & dosboxVersion, const  std::string & dosboxArguments, bool showConsole, bool fullscreen, std::string_view combinedDOSBoxConfigurationFilePath = "") const;
	size_t checkForUnlinkedModFiles() const;
	size_t checkForUnlinkedModFilesForGameVersion(const GameVersion & gameVersion) const;
	size_t checkModForMissingFiles(const std::string & modID, std::optional<size_t> versionIndex = {}, std::optional<size_t> versionTypeIndex = {}) const;
	size_t checkModForMissingFiles(const Mod & mod, std::optional<size_t> versionIndex = {}, std::optional<size_t> versionTypeIndex = {}) const;
	size_t checkAllModsForMissingFiles() const;
	size_t checkForMissingExecutables() const;
	size_t updateFileInfoForAllMods(bool save = true, bool skipPopulatedFiles = true);
	size_t updateModFileInfo(Mod & mod, bool skipPopulatedFiles = true, std::optional<size_t> versionIndex = {}, std::optional<size_t> versionTypeIndex = {});
	static bool testParsing();
	static bool areModFilesPresentInGameDirectory(const GameVersion & gameVersion);
	static bool areModFilesPresentInGameDirectory(const std::string & gamePath);
	bool extractModFilesToGameDirectory(const ModGameVersion & modGameVersion, const GameVersion & selectedGameVersion, const GameVersion & targetGameVersion, InstalledModInfo * installedModInfo = nullptr, const std::vector<std::string> & customGroupFileNames = {});
	bool removeModFilesFromGameDirectory(const GameVersion & gameVersion);
	bool removeModFilesFromGameDirectory(const GameVersion & gameVersion, const InstalledModInfo & installedModInfo);
	bool createApplicationTemporaryDirectory();
	void clearApplicationTemporaryDirectory();
	bool createGameTemporaryDirectory(const GameVersion & gameVersion);
	bool removeGameTemporaryDirectory(const GameVersion & gameVersion);
	bool areSymlinkSettingsValid() const;
	static bool createSymlink(const std::string & symlinkName, const std::string & symlinkTarget, const std::string & symlinkDestinationDirectory);
	static bool removeSymlink(const std::string & symlinkName, const std::string & symlinkDestinationDirectory);
	bool createSymlinksOrCopyTemporaryFiles(const GameVersionCollection & gameVersions, const GameVersion & gameVersion, const ModGameVersion * selectedModGameVersion, const std::string & customMap, bool createTempSymlink = true, InstalledModInfo * installedModInfo = nullptr);
	bool removeSymlinks(const GameVersion & gameVersion);
	std::vector<std::string> deleteFilesWithSuffix(const std::string & suffix, const std::string & path = "");
	std::vector<std::string> renameFilesWithSuffixTo(const std::string & fromSuffix, const std::string & toSuffix, const std::string & path = "");
	void onSelectedModChanged(std::shared_ptr<Mod> mod);
	void onSelectedFavouriteModChanged(std::shared_ptr<ModIdentifier> favouriteMod);
	void onDOSBoxVersionCollectionSizeChanged(DOSBoxVersionCollection & dosboxVersionCollection);
	void onDOSBoxVersionCollectionItemModified(DOSBoxVersionCollection & dosboxVersionCollection, DOSBoxVersion & dosboxVersion);
	void onGameVersionCollectionSizeChanged(GameVersionCollection & gameVersionCollection);
	void onGameVersionCollectionItemModified(GameVersionCollection & gameVersionCollection, GameVersion & gameVersion);
	static size_t createDOSBoxTemplateCommandScriptFiles(bool overwrite = false);
	static size_t createDOSBoxTemplateCommandScriptFiles(const std::string & directoryPath, bool overwrite = false);
	static bool createDOSBoxTemplateCommandScriptFile(GameType gameType, const std::string & directoryPath, bool overwrite = false);
	static std::string getDOSBoxCommandScriptFileName(GameType gameType);
	static std::string getDOSBoxCommandScriptFilePath(GameType gameType);
	static std::string generateDOSBoxTemplateCommandScriptFileData(GameType gameType);
	static void displayArgumentHelp();

	std::atomic<bool> m_initialized;
	std::atomic<bool> m_initializing;
	bool m_localMode;
	bool m_demoRecordingEnabled;
	std::shared_ptr<ArgumentParser> m_arguments;
	std::shared_ptr<DownloadManager> m_downloadManager;
	GameType m_gameType;
	std::shared_ptr<DOSBoxVersion> m_preferredDOSBoxVersion;
	std::shared_ptr<GameVersion> m_preferredGameVersion;
	std::shared_ptr<Mod> m_selectedMod;
	size_t m_selectedModVersionIndex;
	size_t m_selectedModVersionTypeIndex;
	size_t m_selectedModGameVersionIndex;
	std::shared_ptr<DOSBoxManager> m_dosboxManager;
	boost::signals2::connection m_dosboxVersionCollectionSizeChangedConnection;
	boost::signals2::connection m_dosboxVersionCollectionItemModifiedConnection;
	std::shared_ptr<GameManager> m_gameManager;
	boost::signals2::connection m_gameVersionCollectionSizeChangedConnection;
	boost::signals2::connection m_gameVersionCollectionItemModifiedConnection;
	std::shared_ptr<DOSBoxConfiguration> m_generalDOSBoxConfiguration;
	std::shared_ptr<ModCollection> m_mods;
	std::shared_ptr<StandAloneModCollection> m_standAloneMods;
	std::shared_ptr<FavouriteModCollection> m_favouriteMods;
	std::shared_ptr<OrganizedModCollection> m_organizedMods;
	boost::signals2::connection m_selectedModChangedConnection;
	boost::signals2::connection m_selectedFavouriteModChangedConnection;
	std::shared_ptr<Process> m_gameProcess;
	uint8_t m_initializationStep;
	mutable std::recursive_mutex m_mutex;

	ModManager(const ModManager &) = delete;
	const ModManager & operator = (const ModManager &) = delete;
};

#endif // _MOD_MANAGER_H_
