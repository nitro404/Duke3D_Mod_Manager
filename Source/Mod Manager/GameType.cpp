#include "Mod Manager/GameType.h"

const char * GameTypes::gameTypeStrings[] = { "Invalid", "Game", "Setup", "Client", "Server" };
const GameTypes::GameType GameTypes::defaultGameType = GameTypes::Game;

bool GameTypes::isValid(int type) {
	return isValid(static_cast<GameType>(type));
}

bool GameTypes::isValid(GameType type) {
	return type > GameTypes::Invalid && type < GameTypes::NumberOfGameTypes;
}

const char * GameTypes::toString(GameType type) {
	return toString(static_cast<int>(type));
}

const char * GameTypes::toString(int type) {
	if(!isValid(type)) {
		return gameTypeStrings[0];
	}

	return gameTypeStrings[type];
}

GameTypes::GameType GameTypes::parseFrom(const char * data) {
	if(data == NULL) { return Invalid; }

	GameType type = Invalid;

	char * typeString = Utilities::trimCopy(data);

	for(int i=0;i<NumberOfGameTypes;i++) {
		if(Utilities::compareStringsIgnoreCase(typeString, gameTypeStrings[i]) == 0) {
			type = static_cast<GameType>(i);
			break;
		}
	}

	delete [] typeString;

	return type;
}

GameTypes::GameType GameTypes::parseFrom(const QString & data) {
	QByteArray dataBytes = data.toLocal8Bit();
	const char * rawData = dataBytes.data();

	return parseFrom(rawData);
}
