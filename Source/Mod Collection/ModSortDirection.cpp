#include "Mod Collection/ModSortDirection.h"

const char * ModSortDirections::sortDirectionStrings[] = { "Invalid", "Ascending", "Descending" };
const ModSortDirections::ModSortDirection ModSortDirections::defaultSortDirection = ModSortDirections::Ascending;

bool ModSortDirections::isValid(int sortDirection) {
	return isValid(static_cast<ModSortDirections::ModSortDirection>(sortDirection));
}

bool ModSortDirections::isValid(ModSortDirection sortDirection) {
	return sortDirection > ModSortDirections::Invalid && sortDirection < ModSortDirections::NumberOfSortDirections;
}

const char * ModSortDirections::toString(ModSortDirection sortDirection) {
	return toString(static_cast<int>(sortDirection));
}

const char * ModSortDirections::toString(int sortDirection) {
	if(!isValid(sortDirection)) {
		return sortDirectionStrings[0];
	}

	return sortDirectionStrings[sortDirection];
}

ModSortDirections::ModSortDirection ModSortDirections::parseFrom(const char * data) {
	if(data == NULL) { return ModSortDirections::Invalid; }

	ModSortDirections::ModSortDirection sortDirection = Invalid;

	char * sortDirectionString = Utilities::trimCopyString(data);

	for(int i=0;i<ModSortDirections::NumberOfSortDirections;i++) {
		if(Utilities::compareStringsIgnoreCase(sortDirectionString, sortDirectionStrings[i]) == 0) {
			sortDirection = static_cast<ModSortDirections::ModSortDirection>(i);
			break;
		}
	}

	delete [] sortDirectionString;

	return sortDirection;
}

ModSortDirections::ModSortDirection ModSortDirections::parseFrom(const QString & data) {
	QByteArray dataBytes = data.toLocal8Bit();
	const char * rawData = dataBytes.data();

	return parseFrom(rawData);
}
