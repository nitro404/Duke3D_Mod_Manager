#ifndef MOD_MANAGER_MODE_H
#define MOD_MANAGER_MODE_H

#include "Utilities/Utilities.h"

namespace ModManagerModes {
	enum ModManagerMode {
		Invalid,
		DOSBox,
		Windows,
		NumberOfModes
	};
	
	extern const char * modeStrings[];
	extern const ModManagerMode defaultMode;
	
	bool isValid(int mode);
	bool isValid(ModManagerMode mode);
	const char * toString(ModManagerMode mode);
	const char * toString(int mode);
	ModManagerMode parseFrom(const char * data);
	ModManagerMode parseFrom(const QString & data);
}

#endif // MOD_MANAGER_MODE_H
