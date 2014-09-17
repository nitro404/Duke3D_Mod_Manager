#include "Mod Collection/ModSortType.h"

const char * ModSortTypes::sortTypeStrings[] = { "Unsorted", "Name", "Release Date", "Rating", "Number of Mods" };
const ModSortTypes::ModSortType ModSortTypes::defaultSortType = ModSortTypes::Unsorted;

bool ModSortTypes::isValid(ModSortType sortType) {
	return sortType > Invalid && sortType < NumberOfSortTypes;
}

bool ModSortTypes::isValid(int sortType) {
	return sortType > static_cast<int>(Invalid) && sortType < static_cast<int>(NumberOfSortTypes);
}

bool ModSortTypes::isValidInContext(ModSortType sortType, ModFilterTypes::ModFilterType filterType, bool hasSelectedTeam, bool hasSelectedAuthor) {
	if(!isValid(sortType) || !ModFilterTypes::isValid(filterType)) { return false; }

	if( filterType == ModFilterTypes::None ||
	    filterType == ModFilterTypes::Favourites ||
	   (filterType == ModFilterTypes::Teams && hasSelectedTeam) ||
	   (filterType == ModFilterTypes::Authors && hasSelectedAuthor)) {
		return sortType == Unsorted ||
			   sortType == Name ||
			   sortType == ReleaseDate ||
			   sortType == Rating;
	}
	else if((filterType == ModFilterTypes::Teams && !hasSelectedTeam) ||
			(filterType == ModFilterTypes::Authors && !hasSelectedAuthor)) {
		return sortType == Unsorted ||
			   sortType == Name ||
			   sortType == NumberOfMods;
	}
	return false;
}

bool ModSortTypes::isValidInContext(ModSortType sortType, int filterType, bool hasSelectedTeam, bool hasSelectedAuthor) {
	return isValidInContext(sortType, static_cast<ModFilterTypes::ModFilterType>(filterType), hasSelectedTeam, hasSelectedAuthor);
}

bool ModSortTypes::isValidInContext(int sortType, ModFilterTypes::ModFilterType filterType, bool hasSelectedTeam, bool hasSelectedAuthor) {
	return isValidInContext(static_cast<ModSortType>(sortType), filterType, hasSelectedTeam, hasSelectedAuthor);
}

bool ModSortTypes::isValidInContext(int sortType, int filterType, bool hasSelectedTeam, bool hasSelectedAuthor) {
	return isValidInContext(static_cast<ModSortType>(sortType), static_cast<ModFilterTypes::ModFilterType>(filterType), hasSelectedTeam, hasSelectedAuthor);
}

const char * ModSortTypes::toString(ModSortType sortType) {
	return toString(static_cast<int>(sortType));
}

const char * ModSortTypes::toString(int sortType) {
	if(!isValid(sortType)) { return "Invalid"; }

	return sortTypeStrings[sortType];
}

ModSortTypes::ModSortType ModSortTypes::parseFrom(const char * data) {
	if(data == NULL) { return Invalid; }

	ModSortType sortType = Invalid;

	char * sortTypeString = Utilities::trimCopyString(data);

	for(int i=0;i<static_cast<int>(NumberOfSortTypes);i++) {
		if(Utilities::compareStringsIgnoreCase(sortTypeString, sortTypeStrings[i]) == 0) {
			sortType = static_cast<ModSortType>(i);
			break;
		}
	}

	delete [] sortTypeString;

	return sortType;
}

ModSortTypes::ModSortType ModSortTypes::parseFrom(const QString & data) {
	QByteArray dataBytes = data.toLocal8Bit();
	return parseFrom(dataBytes.data());
}
