#ifndef MOD_FILTER_TYPE_H
#define MOD_FILTER_TYPE_H

#include "Utilities/Utilities.h"

namespace ModFilterTypes {
	enum ModFilterType {
		Invalid = -1,
		None,
		Favourites,
		Teams,
		Authors,
		NumberOfFilterTypes
	};
	
	extern const char * filterTypeStrings[];
	extern const ModFilterType defaultFilterType;
	
	bool isValid(ModFilterType filterType);
	bool isValid(int filterType);
	const char * toString(ModFilterType filterType);
	const char * toString(int filterType);
	ModFilterType parseFrom(const char * data);
	ModFilterType parseFrom(const QString & data);
}

#endif // MOD_FILTER_TYPE_H
