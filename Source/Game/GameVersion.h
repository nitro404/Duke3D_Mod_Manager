#ifndef _GAME_VERSION_H_
#define _GAME_VERSION_H_

#include "DOSBox/Configuration/DOSBoxConfiguration.h"

#include <Platform/DeviceInformationBridge.h>

#include <boost/signals2.hpp>
#include <rapidjson/document.h>

#include <any>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class ModGameVersion;

class GameVersion {
public:
	enum class OperatingSystem {
		Windows,
		Linux,
		MacOS,
		DOS
	};

	GameVersion();
	GameVersion(const std::string & id, const std::string & longName, const std::string & shortName, bool removable, const std::string & gamePath, const std::string & gameExecutableName, const std::string & gameConfigurationFileName, const std::string & gameConfigurationDirectoryPath, bool localWorkingDirectory, std::optional<bool> scriptFilesReadFromGroup, bool supportsSubdirectories, std::optional<bool> worldTourGroupSupported, std::optional<bool> zipArchiveGroupsSupported, std::optional<uint16_t> maximumGroupFiles, const std::string & modDirectoryName, const std::optional<std::string> & conFileArgumentFlag, const std::optional<std::string> & extraConFileArgumentFlag, const std::optional<std::string> & groupFileArgumentFlag, const std::optional<std::string> & mapFileArgumentFlag, const std::string & episodeArgumentFlag, const std::string & levelArgumentFlag, const std::string & skillArgumentFlag, uint8_t skillStartValue, const std::string & recordDemoArgumentFlag, const std::optional<std::string> & playDemoArgumentFlag, const std::optional<std::string> & respawnModeArgumentFlag = {}, const std::optional<std::string> & weaponSwitchOrderArgumentFlag = {}, const std::optional<std::string> & disableMonstersArgumentFlag = {}, const std::optional<std::string> & disableSoundArgumentFlag = {}, const std::optional<std::string> & disableMusicArgumentFlag = {}, const std::optional<std::string> & setupExecutableName = {}, const std::optional<std::string> & groupFileInstallPath = {}, const std::optional<std::string> & defFileArgumentFlag = {}, const std::optional<std::string> & extraDefFileArgumentFlag = {}, const std::optional<bool> & requiresCombinedGroup = {}, const std::optional<bool> & requiresGroupFileExtraction = {}, const std::string & website = {}, const std::string & sourceCodeURL = {}, const std::vector<OperatingSystem> & supportedOperatingSystems = {}, const std::vector<std::string> & compatibleGameVersions = {}, const std::vector<std::string> & notes = {}, const DOSBoxConfiguration & dosboxConfiguration = {});
	GameVersion(GameVersion && gameVersion) noexcept;
	GameVersion(const GameVersion & gameVersion);
	GameVersion & operator = (GameVersion && gameVersion) noexcept;
	GameVersion & operator = (const GameVersion & gameVersion);
	virtual ~GameVersion();

