#include "MIDI.h"

#include "MIDIByteBufferReadStream.h"
#include "MIDIByteBufferWriteStream.h"

#include <Utilities/FileUtilities.h>

#include <fmt/core.h>
#include <jdksmidi/world.h>
#include <jdksmidi/fileread.h>
#include <jdksmidi/filereadmultitrack.h>
#include <jdksmidi/fileshow.h>
#include <jdksmidi/filewritemultitrack.h>
#include <jdksmidi/multitrack.h>
#include <jdksmidi/track.h>
#include <jdksmidi/utils.h>
#include <spdlog/spdlog.h>

MIDI::MIDI(const std::string & filePath)
	: GameFile(filePath)
	, m_division(0) { }

MIDI::MIDI(std::unique_ptr<jdksmidi::MIDIMultiTrack> tracks, int division)
	: GameFile()
	, m_tracks(std::move(tracks))
	, m_division(division) { }

MIDI::MIDI(MIDI && midi) noexcept
	: GameFile(std::move(midi))
	, m_tracks(std::move(midi.m_tracks))
	, m_division(midi.m_division) { }

MIDI::MIDI(const MIDI & midi)
	: GameFile(midi)
	, m_tracks(std::make_unique<jdksmidi::MIDIMultiTrack>(*midi.m_tracks))
	, m_division(midi.m_division) { }

MIDI & MIDI::operator = (MIDI && midi) noexcept {
	if(this != &midi) {
		GameFile::operator = (std::move(midi));

		m_tracks = std::move(midi.m_tracks);
		m_division = midi.m_division;
	}

	return *this;
}

MIDI & MIDI::operator = (const MIDI & midi) {
	GameFile::operator = (midi);

	m_tracks = std::make_unique<jdksmidi::MIDIMultiTrack>(*midi.m_tracks);
	m_division = midi.m_division;

	return *this;
}

MIDI::~MIDI() { }

std::chrono::milliseconds MIDI::getDuration() const {
	return std::chrono::milliseconds(static_cast<int64_t>(jdksmidi::GetMusicDurationInSeconds(*m_tracks) * 1000.0));
}

int MIDI::numberOfTracks() const {
	return m_tracks->GetNumTracks();
}

int MIDI::numberOfTracksWithEvents() const {
	return m_tracks->GetNumTracksWithEvents();
}

int MIDI::numberOfEvents() const {
	return m_tracks->GetNumEvents();
}

int MIDI::getClocksPerBeat() const {
	return m_tracks->GetNumEvents();
}

int MIDI::getDivision() const {
	return m_division;
}

std::unique_ptr<MIDI> MIDI::readFrom(const ByteBuffer & byteBuffer) {
	if(byteBuffer.isEmpty()) {
		return nullptr;
	}

	return readFrom(MIDIByteBufferReadStream(byteBuffer));
}

std::unique_ptr<MIDI> MIDI::loadFrom(const std::string & filePath) {
	jdksmidi::MIDIFileReadStreamFile fileReadStream(filePath.c_str());

	if(!fileReadStream.IsValid()) {
		spdlog::error("Failed to open MIDI file for reading: '{}'.", filePath);
		return nullptr;
	}

	return readFrom(fileReadStream);
}

std::unique_ptr<MIDI> MIDI::readFrom(jdksmidi::MIDIFileReadStream & readStream) {
	std::unique_ptr<jdksmidi::MIDIMultiTrack> tracks(std::make_unique<jdksmidi::MIDIMultiTrack>(1));
	jdksmidi::MIDIFileReadMultiTrack trackLoader(tracks.get());
	jdksmidi::MIDIFileRead fileReader(&readStream, &trackLoader);
	int numberOfTracks = fileReader.ReadNumTracks();
	tracks->ClearAndResize(numberOfTracks);

	if(!fileReader.Parse()) {
		spdlog::error("Failed to parse MIDI file.");
		return nullptr;
	}

	return std::unique_ptr<MIDI>(new MIDI(std::move(tracks), fileReader.GetDivision()));
}

bool MIDI::writeTo(ByteBuffer & byteBuffer) const {
	if(!isValid()) {
		return false;
	}

	MIDIByteBufferWriteStream writeStream(byteBuffer);
	jdksmidi::MIDIFileWriteMultiTrack multiTrackWriter(m_tracks.get(), &writeStream);
	int numberOfTracks = m_tracks->GetNumTracksWithEvents();

	return multiTrackWriter.Write(numberOfTracks, m_division);
}

void MIDI::addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const {
	if(!isValid()) {
		return;
	}

	double durationSeconds = jdksmidi::GetMusicDurationInSeconds(*m_tracks);

	metadata.push_back({ "Duration", fmt::format("{:.2f} Seconds", durationSeconds) });
	metadata.push_back({ "Number of Tracks", std::to_string(m_tracks->GetNumTracks()) });
	metadata.push_back({ "Number of Tracks With Events", std::to_string(m_tracks->GetNumTracksWithEvents()) });
	metadata.push_back({ "Number of Events", std::to_string(m_tracks->GetNumEvents()) });
	metadata.push_back({ "Clocks per Beat", std::to_string(m_tracks->GetClksPerBeat()) });
}

Endianness MIDI::getEndianness() const {
	return ENDIANNESS;
}

size_t MIDI::getSizeInBytes() const {
	ByteBuffer data;

	if(!writeTo(data)) {
		return 0;
	}

	return data.getSize();
}

bool MIDI::isValid(bool verifyParent) const {
	return m_tracks != nullptr;
}

bool MIDI::operator == (const MIDI & midi) const {
	return m_tracks.get() == midi.m_tracks.get() &&
		   m_division == midi.m_division;
}

bool MIDI::operator != (const MIDI & midi) const {
	return !operator == (midi);
}
