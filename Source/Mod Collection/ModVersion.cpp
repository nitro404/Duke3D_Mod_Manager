#include "ModVersion.h"

ModVersion::ModVersion(const char * version)
	: m_version(NULL) {
	if(version != NULL) {
		m_version = Utilities::trimCopy(version);
	}
}

ModVersion::ModVersion(const QString & version)
	: m_version(NULL) {
	if(!version.isNull()) {
		QByteArray versionBytes = version.toLocal8Bit();
		const char * versionData = versionBytes.data();
		m_version = Utilities::trimCopy(versionData);
	}
}

ModVersion::ModVersion(const ModVersion & m)
	: m_version(NULL) {
	if(m.m_version != NULL) {
		m_version = Utilities::trimCopy(m.m_version);
	}

	for(int i=0;i<m.m_files.size();i++) {
		m_files.push_back(new ModVersionFile(*m.m_files[i]));
	}
}

ModVersion & ModVersion::operator = (const ModVersion & m) {
	if(m_version != NULL) {
		delete [] m_version;
		m_version = NULL;
	}

	for(int i=0;i<m_files.size();i++) {
		delete m_files[i];
	}
	m_files.clear();

	if(m.m_version != NULL) {
		m_version = Utilities::trimCopy(m.m_version);
	}

	for(int i=0;i<m.m_files.size();i++) {
		m_files.push_back(new ModVersionFile(*m.m_files[i]));
	}

	return *this;
}

ModVersion::~ModVersion(void) {
	if(m_version != NULL) {
		delete [] m_version;
	}

	for(int i=0;i<m_files.size();i++) {
		delete m_files[i];
	}
}

const char * ModVersion::getVersion() const {
	return const_cast<const char *>(m_version);
}

void ModVersion::setVersion(const char * version) {
	if(m_version != NULL) {
		delete [] m_version;
		m_version = NULL;
	}
	
	if(version != NULL) {
		m_version = Utilities::trimCopy(version);
	}
}

void ModVersion::setVersion(const QString & version) {
	if(m_version != NULL) {
		delete [] m_version;
		m_version = NULL;
	}

	if(!version.isNull()) {
		QByteArray versionBytes = version.toLocal8Bit();
		const char * versionData = versionBytes.data();
		m_version = Utilities::trimCopy(versionData);
	}
}

int ModVersion::numberOfFiles() const {
	return m_files.size();
}

bool ModVersion::hasFile(const ModVersionFile & file) const {
	for(int i=0;i<m_files.size();i++) {
		if(*m_files[i] == file) {
			return true;
		}
	}
	return false;
}

bool ModVersion::hasFile(const char * fileName) const {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return false; }

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getName(), fileName) == 0) {
			return true;
		}
	}
	return false;
}

bool ModVersion::hasFile(const QString & fileName) const {
	if(fileName.isEmpty()) { return false; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getName(), fileNameData) == 0) {
			return true;
		}
	}
	return false;
}

bool ModVersion::hasFileOfType(const char * fileType) const {
	if(fileType == NULL || Utilities::stringLength(fileType) == 0) { return false; }

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getType(), fileType) == 0) {
			return true;
		}
	}
	return false;
}

bool ModVersion::hasFileOfType(const QString & fileType) const {
	if(fileType.isEmpty()) { return false; }
	QByteArray fileTypeBytes = fileType.toLocal8Bit();
	const char * fileTypeData = fileTypeBytes.data();

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getType(), fileTypeData) == 0) {
			return true;
		}
	}
	return false;
}

int ModVersion::indexOfFile(const ModVersionFile & file) const {
	for(int i=0;i<m_files.size();i++) {
		if(*m_files[i] == file) {
			return i;
		}
	}
	return -1;
}

int ModVersion::indexOfFile(const char * fileName) const {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return -1; }

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getName(), fileName) == 0) {
			return i;
		}
	}
	return -1;
}

int ModVersion::indexOfFile(const QString & fileName) const {
	if(fileName.isEmpty()) { return -1; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getName(), fileNameData) == 0) {
			return i;
		}
	}
	return -1;
}

int ModVersion::indexOfFileByType(const char * fileType) const {
	if(fileType == NULL || Utilities::stringLength(fileType) == 0) { return -1; }

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getType(), fileType) == 0) {
			return i;
		}
	}
	return -1;
}

