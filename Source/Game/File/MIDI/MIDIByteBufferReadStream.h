#ifndef _MIDI_BYTE_BUFFER_READ_STREAM_H_
#define _MIDI_BYTE_BUFFER_READ_STREAM_H_

#include <ByteBuffer.h>

#include <jdksmidi/world.h>
#include <jdksmidi/midi.h>
#include <jdksmidi/fileread.h>

#include <memory>

class MIDIByteBufferReadStream final : public jdksmidi::MIDIFileReadStream {
public:
	MIDIByteBufferReadStream(const ByteBuffer & data);
	MIDIByteBufferReadStream(std::unique_ptr<ByteBuffer> data);
	MIDIByteBufferReadStream(std::shared_ptr<ByteBuffer> data);
	MIDIByteBufferReadStream(MIDIByteBufferReadStream && readStream) noexcept;
	MIDIByteBufferReadStream(const MIDIByteBufferReadStream & readStream);
	MIDIByteBufferReadStream & operator = (MIDIByteBufferReadStream && readStream) noexcept;
	MIDIByteBufferReadStream & operator = (const MIDIByteBufferReadStream & readStream);
	virtual ~MIDIByteBufferReadStream();

	// MIDIFileReadStream Virtuals
	virtual void Rewind() override;
	virtual int ReadChar() override;

	bool isValid() const;

	bool operator == (const MIDIByteBufferReadStream & readStream) const;
	bool operator != (const MIDIByteBufferReadStream & readStream) const;

private:
	std::shared_ptr<ByteBuffer> m_data;
};

#endif // _MIDI_BYTE_BUFFER_READ_STREAM_H_
