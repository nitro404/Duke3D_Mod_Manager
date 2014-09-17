#include "Mod Collection/ModSortDirection.h"

const char * ModSortDirections::sortDirectionStrings[] = { "Invalid", "Ascending", "Descending" };
const ModSortDirections::ModSortDirection ModSortDirections::defaultSortDirection = ModSortDirections::Ascending;

bool ModSortDirections::isValid(ModSortDirection sortDirection) {
	return sortDirection > Invalid && sortDirection < NumberOfSortDirections;
}

bool ModSortDirections::isValid(int sortDirection) {
	return sortDirection > static_cast<int>(Invalid) && sortDirection < static_cast<int>(NumberOfSortDirections);
}

const char * ModSortDirections::toString(ModSortDirection sortDirection) {
	return toString(static_cast<int>(sortDirection));
}

const char * ModSortDirections::toString(int sortDirection) {
	if(!isValid(sortDirection)) { return "Invalid"; }

	return sortDirectionStrings[sortDirection];
}

ModSortDirections::ModSortDirection ModSortDirections::parseFrom(const char * data) {
	if(data == NULL) { return Invalid; }

	ModSortDirection sortDirection = Invalid;

	char * sortDirectionString = Utilities::trimCopyString(data);

	for(int i=0;i<static_cast<int>(NumberOfSortDirections);i++) {
		if(Utilities::compareStringsIgnoreCase(sortDirectionString, sortDirectionStrings[i]) == 0) {
			sortDirection = static_cast<ModSortDirection>(i);
			break;
		}
	}

	delete [] sortDirectionString;

	return sortDirection;
}

ModSortDirections::ModSortDirection ModSortDirections::parseFrom(const QString & data) {
	QByteArray dataBytes = data.toLocal8Bit();
	return parseFrom(dataBytes.data());
}
