#include "Mod Collection/GameVersion.h"

const GameVersions::GameVersion GameVersions::defaultGameVersion = GameVersions::AtomicEdition;
const char * GameVersions::gameVersionStrings[] = { "Regular Version 1.3", "Atomic Edition 1.4/1.5", "eDuke 32", "Stand-Alone" };

bool GameVersions::isValid(GameVersion gameVersion) {
	return gameVersion > Invalid && gameVersion < NumberOfGameVersions;
}

bool GameVersions::isValid(int gameVersion) {
	return gameVersion > static_cast<int>(Invalid) && gameVersion < static_cast<int>(NumberOfGameVersions);
}

const char * GameVersions::toString(GameVersion gameVersion) {
	return toString(static_cast<int>(gameVersion));
}

const char * GameVersions::toString(int gameVersion) {
	if(!isValid(gameVersion)) { return "Invalid"; }

	return gameVersionStrings[gameVersion];
}

GameVersions::GameVersion GameVersions::parseFrom(const char * data) {
	if(data == NULL) { return Invalid; }

	GameVersion gameVersion = Invalid;

	char * gameVersionString = Utilities::trimCopyString(data);

	for(int i=0;i<static_cast<int>(NumberOfGameVersions);i++) {
		if(Utilities::compareStringsIgnoreCase(gameVersionString, gameVersionStrings[i]) == 0) {
			gameVersion = static_cast<GameVersion>(i);
			break;
		}
	}

	delete [] gameVersionString;

	return gameVersion;
}

GameVersions::GameVersion GameVersions::parseFrom(const QString & data) {
	QByteArray dataBytes = data.toLocal8Bit();
	return parseFrom(dataBytes.data());
}
