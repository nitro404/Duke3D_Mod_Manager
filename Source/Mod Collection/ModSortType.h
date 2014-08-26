#ifndef MOD_SORT_TYPE_H
#define MOD_SORT_TYPE_H

#include "Utilities/Utilities.h"

namespace ModSortTypes {
	enum ModSortType {
		Invalid,
		Unsorted,
		Name,
		ReleaseDate,
		Rating,
		NumberOfMods,
		NumberOfSortTypes
	};
	
	extern const char * sortTypeStrings[];
	extern const ModSortType defaultSortType;
	
	bool isValid(int sortType);
	bool isValid(ModSortType sortType);
	const char * toString(ModSortType sortType);
	const char * toString(int sortType);
	ModSortType parseFrom(const char * data);
	ModSortType parseFrom(const QString & data);
}

#endif // MOD_SORT_TYPE_H
