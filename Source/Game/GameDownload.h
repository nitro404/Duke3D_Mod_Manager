#ifndef _GAME_DOWNLOAD_H_
#define _GAME_DOWNLOAD_H_

#include "GameDownloadFile.h"

#include <Platform/DeviceInformationBridge.h>

#include <rapidjson/document.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

class GameDownloadVersion;

class GameDownload final {
public:
	GameDownload(const std::string & name);
	GameDownload(GameDownload && d) noexcept;
	GameDownload(const GameDownload & d);
	GameDownload & operator = (GameDownload && d) noexcept;
	GameDownload & operator = (const GameDownload & d);
	~GameDownload();

	const std::string & getName() const;

	size_t numberOfVersions() const;
	bool hasVersion(const GameDownloadVersion & version) const;
	bool hasVersion(const std::string & version) const;
	size_t indexOfVersion(const GameDownloadVersion & version) const;
	size_t indexOfVersion(const std::string & version) const;
	std::shared_ptr<GameDownloadVersion> getVersion(size_t index) const;
	std::shared_ptr<GameDownloadVersion> getVersion(const std::string & version) const;
	std::shared_ptr<GameDownloadFile> getLatestGameDownloadFile(GameDownloadFile::Type downloadType, DeviceInformationBridge::OperatingSystemType operatingSystemType, std::optional<DeviceInformationBridge::OperatingSystemArchitectureType> optionalOperatingSystemArchitectureType = {}) const;
	std::shared_ptr<GameDownloadFile> getLatestGameDownloadFile(GameDownloadFile::Type downloadType, GameVersion::OperatingSystem operatingSystem, std::optional<GameDownloadFile::ProcessorArchitecture> optionalProcessorArchitecture = {}) const;
	const std::vector<std::shared_ptr<GameDownloadVersion>> & getVersions() const;
	bool addVersion(const GameDownloadVersion & version);
	bool removeVersion(size_t index);
	bool removeVersion(const GameDownloadVersion & version);
	bool removeVersion(const std::string & version);
	void clearVersions();

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<GameDownload> parseFrom(const rapidjson::Value & gameDownloadValue);

	bool isValid() const;
	static bool isValid(const GameDownload * d);

	bool operator == (const GameDownload & d) const;
	bool operator != (const GameDownload & d) const;

protected:
	void updateParent();

private:
	std::string m_name;
	std::vector<std::shared_ptr<GameDownloadVersion>> m_versions;
};

#endif // _GAME_DOWNLOAD_H_
