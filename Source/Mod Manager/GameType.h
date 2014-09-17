#ifndef GAME_TYPE_H
#define GAME_TYPE_H

#include "Utilities/Utilities.h"

namespace GameTypes {
	enum GameType {
		Invalid = -1,
		Game,
		Setup,
		Client,
		Server,
		NumberOfGameTypes
	};
	
	extern const char * gameTypeStrings[];
	extern const GameType defaultGameType;
	
	bool isValid(GameType type);
	bool isValid(int type);
	const char * toString(GameType type);
	const char * toString(int type);
	GameType parseFrom(const char * data);
	GameType parseFrom(const QString & data);
}

#endif // GAME_TYPE_H
