#ifndef _DOSBOX_MANAGER_H_
#define _DOSBOX_MANAGER_H_

#include "DOSBoxVersion.h"
#include "DOSBoxVersionCollection.h"
#include "Download/DOSBoxDownloadCollection.h"

#include <boost/signals2.hpp>

#include <future>
#include <memory>
#include <string>

class Archive;
class HTTPRequest;

class DOSBoxManager final {
public:
	DOSBoxManager();
	~DOSBoxManager();

	bool isInitialized() const;
	bool initialize();
	bool isUsingLocalMode() const;
	void setLocalMode(bool localMode);
	std::shared_ptr<DOSBoxVersionCollection> getDOSBoxVersions() const;
	std::string getDOSBoxConfigurationsDirectoryPath() const;
	std::string getDOSBoxDownloadsListFilePath() const;
	bool loadDOSBoxConfigurations() const;
	bool saveDOSBoxConfigurations() const;
	bool shouldUpdateDOSBoxDownloadList() const;
	bool loadOrUpdateDOSBoxDownloadList(bool forceUpdate = false) const;
	bool updateDOSBoxDownloadList(bool force = false) const;
	std::string getLatestDOSBoxDownloadURL(const std::string & dosboxVersionID, std::string * latestVersion = nullptr) const;
	std::string getLatestDOSBoxDownloadURL(const std::string & dosboxVersionID, DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, std::string * latestVersion = nullptr) const;
	std::string getRemoteDOSBoxDownloadsBaseURL() const;
	std::string getLatestDOSBoxDownloadFallbackURL(const std::string & dosboxVersionID, DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, std::string * latestVersion = nullptr) const;
	std::string getLatestOriginalDOSBoxVersion() const;
	static std::string getOriginalDOSBoxDownloadURL(const std::string & version);
	std::string getLatestOriginalDOSBoxDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, std::string * latestVersion = nullptr) const;
	std::string getLatestDOSBoxStagingDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, std::string * latestVersion = nullptr) const;
	std::string getLatestDOSBoxEnhancedCommunityEditionDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, std::string * latestVersion = nullptr) const;
	std::string getLatestDOSBoxXDownloadURL(DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, std::string * latestVersion = nullptr) const;
	std::unique_ptr<Archive> downloadLatestDOSBoxVersion(const std::string & dosboxVersionID, std::string * latestVersion = nullptr, bool * aborted = nullptr);
	std::unique_ptr<Archive> downloadLatestDOSBoxVersion(const std::string & dosboxVersionID, DeviceInformationBridge::OperatingSystemType operatingSystemType, DeviceInformationBridge::OperatingSystemArchitectureType operatingSystemArchitectureType, bool useFallback = false, std::string * latestVersion = nullptr, bool * aborted = nullptr);
	bool installLatestDOSBoxVersion(const std::string & dosboxVersionID, const std::string & destinationDirectoryPath, bool overwrite = true, bool * aborted = nullptr);

	static bool isDOSBoxVersionDownloadable(const DOSBoxVersion & dosboxVersion);
	static bool isDOSBoxVersionDownloadable(const std::string & dosboxVersionID);

	boost::signals2::signal<void (std::string /* statusMessage */)> installStatusChanged;
	boost::signals2::signal<bool (const DOSBoxVersion & /* dosboxVersion */, HTTPRequest & /* request */, size_t /* numberOfBytesDownloaded */, size_t /* totalNumberOfBytes */)> dosboxDownloadProgress;

private:
	void onDOSBoxDownloadProgress(const DOSBoxVersion & dosboxVersion, HTTPRequest & request, size_t numberOfBytesDownloaded, size_t totalNumberOfBytes);

	bool m_initialized;
	bool m_localMode;
	std::shared_ptr<DOSBoxVersionCollection> m_dosboxVersions;
	mutable std::unique_ptr<DOSBoxDownloadCollection> m_dosboxDownloads;
};

#endif // _DOSBOX_MANAGER_H_
