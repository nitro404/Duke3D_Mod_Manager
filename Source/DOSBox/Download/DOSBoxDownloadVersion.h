#ifndef _DOSBOX_DOWNLOAD_VERSION_H_
#define _DOSBOX_DOWNLOAD_VERSION_H_

#include "../DOSBoxVersion.h"
#include "DOSBoxDownloadFile.h"

#include <Date.h>
#include <Platform/DeviceInformationBridge.h>

#include <rapidjson/document.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

class DOSBoxDownload;
class DOSBoxDownloadFile;

class DOSBoxDownloadVersion final {
	friend class DOSBoxDownload;

public:
	DOSBoxDownloadVersion(const std::string & version, const std::optional<Date> & releaseDate);
	DOSBoxDownloadVersion(DOSBoxDownloadVersion && v) noexcept;
	DOSBoxDownloadVersion(const DOSBoxDownloadVersion & v);
	DOSBoxDownloadVersion & operator = (DOSBoxDownloadVersion && v) noexcept;
	DOSBoxDownloadVersion & operator = (const DOSBoxDownloadVersion & v);
	~DOSBoxDownloadVersion();

	const std::string & getVersion() const;
	std::string getFullName() const;
	bool hasReleaseDate() const;
	std::optional<Date> getReleaseDate() const;
	const DOSBoxDownload * getParentDOSBoxDownload() const;

	size_t numberOfFiles() const;
	bool hasFile(const DOSBoxDownloadFile & file) const;
	bool hasFileWithName(const std::string & fileName) const;
	size_t indexOfFile(const DOSBoxDownloadFile & file) const;
	size_t indexOfFileWithName(const std::string & fileName) const;
	std::shared_ptr<DOSBoxDownloadFile> getFile(size_t index) const;
	std::shared_ptr<DOSBoxDownloadFile> getFileWithName(const std::string & fileName) const;
	std::shared_ptr<DOSBoxDownloadFile> findFirstMatchingFile(std::optional<DeviceInformationBridge::OperatingSystemType> operatingSystem, std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> processorArchitecture) const;
	std::shared_ptr<DOSBoxDownloadFile> findLastMatchingFile(std::optional<DeviceInformationBridge::OperatingSystemType> operatingSystem, std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> processorArchitecture) const;
	std::vector<std::shared_ptr<DOSBoxDownloadFile>> findAllMatchingFiles(std::optional<DeviceInformationBridge::OperatingSystemType> operatingSystem, std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> processorArchitecture) const;
	const std::vector<std::shared_ptr<DOSBoxDownloadFile>> & getFiles() const;
	bool addFile(const DOSBoxDownloadFile & file);
	bool removeFile(size_t index);
	bool removeFile(const DOSBoxDownloadFile & file);
	bool removeFileWithName(const std::string & fileName);
	void clearFiles();

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<DOSBoxDownloadVersion> parseFrom(const rapidjson::Value & dosboxDownloadVersionValue);

	bool isValid() const;
	static bool isValid(const DOSBoxDownloadVersion * v);

	bool operator == (const DOSBoxDownloadVersion & v) const;
	bool operator != (const DOSBoxDownloadVersion & v) const;

protected:
	void setParentDOSBoxDownload(const DOSBoxDownload * dosboxDownload);
	void updateParent();

private:
	std::string m_version;
	std::optional<Date> m_releaseDate;
	std::vector<std::shared_ptr<DOSBoxDownloadFile>> m_files;
	const DOSBoxDownload * m_parentDOSBoxDownload;
};

#endif // _DOSBOX_DOWNLOAD_VERSION_H_
