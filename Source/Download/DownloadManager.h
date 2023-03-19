#ifndef _DOWNLOAD_MANAGER_H_
#define _DOWNLOAD_MANAGER_H_

class DownloadCache;
class GameVersionCollection;
class Mod;
class ModGameVersion;
class ModVersion;
class ModVersionType;

#include <memory>
#include <string>

class DownloadManager final {
public:
	DownloadManager();
	~DownloadManager();

	bool isInitialized() const;
	bool initialize();
	bool uninitialize();

	size_t numberOfDownloadedMods() const;
	std::string getDownloadCacheFilePath() const;
	std::string getCachedModListFilePath() const;
	std::string getDownloadedModsDirectoryPath() const;
	std::string getDownloadedMapsDirectoryPath() const;
	bool shouldUpdateModList() const;
	bool isModDownloaded(const Mod * mod) const;
	bool isModVersionDownloaded(const ModVersion * modVersion) const;
	bool isModVersionTypeDownloaded(const ModVersionType * modVersionType) const;
	bool isModGameVersionDownloaded(const ModGameVersion * modGameVersion) const;
	bool downloadModList(bool force = false);
	bool downloadModGameVersion(ModGameVersion * modGameVersion, GameVersionCollection * gameVersions, bool force = false);

private:
	bool createRequiredDirectories();
	bool loadDownloadCache();
	bool saveDownloadCache() const;

	bool m_initialized;
	std::unique_ptr<DownloadCache> m_downloadCache;

	DownloadManager(const DownloadManager &) = delete;
	DownloadManager(DownloadManager &&) noexcept = delete;
	const DownloadManager & operator = (const DownloadManager &) = delete;
	const DownloadManager & operator = (DownloadManager &&) noexcept = delete;
};

#endif // _DOWNLOAD_MANAGER_H_
