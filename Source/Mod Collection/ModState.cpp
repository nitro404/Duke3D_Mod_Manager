#include "Mod Collection/ModState.h"

const ModStates::ModState ModStates::defaultModState = ModStates::Native;
const char * ModStates::modStateStrings[] = { "Native", "Converted" };

bool ModStates::isValid(ModState state) {
	return state > Invalid && state < NumberOfModStates;
}

bool ModStates::isValid(int state) {
	return state > static_cast<int>(Invalid) && state < static_cast<int>(NumberOfModStates);
}

const char * ModStates::toString(ModState state) {
	return toString(static_cast<int>(state));
}

const char * ModStates::toString(int state) {
	if(!isValid(state)) { return "Invalid"; }

	return modStateStrings[state];
}

ModStates::ModState ModStates::parseFrom(const char * data) {
	if(data == NULL) { return Invalid; }

	ModState state = Invalid;

	char * stateString = Utilities::trimCopyString(data);

	for(int i=0;i<static_cast<int>(NumberOfModStates);i++) {
		if(Utilities::compareStringsIgnoreCase(stateString, modStateStrings[i]) == 0) {
			state = static_cast<ModState>(i);
			break;
		}
	}

	delete [] stateString;

	return state;
}

ModStates::ModState ModStates::parseFrom(const QString & data) {
	QByteArray dataBytes = data.toLocal8Bit();
	return parseFrom(dataBytes.data());
}
