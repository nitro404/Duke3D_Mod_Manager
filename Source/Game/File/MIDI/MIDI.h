#ifndef _MIDI_H_
#define _MIDI_H_

#include "../GameFile.h"

#include <ByteBuffer.h>
#include <Endianness.h>

#include <chrono>
#include <memory>
#include <string>

namespace jdksmidi {
	class MIDIFileReadStream;
	class MIDIMultiTrack;
}

class MIDI final : public GameFile {
public:
	MIDI(const std::string & filePath = {});
	MIDI(MIDI && midi) noexcept;
	MIDI(const MIDI & midi);
	MIDI & operator = (MIDI && midi) noexcept;
	MIDI & operator = (const MIDI & midi);
	virtual ~MIDI();

	std::chrono::milliseconds getDuration() const;
	int numberOfTracks() const;
	int numberOfTracksWithEvents() const;
	int numberOfEvents() const;
	int getClocksPerBeat() const;
	int getDivision() const;

	static std::unique_ptr<MIDI> readFrom(const ByteBuffer & byteBuffer);
	static std::unique_ptr<MIDI> loadFrom(const std::string & filePath);
	virtual bool writeTo(ByteBuffer & byteBuffer) const override;
	virtual void addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const override;
	virtual Endianness getEndianness() const override;
	virtual size_t getSizeInBytes() const override;
	virtual bool isValid(bool verifyParent = true) const override;

	bool operator == (const MIDI & midi) const;
	bool operator != (const MIDI & midi) const;

	static constexpr Endianness ENDIANNESS = Endianness::BigEndian;

private:
	MIDI(std::unique_ptr<jdksmidi::MIDIMultiTrack> tracks, int division);

	static std::unique_ptr<MIDI> readFrom(jdksmidi::MIDIFileReadStream & readStream);

	std::unique_ptr<jdksmidi::MIDIMultiTrack> m_tracks;
	int m_division;
};

#endif // _MIDI_H_
