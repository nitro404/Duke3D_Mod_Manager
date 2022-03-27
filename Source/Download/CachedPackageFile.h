#ifndef _CACHED_PACKAGE_FILE_H_
#define _CACHED_PACKAGE_FILE_H_

#include "CachedFile.h"

#include <rapidjson/document.h>

#include <cstdint>
#include <map>
#include <string>

class CachedPackageFile final : public CachedFile {
public:
	CachedPackageFile(uint64_t id, const std::string & fileName, uint64_t fileSize, const std::string & sha1, const std::string & eTag);
	CachedPackageFile(CachedPackageFile && f) noexcept;
	CachedPackageFile(CachedFile && f) noexcept;
	CachedPackageFile(const CachedPackageFile & f);
	CachedPackageFile & operator = (CachedPackageFile && f) noexcept;
	CachedPackageFile & operator = (CachedFile && f) noexcept;
	CachedPackageFile & operator = (const CachedPackageFile & f);
	~CachedPackageFile();

	size_t numberOfCachedFiles() const;
	bool hasCachedFile(const CachedFile * cachedFile) const;
	bool hasCachedFileWithID(uint64_t id) const;
	bool hasCachedFileWithName(const std::string & fileName) const;
	std::shared_ptr<CachedFile> getCachedFileWithID(uint64_t id) const;
	std::shared_ptr<CachedFile> getCachedFileWithName(const std::string & fileName) const;
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
