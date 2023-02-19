#ifndef _GAME_VERSION_H_
#define _GAME_VERSION_H_

#include <Platform/DeviceInformationBridge.h>

#include <rapidjson/document.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

class ModGameVersion;

class GameVersion final {
public:
	enum class OperatingSystem {
		Windows,
		Linux,
		MacOS,
		DOS
	};

	class Listener {
	public:
		virtual ~Listener();

		virtual void gameVersionModified(GameVersion & gameVersion) = 0;
	};

	GameVersion();
	GameVersion(const std::string & name, bool removable, bool renamable, const std::string & gamePath, const std::string & gameExecutableName, bool localWorkingDirectory, bool relativeConFilePath, bool supportsSubdirectories, const std::string & modDirectoryName, const std::optional<std::string> & conFileArgumentFlag, const std::optional<std::string> & groupFileArgumentFlag, const std::string & mapFileArgumentFlag, const std::string & episodeArgumentFlag, const std::string & levelArgumentFlag, const std::string & skillArgumentFlag, uint8_t skillStartValue, const std::string & recordDemoArgumentFlag, const std::optional<std::string> & playDemoArgumentFlag, const std::optional<std::string> & respawnModeArgumentFlag = {}, const std::optional<std::string> & weaponSwitchOrderArgumentFlag = {}, const std::optional<std::string> & disableMonstersArgumentFlag = {}, const std::optional<std::string> & disableSoundArgumentFlag = {}, const std::optional<std::string> & disableMusicArgumentFlag = {}, const std::optional<std::string> & setupExecutableName = {}, const std::optional<std::string> & groupFileInstallPath = {}, const std::optional<std::string> & defFileArgumentFlag = {}, const std::optional<bool> & requiresCombinedGroup = {}, const std::optional<bool> & requiresDOSBox = {}, const std::string & website = std::string(), const std::string & sourceCodeURL = std::string(), const std::vector<OperatingSystem> & supportedOperatingSystems = std::vector<OperatingSystem>(), const std::vector<std::string> & compatibleGameVersions = std::vector<std::string>());
	GameVersion(GameVersion && gameVersion) noexcept;
	GameVersion(const GameVersion & gameVersion);
	GameVersion & operator = (GameVersion && gameVersion) noexcept;
	GameVersion & operator = (const GameVersion & gameVersion);
	~GameVersion();

	bool isModified() const;
	bool hasName() const;
	const std::string & getName() const;
	bool setName(const std::string & name);
	bool isRemovable() const;
	bool isRenamable() const;
	bool hasGamePath() const;
	const std::string & getGamePath() const;
	void setGamePath(const std::string & gamePath);
	const std::string & getGameExecutableName() const;
	void setGameExecutableName(const std::string & gameExecutableName);
	bool hasSetupExecutableName() const;
	const std::optional<std::string> & getSetupExecutableName() const;
	void setSetupExecutableName(const std::string & setupExecutableName);
	void clearSetupExecutableName();
	bool hasGroupFileInstallPath() const;
	const std::optional<std::string> & getGroupFileInstallPath() const;
	void setGroupFileInstallPath(const std::string & setupExecutableName);
	void clearGroupFileInstallPath();
	bool doesRequireCombinedGroup() const;
	const std::optional<bool> & getRequiresCombinedGroup() const;
	void setRequiresCombinedGroup(bool requiresCombinedGroup);
	void clearRequiresCombinedGroup();
	bool doesRequireDOSBox() const;
	const std::optional<bool> & getRequiresDOSBox() const;
	void setRequiresDOSBox(bool requiresDOSBox);
	void clearRequiresDOSBox();
	bool hasLocalWorkingDirectory() const;
	void setLocalWorkingDirectory(bool localWorkingDirectory);
	const std::string & getModDirectoryName() const;
	void setModDirectoryName(const std::string & modDirectoryName);
	bool hasRelativeConFilePath() const;
	void setRelativeConFilePath(bool relativeConFilePath);
	bool doesSupportSubdirectories() const;
	void setSupportsSubdirectories(bool supportsSubdirectories);
	bool hasConFileArgumentFlag() const;
	const std::optional<std::string> & getConFileArgumentFlag() const;
	bool setConFileArgumentFlag(const std::string & flag);
	void clearConFileArgumentFlag();
	bool hasGroupFileArgumentFlag() const;
	const std::optional<std::string> & getGroupFileArgumentFlag() const;
	bool setGroupFileArgumentFlag(const std::string & flag);
	void clearGroupFileArgumentFlag();
	bool hasDefFileArgumentFlag() const;
	const std::optional<std::string> & getDefFileArgumentFlag() const;
	bool setDefFileArgumentFlag(const std::string & flag);
	void clearDefFileArgumentFlag();
	const std::string & getMapFileArgumentFlag() const;
	bool setMapFileArgumentFlag(const std::string & flag);
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

