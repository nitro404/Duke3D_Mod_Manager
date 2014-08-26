#include "Mod Collection/Mod.h"

Mod::Mod(const char * name, const char * id)
	: m_name(NULL)
	, m_id(NULL)
	, m_type(NULL)
	, m_gameVersion(NULL)
	, m_latestVersion(NULL)
	, m_website(NULL)
	, m_team(NULL) {
	if(name == NULL) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		m_name = Utilities::trimCopy(name);
	}

	if(id == NULL) {
		m_id = new char[1];
		m_id[0] = '\0';
	}
	else {
		m_id = Utilities::trimCopy(id);
	}
}

Mod::Mod(const QString & name, const QString & id)
	: m_name(NULL)
	, m_id(NULL)
	, m_type(NULL)
	, m_gameVersion(NULL)
	, m_latestVersion(NULL)
	, m_website(NULL)
	, m_team(NULL) {
	if(name.isEmpty()) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		QByteArray nameBytes = name.toLocal8Bit();
		const char * nameData = nameBytes.data();
		m_name = Utilities::trimCopy(nameData);
	}

	if(id.isEmpty()) {
		m_id = new char[1];
		m_id[0] = '\0';
	}
	else {
		QByteArray idBytes = id.toLocal8Bit();
		const char * idData = idBytes.data();
		m_id = Utilities::trimCopy(idData);
	}
}

Mod::Mod(const Mod & m)
	: m_name(NULL)
	, m_id(NULL)
	, m_type(NULL)
	, m_gameVersion(NULL)
	, m_latestVersion(NULL)
	, m_website(NULL)
	, m_team(NULL) {
	m_name = Utilities::trimCopy(m.m_name);
	
	m_id = Utilities::trimCopy(m.m_id);

	if(m.m_type != NULL) {
		m_type = Utilities::trimCopy(m.m_type);
	}

	if(m.m_gameVersion != NULL) {
		m_gameVersion = Utilities::trimCopy(m.m_gameVersion);
	}

	if(m.m_latestVersion != NULL) {
		m_latestVersion = Utilities::trimCopy(m.m_latestVersion);
	}

	m_releaseDate = m.m_releaseDate;

	if(m.m_website != NULL) {
		m_website = Utilities::trimCopy(m.m_website);
	}

	if(m.m_team != NULL) {
		m_team = new ModTeam(*m.m_team);
	}

	for(int i=0;i<m.m_versions.size();i++) {
		m_versions.push_back(new ModVersion(*m.m_versions[i]));
	}

	for(int i=0;i<m.m_downloads.size();i++) {
		m_downloads.push_back(new ModDownload(*m.m_downloads[i]));
	}

	for(int i=0;i<m.m_screenshots.size();i++) {
		m_screenshots.push_back(new ModScreenshot(*m.m_screenshots[i]));
	}
}

Mod & Mod::operator = (const Mod & m) {
	delete [] m_name;
	delete [] m_id;

	if(m_type != NULL) {
		delete [] m_type;
		m_type = NULL;
	}

	if(m_gameVersion != NULL) {
		delete [] m_gameVersion;
		m_gameVersion = NULL;
	}

	if(m_latestVersion != NULL) {
		delete [] m_latestVersion;
		m_latestVersion = NULL;
	}

	if(m_website != NULL) {
		delete [] m_website;
		m_website = NULL;
	}

	if(m_team != NULL) {
		delete m_team;
		m_team = NULL;
	}

	for(int i=0;i<m_versions.size();i++) {
		delete m_versions[i];
	}
	m_versions.clear();

	for(int i=0;i<m_downloads.size();i++) {
		delete m_downloads[i];
	}
	m_downloads.clear();

	for(int i=0;i<m_screenshots.size();i++) {
		delete m_screenshots[i];
	}
	m_screenshots.clear();
	
	m_name = Utilities::trimCopy(m.m_name);

	m_id = Utilities::trimCopy(m.m_id);

	if(m.m_type != NULL) {
		m_type = Utilities::trimCopy(m.m_type);
	}

	if(m.m_gameVersion != NULL) {
		m_gameVersion = Utilities::trimCopy(m.m_gameVersion);
	}

	if(m.m_latestVersion != NULL) {
		m_latestVersion = Utilities::trimCopy(m.m_latestVersion);
	}

	m_releaseDate = m.m_releaseDate;

	if(m.m_website != NULL) {
		m_website = Utilities::trimCopy(m.m_website);
	}

	if(m.m_team != NULL) {
		m_team = new ModTeam(*m.m_team);
	}

	for(int i=0;i<m.m_versions.size();i++) {
		m_versions.push_back(new ModVersion(*m.m_versions[i]));
	}

	for(int i=0;i<m.m_downloads.size();i++) {
		m_downloads.push_back(new ModDownload(*m.m_downloads[i]));
	}

	for(int i=0;i<m.m_screenshots.size();i++) {
		m_screenshots.push_back(new ModScreenshot(*m.m_screenshots[i]));
	}

	return *this;
}

