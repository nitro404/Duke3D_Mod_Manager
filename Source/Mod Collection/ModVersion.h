#ifndef MOD_VERSION_H
#define MOD_VERSION_H

#include <QString.h>
#include <QVector.h>
#include "Utilities/Utilities.h"
#include "Mod Collection/ModVersionFile.h"

class ModVersion {
public:
	ModVersion(const char * version = NULL);
	ModVersion(const QString & version);
	ModVersion(const ModVersion & m);
	ModVersion & operator = (const ModVersion & m);
	~ModVersion(void);

	const char * getVersion() const;

	void setVersion(const char * version);
	void setVersion(const QString & version);

	int numberOfFiles() const;
	bool hasFile(const ModVersionFile & file) const;
	bool hasFile(const char * fileName) const;
	bool hasFile(const QString & fileName) const;
	bool hasFileOfType(const char * fileType) const;
	bool hasFileOfType(const QString & fileType) const;
	int indexOfFile(const ModVersionFile & file) const;
	int indexOfFile(const char * fileName) const;
	int indexOfFile(const QString & fileName) const;
	int indexOfFileByType(const char * fileType) const;
	int indexOfFileByType(const QString & fileType) const;
	const ModVersionFile * getFile(int index) const;
	const ModVersionFile * getFile(const char * fileName) const;
	const ModVersionFile * getFile(const QString & fileName) const;
	const ModVersionFile * getFileByType(const char * fileType) const;
	const ModVersionFile * getFileByType(const QString & fileType) const;
	const char * getFileNameByType(const char * fileType) const;
	const char * getFileNameByType(const QString & fileType) const;
	bool addFile(ModVersionFile * file);
	bool removeFile(int index);
	bool removeFile(const ModVersionFile & file);
	bool removeFile(const char * fileName);
	bool removeFile(const QString & fileName);
	bool removeFileByType(const char * fileType);
	bool removeFileByType(const QString & fileType);
	void clearFiles();

	bool operator == (const ModVersion & m) const;
	bool operator != (const ModVersion & m) const;

private:
	char * m_version;
	QVector<ModVersionFile *> m_files;
};

#endif // MOD_VERSION_H
