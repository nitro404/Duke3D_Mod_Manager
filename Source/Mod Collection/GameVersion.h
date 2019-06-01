#ifndef GAME_VERSION_H
#define GAME_VERSION_H

#include "Utilities/Utilities.h"

namespace GameVersions {
	
	enum GameVersion {
		Invalid = -1,
		RegularVersion,
		AtomicEdition,
		eDuke32,
		StandAlone,
		NumberOfGameVersions
	};
	
	extern const GameVersion defaultGameVersion;
	extern const char * gameVersionStrings[];
	
	bool isValid(GameVersion gameVersion);
	bool isValid(int gameVersion);
	const char * toString(GameVersion gameVersion);
	const char * toString(int gameVersion);
	GameVersion parseFrom(const char * data);
	GameVersion parseFrom(const QString & data);

}

#endif // GAME_VERSION_H
