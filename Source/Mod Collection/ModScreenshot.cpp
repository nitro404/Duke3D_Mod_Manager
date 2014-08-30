#include "Mod Collection/ModScreenshot.h"

ModScreenshot::ModScreenshot(const char * fileName, const char * thumbnail, const char * width, const char * height)
	: m_fileName(NULL)
	, m_thumbnail(NULL)
	, m_width(-1)
	, m_height(-1) {
	if(fileName == NULL) {
		m_fileName = new char[1];
		m_fileName[0] = '\0';
	}
	else {
		m_fileName = Utilities::trimCopyString(fileName);
	}

	if(thumbnail != NULL) {
		m_thumbnail = Utilities::trimCopyString(thumbnail);
	}

	setWidth(width);
	setHeight(height);
}

ModScreenshot::ModScreenshot(const char * fileName, const char * thumbnail, int width, int height)
	: m_fileName(NULL)
	, m_thumbnail(NULL)
	, m_width(width < -1 ? -1 : width)
	, m_height(height < -1 ? -1 : height) {
	if(fileName == NULL) {
		m_fileName = new char[1];
		m_fileName[0] = '\0';
	}
	else {
		m_fileName = Utilities::trimCopyString(fileName);
	}

	if(thumbnail != NULL) {
		m_thumbnail = Utilities::trimCopyString(thumbnail);
	}
}

ModScreenshot::ModScreenshot(const QString & fileName, const QString & thumbnail, const QString & width, const QString & height)
	: m_fileName(NULL)
	, m_thumbnail(NULL)
	, m_width(-1)
	, m_height(-1) {
	if(fileName.isEmpty()) {
		m_fileName = new char[1];
		m_fileName[0] = '\0';
	}
	else {
		QByteArray fileNameBytes = fileName.toLocal8Bit();
		const char * fileNameData = fileNameBytes.data();
		m_fileName = Utilities::trimCopyString(fileNameData);
	}

	if(!thumbnail.isEmpty()) {
		QByteArray thumbnailBytes = thumbnail.toLocal8Bit();
		const char * thumbnailData = thumbnailBytes.data();
		m_thumbnail = Utilities::trimCopyString(thumbnailData);
	}

	setWidth(width);
	setHeight(height);
}

ModScreenshot::ModScreenshot(const QString & fileName, const QString & thumbnail, int width, int height)
	: m_fileName(NULL)
	, m_thumbnail(NULL)
	, m_width(width < -1 ? -1 : width)
	, m_height(height < -1 ? -1 : height) {
	if(fileName.isEmpty()) {
		m_fileName = new char[1];
		m_fileName[0] = '\0';
	}
	else {
		QByteArray fileNameBytes = fileName.toLocal8Bit();
		const char * fileNameData = fileNameBytes.data();
		m_fileName = Utilities::trimCopyString(fileNameData);
	}

	if(!thumbnail.isEmpty()) {
		QByteArray thumbnailBytes = thumbnail.toLocal8Bit();
		const char * thumbnailData = thumbnailBytes.data();
		m_thumbnail = Utilities::trimCopyString(thumbnailData);
	}
}

ModScreenshot::ModScreenshot(const ModScreenshot & s)
	: m_fileName(NULL)
	, m_thumbnail(NULL)
	, m_width(s.m_width)
	, m_height(s.m_height) {
	m_fileName = Utilities::trimCopyString(s.m_fileName);
	m_thumbnail = Utilities::trimCopyString(s.m_thumbnail);
}

ModScreenshot & ModScreenshot::operator = (const ModScreenshot & s) {
	delete [] m_fileName;
	if(m_thumbnail != NULL) { delete [] m_thumbnail; }

	m_fileName = Utilities::trimCopyString(s.m_fileName);
	m_thumbnail = Utilities::trimCopyString(s.m_thumbnail);

	m_width = s.m_width;
	m_height = s.m_height;

	return *this;
}

ModScreenshot::~ModScreenshot() {
	delete [] m_fileName;
	if(m_thumbnail != NULL) { delete [] m_thumbnail; }
}

const char * ModScreenshot::getFileName() const {
	return m_fileName;
}

const char * ModScreenshot::getThumbnail() const {
	return m_thumbnail;
}

int ModScreenshot::getWidth() const {
	return m_width;
}

int ModScreenshot::getHeight() const {
	return m_height;
}

void ModScreenshot::setFileName(const char * fileName) {
	delete [] m_fileName;
	
	if(fileName == NULL) {
		m_fileName = new char[1];
		m_fileName[0] = '\0';
	}
	else {
		m_fileName = Utilities::trimCopyString(fileName);
	}
}

void ModScreenshot::setFileName(const QString & fileName) {
	delete [] m_fileName;

	if(fileName.isEmpty()) {
		m_fileName = new char[1];
		m_fileName[0] = '\0';
	}
	else {
		QByteArray fileNameBytes = fileName.toLocal8Bit();
		const char * fileNameData = fileNameBytes.data();
		m_fileName = Utilities::trimCopyString(fileNameData);
	}
}

void ModScreenshot::setThumbnail(const char * thumbnail) {
	if(m_thumbnail != NULL) {
		delete [] m_thumbnail;
		m_thumbnail = NULL;
	}
	
	if(thumbnail != NULL) {
		m_thumbnail = Utilities::trimCopyString(thumbnail);
	}
}

void ModScreenshot::setThumbnail(const QString & thumbnail) {
	if(m_thumbnail != NULL) {
		delete [] m_thumbnail;
		m_thumbnail = NULL;
	}

	if(!thumbnail.isNull()) {
		QByteArray thumbnailBytes = thumbnail.toLocal8Bit();
		const char * thumbnailData = thumbnailBytes.data();
		m_thumbnail = Utilities::trimCopyString(thumbnailData);
	}
}

void ModScreenshot::setWidth(int width) {
	m_width = width < -1 ? -1 : width;
}

void ModScreenshot::setWidth(const char * width) {
	if(width == NULL || Utilities::stringLength(width) == 0) { return; }
	bool valid = false;
	int value = QString(width).toInt(&valid, 10);
	if(valid) {
		setWidth(value);
	}
}

void ModScreenshot::setWidth(const QString & width) {
	if(width.isEmpty()) { return; }
	bool valid = false;
	int value = width.toInt(&valid, 10);
	if(valid) {
		setWidth(value);
	}
}

void ModScreenshot::setHeight(int height) {
	m_height = height < -1 ? -1 : height;
}

void ModScreenshot::setHeight(const char * height) {
	if(height == NULL || Utilities::stringLength(height) == 0) { return; }
	bool valid = false;
	int value = QString(height).toInt(&valid, 10);
	if(valid) {
		setHeight(value);
	}
}

void ModScreenshot::setHeight(const QString & height) {
	if(height.isEmpty()) { return; }
	bool valid = false;
	int value = height.toInt(&valid, 10);
	if(valid) {
		setHeight(value);
	}
}

bool ModScreenshot::operator == (const ModScreenshot & s) const {
	return Utilities::compareStringsIgnoreCase(m_fileName, s.m_fileName) == 0;
}

bool ModScreenshot::operator != (const ModScreenshot & s) const {
	return !operator == (s);
}
