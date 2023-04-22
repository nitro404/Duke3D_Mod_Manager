#ifndef _DOSBOX_MANAGER_H_
#define _DOSBOX_MANAGER_H_

#include "DOSBoxDownloadCollection.h"
#include "DOSBoxVersion.h"
#include "DOSBoxVersionCollection.h"

#include <memory>
#include <string>

class Archive;

class DOSBoxManager final {
public:
	DOSBoxManager();
	~DOSBoxManager();

	bool isInitialized() const;
	bool initialize();
	bool isUsingLocalMode() const;
	void setLocalMode(bool localMode);
	std::shared_ptr<DOSBoxVersionCollection> getDOSBoxVersions() const;
	std::string getDOSBoxDownloadsListFilePath() const;
	bool shouldUpdateDOSBoxDownloadList() const;
	bool loadOrUpdateDOSBoxDownloadList(bool forceUpdate = false) const;
	bool updateDOSBoxDownloadList(bool force = false) const;
	std::string getLatestDOSBoxDownloadURL(const DOSBoxVersion & dosboxVersion, std::string * latestVersion = nullptr) const;
	std::string getLatestDOSBoxDownloadURL(const DOSBoxVersion & dosboxVersion, DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, std::string * latestVersion = nullptr) const;
	std::string getRemoteDOSBoxDownloadsBaseURL() const;
	std::string getLatestDOSBoxDownloadFallbackURL(const DOSBoxVersion & dosboxVersion, DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, std::string * latestVersion = nullptr) const;
	std::string getLatestOriginalDOSBoxVersion() const;
	static std::string getOriginalDOSBoxDownloadURL(const std::string & version);
	std::string getLatestOriginalDOSBoxDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, std::string * latestVersion = nullptr) const;
	std::string getLatestDOSBoxStagingDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, std::string * latestVersion = nullptr) const;
	std::string getLatestDOSBoxEnhancedCommunityEditionDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, std::string * latestVersion = nullptr) const;
	std::string getLatestDOSBoxXDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, std::string * latestVersion = nullptr) const;
	std::unique_ptr<Archive> downloadLatestDOSBoxVersion(const DOSBoxVersion & dosboxVersion, std::string * latestVersion = nullptr) const;
	std::unique_ptr<Archive> downloadLatestDOSBoxVersion(const DOSBoxVersion & dosboxVersion, DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, bool useFallback = false, std::string * latestVersion = nullptr) const;
	bool installLatestDOSBoxVersion(const DOSBoxVersion & dosboxVersion, const std::string & destinationDirectoryPath, bool overwrite = true) const;

	static bool isDOSBoxVersionDownloadable(const DOSBoxVersion & dosboxVersion);
	static bool isDOSBoxVersionDownloadable(const std::string & dosboxVersionID);

private:
	bool m_initialized;
	bool m_localMode;
	std::shared_ptr<DOSBoxVersionCollection> m_dosboxVersions;
	mutable std::unique_ptr<DOSBoxDownloadCollection> m_dosboxDownloads;
};

#endif // _DOSBOX_MANAGER_H_
