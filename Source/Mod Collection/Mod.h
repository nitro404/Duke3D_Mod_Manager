#ifndef MOD_H
#define MOD_H

#include <QString.h>
#include <QVector.h>
#include "Utilities/Utilities.h"
#include "Mod Collection/ModVersion.h"
#include "Mod Collection/ModTeam.h"
#include "Mod Collection/ModDownload.h"

class Mod {
public:
	Mod(const char * name, const char * id);
	Mod(const QString & name, const QString & id);
	Mod(const Mod & m);
	Mod & operator = (const Mod & m);
	~Mod();

public:
	const char * getName() const;
	const QString getFullName(int versionIndex = 0) const;
	const char * getID() const;
	const char * getType() const;
	const char * getGameVersion() const;
	const char * getLatestVersion() const;
	const char * getReleaseDate() const;
	const char * getWebsite() const;
	const ModTeam * getTeam() const;

	void setName(const char * name);
	void setName(const QString & name);
	void setID(const char * id);
	void setID(const QString & id);
	void setType(const char * type);
	void setType(const QString & type);
	void setGameVersion(const char * gameVersion);
	void setGameVersion(const QString & gameVersion);
	void setLatestVersion(const char * latestVersion);
	void setLatestVersion(const QString & latestVersion);
	void setReleaseDate(const char * releaseDate);
	void setReleaseDate(const QString & releaseDate);
	void setWebsite(const char * website);
	void setWebsite(const QString & website);
	void setTeam(ModTeam * team);
	bool addTeamMember(ModTeamMember * teamMember);

	int numberOfVersions() const;
	bool hasVersion(const ModVersion & version) const;
	bool hasVersion(const char * version) const;
	bool hasVersion(const QString & version) const;
	int indexOfVersion(const ModVersion & version) const;
	int indexOfVersion(const char * version) const;
	int indexOfVersion(const QString & version) const;
	const ModVersion * getVersion(int index) const;
	const ModVersion * getVersion(const char * version) const;
	const ModVersion * getVersion(const QString & version) const;
	bool addVersion(ModVersion * version);
	bool removeVersion(int index);
	bool removeVersion(const ModVersion & version);
	bool removeVersion(const char * version);
	bool removeVersion(const QString & version);
	void clearVersions();

	int numberOfDownloads() const;
	bool hasDownload(const ModDownload & download) const;
	bool hasDownload(const char * fileName) const;
	bool hasDownload(const QString & fileName) const;
	bool hasDownloadOfType(const char * type) const;
	bool hasDownloadOfType(const QString & type) const;
	int indexOfDownload(const ModDownload & download) const;
	int indexOfDownload(const char * fileName) const;
	int indexOfDownload(const QString & fileName) const;
	int indexOfDownloadByType(const char * type) const;
	int indexOfDownloadByType(const QString & type) const;
	const ModDownload * getDownload(int index) const;
	const ModDownload * getDownload(const char * fileName) const;
	const ModDownload * getDownload(const QString & fileName) const;
	const ModDownload * getDownloadByType(const char * type) const;
	const ModDownload * getDownloadByType(const QString & type) const;
	const char * getFileNameByType(const char * type) const;
	const char * getFileNameByType(const QString & type) const;
	bool addDownload(ModDownload * download);
	bool removeDownload(int index);
	bool removeDownload(const ModDownload & download);
	bool removeDownload(const char * fileName);
	bool removeDownload(const QString & fileName);
	bool removeDownloadByType(const char * type);
	bool removeDownloadByType(const QString & type);
	void clearDownloads();

	bool operator == (const Mod & m) const;
	bool operator != (const Mod & m) const;

private:
	char * m_name;
	char * m_id;
	char * m_type;
	char * m_gameVersion;
	char * m_latestVersion;
	char * m_releaseDate;
	char * m_website;
	ModTeam * m_team;
//	QVector<ModReview *> m_reviews;
	QVector<ModVersion *> m_versions;
	QVector<ModDownload *> m_downloads;
//	QVector<ModVideo *> m_videos;
//	QVector<ModImage *> m_images;
//	QVector<ModScreenshot *> m_screenshots;
};

#endif // MOD_H