	bool isModified() const;
	bool hasID() const;
	const std::string & getID() const;
	bool setID(const std::string & id);
	bool hasLongName() const;
	const std::string & getLongName() const;
	bool setLongName(const std::string & shortName);
	bool hasShortName() const;
	const std::string & getShortName() const;
	bool setShortName(const std::string & longName);
	bool hasInstalledTimePoint() const;
	const std::optional<std::chrono::time_point<std::chrono::system_clock>> & getInstalledTimePoint() const;
	void setInstalledTimePoint(std::chrono::time_point<std::chrono::system_clock> installedTimePoint);
	void clearInstalledTimePoint();
	bool hasLastPlayedTimePoint() const;
	const std::optional<std::chrono::time_point<std::chrono::system_clock>> & getLastPlayedTimePoint() const;
	void setLastPlayedTimePoint(std::chrono::time_point<std::chrono::system_clock> lastPlayedTimePoint);
	void updateLastPlayedTimePoint();
	void clearLastPlayedTimePoint();
	bool isStandAlone() const;
	void setStandAlone(bool standAlone);
	std::string getBase() const;
	void setBase(const std::string & base);
	bool isRemovable() const;
	bool hasGroupFile() const;
	bool hasGamePath() const;
	const std::string & getGamePath() const;
	void setGamePath(const std::string & gamePath);
	const std::string & getGameExecutableName() const;
	void setGameExecutableName(const std::string & gameExecutableName);
	bool hasSetupExecutableName() const;
	const std::optional<std::string> & getSetupExecutableName() const;
	void setSetupExecutableName(const std::string & setupExecutableName);
	void clearSetupExecutableName();
	const std::string & getGameConfigurationFileName() const;
	bool setGameConfigurationFileName(const std::string & fileName);
	const std::string & getGameConfigurationDirectoryPath() const;
	std::string getEvaluatedGameConfigurationDirectoryPath() const;
	std::string getEvaluatedGameConfigurationFilePath() const;
	bool setGameConfigurationDirectoryPath(const std::string & directoryPath);
	bool hasLaunchArguments() const;
	const std::string & getLaunchArguments() const;
	void setLaunchArguments(const std::string & arguments);
	void clearLaunchArguments();
	bool hasGroupFileInstallPath() const;
	const std::optional<std::string> & getGroupFileInstallPath() const;
	void setGroupFileInstallPath(const std::string & setupExecutableName);
	void clearGroupFileInstallPath();
	bool hasGroupFileMaximum() const;
	const std::optional<uint16_t> & getMaximumGroupFiles() const;
	void setMaximumGroupFiles(uint16_t maximumGroupFiles);
	void clearMaximumGroupFiles();
	bool doesRequireCombinedGroup() const;
	const std::optional<bool> & getRequiresCombinedGroup() const;
	void setRequiresCombinedGroup(bool requiresCombinedGroup);
	void clearRequiresCombinedGroup();
	bool doesRequireGroupFileExtraction() const;
	const std::optional<bool> & getRequiresGroupFileExtraction() const;
	void setRequiresGroupFileExtraction(bool requiresGroupFileExtraction);
	void clearRequiresGroupFileExtraction();
	bool doesRequireDOSBox() const;
	bool hasLocalWorkingDirectory() const;
	void setLocalWorkingDirectory(bool localWorkingDirectory);
	const std::string & getModDirectoryName() const;
	void setModDirectoryName(const std::string & modDirectoryName);
	bool areScriptFilesReadFromGroup() const;
	std::optional<bool> getScriptFilesReadFromGroup() const;
	void setScriptFilesReadFromGroup(bool scriptFilesReadFromGroup);
	void clearScriptFilesReadFromGroup();
	bool doesSupportSubdirectories() const;
	void setSupportsSubdirectories(bool supportsSubdirectories);
	bool isWorldTourGroupSupported() const;
	std::optional<bool> getWorldTourGroupSupported() const;
	void setWorldTourGroupSupported(bool worldTourGroupSupported);
	void clearWorldTourGroupSupported();
	bool areZipArchiveGroupsSupported() const;
	std::optional<bool> getZipArchiveGroupsSupported() const;
	void setZipArchiveGroupsSupported(bool zipArchiveGroupsSupported);
	void clearZipArchiveGroupsSupported();
	bool hasConFileArgumentFlag() const;
	const std::optional<std::string> & getConFileArgumentFlag() const;
	bool setConFileArgumentFlag(const std::string & flag);
	void clearConFileArgumentFlag();
	bool hasExtraConFileArgumentFlag() const;
	const std::optional<std::string> & getExtraConFileArgumentFlag() const;
	bool setExtraConFileArgumentFlag(const std::string & flag);
	void clearExtraConFileArgumentFlag();
	bool hasGroupFileArgumentFlag() const;
	const std::optional<std::string> & getGroupFileArgumentFlag() const;
	bool setGroupFileArgumentFlag(const std::string & flag);
	void clearGroupFileArgumentFlag();
	bool hasDefFileArgumentFlag() const;
	const std::optional<std::string> & getDefFileArgumentFlag() const;
	bool setDefFileArgumentFlag(const std::string & flag);
	void clearDefFileArgumentFlag();
	bool hasExtraDefFileArgumentFlag() const;
	const std::optional<std::string> & getExtraDefFileArgumentFlag() const;
	bool setExtraDefFileArgumentFlag(const std::string & flag);
	void clearExtraDefFileArgumentFlag();
	bool hasMapFileArgumentFlag() const;
	const std::optional<std::string> & getMapFileArgumentFlag() const;
	bool setMapFileArgumentFlag(const std::string & flag);
	void clearMapFileArgumentFlag();
	const std::string & getEpisodeArgumentFlag() const;
	bool setEpisodeArgumentFlag(const std::string & flag);
	const std::string & getLevelArgumentFlag() const;
	bool setLevelArgumentFlag(const std::string & flag);
	const std::string & getSkillArgumentFlag() const;
	bool setSkillArgumentFlag(const std::string & flag);
	uint8_t getSkillStartValue() const;
	void setSkillStartValue(uint8_t skillStartValue);
	const std::string & getRecordDemoArgumentFlag() const;
	bool setRecordDemoArgumentFlag(const std::string & flag);
	bool hasPlayDemoArgumentFlag() const;
	const std::optional<std::string> & getPlayDemoArgumentFlag() const;
	bool setPlayDemoArgumentFlag(const std::string & flag);
	void clearPlayDemoArgumentFlag();
	bool hasRespawnModeArgumentFlag() const;
	const std::optional<std::string> & getRespawnModeArgumentFlag() const;
	bool setRespawnModeArgumentFlag(const std::string & flag);
	void clearRespawnModeArgumentFlag();
	bool hasWeaponSwitchOrderArgumentFlag() const;
	const std::optional<std::string> & getWeaponSwitchOrderArgumentFlag() const;
	bool setWeaponSwitchOrderArgumentFlag(const std::string & flag);
	void clearWeaponSwitchOrderArgumentFlag();
	bool hasDisableMonstersArgumentFlag() const;
	const std::optional<std::string> & getDisableMonstersArgumentFlag() const;
	bool setDisableMonstersArgumentFlag(const std::string & flag);
	void clearDisableMonstersArgumentFlag();
	bool hasDisableSoundArgumentFlag() const;
	const std::optional<std::string> & getDisableSoundArgumentFlag() const;
	bool setDisableSoundArgumentFlag(const std::string & flag);
	void clearDisableSoundArgumentFlag();
	bool hasDisableMusicArgumentFlag() const;
	const std::optional<std::string> & getDisableMusicArgumentFlag() const;
	bool setDisableMusicArgumentFlag(const std::string & flag);
	void clearDisableMusicArgumentFlag();
	const std::string & getWebsite() const;
	void setWebsite(const std::string & website);
	const std::string & getSourceCodeURL() const;
	void setSourceCodeURL(const std::string & sourceCodeURL);
	std::shared_ptr<DOSBoxConfiguration> getDOSBoxConfiguration() const;
	bool setDOSBoxConfiguration(const DOSBoxConfiguration & dosboxConfiguration);
	void resetDOSBoxConfigurationToDefault();
	std::shared_ptr<const DOSBoxConfiguration> getDefaultDOSBoxConfiguration() const;

