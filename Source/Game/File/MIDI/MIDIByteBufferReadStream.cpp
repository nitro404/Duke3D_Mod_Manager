#include "MIDIByteBufferReadStream.h"

MIDIByteBufferReadStream::MIDIByteBufferReadStream(const ByteBuffer & data)
	: jdksmidi::MIDIFileReadStream()
	, m_data(std::make_shared<ByteBuffer>(data)) { }

MIDIByteBufferReadStream::MIDIByteBufferReadStream(std::unique_ptr<ByteBuffer> data)
	: jdksmidi::MIDIFileReadStream()
	, m_data(std::shared_ptr<ByteBuffer>(data.release())) { }

MIDIByteBufferReadStream::MIDIByteBufferReadStream(std::shared_ptr<ByteBuffer> data)
	: jdksmidi::MIDIFileReadStream()
	, m_data(data) { }

MIDIByteBufferReadStream::MIDIByteBufferReadStream(MIDIByteBufferReadStream && readStream) noexcept
	: jdksmidi::MIDIFileReadStream(std::move(readStream))
	, m_data(std::move(readStream.m_data)) { }

MIDIByteBufferReadStream::MIDIByteBufferReadStream(const MIDIByteBufferReadStream & readStream)
	: jdksmidi::MIDIFileReadStream(readStream)
	, m_data(readStream.m_data) { }

MIDIByteBufferReadStream & MIDIByteBufferReadStream::operator = (MIDIByteBufferReadStream && readStream) noexcept {
	if(this != &readStream) {
		jdksmidi::MIDIFileReadStream::operator = (std::move(readStream));

		m_data = std::move(readStream.m_data);
	}

	return *this;
}

MIDIByteBufferReadStream & MIDIByteBufferReadStream::operator = (const MIDIByteBufferReadStream & readStream) {
	jdksmidi::MIDIFileReadStream::operator = (readStream);

	m_data = readStream.m_data;

	return *this;
}

MIDIByteBufferReadStream::~MIDIByteBufferReadStream() { }

void MIDIByteBufferReadStream::Rewind() {
	if(m_data == nullptr) {
		return;
	}

	m_data->setReadOffset(0);
}

int MIDIByteBufferReadStream::ReadChar() {
	if(m_data == nullptr) {
		return -1;
	}

	std::optional<uint8_t> optionalByte(m_data->readUnsignedByte());

	return optionalByte.has_value() ? optionalByte.value() : -1;
}

bool MIDIByteBufferReadStream::isValid() const {
	return m_data != nullptr &&
		   m_data->isNotEmpty();
}

bool MIDIByteBufferReadStream::operator == (const MIDIByteBufferReadStream & readStream) const {
	return m_data == nullptr && readStream.m_data == nullptr ||
		   (m_data != nullptr && readStream.m_data != nullptr && *m_data == *readStream.m_data);
}

bool MIDIByteBufferReadStream::operator != (const MIDIByteBufferReadStream & readStream) const {
	return !operator == (readStream);
}
