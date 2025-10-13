#ifndef _GAME_DOWNLOAD_COLLECTION_H_
#define _GAME_DOWNLOAD_COLLECTION_H_

#include "../GameVersion.h"
#include "GameDownloadFile.h"

#include <Platform/DeviceInformationBridge.h>

#include <boost/signals2.hpp>
#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class GameDownload;

class GameDownloadCollection final {
public:
	GameDownloadCollection(uint32_t fileRevision = 1);
	GameDownloadCollection(GameDownloadCollection && c) noexcept;
	GameDownloadCollection(const GameDownloadCollection & c);
	GameDownloadCollection & operator = (GameDownloadCollection && c) noexcept;
	GameDownloadCollection & operator = (const GameDownloadCollection & c);
	~GameDownloadCollection();

	uint32_t getFileRevision() const;

	size_t numberOfDownloads() const;
	bool hasDownload(const GameDownload & download) const;
	bool hasDownloadWithID(const std::string & gameVersionID) const;
	size_t indexOfDownload(const GameDownload & download) const;
	size_t indexOfDownloadWithID(const std::string & download) const;
	std::shared_ptr<GameDownload> getDownload(size_t index) const;
	std::shared_ptr<GameDownload> getDownloadWithID(const std::string & download) const;
	std::shared_ptr<GameDownloadFile> getLatestGameDownloadFile(const std::string & gameID, GameDownloadFile::Type downloadType, DeviceInformationBridge::OperatingSystemType operatingSystemType, std::optional<DeviceInformationBridge::ArchitectureType> optionalArchitectureType = {}) const;
	std::shared_ptr<GameDownloadFile> getLatestGameDownloadFile(const std::string & gameID, GameDownloadFile::Type downloadType, GameVersion::OperatingSystem operatingSystem, std::optional<GameDownloadFile::ProcessorArchitecture> optionalProcessorArchitecture = {}) const;
	const std::vector<std::shared_ptr<GameDownload>> & getDownloads() const;
	bool addDownload(const GameDownload & download);
	bool removeDownload(size_t index);
	bool removeDownload(const GameDownload & download);
	bool removeDownloadWithID(const std::string & download);
	void clearDownloads();

	rapidjson::Document toJSON() const;
	static std::unique_ptr<GameDownloadCollection> parseFrom(const rapidjson::Value & gameDownloadCollectionValue);

	bool loadFrom(const std::string & filePath);
	bool loadFromJSON(const std::string & filePath);
	bool saveTo(const std::string & filePath, bool overwrite = true) const;
	bool saveToJSON(const std::string & filePath, bool overwrite = true) const;

	bool isValid() const;
	static bool isValid(const GameDownloadCollection * c);

	bool operator == (const GameDownloadCollection & c) const;
	bool operator != (const GameDownloadCollection & c) const;

	boost::signals2::signal<void (GameDownloadCollection & /* gameDownloads */)> updated;

	static const std::string FILE_TYPE;
	static const uint32_t FILE_FORMAT_VERSION;

private:
	uint32_t m_fileRevision;
	std::vector<std::shared_ptr<GameDownload>> m_downloads;
};

#endif // _GAME_DOWNLOAD_COLLECTION_H_
