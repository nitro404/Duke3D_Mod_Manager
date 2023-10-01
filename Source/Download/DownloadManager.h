#ifndef _DOWNLOAD_MANAGER_H_
#define _DOWNLOAD_MANAGER_H_

class DownloadCache;
class GameVersionCollection;
class HTTPRequest;
class Mod;
class ModCollection;
class ModGameVersion;
class ModVersion;
class ModVersionType;

#include <boost/signals2.hpp>

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
	bool isModDownloaded(const Mod & mod, const ModCollection * mods = nullptr, const GameVersionCollection * gameVersions = nullptr, bool checkDependencies = false, bool allowCompatibleGameVersions = false) const;
	bool isModVersionDownloaded(const ModVersion & modVersion, const ModCollection * mods = nullptr, const GameVersionCollection * gameVersions = nullptr, bool checkDependencies = false, bool allowCompatibleGameVersions = false) const;
	bool isModVersionTypeDownloaded(const ModVersionType & modVersionType, const ModCollection * mods = nullptr, const GameVersionCollection * gameVersions = nullptr, bool checkDependencies = false, bool allowCompatibleGameVersions = false) const;
	bool isModGameVersionDownloaded(const ModGameVersion & modGameVersion, const ModCollection * mods = nullptr, const GameVersionCollection * gameVersions = nullptr, bool checkDependencies = false, bool allowCompatibleGameVersions = false) const;
	DownloadCache * getDownloadCache() const;
	bool downloadModList(bool force = false);
	bool downloadModGameVersion(const ModGameVersion & modGameVersion, const ModCollection & mods, const GameVersionCollection & gameVersions, bool downloadDependencies = true, bool allowCompatibleGameVersions = true, bool force = false);
	bool uninstallModGameVersion(const ModGameVersion & modGameVersion, const GameVersionCollection & gameVersions);
	bool saveDownloadCache() const;

	boost::signals2::signal<void (const ModGameVersion & /* modGameVersion */, bool /* alreadyUpToDate */)> modDownloadCompleted;
	boost::signals2::signal<void (const ModGameVersion & /* modGameVersion */)> modDownloadCancelled;
	boost::signals2::signal<void (const ModGameVersion & /* modGameVersion */)> modDownloadFailed;
	boost::signals2::signal<bool (const ModGameVersion & /* modGameVersion */, uint8_t /* downloadStep */, uint8_t /* downloadStepCount */, std::string /* status */)> modDownloadStatusChanged;
	boost::signals2::signal<bool (const ModGameVersion & /* modGameVersion */, HTTPRequest & /* request */, size_t /* numberOfBytesDownloaded */, size_t /* totalNumberOfBytes */)> modDownloadProgress;

private:
	bool createRequiredDirectories();
	bool loadDownloadCache();
	bool notifyModDownloadStatusChanged(const ModGameVersion & modGameVersion, const std::string & status);
	void onModDownloadProgress(const ModGameVersion & modGameVersion, HTTPRequest & request, size_t numberOfBytesDownloaded, size_t totalNumberOfBytes);

	bool m_initialized;
	uint8_t m_modDownloadStep;
	std::unique_ptr<DownloadCache> m_downloadCache;

	DownloadManager(const DownloadManager &) = delete;
	const DownloadManager & operator = (const DownloadManager &) = delete;
};

#endif // _DOWNLOAD_MANAGER_H_
