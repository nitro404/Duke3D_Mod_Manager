#ifndef _MIDI_BYTE_BUFFER_WRITE_STREAM_H_
#define _MIDI_BYTE_BUFFER_WRITE_STREAM_H_

#include <ByteBuffer.h>

#include <jdksmidi/world.h>
#include <jdksmidi/filewrite.h>
#include <jdksmidi/multitrack.h>

#include <memory>

class MIDIByteBufferWriteStream final : public jdksmidi::MIDIFileWriteStream {
public:
	MIDIByteBufferWriteStream(ByteBuffer & byteBuffer);
	MIDIByteBufferWriteStream(MIDIByteBufferWriteStream && writeStream) noexcept;
	MIDIByteBufferWriteStream(const MIDIByteBufferWriteStream & writeStream);
	MIDIByteBufferWriteStream & operator = (MIDIByteBufferWriteStream && writeStream) noexcept;
	MIDIByteBufferWriteStream & operator = (const MIDIByteBufferWriteStream & writeStream);
	virtual ~MIDIByteBufferWriteStream();

	ByteBuffer & getData();
	const ByteBuffer & getData() const;

	// MIDIFileWriteStream Virtuals
	virtual long Seek(long offset, int seekOrigin = SEEK_SET) override;
	virtual int WriteChar(int character) override;

	bool operator == (const MIDIByteBufferWriteStream & writeStream) const;
	bool operator != (const MIDIByteBufferWriteStream & writeStream) const;

private:
	ByteBuffer & m_byteBuffer;
};

#endif // _MIDI_BYTE_BUFFER_WRITE_STREAM_H_
