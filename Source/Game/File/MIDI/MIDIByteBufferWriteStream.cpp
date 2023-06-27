#include "MIDIByteBufferWriteStream.h"

MIDIByteBufferWriteStream::MIDIByteBufferWriteStream(ByteBuffer & byteBuffer)
	: m_byteBuffer(byteBuffer) { }

MIDIByteBufferWriteStream::MIDIByteBufferWriteStream(MIDIByteBufferWriteStream && writeStream) noexcept
	: m_byteBuffer(writeStream.m_byteBuffer) { }

MIDIByteBufferWriteStream::MIDIByteBufferWriteStream(const MIDIByteBufferWriteStream & writeStream)
	: m_byteBuffer(writeStream.m_byteBuffer) { }

MIDIByteBufferWriteStream & MIDIByteBufferWriteStream::operator = (MIDIByteBufferWriteStream && writeStream) noexcept {
	if(this != &writeStream) {
		m_byteBuffer = std::move(writeStream.m_byteBuffer);
	}

	return *this;
}

MIDIByteBufferWriteStream & MIDIByteBufferWriteStream::operator = (const MIDIByteBufferWriteStream & writeStream) {
	m_byteBuffer = writeStream.m_byteBuffer;

	return *this;
}

MIDIByteBufferWriteStream::~MIDIByteBufferWriteStream() { }

ByteBuffer & MIDIByteBufferWriteStream::getData() {
	return m_byteBuffer;
}

const ByteBuffer & MIDIByteBufferWriteStream::getData() const {
	return m_byteBuffer;
}

long MIDIByteBufferWriteStream::Seek(long offset, int seekOrigin) {
	switch(seekOrigin) {
		case SEEK_SET: {
			m_byteBuffer.setReadOffset(offset);
			break;
		}
		case SEEK_CUR: {
			m_byteBuffer.setReadOffset(m_byteBuffer.getReadOffset() + offset);
			break;
		}
		case SEEK_END: {
			m_byteBuffer.setReadOffset(m_byteBuffer.getSize() + offset);
			break;
		}
		default: {
			return -1;
		}
	}

	return 0;
}

int MIDIByteBufferWriteStream::WriteChar(int character) {
	if(!m_byteBuffer.writeByte(static_cast<uint8_t>(character))) {
		return -1;
	}

	return 0;
}

bool MIDIByteBufferWriteStream::operator == (const MIDIByteBufferWriteStream & writeStream) const {
	return m_byteBuffer == writeStream.m_byteBuffer;
}

bool MIDIByteBufferWriteStream::operator != (const MIDIByteBufferWriteStream & writeStream) const {
	return !operator == (writeStream);
}