Mod::~Mod() {
	delete [] m_name;
	delete [] m_id;
	if(m_type != NULL)          { delete [] m_type; }
	if(m_gameVersion != NULL)   { delete [] m_gameVersion; }
	if(m_latestVersion != NULL) { delete [] m_latestVersion; }
	if(m_website != NULL)       { delete [] m_website; }
	if(m_team != NULL)          { delete m_team; }

	for(int i=0;i<m_versions.size();i++) {
		delete m_versions[i];
	}

	for(int i=0;i<m_downloads.size();i++) {
		delete m_downloads[i];
	}

	for(int i=0;i<m_screenshots.size();i++) {
		delete m_screenshots[i];
	}
}

const char * Mod::getName() const {
	return m_name;
}

const QString Mod::getFullName(int versionIndex) const {
	if(m_versions.size() == 0 || versionIndex < 0 || versionIndex >= m_versions.size()) { return QString(); }

	return QString("%1%2%3").arg(m_name).arg(m_versions[versionIndex]->getVersion() == NULL ? "" : " ").arg(m_versions[versionIndex]->getVersion() == NULL ? "" : m_versions[versionIndex]->getVersion());
}

const char * Mod::getID() const {
	return m_id;
}

const char * Mod::getType() const {
	return m_type;
}

const char * Mod::getGameVersion() const {
	return m_gameVersion;
}

const char * Mod::getLatestVersion() const {
	return m_latestVersion;
}

const QDate & Mod::getReleaseDate() const {
	return m_releaseDate;
}

const QString Mod::getReleaseDateAsString() const {
	return m_releaseDate.isNull() ? QString() : Utilities::dateToString(m_releaseDate);
}

const char * Mod::getWebsite() const {
	return m_website;
}

const ModTeam * Mod::getTeam() const {
	return m_team;
}

void Mod::setName(const char * name) {
	if(m_name != NULL) {
		delete [] m_name;
	}
	
	if(name == NULL) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		m_name = Utilities::trimCopy(name);
	}
}

void Mod::setName(const QString & name) {
	delete [] m_name;

	if(name.isEmpty()) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		QByteArray nameBytes = name.toLocal8Bit();
		const char * nameData = nameBytes.data();
		m_name = Utilities::trimCopy(nameData);
	}
}

void Mod::setID(const char * id) {
	if(m_id != NULL) {
		delete [] m_id;
	}
	
	if(id == NULL) {
		m_id = new char[1];
		m_id[0] = '\0';
	}
	else {
		m_id = Utilities::trimCopy(id);
	}
}

void Mod::setID(const QString & id) {
	delete [] m_id;

	if(id.isEmpty()) {
		m_id = new char[1];
		m_id[0] = '\0';
	}
	else {
		QByteArray idBytes = id.toLocal8Bit();
		const char * idData = idBytes.data();
		m_id = Utilities::trimCopy(idData);
	}
}

void Mod::setType(const char * type) {
	if(m_type != NULL) {
		delete [] m_type;
		m_type = NULL;
	}
	
	if(type != NULL) {
		m_type = Utilities::trimCopy(type);
	}
}

void Mod::setType(const QString & type) {
	if(m_type != NULL) {
		delete [] m_type;
		m_type = NULL;
	}

	if(!type.isNull()) {
		QByteArray typeBytes = type.toLocal8Bit();
		const char * typeData = typeBytes.data();
		m_type = Utilities::trimCopy(typeData);
	}
}

void Mod::setGameVersion(const char * gameVersion) {
	if(m_gameVersion != NULL) {
		delete [] m_gameVersion;
		m_gameVersion = NULL;
	}
	
	if(gameVersion != NULL) {
		m_gameVersion = Utilities::trimCopy(gameVersion);
	}
}

void Mod::setGameVersion(const QString & gameVersion) {
	if(m_gameVersion != NULL) {
		delete [] m_gameVersion;
		m_gameVersion = NULL;
	}

	if(!gameVersion.isNull()) {
		QByteArray gameVersionBytes = gameVersion.toLocal8Bit();
		const char * gameVersionData = gameVersionBytes.data();
		m_gameVersion = Utilities::trimCopy(gameVersionData);
	}
}

