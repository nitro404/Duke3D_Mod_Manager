#include "Mod Manager/ModManagerMode.h"

const char * ModManagerModes::modeStrings[] = { "DOSBox", "Windows" };
const ModManagerModes::ModManagerMode ModManagerModes::defaultMode = ModManagerModes::DOSBox;

bool ModManagerModes::isValid(ModManagerMode mode) {
	return mode > Invalid && mode < NumberOfModes;
}

bool ModManagerModes::isValid(int mode) {
	return mode > static_cast<int>(Invalid) && mode < static_cast<int>(NumberOfModes);
}

const char * ModManagerModes::toString(ModManagerMode mode) {
	return toString(static_cast<int>(mode));
}

const char * ModManagerModes::toString(int mode) {
	if(!isValid(mode)) { return "Invalid"; }

	return modeStrings[mode];
}

ModManagerModes::ModManagerMode ModManagerModes::parseFrom(const char * data) {
	if(data == NULL) { return Invalid; }

	ModManagerMode mode = Invalid;

	char * modeString = Utilities::trimCopyString(data);

	for(int i=0;i<static_cast<int>(NumberOfModes);i++) {
		if(Utilities::compareStringsIgnoreCase(modeString, modeStrings[i]) == 0) {
			mode = static_cast<ModManagerMode>(i);
			break;
		}
	}

	delete [] modeString;

	return mode;
}

ModManagerModes::ModManagerMode ModManagerModes::parseFrom(const QString & data) {
	QByteArray dataBytes = data.toLocal8Bit();
	return parseFrom(dataBytes.data());
}
