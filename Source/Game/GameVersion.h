#ifndef _GAME_VERSION_H_
#define _GAME_VERSION_H_

class ModGameVersion;

#include <rapidjson/document.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

class GameVersion final {
public:
	GameVersion(const std::string & name, const std::string & gamePath, const std::string & gameExecutableName, bool relativeConFilePath, const std::string & conFileArgumentFlag, const std::string & groupFileArgumentFlag, const std::string & mapFileArgumentFlag, const std::string & modDirectoryName, std::optional<std::string> setupExecutableName = {}, std::optional<std::string> defFileArgumentFlag = {}, std::optional<bool> requiresDOSBox = {}, const std::string & website = std::string(), const std::string & sourceCodeURL = std::string(), const std::vector<std::string> & compatibleGameVersions = std::vector<std::string>());
	GameVersion(GameVersion && gameVersion) noexcept;
	GameVersion(const GameVersion & gameVersion);
	GameVersion & operator = (GameVersion && gameVersion) noexcept;
	GameVersion & operator = (const GameVersion & gameVersion);
	~GameVersion();

	const std::string & getName() const;
	void setName(const std::string & name);
	const std::string & getGamePath() const;
	void setGamePath(const std::string & gamePath);
	const std::string & getGameExecutableName() const;
	bool hasSetupExecutableName() const;
	std::optional<std::string> getSetupExecutableName() const;
	void setGameExecutableName(const std::string & gameExecutableName);
	void setSetupExecutableName(const std::string & setupExecutableName);
	void clearSetupExecutableName();
	bool doesRequiresDOSBox() const;
	std::optional<bool> getRequiresDOSBox() const;
	void setRequiresDOSBox(bool requiresDOSBox);
	void clearRequiresDOSBox();
	const std::string & getModDirectoryName() const;
	void setModDirectoryName(const std::string & modDirectoryName);
	bool hasRelativeConFilePath() const;
	void setRelativeConFilePath(bool relativeConFilePath);
	const std::string & getConFileArgumentFlag() const;
	bool setConFileArgumentFlag(const std::string & flag);
	const std::string & getGroupFileArgumentFlag() const;
	bool setGroupFileArgumentFlag(const std::string & flag);
	bool hasDefFileArgumentFlag() const;
	std::optional<std::string> getDefFileArgumentFlag() const;
	bool setDefFileArgumentFlag(const std::string & flag);
	void clearDefFileArgumentFlag();
	const std::string & getMapFileArgumentFlag() const;
	bool setMapFileArgumentFlag(const std::string & flag);
	const std::string & getWebsite() const;
	void setWebsite(const std::string & website);
	const std::string & getSourceCodeURL() const;
	void setSourceCodeURL(const std::string & sourceCodeURL);

	size_t numberOfCompatibleGameVersions() const;
	bool hasCompatibleGameVersion(const std::string & gameVersion) const;
	bool hasCompatibleGameVersion(const GameVersion & gameVersion) const;
	size_t indexOfCompatibleGameVersion(const std::string & gameVersion) const;
	size_t indexOfCompatibleGameVersion(const GameVersion & gameVersion) const;
	std::optional<std::string> getCompatibleGameVersion(size_t index) const;
	std::shared_ptr<ModGameVersion> getMostCompatibleModGameVersion(const std::vector<std::shared_ptr<ModGameVersion>> & modGameVersions) const;
	std::vector<std::shared_ptr<ModGameVersion>> getCompatibleModGameVersions(const std::vector<std::shared_ptr<ModGameVersion>> & modGameVersions) const;
	bool addCompatibleGameVersion(const std::string & gameVersion);
	bool addCompatibleGameVersion(const GameVersion & gameVersion);
	size_t addCompatibleGameVersions(const std::vector<std::string> & gameVersions);
	bool removeCompatibleGameVersion(size_t index);
	bool removeCompatibleGameVersion(const std::string & gameVersion);
	bool removeCompatibleGameVersion(const GameVersion & gameVersion);
	void clearCompatibleGameVersions();

	size_t checkForMissingExecutables() const;

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<GameVersion> parseFrom(const rapidjson::Value & gameVersionValue);

	bool isConfigured() const;
	bool isValid() const;
	static bool isValid(const GameVersion * gameVersion);

	bool operator == (const GameVersion & gameVersion) const;
	bool operator != (const GameVersion & gameVersion) const;

	static const std::string ALL_VERSIONS;
	static const GameVersion REGULAR_VERSION;
	static const GameVersion ATOMIC_EDITION;
	static const GameVersion JFDUKE3D;
	static const GameVersion EDUKE32;
	//static const GameVersion MEGATON_EDITION;
	static const GameVersion WORLD_TOUR;
	//static const GameVersion BUILDGDX;
	static const GameVersion RAZE;
	static const GameVersion REDNUKEM;
	static const GameVersion CHOCOLATE_DUKE3D;
	static const GameVersion BELGIAN_CHOCOLATE_DUKE3D;
	static const std::vector<GameVersion> DEFAULT_GAME_VERSIONS;

private:
	std::string m_name;
	std::string m_gamePath;
	std::string m_gameExecutableName;
	std::optional<std::string> m_setupExecutableName;
	std::optional<bool> m_requiresDOSBox;
	std::string m_modDirectoryName;
	std::string m_website;
	std::string m_sourceCodeURL;
	bool m_relativeConFilePath;
	std::string m_conFileArgumentFlag;
	std::string m_groupFileArgumentFlag;
	std::optional<std::string> m_defFileArgumentFlag;
	std::string m_mapFileArgumentFlag;
	std::vector<std::string> m_compatibleGameVersions;
};

#endif // _GAME_VERSION_H_