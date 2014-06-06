#ifndef GAME_TYPE_H
#define GAME_TYPE_H

#include "Utilities/Utilities.h"

namespace GameTypes {
	enum GameType {
		Invalid,
		Game,
		Setup,
		Client,
		Server,
		NumberOfGameTypes
	};
	
	extern const char * gameTypeStrings[];
	extern const GameType defaultGameType;
	
	bool isValid(int type);
	bool isValid(GameType type);
	const char * toString(GameType type);
	const char * toString(int type);
	GameType parseFrom(const char * data);
	GameType parseFrom(const QString & data);
}

#endif // GAME_TYPE_H
