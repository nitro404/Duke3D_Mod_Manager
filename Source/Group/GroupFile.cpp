#include <QDataStream.h>
#include "Group/GroupFile.h"

const int GroupFile::MAX_FILE_NAME_LENGTH = 12;
const bool GroupFile::DEFAULT_OVERWRITE_FILES = false;

GroupFile::GroupFile(const char * fileName, int fileSize)
	: m_fileName(NULL)
	, m_fileSize(fileSize < 0 ? 0 : fileSize)
	, m_data() {
	setFileName(fileName);
}

GroupFile::GroupFile(const QString & fileName, int fileSize)
	: m_fileName(NULL)
	, m_fileSize(fileSize < 0 ? 0 : fileSize)
	, m_data() {
	setFileName(fileName);
}

GroupFile::GroupFile(const GroupFile & g)
	: m_fileName(NULL)
	, m_fileSize(g.m_fileSize)
	, m_data() {
	setFileName(g.m_fileName);
	setData(g.m_data);
}

GroupFile & GroupFile::operator = (const GroupFile & g) {
	if(m_fileName != NULL) {
		delete [] m_fileName;
		m_fileName = NULL;
	}
	
	m_data.clear();
	
	m_fileSize = g.m_fileSize;
	
	setFileName(g.m_fileName);
	setData(g.m_data);
	
	return *this;
}

GroupFile::~GroupFile() {
	if(m_fileName != NULL) { delete [] m_fileName; }
}

const char * GroupFile::getFileName() const {
	return m_fileName;
}

int GroupFile::getFileSize() const {
	return m_fileSize;
}

int GroupFile::getDataSize() const {
	return m_data.size();
}

const QByteArray & GroupFile::getData() const {
	return m_data;
}

void GroupFile::setFileName(const char * fileName) {
	if(m_fileName != NULL) {
		delete [] m_fileName;
		m_fileName = NULL;
	}
	
	if(fileName != NULL) {
		m_fileName = Utilities::trimCopyString(fileName);
	}
}

void GroupFile::setFileName(const QString & fileName) {
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	setFileName(fileNameBytes.data());
}

void GroupFile::setData(const char * data, int size) {
	if(size < 0) { return; }
	
	m_data.clear();
	
	if(data != NULL) {
		m_data.append(data, size);
	}
}

void GroupFile::setData(const QByteArray & data) {
	m_data = data;
}

void GroupFile::clearAllData() {
	m_data.clear();
}

QString GroupFile::toString() const {
	return QString("%1 (%2 bytes)").arg(m_data.size());
}

bool GroupFile::isValid() const {
	const int fileNameLength = Utilities::stringLength(m_fileName);
	
	return fileNameLength > 0 &&
	       fileNameLength < MAX_FILE_NAME_LENGTH;
}

bool GroupFile::isValid(const GroupFile * g) {
	return g != NULL && g->isValid();
}

bool GroupFile::writeTo(const char * directoryName, bool overwrite, const char * alternateFileName) const {
	if(!isValid()) { return false; }

	QString filePath = Utilities::generateFullPath(directoryName, alternateFileName == NULL ? m_fileName : alternateFileName);
	
	QFileInfo fileInfo(filePath);

	if(fileInfo.exists() && !overwrite) {
		return false;
	}

	QFile file(filePath);

	if(!file.open(QIODevice::WriteOnly)) {
		return false;
	}
	
	QDataStream out(&file);

	out.writeRawData(m_data.data(), m_data.size());

	file.close();
	
	return true;
}

bool GroupFile::writeTo(const char * directoryName, bool overwrite, const QString & alternateFileName) const {
	QByteArray alternateFileNameBytes = alternateFileName.toLocal8Bit();

	return writeTo(directoryName, alternateFileNameBytes.data());
}

bool GroupFile::writeTo(const QString & directoryName, bool overwrite, const char * alternateFileName) const {
	QByteArray directoryNameBytes = directoryName.toLocal8Bit();

	return writeTo(directoryNameBytes.data(), alternateFileName);
}

bool GroupFile::writeTo(const QString & directoryName, bool overwrite, const QString & alternateFileName) const {
	QByteArray directoryNameBytes = directoryName.toLocal8Bit();
	QByteArray alternateFileNameBytes = alternateFileName.toLocal8Bit();

	return writeTo(directoryNameBytes.data(), alternateFileNameBytes.data());
}

bool GroupFile::operator == (const GroupFile & g) const {
	return Utilities::compareStringsIgnoreCase(m_fileName, g.m_fileName) == 0;
}

bool GroupFile::operator != (const GroupFile & g) const {
	return !operator == (g);
}
