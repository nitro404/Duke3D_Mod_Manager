#include "Mod Collection/ModFilterType.h"

const char * ModFilterTypes::filterTypeStrings[] = { "Invalid", "None", "Favourites", "Teams", "Authors" };
const ModFilterTypes::ModFilterType ModFilterTypes::defaultFilterType = ModFilterTypes::None;

bool ModFilterTypes::isValid(int filterType) {
	return filterType > static_cast<int>(Invalid) && filterType < static_cast<int>(NumberOfFilterTypes);
}

bool ModFilterTypes::isValid(ModFilterType filterType) {
	return filterType > Invalid && filterType < NumberOfFilterTypes;
}

const char * ModFilterTypes::toString(ModFilterType filterType) {
	return toString(static_cast<int>(filterType));
}

const char * ModFilterTypes::toString(int filterType) {
	if(!isValid(filterType)) {
		return filterTypeStrings[0];
	}

	return filterTypeStrings[filterType];
}

ModFilterTypes::ModFilterType ModFilterTypes::parseFrom(const char * data) {
	if(data == NULL) { return Invalid; }

	ModFilterType filterType = Invalid;

	char * filterTypeString = Utilities::trimCopyString(data);

	for(int i=static_cast<int>(Invalid)+1;i<static_cast<int>(NumberOfFilterTypes);i++) {
		if(Utilities::compareStringsIgnoreCase(filterTypeString, filterTypeStrings[i]) == 0) {
			filterType = static_cast<ModFilterType>(i);
			break;
		}
	}

	delete [] filterTypeString;

	return filterType;
}

ModFilterTypes::ModFilterType ModFilterTypes::parseFrom(const QString & data) {
	QByteArray dataBytes = data.toLocal8Bit();
	const char * rawData = dataBytes.data();

	return parseFrom(rawData);
}
