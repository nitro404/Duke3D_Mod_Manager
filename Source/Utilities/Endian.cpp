#include "Utilities/Endian.h"

const char * Endian::endiannessStrings[] = { "Big Endian", "Little Endian" };
const Endian::Endianness Endian::defaultEndianness = Endian::BigEndian;

bool Endian::isValid(Endianness endianness) {
	return endianness > Invalid && endianness < NumberOfEndianness;
}

bool Endian::isValid(int endianness) {
	return endianness > static_cast<int>(Invalid) && endianness < static_cast<int>(NumberOfEndianness);
}

const char * Endian::toString(Endianness endianness) {
	return toString(static_cast<int>(endianness));
}

const char * Endian::toString(int endianness) {
	if(!isValid(endianness)) { return "Invalid"; }

	return endiannessStrings[endianness];
}

Endian::Endianness Endian::parseFrom(const char * data) {
	if(data == NULL) { return Invalid; }

	Endianness endianness = Invalid;

	char * endiannessString = Utilities::trimCopyString(data);

	for(int i=0;i<static_cast<int>(NumberOfEndianness);i++) {
		if(Utilities::compareStringsIgnoreCase(endiannessString, endiannessStrings[i]) == 0) {
			endianness = static_cast<Endianness>(i);
			break;
		}
	}

	delete [] endiannessString;

	return endianness;
}

Endian::Endianness Endian::parseFrom(const QString & data) {
	QByteArray dataBytes = data.toLocal8Bit();
	return parseFrom(dataBytes.data());
}
