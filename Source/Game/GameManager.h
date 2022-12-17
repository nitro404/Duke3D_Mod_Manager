#ifndef _GAME_MANAGER_H_
#define _GAME_MANAGER_H_

#include "GameVersionCollection.h"

#include <Platform/DeviceInformationBridge.h>

#include <memory>
#include <string>

class GameDownloadCollection;
class GameVersion;

class GameManager final {
public:
	GameManager();
	~GameManager();

	bool isInitialized() const;
	bool initialize(std::shared_ptr<GameVersionCollection> gameVersions);
	bool updateGameDownloadList(bool force = false) const;
	std::string getGameDownloadURL(const std::string & gameName);
	std::string getRemoteGameDownloadsBaseURL() const;
	std::string getFallbackGameDownloadURL(const std::string & gameName) const;
	std::string getGroupDownloadURL(const std::string & gameName) const;
	std::string getFallbackGroupDownloadURL(const std::string & gameName) const;
	std::string getJFDuke3DDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const;
	std::string getEDuke32DownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const;
	std::string getRazeDownloadURL() const;
	std::string getRedNukemDownloadURL() const;
	std::string getBelgianChocolateDuke3DDownloadURL() const;
	bool installGame(const GameVersion & gameVersion, const std::string & destinationDirectoryPath, bool useFallback = false, bool overwrite = false);
	bool installGroupFile(const std::string & gameName, const std::string & directoryPath, bool useFallback = false, bool overwrite = false) const;

	static bool isGameDownloadable(const std::string & gameName);

private:
	std::string getFallbackGroupDownloadSHA1(const std::string & gameName) const;

	bool m_initialized;
	std::shared_ptr<GameVersionCollection> m_gameVersions;
	mutable std::unique_ptr<GameDownloadCollection> m_gameDownloads;
	mutable std::string m_gameListFileETag;
};

#endif // _GAME_MANAGER_H_
