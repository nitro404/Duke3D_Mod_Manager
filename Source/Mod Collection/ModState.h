#ifndef MOD_STATE_H
#define MOD_STATE_H

#include "Utilities/Utilities.h"

namespace ModStates {

	enum ModState {
		Invalid = -1,
		Native,
		Converted,
		NumberOfModStates
	};
	
	extern const ModState defaultModState;
	extern const char * modStateStrings[];
	
	bool isValid(ModState state);
	bool isValid(int state);
	const char * toString(ModState state);
	const char * toString(int state);
	ModState parseFrom(const char * data);
	ModState parseFrom(const QString & data);

}

#endif // MOD_STATE_H
