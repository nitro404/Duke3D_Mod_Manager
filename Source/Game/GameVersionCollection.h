#ifndef _GAME_VERSION_COLLECTION_H_
#define _GAME_VERSION_COLLECTION_H_

#include "GameVersion.h"

#include <boost/signals2.hpp>
#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class ModGameVersion;

class GameVersionCollection final {
public:
	GameVersionCollection();
	GameVersionCollection(const std::vector<GameVersion> & gameVersions);
	GameVersionCollection(const std::vector<const GameVersion *> & gameVersions);
	GameVersionCollection(const std::vector<std::shared_ptr<GameVersion>> & gameVersions);
	GameVersionCollection(GameVersionCollection && g) noexcept;
	GameVersionCollection(const GameVersionCollection & g);
	GameVersionCollection & operator = (GameVersionCollection && g) noexcept;
	GameVersionCollection & operator = (const GameVersionCollection & g);
	~GameVersionCollection();

	size_t numberOfGameVersions() const;
	bool hasGameVersion(const GameVersion & gameVersion) const;
	bool hasGameVersionWithID(const std::string & gameVersionID) const;
	size_t indexOfGameVersion(const GameVersion & gameVersion) const;
	size_t indexOfGameVersionWithID(const std::string & gameVersionID) const;
	std::shared_ptr<GameVersion> getGameVersion(size_t index) const;
	std::shared_ptr<GameVersion> getGameVersionWithID(const std::string & gameVersionID) const;
	std::string getLongNameOfGameVersionWithID(const std::string & gameVersionID) const;
	std::string getShortNameOfGameVersionWithID(const std::string & gameVersionID) const;
	const std::vector<std::shared_ptr<GameVersion>> & getGameVersions() const;
	std::vector<std::shared_ptr<GameVersion>> getGameVersionsCompatibleWith(size_t index, bool includeSupported = false, std::optional<bool> configured = {}) const;
	std::vector<std::shared_ptr<GameVersion>> getGameVersionsCompatibleWith(const std::string & gameVersionID, bool includeSupported = false, std::optional<bool> configured = {}) const;
	std::vector<std::shared_ptr<GameVersion>> getGameVersionsCompatibleWith(const GameVersion & gameVersion, bool includeSupported = false, std::optional<bool> configured = {}) const;
	std::vector<std::shared_ptr<GameVersion>> getGameVersionsCompatibleWith(const ModGameVersion & modGameVersion, bool includeSupported = false, std::optional<bool> configured = {}) const;
	std::vector<std::pair<std::shared_ptr<GameVersion>, std::vector<std::shared_ptr<ModGameVersion>>>> getGameVersionsCompatibleWith(const std::vector<std::shared_ptr<ModGameVersion>> & modGameVersions, bool includeSupported = false, std::optional<bool> configured = {}) const;
	std::vector<std::shared_ptr<GameVersion>> getConfiguredGameVersions() const;
	std::vector<std::shared_ptr<GameVersion>> getUnconfiguredGameVersions() const;
	std::vector<std::string> getGameVersionIdentifiers() const;
	static std::vector<std::string> getGameVersionIdentifiersFrom(const std::vector<std::shared_ptr<GameVersion>> & gameVersions);
	static std::vector<std::string> getGameVersionIdentifiersFrom(const std::vector<const GameVersion *> & gameVersions);
	std::vector<std::string> getGameVersionLongNames(bool prependItemNumber = true) const;
	static std::vector<std::string> getGameVersionLongNamesFrom(const std::vector<std::shared_ptr<GameVersion>> & gameVersions, bool prependItemNumber = true);
	static std::vector<std::string> getGameVersionLongNamesFrom(const std::vector<const GameVersion *> & gameVersions, bool prependItemNumber = true);
	std::vector<std::string> getGameVersionShortNames(bool prependItemNumber = true) const;
	static std::vector<std::string> getGameVersionShortNamesFrom(const std::vector<std::shared_ptr<GameVersion>> & gameVersions, bool prependItemNumber = true);
	static std::vector<std::string> getGameVersionShortNamesFrom(const std::vector<const GameVersion *> & gameVersions, bool prependItemNumber = true);
	bool addGameVersion(const GameVersion & gameVersion);
	bool addGameVersion(std::shared_ptr<GameVersion> gameVersion);
	size_t addGameVersions(const std::vector<GameVersion> & gameVersions);
	size_t addGameVersions(const std::vector<const GameVersion *> & gameVersions);
	size_t addGameVersions(const std::vector<std::shared_ptr<GameVersion>> & gameVersions);
	bool removeGameVersion(size_t index);
	bool removeGameVersion(const GameVersion & gameVersion);
	bool removeGameVersionWithID(const std::string & gameVersionID);
	size_t addMissingDefaultGameVersions();
	void setDefaultGameVersions();
	void clearGameVersions();

	size_t checkForMissingExecutables() const;
	size_t checkForMissingExecutables(const std::string & gameVersionID) const;

	rapidjson::Document toJSON() const;
	static std::unique_ptr<GameVersionCollection> parseFrom(const rapidjson::Value & gameVersionCollectionValue);

	bool loadFrom(const std::string & filePath, bool autoCreate = true);
	bool loadFromJSON(const std::string & filePath, bool autoCreate = true);
	bool saveTo(const std::string & filePath, bool overwrite = true) const;
	bool saveToJSON(const std::string & filePath, bool overwrite = true) const;

	bool isValid() const;
	static bool isValid(const GameVersionCollection * g);

	bool operator == (const GameVersionCollection & g) const;
	bool operator != (const GameVersionCollection & g) const;

	boost::signals2::signal<void (GameVersionCollection & /* gameVersions */, GameVersion & /* gameVersion */)> itemModified;
	boost::signals2::signal<void (GameVersionCollection & /* gameVersions */)> sizeChanged;

	static const std::string FILE_FORMAT_VERSION;

private:
	void onGameVersionModified(GameVersion & gameVersion);

	std::vector<std::shared_ptr<GameVersion>> m_gameVersions;
	std::vector<boost::signals2::connection> m_gameVersionConnections;
};

#endif // _MOD_COLLECTION_H_
