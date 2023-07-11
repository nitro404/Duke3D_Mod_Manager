#ifndef _GAME_DOWNLOAD_VERSION_H_
#define _GAME_DOWNLOAD_VERSION_H_

#include "../GameVersion.h"
#include "GameDownloadFile.h"

#include <Date.h>
#include <Platform/DeviceInformationBridge.h>

#include <rapidjson/document.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

class GameDownload;
class GameDownloadFile;

class GameDownloadVersion final {
	friend class GameDownload;

public:
	GameDownloadVersion(const std::string & version, const std::optional<Date> & releaseDate);
	GameDownloadVersion(GameDownloadVersion && v) noexcept;
	GameDownloadVersion(const GameDownloadVersion & v);
	GameDownloadVersion & operator = (GameDownloadVersion && v) noexcept;
	GameDownloadVersion & operator = (const GameDownloadVersion & v);
	~GameDownloadVersion();

	const std::string & getVersion() const;
	std::string getFullName() const;
	bool hasReleaseDate() const;
	std::optional<Date> getReleaseDate() const;
	const GameDownload * getParentGameDownload() const;

	size_t numberOfFiles() const;
	bool hasFile(const GameDownloadFile & file) const;
	bool hasFileWithName(const std::string & fileName) const;
	bool hasFileOfType(GameDownloadFile::Type type) const;
	size_t indexOfFile(const GameDownloadFile & file) const;
	size_t indexOfFileWithName(const std::string & fileName) const;
	size_t indexOfFileOfType(GameDownloadFile::Type type) const;
	std::shared_ptr<GameDownloadFile> getFile(size_t index) const;
	std::shared_ptr<GameDownloadFile> getFileWithName(const std::string & fileName) const;
	std::shared_ptr<GameDownloadFile> getFileOfType(GameDownloadFile::Type type) const;
	std::shared_ptr<GameDownloadFile> findFirstMatchingFile(std::optional<GameDownloadFile::Type> type, std::optional<GameVersion::OperatingSystem> operatingSystem, std::optional<GameDownloadFile::ProcessorArchitecture> processorArchitecture) const;
	std::shared_ptr<GameDownloadFile> findLastMatchingFile(std::optional<GameDownloadFile::Type> type, std::optional<GameVersion::OperatingSystem> operatingSystem, std::optional<GameDownloadFile::ProcessorArchitecture> processorArchitecture) const;
	std::vector<std::shared_ptr<GameDownloadFile>> findAllMatchingFiles(std::optional<GameDownloadFile::Type> type, std::optional<GameVersion::OperatingSystem> operatingSystem, std::optional<GameDownloadFile::ProcessorArchitecture> processorArchitecture) const;
	const std::vector<std::shared_ptr<GameDownloadFile>> & getFiles() const;
	bool addFile(const GameDownloadFile & file);
	bool removeFile(size_t index);
	bool removeFile(const GameDownloadFile & file);
	bool removeFileWithName(const std::string & fileName);
	bool removeFileOfType(GameDownloadFile::Type type);
	void clearFiles();

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<GameDownloadVersion> parseFrom(const rapidjson::Value & gameDownloadVersionValue);

	bool isValid() const;
	static bool isValid(const GameDownloadVersion * v);

	bool operator == (const GameDownloadVersion & v) const;
	bool operator != (const GameDownloadVersion & v) const;

protected:
	void setParentGameDownload(const GameDownload * gameDownload);
	void updateParent();

private:
	std::string m_version;
	std::optional<Date> m_releaseDate;
	std::vector<std::shared_ptr<GameDownloadFile>> m_files;
	const GameDownload * m_parentGameDownload;
};

#endif // _GAME_DOWNLOAD_VERSION_H_