	size_t numberOfSupportedOperatingSystems() const;
	bool hasSupportedOperatingSystem(OperatingSystem operatingSystem) const;
	bool hasSupportedOperatingSystemType(DeviceInformationBridge::OperatingSystemType operatingSystemType) const;
	bool hasSupportedOperatingSystemWithName(const std::string & operatingSystemName) const;
	size_t indexOfSupportedOperatingSystem(OperatingSystem operatingSystem) const;
	size_t indexOfSupportedOperatingSystemType(DeviceInformationBridge::OperatingSystemType operatingSystemType) const;
	size_t indexOfSupportedOperatingSystemWithName(const std::string & operatingSystemName) const;
	std::optional<OperatingSystem> getSupportedOperatingSystem(size_t index) const;
	std::string getSupportedOperatingSystemName(size_t index) const;
	const std::vector<OperatingSystem> & getSupportedOperatingSystems() const;
	bool addSupportedOperatingSystem(OperatingSystem operatingSystem);
	bool addSupportedOperatingSystemWithName(const std::string & operatingSystemName);
	size_t addSupportedOperatingSystems(const std::vector<OperatingSystem> & operatingSystems);
	bool removeSupportedOperatingSystem(size_t index);
	bool removeSupportedOperatingSystem(OperatingSystem operatingSystem);
	bool removeSupportedOperatingSystemWithName(const std::string & operatingSystemName);
	void clearSupportedOperatingSystems();

