#ifndef _DOSBOX_DOWNLOAD_COLLECTION_H_
#define _DOSBOX_DOWNLOAD_COLLECTION_H_

#include "DOSBoxDownloadFile.h"

#include <Platform/DeviceInformationBridge.h>

#include <boost/signals2.hpp>
#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class DOSBoxDownload;

class DOSBoxDownloadCollection final {
public:
	DOSBoxDownloadCollection();
	DOSBoxDownloadCollection(DOSBoxDownloadCollection && c) noexcept;
	DOSBoxDownloadCollection(const DOSBoxDownloadCollection & c);
	DOSBoxDownloadCollection & operator = (DOSBoxDownloadCollection && c) noexcept;
	DOSBoxDownloadCollection & operator = (const DOSBoxDownloadCollection & c);
	~DOSBoxDownloadCollection();

	size_t numberOfDownloads() const;
	bool hasDownload(const DOSBoxDownload & download) const;
	bool hasDownloadWithName(const std::string & name) const;
	size_t indexOfDownload(const DOSBoxDownload & download) const;
	size_t indexOfDownloadWithName(const std::string & name) const;
	std::shared_ptr<DOSBoxDownload> getDownload(size_t index) const;
	std::shared_ptr<DOSBoxDownload> getDownloadWithName(const std::string & name) const;
	std::shared_ptr<DOSBoxDownloadFile> getLatestDOSBoxDownloadFile(const std::string & dosboxVersionName, DeviceInformationBridge::OperatingSystemType operatingSystemType, std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> optionalOperatingSystemArchitectureType = {}) const;
	const std::vector<std::shared_ptr<DOSBoxDownload>> & getDownloads() const;
	bool addDownload(const DOSBoxDownload & download);
	bool removeDownload(size_t index);
	bool removeDownload(const DOSBoxDownload & download);
	bool removeDownloadWithName(const std::string & name);
	void clearDownloads();

	rapidjson::Document toJSON() const;
	static std::unique_ptr<DOSBoxDownloadCollection> parseFrom(const rapidjson::Value & dosboxDownloadCollectionValue);

	bool loadFrom(const std::string & filePath);
	bool loadFromJSON(const std::string & filePath);
	bool saveTo(const std::string & filePath, bool overwrite = true) const;
	bool saveToJSON(const std::string & filePath, bool overwrite = true) const;

	bool isValid() const;
	static bool isValid(const DOSBoxDownloadCollection * c);

	bool operator == (const DOSBoxDownloadCollection & c) const;
	bool operator != (const DOSBoxDownloadCollection & c) const;

	boost::signals2::signal<void (DOSBoxDownloadCollection & /* dosboxDownloads */)> updated;

	static const std::string FILE_FORMAT_VERSION;

private:
	std::vector<std::shared_ptr<DOSBoxDownload>> m_downloads;
};

#endif // _DOSBOX_DOWNLOAD_COLLECTION_H_
