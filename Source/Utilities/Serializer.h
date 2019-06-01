#ifndef SERIALIZER_H
#define SERIALIZER_H

#include "Utilities/Endian.h"

namespace Serializer {

	QByteArray serializeBoolean(bool b);
	bool deserializeBoolean(const char * data, int length);
	bool deserializeBoolean(const QByteArray & data);
	QByteArray serializeShort(short s, Endian::Endianness e = Endian::defaultEndianness);
	short deserializeShort(const char * data, int length, Endian::Endianness e = Endian::defaultEndianness);
	short deserializeShort(const QByteArray & data, Endian::Endianness e = Endian::defaultEndianness);
	QByteArray serializeInteger(int i, Endian::Endianness e = Endian::defaultEndianness);
	int deserializeInteger(const char * data, int length, Endian::Endianness e = Endian::defaultEndianness);
	int deserializeInteger(const QByteArray & data, Endian::Endianness e = Endian::defaultEndianness);
	QByteArray serializeLong(long l, Endian::Endianness e = Endian::defaultEndianness);
	long deserializeLong(const char * data, int length, Endian::Endianness e = Endian::defaultEndianness);
	long deserializeLong(const QByteArray & data, Endian::Endianness e = Endian::defaultEndianness);
	QByteArray serializeByteCharacter(char c);
	char deserializeByteCharacter(const char data);
	char deserializeByteCharacter(const QByteArray & data);
	QByteArray serializeByteString(const char * s, int length);
	QString deserializeByteString(const char * data, int length);
	QString deserializeByteString(const QByteArray & data);

}

#endif // SERIALIZER_H
