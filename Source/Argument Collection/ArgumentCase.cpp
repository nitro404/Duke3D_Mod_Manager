#include "Argument Collection/ArgumentCase.h"

const char * ArgumentCases::caseStrings[] = { "Invalid", "Original", "UpperCase", "LowerCase" };
const ArgumentCases::ArgumentCase ArgumentCases::defaultCase = ArgumentCases::OriginalCase;

bool ArgumentCases::isValid(int caseType) {
	return isValid(static_cast<ArgumentCases::ArgumentCase>(caseType));
}

bool ArgumentCases::isValid(ArgumentCase caseType) {
	return caseType > ArgumentCases::Invalid && caseType < ArgumentCases::NumberOfCases;
}

const char * ArgumentCases::toString(ArgumentCase caseType) {
	return toString(static_cast<int>(caseType));
}

const char * ArgumentCases::toString(int caseType) {
	if(!isValid(caseType)) {
		return caseStrings[0];
	}

	return caseStrings[caseType];
}

ArgumentCases::ArgumentCase ArgumentCases::parseFrom(const char * data) {
	if(data == NULL) { return ArgumentCases::Invalid; }

	ArgumentCases::ArgumentCase caseType = Invalid;

	char * caseString = Utilities::trimCopy(data);

	for(int i=0;i<ArgumentCases::NumberOfCases;i++) {
		if(Utilities::compareStringsIgnoreCase(caseString, caseStrings[i]) == 0) {
			caseType = static_cast<ArgumentCases::ArgumentCase>(i);
			break;
		}
	}

	delete [] caseString;

	return caseType;
}

ArgumentCases::ArgumentCase ArgumentCases::parseFrom(const QString & data) {
	QByteArray dataBytes = data.toLocal8Bit();
	const char * rawData = dataBytes.data();

	return parseFrom(rawData);
}
