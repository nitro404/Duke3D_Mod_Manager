#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString.h>
#include <QDir.h>
#include <QFile.h>
#include <QFileInfo.h>
#include <QGlobal.h>
#include <QDate>
#include <QTime>

namespace Utilities {
	extern const char newLine[];
	extern const char * monthStrings[];

	int randomInteger(int min, int max, bool randomize = true);
	unsigned int stringLength(const char * s);
	bool copyString(char * destination, int size, const char * source);
	char * copyString(const char * data);
	char * trimCopy(const char * data);
	char * substring(const char * data, int start, int end);
	QString substring(const QString & data, int start, int end);
	int compareStrings(const char * s1, const char * s2, bool caseSensitive = true);
	int compareStrings(const char * s1, const QString & s2, bool caseSensitive = true);
	int compareStrings(const QString & s1, const char * s2, bool caseSensitive = true);
	int compareStringsIgnoreCase(const char * s1, const char * s2);
	int compareStringsIgnoreCase(const char * s1, const QString & s2);
	int compareStringsIgnoreCase(const QString & s1, const char * s2);
	int firstIndexOf(const char * data, char c);
	int lastIndexOf(const char * data, char c);
	char * getFileNameNoExtension(const char * fileName);
	char * getFileExtension(const char * fileName);
	bool fileHasExtension(const char * fileName, const char * fileExtension);
	QString getFileNameNoExtension(const QString & fileName);
	QString getFileExtension(const QString & fileName);
	bool fileHasExtension(const QString & fileName, const QString & fileExtension);
	QDate parseDate(const char * date);
	QDate parseDate(const QString & date);
	QString dateToString(const QDate & date);
	void deleteFiles(const char * suffix);
	void renameFiles(const char * fromSuffix, const char * toSuffix);
	void pause();
}

#endif // UTILITIES_H
