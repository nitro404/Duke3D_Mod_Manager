#ifndef UTILITIES_H
#define UTILITIES_H

#include <QDir.h>
#include <QFile.h>
#include <QFileInfo.h>
#include <QGlobal.h>
#include <QTime>

namespace Utilities {
	extern const char newLine[];

	int randomInteger(int min, int max, bool randomize = true);
	unsigned int stringLength(const char * s);
	bool copyString(char * destination, int size, const char * source);
	char * trimCopy(const char * data);
	int compareStrings(const char * s1, const char * s2, bool caseSensitive = true);
	int compareStringsIgnoreCase(const char * s1, const char * s2);
	void pause();
	void deleteFiles(const char * suffix);
	void renameFiles(const char * fromSuffix, const char * toSuffix);
}

#endif // UTILITIES_H
