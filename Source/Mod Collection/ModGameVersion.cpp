#include "ModGameVersion.h"

ModGameVersion::ModGameVersion(GameVersions::GameVersion gameVersion, ModStates::ModState state)
	: m_gameVersion(gameVersion)
	, m_state(state) {
}

ModGameVersion::ModGameVersion(const ModGameVersion & m)
	: m_gameVersion(m.m_gameVersion)
	, m_state(m.m_state) {
	for(int i=0;i<m.m_files.size();i++) {
		m_files.push_back(new ModVersionFile(*m.m_files[i]));
	}
}

ModGameVersion & ModGameVersion::operator = (const ModGameVersion & m) {
	for(int i=0;i<m_files.size();i++) {
		delete m_files[i];
	}
	m_files.clear();

	m_gameVersion = m.m_gameVersion;
	m_state = m.m_state;

	for(int i=0;i<m.m_files.size();i++) {
		m_files.push_back(new ModVersionFile(*m.m_files[i]));
	}

	return *this;
}

ModGameVersion::~ModGameVersion() {
	for(int i=0;i<m_files.size();i++) {
		delete m_files[i];
	}
}

GameVersions::GameVersion ModGameVersion::getGameVersion() const {
	return m_gameVersion;
}

bool ModGameVersion::setGameVersion(int gameVersion) {
	if(!GameVersions::isValid(gameVersion)) { return false; }

	m_gameVersion = static_cast<GameVersions::GameVersion>(gameVersion);

	return true;
}

bool ModGameVersion::setGameVersion(GameVersions::GameVersion gameVersion) {
	if(!GameVersions::isValid(gameVersion)) { return false; }

	m_gameVersion = gameVersion;

	return true;
}

bool ModGameVersion::setGameVersion(const char * data) {
	if(data == NULL) { return false; }

	GameVersions::GameVersion gameVersion = GameVersions::parseFrom(data);

	if(!GameVersions::isValid(gameVersion)) { return false; }

	m_gameVersion = gameVersion;

	return true;
}

bool ModGameVersion::setGameVersion(const QString & data) {
	if(data == NULL) { return false; }

	GameVersions::GameVersion gameVersion = GameVersions::parseFrom(data);

	if(!GameVersions::isValid(gameVersion)) { return false; }

	m_gameVersion = gameVersion;

	return true;
}

ModStates::ModState ModGameVersion::getState() const {
	return m_state;
}

bool ModGameVersion::setState(ModStates::ModState state) {
	if(!ModStates::isValid(state)) { return false; }

	m_state = state;

	return true;
}

int ModGameVersion::numberOfFiles() const {
	return m_files.size();
}

bool ModGameVersion::hasFile(const ModVersionFile & file) const {
	for(int i=0;i<m_files.size();i++) {
		if(*m_files[i] == file) {
			return true;
		}
	}
	return false;
}

bool ModGameVersion::hasFile(const char * fileName) const {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return false; }

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getName(), fileName) == 0) {
			return true;
		}
	}
	return false;
}

bool ModGameVersion::hasFile(const QString & fileName) const {
	if(fileName.isEmpty()) { return false; }

	QByteArray fileNameBytes = fileName.toLocal8Bit();

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getName(), fileNameBytes.data()) == 0) {
			return true;
		}
	}
	return false;
}

bool ModGameVersion::hasFileOfType(const char * fileType) const {
	if(fileType == NULL || Utilities::stringLength(fileType) == 0) { return false; }

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getType(), fileType) == 0) {
			return true;
		}
	}
	return false;
}

bool ModGameVersion::hasFileOfType(const QString & fileType) const {
	if(fileType.isEmpty()) { return false; }

	QByteArray fileTypeBytes = fileType.toLocal8Bit();

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getType(), fileTypeBytes.data()) == 0) {
			return true;
		}
	}
	return false;
}

int ModGameVersion::indexOfFile(const ModVersionFile & file) const {
	for(int i=0;i<m_files.size();i++) {
		if(*m_files[i] == file) {
			return i;
		}
	}
	return -1;
}

int ModGameVersion::indexOfFile(const char * fileName) const {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return -1; }

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getName(), fileName) == 0) {
			return i;
		}
	}
	return -1;
}

int ModGameVersion::indexOfFile(const QString & fileName) const {
	if(fileName.isEmpty()) { return -1; }

	QByteArray fileNameBytes = fileName.toLocal8Bit();

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getName(), fileNameBytes.data()) == 0) {
			return i;
		}
	}
	return -1;
}

int ModGameVersion::indexOfFileByType(const char * fileType) const {
	if(fileType == NULL || Utilities::stringLength(fileType) == 0) { return -1; }

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getType(), fileType) == 0) {
			return i;
		}
	}
	return -1;
}

