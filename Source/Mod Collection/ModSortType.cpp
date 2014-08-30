#include "Mod Collection/ModSortType.h"

const char * ModSortTypes::sortTypeStrings[] = { "Invalid", "Unsorted", "Name", "Release Date", "Rating", "Number of Mods" };
const ModSortTypes::ModSortType ModSortTypes::defaultSortType = ModSortTypes::Unsorted;

bool ModSortTypes::isValid(int sortType) {
	return isValid(static_cast<ModSortType>(sortType));
}

bool ModSortTypes::isValid(ModSortType sortType) {
	return sortType > ModSortTypes::Invalid && sortType < ModSortTypes::NumberOfSortTypes;
}

bool ModSortTypes::isValidInContext(int sortType, int filterType, bool hasSelectedTeam, bool hasSelectedAuthor) {
	return isValidInContext(static_cast<ModSortType>(sortType), static_cast<ModFilterTypes::ModFilterType>(filterType), hasSelectedTeam, hasSelectedAuthor);
}

bool ModSortTypes::isValidInContext(ModSortType sortType, int filterType, bool hasSelectedTeam, bool hasSelectedAuthor) {
	return isValidInContext(sortType, static_cast<ModFilterTypes::ModFilterType>(filterType), hasSelectedTeam, hasSelectedAuthor);
}

bool ModSortTypes::isValidInContext(int sortType, ModFilterTypes::ModFilterType filterType, bool hasSelectedTeam, bool hasSelectedAuthor) {
	return isValidInContext(static_cast<ModSortType>(sortType), filterType, hasSelectedTeam, hasSelectedAuthor);
}

bool ModSortTypes::isValidInContext(ModSortType sortType, ModFilterTypes::ModFilterType filterType, bool hasSelectedTeam, bool hasSelectedAuthor) {
	if(!isValid(sortType) || !ModFilterTypes::isValid(filterType)) { return false; }

	if( filterType == ModFilterTypes::None ||
	    filterType == ModFilterTypes::Favourites ||
	   (filterType == ModFilterTypes::Teams && hasSelectedTeam) ||
	   (filterType == ModFilterTypes::Authors && hasSelectedAuthor)) {
		return sortType == ModSortTypes::Unsorted ||
			   sortType == ModSortTypes::Name ||
			   sortType == ModSortTypes::ReleaseDate ||
			   sortType == ModSortTypes::Rating;
	}
	else if((filterType == ModFilterTypes::Teams && !hasSelectedTeam) ||
			(filterType == ModFilterTypes::Authors && !hasSelectedAuthor)) {
		return sortType == ModSortTypes::Unsorted ||
			   sortType == ModSortTypes::Name ||
			   sortType == ModSortTypes::NumberOfMods;
	}
	return false;
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

	char * sortTypeString = Utilities::trimCopyString(data);

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
