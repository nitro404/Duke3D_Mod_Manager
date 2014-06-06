#ifndef MOD_TYPE_H
#define MOD_TYPE_H

#include "Utilities/Utilities.h"

namespace ModTypes {
	enum ModType {
		Unknown,
		TotalConversion,
		PartialConversion,
		Dukematch,
		NumberOfModTypes
	};
	
	extern const char * modTypeStrings[];
	
	bool isValid(int type);
	bool isValid(ModType type);
	const char * toString(ModType type);
	const char * toString(int type);
	ModType parseFrom(const char * data);
	ModType parseFrom(const QString & data);
}

#endif // MOD_TYPE_H
