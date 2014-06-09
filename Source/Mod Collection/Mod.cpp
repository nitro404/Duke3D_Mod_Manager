#include "Mod Collection/Mod.h"

Mod::Mod(const char * name, const char * id)
	: m_name(NULL)
	, m_id(NULL)
	, m_type(NULL)
	, m_gameVersion(NULL)
	, m_latestVersion(NULL)
	, m_releaseDate(NULL)
	, m_website(NULL) {
//	, m_team(NULL) { // TODO: uncomment when implemented
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
	, m_releaseDate(NULL)
	, m_website(NULL) {
//	, m_team(NULL) { // TODO: uncomment when implemented
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
	, m_releaseDate(NULL)
	, m_website(NULL) {
//	, m_team(NULL) { // TODO: uncomment when implemented
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

	if(m.m_releaseDate != NULL) {
		m_releaseDate = Utilities::trimCopy(m.m_releaseDate);
	}

	if(m.m_website != NULL) {
		m_website = Utilities::trimCopy(m.m_website);
	}

	for(int i=0;i<m.m_versions.size();i++) {
		m_versions.push_back(new ModVersion(*m.m_versions[i]));
	}

// TODO: add new variables
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

	if(m_releaseDate != NULL) {
		delete [] m_releaseDate;
		m_releaseDate = NULL;
	}

	if(m_website != NULL) {
		delete [] m_website;
		m_website = NULL;
	}

	for(int i=0;i<m_versions.size();i++) {
		delete m_versions[i];
	}
	m_versions.clear();
	
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

	if(m.m_releaseDate != NULL) {
		m_releaseDate = Utilities::trimCopy(m.m_releaseDate);
	}

	if(m.m_website != NULL) {
		m_website = Utilities::trimCopy(m.m_website);
	}

	for(int i=0;i<m.m_versions.size();i++) {
		m_versions.push_back(new ModVersion(*m.m_versions[i]));
	}

// TODO: add new variables

	return *this;
}

Mod::~Mod() {
	delete [] m_name;
	delete [] m_id;
	if(m_type != NULL)          { delete [] m_type; }
	if(m_gameVersion != NULL)   { delete [] m_gameVersion; }
	if(m_latestVersion != NULL) { delete [] m_latestVersion; }
	if(m_releaseDate != NULL)   { delete [] m_releaseDate; }
	if(m_website != NULL)       { delete [] m_website; }

	for(int i=0;i<m_versions.size();i++) {
		delete m_versions[i];
	}

// TODO: add new variables
}

const char * Mod::getName() const {
	return const_cast<const char *>(m_name);
}

const QString Mod::getFullName(int versionIndex) const {
	if(m_versions.size() == 0 || versionIndex < 0 || versionIndex >= m_versions.size()) { return QString(); }

	return QString("%1%2%3").arg(m_name).arg(m_versions[versionIndex]->getVersion() == NULL ? "" : " ").arg(m_versions[versionIndex]->getVersion() == NULL ? "" : m_versions[versionIndex]->getVersion());
}

const char * Mod::getID() const {
	return const_cast<const char *>(m_id);
}

const char * Mod::getType() const {
	return const_cast<const char *>(m_type);
}

const char * Mod::getGameVersion() const {
	return const_cast<const char *>(m_gameVersion);
}

const char * Mod::getLatestVersion() const {
	return const_cast<const char *>(m_latestVersion);
}

const char * Mod::getReleaseDate() const {
	return const_cast<const char *>(m_releaseDate);
}

const char * Mod::getWebsite() const {
	return const_cast<const char *>(m_website);
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

void Mod::setReleaseDate(const char * releaseDate) {
	if(m_releaseDate != NULL) {
		delete [] m_releaseDate;
		m_releaseDate = NULL;
	}
	
	if(releaseDate != NULL) {
		m_releaseDate = Utilities::trimCopy(releaseDate);
	}
}

void Mod::setReleaseDate(const QString & releaseDate) {
	if(m_releaseDate != NULL) {
		delete [] m_releaseDate;
		m_releaseDate = NULL;
	}

	if(!releaseDate.isNull()) {
		QByteArray releaseDateBytes = releaseDate.toLocal8Bit();
		const char * releaseDateData = releaseDateBytes.data();
		m_releaseDate = Utilities::trimCopy(releaseDateData);
	}
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

bool Mod::operator == (const Mod & m) const {
	return Utilities::compareStringsIgnoreCase(m_name, m.m_name) == 0;
}

bool Mod::operator != (const Mod & m) const {
	return !operator == (m);
}
