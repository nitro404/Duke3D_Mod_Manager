#include "Mod Collection/ModType.h"

const char * ModTypes::modTypeStrings[] = { "Unknown", "Total Conversion", "Partial Conversion", "Dukematch" };

bool ModTypes::isValid(int type) {
	return isValid(static_cast<ModType>(type));
}

bool ModTypes::isValid(ModType type) {
	return type >= ModTypes::Unknown && type < ModTypes::NumberOfModTypes;
}

const char * ModTypes::toString(ModType type) {
	return toString(static_cast<int>(type));
}

const char * ModTypes::toString(int type) {
	if(!isValid(type)) {
		return modTypeStrings[0];
	}

	return modTypeStrings[type];
}

ModTypes::ModType ModTypes::parseFrom(const char * data) {
	if(data == NULL) { return Unknown; }

	ModType type = Unknown;

	char * typeString = Utilities::trimCopy(data);

	for(int i=0;i<NumberOfModTypes;i++) {
		if(Utilities::compareStringsIgnoreCase(typeString, modTypeStrings[i]) == 0) {
			type = static_cast<ModType>(i);
			break;
		}
	}

	delete [] typeString;

	return type;
}

ModTypes::ModType ModTypes::parseFrom(const QString & data) {
	QByteArray dataBytes = data.toLocal8Bit();
	const char * rawData = dataBytes.data();

	return parseFrom(rawData);
}