int ModVersion::indexOfFileByType(const QString & fileType) const {
	if(fileType.isEmpty()) { return -1; }
	QByteArray fileTypeBytes = fileType.toLocal8Bit();
	const char * fileTypeData = fileTypeBytes.data();

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getType(), fileTypeData) == 0) {
			return i;
		}
	}
	return -1;
}

const ModVersionFile * ModVersion::getFile(int index) const {
	if(index < 0 || index >= m_files.size()) { return NULL; }

	return m_files[index];
}

const ModVersionFile * ModVersion::getFile(const char * fileName) const {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return NULL; }

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getName(), fileName) == 0) {
			return m_files[i];
		}
	}
	return NULL;
}

const ModVersionFile * ModVersion::getFile(const QString & fileName) const {
	if(fileName.isEmpty()) { return NULL; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getName(), fileNameData) == 0) {
			return m_files[i];
		}
	}
	return NULL;
}

const ModVersionFile * ModVersion::getFileByType(const char * fileType) const {
	if(fileType == NULL || Utilities::stringLength(fileType) == 0) { return NULL; }

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getType(), fileType) == 0) {
			return m_files[i];
		}
	}
	return NULL;
}

const ModVersionFile * ModVersion::getFileByType(const QString & fileType) const {
	if(fileType.isEmpty()) { return NULL; }
	QByteArray fileTypeBytes = fileType.toLocal8Bit();
	const char * fileTypeData = fileTypeBytes.data();

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getType(), fileTypeData) == 0) {
			return m_files[i];
		}
	}
	return NULL;
}

const char * ModVersion::getFileNameByType(const char * fileType) const {
	if(fileType == NULL || Utilities::stringLength(fileType) == 0) { return NULL; }

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getType(), fileType) == 0) {
			return m_files[i]->getName();
		}
	}
	return NULL;
}

const char * ModVersion::getFileNameByType(const QString & fileType) const {
	if(fileType.isEmpty()) { return NULL; }
	QByteArray fileTypeBytes = fileType.toLocal8Bit();
	const char * fileTypeData = fileTypeBytes.data();

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getType(), fileTypeData) == 0) {
			return m_files[i]->getName();
		}
	}
	return NULL;
}

bool ModVersion::addFile(ModVersionFile * file) {
	if(file == NULL || Utilities::stringLength(file->getName()) == 0 || hasFile(*file)) {
		return false;
	}
	
	m_files.push_back(file);

	return true;
}

bool ModVersion::removeFile(int index) {
	if(index < 0 || index >= m_files.size()) { return false; }
	
	delete m_files[index];
	m_files.remove(index);
	
	return true;
}

bool ModVersion::removeFile(const ModVersionFile & file) {
	for(int i=0;i<m_files.size();i++) {
		if(*m_files[i] == file) {
			delete m_files[i];
			m_files.remove(i);
			
			return true;
		}
	}
	return false;
}

bool ModVersion::removeFile(const char * fileName) {
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

bool ModVersion::removeFile(const QString & fileName) {
	if(fileName.isEmpty()) { return false; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getName(), fileNameData) == 0) {
			delete m_files[i];
			m_files.remove(i);

			return true;
		}
	}
	return false;
}

bool ModVersion::removeFileByType(const char * fileType) {
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

bool ModVersion::removeFileByType(const QString & fileType) {
	if(fileType.isEmpty()) { return false; }
	QByteArray fileTypeBytes = fileType.toLocal8Bit();
	const char * fileTypeData = fileTypeBytes.data();

	for(int i=0;i<m_files.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getType(), fileTypeData) == 0) {
			delete m_files[i];
			m_files.remove(i);

			return true;
		}
	}
	return false;
}

void ModVersion::clearFiles() {
	for(int i=0;i<m_files.size();i++) {
		delete m_files[i];
	}
	m_files.clear();
}

bool ModVersion::operator == (const ModVersion & m) const {
	if(m_files.size() != m.m_files.size() || Utilities::compareStringsIgnoreCase(m_version, m.m_version) != 0) {
		return false;
	}

	for(int i=0;i<m_files.size();i++) {
		if(!m.hasFile(*m_files[i])) {
			return false;
		}
	}
	return true;
}

bool ModVersion::operator != (const ModVersion & m) const {
	return !operator == (m);
}
