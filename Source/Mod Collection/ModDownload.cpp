#include "Mod Collection/ModDownload.h"

ModDownload::ModDownload(const char * fileName, const char * type)
	: m_fileName(NULL)
	, m_partNumber(1)
	, m_partCount(1)
	, m_version(NULL)
	, m_special(NULL)
	, m_subfolder(NULL)
	, m_type(NULL)
	, m_description(NULL) {
	if(fileName == NULL) {
		m_fileName = new char[1];
		m_fileName[0] = '\0';
	}
	else {
		m_fileName = Utilities::trimCopyString(fileName);
	}

	if(type == NULL) {
		m_type = new char[1];
		m_type[0] = '\0';
	}
	else {
		m_type = Utilities::trimCopyString(type);
	}
}

ModDownload::ModDownload(const QString & fileName, const QString & type)
	: m_fileName(NULL)
	, m_partNumber(1)
	, m_partCount(1)
	, m_version(NULL)
	, m_special(NULL)
	, m_subfolder(NULL)
	, m_type(NULL)
	, m_description(NULL) {
	if(fileName.isEmpty()) {
		m_fileName = new char[1];
		m_fileName[0] = '\0';
	}
	else {
		QByteArray fileNameBytes = fileName.toLocal8Bit();
		m_fileName = Utilities::trimCopyString(fileNameBytes.data());
	}

	if(type.isEmpty()) {
		m_type = new char[1];
		m_type[0] = '\0';
	}
	else {
		QByteArray typeBytes = type.toLocal8Bit();
		m_type = Utilities::trimCopyString(typeBytes.data());
	}
}

ModDownload::ModDownload(const ModDownload & d)
	: m_fileName(NULL)
	, m_partNumber(d.m_partNumber)
	, m_partCount(d.m_partCount)
	, m_version(NULL)
	, m_special(NULL)
	, m_subfolder(NULL)
	, m_type(NULL)
	, m_description(NULL) {
	m_fileName = Utilities::trimCopyString(d.m_fileName);
	m_version = Utilities::trimCopyString(d.m_version);
	m_special = Utilities::trimCopyString(d.m_special);
	m_subfolder = Utilities::trimCopyString(d.m_subfolder);
	m_type = Utilities::trimCopyString(d.m_type);
	m_description = Utilities::trimCopyString(d.m_description);
}

ModDownload & ModDownload::operator = (const ModDownload & d) {
	delete [] m_fileName;
	if(m_version != NULL)     { delete [] m_version; }
	if(m_special != NULL)     { delete [] m_special; }
	if(m_subfolder != NULL)   { delete [] m_subfolder; }
	delete [] m_type;
	if(m_description != NULL) { delete [] m_description; }

	m_fileName = Utilities::trimCopyString(d.m_fileName);
	m_partNumber = d.m_partNumber;
	m_partCount = d.m_partCount;
	m_version = Utilities::trimCopyString(d.m_version);
	m_special = Utilities::trimCopyString(d.m_special);
	m_subfolder = Utilities::trimCopyString(d.m_subfolder);
	m_type = Utilities::trimCopyString(d.m_type);
	m_description = Utilities::trimCopyString(d.m_description);

	return *this;
}

ModDownload::~ModDownload(void) {
	delete [] m_fileName;
	if(m_version != NULL)     { delete [] m_version; }
	if(m_special != NULL)     { delete [] m_special; }
	if(m_subfolder != NULL)   { delete [] m_subfolder; }
	delete [] m_type;
	if(m_description != NULL) { delete [] m_description; }
}

const char * ModDownload::getFileName() const {
	return m_fileName;
}

int ModDownload::getPartNumber() const {
	return m_partNumber;
}

int ModDownload::getPartCount() const {
	return m_partCount;
}

const char * ModDownload::getVersion() const {
	return m_version;
}

const char * ModDownload::getSpecial() const {
	return m_special;
}

const char * ModDownload::getSubfolder() const {
	return m_subfolder;
}

const char * ModDownload::getType() const {
	return m_type;
}

const char * ModDownload::getDescription() const {
	return m_description;
}

void ModDownload::setFileName(const char * fileName) {
	delete [] m_fileName;
	
	if(fileName == NULL) {
		m_fileName = new char[1];
		m_fileName[0] = '\0';
	}
	else {
		m_fileName = Utilities::trimCopyString(fileName);
	}
}

void ModDownload::setFileName(const QString & fileName) {
	delete [] m_fileName;

	if(fileName.isEmpty()) {
		m_fileName = new char[1];
		m_fileName[0] = '\0';
	}
	else {
		QByteArray fileNameBytes = fileName.toLocal8Bit();
		m_fileName = Utilities::trimCopyString(fileNameBytes.data());
	}
}

