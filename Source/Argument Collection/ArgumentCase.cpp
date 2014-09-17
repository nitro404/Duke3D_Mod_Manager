#include "Argument Collection/ArgumentCase.h"

const char * ArgumentCases::caseStrings[] = { "Original", "UpperCase", "LowerCase" };
const ArgumentCases::ArgumentCase ArgumentCases::defaultCase = ArgumentCases::OriginalCase;

bool ArgumentCases::isValid(ArgumentCase caseType) {
	return caseType > Invalid && caseType < NumberOfCases;
}

bool ArgumentCases::isValid(int caseType) {
	return caseType > static_cast<int>(Invalid) && caseType < static_cast<int>(NumberOfCases);
}

const char * ArgumentCases::toString(ArgumentCase caseType) {
	return toString(static_cast<int>(caseType));
}

const char * ArgumentCases::toString(int caseType) {
	if(!isValid(caseType)) { return "Invalid"; }

	return caseStrings[caseType];
}

ArgumentCases::ArgumentCase ArgumentCases::parseFrom(const char * data) {
	if(data == NULL) { return Invalid; }

	ArgumentCase caseType = Invalid;

	char * caseString = Utilities::trimCopyString(data);

	for(int i=0;i<static_cast<int>(NumberOfCases);i++) {
		if(Utilities::compareStringsIgnoreCase(caseString, caseStrings[i]) == 0) {
			caseType = static_cast<ArgumentCase>(i);
			break;
		}
	}

	delete [] caseString;

	return caseType;
}

ArgumentCases::ArgumentCase ArgumentCases::parseFrom(const QString & data) {
	QByteArray dataBytes = data.toLocal8Bit();
	return parseFrom(dataBytes.data());
}
