#ifndef _GAME_DOWNLOAD_COLLECTION_H_
#define _GAME_DOWNLOAD_COLLECTION_H_

#include "GameDownloadCollectionBroadcaster.h"
#include "GameDownloadFile.h"
#include "GameVersion.h"

#include <Platform/DeviceInformationBridge.h>

#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class GameDownload;

class GameDownloadCollection final : public GameDownloadCollectionBroadcaster {
public:
	GameDownloadCollection();
	GameDownloadCollection(GameDownloadCollection && c) noexcept;
	GameDownloadCollection(const GameDownloadCollection & c);
	GameDownloadCollection & operator = (GameDownloadCollection && c) noexcept;
	GameDownloadCollection & operator = (const GameDownloadCollection & c);
	virtual ~GameDownloadCollection();

	size_t numberOfDownloads() const;
	bool hasDownload(const GameDownload & download) const;
	bool hasDownloadWithName(const std::string & name) const;
	size_t indexOfDownload(const GameDownload & download) const;
	size_t indexOfDownloadWithName(const std::string & name) const;
	std::shared_ptr<GameDownload> getDownload(size_t index) const;
	std::shared_ptr<GameDownload> getDownloadWithName(const std::string & name) const;
	std::shared_ptr<GameDownloadFile> getLatestGameDownloadFile(const std::string & gameName, GameDownloadFile::Type downloadType, DeviceInformationBridge::OperatingSystemType operatingSystemType, std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> optionalOperatingSystemArchitectureType = {}) const;
	std::shared_ptr<GameDownloadFile> getLatestGameDownloadFile(const std::string & gameName, GameDownloadFile::Type downloadType, GameVersion::OperatingSystem operatingSystem, std::optional<GameDownloadFile::ProcessorArchitecture> optionalProcessorArchitecture = {}) const;
	const std::vector<std::shared_ptr<GameDownload>> & getDownloads() const;
	bool addDownload(const GameDownload & download);
	bool removeDownload(size_t index);
	bool removeDownload(const GameDownload & download);
	bool removeDownloadWithName(const std::string & name);
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

private:
	void notifyCollectionChanged() const;

	std::vector<std::shared_ptr<GameDownload>> m_downloads;
};

#endif // _GAME_DOWNLOAD_COLLECTION_H_
