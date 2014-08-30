#include "Mod Collection/ModVersionFile.h"

ModVersionFile::ModVersionFile(const char * name, const char * type)
	: m_name(NULL)
	, m_type(NULL) {
	if(name == NULL) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		m_name = Utilities::trimCopyString(name);
	}

	if(type == NULL) {
		m_type = Utilities::getFileExtension(m_name);
	}
	else {
		m_type = Utilities::trimCopyString(type);
	}
}

ModVersionFile::ModVersionFile(const QString & name, const QString & type)
	: m_name(NULL)
	, m_type(NULL) {
	if(name.isEmpty()) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		QByteArray nameBytes = name.toLocal8Bit();
		const char * nameData = nameBytes.data();
		m_name = Utilities::trimCopyString(nameData);
	}

	if(type.isNull()) {
		m_type = Utilities::getFileExtension(m_name);
	}
	else {
		QByteArray typeBytes = type.toLocal8Bit();
		const char * typeData = typeBytes.data();
		m_type = Utilities::trimCopyString(typeData);
	}
}

ModVersionFile::ModVersionFile(const ModVersionFile & f)
	: m_name(NULL)
	, m_type(NULL) {
	m_name = Utilities::trimCopyString(f.m_name);

	if(f.m_type != NULL) {
		m_type = Utilities::trimCopyString(f.m_type);
	}
}

ModVersionFile & ModVersionFile::operator = (const ModVersionFile & f) {
	delete [] m_name;

	if(m_type != NULL) {
		delete [] m_type;
		m_type = NULL;
	}

	m_name = Utilities::trimCopyString(f.m_name);

	if(f.m_type != NULL) {
		m_type = Utilities::trimCopyString(f.m_type);
	}

	return *this;
}

ModVersionFile::~ModVersionFile(void) {
	delete [] m_name;

	if(m_type != NULL) {
		delete [] m_type;
	}
}

const char * ModVersionFile::getName() const {
	return m_name;
}

const char * ModVersionFile::getType() const {
	return m_type;
}

void ModVersionFile::setName(const char * name) {
	delete [] m_name;
	
	if(name == NULL) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		m_name = Utilities::trimCopyString(name);
	}

	if(m_type == NULL) {
		m_type = Utilities::getFileExtension(m_name);
	}
}

void ModVersionFile::setName(const QString & name) {
	delete [] m_name;

	if(name.isEmpty()) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		QByteArray nameBytes = name.toLocal8Bit();
		const char * nameData = nameBytes.data();
		m_name = Utilities::trimCopyString(nameData);
	}

	if(m_type == NULL) {
		m_type = Utilities::getFileExtension(m_name);
	}
}

void ModVersionFile::setType(const char * type) {
	if(m_type != NULL) {
		delete [] m_type;
		m_type = NULL;
	}
	
	if(type != NULL) {
		m_type = Utilities::trimCopyString(type);
	}
}

void ModVersionFile::setType(const QString & type) {
	if(m_type != NULL) {
		delete [] m_type;
		m_type = NULL;
	}

	if(!type.isNull()) {
		QByteArray typeBytes = type.toLocal8Bit();
		const char * typeData = typeBytes.data();
		m_type = Utilities::trimCopyString(typeData);
	}
}

bool ModVersionFile::operator == (const ModVersionFile & f) const {
	return Utilities::compareStringsIgnoreCase(m_name, f.m_name) == 0;
}

bool ModVersionFile::operator != (const ModVersionFile & f) const {
	return !operator == (f);
}
