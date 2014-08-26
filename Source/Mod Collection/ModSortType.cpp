#include "Mod Collection/ModSortType.h"

const char * ModSortTypes::sortTypeStrings[] = { "Invalid", "Unsorted", "Name", "Release Date", "Rating", "Number of Mods" };
const ModSortTypes::ModSortType ModSortTypes::defaultSortType = ModSortTypes::Unsorted;

bool ModSortTypes::isValid(int sortType) {
	return isValid(static_cast<ModSortTypes::ModSortType>(sortType));
}

bool ModSortTypes::isValid(ModSortType sortType) {
	return sortType > ModSortTypes::Invalid && sortType < ModSortTypes::NumberOfSortTypes;
}

const char * ModSortTypes::toString(ModSortType sortType) {
	return toString(static_cast<int>(sortType));
}

const char * ModSortTypes::toString(int sortType) {
	if(!isValid(sortType)) {
		return sortTypeStrings[0];
	}

	return sortTypeStrings[sortType];
}

ModSortTypes::ModSortType ModSortTypes::parseFrom(const char * data) {
	if(data == NULL) { return ModSortTypes::Invalid; }

	ModSortTypes::ModSortType sortType = Invalid;

	char * sortTypeString = Utilities::trimCopy(data);

	for(int i=0;i<ModSortTypes::NumberOfSortTypes;i++) {
		if(Utilities::compareStringsIgnoreCase(sortTypeString, sortTypeStrings[i]) == 0) {
			sortType = static_cast<ModSortTypes::ModSortType>(i);
			break;
		}
	}

	delete [] sortTypeString;

	return sortType;
}

ModSortTypes::ModSortType ModSortTypes::parseFrom(const QString & data) {
	QByteArray dataBytes = data.toLocal8Bit();
	const char * rawData = dataBytes.data();

	return parseFrom(rawData);
}