	size_t numberOfCompatibleGameVersions() const;
	bool hasCompatibleGameVersion(const GameVersion & gameVersion) const;
	bool hasCompatibleGameVersionWithID(const std::string & gameVersionID) const;
	size_t indexOfCompatibleGameVersion(const GameVersion & gameVersion) const;
	size_t indexOfCompatibleGameVersionWithID(const std::string & gameVersionID) const;
	std::optional<std::string> getCompatibleGameVersionID(size_t index) const;
	const std::vector<std::string> & getCompatibleGameVersionIdentifiers() const;
	std::shared_ptr<ModGameVersion> getMostCompatibleModGameVersion(const std::vector<std::shared_ptr<ModGameVersion>> & modGameVersions) const;
	std::vector<std::shared_ptr<ModGameVersion>> getCompatibleModGameVersions(const std::vector<std::shared_ptr<ModGameVersion>> & modGameVersions) const;
	bool addCompatibleGameVersion(const GameVersion & gameVersion);
	bool addCompatibleGameVersionWithID(const std::string & gameVersionID);
	size_t addCompatibleGameVersionIdentifiers(const std::vector<std::string> & gameVersionIdentifiers);
	bool removeCompatibleGameVersion(size_t index);
	bool removeCompatibleGameVersion(const GameVersion & gameVersion);
	bool removeCompatibleGameVersionWithID(const std::string & gameVersionID);
	void clearCompatibleGameVersions();

	size_t numberOfNotes() const;
	bool hasNote(const std::string & note) const;
	size_t indexOfNote(const std::string & note) const;
	std::string getNote(size_t index) const;
	const std::vector<std::string> & getNotes() const;
	std::string getNotesAsString() const;
	bool addNote(const std::string & note);
	void setNotes(const std::string & notes);
	bool removeNote(size_t index);
	bool removeNote(const std::string & note);
	void clearNotes();

	void addMetadata(std::map<std::string, std::any> & metadata) const;
	std::unique_ptr<GameVersion> createTemplateFrom() const;
	size_t checkForMissingExecutables() const;

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<GameVersion> parseFrom(const rapidjson::Value & gameVersionValue);

	std::string getDOSBoxConfigurationFileName() const;
	bool loadDOSBoxConfigurationFrom(const std::string & directoryPath, bool * loaded = nullptr);
	bool saveDOSBoxConfigurationTo(const std::string & directoryPath, bool * saved = nullptr) const;

	static std::optional<OperatingSystem> convertOperatingSystemType(DeviceInformationBridge::OperatingSystemType operatingSystemType);

	bool isConfigured() const;
	static bool isConfigured(const GameVersion * gameVersion);
	virtual bool isValid() const;
	static bool isValid(const GameVersion * gameVersion);

	bool operator == (const GameVersion & gameVersion) const;
	bool operator != (const GameVersion & gameVersion) const;

	boost::signals2::signal<void (GameVersion & /* gameVersion */)> modified;

	static const std::string ALL_VERSIONS;
	static const std::string STANDALONE;
	static const std::string STANDALONE_DIRECTORY_NAME;
	static const GameVersion LAMEDUKE;
	static const GameVersion ORIGINAL_BETA_VERSION;
	static const GameVersion ORIGINAL_REGULAR_VERSION;
	static const GameVersion ORIGINAL_PLUTONIUM_PAK;
	static const GameVersion ORIGINAL_ATOMIC_EDITION;
	static const GameVersion JFDUKE3D;
	static const GameVersion EDUKE32;
	static const GameVersion NETDUKE32;
	//static const GameVersion MEGATON_EDITION;
	//static const GameVersion WORLD_TOUR;
	//static const GameVersion BUILDGDX;
	static const GameVersion RAZE;
	static const GameVersion RED_NUKEM;
	//static const GameVersion CHOCOLATE_DUKE3D;
	static const GameVersion BELGIAN_CHOCOLATE_DUKE3D;
	static const GameVersion DUKE3DW;
	static const GameVersion PKDUKE3D;
	static const GameVersion XDUKE;
	static const GameVersion RDUKE;
	static const GameVersion DUKE3D_W32;
	static const std::vector<const GameVersion *> DEFAULT_GAME_VERSIONS;

