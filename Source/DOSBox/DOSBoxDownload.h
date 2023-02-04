#ifndef _DOSBOX_DOWNLOAD_H_
#define _DOSBOX_DOWNLOAD_H_

#include "DOSBoxDownloadFile.h"

#include <Platform/DeviceInformationBridge.h>

#include <rapidjson/document.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

class DOSBoxDownloadVersion;

class DOSBoxDownload final {
public:
	DOSBoxDownload(const std::string & name);
	DOSBoxDownload(DOSBoxDownload && d) noexcept;
	DOSBoxDownload(const DOSBoxDownload & d);
	DOSBoxDownload & operator = (DOSBoxDownload && d) noexcept;
	DOSBoxDownload & operator = (const DOSBoxDownload & d);
	~DOSBoxDownload();

	const std::string & getName() const;

	size_t numberOfVersions() const;
	bool hasVersion(const DOSBoxDownloadVersion & version) const;
	bool hasVersion(const std::string & version) const;
	size_t indexOfVersion(const DOSBoxDownloadVersion & version) const;
	size_t indexOfVersion(const std::string & version) const;
	std::shared_ptr<DOSBoxDownloadVersion> getVersion(size_t index) const;
	std::shared_ptr<DOSBoxDownloadVersion> getVersion(const std::string & version) const;
	std::shared_ptr<DOSBoxDownloadFile> getLatestDOSBoxDownloadFile(DeviceInformationBridge::OperatingSystemType operatingSystemType, std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> optionalOperatingSystemArchitectureType = {}) const;
	const std::vector<std::shared_ptr<DOSBoxDownloadVersion>> & getVersions() const;
	bool addVersion(const DOSBoxDownloadVersion & version);
	bool removeVersion(size_t index);
	bool removeVersion(const DOSBoxDownloadVersion & version);
	bool removeVersion(const std::string & version);
	void clearVersions();

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<DOSBoxDownload> parseFrom(const rapidjson::Value & dosboxDownloadValue);

	bool isValid() const;
	static bool isValid(const DOSBoxDownload * d);

	bool operator == (const DOSBoxDownload & d) const;
	bool operator != (const DOSBoxDownload & d) const;

protected:
	void updateParent();

private:
	std::string m_name;
	std::vector<std::shared_ptr<DOSBoxDownloadVersion>> m_versions;
};

#endif // _DOSBOX_DOWNLOAD_H_