void Mod::setLatestVersion(const char * latestVersion) {
	if(m_latestVersion != NULL) {
		delete [] m_latestVersion;
		m_latestVersion = NULL;
	}
	
	if(latestVersion != NULL) {
		m_latestVersion = Utilities::trimCopy(latestVersion);
	}
}

void Mod::setLatestVersion(const QString & latestVersion) {
	if(m_latestVersion != NULL) {
		delete [] m_latestVersion;
		m_latestVersion = NULL;
	}

	if(!latestVersion.isNull()) {
		QByteArray latestVersionBytes = latestVersion.toLocal8Bit();
		const char * latestVersionData = latestVersionBytes.data();
		m_latestVersion = Utilities::trimCopy(latestVersionData);
	}
}

bool Mod::setReleaseDate(const char * releaseDate) {
	if(releaseDate == NULL) {
		m_releaseDate = QDate();
		return true;
	}
	
	QDate newReleaseDate = Utilities::parseDate(releaseDate);
	if(newReleaseDate.isValid()) {
		m_releaseDate = newReleaseDate;
		return true;
	}
	return false;
}

bool Mod::setReleaseDate(const QString & releaseDate) {
	if(releaseDate.isNull()) {
		m_releaseDate = QDate();
		return true;
	}

	QDate newReleaseDate = Utilities::parseDate(releaseDate);
	if(newReleaseDate.isValid()) {
		m_releaseDate = newReleaseDate;
		return true;
	}
	return false;
}

bool Mod::setReleaseDate(const QDate & releaseDate) {
	if(releaseDate.isNull()) {
		m_releaseDate = QDate();
		return true;
	}

	if(!releaseDate.isValid()) {
		return false;
	}

	m_releaseDate = releaseDate;

	return true;
}

void Mod::setWebsite(const char * website) {
	if(m_website != NULL) {
		delete [] m_website;
		m_website = NULL;
	}
	
	if(website != NULL) {
		m_website = Utilities::trimCopy(website);
	}
}

void Mod::setWebsite(const QString & website) {
	if(m_website != NULL) {
		delete [] m_website;
		m_website = NULL;
	}

	if(!website.isNull()) {
		QByteArray websiteBytes = website.toLocal8Bit();
		const char * websiteData = websiteBytes.data();
		m_website = Utilities::trimCopy(websiteData);
	}
}

void Mod::setTeam(ModTeam * team) {
	if(m_team != NULL) { delete m_team; }

	m_team = team;
}

bool Mod::addTeamMember(ModTeamMember * teamMember) {
	if(m_team == NULL) { return false; }

	return m_team->addMember(teamMember);
}

int Mod::numberOfVersions() const {
	return m_versions.size();
}

bool Mod::hasVersion(const ModVersion & version) const {
	for(int i=0;i<m_versions.size();i++) {
		if(*m_versions[i] == version) {
			return true;
		}
	}
	return false;
}

bool Mod::hasVersion(const char * version) const {
	for(int i=0;i<m_versions.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_versions[i]->getVersion(), version) == 0) {
			return true;
		}
	}
	return false;
}

bool Mod::hasVersion(const QString & version) const {
	const char * versionData = NULL;
	QByteArray versionBytes;
	if(!version.isNull()) {
		versionBytes = version.toLocal8Bit();
		versionData = versionBytes.data();
	}

	for(int i=0;i<m_versions.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_versions[i]->getVersion(), versionData) == 0) {
			return true;
		}
	}
	return false;
}

int Mod::indexOfVersion(const ModVersion & version) const {
	for(int i=0;i<m_versions.size();i++) {
		if(*m_versions[i] == version) {
			return i;
		}
	}
	return -1;
}

int Mod::indexOfVersion(const char * version) const {
	for(int i=0;i<m_versions.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_versions[i]->getVersion(), version) == 0) {
			return i;
		}
	}
	return -1;
}

int Mod::indexOfVersion(const QString & version) const {
	const char * versionData = NULL;
	QByteArray versionBytes;
	if(!version.isNull()) {
		versionBytes = version.toLocal8Bit();
		versionData = versionBytes.data();
	}

	for(int i=0;i<m_versions.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_versions[i]->getVersion(), versionData) == 0) {
			return i;
		}
	}
	return -1;
}

