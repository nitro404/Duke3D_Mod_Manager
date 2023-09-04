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
	DownloadManager(DownloadManager && downloadManager) noexcept;
	const DownloadManager & operator = (DownloadManager && downloadManager) noexcept;
	~DownloadManager();

	bool isInitialized() const;
	bool initialize();
	bool uninitialize();

	size_t numberOfDownloadedMods() const;
	std::string getDownloadCacheFilePath() const;
	std::string getCachedModListFilePath() const;
	std::string getDownloadedModsDirectoryPath() const;
	std::string getDownloadedMapsDirectoryPath() const;
	bool isModListDownloaded() const;
	bool shouldUpdateModList() const;
	bool isModDownloaded(const Mod & mod) const;
	bool isModVersionDownloaded(const ModVersion & modVersion) const;
	bool isModVersionTypeDownloaded(const ModVersionType & modVersionType) const;
	bool isModGameVersionDownloaded(const ModGameVersion & modGameVersion) const;
	DownloadCache * getDownloadCache() const;
	bool downloadModList(bool force = false);
	bool downloadModGameVersion(const ModGameVersion & modGameVersion, const GameVersionCollection & gameVersions, bool force = false);
	bool uninstallModGameVersion(const ModGameVersion & modGameVersion, const GameVersionCollection & gameVersions);
	bool saveDownloadCache() const;

private:
	bool createRequiredDirectories();
	bool loadDownloadCache();

	bool m_initialized;
	std::unique_ptr<DownloadCache> m_downloadCache;

	DownloadManager(const DownloadManager &) = delete;
	const DownloadManager & operator = (const DownloadManager &) = delete;
};

#endif // _DOWNLOAD_MANAGER_H_
