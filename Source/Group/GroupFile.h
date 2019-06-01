#ifndef GROUP_FILE_H
#define GROUP_FILE_H

#include "Utilities/Utilities.h"

class GroupFile {
public:
	GroupFile(const char * fileName, int fileSize);
	GroupFile(const QString & fileName, int fileSize);
	GroupFile(const GroupFile & g);
	GroupFile & operator = (const GroupFile & g);
	virtual ~GroupFile();
	
	const char * getFileName() const;
	int getFileSize() const;
	int getDataSize() const;
	const QByteArray & getData() const;
	
	void setFileName(const char * fileName);
	void setFileName(const QString & fileName);
	void setData(const char * data, int size);
	void setData(const QByteArray & data);
	void clearAllData();
	
	QString toString() const;
	
	bool isValid() const;
	static bool isValid(const GroupFile * g);
	
	bool writeTo(const char * directoryName, bool overwrite = DEFAULT_OVERWRITE_FILES, const char * alternateFileName = NULL) const;
	bool writeTo(const char * directoryName, bool overwrite, const QString & alternateFileName) const;
	bool writeTo(const QString & directoryName, bool overwrite, const char * alternateFileName) const;
	bool writeTo(const QString & directoryName, bool overwrite, const QString & alternateFileName) const;
	
	bool operator == (const GroupFile & g) const;
	bool operator != (const GroupFile & g) const;
	
public:
	static const int MAX_FILE_NAME_LENGTH;
	static const bool DEFAULT_OVERWRITE_FILES;
	
protected:
	char * m_fileName;
	int m_fileSize;
	QByteArray m_data;
};

#endif // GROUP_FILE_H
