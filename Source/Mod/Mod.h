#ifndef _MOD_H_
#define _MOD_H_

#include <Date.h>

#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class GameVersion;
class GameVersionCollection;
class ModDownload;
class ModGameVersion;
class ModImage;
class ModScreenshot;
class ModTeam;
class ModTeamMember;
class ModVersion;
class ModVersionType;
class ModVideo;

namespace tinyxml2 {
	class XMLDocument;
	class XMLElement;
}

class Mod final {
public:
	Mod(const std::string & id, const std::string & name, const std::string & type);
	Mod(Mod && m) noexcept;
	Mod(const Mod & m);
	Mod & operator = (Mod && m) noexcept;
	Mod & operator = (const Mod & m);
	~Mod();

	const std::string & getID() const;
	const std::string & getName() const;
	std::string getFullName(size_t versionIndex = std::numeric_limits<size_t>::max(), size_t versionTypeIndex = std::numeric_limits<size_t>::max()) const;
	bool hasAlias() const;
	const std::string & getAlias() const;
	const std::string & getType() const;
	const std::string & getPreferredVersionName() const;
	size_t indexOfPreferredVersion() const;
	size_t indexOfDefaultVersionType() const;
	std::shared_ptr<ModVersion> getPreferredVersion() const;
	const std::string & getDefaultVersionType() const;
	std::optional<Date> getLatestReleaseDate() const;
	std::optional<Date> getInitialReleaseDate() const;
	std::string getLatestReleaseDateAsString() const;
	std::string getInitialReleaseDateAsString() const;
	const std::string & getWebsite() const;
	const std::string & getRepositoryURL() const;
	bool hasTeam() const;
	std::shared_ptr<ModTeam> getTeam() const;

	void setID(const std::string & id);
	void setName(const std::string & name);
	void setAlias(const std::string & alias);
	void setType(const std::string & type);
	void setPreferredVersionName(const std::string & preferredVersionName);
	void setDefaultVersionType(const std::string & versionType);
	void setWebsite(const std::string & website);
	void setRepositoryURL(const std::string & repositoryURL);
	void setTeam(const ModTeam & team);
	void removeTeam();
	bool addTeamMember(const ModTeamMember & teamMember);

	size_t numberOfVersions() const;
	bool hasVersion(const ModVersion & version) const;
	bool hasVersion(const std::string & version) const;
	size_t indexOfVersion(const ModVersion & version) const;
	size_t indexOfVersion(const std::string & version) const;
	std::shared_ptr<ModVersion> getVersion(size_t index) const;
	std::shared_ptr<ModVersion> getVersion(const std::string & version) const;
	size_t indexOfInitialVersion() const;
	std::shared_ptr<ModVersion> getInitialVersion() const;
	size_t indexOfLatestVersion() const;
	std::shared_ptr<ModVersion> getLatestVersion() const;
	const std::vector<std::shared_ptr<ModVersion>> & getVersions() const;
	std::vector<std::string> getVersionDisplayNames(const std::string & emptySubstitution) const;
	bool addVersion(const ModVersion & version);
	bool removeVersion(size_t index);
	bool removeVersion(const ModVersion & version);
	bool removeVersion(const std::string & version);
	void clearVersions();

	size_t numberOfDownloads() const;
	bool hasDownload(const ModDownload & download) const;
	bool hasDownload(const std::string & fileName) const;
	bool hasDownloadOfType(const std::string & type) const;
	size_t indexOfDownload(const ModDownload & download) const;
	size_t indexOfDownload(const std::string & fileName) const;
	size_t indexOfDownloadByType(const std::string & type) const;
	std::shared_ptr<ModDownload> getDownload(size_t index) const;
	std::shared_ptr<ModDownload> getDownload(const std::string & fileName) const;
	std::shared_ptr<ModDownload> getDownloadByType(const std::string & type) const;
	std::optional<std::string> getDownloadFileNameByType(const std::string & type) const;
	std::shared_ptr<ModDownload> getDownloadForGameVersion(const ModGameVersion * modGameVersion) const;
	std::shared_ptr<ModVersion> getModVersionForDownload(const ModDownload * modDownload) const;
	std::shared_ptr<ModVersionType> getModVersionTypeForDownload(const ModDownload * modDownload) const;
	const std::vector<std::shared_ptr<ModDownload>> & getDownloads() const;
	bool addDownload(const ModDownload & download);
	bool removeDownload(size_t index);
	bool removeDownload(const ModDownload & download);
	bool removeDownload(const std::string & fileName);
	bool removeDownloadByType(const std::string & type);
	void clearDownloads();

	size_t numberOfScreenshots() const;
	bool hasScreenshot(const ModScreenshot & screenshot) const;
	bool hasScreenshot(const std::string & fileName) const;
	size_t indexOfScreenshot(const ModScreenshot & screenshot) const;
	size_t indexOfScreenshot(const std::string & fileName) const;
	std::shared_ptr<ModScreenshot> getScreenshot(size_t index) const;
	std::shared_ptr<ModScreenshot> getScreenshot(const std::string & fileName) const;
	const std::vector<std::shared_ptr<ModScreenshot>> & getScreenshots() const;
	bool addScreenshot(const ModScreenshot & screenshot);
	bool removeScreenshot(size_t index);
	bool removeScreenshot(const ModScreenshot & screenshot);
	bool removeScreenshot(const std::string & fileName);
	void clearScreenshots();