const ModVersion * Mod::getVersion(int index) const {
	if(index < 0 || index >= m_versions.size()) { return NULL; }

	return m_versions[index];
}

const ModVersion * Mod::getVersion(const char * version) const {
	for(int i=0;i<m_versions.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_versions[i]->getVersion(), version) == 0) {
			return m_versions[i];
		}
	}
	return NULL;
}

const ModVersion * Mod::getVersion(const QString & version) const {
	const char * versionData = NULL;
	QByteArray versionBytes;
	if(!version.isNull()) {
		versionBytes = version.toLocal8Bit();
		versionData = versionBytes.data();
	}

	for(int i=0;i<m_versions.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_versions[i]->getVersion(), versionData) == 0) {
			return m_versions[i];
		}
	}
	return NULL;
}

bool Mod::addVersion(ModVersion * version) {
	if(version == NULL || !version->hasFileOfType("grp") || hasVersion(*version)) {
		return false;
	}
	
	m_versions.push_back(version);

	return true;
}

bool Mod::removeVersion(int index) {
	if(index < 0 || index >= m_versions.size()) { return false; }
	
	delete m_versions[index];
	m_versions.remove(index);
	
	return true;
}

bool Mod::removeVersion(const ModVersion & version) {
	for(int i=0;i<m_versions.size();i++) {
		if(*m_versions[i] == version) {
			delete m_versions[i];
			m_versions.remove(i);
			
			return true;
		}
	}
	return false;
}

bool Mod::removeVersion(const char * version) {
	for(int i=0;i<m_versions.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_versions[i]->getVersion(), version) == 0) {
			delete m_versions[i];
			m_versions.remove(i);

			return true;
		}
	}
	return false;
}

bool Mod::removeVersion(const QString & version) {
	const char * versionData = NULL;
	QByteArray versionBytes;
	if(!version.isNull()) {
		versionBytes = version.toLocal8Bit();
		versionData = versionBytes.data();
	}

	for(int i=0;i<m_versions.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_versions[i]->getVersion(), versionData) == 0) {
			delete m_versions[i];
			m_versions.remove(i);

			return true;
		}
	}
	return false;
}

void Mod::clearVersions() {
	for(int i=0;i<m_versions.size();i++) {
		delete m_versions[i];
	}
	m_versions.clear();
}

int Mod::numberOfDownloads() const {
	return m_downloads.size();
}

bool Mod::hasDownload(const ModDownload & download) const {
	for(int i=0;i<m_downloads.size();i++) {
		if(*m_downloads[i] == download) {
			return true;
		}
	}
	return false;
}

bool Mod::hasDownload(const char * fileName) const {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return false; }

	for(int i=0;i<m_downloads.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_downloads[i]->getFileName(), fileName) == 0) {
			return true;
		}
	}
	return false;
}

bool Mod::hasDownload(const QString & fileName) const {
	if(fileName.isEmpty()) { return false; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	for(int i=0;i<m_downloads.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_downloads[i]->getFileName(), fileNameData) == 0) {
			return true;
		}
	}
	return false;
}

bool Mod::hasDownloadOfType(const char * type) const {
	if(type == NULL || Utilities::stringLength(type) == 0) { return false; }

	for(int i=0;i<m_downloads.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_downloads[i]->getType(), type) == 0) {
			return true;
		}
	}
	return false;
}

bool Mod::hasDownloadOfType(const QString & type) const {
	if(type.isEmpty()) { return false; }
	QByteArray typeBytes = type.toLocal8Bit();
	const char * typeData = typeBytes.data();

	for(int i=0;i<m_downloads.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_downloads[i]->getType(), typeData) == 0) {
			return true;
		}
	}
	return false;
}

int Mod::indexOfDownload(const ModDownload & download) const {
	for(int i=0;i<m_downloads.size();i++) {
		if(*m_downloads[i] == download) {
			return i;
		}
	}
	return -1;
}

int Mod::indexOfDownload(const char * fileName) const {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return -1; }

	for(int i=0;i<m_downloads.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_downloads[i]->getFileName(), fileName) == 0) {
			return i;
		}
	}
	return -1;
}

int Mod::indexOfDownload(const QString & fileName) const {
	if(fileName.isEmpty()) { return -1; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	for(int i=0;i<m_downloads.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_downloads[i]->getFileName(), fileNameData) == 0) {
			return i;
		}
	}
	return -1;
}

int Mod::indexOfDownloadByType(const char * type) const {
	if(type == NULL || Utilities::stringLength(type) == 0) { return -1; }

	for(int i=0;i<m_downloads.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_downloads[i]->getType(), type) == 0) {
			return i;
		}
	}
	return -1;
}

