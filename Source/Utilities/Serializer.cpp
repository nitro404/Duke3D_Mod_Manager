#include "Utilities/Serializer.h"

namespace Serializer {
	
	// serialize a boolean to a byte array
	QByteArray serializeBoolean(bool b) {
		QByteArray data;
		data.append((char) (b ? 1 : 0));
		return data;
	}
	
	// deserialize a byte array back into a boolean
	bool deserializeBoolean(const char * data, int length) {
		if(data == NULL || length != 1) { return false; }
		return data[0] != 0;
	}

	// deserialize a byte array back into a boolean
	bool deserializeBoolean(const QByteArray & data) {
		if(data.isEmpty()) { return false; }
		return deserializeBoolean(data.data(), data.size());
	}
	
	// serialize a short to a byte array
	QByteArray serializeShort(short s, Endian::Endianness e) {
		if(!Endian::isValid(e)) { return NULL; }
		char data[2];
		data[e == Endian::BigEndian ? 0 : 1] = (char) (s >> 8);
		data[e == Endian::BigEndian ? 1 : 0] = (char) (s);
		return QByteArray(data, 2);
	}
	// deserialize a byte array back into a short
	short deserializeShort(const char * data, int length, Endian::Endianness e) {
		if(!Endian::isValid(e)) { return -1; }
		if(data == NULL || length != 2) { return -1; }
		return (short) (data[e == Endian::BigEndian ? 0 : 1] << 8
					 | (data[e == Endian::BigEndian ? 1 : 0] & 0xff));
	}

	// deserialize a byte array back into a short
	short deserializeShort(const QByteArray & data, Endian::Endianness e) {
		if(data.isEmpty()) { return -1; }
		return deserializeShort(data.data(), data.size(), e);
	}
	
	// serialize an integer to a byte array
	QByteArray serializeInteger(int i, Endian::Endianness e) {
		if(!Endian::isValid(e)) { return NULL; }
		char data[4];
		data[e == Endian::BigEndian ? 0 : 3] = (char) (i >> 24);
		data[e == Endian::BigEndian ? 1 : 2] = (char) (i >> 16);
		data[e == Endian::BigEndian ? 2 : 1] = (char) (i >> 8);
		data[e == Endian::BigEndian ? 3 : 0] = (char) (i);
		return QByteArray(data, 4);
	}
	
	// deserialize a byte array back into an integer
	int deserializeInteger(const char * data, int length, Endian::Endianness e) {
		if(!Endian::isValid(e)) { return -1; }
		if(data == NULL || length != 4) { return -1; }
		return (int) (data[e == Endian::BigEndian ? 0 : 3] << 24
				   | (data[e == Endian::BigEndian ? 1 : 2] & 0xff) << 16
				   | (data[e == Endian::BigEndian ? 2 : 1] & 0xff) << 8
				   | (data[e == Endian::BigEndian ? 3 : 0] & 0xff));
	}

	// deserialize a byte array back into an integer
	int deserializeInteger(const QByteArray & data, Endian::Endianness e) {
		if(data.isEmpty()) { return -1; }
		return deserializeInteger(data.data(), data.size(), e);
	}
	
	// serialize a long to a byte array
	QByteArray serializeLong(long l, Endian::Endianness e) {
		if(!Endian::isValid(e)) { return NULL; }
		char data[8];
		data[e == Endian::BigEndian ? 0 : 7] = (char) (l >> 56);
		data[e == Endian::BigEndian ? 1 : 6] = (char) (l >> 48);
		data[e == Endian::BigEndian ? 2 : 5] = (char) (l >> 40);
		data[e == Endian::BigEndian ? 3 : 4] = (char) (l >> 32);
		data[e == Endian::BigEndian ? 4 : 3] = (char) (l >> 24);
		data[e == Endian::BigEndian ? 5 : 2] = (char) (l >> 16);
		data[e == Endian::BigEndian ? 6 : 1] = (char) (l >> 8);
		data[e == Endian::BigEndian ? 7 : 0] = (char) (l);
		return QByteArray(data, 8);
	}
	
	// deserialize a byte array back into a long
	long deserializeLong(const char * data, int length, Endian::Endianness e) {
		if(!Endian::isValid(e)) { return -1L; }
		if(data == NULL || length != 8) { return -1; }
		long l = 0;
		int x;
		for(int i=0;i<8;++i) {
			x = e == Endian::BigEndian ? i : 7 - i;
			l |= ((long) data[x] & 0xff) << ((8-x-1) << 3);
		}
		return l;
	}
	
	// deserialize a byte array back into an long
	long deserializeLong(const QByteArray & data, Endian::Endianness e) {
		if(data.isEmpty()) { return -1; }
		return deserializeLong(data.data(), data.size(), e);
	}
	
	// serialize a single byte character to a byte
	QByteArray serializeByteCharacter(char c) {
		QByteArray data;
		data.append(c);
		return data;
	}
	
	// deserialize a byte into a single byte character
	char deserializeByteCharacter(char data) {
		return (char) (data & 0xff);
	}
	
	// deserialize a byte into a single byte character
	char deserializeByteCharacter(const QByteArray & data) {
		if(data.size() < 1) { return '\0'; }
		return deserializeByteCharacter(data.data()[0]);
	}
	
	// serialize the specified byte string
	QByteArray serializeByteString(const char * s, int length) {
		if(length == 0) { return NULL; }
		
		QByteArray data;
		
		// serialize and store the bytes for each character in the string
		for(int i=0;i<length;i++) {
			data.append((char) s[i]);
		}
		
		return data;
	}
	
	// de-serialize the specified byte string
	QString deserializeByteString(const char * data, int length) {
		if(data == NULL || length == 0) { return QString(); }
		
		QString s("");
		char c = '\0';
		
		for(int i=0;i<length;i++) {
			c = (char) (data[i] & 0xff);
			if(c == '\0') { break; }
			s.append(c);
		}
		
		return s;
	}

	// de-serialize the specified byte string
	QString deserializeByteString(const QByteArray & data) {
		if(data.isEmpty()) { return QString(); }
		return deserializeByteString(data.data(), data.size());
	}
	
}
