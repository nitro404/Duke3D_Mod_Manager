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
	bool initialize();
	bool isUsingLocalMode() const;
	void setLocalMode(bool localMode);
	std::shared_ptr<GameVersionCollection> getGameVersions() const;
	std::string getGameDownloadsListFilePath() const;
	bool shouldUpdateGameDownloadList() const;
	bool loadOrUpdateGameDownloadList(bool forceUpdate = false) const;
	bool updateGameDownloadList(bool force = false) const;
	std::string getGameDownloadURL(const std::string & gameVersionID);
	std::string getGameDownloadSHA1(const std::string & gameVersionID);
	std::string getGroupDownloadURL(const std::string & gameVersionID);
	static std::string getGroupDownloadSHA1(const std::string & gameVersionID);
	std::string getRemoteGameDownloadsBaseURL() const;
	std::string getFallbackGameDownloadURL(const std::string & gameVersionID) const;
	std::string getFallbackGameDownloadSHA1(const std::string & gameVersionID) const;
	std::string getJFDuke3DDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const;
	std::string getEDuke32DownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const;
	std::string getNetDuke32DownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const;
	std::string getRazeDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const;
	std::string getRedNukemDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const;
	std::string getBelgianChocolateDuke3DDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const;
	std::string getDuke3dwDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const;
	std::string getPKDuke3DDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const;
	std::string getXDukeDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const;
	std::string getRDukeDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const;
	std::string getDuke3d_w32DownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const;
	bool installGame(const GameVersion & gameVersion, const std::string & destinationDirectoryPath, bool useFallback = false, bool overwrite = true);
	static bool isGameDownloadable(const std::string & gameVersionID);
	static bool isGroupFileDownloaded(const std::string & gameVersionID);
	bool downloadGroupFile(const std::string & gameVersionID);
	bool installGroupFile(const std::string & gameVersionID, const std::string & directoryPath, bool overwrite = true);
	void updateGroupFileSymlinks();

private:
	bool downloadGroupFile(const std::string & gameVersionID, bool useFallback);

	bool m_initialized;
	bool m_localMode;
	std::shared_ptr<GameVersionCollection> m_gameVersions;
	mutable std::unique_ptr<GameDownloadCollection> m_gameDownloads;
};

#endif // _GAME_MANAGER_H_
