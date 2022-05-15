#ifndef _GAME_VERSION_COLLECTION_H_
#define _GAME_VERSION_COLLECTION_H_

#include "GameVersionCollectionBroadcaster.h"

#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class GameVersion;
class ModGameVersion;

class GameVersionCollection final : public GameVersionCollectionBroadcaster {
public:
	GameVersionCollection();
	GameVersionCollection(const std::vector<GameVersion> & gameVersions);
	GameVersionCollection(const std::vector<std::shared_ptr<GameVersion>> & gameVersions);
	GameVersionCollection(GameVersionCollection && g) noexcept;
	GameVersionCollection(const GameVersionCollection & g);
	GameVersionCollection & operator = (GameVersionCollection && g) noexcept;
	GameVersionCollection & operator = (const GameVersionCollection & g);
	virtual ~GameVersionCollection();

	size_t numberOfGameVersions() const;
	bool hasGameVersion(const GameVersion & gameVersion) const;
	bool hasGameVersion(const std::string & name) const;
	size_t indexOfGameVersion(const GameVersion & gameVersion) const;
	size_t indexOfGameVersion(const std::string & name) const;
	std::shared_ptr<GameVersion> getGameVersion(size_t index) const;
	std::shared_ptr<GameVersion> getGameVersion(const std::string & name) const;
	std::vector<std::shared_ptr<GameVersion>> getGameVersionsCompatibleWith(size_t index, bool includeSupported = false, std::optional<bool> configured = {}) const;
	std::vector<std::shared_ptr<GameVersion>> getGameVersionsCompatibleWith(const std::string & name, bool includeSupported = false, std::optional<bool> configured = {}) const;
	std::vector<std::shared_ptr<GameVersion>> getGameVersionsCompatibleWith(const GameVersion & gameVersion, bool includeSupported = false, std::optional<bool> configured = {}) const;
	std::vector<std::shared_ptr<GameVersion>> getGameVersionsCompatibleWith(const ModGameVersion & modGameVersion, bool includeSupported = false, std::optional<bool> configured = {}) const;
	std::vector<std::pair<std::shared_ptr<GameVersion>, std::vector<std::shared_ptr<ModGameVersion>>>> getGameVersionsCompatibleWith(const std::vector<std::shared_ptr<ModGameVersion>> & modGameVersions, bool includeSupported = false, std::optional<bool> configured = {}) const;
	std::vector<std::shared_ptr<GameVersion>> getConfiguredGameVersions() const;
	std::vector<std::shared_ptr<GameVersion>> getUnconfiguredGameVersions() const;
	bool addGameVersion(const GameVersion & gameVersion);
	size_t addGameVersions(const std::vector<GameVersion> & gameVersions);
	size_t addGameVersions(const std::vector<std::shared_ptr<GameVersion>> & gameVersions);
	bool removeGameVersion(size_t index);
	bool removeGameVersion(const GameVersion & gameVersion);
	bool removeGameVersion(const std::string & name);
	size_t addMissingDefaultGameVersions();
	void setDefaultGameVersions();
	void clearGameVersions();

	size_t checkForMissingExecutables() const;
	size_t checkForMissingExecutables(const std::string & name) const;

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

private:
	void notifyCollectionChanged() const;

	std::vector<std::shared_ptr<GameVersion>> m_gameVersions;
};

#endif // _MOD_COLLECTION_H_
