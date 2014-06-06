#include "Utilities.h"

#if _WIN32
const char Utilities::newLine[] = "\r\n";
#else
const char Utilities::newLine[] = "\n";
#endif // _WIN32

int Utilities::randomInteger(int min, int max, bool randomize) {
	if(randomize) {
		QTime time = QTime::currentTime();
		qsrand((uint) time.msec());
	}

	return qrand() % ((max + 1) - min) + min;
}

unsigned int Utilities::stringLength(const char * s) {
	return strlen(s);
}

bool Utilities::copyString(char * destination, int size, const char * source) {
#if _WIN32
	return strcpy_s(destination, size, source) == 0;
#else
	return strcpy(destination, source) == 0;
#endif // _WIN32
}

// trims whitespace off of the front and end of string passed into it, and returns a copy of the trimmed string
char * Utilities::trimCopy(const char * data) {
	// verify the string
	if(data == NULL) { return NULL; }
	char * newData;
	int length = strlen(data);
	if(length == 0) {
		newData = new char[1];
		*newData = '\0';
		return newData;
	}

	// find the new start and end of the string and verify that they do not overlap (0 length string)
	const char * head = data;
	const char * tail = data + (sizeof(char) * length) - 1;
	int startPos = 0, endPos = length - 1;
	while((*head == ' ' || *head == '\t') && startPos <= endPos) {
		head += sizeof(char);
		startPos++;
	}
	while((*tail == ' ' || *tail == '\t') && startPos <= endPos) {
		tail -= sizeof(char);
		endPos--;
	}
	if(startPos > endPos) {
		newData = new char[1];
		*newData = '\0';
		return newData;
	}

	// copy the contents of the string from the start to the end into a new string (trim) and return the copy
	newData = new char[endPos - startPos + 2];
	char * temp = newData;
	for(int i=startPos;i<=endPos;i++) {
		*temp = data[i];
		temp += sizeof(char);
	}
	*temp = '\0';
	return newData;
}

int Utilities::compareStrings(const char * s1, const char * s2, bool caseSensitive) {
	if(caseSensitive) {
#if _WIN32
		return strcmp(s1, s2);
#else
		return strcmp(s1, s2);
#endif // _WIN32
	}
	else {
#if _WIN32
		return _stricmp(s1, s2);
#else
		return stricmp(s1, s2);
#endif // _WIN32
	}
}

int Utilities::compareStringsIgnoreCase(const char * s1, const char * s2) {
	return compareStrings(s1, s2, false);
}

void Utilities::pause() {
	system("pause");
}

void Utilities::deleteFiles(const char * suffix) {
	if(suffix == NULL || Utilities::stringLength(suffix) == 0) { return; }

	QDir dir = QDir::currentPath();
	QFileInfoList files = dir.entryInfoList();
	for(int i=0;i<files.size();i++) {
		if(files[i].isFile() && QString::compare(files[i].completeSuffix(), suffix, Qt::CaseInsensitive) == 0) {
			QFile file(files[i].fileName());

			file.remove();
		}
	}
}

void Utilities::renameFiles(const char * fromSuffix, const char * toSuffix) {
	if(fromSuffix == NULL || Utilities::stringLength(fromSuffix) == 0 || toSuffix == NULL || Utilities::stringLength(toSuffix) == 0) { return; }
	
	QDir dir = QDir::currentPath();
	QFileInfoList files = dir.entryInfoList();
	for(int i=0;i<files.size();i++) {
		if(files[i].isFile() && QString::compare(files[i].completeSuffix(), fromSuffix, Qt::CaseInsensitive) == 0) {
			QFile file(files[i].fileName());

			file.rename(QString("%1.%2").arg(files[i].completeBaseName()).arg(toSuffix));
		}
	}
}
