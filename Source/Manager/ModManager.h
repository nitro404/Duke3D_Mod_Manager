#ifndef _MOD_MANAGER_H_
#define _MOD_MANAGER_H_

#include "DOSBox/DOSBoxVersionCollection.h"
#include "Game/GameType.h"
#include "Game/GameVersionCollectionListener.h"
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
class ModMatch;
class ModVersion;
class ModVersionType;
class Script;
class ScriptArguments;

class ModManager final : public Application,
						 public OrganizedModCollection::Listener,
						 public DOSBoxVersionCollection::Listener,
						 public GameVersionCollectionListener {
public:
	class Listener {
	public:
		virtual ~Listener();

		virtual void modSelectionChanged(const std::shared_ptr<Mod> & mod, size_t modVersionIndex, size_t modVersionTypeIndex, size_t modGameVersionIndex) = 0;
		virtual void gameTypeChanged(GameType gameType) = 0;
		virtual void preferredDOSBoxVersionChanged(const std::shared_ptr<DOSBoxVersion> & dosboxVersion) = 0;
		virtual void preferredGameVersionChanged(const std::shared_ptr<GameVersion> & gameVersion) = 0;
		virtual void dosboxServerIPAddressChanged(const std::string & ipAddress) = 0;
		virtual void dosboxLocalServerPortChanged(uint16_t port) = 0;
		virtual void dosboxRemoteServerPortChanged(uint16_t port) = 0;
	};

	ModManager();
	virtual ~ModManager();

	bool isInitialized() const;
	bool initialize(int argc = 0, char * argv[] = nullptr);
	bool initialize(std::shared_ptr<ArgumentParser> arguments);
	bool uninitialize();

	bool isUsingLocalMode() const;
	std::shared_ptr<OrganizedModCollection> getOrganizedMods() const;
	std::string getModsListFilePath() const;
	std::string getModsDirectoryPath() const;
	std::string getMapsDirectoryPath() const;

	GameType getGameType() const;
	bool setGameType(const std::string & gameType);
	void setGameType(GameType gameType);

	bool hasPreferredDOSBoxVersion() const;
	std::shared_ptr<DOSBoxVersion> getPreferredDOSBoxVersion() const;
	std::shared_ptr<DOSBoxVersion> getSelectedDOSBoxVersion() const;
	bool setPreferredDOSBoxVersion(const std::string & dosboxVersionName);
	bool setPreferredDOSBoxVersion(std::shared_ptr<DOSBoxVersion> dosboxVersion);

	bool hasPreferredGameVersion() const;
	std::shared_ptr<GameVersion> getPreferredGameVersion() const;
	std::shared_ptr<GameVersion> getSelectedGameVersion() const;
	bool setPreferredGameVersion(const std::string & gameVersionName);
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
	bool setSelectedModByName(const std::string & name);
	bool setSelectedMod(std::shared_ptr<Mod> mod);
	bool setSelectedModFromMatch(const ModMatch & modMatch);
	bool setSelectedModVersionIndex(size_t modVersionIndex);
	bool setSelectedModVersionTypeIndex(size_t modVersionTypeIndex);
	bool setSelectedModGameVersionIndex(size_t modGameVersionIndex);
	bool selectRandomMod(bool selectPreferredVersion, bool selectFirstVersionType);
	bool selectRandomGameVersion();
	bool selectRandomTeam();
	bool selectRandomAuthor();
	static std::vector<ModMatch> searchForMod(const std::vector<std::shared_ptr<Mod>> & mods, const std::string & query);
	size_t searchForAndSelectGameVersion(const std::string & query, std::vector<std::shared_ptr<GameVersion>> * matches = nullptr);
	size_t searchForAndSelectTeam(const std::string & query, std::vector<std::shared_ptr<ModAuthorInformation>> * matches = nullptr);
	size_t searchForAndSelectAuthor(const std::string & query, std::vector<std::shared_ptr<ModAuthorInformation>> * matches = nullptr);
	void clearSelectedMod();

	bool isModSupportedOnSelectedGameVersion();
	bool runSelectedMod(std::shared_ptr<GameVersion> alternateGameVersion = nullptr, std::shared_ptr<ModGameVersion> alternateModGameVersion = nullptr);

	size_t numberOfListeners() const;
	bool hasListener(const Listener & listener) const;
	size_t indexOfListener(const Listener & listener) const;
	Listener * getListener(size_t index) const;
	bool addListener(Listener & listener);
	bool removeListener(size_t index);
	bool removeListener(const Listener & listener);
	void clearListeners();

	static std::string getArgumentHelpInfo();

	// OrganizedModCollection::Listener Virtuals
	virtual void selectedModChanged(const std::shared_ptr<Mod> & mod) override;

	// DOSBoxVersionCollection::Listener Virtuals
	virtual void dosboxVersionCollectionSizeChanged(DOSBoxVersionCollection & dosboxVersionCollection) override;
	virtual void dosboxVersionCollectionItemModified(DOSBoxVersionCollection & dosboxVersionCollection, DOSBoxVersion & dosboxVersion) override;

	// GameVersionCollectionListener Virtuals
	virtual void gameVersionCollectionSizeChanged(GameVersionCollection & gameVersionCollection) override;
	virtual void gameVersionCollectionItemModified(GameVersionCollection & gameVersionCollection, GameVersion & gameVersion) override;

	static const GameType DEFAULT_GAME_TYPE;
	static const std::string DEFAULT_PREFERRED_DOSBOX_VERSION;
	static const std::string DEFAULT_PREFERRED_GAME_VERSION;
	static const std::string HTTP_USER_AGENT;
	static const std::string DEFAULT_BACKUP_FILE_RENAME_SUFFIX;

private:
	void assignPlatformFactories();
	bool handleArguments(const ArgumentParser * args);
	std::string generateCommand(std::shared_ptr<ModGameVersion> modGameVersion, std::shared_ptr<GameVersion> selectedGameVersion, ScriptArguments & scriptArgs, std::string_view combinedGroupFileName = "", bool * customMod = nullptr, std::string * customMap = nullptr, std::shared_ptr<GameVersion> * customTargetGameVersion = nullptr, std::vector<std::string> * customGroupFileNames = nullptr) const;
	std::string generateDOSBoxCommand(const Script & script, const ScriptArguments & arguments, const DOSBoxVersion & dosboxVersion, const  std::string & dosboxArguments) const;
	size_t checkForUnlinkedModFiles() const;
	size_t checkForUnlinkedModFilesForGameVersion(const GameVersion & gameVersion) const;
	size_t checkModForMissingFiles(const std::string & modName, std::optional<size_t> versionIndex = {}, std::optional<size_t> versionTypeIndex = {}) const;
	size_t checkModForMissingFiles(const Mod & mod, std::optional<size_t> versionIndex = {}, std::optional<size_t> versionTypeIndex = {}) const;
	size_t checkAllModsForMissingFiles() const;
	size_t checkForMissingExecutables() const;
	size_t updateAllFileHashes(bool save = true, bool skipHashedFiles = true);
	size_t updateModHashes(Mod & mod, bool skipHashedFiles = true, std::optional<size_t> versionIndex = {}, std::optional<size_t> versionTypeIndex = {});
	static bool areModFilesPresentInGameDirectory(const GameVersion & gameVersion);
	static bool areModFilesPresentInGameDirectory(const std::string & gamePath);
	std::unique_ptr<InstalledModInfo> extractModFilesToGameDirectory(const ModGameVersion & modGameVersion, const GameVersion & selectedGameVersion, const GameVersion & targetGameVersion, const std::vector<std::string> & customGroupFileNames = {});
	bool removeModFilesFromGameDirectory(const GameVersion & gameVersion);
	bool removeModFilesFromGameDirectory(const GameVersion & gameVersion, const InstalledModInfo & installedModInfo);
	bool createApplicationTemporaryDirectory();
	bool createGameTemporaryDirectory(const GameVersion & gameVersion);
	bool removeGameTemporaryDirectory(const GameVersion & gameVersion);
	bool areSymlinkSettingsValid() const;
	bool areSymlinksSupported() const;
	bool createSymlink(const std::string & symlinkName, const std::string & symlinkTarget, const std::string & symlinkDestinationDirectory) const;
	bool removeSymlink(const std::string & symlinkName, const std::string & symlinkDestinationDirectory) const;
	bool createSymlinksOrCopyTemporaryFiles(const GameVersionCollection & gameVersions, const GameVersion & gameVersion, const ModGameVersion * selectedModGameVersion, const std::string & customMap, bool createTempSymlink = true, std::vector<std::string> * temporaryCopiedFilePaths = nullptr);
	bool removeSymlinksOrTemporaryFiles(const GameVersion & gameVersion, const std::vector<std::string> * temporaryCopiedFilePaths = nullptr);
	size_t deleteFilesWithSuffix(const std::string & suffix, const std::string & path = "");
	size_t renameFilesWithSuffixTo(const std::string & fromSuffix, const std::string & toSuffix, const std::string & path = "");
	void notifyModSelectionChanged();
	void notifyGameTypeChanged();
	void notifyPreferredDOSBoxVersionChanged();
	void notifyPreferredGameVersionChanged();
	void notifyDOSBoxServerIPAddressChanged();
	void notifyDOSBoxLocalServerPortChanged();
	void notifyDOSBoxRemoteServerPortChanged();
	static size_t createDOSBoxTemplateScriptFiles(bool overwrite = false);
	static size_t createDOSBoxTemplateScriptFiles(const std::string & directoryPath, bool overwrite = false);
	static bool createDOSBoxTemplateScriptFile(GameType gameType, const std::string & directoryPath, bool overwrite = false);
	static std::string getDOSBoxTemplateScriptFileName(GameType gameType);
	static std::string generateDOSBoxTemplateScriptFileData(GameType gameType);
	static void displayArgumentHelp();

	bool m_initialized;
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
	std::shared_ptr<GameVersionCollection> m_gameVersions;
	std::shared_ptr<GameManager> m_gameManager;
	std::shared_ptr<ModCollection> m_mods;
	std::shared_ptr<FavouriteModCollection> m_favouriteMods;
	std::shared_ptr<OrganizedModCollection> m_organizedMods;
	std::vector<Listener *> m_listeners;

	ModManager(const ModManager &) = delete;
	ModManager(ModManager &&) noexcept = delete;
	const ModManager & operator = (const ModManager &) = delete;
	const ModManager & operator = (ModManager &&) noexcept = delete;
};

#endif // _MOD_MANAGER_H_
