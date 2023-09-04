#ifndef _GAME_MANAGER_H_
#define _GAME_MANAGER_H_

#include "GameVersionCollection.h"

#include <Platform/DeviceInformationBridge.h>

#include <boost/signals2.hpp>

#include <future>
#include <memory>
#include <string>

class GameDownloadCollection;
class GameVersion;
class HTTPRequest;

class GameManager final {
public:
	GameManager();
	~GameManager();

	bool isInitialized() const;
	bool initialize();
	bool isUsingLocalMode() const;
	void setLocalMode(bool localMode);
	std::shared_ptr<GameVersionCollection> getGameVersions() const;
	std::string getDOSBoxConfigurationsDirectoryPath() const;
	std::string getGameDownloadsListFilePath() const;
	bool loadDOSBoxConfigurations() const;
	bool saveDOSBoxConfigurations() const;
	bool shouldUpdateGameDownloadList() const;
	bool loadOrUpdateGameDownloadList(bool forceUpdate = false) const;
	bool updateGameDownloadList(bool force = false) const;
	std::vector<std::string> getGameDownloadURLs(const std::string & gameVersionID);
	std::vector<std::string> getGameDownloadSHA1s(const std::string & gameVersionID);
	std::string getGroupDownloadURL(const std::string & gameVersionID);
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
	std::vector<std::string> getDuke3d_w32DownloadURLs(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType) const;
	bool installGame(const std::string & gameVersionID, const std::string & destinationDirectoryPath, bool useFallback = false, bool overwrite = true);
	static bool isGameDownloadable(const std::string & gameVersionID);
	bool isGroupFileDownloaded(const std::string & gameVersionID) const;
	std::shared_ptr<GameVersion> getGroupGameVersion(const std::string & gameVersionID) const;
	std::string getGroupFilePath(const std::string & gameVersionID) const;
	std::string getFallbackGroupDownloadURL(const std::string & gameVersionID) const;
	std::string getFallbackGroupDownloadSHA1(const std::string & gameVersionID) const;
	bool downloadGroupFile(const std::string & gameVersionID);
	bool installGroupFile(const std::string & gameVersionID, const std::string & directoryPath, bool overwrite = true, bool * groupSymlinked = nullptr, bool * worldTourGroup = nullptr, std::string * groupGameVersionID = nullptr);
	void updateGroupFileSymlinks();

	boost::signals2::signal<void (std::string /* statusMessage */)> installStatusChanged;
	boost::signals2::signal<void (GameVersion & /* gameVersion */, HTTPRequest & /* request */, size_t /* numberOfBytesDownloaded */, size_t /* totalNumberOfBytes */)> gameDownloadProgress;
	boost::signals2::signal<void (GameVersion & /* groupGameVersion */, HTTPRequest & /* request */, size_t /* numberOfBytesDownloaded */, size_t /* totalNumberOfBytes */)> groupDownloadProgress;

private:
	bool downloadGroupFile(const std::string & gameVersionID, bool useFallback);
	void onGameDownloadProgress(GameVersion & gameVersion, HTTPRequest & request, size_t numberOfBytesDownloaded, size_t totalNumberOfBytes);
	void onGroupDownloadProgress(GameVersion & groupGameVersion, HTTPRequest & request, size_t numberOfBytesDownloaded, size_t totalNumberOfBytes);

	bool m_initialized;
	bool m_localMode;
	std::shared_ptr<GameVersionCollection> m_gameVersions;
	mutable std::unique_ptr<GameDownloadCollection> m_gameDownloads;
};

#endif // _GAME_MANAGER_H_
