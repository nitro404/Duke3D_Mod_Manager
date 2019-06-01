#include "ModVersion.h"

ModVersion::ModVersion(const char * version)
	: m_version(NULL) {
	if(version != NULL) {
		m_version = Utilities::trimCopyString(version);
	}
}

ModVersion::ModVersion(const QString & version)
	: m_version(NULL) {
	if(!version.isNull()) {
		QByteArray versionBytes = version.toLocal8Bit();
		m_version = Utilities::trimCopyString(versionBytes.data());
	}
}

ModVersion::ModVersion(const ModVersion & m)
	: m_version(NULL) {
	if(m.m_version != NULL) {
		m_version = Utilities::trimCopyString(m.m_version);
	}
	
	for(int i=0;i<m.m_gameVersions.size();i++) {
		m_gameVersions.push_back(new ModGameVersion(*m.m_gameVersions[i]));
	}
}

ModVersion & ModVersion::operator = (const ModVersion & m) {
	if(m_version != NULL) {
		delete [] m_version;
		m_version = NULL;
	}

	for(int i=0;i<m_gameVersions.size();i++) {
		delete m_gameVersions[i];
	}
	m_gameVersions.clear();

	if(m.m_version != NULL) {
		m_version = Utilities::trimCopyString(m.m_version);
	}

	for(int i=0;i<m.m_gameVersions.size();i++) {
		m_gameVersions.push_back(new ModGameVersion(*m.m_gameVersions[i]));
	}

	return *this;
}

ModVersion::~ModVersion(void) {
	if(m_version != NULL) {
		delete [] m_version;
	}

	for(int i=0;i<m_gameVersions.size();i++) {
		delete m_gameVersions[i];
	}
}

const char * ModVersion::getVersion() const {
	return m_version;
}

void ModVersion::setVersion(const char * version) {
	if(m_version != NULL) {
		delete [] m_version;
		m_version = NULL;
	}
	
	if(version != NULL) {
		m_version = Utilities::trimCopyString(version);
	}
}

void ModVersion::setVersion(const QString & version) {
	if(m_version != NULL) {
		delete [] m_version;
		m_version = NULL;
	}

	if(!version.isNull()) {
		QByteArray versionBytes = version.toLocal8Bit();
		m_version = Utilities::trimCopyString(versionBytes.data());
	}
}

int ModVersion::numberOfGameVersions() const {
	return m_gameVersions.size();
}

bool ModVersion::hasGameVersion(GameVersions::GameVersion version) const {
	if(!GameVersions::isValid(version)) { return false; }

	for(int i=0;i<m_gameVersions.size();i++) {
		if(version == m_gameVersions[i]->getGameVersion()) {
			return true;
		}
	}
	return false;
}

bool ModVersion::hasGameVersion(const char * data) const {
	if(data == NULL) { return false; }

	return hasGameVersion(GameVersions::parseFrom(data));
}

bool ModVersion::hasGameVersion(const QString & data) const {
	if(data.isEmpty()) { return false; }
	
	return hasGameVersion(GameVersions::parseFrom(data));
}

bool ModVersion::hasGameVersion(const ModGameVersion * version) const {
	if(version == NULL) { return false; }

	for(int i=0;i<m_gameVersions.size();i++) {
		if(*m_gameVersions[i] == *version) {
			return true;
		}
	}
	return false;
}

int ModVersion::indexOfGameVersion(GameVersions::GameVersion version) const {
	if(!GameVersions::isValid(version)) { return -1; }

	for(int i=0;i<m_gameVersions.size();i++) {
		if(version == m_gameVersions[i]->getGameVersion()) {
			return i;
		}
	}
	return -1;
}

int ModVersion::indexOfGameVersion(const char * data) const {
	if(data == NULL) { return -1; }
	
	return indexOfGameVersion(GameVersions::parseFrom(data));
}

int ModVersion::indexOfGameVersion(const QString & data) const {
	if(data.isEmpty()) { return -1; }
	
	return indexOfGameVersion(GameVersions::parseFrom(data));
}

int ModVersion::indexOfGameVersion(const ModGameVersion * version) const {
	if(version == NULL) { return -1; }

	for(int i=0;i<m_gameVersions.size();i++) {
		if(*m_gameVersions[i] == *version) {
			return i;
		}
	}
	return -1;
}

const ModGameVersion * ModVersion::getGameVersion(int index) const {
	if(index < 0 || index >= m_gameVersions.size()) { return false; }

	return m_gameVersions[index];
}

const ModGameVersion * ModVersion::getGameVersion(GameVersions::GameVersion version) const {
	if(!GameVersions::isValid(version)) { return NULL; }
	
	for(int i=0;i<m_gameVersions.size();i++) {
		if(version == m_gameVersions[i]->getGameVersion()) {
			return m_gameVersions[i];
		}
	}
	return NULL;
}

const ModGameVersion * ModVersion::getGameVersion(const char * data) const {
	if(data == NULL) { return NULL; }

	return getGameVersion(GameVersions::parseFrom(data));
}

const ModGameVersion * ModVersion::getGameVersion(const QString & data) const {
	if(data.isEmpty()) { return NULL; }
	
	return getGameVersion(GameVersions::parseFrom(data));
}

bool ModVersion::addGameVersion(ModGameVersion * version) {
	if(!ModGameVersion::isValid(version) || !version->hasFileOfType("grp") || hasGameVersion(version)) { return false; }

	m_gameVersions.push_back(version);
	
	return true;
}

bool ModVersion::removeGameVersion(int index) {
	if(index < 0 || index >= m_gameVersions.size()) { return false; }

	delete m_gameVersions[index];
	m_gameVersions.remove(index);

	return true;
}

bool ModVersion::removeGameVersion(GameVersions::GameVersion version) {
	if(!GameVersions::isValid(version)) { return false; }
	
	for(int i=0;i<m_gameVersions.size();i++) {
		if(version == m_gameVersions[i]->getGameVersion()) {
			delete m_gameVersions[i];
			m_gameVersions.remove(i);
			return true;
		}
	}
	return false;
}

bool ModVersion::removeGameVersion(const char * data) {
	if(data == NULL) { return false; }
	
	return removeGameVersion(GameVersions::parseFrom(data));
}

bool ModVersion::removeGameVersion(const QString & data) {
	if(data.isEmpty()) { return false; }
	
	return removeGameVersion(GameVersions::parseFrom(data));
}

bool ModVersion::removeGameVersion(const ModGameVersion * version) {
	if(version == NULL) { return false; }

	for(int i=0;i<m_gameVersions.size();i++) {
		if(*m_gameVersions[i] == *version) {
			delete m_gameVersions[i];
			m_gameVersions.remove(i);
			return true;
		}
	}
	return false;
}

void ModVersion::clearGameVersions() {
	for(int i=0;i<m_gameVersions.size();i++) {
		delete m_gameVersions[i];
	}
	m_gameVersions.clear();
}

bool ModVersion::isValid() const {
	return m_gameVersions.size() > 0;
}

bool ModVersion::isValid(const ModVersion * m) {
	return m != NULL && m->isValid();
}

bool ModVersion::operator == (const ModVersion & m) const {
	return Utilities::compareStringsIgnoreCase(m_version, m.m_version) == 0;
}

bool ModVersion::operator != (const ModVersion & m) const {
	return !operator == (m);
}
