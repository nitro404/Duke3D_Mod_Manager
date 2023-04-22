#ifndef _CACHED_FILE_H_
#define _CACHED_FILE_H_

#include <rapidjson/document.h>

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

class CachedFile {
public:
	CachedFile(const std::string & fileName, uint64_t fileSize, const std::string & sha1, const std::string & eTag, std::optional<std::chrono::time_point<std::chrono::system_clock>> downloadedTimePoint = {});
	CachedFile(CachedFile && f) noexcept;
	CachedFile(const CachedFile & f);
	CachedFile & operator = (CachedFile && f) noexcept;
	CachedFile & operator = (const CachedFile & f);
	virtual ~CachedFile();

	const std::string & getFileName() const;
	bool setFileName(const std::string & fileName);
	uint64_t getFileSize() const;
	void setFileSize(uint64_t fileSize);
	const std::string & getSHA1() const;
	bool setSHA1(const std::string & sha1);
	bool hasETag() const;
	const std::string & getETag() const;
	bool setETag(const std::string & eTag);
	bool hasDownloadedTimePoint() const;
	const std::optional<std::chrono::time_point<std::chrono::system_clock>> & getDownloadedTimePoint() const;
	void setDownloadedTimePoint(std::chrono::time_point<std::chrono::system_clock> downloadedTimePoint);
	void clearDownloadedTimePoint();

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<CachedFile> parseFrom(const rapidjson::Value & cachedFileValue);

	bool isValid() const;
	static bool isValid(const CachedFile * f);

	bool operator == (const CachedFile & f) const;
	bool operator != (const CachedFile & f) const;

private:
	std::string m_fileName;
	uint64_t m_fileSize;
	std::string m_sha1;
	std::string m_eTag;
	std::optional<std::chrono::time_point<std::chrono::system_clock>> m_downloadedTimePoint;
};

#endif // _CACHED_FILE_H_
