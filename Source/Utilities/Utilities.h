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
#if USE_STL
#include <string>
#include <regex>
#include <iostream>
#endif // USE_STL

#define USE_QT 1

#define byte char

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
#if USE_STL
	std::string trimString(const std::string & data);
#endif // USE_STL
	char * substring(const char * data, int start, int end);
	QString substring(const QString & data, int start, int end);
#if USE_STL
	std::string substring(const std::string & data, int start, int end);
#endif // USE_STL
	int compareStrings(const char * s1, const char * s2, bool caseSensitive = true);
	int compareStrings(const QString & s1, const QString & s2, bool caseSensitive = true);
	int compareStrings(const char * s1, const QString & s2, bool caseSensitive = true);
	int compareStrings(const QString & s1, const char * s2, bool caseSensitive = true);
#if USE_STL
	int compareStrings(const std::string & s1, const std::string & s2, bool caseSensitive = true);
	int compareStrings(const char * s1, const std::string & s2, bool caseSensitive = true);
	int compareStrings(const std::string & s1, const char * s2, bool caseSensitive = true);
#if USE_QT
	int compareStrings(const QString & s1, const std::string & s2, bool caseSensitive = true);
	int compareStrings(const std::string & s1, const QString & s2, bool caseSensitive = true);
#endif // USE_QT
#endif // USE_STL
	int compareStringsIgnoreCase(const char * s1, const char * s2);
	int compareStringsIgnoreCase(const QString & s1, const QString & s2);
	int compareStringsIgnoreCase(const char * s1, const QString & s2);
	int compareStringsIgnoreCase(const QString & s1, const char * s2);
#if USE_STL
	int compareStringsIgnoreCase(const std::string & s1, const std::string & s2);
	int compareStringsIgnoreCase(const char * s1, const std::string & s2);
	int compareStringsIgnoreCase(const std::string & s1, const char * s2);
#if USE_QT
	int compareStringsIgnoreCase(const QString & s1, const std::string & s2);
	int compareStringsIgnoreCase(const std::string & s1, const QString & s2);
#endif // USE_QT
#endif // USE_STL
	int firstIndexOf(const char * data, char c);
	int lastIndexOf(const char * data, char c);
	QString getVariableID(const char * data);
	QString getVariableID(const QString & data);
	QString getVariableValue(const char * data);
	QString getVariableValue(const QString & data);
	QString getCommand(const char * data);
	QString getCommand(const QString & data);
	QString getArguments(const char * data);
	QString getArguments(const QString & data);
	bool parseBoolean(const char * data, bool * valid = NULL);
	bool parseBoolean(const QString & data, bool * valid = NULL);
#if USE_STL
	bool parseBoolean(const std::string & data, bool * valid = NULL);
#endif // USE_STL
	int parseInteger(const char * data, bool * valid = NULL);
	int parseInteger(const QString & data, bool * valid = NULL);
#if USE_STL
	int parseInteger(const std::string & data, bool * valid = NULL);
#endif // USE_STL
	float parseFloat(const char * data, bool * valid = NULL);
	float parseFloat(const QString & data, bool * valid = NULL);
#if USE_STL
	float parseFloat(const std::string & data, bool * valid = NULL);
#endif // USE_STL
	QDate parseDate(const char * date);
	QDate parseDate(const QString & date);
	QString dateToString(const QDate & date);
	QString generateFullPath(const char * path, const char * fileName);
	QString generateFullPath(const QString & path, const QString & fileName);
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