int Mod::indexOfDownloadByType(const QString & type) const {
	if(type.isEmpty()) { return -1; }
	QByteArray typeBytes = type.toLocal8Bit();
	const char * typeData = typeBytes.data();

	for(int i=0;i<m_downloads.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_downloads[i]->getType(), typeData) == 0) {
			return i;
		}
	}
	return -1;
}

const ModDownload * Mod::getDownload(int index) const {
	if(index < 0 || index >= m_downloads.size()) { return NULL; }

	return m_downloads[index];
}

const ModDownload * Mod::getDownload(const char * fileName) const {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return NULL; }

	for(int i=0;i<m_downloads.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_downloads[i]->getFileName(), fileName) == 0) {
			return m_downloads[i];
		}
	}
	return NULL;
}

const ModDownload * Mod::getDownload(const QString & fileName) const {
	if(fileName.isEmpty()) { return NULL; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	for(int i=0;i<m_downloads.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_downloads[i]->getFileName(), fileNameData) == 0) {
			return m_downloads[i];
		}
	}
	return NULL;
}

const ModDownload * Mod::getDownloadByType(const char * type) const {
	if(type == NULL || Utilities::stringLength(type) == 0) { return NULL; }

	for(int i=0;i<m_downloads.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_downloads[i]->getType(), type) == 0) {
			return m_downloads[i];
		}
	}
	return NULL;
}

const ModDownload * Mod::getDownloadByType(const QString & type) const {
	if(type.isEmpty()) { return NULL; }
	QByteArray typeBytes = type.toLocal8Bit();
	const char * typeData = typeBytes.data();

	for(int i=0;i<m_downloads.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_downloads[i]->getType(), typeData) == 0) {
			return m_downloads[i];
		}
	}
	return NULL;
}

const char * Mod::getFileNameByType(const char * type) const {
	if(type == NULL || Utilities::stringLength(type) == 0) { return NULL; }

	for(int i=0;i<m_downloads.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_downloads[i]->getType(), type) == 0) {
			return m_downloads[i]->getFileName();
		}
	}
	return NULL;
}

const char * Mod::getFileNameByType(const QString & type) const {
	if(type.isEmpty()) { return NULL; }
	QByteArray typeBytes = type.toLocal8Bit();
	const char * typeData = typeBytes.data();

	for(int i=0;i<m_downloads.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_downloads[i]->getType(), typeData) == 0) {
			return m_downloads[i]->getFileName();
		}
	}
	return NULL;
}

bool Mod::addDownload(ModDownload * download) {
	if(download == NULL || Utilities::stringLength(download->getFileName()) == 0 || Utilities::stringLength(download->getType()) == 0 || hasDownload(*download)) {
		return false;
	}
	
	m_downloads.push_back(download);

	return true;
}

bool Mod::removeDownload(int index) {
	if(index < 0 || index >= m_downloads.size()) { return false; }
	
	delete m_downloads[index];
	m_downloads.remove(index);
	
	return true;
}

bool Mod::removeDownload(const ModDownload & download) {
	for(int i=0;i<m_downloads.size();i++) {
		if(*m_downloads[i] == download) {
			delete m_downloads[i];
			m_downloads.remove(i);
			
			return true;
		}
	}
	return false;
}

bool Mod::removeDownload(const char * fileName) {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return false; }

	for(int i=0;i<m_downloads.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_downloads[i]->getFileName(), fileName) == 0) {
			delete m_downloads[i];
			m_downloads.remove(i);

			return true;
		}
	}
	return false;
}

bool Mod::removeDownload(const QString & fileName) {
	if(fileName.isEmpty()) { return false; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	for(int i=0;i<m_downloads.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_downloads[i]->getFileName(), fileNameData) == 0) {
			delete m_downloads[i];
			m_downloads.remove(i);

			return true;
		}
	}
	return false;
}

bool Mod::removeDownloadByType(const char * type) {
	if(type == NULL || Utilities::stringLength(type) == 0) { return false; }

	for(int i=0;i<m_downloads.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_downloads[i]->getType(), type) == 0) {
			delete m_downloads[i];
			m_downloads.remove(i);

			return true;
		}
	}
	return false;
}

bool Mod::removeDownloadByType(const QString & type) {
	if(type.isEmpty()) { return false; }
	QByteArray typeBytes = type.toLocal8Bit();
	const char * typeData = typeBytes.data();

	for(int i=0;i<m_downloads.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_downloads[i]->getType(), typeData) == 0) {
			delete m_downloads[i];
			m_downloads.remove(i);

			return true;
		}
	}
	return false;
}

