#ifndef MOD_SORT_TYPE_H
#define MOD_SORT_TYPE_H

#include "Utilities/Utilities.h"
#include "Mod Collection/ModFilterType.h"

namespace ModSortTypes {
	enum ModSortType {
		Invalid = -1,
		Unsorted,
		Name,
		ReleaseDate,
		Rating,
		NumberOfMods,
		NumberOfSortTypes
	};
	
	extern const char * sortTypeStrings[];
	extern const ModSortType defaultSortType;
	
	bool isValid(ModSortType sortType);
	bool isValid(int sortType);
	bool isValidInContext(ModSortType sortType, ModFilterTypes::ModFilterType filterType, bool hasSelectedTeam = false, bool hasSelectedAuthor = false);
	bool isValidInContext(ModSortType sortType, int filterType, bool hasSelectedTeam = false, bool hasSelectedAuthor = false);
	bool isValidInContext(int sortType, ModFilterTypes::ModFilterType filterType, bool hasSelectedTeam = false, bool hasSelectedAuthor = false);
	bool isValidInContext(int sortType, int filterType, bool hasSelectedTeam = false, bool hasSelectedAuthor = false);
	const char * toString(ModSortType sortType);
	const char * toString(int sortType);
	ModSortType parseFrom(const char * data);
	ModSortType parseFrom(const QString & data);
}

#endif // MOD_SORT_TYPE_H