void ModDownload::setPartNumber(int partNumber) {
	if(partNumber < 1) { return; }
	m_partNumber = partNumber;
}

void ModDownload::setPartNumber(const char * partNumber) {
	if(partNumber == NULL || Utilities::stringLength(partNumber) == 0) { return; }
	bool valid = false;
	int value = QString(partNumber).toInt(&valid, 10);
	if(valid) {
		setPartNumber(value);
	}
}

void ModDownload::setPartNumber(const QString & partNumber) {
	if(partNumber.isEmpty()) { return; }
	bool valid = false;
	int value = partNumber.toInt(&valid, 10);
	if(valid) {
		setPartNumber(value);
	}
}

void ModDownload::setPartCount(int partCount) {
	if(partCount < 1) { return; }
	m_partCount = partCount;
}

void ModDownload::setPartCount(const char * partCount) {
	if(partCount == NULL || Utilities::stringLength(partCount) == 0) { return; }
	bool valid = false;
	int value = QString(partCount).toInt(&valid, 10);
	if(valid) {
		setPartCount(value);
	}
}

void ModDownload::setPartCount(const QString & partCount) {
	if(partCount.isEmpty()) { return; }
	bool valid = false;
	int value = partCount.toInt(&valid, 10);
	if(valid) {
		setPartCount(value);
	}
}

void ModDownload::setVersion(const char * version) {
	if(m_version != NULL) {
		delete [] m_version;
		m_version = NULL;
	}
	
	if(version != NULL) {
		m_version = Utilities::trimCopyString(version);
	}
}

void ModDownload::setVersion(const QString & version) {
	if(m_version != NULL) {
		delete [] m_version;
		m_version = NULL;
	}

	if(!version.isNull()) {
		QByteArray versionBytes = version.toLocal8Bit();
		m_version = Utilities::trimCopyString(versionBytes.data());
	}
}

void ModDownload::setSpecial(const char * special) {
	if(m_special != NULL) {
		delete [] m_special;
		m_special = NULL;
	}
	
	if(special != NULL) {
		m_special = Utilities::trimCopyString(special);
	}
}

void ModDownload::setSpecial(const QString & special) {
	if(m_special != NULL) {
		delete [] m_special;
		m_special = NULL;
	}

	if(!special.isNull()) {
		QByteArray specialBytes = special.toLocal8Bit();
		m_special = Utilities::trimCopyString(specialBytes.data());
	}
}

void ModDownload::setSubfolder(const char * subfolder) {
	if(m_subfolder != NULL) {
		delete [] m_subfolder;
		m_subfolder = NULL;
	}
	
	if(subfolder != NULL) {
		m_subfolder = Utilities::trimCopyString(subfolder);
	}
}

void ModDownload::setSubfolder(const QString & subfolder) {
	if(m_subfolder != NULL) {
		delete [] m_subfolder;
		m_subfolder = NULL;
	}

	if(!subfolder.isNull()) {
		QByteArray subfolderBytes = subfolder.toLocal8Bit();
		m_subfolder = Utilities::trimCopyString(subfolderBytes.data());
	}
}

void ModDownload::setType(const char * type) {
	delete [] m_type;
	
	if(type == NULL) {
		m_type = new char[1];
		m_type[0] = '\0';
	}
	else {
		m_type = Utilities::trimCopyString(type);
	}
}

void ModDownload::setType(const QString & type) {
	delete [] m_type;

	if(type.isEmpty()) {
		m_type = new char[1];
		m_type[0] = '\0';
	}
	else {
		QByteArray typeBytes = type.toLocal8Bit();
		m_type = Utilities::trimCopyString(typeBytes.data());
	}
}

void ModDownload::setDescription(const char * description) {
	if(m_description != NULL) {
		delete [] m_description;
		m_description = NULL;
	}
	
	if(description != NULL) {
		m_description = Utilities::trimCopyString(description);
	}
}

void ModDownload::setDescription(const QString & description) {
	if(m_description != NULL) {
		delete [] m_description;
		m_description = NULL;
	}

	if(!description.isNull()) {
		QByteArray descriptionBytes = description.toLocal8Bit();
		m_description = Utilities::trimCopyString(descriptionBytes.data());
	}
}

bool ModDownload::operator == (const ModDownload & d) const {
	return Utilities::compareStringsIgnoreCase(m_fileName, d.m_fileName) == 0;
}

bool ModDownload::operator != (const ModDownload & d) const {
	return !operator == (d);
}