	size_t numberOfSupportedOperatingSystems() const;
	bool hasSupportedOperatingSystem(OperatingSystem operatingSystem) const;
	bool hasSupportedOperatingSystemType(DeviceInformationBridge::OperatingSystemType operatingSystemType) const;
	bool hasSupportedOperatingSystemWithName(const std::string & operatingSystemName) const;
	size_t indexOfSupportedOperatingSystem(OperatingSystem operatingSystem) const;
	size_t indexOfSupportedOperatingSystemType(DeviceInformationBridge::OperatingSystemType operatingSystemType) const;
	size_t indexOfSupportedOperatingSystemWithName(const std::string & operatingSystemName) const;
	std::optional<OperatingSystem> getSupportedOperatingSystem(size_t index) const;
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
	bool hasCompatibleGameVersionWithName(const std::string & name) const;
	size_t indexOfCompatibleGameVersion(const GameVersion & gameVersion) const;
	size_t indexOfCompatibleGameVersionWithName(const std::string & name) const;
	std::optional<std::string> getCompatibleGameVersion(size_t index) const;
	const std::vector<std::string> & getCompatibleGameVersions() const;
	std::shared_ptr<ModGameVersion> getMostCompatibleModGameVersion(const std::vector<std::shared_ptr<ModGameVersion>> & modGameVersions) const;
	std::vector<std::shared_ptr<ModGameVersion>> getCompatibleModGameVersions(const std::vector<std::shared_ptr<ModGameVersion>> & modGameVersions) const;
	bool addCompatibleGameVersion(const GameVersion & gameVersion);
	bool addCompatibleGameVersionWithName(const std::string & name);
	size_t addCompatibleGameVersionNames(const std::vector<std::string> & gameVersions);
	bool removeCompatibleGameVersion(size_t index);
	bool removeCompatibleGameVersion(const GameVersion & gameVersion);
	bool removeCompatibleGameVersionWithName(const std::string & name);
	void clearCompatibleGameVersions();

	size_t checkForMissingExecutables() const;

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<GameVersion> parseFrom(const rapidjson::Value & gameVersionValue);

	static std::optional<OperatingSystem> convertOperatingSystemType(DeviceInformationBridge::OperatingSystemType operatingSystemType);

	bool isConfigured() const;
	static bool isConfigured(const GameVersion * gameVersion);
	bool isValid() const;
	static bool isValid(const GameVersion * gameVersion);

	size_t numberOfListeners() const;
	bool hasListener(const Listener & listener) const;
	size_t indexOfListener(const Listener & listener) const;
	Listener * getListener(size_t index) const;
	bool addListener(Listener & listener);
	bool removeListener(size_t index);
	bool removeListener(const Listener & listener);
	void clearListeners();

	bool operator == (const GameVersion & gameVersion) const;
	bool operator != (const GameVersion & gameVersion) const;

	static const std::string ALL_VERSIONS;
	static const GameVersion LAMEDUKE;
	static const GameVersion ORIGINAL_REGULAR_VERSION;
	static const GameVersion ORIGINAL_ATOMIC_EDITION;
	static const GameVersion JFDUKE3D;
	static const GameVersion EDUKE32;
	//static const GameVersion MEGATON_EDITION;
	//static const GameVersion WORLD_TOUR;
	//static const GameVersion BUILDGDX;
	static const GameVersion RAZE;
	static const GameVersion RED_NUKEM;
	static const GameVersion CHOCOLATE_DUKE3D;
	static const GameVersion BELGIAN_CHOCOLATE_DUKE3D;
	static const std::vector<const GameVersion *> DEFAULT_GAME_VERSIONS;

	static const bool DEFAULT_LOCAL_WORKING_DIRECTORY;
	static const bool DEFAULT_RELATIVE_CON_FILE_PATH;
	static const bool DEFAULT_SUPPORTS_SUBDIRECTORIES;
	static const std::string DEFAULT_CON_FILE_ARGUMENT_FLAG;
	static const std::string DEFAULT_GROUP_FILE_ARGUMENT_FLAG;
	static const std::string DEFAULT_MAP_FILE_ARGUMENT_FLAG;
	static const std::string DEFAULT_EPISODE_ARGUMENT_FLAG;
	static const std::string DEFAULT_LEVEL_ARGUMENT_FLAG;
	static const std::string DEFAULT_SKILL_ARGUMENT_FLAG;
	static const uint8_t DEFAULT_SKILL_START_VALUE;
	static const std::string DEFAULT_RECORD_DEMO_ARGUMENT_FLAG;
	static const std::string DEFAULT_PLAY_DEMO_ARGUMENT_FLAG;
	static const std::string DEFAULT_RESPAWN_MODE_ARGUMENT_FLAG;
	static const std::string DEFAULT_WEAPON_SWITCH_ORDER_ARGUMENT_FLAG;
	static const std::string DEFAULT_DISABLE_MONSTERS_ARGUMENT_FLAG;
	static const std::string DEFAULT_DISABLE_SOUND_ARGUMENT_FLAG;
	static const std::string DEFAULT_DISABLE_MUSIC_ARGUMENT_FLAG;

private:
	void setModified(bool modified);
	void notifyGameVersionModified();

	std::string m_name;
	bool m_removable;
	bool m_renamable;
	std::string m_gamePath;
	std::string m_gameExecutableName;
	std::optional<std::string> m_setupExecutableName;
	std::optional<std::string> m_groupFileInstallPath;
	std::optional<bool> m_requiresCombinedGroup;
	std::optional<bool> m_requiresDOSBox;
	std::string m_modDirectoryName;
	std::string m_website;
	std::string m_sourceCodeURL;
	bool m_localWorkingDirectory;
	bool m_relativeConFilePath;
	bool m_supportsSubdirectories;
	std::optional<std::string> m_conFileArgumentFlag;
	std::optional<std::string> m_groupFileArgumentFlag;
	std::optional<std::string> m_defFileArgumentFlag;
	std::string m_mapFileArgumentFlag;
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
	std::vector<std::string> m_compatibleGameVersions;
	mutable bool m_modified;
	mutable std::vector<Listener *> m_listeners;
};

#endif // _GAME_VERSION_H_