int ModGameVersion::indexOfFileByType(const QString & fileType) const {
	if(fileType.isEmpty()) { return -1; }

	QByteArray fileTypeBytes = fileType.toLocal8Bit();

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getType(), fileTypeBytes.data()) == 0) {
			return i;
		}
	}
	return -1;
}

const ModVersionFile * ModGameVersion::getFile(int index) const {
	if(index < 0 || index >= m_files.size()) { return NULL; }

	return m_files[index];
}

const ModVersionFile * ModGameVersion::getFile(const char * fileName) const {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return NULL; }

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getName(), fileName) == 0) {
			return m_files[i];
		}
	}
	return NULL;
}

const ModVersionFile * ModGameVersion::getFile(const QString & fileName) const {
	if(fileName.isEmpty()) { return NULL; }

	QByteArray fileNameBytes = fileName.toLocal8Bit();

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getName(), fileNameBytes.data()) == 0) {
			return m_files[i];
		}
	}
	return NULL;
}

const ModVersionFile * ModGameVersion::getFileByType(const char * fileType) const {
	if(fileType == NULL || Utilities::stringLength(fileType) == 0) { return NULL; }

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getType(), fileType) == 0) {
			return m_files[i];
		}
	}
	return NULL;
}

const ModVersionFile * ModGameVersion::getFileByType(const QString & fileType) const {
	if(fileType.isEmpty()) { return NULL; }

	QByteArray fileTypeBytes = fileType.toLocal8Bit();

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getType(), fileTypeBytes.data()) == 0) {
			return m_files[i];
		}
	}
	return NULL;
}

const char * ModGameVersion::getFileNameByType(const char * fileType) const {
	if(fileType == NULL || Utilities::stringLength(fileType) == 0) { return NULL; }

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getType(), fileType) == 0) {
			return m_files[i]->getName();
		}
	}
	return NULL;
}

const char * ModGameVersion::getFileNameByType(const QString & fileType) const {
	if(fileType.isEmpty()) { return NULL; }

	QByteArray fileTypeBytes = fileType.toLocal8Bit();

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getType(), fileTypeBytes.data()) == 0) {
			return m_files[i]->getName();
		}
	}
	return NULL;
}

bool ModGameVersion::addFile(ModVersionFile * file) {
	if(file == NULL || Utilities::stringLength(file->getName()) == 0 || hasFile(*file)) {
		return false;
	}
	
	m_files.push_back(file);

	return true;
}

bool ModGameVersion::removeFile(int index) {
	if(index < 0 || index >= m_files.size()) { return false; }
	
	delete m_files[index];
	m_files.remove(index);
	
	return true;
}

bool ModGameVersion::removeFile(const ModVersionFile & file) {
	for(int i=0;i<m_files.size();i++) {
		if(*m_files[i] == file) {
			delete m_files[i];
			m_files.remove(i);
			
			return true;
		}
	}
	return false;
}

bool ModGameVersion::removeFile(const char * fileName) {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return false; }

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getName(), fileName) == 0) {
			delete m_files[i];
			m_files.remove(i);

			return true;
		}
	}
	return false;
}

bool ModGameVersion::removeFile(const QString & fileName) {
	if(fileName.isEmpty()) { return false; }

	QByteArray fileNameBytes = fileName.toLocal8Bit();

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getName(), fileNameBytes.data()) == 0) {
			delete m_files[i];
			m_files.remove(i);

			return true;
		}
	}
	return false;
}

bool ModGameVersion::removeFileByType(const char * fileType) {
	if(fileType == NULL || Utilities::stringLength(fileType) == 0) { return false; }

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getType(), fileType) == 0) {
			delete m_files[i];
			m_files.remove(i);

			return true;
		}
	}
	return false;
}

bool ModGameVersion::removeFileByType(const QString & fileType) {
	if(fileType.isEmpty()) { return false; }

	QByteArray fileTypeBytes = fileType.toLocal8Bit();

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getType(), fileTypeBytes.data()) == 0) {
			delete m_files[i];
			m_files.remove(i);

			return true;
		}
	}
	return false;
}

void ModGameVersion::clearFiles() {
	for(int i=0;i<m_files.size();i++) {
		delete m_files[i];
	}
	m_files.clear();
}

bool ModGameVersion::isValid() const {
	return GameVersions::isValid(m_gameVersion) &&
		   ModStates::isValid(m_state) &&
		   m_files.size() > 0;
}

bool ModGameVersion::isValid(const ModGameVersion * m) {
	return m != NULL && m->isValid();
}

bool ModGameVersion::operator == (const ModGameVersion & m) const {
	if(m_files.size() != m.m_files.size() || m_gameVersion != m.m_gameVersion || m_state != m.m_state) {
		return false;
	}

	for(int i=0;i<m_files.size();i++) {
		if(!m.hasFile(*m_files[i])) {
			return false;
		}
	}
	return true;
}

bool ModGameVersion::operator != (const ModGameVersion & m) const {
	return !operator == (m);
}
