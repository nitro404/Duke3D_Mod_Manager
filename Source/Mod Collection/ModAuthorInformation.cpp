#include "Mod Collection/ModAuthorInformation.h"

ModAuthorInformation::ModAuthorInformation(const char * name)
	: m_name(NULL)
	, m_numberOfMods(1) {
	if(name == NULL) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		m_name = Utilities::trimCopyString(name);
	}
}

ModAuthorInformation::ModAuthorInformation(const QString & name)
	: m_name(NULL)
	, m_numberOfMods(1) {
	if(name.isEmpty()) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		QByteArray nameBytes = name.toLocal8Bit();
		m_name = Utilities::trimCopyString(nameBytes.data());
	}
}

ModAuthorInformation::ModAuthorInformation(const ModAuthorInformation & m)
	: m_name(NULL)
	, m_numberOfMods(m.m_numberOfMods) {
	m_name = Utilities::trimCopyString(m.m_name);
}

ModAuthorInformation & ModAuthorInformation::operator = (const ModAuthorInformation & m) {
	delete [] m_name;

	m_name = Utilities::trimCopyString(m.m_name);

	m_numberOfMods = m.m_numberOfMods;

	return *this;
}

ModAuthorInformation::~ModAuthorInformation() {
	delete [] m_name;
}

const char * ModAuthorInformation::getName() const {
	return m_name;
}

void ModAuthorInformation::setName(const char * name) {
	delete [] m_name;
	
	if(name == NULL) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		m_name = Utilities::trimCopyString(name);
	}
}

void ModAuthorInformation::setName(const QString & name) {
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

int ModAuthorInformation::getModCount() const {
	return m_numberOfMods;
}

int ModAuthorInformation::incrementModCount() {
	return m_numberOfMods++;
}

void ModAuthorInformation::setModCount(int numberOfMods) {
	if(numberOfMods < 0) { return; }

	m_numberOfMods = numberOfMods;
}

void ModAuthorInformation::resetModCount() {
	m_numberOfMods = 0;
}

bool ModAuthorInformation::operator == (const ModAuthorInformation & m) const {
	return Utilities::compareStringsIgnoreCase(m_name, m.m_name) == 0;
}

bool ModAuthorInformation::operator != (const ModAuthorInformation & m) const {
	return !operator == (m);
}
