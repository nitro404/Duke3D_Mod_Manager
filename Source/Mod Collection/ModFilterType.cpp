#include "Mod Collection/ModFilterType.h"

const char * ModFilterTypes::filterTypeStrings[] = { "Invalid", "None", "Favourites", "Teams", "Authors" };
const ModFilterTypes::ModFilterType ModFilterTypes::defaultFilterType = ModFilterTypes::None;

bool ModFilterTypes::isValid(int filterType) {
	return isValid(static_cast<ModFilterTypes::ModFilterType>(filterType));
}

bool ModFilterTypes::isValid(ModFilterType filterType) {
	return filterType > ModFilterTypes::Invalid && filterType < ModFilterTypes::NumberOfFilterTypes;
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
	if(data == NULL) { return ModFilterTypes::Invalid; }

	ModFilterTypes::ModFilterType filterType = Invalid;

	char * filterTypeString = Utilities::trimCopy(data);

	for(int i=0;i<ModFilterTypes::NumberOfFilterTypes;i++) {
		if(Utilities::compareStringsIgnoreCase(filterTypeString, filterTypeStrings[i]) == 0) {
			filterType = static_cast<ModFilterTypes::ModFilterType>(i);
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
