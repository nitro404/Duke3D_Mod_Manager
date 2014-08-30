#include "Mod Manager/ModManagerMode.h"

const char * ModManagerModes::modeStrings[] = { "Invalid", "DOSBox", "Windows" };
const ModManagerModes::ModManagerMode ModManagerModes::defaultMode = ModManagerModes::DOSBox;

bool ModManagerModes::isValid(int mode) {
	return isValid(static_cast<ModManagerModes::ModManagerMode>(mode));
}

bool ModManagerModes::isValid(ModManagerMode mode) {
	return mode > ModManagerModes::Invalid && mode < ModManagerModes::NumberOfModes;
}

const char * ModManagerModes::toString(ModManagerMode mode) {
	return toString(static_cast<int>(mode));
}

const char * ModManagerModes::toString(int mode) {
	if(!isValid(mode)) {
		return modeStrings[0];
	}

	return modeStrings[mode];
}

ModManagerModes::ModManagerMode ModManagerModes::parseFrom(const char * data) {
	if(data == NULL) { return ModManagerModes::Invalid; }

	ModManagerModes::ModManagerMode mode = Invalid;

	char * modeString = Utilities::trimCopyString(data);

	for(int i=0;i<ModManagerModes::NumberOfModes;i++) {
		if(Utilities::compareStringsIgnoreCase(modeString, modeStrings[i]) == 0) {
			mode = static_cast<ModManagerModes::ModManagerMode>(i);
			break;
		}
	}

	delete [] modeString;

	return mode;
}

ModManagerModes::ModManagerMode ModManagerModes::parseFrom(const QString & data) {
	QByteArray dataBytes = data.toLocal8Bit();
	const char * rawData = dataBytes.data();

	return parseFrom(rawData);
}