void Mod::clearDownloads() {
	for(int i=0;i<m_downloads.size();i++) {
		delete m_downloads[i];
	}
	m_downloads.clear();
}

int Mod::numberOfScreenshots() const {
	return m_screenshots.size();
}

bool Mod::hasScreenshot(const ModScreenshot & screenshot) const {
	for(int i=0;i<m_screenshots.size();i++) {
		if(*m_screenshots[i] == screenshot) {
			return true;
		}
	}
	return false;
}

bool Mod::hasScreenshot(const char * fileName) const {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return false; }

	for(int i=0;i<m_screenshots.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_screenshots[i]->getFileName(), fileName) == 0) {
			return true;
		}
	}
	return false;
}

bool Mod::hasScreenshot(const QString & fileName) const {
	if(fileName.isEmpty()) { return false; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	for(int i=0;i<m_screenshots.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_screenshots[i]->getFileName(), fileNameData) == 0) {
			return true;
		}
	}
	return false;
}

int Mod::indexOfScreenshot(const ModScreenshot & screenshot) const {
	for(int i=0;i<m_screenshots.size();i++) {
		if(*m_screenshots[i] == screenshot) {
			return i;
		}
	}
	return -1;
}

int Mod::indexOfScreenshot(const char * fileName) const {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return -1; }

	for(int i=0;i<m_screenshots.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_screenshots[i]->getFileName(), fileName) == 0) {
			return i;
		}
	}
	return -1;
}

int Mod::indexOfScreenshot(const QString & fileName) const {
	if(fileName.isEmpty()) { return -1; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	for(int i=0;i<m_screenshots.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_screenshots[i]->getFileName(), fileNameData) == 0) {
			return i;
		}
	}
	return -1;
}

const ModScreenshot * Mod::getScreenshot(int index) const {
	if(index < 0 || index >= m_screenshots.size()) { return NULL; }

	return m_screenshots[index];
}

const ModScreenshot * Mod::getScreenshot(const char * fileName) const {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return NULL; }

	for(int i=0;i<m_screenshots.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_screenshots[i]->getFileName(), fileName) == 0) {
			return m_screenshots[i];
		}
	}
	return NULL;
}

const ModScreenshot * Mod::getScreenshot(const QString & fileName) const {
	if(fileName.isEmpty()) { return NULL; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	for(int i=0;i<m_screenshots.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_screenshots[i]->getFileName(), fileNameData) == 0) {
			return m_screenshots[i];
		}
	}
	return NULL;
}

bool Mod::addScreenshot(ModScreenshot * screenshot) {
	if(screenshot == NULL || Utilities::stringLength(screenshot->getFileName()) == 0 || hasScreenshot(*screenshot)) {
		return false;
	}
	
	m_screenshots.push_back(screenshot);

	return true;
}

bool Mod::removeScreenshot(int index) {
	if(index < 0 || index >= m_screenshots.size()) { return false; }
	
	delete m_screenshots[index];
	m_screenshots.remove(index);
	
	return true;
}

bool Mod::removeScreenshot(const ModScreenshot & screenshot) {
	for(int i=0;i<m_screenshots.size();i++) {
		if(*m_screenshots[i] == screenshot) {
			delete m_screenshots[i];
			m_screenshots.remove(i);
			
			return true;
		}
	}
	return false;
}

bool Mod::removeScreenshot(const char * fileName) {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return false; }

	for(int i=0;i<m_screenshots.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_screenshots[i]->getFileName(), fileName) == 0) {
			delete m_screenshots[i];
			m_screenshots.remove(i);

			return true;
		}
	}
	return false;
}

bool Mod::removeScreenshot(const QString & fileName) {
	if(fileName.isEmpty()) { return false; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	for(int i=0;i<m_screenshots.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_screenshots[i]->getFileName(), fileNameData) == 0) {
			delete m_screenshots[i];
			m_screenshots.remove(i);

			return true;
		}
	}
	return false;
}

void Mod::clearScreenshots() {
	for(int i=0;i<m_screenshots.size();i++) {
		delete m_screenshots[i];
	}
	m_screenshots.clear();
}

bool Mod::operator == (const Mod & m) const {
	return Utilities::compareStringsIgnoreCase(m_name, m.m_name) == 0;
}

bool Mod::operator != (const Mod & m) const {
	return !operator == (m);
}
