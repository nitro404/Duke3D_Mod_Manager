#ifndef MOD_SORT_DIRECTION_H
#define MOD_SORT_DIRECTION_H

#include "Utilities/Utilities.h"

namespace ModSortDirections {
	enum ModSortDirection {
		Invalid,
		Ascending,
		Descending,
		NumberOfSortDirections
	};
	
	extern const char * sortDirectionStrings[];
	extern const ModSortDirection defaultSortDirection;
	
	bool isValid(int sortDirection);
	bool isValid(ModSortDirection sortDirection);
	const char * toString(ModSortDirection sortDirection);
	const char * toString(int sortDirection);
	ModSortDirection parseFrom(const char * data);
	ModSortDirection parseFrom(const QString & data);
}

#endif // MOD_SORT_DIRECTION_H
