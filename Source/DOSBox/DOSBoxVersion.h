#ifndef _DOSBOX_VERSION_H_
#define _DOSBOX_VERSION_H_

#include "Platform/DeviceInformationBridge.h"

#include <rapidjson/document.h>

#include <memory>
#include <string>
#include <vector>

class DOSBoxVersion final {
public:
	class Listener {
	public:
		virtual ~Listener();

		virtual void dosboxVersionModified(DOSBoxVersion & dosboxVersion) = 0;
	};

	DOSBoxVersion();
	DOSBoxVersion(const std::string & name, bool removable, bool renamable, const std::string & executableName, const std::string & directoryPath, const std::string & website, const std::string & sourceCodeURL, const std::vector<DeviceInformationBridge::OperatingSystemType> & supportedOperatingSystems = std::vector<DeviceInformationBridge::OperatingSystemType>());
	DOSBoxVersion(DOSBoxVersion && dosboxVersion) noexcept;
	DOSBoxVersion(const DOSBoxVersion & dosboxVersion);
	DOSBoxVersion & operator = (DOSBoxVersion && dosboxVersion) noexcept;
	DOSBoxVersion & operator = (const DOSBoxVersion & dosboxVersion);
	~DOSBoxVersion();

	bool isModified() const;
	bool hasName() const;
	const std::string & getName() const;
	bool setName(const std::string & name);
	bool isRemovable() const;
	bool isRenamable() const;
	bool hasDirectoryPath() const;
	const std::string & getDirectoryPath() const;
	void setDirectoryPath(const std::string & directoryPath);
	const std::string & getExecutableName() const;
	void setExecutableName(const std::string & executableName);
	const std::string & getWebsite() const;
	void setWebsite(const std::string & website);
	const std::string & getSourceCodeURL() const;
	void setSourceCodeURL(const std::string & sourceCodeURL);

	size_t numberOfSupportedOperatingSystems() const;
	bool hasSupportedOperatingSystem(DeviceInformationBridge::OperatingSystemType operatingSystem) const;
	bool hasSupportedOperatingSystemWithName(const std::string & operatingSystemName) const;
	size_t indexOfSupportedOperatingSystem(DeviceInformationBridge::OperatingSystemType operatingSystem) const;
	size_t indexOfSupportedOperatingSystemWithName(const std::string & operatingSystemName) const;
	std::optional<DeviceInformationBridge::OperatingSystemType> getSupportedOperatingSystem(size_t index) const;
	const std::vector<DeviceInformationBridge::OperatingSystemType> & getSupportedOperatingSystems() const;
	bool addSupportedOperatingSystem(DeviceInformationBridge::OperatingSystemType operatingSystem);
	bool addSupportedOperatingSystemWithName(const std::string & operatingSystemName);
	size_t addSupportedOperatingSystems(const std::vector<DeviceInformationBridge::OperatingSystemType> & operatingSystems);
	bool removeSupportedOperatingSystem(size_t index);
	bool removeSupportedOperatingSystem(DeviceInformationBridge::OperatingSystemType operatingSystem);
	bool removeSupportedOperatingSystemWithName(const std::string & operatingSystemName);
	void clearSupportedOperatingSystems();

	bool checkForMissingExecutable() const;

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<DOSBoxVersion> parseFrom(const rapidjson::Value & dosboxVersionValue);

	bool isConfigured() const;
	static bool isConfigured(const DOSBoxVersion * dosboxVersion);
	bool isValid() const;
	static bool isValid(const DOSBoxVersion * dosboxVersion);

	size_t numberOfListeners() const;
	bool hasListener(const Listener & listener) const;
	size_t indexOfListener(const Listener & listener) const;
	Listener * getListener(size_t index) const;
	bool addListener(Listener & listener);
	bool removeListener(size_t index);
	bool removeListener(const Listener & listener);
	void clearListeners();

	bool operator == (const DOSBoxVersion & dosboxVersion) const;
	bool operator != (const DOSBoxVersion & dosboxVersion) const;

	static const DOSBoxVersion DOSBOX;
	static const DOSBoxVersion DOSBOX_STAGING;
	static const DOSBoxVersion DOSBOX_X;
	static const std::vector<const DOSBoxVersion *> DEFAULT_DOSBOX_VERSIONS;

private:
	void setModified(bool modified);
	void notifyModified();

	std::string m_name;
	bool m_removable;
	bool m_renamable;
	std::string m_executableName;
	std::string m_directoryPath;
	std::string m_website;
	std::string m_sourceCodeURL;
	std::vector<DeviceInformationBridge::OperatingSystemType> m_supportedOperatingSystems;
	mutable bool m_modified;
	mutable std::vector<Listener *> m_listeners;
};

#endif // _DOSBOX_VERSION_H_
