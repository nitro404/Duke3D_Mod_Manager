#ifndef _DOWNLOAD_CACHE_H_
#define _DOWNLOAD_CACHE_H_

#include <rapidjson/document.h>

#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>

class CachedFile;
class CachedPackageFile;
class ModDownload;
class ModFile;
class ModGameVersion;

class DownloadCache final {
public:
	DownloadCache();
	DownloadCache(DownloadCache && downloadCache) noexcept;
	const DownloadCache & operator = (DownloadCache && downloadcache) noexcept;
	~DownloadCache();

	bool hasCachedModListFile() const;
	std::shared_ptr<CachedFile> getCachedModListFile() const;
	bool updateCachedModListFile(const std::string & fileName, uint64_t fileSize, const std::string & sha1, const std::string & eTag);

	size_t numberOfCachedPackageFiles() const;
	bool hasCachedPackageFileWithName(const std::string & fileName) const;
	bool hasCachedPackageFile(const ModDownload & modDownload) const;
	std::shared_ptr<CachedPackageFile> getCachedPackageFile(const ModDownload & modDownload) const;
	bool hasCachedFile(const ModFile & modFile) const;
	std::shared_ptr<CachedFile> getCachedFile(const ModFile & modFile) const;
	bool updateCachedPackageFile(const ModDownload & modDownload, const ModGameVersion & modGameVersion, uint64_t fileSize, const std::string & eTag);
	void removeCachedPackageFile(const std::string & modPackageDownloadFileName);
	void removeCachedPackageFile(const CachedPackageFile & cachedPackageFile);
	void removeCachedPackageFile(const ModDownload & modDownload);
	void clearCachedPackageFiles();

	bool loadFrom(const std::string & filePath);
	bool saveTo(const std::string & filePath) const;

	bool isValid() const;
	static bool isValid(const DownloadCache * d);

	static const std::string FILE_TYPE;
	static const std::string FILE_FORMAT_VERSION;

private:
	std::unique_ptr<CachedFile> createCachedFile(const std::string & fileName, uint64_t fileSize, const std::string & sha1, const std::string & eTag, std::optional<std::chrono::time_point<std::chrono::system_clock>> downloadedTimePoint = {});
	std::unique_ptr<CachedPackageFile> createCachedPackageFile(const std::string & fileName, uint64_t fileSize, const std::string & sha1, const std::string & eTag, std::optional<std::chrono::time_point<std::chrono::system_clock>> downloadedTimePoint = {});

	rapidjson::Document toJSON() const;
	static std::unique_ptr<DownloadCache> parseFrom(const rapidjson::Value & downloadCacheValue);

	std::shared_ptr<CachedFile> m_cachedModListFile;
	std::map<std::string, std::shared_ptr<CachedPackageFile>> m_cachedPackageFiles;

	DownloadCache(const DownloadCache &) = delete;
	const DownloadCache & operator = (const DownloadCache &) = delete;
};

#endif // _DOWNLOAD_CACHE_H_
