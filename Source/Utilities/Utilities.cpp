#include "Utilities.h"

#if _WIN32
const char Utilities::newLine[] = "\r\n";
#elif __APPLE__
const char Utilities::newLine[] = "\r";
#else
const char Utilities::newLine[] = "\n";
#endif // _WIN32

bool Utilities::initialRandomize = true;

const char * Utilities::monthStrings[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

void Utilities::randomizeSeed() {
	qsrand(static_cast<unsigned int>(QTime::currentTime().msec()));
}

void Utilities::randomSeed(unsigned int seed) {
	qsrand(seed);
}

int Utilities::randomInteger(int min, int max, bool randomize) {
	if(max <= min) { return min; }

	if(randomize || initialRandomize) {
		randomizeSeed();
		initialRandomize = false;
	}

	return (qrand() % (max - min + 1)) + min;
}

float Utilities::randomFloat(float min, float max, bool randomize) {
	if(max <= min) { return min; }

	if(randomize || initialRandomize) {
		randomizeSeed();
		initialRandomize = false;
	}

	return ((static_cast<float>(qrand()) / static_cast<float>(RAND_MAX)) * max) + min;
}

int Utilities::intLength(int n) {
	return n < 100000 ? n < 100 ? n < 10 ? 1 : 2 : n < 1000 ? 3 : n < 10000 ? 4 : 5 : n < 10000000 ? n < 1000000 ? 6 : 7 : n < 100000000 ? 8 : n < 1000000000 ? 9 : 10;
}

unsigned int Utilities::stringLength(const char * s) {
	return s == NULL ? 0 : strlen(s);
}

const char * Utilities::toString(int value) {
	static char buffer[32];
#if _WIN32
	sprintf_s(buffer, 32, "%d", value);
#else
	sprintf(buffer, "%d", value);
#endif // _WIN32

	return buffer;
}

const char * Utilities::toString(double value) {
	static char buffer[32];
#if _WIN32
	sprintf_s(buffer, 32, "%f", value);
#else
	sprintf(buffer, "%f", value);
#endif // _WIN32

	return buffer;
}

bool Utilities::copyString(char * destination, int size, const char * source) {
	if(source == NULL || destination == NULL || size < 1) { return false; }

#if _WIN32
	return strcpy_s(destination, size, source) == 0;
#else
	return strcpy(destination, source) == 0;
#endif // _WIN32
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
char * Utilities::trimCopyString(const char * data) {
	// verify the string
	if(data == NULL) { return NULL; }
	char * newData;
	int length = stringLength(data);
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

#if USE_STL
std::string Utilities::trimString(const std::string & data) {
	if(data.empty()) { return std::string(); }
	if(data.length() == 1 && (data[0] == ' ' || data[0] == '\t' || data[0] == '\n' || data[0] == '\r')) { return std::string(); }

	int start = 0;
	int end = data.length();

	for(int i=0;i<static_cast<int>(data.length());i++) {
		if(data[i] != ' ' && data[i] != '\t' && data[i] != '\n' && data[i] != '\r') {
			break;
		}

		start++;
	}

	for(int i=static_cast<int>(data.length()) - 1;i>=0;i--) {
		if(data[i] != ' ' && data[i] != '\t' && data[i] != '\n' && data[i] != '\r') {
			break;
		}

		end--;
	}

	if(start > end) { return std::string(); }

	return data.substr(start, end - start);
}
#endif // USE_STL

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

#if USE_STL
std::string Utilities::substring(const std::string & data, int start, int end) {
	if(data.empty() || start > end) { return std::string(); }

	int startPos = start < 0 ? 0 : start;
	int endPos = end > static_cast<int>(data.length()) ? static_cast<int>(data.length()) : end;

	return data.substr(startPos, endPos - startPos);
}
#endif // USE_STL

int Utilities::compareStrings(const char * s1, const char * s2, bool caseSensitive) {
	if(s1 == NULL && s2 == NULL) { return 0; }
	if(s1 == NULL && s2 != NULL) { return -1; }
	if(s1 != NULL && s2 == NULL) { return 1; }

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

int Utilities::compareStrings(const QString & s1, const QString & s2, bool caseSensitive) {
	return s1.compare(s2, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
}

int Utilities::compareStrings(const char * s1, const QString & s2, bool caseSensitive) {
	QByteArray bytes = s2.toLocal8Bit();
	return compareStrings(s1, s2.isNull() ? NULL : bytes.data(), caseSensitive);
}

int Utilities::compareStrings(const QString & s1, const char * s2, bool caseSensitive) {
	QByteArray bytes = s1.toLocal8Bit();
	return compareStrings(s1.isNull() ? NULL : bytes.data(), s2, caseSensitive);
}

#if USE_STL
int Utilities::compareStrings(const std::string & s1, const std::string & s2, bool caseSensitive) {
	return compareStrings(s1.data(), s2.data(), caseSensitive);
}

int Utilities::compareStrings(const char * s1, const std::string & s2, bool caseSensitive) {
	return compareStrings(s1, s2.data(), caseSensitive);
}

int Utilities::compareStrings(const std::string & s1, const char * s2, bool caseSensitive) {
	return compareStrings(s1.data(), s2, caseSensitive);
}

#if USE_QT
int Utilities::compareStrings(const QString & s1, const std::string & s2, bool caseSensitive) {
	QByteArray bytes = s1.toLocal8Bit();
	return compareStrings(bytes.data(), s2.data(), caseSensitive);
}

int Utilities::compareStrings(const std::string & s1, const QString & s2, bool caseSensitive) {
	QByteArray bytes = s2.toLocal8Bit();
	return compareStrings(s1.data(), bytes.data(), caseSensitive);
}
#endif // USE_QT
#endif // USE_STL

int Utilities::compareStringsIgnoreCase(const char * s1, const char * s2) {
	return compareStrings(s1, s2, false);
}

int Utilities::compareStringsIgnoreCase(const QString & s1, const QString & s2) {
	return compareStrings(s1, s2, false);
}

int Utilities::compareStringsIgnoreCase(const char * s1, const QString & s2) {
	QByteArray bytes = s2.toLocal8Bit();
	return compareStrings(s1, s2.isNull() ? NULL : bytes.data(), false);
}

int Utilities::compareStringsIgnoreCase(const QString & s1, const char * s2) {
	QByteArray bytes = s1.toLocal8Bit();
	return compareStrings(s1.isNull() ? NULL : bytes.data(), s2, false);
}

#if USE_STL
int Utilities::compareStringsIgnoreCase(const std::string & s1, const std::string & s2) {
	return compareStrings(s1.data(), s2.data(), false);
}

int Utilities::compareStringsIgnoreCase(const char * s1, const std::string & s2) {
	return compareStrings(s1, s2.data(), false);
}

int Utilities::compareStringsIgnoreCase(const std::string & s1, const char * s2) {
	return compareStrings(s1.data(), s2, false);
}

#if USE_QT
int Utilities::compareStringsIgnoreCase(const QString & s1, const std::string & s2) {
	QByteArray bytes = s1.toLocal8Bit();
	return compareStrings(bytes.data(), s2.data(), false);
}

int Utilities::compareStringsIgnoreCase(const std::string & s1, const QString & s2) {
	QByteArray bytes = s2.toLocal8Bit();
	return compareStrings(s1.data(), bytes.data(), false);
}
#endif // USE_QT
#endif // USE_STL

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

QString Utilities::getVariableID(const char * data) {
	if(data == NULL) { return QString(); }

	return getVariableID(QString(data));
}

QString Utilities::getVariableID(const QString & data) {
	if(data.isNull()) { return QString(); }

	QString formattedData = data.trimmed();
	if(formattedData.isEmpty()) { return QString(""); }

	int separatorIndex = -1;
	for(int i=0;i<formattedData.length();i++) {
		if(formattedData[i] == ':' || formattedData[i] == '=') {
			separatorIndex = i;
			break;
		}
	}
	if(separatorIndex < 0) { return QString(); }

	return substring(formattedData, 0, separatorIndex).trimmed();
}

QString Utilities::getVariableValue(const char * data) {
	return getVariableValue(QString(data));
}

QString Utilities::getVariableValue(const QString & data) {
	if(data.isNull()) { return QString(); }

	QString formattedData = data.trimmed();
	if(formattedData.isEmpty()) { return QString(""); }

	int separatorIndex = -1;
	for(int i=0;i<formattedData.length();i++) {
		if(formattedData[i] == ':' || formattedData[i] == '=') {
			separatorIndex = i;
			break;
		}
	}
	if(separatorIndex < 0) { return QString(); }

	return substring(formattedData, separatorIndex + 1, formattedData.length()).trimmed();
}

QString Utilities::getCommand(const char * data) {
	if(data == NULL) { return QString(); }

	return getCommand(QString(data));
}

QString Utilities::getCommand(const QString & data) {
	if(data.isNull()) { return QString(); }

	QString newData = data.trimmed();
	if(newData.isEmpty()) { return QString(""); }

	int split = newData.indexOf(" ");
	if(split < 1) { return newData; }

	return substring(newData, 0, split).trimmed();
}

QString Utilities::getArguments(const char * data) {
	return getArguments(QString(data));
}

QString Utilities::getArguments(const QString & data) {
	if(data.isNull()) { return QString(); }

	QString newData = data.trimmed();
	if(newData.isEmpty()) { return QString(""); }

	int split = newData.indexOf(" ");
	if(split < 1 || split == newData.length() - 1) { return QString(""); }

	return substring(newData, split + 1, newData.length()).trimmed();
}

bool Utilities::parseBoolean(const char * data, bool * valid) {
	if(data == NULL) {
		if(valid != NULL) { *valid = false; }
		return false;
	}

	char * trimmedData = trimCopyString(data);
	if(stringLength(trimmedData) == 0) {
		if(valid != NULL) { *valid = false; }
		delete [] trimmedData;
		return false;
	}

	if(stringLength(trimmedData) == 1) {
		if(trimmedData[0] == 't' ||
		   trimmedData[0] == '1' ||
		   trimmedData[0] == 'y') {
			if(valid != NULL) { *valid = true; }
			delete [] trimmedData;
			return true;
		}
		else if(trimmedData[0] == 'f' ||
				trimmedData[0] == '0' ||
				trimmedData[0] == 'n') {
			if(valid != NULL) { *valid = true; }
			delete [] trimmedData;
			return false;
		}
	}
	else {
		if(compareStringsIgnoreCase(trimmedData, "true") == 0 ||
		   compareStringsIgnoreCase(trimmedData, "on") == 0 ||
		   compareStringsIgnoreCase(trimmedData, "yes") == 0) {
			if(valid != NULL) { *valid = true; }
			delete [] trimmedData;
			return true;
		}
		else if(compareStringsIgnoreCase(trimmedData, "false") == 0 ||
				compareStringsIgnoreCase(trimmedData, "off") == 0 ||
				compareStringsIgnoreCase(trimmedData, "no") == 0) {
			if(valid != NULL) { *valid = true; }
			delete [] trimmedData;
			return false;
		}
	}

	if(valid != NULL) { *valid = false; }
	delete [] trimmedData;
	return false;
}

bool Utilities::parseBoolean(const QString & data, bool * valid) {
	if(data.isEmpty()) {
		if(valid != NULL) { *valid = false; }
		return false;
	}

	QByteArray bytes = data.toLocal8Bit();

	return parseBoolean(bytes.data(), valid);
}

#if USE_STL
bool Utilities::parseBoolean(const std::string & data, bool * valid) {
	if(data.empty()) {
		if(valid != NULL) { *valid = false; }
		return false;
	}

	return parseBoolean(data.data(), valid);
}
#endif // USE_STL

int Utilities::parseInteger(const char * data, bool * valid) {
	if(data == NULL || stringLength(data) == 0) {
		if(valid != NULL) { *valid = false; }
		return 0;
	}

#if USE_QT
	QString formattedData = QString(data).trimmed();
	int value = formattedData.toInt(valid);
	return value;
#elif USE_STL
	char * trimmedData = trimCopyString(data);
	if(stringLength(trimmedData) == 0) {
		if(valid != NULL) { *valid = false; }
		delete [] trimmedData;
		return 0;
	}

	for(int i=0;i<stringLength(trimmedData);i++) {
		if(!(trimmedData[i] == '-' || (trimmedData[i] >= '0' && trimmedData[i] <= '9'))) {
			if(valid != NULL) { *valid = false; }
			delete [] trimmedData;
			return 0;
		}
	}

	if(valid != NULL) { *valid = true; }
	float value = atoi(trimmedData);

	delete [] trimmedData;

	return value;
#endif // USE_QT
}

int Utilities::parseInteger(const QString & data, bool * valid) {
	if(data.isEmpty()) {
		if(valid != NULL) { *valid = false; }
		return 0;
	}

	QByteArray bytes = data.toLocal8Bit();

	return parseInteger(bytes.data(), valid);
}

#if USE_STL
int Utilities::parseInteger(const std::string & data, bool * valid) {
	if(data.empty()) {
		if(valid != NULL) { *valid = false; }
		return 0;
	}

	return parseInteger(data.data(), valid);
}
#endif // USE_STL

float Utilities::parseFloat(const char * data, bool * valid) {
	if(data == NULL || stringLength(data) == 0) {
		if(valid != NULL) { *valid = false; }
		return 0.0f;
	}

#if USE_QT
	QString formattedData = QString(data).trimmed();
	float value = formattedData.toFloat(valid);
	return value;
#elif USE_STL
	char * trimmedData = trimCopyString(data);
	if(stringLength(trimmedData) == 0) {
		if(valid != NULL) { *valid = false; }
		delete [] trimmedData;
		return 0.0f;
	}

	for(int i=0;i<stringLength(trimmedData);i++) {
		if(!(trimmedData[i] == '-' || trimmedData[i] == '.' || (trimmedData[i] >= '0' && trimmedData[i] <= '9'))) {
			if(valid != NULL) { *valid = false; }
			delete [] trimmedData;
			return 0.0f;
		}
	}

	if(valid != NULL) { *valid = true; }
	float value = atof(trimmedData);

	delete [] trimmedData;

	return value;
#endif // USE_QT
}

float Utilities::parseFloat(const QString & data, bool * valid) {
	if(data.isEmpty()) {
		if(valid != NULL) { *valid = false; }
		return 0.0f;
	}

	QByteArray bytes = data.toLocal8Bit();

	return parseFloat(bytes.data(), valid);
}

#if USE_STL
float Utilities::parseFloat(const std::string & data, bool * valid) {
	if(data.empty()) {
		if(valid != NULL) { *valid = false; }
		return 0.0f;
	}

	return parseFloat(data.data(), valid);
}
#endif // USE_STL

QDate Utilities::parseDate(const char * date) {
	if(date == NULL) { return QDate(); }

	return parseDate(QString(date));
}

QDate Utilities::parseDate(const QString & date) {
	if(date.trimmed().isEmpty()) { return QDate(); }

	static const QRegExp    dateRegExp("[ ,]+");
	static const QRegExp integerRegExp("^[-0-9]+$");

	QStringList dateParts = date.trimmed().split(dateRegExp, QString::SkipEmptyParts);

	if(dateParts.size() != 3) {
		return QDate();
	}

	int year = -1;
	int month = -1;
	int day = -1;
	for(int i=0;i<12;i++) {
		if(Utilities::compareStringsIgnoreCase(dateParts[0], monthStrings[i]) == 0) {
			month = i + 1;
			break;
		}
	}
	if(month < 1) { return QDate(); }

	if(!integerRegExp.exactMatch(dateParts[1]) || !integerRegExp.exactMatch(dateParts[2])) { return QDate(); }

	bool success = false;

	day = dateParts[1].toInt(&success, 10);
	if(!success || day < 1 || day > 31) { return QDate(); }

	year = dateParts[2].toInt(&success, 10);
	if(!success || year < 1) { return QDate(); }

	return QDate(year, month, day);
}

QString Utilities::dateToString(const QDate & date) {
	if(date.isNull()) { return QString(); }

	return QString("%1 %2, %3").arg(monthStrings[date.month() - 1]).arg(date.day()).arg(date.year());
}

QString Utilities::generateFullPath(const char * path, const char * fileName) {
	return generateFullPath(QString(path), QString(fileName));
}

QString Utilities::generateFullPath(const QString & path, const QString & fileName) {
	QString trimmedPath = path.trimmed();
	QString trimmedFileName = fileName.trimmed();
	QString fullPath("");

	if(trimmedPath.isEmpty())	 { return trimmedFileName; }
	if(trimmedFileName.isEmpty()) { return trimmedPath; }

	if(Utilities::compareStrings(trimmedPath, ".") != 0) {
		fullPath.append(trimmedPath);

		if(trimmedPath[trimmedPath.length() - 1] != '/' && trimmedPath[trimmedPath.length() - 1] != '\\') {
			fullPath.append("/");
		}
	}

	fullPath.append(trimmedFileName);

	return fullPath;
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

void Utilities::clear() {
	system("cls");
}

void Utilities::pause() {
	system("pause");
}