	size_t numberOfImages() const;
	bool hasImage(const ModImage & image) const;
	bool hasImage(const std::string & fileName) const;
	size_t indexOfImage(const ModImage & image) const;
	size_t indexOfImage(const std::string & fileName) const;
	std::shared_ptr<ModImage> getImage(size_t index) const;
	std::shared_ptr<ModImage> getImage(const std::string & fileName) const;
	const std::vector<std::shared_ptr<ModImage>> & getImages() const;
	bool addImage(const ModImage & image);
	bool removeImage(size_t index);
	bool removeImage(const ModImage & image);
	bool removeImage(const std::string & fileName);
	void clearImages();

	size_t numberOfVideos() const;
	bool hasVideo(const ModVideo & video) const;
	bool hasVideo(const std::string & url) const;
	size_t indexOfVideo(const ModVideo & video) const;
	size_t indexOfVideo(const std::string & url) const;
	std::shared_ptr<ModVideo> getVideo(size_t index) const;
	std::shared_ptr<ModVideo> getVideo(const std::string & url) const;
	const std::vector<std::shared_ptr<ModVideo>> & getVideos() const;
	bool addVideo(const ModVideo & video);
	bool removeVideo(size_t index);
	bool removeVideo(const ModVideo & video);
	bool removeVideo(const std::string & url);
	void clearVideos();

	size_t numberOfNotes() const;
	bool hasNote(const std::string & note) const;
	size_t indexOfNote(const std::string & note) const;
	std::string getNote(size_t index) const;
	const std::vector<std::string> & getNotes() const;
	bool addNote(const std::string & note);
	bool removeNote(size_t index);
	bool removeNote(const std::string & note);
	void clearNotes();

	size_t numberOfRelatedMods() const;
	bool hasRelatedMod(const std::string & relatedModID) const;
	size_t indexOfRelatedMod(const std::string & relatedModID) const;
	std::string getRelatedMod(size_t index) const;
	const std::vector<std::string> & getRelatedMods() const;
	bool addRelatedMod(const std::string & relatedModID);
	bool removeRelatedMod(size_t index);
	bool removeRelatedMod(const std::string & relatedModID);
	void clearRelatedMods();

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	tinyxml2::XMLElement * toXML(tinyxml2::XMLDocument * document) const;
	static std::unique_ptr<Mod> parseFrom(const rapidjson::Value & modValue, bool skipFileInfoValidation = false);
	static std::unique_ptr<Mod> parseFrom(const tinyxml2::XMLElement * modElement, bool skipFileInfoValidation = false);

	bool isGameVersionSupported(const GameVersion & gameVersion) const;
	bool isGameVersionCompatible(const GameVersion & gameVersion) const;
	std::vector<std::shared_ptr<GameVersion>> getSupportedGameVersions(const GameVersionCollection & gameVersions) const;
	std::vector<std::shared_ptr<GameVersion>> getSupportedGameVersions(const std::vector<std::shared_ptr<GameVersion>> & gameVersions) const;
	std::vector<std::string> getSupportedGameVersionLongNames(const GameVersionCollection & gameVersions) const;
	std::vector<std::string> getSupportedGameVersionLongNames(const std::vector<std::shared_ptr<GameVersion>> & gameVersions) const;
	std::vector<std::string> getSupportedGameVersionShortNames(const GameVersionCollection & gameVersions) const;
	std::vector<std::string> getSupportedGameVersionShortNames(const std::vector<std::shared_ptr<GameVersion>> & gameVersions) const;
	std::vector<std::shared_ptr<GameVersion>> getCompatibleGameVersions(const GameVersionCollection & gameVersions) const;
	std::vector<std::shared_ptr<GameVersion>> getCompatibleGameVersions(const std::vector<std::shared_ptr<GameVersion>> & gameVersions) const;
	std::vector<std::string> getCompatibleGameVersionLongNames(const GameVersionCollection & gameVersions) const;
	std::vector<std::string> getCompatibleGameVersionLongNames(const std::vector<std::shared_ptr<GameVersion>> & gameVersions) const;
	std::vector<std::string> getCompatibleGameVersionShortNames(const GameVersionCollection & gameVersions) const;
	std::vector<std::string> getCompatibleGameVersionShortNames(const std::vector<std::shared_ptr<GameVersion>> & gameVersions) const;

	bool checkVersions(bool verbose = true) const;
	bool checkVersionTypes(bool verbose = true) const;
	bool checkGameVersionsHaveCorrespondingDownloads(bool verbose = true) const;
	bool checkSplitDownloadsNotMissingParts(bool verbose = true) const;

	bool isValid(bool skipFileInfoValidation = false) const;
	static bool isValid(const Mod * m, bool skipFileInfoValidation = false);

	bool operator == (const Mod & m) const;
	bool operator != (const Mod & m) const;

private:
	void updateParent();

	std::string m_id;
	std::string m_name;
	std::string m_alias;
	std::string m_type;
	std::string m_preferredVersion;
	std::string m_defaultVersionType;
	std::string m_website;
	std::string m_repositoryURL;
	std::shared_ptr<ModTeam> m_team;
	std::vector<std::shared_ptr<ModVersion>> m_versions;
	std::vector<std::shared_ptr<ModDownload>> m_downloads;
	std::vector<std::shared_ptr<ModScreenshot>> m_screenshots;
	std::vector<std::shared_ptr<ModImage>> m_images;
	std::vector<std::shared_ptr<ModVideo>> m_videos;
	std::vector<std::string> m_notes;
	std::vector<std::string> m_relatedMods;
};

#endif // _MOD_H_
