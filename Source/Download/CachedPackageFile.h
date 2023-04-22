#ifndef _CACHED_PACKAGE_FILE_H_
#define _CACHED_PACKAGE_FILE_H_

#include "CachedFile.h"

#include <rapidjson/document.h>

#include <cstdint>
#include <map>
#include <string>
#include <vector>

class CachedPackageFile final : public CachedFile {
public:
	CachedPackageFile(const std::string & fileName, uint64_t fileSize, const std::string & sha1, const std::string & eTag, std::optional<std::chrono::time_point<std::chrono::system_clock>> downloadedTimePoint = {});
	CachedPackageFile(CachedPackageFile && f) noexcept;
	CachedPackageFile(CachedFile && f) noexcept;
	CachedPackageFile(const CachedPackageFile & f);
	CachedPackageFile & operator = (CachedPackageFile && f) noexcept;
	CachedPackageFile & operator = (CachedFile && f) noexcept;
	CachedPackageFile & operator = (const CachedPackageFile & f);
	virtual ~CachedPackageFile();

	size_t numberOfCachedFiles() const;
	bool hasCachedFile(const CachedFile * cachedFile) const;
	bool hasCachedFileWithName(const std::string & fileName) const;
	std::shared_ptr<CachedFile> getCachedFileWithName(const std::string & fileName) const;
	std::vector<std::shared_ptr<CachedFile>> getCachedFiles() const;
	bool addCachedFile(std::unique_ptr<CachedFile> cachedFile);

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<CachedPackageFile> parseFrom(const rapidjson::Value & cachedPackageFileValue);

	bool isValid() const;
	static bool isValid(const CachedPackageFile * f);

	bool operator == (const CachedPackageFile & f) const;
	bool operator != (const CachedPackageFile & f) const;

private:
	std::map<std::string, std::shared_ptr<CachedFile>> m_cachedFiles;
};

#endif // _CACHED_FILE_H_
