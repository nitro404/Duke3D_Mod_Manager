#ifndef ENDIAN_H
#define ENDIAN_H

#include "Utilities/Utilities.h"

namespace Endian {
	enum Endianness {
		Invalid = -1,
		BigEndian,
		LittleEndian,
		NumberOfEndianness
	};
	
	extern const char * endiannessStrings[];
	extern const Endianness defaultEndianness;
	
	bool isValid(Endianness endianness);
	bool isValid(int endianness);
	const char * toString(Endianness endianness);
	const char * toString(int endianness);
	Endianness parseFrom(const char * data);
	Endianness parseFrom(const QString & data);
}

#endif // ENDIAN_H
