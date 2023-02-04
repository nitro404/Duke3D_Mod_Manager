#ifndef _DOSBOX_DOWNLOAD_FILE_H_
#define _DOSBOX_DOWNLOAD_FILE_H_

#include "DOSBoxVersion.h"

#include <Platform/DeviceInformationBridge.h>

#include <rapidjson/document.h>

#include <memory>
#include <optional>
#include <string>

class DOSBoxDownload;
class DOSBoxDownloadVersion;

class DOSBoxDownloadFile final {
	friend class DOSBoxDownloadVersion;

public:
	DOSBoxDownloadFile(const std::string & fileName, uint64_t fileSize, const std::string & sha1, DeviceInformationBridge::OperatingSystemType operatingSystem, std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> processorArchitecture = {});
	DOSBoxDownloadFile(DOSBoxDownloadFile && f) noexcept;
	DOSBoxDownloadFile(const DOSBoxDownloadFile & f);
	DOSBoxDownloadFile & operator = (DOSBoxDownloadFile && f) noexcept;
	DOSBoxDownloadFile & operator = (const DOSBoxDownloadFile & f);
	~DOSBoxDownloadFile();

	const std::string & getFileName() const;
	std::string_view getFileExtension() const;
	uint64_t getFileSize() const;
	const std::string & getSHA1() const;
	DeviceInformationBridge::OperatingSystemType getOperatingSystem() const;
	bool hasProcessorArchitecture() const;
	std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> getProcessorArchitecture() const;
	const DOSBoxDownload * getParentDOSBoxDownload() const;
	const DOSBoxDownloadVersion * getParentDOSBoxDownloadVersion() const;

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<DOSBoxDownloadFile> parseFrom(const rapidjson::Value & modFileValue);

	bool isValid() const;
	static bool isValid(const DOSBoxDownloadFile * f);

	bool operator == (const DOSBoxDownloadFile & f) const;
	bool operator != (const DOSBoxDownloadFile & f) const;

protected:
	void setParentDOSBoxDownloadVersion(const DOSBoxDownloadVersion * dosboxDownloadVersion);

private:
	std::string m_fileName;
	uint64_t m_fileSize;
	std::string m_sha1;
	DeviceInformationBridge::OperatingSystemType m_operatingSystem;
	std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> m_processorArchitecture;

	const DOSBoxDownloadVersion * m_parentDOSBoxDownloadVersion;
};

#endif // _DOSBOX_DOWNLOAD_FILE_H_
