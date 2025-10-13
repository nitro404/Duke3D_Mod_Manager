#ifndef _GAME_DOWNLOAD_FILE_H_
#define _GAME_DOWNLOAD_FILE_H_

#include "../GameVersion.h"

#include <Platform/DeviceInformationBridge.h>

#include <rapidjson/document.h>

#include <memory>
#include <optional>
#include <string>

class GameDownload;
class GameDownloadVersion;

class GameDownloadFile final {
	friend class GameDownloadVersion;

public:
	enum class Type {
		Game,
		Group
	};

	enum class ProcessorArchitecture {
		x86,
		x64
	};

	GameDownloadFile(const std::string & fileName, uint64_t fileSize, Type type, const std::string & sha1, GameVersion::OperatingSystem operatingSystem, std::optional<ProcessorArchitecture> processorArchitecture = {});
	GameDownloadFile(GameDownloadFile && f) noexcept;
	GameDownloadFile(const GameDownloadFile & f);
	GameDownloadFile & operator = (GameDownloadFile && f) noexcept;
	GameDownloadFile & operator = (const GameDownloadFile & f);
	~GameDownloadFile();

	const std::string & getFileName() const;
	std::string_view getFileExtension() const;
	uint64_t getFileSize() const;
	Type getType() const;
	const std::string & getSHA1() const;
	GameVersion::OperatingSystem getOperatingSystem() const;
	bool hasProcessorArchitecture() const;
	std::optional<ProcessorArchitecture> getProcessorArchitecture() const;
	const GameDownload * getParentGameDownload() const;
	const GameDownloadVersion * getParentGameDownloadVersion() const;

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<GameDownloadFile> parseFrom(const rapidjson::Value & modFileValue);

	static std::optional<ProcessorArchitecture> convertOperatingSystemArchitectureType(DeviceInformationBridge::ArchitectureType architectureType);

	bool isValid() const;
	static bool isValid(const GameDownloadFile * f);

	bool operator == (const GameDownloadFile & f) const;
	bool operator != (const GameDownloadFile & f) const;

protected:
	void setParentGameDownloadVersion(const GameDownloadVersion * gameDownloadVersion);

private:
	std::string m_fileName;
	Type m_type;
	uint64_t m_fileSize;
	std::string m_sha1;
	GameVersion::OperatingSystem m_operatingSystem;
	std::optional<ProcessorArchitecture> m_processorArchitecture;

	const GameDownloadVersion * m_parentGameDownloadVersion;
};

#endif // _GAME_DOWNLOAD_FILE_H_
