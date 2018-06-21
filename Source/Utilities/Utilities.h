#ifndef UTILITIES_H
#define UTILITIES_H

#ifndef USE_QT
#define USE_QT 1
#endif // USE_QT

#ifndef USE_STL
#define USE_STL 0
#endif // USE_STL

#if USE_QT
#include <QString.h>
#include <QRegExp.h>
#include <QDir.h>
#include <QFile.h>
#include <QFileInfo.h>
#include <QGlobal.h>
#include <QDate>
#include <QTime>
#endif // USE_QT

#if USE_STL
#include <string>
#include <regex>
#include <iostream>
#endif // USE_STL

#if __APPLE__
#include "TargetConditionals.h"
#endif // __APPLE__

namespace Utilities {

	extern const char newLine[];
	extern bool initialRandomize;
	extern const char * monthStrings[];

	void randomizeSeed();
	void randomSeed(unsigned int seed);
	int randomInteger(int min, int max, bool randomize = false);
	float randomFloat(float min, float max, bool randomize = false);
	int byteLength(char n);
	int unsignedByteLength(unsigned char n);
	int shortLength(short n);
	int unsignedShortLength(unsigned short n);
	int intLength(int n);
	int unsignedIntLength(int n);
	int longLength(long n);
	int unsignedLongLength(unsigned long n);
	int longLongLength(long long n);
	int unsignedLongLongLength(unsigned long long n);
	unsigned int stringLength(const char * s);
	const char * toString(int value);
	const char * toString(double value);
	bool copyString(char * destination, int size, const char * source);
	char * copyString(const char * data);
	char * trimCopyString(const char * data);
#if USE_STL
	std::string trimString(const std::string & data, bool trimWhiteSpace = true, bool trimNewLines = true);
#endif // USE_STL
	char * substring(const char * data, int start, int end);
#if USE_QT
	QString substring(const QString & data, int start, int end);
#endif // USE_QT
#if USE_STL
	std::string substring(const std::string & data, int start, int end);
#endif // USE_STL
	int firstIndexOf(const char * data, char c);
	int lastIndexOf(const char * data, char c);
	int compareStrings(const char * s1, const char * s2, bool caseSensitive = true);
#if USE_QT
	int compareStrings(const QString & s1, const QString & s2, bool caseSensitive = true);
	int compareStrings(const char * s1, const QString & s2, bool caseSensitive = true);
	int compareStrings(const QString & s1, const char * s2, bool caseSensitive = true);
#endif // USE_QT
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
#if USE_QT
	int compareStringsIgnoreCase(const QString & s1, const QString & s2);
	int compareStringsIgnoreCase(const char * s1, const QString & s2);
	int compareStringsIgnoreCase(const QString & s1, const char * s2);
#endif // USE_QT
#if USE_STL
	int compareStringsIgnoreCase(const std::string & s1, const std::string & s2);
	int compareStringsIgnoreCase(const char * s1, const std::string & s2);
	int compareStringsIgnoreCase(const std::string & s1, const char * s2);
#if USE_QT
	int compareStringsIgnoreCase(const QString & s1, const std::string & s2);
	int compareStringsIgnoreCase(const std::string & s1, const QString & s2);
#endif // USE_QT
#endif // USE_STL
	bool isComment(const char * data);
#if USE_QT
	bool isComment(const QString & data);
#endif // USE_QT
#if USE_STL
	bool isComment(const std::string & data);
#endif // USE_STL
#if USE_QT
	bool isComment(const char * data, const QString & comment);
	bool isComment(const QString & data, const char * comment);
	bool isComment(const QString & data, const QString & comment);
#endif // USE_QT
#if USE_STL
	bool isComment(const char * data, const std::string & comment);
	bool isComment(const std::string & data, const char * comment);
	bool isComment(const std::string & data, const std::string & comment);
#endif // USE_STL
	bool isComment(const char * data, const char * comment);
	
#if USE_QT
	QString getVariableID(const char * data);
	QString getVariableID(const QString & data);
	QString getVariableValue(const char * data);
	QString getVariableValue(const QString & data);
#elif USE_STL
	std::string getVariableID(const char * data);
	std::string getVariableID(const std::string & data);
	std::string getVariableValue(const char * data);
	std::string getVariableValue(const std::string & data);
#endif // USE_STL
#if USE_QT
	QString getCommand(const char * data);
	QString getCommand(const QString & data);
	QString getArguments(const char * data);
	QString getArguments(const QString & data);
#endif // USE_QT
	bool parseBoolean(const char * data, bool * valid = NULL);
#if USE_QT
	bool parseBoolean(const QString & data, bool * valid = NULL);
#endif // USE_QT
#if USE_STL
	bool parseBoolean(const std::string & data, bool * valid = NULL);
#endif // USE_STL
	int parseInteger(const char * data, bool * valid = NULL);
#if USE_QT
	int parseInteger(const QString & data, bool * valid = NULL);
#endif // USE_QT
#if USE_STL
	int parseInteger(const std::string & data, bool * valid = NULL);
#endif // USE_STL
	float parseFloat(const char * data, bool * valid = NULL);
#if USE_QT
	float parseFloat(const QString & data, bool * valid = NULL);
#endif // USE_QT
#if USE_STL
	float parseFloat(const std::string & data, bool * valid = NULL);
#endif // USE_STL
#if USE_QT
	QDate parseDate(const char * date);
	QDate parseDate(const QString & date);
	QString dateToString(const QDate & date);
#endif // USE_QT
#if USE_QT
	QString generateFullPath(const char * path, const char * fileName);
	QString generateFullPath(const QString & path, const QString & fileName);
#elif USE_STL // USE_STL
	std::string generateFullPath(const char * path, const char * fileName);
	std::string generateFullPath(const std::string & path, const std::string & fileName);
#endif // USE_STL
	char * getFileNameNoExtension(const char * fileName);
	char * getFileExtension(const char * fileName);
	bool fileHasExtension(const char * fileName, const char * fileExtension);
#if USE_QT
	QString getFileNameNoExtension(const QString & fileName);
	QString getFileExtension(const QString & fileName);
	bool fileHasExtension(const QString & fileName, const QString & fileExtension);
	void deleteFiles(const char * suffix);
	void renameFiles(const char * fromSuffix, const char * toSuffix);
#endif // USE_QT
	void clear();
	void pause();
}

#endif // UTILITIES_H
