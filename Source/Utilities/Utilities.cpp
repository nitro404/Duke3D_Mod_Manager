#include "Utilities.h"

#if _WIN32
const char Utilities::newLine[] = "\r\n";
#elif __APPLE__
const char Utilities::newLine[] = "\r";
#else
const char Utilities::newLine[] = "\n";
#endif

int Utilities::randomInteger(int min, int max, bool randomize) {
	if(randomize) {
		QTime time = QTime::currentTime();
		qsrand((uint) time.msec());
	}

	return qrand() % ((max + 1) - min) + min;
}

unsigned int Utilities::stringLength(const char * s) {
	return s == NULL ? 0 : strlen(s);
}

bool Utilities::copyString(char * destination, int size, const char * source) {
#if _WIN32
	return strcpy_s(destination, size, source) == 0;
#else
	return strcpy(destination, source) == 0;
#endif
}

char * Utilities::copyString(const char * data) {
	if(data == NULL) { return NULL; }

	int bufferSize = stringLength(data) + 1;
	char * newData = new char[bufferSize];
	if(copyString(newData, bufferSize, data)) {
		return newData;
	}
	return NULL;
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

char * Utilities::substring(const char * data, int start, int end) {
	if(data == NULL || start > end) { return NULL; }

	int dataLength = stringLength(data);

	if(dataLength == 0) {
		char * newString = new char[1];
		newString[0] = '\0';

		return newString;
	}
	
	int startPos = start < 0 ? 0 : start;
	int endPos = end > dataLength ? dataLength : end;

	char * newString = new char[endPos - startPos + 1];
	int x = 0;
	for(int i=startPos;i<endPos;i++) {
		newString[x++] = data[i];
	}
	newString[x++] = '\0';

	return newString;
}

QString Utilities::substring(const QString & data, int start, int end) {
	if(data.isNull() || start > end) { return QString(); }

	if(data.isEmpty()) {
		return QString("");
	}

	int startPos = start < 0 ? 0 : start;
	int endPos = end > data.length() ? data.length() : end;

	return data.mid(startPos, endPos - startPos);
}

int Utilities::compareStrings(const char * s1, const char * s2, bool caseSensitive) {
	if(s1 == NULL && s2 == NULL) { return 0; }
	if(s1 == NULL && s2 != NULL) { return -1; }
	if(s1 != NULL && s2 == NULL) { return 1; }

	if(caseSensitive) {
#if _WIN32
		return strcmp(s1, s2);
#else
		return strcmp(s1, s2);
#endif
	}
	else {
#if _WIN32
		return _stricmp(s1, s2);
#else
		return stricmp(s1, s2);
#endif
	}
}

int Utilities::compareStringsIgnoreCase(const char * s1, const char * s2) {
	return compareStrings(s1, s2, false);
}

int Utilities::firstIndexOf(const char * data, char c) {
	if(data == NULL) { return -1; }
	
	int length = stringLength(data);
	for(int i=0;i<length;i++) {
		if(data[i] == c) {
			return i;
		}
	}
	return -1;
}

int Utilities::lastIndexOf(const char * data, char c) {
	if(data == NULL) { return -1; }
	
	for(int i=stringLength(data)-1;i>=0;i--) {
		if(data[i] == c) {
			return i;
		}
	}
	return -1;
}

char * Utilities::getFileNameNoExtension(const char * fileName) {
	if(fileName == NULL) { return NULL; }
	
	int index = lastIndexOf(fileName, '.');
	if(index > 0) {
		return substring(fileName, 0, index);
	}
	return copyString(fileName);
}

char * Utilities::getFileExtension(const char * fileName) {
	if(fileName == NULL) { return NULL; }
	
	int index = lastIndexOf(fileName, '.');
	if(index > 0) {
		return substring(fileName, index + 1, stringLength(fileName));
	}
	return NULL;
}

bool Utilities::fileHasExtension(const char * fileName, const char * fileExtension) {
	if(fileName == NULL || fileExtension == NULL) { return false; }
	
	char * actualFileExtension = getFileExtension(fileName);
	bool fileExtensionMatches = actualFileExtension != NULL && compareStringsIgnoreCase(actualFileExtension, fileExtension) == 0;
	delete [] actualFileExtension;
	return fileExtensionMatches;
}

QString Utilities::getFileNameNoExtension(const QString & fileName) {
	int index = fileName.lastIndexOf('.');
	if(index > 0) {
		return fileName.mid(0, index);
	}
	return fileName;
}

QString Utilities::getFileExtension(const QString & fileName) {
	int index = fileName.lastIndexOf('.');
	if(index > 0) {
		return fileName.mid(index + 1, fileName.length() - index + 1);
	}
	return QString();
}

bool Utilities::fileHasExtension(const QString & fileName, const QString & fileExtension) {
	QString actualFileExtension = getFileExtension(fileName);
	return !actualFileExtension.isNull() && QString::compare(actualFileExtension, fileExtension, Qt::CaseInsensitive) == 0;
}

void Utilities::deleteFiles(const char * suffix) {
	if(suffix == NULL || stringLength(suffix) == 0) { return; }

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
	if(fromSuffix == NULL || stringLength(fromSuffix) == 0 || toSuffix == NULL || stringLength(toSuffix) == 0) { return; }
	
	QDir dir = QDir::currentPath();
	QFileInfoList files = dir.entryInfoList();
	for(int i=0;i<files.size();i++) {
		if(files[i].isFile() && QString::compare(files[i].completeSuffix(), fromSuffix, Qt::CaseInsensitive) == 0) {
			QFile file(files[i].fileName());

			file.rename(QString("%1.%2").arg(files[i].completeBaseName()).arg(toSuffix));
		}
	}
}

void Utilities::pause() {
	system("pause");
}
