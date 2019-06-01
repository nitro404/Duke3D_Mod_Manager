#ifndef GROUP_H
#define GROUP_H

#include <QVector.h>
#include "Group/GroupFile.h"
#include "Utilities/Endian.h"

class Group {
public:
	Group(const char * fileName = NULL);
	Group(const QString & fileName);
	Group(const Group & g);
	Group & operator = (const Group & g);
	virtual ~Group();
	
	const char * getFilePath() const;
	QString getFileName() const;
	const char * getFileExtension() const;
	void setFilePath(const char * filePath);
	void setFilePath(const QString & filePath);
	
	int numberOfFiles() const;
	bool hasFile(const char * fileName) const;
	bool hasFile(const QString & fileName) const;
	bool hasFile(const GroupFile * groupFile) const;
	int indexOfFile(const char * fileName) const;
	int indexOfFile(const QString & fileName) const;
	int indexOfFile(const GroupFile * groupFile) const;
	const GroupFile * getFile(int index) const;
	const GroupFile * getFile(const char * fileName) const;
	const GroupFile * getFile(const QString & fileName) const;
	QVector<GroupFile *> getFilesWithExtension(const char * extension) const;
	QVector<GroupFile *> getFilesWithExtension(const QString & extension) const;
	QVector<QString> getFileExtensions() const;
	bool extractFile(int index, const char * directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	bool extractFile(int index, const QString & directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	bool extractFile(const char * fileName, const char * directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	bool extractFile(const QString & fileName, const char * directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	bool extractFile(const char * fileName, const QString & directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	bool extractFile(const QString & fileName, const QString & directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	int extractAllFilesWithExtension(const char * extension, const char * directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	int extractAllFilesWithExtension(const  QString & extension, const char * directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	int extractAllFilesWithExtension(const char * extension, const QString & directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	int extractAllFilesWithExtension(const QString & extension, const QString & directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	int extractAllFiles(const char * directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	int extractAllFiles(const QString & directory, bool overwrite = GroupFile::DEFAULT_OVERWRITE_FILES) const;
	bool addFile(const GroupFile * groupFile, bool replace = DEFAULT_REPLACE_FILES);
	bool addFiles(const QVector<GroupFile *> & files, bool replace = DEFAULT_REPLACE_FILES);
	bool addFiles(const Group * group, bool replace = DEFAULT_REPLACE_FILES);
	bool removeFile(int index);
	bool removeFile(const char * fileName);
	bool removeFile(const QString & fileName);
	bool removeFile(const GroupFile * groupFile);
	int removeFiles(const QVector<QString> & fileNames);
	int removeFiles(const QVector<const GroupFile *> & files);
	void clearFiles();
	
	QString toString() const;
	
	bool load();
	
	int getGroupFileSize() const;
	bool verifyAllFiles() const;
	
	bool operator == (const Group & g) const;
	bool operator != (const Group & g) const;
	
public:
	static const bool DEFAULT_REPLACE_FILES;

	static const Endian::Endianness FILE_ENDIANNESS;
	
	static const char * HEADER_TEXT;
	
	static const int HEADER_TEXT_LENGTH;
	static const int NUMBER_OF_FILES_LENGTH;
	static const int GROUP_FILE_NAME_LENGTH;
	static const int GROUP_FILE_SIZE_LENGTH;
	
	static const int HEADER_LENGTH;
	static const int GROUP_FILE_HEADER_LENGTH;
	
	static const char GROUP_HEADER_TEXT_DATA[];
	
protected:
	char * m_filePath;
	
	QVector<GroupFile *> m_files;
};

#endif // GROUP_H
