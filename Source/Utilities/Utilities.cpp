#include "Utilities.h"

namespace Utilities {
	
#if _WIN32
	const char newLine[] = "\r\n";
#elif __APPLE__
	const char newLine[] = "\n";
#else
	const char newLine[] = "\n";
#endif // _WIN32

	bool initialRandomize = true;

	const char * monthStrings[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

	void randomizeSeed() {
#if USE_QT
		qsrand(static_cast<unsigned int>(QTime::currentTime().msec()));
#else // USE_STL
		srand(static_cast<unsigned int>(time(NULL)));
#endif // USE_QT / USE_STL
	}

	void randomSeed(unsigned int seed) {
#if USE_QT
		qsrand(seed);
#else // USE_STL
		srand(seed);
#endif // USE_QT / USE_STL
	}

	int randomInteger(int min, int max, bool randomize) {
		if(max <= min) { return min; }

		if(randomize || initialRandomize) {
			randomizeSeed();
			initialRandomize = false;
		}

#if USE_QT
		return (qrand() % (max - min + 1)) + min;
#else // USE_STL
		return (rand() % (max - min + 1)) + min;
#endif // USE_QT / USE_STL
	}

	float randomFloat(float min, float max, bool randomize) {
		if(max <= min) { return min; }

		if(randomize || initialRandomize) {
			randomizeSeed();
			initialRandomize = false;
		}

#if USE_QT
		return ((static_cast<float>(qrand()) / static_cast<float>(RAND_MAX)) * (max - min)) + min;
#else // USE_STL
		return ((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (max - min)) + min;
#endif // USE_QT / USE_STL
	}

	int byteLength(char n) {
		return n < 0 ? n < -99 ? 4 : n < -9 ? 3 : 2 : n < 10 ? 1 : n < 100 ? 2 : 3;
	}

	int unsignedByteLength(unsigned char n) {
		return n < 10 ? 1 : n < 100 ? 2 : 3;
	}

	int shortLength(short n) {
		return n < 0 ? n < -99 ? n < -9999 ? 6 : n < -999 ? 5 : 4 : n < -9 ? 3 : 2 : n < 100 ? n < 10 ? 1 : 2 : n < 1000 ? 3 : n < 10000 ? 4 : 5;
	}

	int unsignedShortLength(unsigned short n) {
		return n < 100 ? n < 10 ? 1 : 2 : n < 1000 ? 3 : n < 10000 ? 4 : 5;
	}

	int intLength(int n) {
		return n < 0 ? n < -999999 ? n < -99999999 ? n < -999999999 ? 11 : 10 : n < -9999999 ? 9 : 8 : n < -999 ? n < -99999 ? 7 : n < -9999 ? 6 : 5 : n < -99 ? 4 : n < -9 ? 3 : 2 : n < 100000 ? n < 100 ? n < 10 ? 1 : 2 : n < 1000 ? 3 : n < 10000 ? 4 : 5 : n < 10000000 ? n < 1000000 ? 6 : 7 : n < 100000000 ? 8 : n < 1000000000 ? 9 : 10;
	}
	
	int unsignedIntLength(int n) {
		return n < 100000 ? n < 100 ? n < 10 ? 1 : 2 : n < 1000 ? 3 : n < 10000 ? 4 : 5 : n < 10000000 ? n < 1000000 ? 6 : 7 : n < 100000000 ? 8 : n < 1000000000 ? 9 : 10;
	}

	int longLength(long n) {
		return intLength(n);
	}

	int unsignedLongLength(unsigned long n) {
		return unsignedIntLength(n);
	}

	int longLongLength(long long n) {
		return n < 0 ? n < -999999999L ? n < -99999999999999L ? n < -9999999999999999L ? n < -999999999999999999L ? 20 : n < -99999999999999999L ? 19 : 18 : n < -999999999999999L ? 17 : 16 : n < -99999999999L ? n < -9999999999999L ? 15 : n < -999999999999L ? 14 : 13 : n < -9999999999L ? 12 : 11 : n < -9999L ? n < -999999L ? n < -99999999L ? 10 : n < -9999999L ? 9 : 8 : n < -99999L ? 7 : 6 : n < -99L ? n < -999L ? 5 : 4 : n < -9L ? 3 : 2 : n < 1000000000L ? n < 10000L ? n < 100L ? n < 10L ? 1 : 2 : n < 1000L ? 3 : 4 : n < 10000000L ? n < 100000L ? 5 : n < 1000000L ? 6 : 7 : n < 100000000L ? 8 : 9 : n < 100000000000000L ? n < 1000000000000L ? n < 100000000000L ? n < 10000000000L ? 10 : 11 : 12 : n < 10000000000000L ? 13 : 14 : n < 100000000000000000L ? n < 10000000000000000L ? n < 1000000000000000L ? 15 : 16 : 17 : n < 1000000000000000000L ? 18 : 19;
	}
	
	int unsignedLongLongLength(unsigned long long n) {
		return n < 1000000000L ? n < 10000L ? n < 100L ? n < 10L ? 1 : 2 : n < 1000L ? 3 : 4 : n < 10000000L ? n < 100000L ? 5 : n < 1000000L ? 6 : 7 : n < 100000000L ? 8 : 9 : n < 100000000000000L ? n < 1000000000000L ? n < 100000000000L ? n < 10000000000L ? 10 : 11 : 12 : n < 10000000000000L ? 13 : 14 : n < 100000000000000000L ? n < 10000000000000000L ? n < 1000000000000000L ? 15 : 16 : 17 : n < 1000000000000000000L ? 18 : 19;
	}

	unsigned int stringLength(const char * s) {
		return s == NULL ? 0 : strlen(s);
	}

	const char * toString(int value) {
		static char buffer[12];
#if _WIN32
		sprintf_s(buffer, 12, "%d", value);
#else
		sprintf(buffer, "%d", value);
#endif // _WIN32

		return buffer;
	}

	const char * toString(double value) {
		static const int MAX_DOUBLE_LENGTH = DBL_MANT_DIG - DBL_MIN_EXP + 4;

		static char buffer[MAX_DOUBLE_LENGTH];
#if _WIN32
		sprintf_s(buffer, MAX_DOUBLE_LENGTH, "%f", value);
#else
		sprintf(buffer, "%f", value);
#endif // _WIN32

		return buffer;
	}

	bool copyString(char * destination, int size, const char * source) {
		if(source == NULL || destination == NULL || size < 1) { return false; }

#if _WIN32
		return strcpy_s(destination, size, source) == 0;
#else
		return strcpy(destination, source) == 0;
#endif // _WIN32
	}

	char * copyString(const char * data) {
		if(data == NULL) { return NULL; }

		int bufferSize = stringLength(data) + 1;
		char * newData = new char[bufferSize];
		if(copyString(newData, bufferSize, data)) {
			return newData;
		}
		return NULL;
	}

	// trims whitespace off of the front and end of string passed into it, and returns a copy of the trimmed string
	char * trimCopyString(const char * data) {
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
	std::string trimString(const std::string & data, bool trimWhiteSpace, bool trimNewLines) {
		if(data.empty()) { return std::string(); }
		if(trimWhiteSpace == false && trimNewLines == false) { return data; }
		if(data.length() == 1 && ((trimWhiteSpace && (data[0] == ' ' || data[0] == '\t')) || (trimNewLines && (data[0] == '\n' || data[0] == '\r')))) { return std::string(); }
		
		int start = 0;
		int end = 0;
		
		for(int i=0;i<static_cast<int>(data.length());i++) {
			start = i;
			
			if(!((trimWhiteSpace && (data[i] == ' ' || data[i] == '\t')) || (trimNewLines && (data[i] == '\n' || data[i] == '\r')))) {
				break;
			}
		}
		
		for(int i=static_cast<int>(data.length())-1;i>=0;i--) {
			end = i;
			
			if(!((trimWhiteSpace && (data[i] == ' ' || data[i] == '\t')) || (trimNewLines && (data[i] == '\n' || data[i] == '\r')))) {
				break;
			}
		}
		
		if(start > end) { return std::string(); }
		
		return data.substr(start, end - start + 1);
	}
#endif // USE_STL

	char * substring(const char * data, int start, int end) {
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

#if USE_QT
	QString substring(const QString & data, int start, int end) {
		if(data.isNull() || start > end) { return QString(); }

		if(data.isEmpty()) {
			return QString("");
		}

		int startPos = start < 0 ? 0 : start;
		int endPos = end > data.length() ? data.length() : end;

		return data.mid(startPos, endPos - startPos);
	}
#endif // USE_QT

#if USE_STL
	std::string substring(const std::string & data, int start, int end) {
		if(data.empty() || start > end) { return std::string(); }

		int startPos = start < 0 ? 0 : start;
		int endPos = end > static_cast<int>(data.length()) ? static_cast<int>(data.length()) : end;

		return data.substr(startPos, endPos - startPos);
	}
#endif // USE_STL
	
	int firstIndexOf(const char * data, char c) {
		if(data == NULL) { return -1; }
	
		int length = stringLength(data);
		for(int i=0;i<length;i++) {
			if(data[i] == c) {
				return i;
			}
		}
		return -1;
	}

	int lastIndexOf(const char * data, char c) {
		if(data == NULL) { return -1; }
	
		for(int i=stringLength(data)-1;i>=0;i--) {
			if(data[i] == c) {
				return i;
			}
		}
		return -1;
	}
	
	int compareStrings(const char * s1, const char * s2, bool caseSensitive) {
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
			return strcasecmp(s1, s2);
#endif // _WIN32
		}
	}

#if USE_QT
	int compareStrings(const QString & s1, const QString & s2, bool caseSensitive) {
		return s1.compare(s2, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
	}

	int compareStrings(const char * s1, const QString & s2, bool caseSensitive) {
		QByteArray bytes = s2.toLocal8Bit();
		return compareStrings(s1, s2.isNull() ? NULL : bytes.data(), caseSensitive);
	}

	int compareStrings(const QString & s1, const char * s2, bool caseSensitive) {
		QByteArray bytes = s1.toLocal8Bit();
		return compareStrings(s1.isNull() ? NULL : bytes.data(), s2, caseSensitive);
	}
#endif // USE_QT

#if USE_STL
	int compareStrings(const std::string & s1, const std::string & s2, bool caseSensitive) {
		return compareStrings(s1.data(), s2.data(), caseSensitive);
	}

	int compareStrings(const char * s1, const std::string & s2, bool caseSensitive) {
		return compareStrings(s1, s2.data(), caseSensitive);
	}

	int compareStrings(const std::string & s1, const char * s2, bool caseSensitive) {
		return compareStrings(s1.data(), s2, caseSensitive);
	}

#if USE_QT
	int compareStrings(const QString & s1, const std::string & s2, bool caseSensitive) {
		QByteArray bytes = s1.toLocal8Bit();
		return compareStrings(bytes.data(), s2.data(), caseSensitive);
	}

	int compareStrings(const std::string & s1, const QString & s2, bool caseSensitive) {
		QByteArray bytes = s2.toLocal8Bit();
		return compareStrings(s1.data(), bytes.data(), caseSensitive);
	}
#endif // USE_QT
#endif // USE_STL

	int compareStringsIgnoreCase(const char * s1, const char * s2) {
		return compareStrings(s1, s2, false);
	}

#if USE_QT
	int compareStringsIgnoreCase(const QString & s1, const QString & s2) {
		return compareStrings(s1, s2, false);
	}

	int compareStringsIgnoreCase(const char * s1, const QString & s2) {
		QByteArray bytes = s2.toLocal8Bit();
		return compareStrings(s1, s2.isNull() ? NULL : bytes.data(), false);
	}

	int compareStringsIgnoreCase(const QString & s1, const char * s2) {
		QByteArray bytes = s1.toLocal8Bit();
		return compareStrings(s1.isNull() ? NULL : bytes.data(), s2, false);
	}
#endif // USE_QT

#if USE_STL
	int compareStringsIgnoreCase(const std::string & s1, const std::string & s2) {
		return compareStrings(s1.data(), s2.data(), false);
	}

	int compareStringsIgnoreCase(const char * s1, const std::string & s2) {
		return compareStrings(s1, s2.data(), false);
	}

	int compareStringsIgnoreCase(const std::string & s1, const char * s2) {
		return compareStrings(s1.data(), s2, false);
	}

#if USE_QT
	int compareStringsIgnoreCase(const QString & s1, const std::string & s2) {
		QByteArray bytes = s1.toLocal8Bit();
		return compareStrings(bytes.data(), s2.data(), false);
	}

	int compareStringsIgnoreCase(const std::string & s1, const QString & s2) {
		QByteArray bytes = s2.toLocal8Bit();
		return compareStrings(s1.data(), bytes.data(), false);
	}
#endif // USE_QT
#endif // USE_STL

	bool isComment(const char * data) {
		return isComment(data, "//");
	}

#if USE_QT
	bool isComment(const QString & data) {
		QByteArray bytes = data.toLocal8Bit();
		return isComment(bytes.data());
	}
#endif // USE_QT
	
#if USE_STL
	bool isComment(const std::string & data) {
		return isComment(data.data());
	}
#endif // USE_STL
	
#if USE_QT
	bool isComment(const char * data, const QString & comment) {
		QByteArray commentBytes = comment.toLocal8Bit();
		return isComment(data, commentBytes.data());
	}
	
	bool isComment(const QString & data, const char * comment) {
		QByteArray dataBytes = data.toLocal8Bit();
		return isComment(dataBytes.data(), comment);
	}
	
	bool isComment(const QString & data, const QString & comment) {
		QByteArray dataBytes = data.toLocal8Bit();
		QByteArray commentBytes = comment.toLocal8Bit();
		return isComment(dataBytes.data(), commentBytes.data());
	}
#endif // USE_QT

#if USE_STL
	bool isComment(const char * data, const std::string & comment) {
		return isComment(data, comment.data());
	}
	
	bool isComment(const std::string & data, const char * comment) {
		return isComment(data.data(), comment);
	}
	
	bool isComment(const std::string & data, const std::string & comment) {
		return isComment(data.data(), comment.data());
	}
#endif // USE_STL
	
	bool isComment(const char * data, const char * comment) {
		if(data == NULL || comment == NULL || stringLength(comment) == 0) { return false; }
		
		int commentStartIndex = -1;
		for(unsigned int i=0;i<stringLength(data);i++) {
			if(data[i] == ' ' || data[i] == '\t') { continue; }
			
			if(data[i] == comment[0]) {
				commentStartIndex = i;
				break;
			}
			else {
				return false;
			}
		}
		
		if(commentStartIndex < 0 || stringLength(data) - commentStartIndex < stringLength(comment)) { return false; }
		
		for(unsigned int i=commentStartIndex;i<stringLength(data);i++) {
			if(static_cast<int>(i) - commentStartIndex >= static_cast<int>(stringLength(comment))) { break; }
			
			if(data[i] != comment[i - commentStartIndex]) {
				return false;
			}
		}
		
		return true;
	}
	
#if USE_QT
	QString getVariableID(const char * data) {
		if(data == NULL) { return QString(); }

		return getVariableID(QString(data));
	}
	
	QString getVariableID(const QString & data) {
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

	QString getVariableValue(const char * data) {
		return getVariableValue(QString(data));
	}

	QString getVariableValue(const QString & data) {
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
#elif USE_STL
	std::string getVariableID(const char * data) {
		if(data == NULL) { return std::string(); }
		
		return getVariableID(std::string(data));
	}
	
	std::string getVariableID(const std::string & data) {
		if(data.empty()) { return std::string(); }
		
		std::string formattedData = trimString(data);
		
		if(formattedData.empty()) { return std::string(); }
		
		int separatorIndex = -1;
		for(unsigned int i=0;i<formattedData.length();i++) {
			if(formattedData[i] == ':' || formattedData[i] == '=') {
				separatorIndex = i;
				break;
			}
		}
		if(separatorIndex < 0) { return std::string(); }
		
		return trimString(substring(formattedData, 0, separatorIndex));
	}
	
	std::string getVariableValue(const char * data) {
		return getVariableValue(std::string(data));
	}
	
	std::string getVariableValue(const std::string & data) {
		if(data.empty()) { return std::string(); }
		
		std::string formattedData = trimString(data);
		
		if(formattedData.empty()) { return std::string(); }
		
		int separatorIndex = -1;
		for(unsigned int i=0;i<formattedData.length();i++) {
			if(formattedData[i] == ':' || formattedData[i] == '=') {
				separatorIndex = i;
				break;
			}
		}
		if(separatorIndex < 0) { return std::string(); }
		
		return trimString(substring(formattedData, separatorIndex + 1, formattedData.length()));
	}
#endif // USE_STL

#if USE_QT
	QString getCommand(const char * data) {
		if(data == NULL) { return QString(); }

		return getCommand(QString(data));
	}

	QString getCommand(const QString & data) {
		if(data.isNull()) { return QString(); }

		QString newData = data.trimmed();
		if(newData.isEmpty()) { return QString(""); }

		int split = newData.indexOf(" ");
		if(split < 1) { return newData; }

		return substring(newData, 0, split).trimmed();
	}

	QString getArguments(const char * data) {
		return getArguments(QString(data));
	}

	QString getArguments(const QString & data) {
		if(data.isNull()) { return QString(); }

		QString newData = data.trimmed();
		if(newData.isEmpty()) { return QString(""); }

		int split = newData.indexOf(" ");
		if(split < 1 || split == newData.length() - 1) { return QString(""); }

		return substring(newData, split + 1, newData.length()).trimmed();
	}
#endif // USE_QT

	bool parseBoolean(const char * data, bool * valid) {
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

#if USE_QT
	bool parseBoolean(const QString & data, bool * valid) {
		if(data.isEmpty()) {
			if(valid != NULL) { *valid = false; }
			return false;
		}

		QByteArray bytes = data.toLocal8Bit();

		return parseBoolean(bytes.data(), valid);
	}
#endif // USE_QT

#if USE_STL
	bool parseBoolean(const std::string & data, bool * valid) {
		if(data.empty()) {
			if(valid != NULL) { *valid = false; }
			return false;
		}

		return parseBoolean(data.data(), valid);
	}
#endif // USE_STL

	int parseInteger(const char * data, bool * valid) {
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

		for(unsigned int i=0;i<stringLength(trimmedData);i++) {
			if(!(trimmedData[i] == '-' || (trimmedData[i] >= '0' && trimmedData[i] <= '9'))) {
				if(valid != NULL) { *valid = false; }
				delete [] trimmedData;
				return 0;
			}
		}

		if(valid != NULL) { *valid = true; }
		int value = atoi(trimmedData);

		delete [] trimmedData;

		return value;
#endif // USE_QT
	}

#if USE_QT
	int parseInteger(const QString & data, bool * valid) {
		if(data.isEmpty()) {
			if(valid != NULL) { *valid = false; }
			return 0;
		}

		QByteArray bytes = data.toLocal8Bit();

		return parseInteger(bytes.data(), valid);
	}
#endif // USE_QT

#if USE_STL
	int parseInteger(const std::string & data, bool * valid) {
		if(data.empty()) {
			if(valid != NULL) { *valid = false; }
			return 0;
		}

		return parseInteger(data.data(), valid);
	}
#endif // USE_STL

	float parseFloat(const char * data, bool * valid) {
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

		for(unsigned int i=0;i<stringLength(trimmedData);i++) {
			if(!(trimmedData[i] == '-' || trimmedData[i] == '.' || (trimmedData[i] >= '0' && trimmedData[i] <= '9'))) {
				if(valid != NULL) { *valid = false; }
				delete [] trimmedData;
				return 0.0f;
			}
		}

		if(valid != NULL) { *valid = true; }
		float value = static_cast<float>(atof(trimmedData));

		delete [] trimmedData;

		return value;
#endif // USE_QT
	}

#if USE_QT
	float parseFloat(const QString & data, bool * valid) {
		if(data.isEmpty()) {
			if(valid != NULL) { *valid = false; }
			return 0.0f;
		}

		QByteArray bytes = data.toLocal8Bit();

		return parseFloat(bytes.data(), valid);
	}
#endif // USE_QT

#if USE_STL
	float parseFloat(const std::string & data, bool * valid) {
		if(data.empty()) {
			if(valid != NULL) { *valid = false; }
			return 0.0f;
		}

		return parseFloat(data.data(), valid);
	}
#endif // USE_STL

#if USE_QT
	QDate parseDate(const char * date) {
		if(date == NULL) { return QDate(); }

		return parseDate(QString(date));
	}

	QDate parseDate(const QString & date) {
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
			if(compareStringsIgnoreCase(dateParts[0], monthStrings[i]) == 0) {
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

	QString dateToString(const QDate & date) {
		if(date.isNull()) { return QString(); }

		return QString("%1 %2, %3").arg(monthStrings[date.month() - 1]).arg(date.day()).arg(date.year());
	}
#endif // USE_QT

#if USE_QT
	QString generateFullPath(const char * path, const char * fileName) {
		return generateFullPath(QString(path), QString(fileName));
	}

	QString generateFullPath(const QString & path, const QString & fileName) {
		QString trimmedPath = path.trimmed();
		QString trimmedFileName = fileName.trimmed();
		QString fullPath("");

		if(trimmedPath.isEmpty())	 { return trimmedFileName; }
		if(trimmedFileName.isEmpty()) { return trimmedPath; }

		if(compareStrings(trimmedPath, ".") != 0) {
			fullPath.append(trimmedPath);

			if(trimmedPath[trimmedPath.length() - 1] != '/' && trimmedPath[trimmedPath.length() - 1] != '\\') {
				fullPath.append("/");
			}
		}

		fullPath.append(trimmedFileName);

		return fullPath;
	}
#elif USE_STL
	std::string generateFullPath(const char * path, const char * fileName) {
		return generateFullPath(std::string(path), std::string(fileName));
	}
	
	std::string generateFullPath(const std::string & path, const std::string & fileName) {
		std::string trimmedPath = trimString(path);
		std::string trimmedFileName = trimString(fileName);
		std::string fullPath("");
		
		if(trimmedPath.empty())	 { return trimmedFileName; }
		if(trimmedFileName.empty()) { return trimmedPath; }
		
		if(compareStrings(trimmedPath, ".") != 0) {
			fullPath.append(trimmedPath);
			
			if(trimmedPath[trimmedPath.length() - 1] != '/' && trimmedPath[trimmedPath.length() - 1] != '\\') {
				fullPath.append("/");
			}
		}
		
		fullPath.append(trimmedFileName);
		
		return fullPath;
	}
#endif // USE_STL

	char * getFileNameNoExtension(const char * fileName) {
		if(fileName == NULL) { return NULL; }
	
		int index = lastIndexOf(fileName, '.');
		if(index > 0) {
			return substring(fileName, 0, index);
		}
		return copyString(fileName);
	}
	
	char * getFileExtension(const char * fileName) {
		if(fileName == NULL) { return NULL; }
	
		int index = lastIndexOf(fileName, '.');
		if(index > 0) {
			return substring(fileName, index + 1, stringLength(fileName));
		}
		return NULL;
	}

	bool fileHasExtension(const char * fileName, const char * fileExtension) {
		if(fileName == NULL || fileExtension == NULL) { return false; }
	
		char * actualFileExtension = getFileExtension(fileName);
		bool fileExtensionMatches = actualFileExtension != NULL && compareStringsIgnoreCase(actualFileExtension, fileExtension) == 0;
		delete [] actualFileExtension;
		return fileExtensionMatches;
	}

#if USE_QT
	QString getFileNameNoExtension(const QString & fileName) {
		int index = fileName.lastIndexOf('.');
		if(index > 0) {
			return fileName.mid(0, index);
		}
		return fileName;
	}

	QString getFileExtension(const QString & fileName) {
		int index = fileName.lastIndexOf('.');
		if(index > 0) {
			return fileName.mid(index + 1, fileName.length() - index + 1);
		}
		return QString();
	}

	bool fileHasExtension(const QString & fileName, const QString & fileExtension) {
		QString actualFileExtension = getFileExtension(fileName);
		return !actualFileExtension.isNull() && QString::compare(actualFileExtension, fileExtension, Qt::CaseInsensitive) == 0;
	}

	void deleteFiles(const char * suffix) {
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

	void renameFiles(const char * fromSuffix, const char * toSuffix) {
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
#endif // USE_QT

	void clear() {
		system("cls");
	}

	void pause() {
		system("pause");
	}

}