	// Note: These SHA1 hashes are defined inline in the header file so that they are available at compile time in other classes
	static inline const std::string BETA_VERSION_GAME_EXECUTABLE_SHA1 = "69b0efe8963d2039240a257662dd1cac0748bc77";
	static inline const std::string REGULAR_VERSION_GAME_EXECUTABLE_SHA1 = "a64cc5b61cba728427cfcc537aa2f74438ea4c65";
	static inline const std::string PLUTONIUM_PAK_GAME_EXECTUABLE_UNCRACKED_SHA1 = "772d922b16f7b0b11305f1aa7a3fcbb534f884d1";
	static inline const std::string PLUTONIUM_PAK_GAME_EXECTUABLE_CRACKED_SHA1 = "b440dd343df1ce318a03991a056db95a43d5d30c";
	static inline const std::string ATOMIC_EDITION_GAME_EXECTUABLE_UNCRACKED_SHA1 = "f0dc7f1ca810aa517fcad544a3bf5af623a3e44e";
	static inline const std::string ATOMIC_EDITION_GAME_EXECTUABLE_CRACKED_SHA1 = "a849e1e00ac58c0271498dd302d5c5f2819ab2e9";

	static const bool DEFAULT_LOCAL_WORKING_DIRECTORY;
	static const bool DEFAULT_SUPPORTS_SUBDIRECTORIES;
	static const bool DEFAULT_WORLD_TOUR_GROUP_SUPPORTED;
	static const uint8_t DEFAULT_SKILL_START_VALUE;
	static const uint16_t DEFAULT_MAXIMUM_GROUP_FILES;

private:
	void setModified(bool modified);

	std::string m_id;
	std::string m_longName;
	std::string m_shortName;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> m_installedTimePoint;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> m_lastPlayedTimePoint;
	bool m_standAlone;
	std::string m_base;
	bool m_removable;
	std::string m_gamePath;
	std::string m_gameExecutableName;
	std::optional<std::string> m_setupExecutableName;
	std::string m_gameConfigurationFileName;
	std::string m_gameConfigurationDirectoryPath;
	std::string m_launchArguments;
	std::optional<std::string> m_groupFileInstallPath;
	std::optional<uint16_t> m_maximumGroupFiles;
	std::optional<bool> m_requiresCombinedGroup;
	std::optional<bool> m_requiresGroupFileExtraction;
	std::string m_modDirectoryName;
	std::string m_website;
	std::string m_sourceCodeURL;
	bool m_localWorkingDirectory;
	std::optional<bool> m_scriptFilesReadFromGroup;
	bool m_supportsSubdirectories;
	std::optional<bool> m_worldTourGroupSupported;
	std::optional<bool> m_zipArchiveGroupsSupported;
	std::optional<std::string> m_conFileArgumentFlag;
	std::optional<std::string> m_extraConFileArgumentFlag;
	std::optional<std::string> m_groupFileArgumentFlag;
	std::optional<std::string> m_defFileArgumentFlag;
	std::optional<std::string> m_extraDefFileArgumentFlag;
	std::optional<std::string> m_mapFileArgumentFlag;
	std::string m_episodeArgumentFlag;
	std::string m_levelArgumentFlag;
	std::string m_skillArgumentFlag;
	int8_t m_skillStartValue;
	std::string m_recordDemoArgumentFlag;
	std::optional<std::string> m_playDemoArgumentFlag;
	std::optional<std::string> m_respawnModeArgumentFlag;
	std::optional<std::string> m_weaponSwitchOrderArgumentFlag;
	std::optional<std::string> m_disableMonstersArgumentFlag;
	std::optional<std::string> m_disableSoundArgumentFlag;
	std::optional<std::string> m_disableMusicArgumentFlag;
	std::vector<OperatingSystem> m_supportedOperatingSystems;
	std::vector<std::string> m_compatibleGameVersionIdentifiers;
	std::vector<std::string> m_notes;
	std::shared_ptr<DOSBoxConfiguration> m_dosboxConfiguration;
	mutable bool m_modified;
};

#endif // _GAME_VERSION_H_
