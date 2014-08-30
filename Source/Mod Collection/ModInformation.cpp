#include "Mod Collection/ModInformation.h"

ModInformation::ModInformation(const char * name, const char * version)
	: m_name(NULL)
	, m_version(NULL) {
	if(name == NULL) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		m_name = Utilities::trimCopyString(name);
	}

	if(version != NULL && Utilities::stringLength(version) > 0) {
		m_version = Utilities::trimCopyString(version);
	}
}

ModInformation::ModInformation(const QString & name, const QString & version)
	: m_name(NULL)
	, m_version(NULL) {
	if(name.isEmpty()) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		QByteArray nameBytes = name.toLocal8Bit();
		m_name = Utilities::trimCopyString(nameBytes.data());
	}

	if(!version.isEmpty()) {
		QByteArray versionBytes = version.toLocal8Bit();
		m_version = Utilities::trimCopyString(versionBytes.data());
	}
}

ModInformation::ModInformation(const ModInformation & m)
	: m_name(NULL)
	, m_version(NULL) {
	m_name = Utilities::trimCopyString(m.m_name);

	if(m.m_version != NULL) {
		m_version = Utilities::trimCopyString(m.m_version);
	}
}

ModInformation & ModInformation::operator = (const ModInformation & m) {
	delete [] m_name;
	if(m_version != NULL) { delete [] m_version; }

	m_name = Utilities::trimCopyString(m.m_name);
	
	if(m.m_version != NULL) {
		m_version = Utilities::trimCopyString(m.m_version);
	}
	
	return *this;
}

ModInformation::~ModInformation() {
	delete [] m_name;
	if(m_version != NULL) { delete [] m_version; }
}

const char * ModInformation::getName() const {
	return m_name;
}

const QString ModInformation::getFullName() const {
	return QString("%1%2%3").arg(m_name).arg(m_version == NULL ? "" : " ").arg(m_version == NULL ? "" : m_version);
}

const char * ModInformation::getVersion() const {
	return m_version;
}

void ModInformation::setName(const char * name) {
	if(m_name != NULL) {
		delete [] m_name;
	}
	
	if(name == NULL) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		m_name = Utilities::trimCopyString(name);
	}
}

void ModInformation::setName(const QString & name) {
	delete [] m_name;

	if(name.isEmpty()) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		QByteArray nameBytes = name.toLocal8Bit();
		m_name = Utilities::trimCopyString(nameBytes.data());
	}
}

void ModInformation::setVersion(const char * version) {
	if(version != NULL) {
		delete [] m_version;
		m_version = NULL;
	}
	
	if(version != NULL && Utilities::stringLength(version) > 0) {
		m_version = Utilities::trimCopyString(version);
	}
}

void ModInformation::setVersion(const QString & version) {
	if(m_version != NULL) {
		delete [] m_version;
		m_version = NULL;
	}

	if(!version.isEmpty()) {
		QByteArray versionBytes = version.toLocal8Bit();
		m_version = Utilities::trimCopyString(versionBytes.data());
	}
}

bool ModInformation::operator == (const ModInformation & m) const {
	if(Utilities::compareStringsIgnoreCase(m_name, m.m_name) != 0) {
		return false;
	}

	if(m_version == NULL && m.m_version == NULL) {
		return true;
	}
	else if((m_version == NULL && m.m_version != NULL) ||
		    (m_version != NULL && m.m_version == NULL)) {
		return false;
	}

	return Utilities::compareStringsIgnoreCase(m_version, m.m_version) == 0;
}

bool ModInformation::operator != (const ModInformation & m) const {
	return !operator == (m);
}
