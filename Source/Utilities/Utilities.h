#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString.h>
#include <QRegExp.h>
#include <QDir.h>
#include <QFile.h>
#include <QFileInfo.h>
#include <QGlobal.h>
#include <QDate>
#include <QTime>

#define USE_QT 1

namespace Utilities {
	extern const char newLine[];
	extern bool initialRandomize;
	extern const char * monthStrings[];

	void randomizeSeed();
	void randomSeed(unsigned int seed);
	int randomInteger(int min, int max, bool randomize = false);
	float randomFloat(float min, float max, bool randomize = false);
	int intLength(int n);
	unsigned int stringLength(const char * s);
	const char * toString(int value);
	const char * toString(double value);
	bool copyString(char * destination, int size, const char * source);
	char * copyString(const char * data);
	char * trimCopyString(const char * data);
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
	QString getCommand(const char * data);
	QString getCommand(const QString & data);
	QString getArguments(const char * data);
	QString getArguments(const QString & data);
	int parseBoolean(const char * data);
	int parseBoolean(const QString & data);
	bool parseBoolean(const char * data, bool & boolean);
	bool parseBoolean(const QString & data, bool & boolean);
	QString generateFullPath(const char * path, const char * fileName);
	QString generateFullPath(const QString & path, const QString & fileName);
	QDate parseDate(const char * date);
	QDate parseDate(const QString & date);
	QString dateToString(const QDate & date);
	char * getFileNameNoExtension(const char * fileName);
	char * getFileExtension(const char * fileName);
	bool fileHasExtension(const char * fileName, const char * fileExtension);
	QString getFileNameNoExtension(const QString & fileName);
	QString getFileExtension(const QString & fileName);
	bool fileHasExtension(const QString & fileName, const QString & fileExtension);
	void deleteFiles(const char * suffix);
	void renameFiles(const char * fromSuffix, const char * toSuffix);
	void clear();
	void pause();
}

#endif // UTILITIES_H
