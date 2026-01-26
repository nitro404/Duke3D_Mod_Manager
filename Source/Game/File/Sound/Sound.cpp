#include "Sound.h"

#include <Utilities/StringUtilities.h>

#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

static sf_count_t getByteBufferLength(void * userData) {
	return reinterpret_cast<ByteBuffer *>(userData)->getSize();
}

static sf_count_t seekByteBufferReadOffset(sf_count_t offset, int seekOrigin, void * userData) {
	ByteBuffer * byteBuffer = reinterpret_cast<ByteBuffer *>(userData);

	size_t newOffset = 0;

	switch(seekOrigin) {
		case SEEK_SET: {
			newOffset = offset;
			break;
		}
		case SEEK_CUR: {
			newOffset = byteBuffer->getReadOffset() + offset;
			break;
		}
		case SEEK_END: {
			newOffset = byteBuffer->getSize() + offset;
			break;
		}
		default: {
			return -1;
		}
	}

	if(newOffset > byteBuffer->getSize()) {
		return -1;
	}

	byteBuffer->setReadOffset(newOffset);

	return newOffset;
}

static sf_count_t seekByteBufferWriteOffset(sf_count_t offset, int seekOrigin, void * userData) {
	ByteBuffer * byteBuffer = reinterpret_cast<ByteBuffer *>(userData);

	switch(seekOrigin) {
		case SEEK_SET: {
			byteBuffer->setWriteOffset(offset);
			break;
		}
		case SEEK_CUR: {
			byteBuffer->setWriteOffset(byteBuffer->getWriteOffset() + offset);
			break;
		}
		case SEEK_END: {
			byteBuffer->setWriteOffset(byteBuffer->getSize() + offset);
			break;
		}
		default: {
			return -1;
		}
	}

	return byteBuffer->getWriteOffset();
}

static sf_count_t readFromByteBuffer(void * data, sf_count_t size, void * userData) {
	ByteBuffer * byteBuffer = reinterpret_cast<ByteBuffer *>(userData);

	size_t numberOfBytesRemaining = byteBuffer->numberOfBytesRemaining();

	if(numberOfBytesRemaining == 0) {
		return 0;
	}

	size_t numberOfBytesToRead = std::min(numberOfBytesRemaining, static_cast<size_t>(size));

	std::memcpy(data, byteBuffer->getRawData() + byteBuffer->getReadOffset(), numberOfBytesToRead);

	byteBuffer->skipReadBytes(numberOfBytesToRead);

	return numberOfBytesToRead;
}

static sf_count_t writeToByteBuffer(const void * data, sf_count_t size, void * userData) {
	ByteBuffer * byteBuffer = reinterpret_cast<ByteBuffer *>(userData);

	if(!byteBuffer->writeBytes(reinterpret_cast<const uint8_t *>(data), size)) {
		return 0;
	}

	return size;
}

static sf_count_t getByteBufferReadOffset(void * userData) {
	return reinterpret_cast<ByteBuffer *>(userData)->getReadOffset();
}

static sf_count_t getByteBufferWriteOffset(void * userData) {
	return reinterpret_cast<ByteBuffer *>(userData)->getWriteOffset();
}

static SF_VIRTUAL_IO BYTE_BUFFER_VIRTUAL_SOUND_INPUT_CALLBACKS({
	&getByteBufferLength,
	&seekByteBufferReadOffset,
	&readFromByteBuffer,
	nullptr,
	&getByteBufferReadOffset
});

static SF_VIRTUAL_IO BYTE_BUFFER_VIRTUAL_SOUND_OUTPUT_CALLBACKS({
	&getByteBufferLength,
	&seekByteBufferWriteOffset,
	nullptr,
	&writeToByteBuffer,
	&getByteBufferWriteOffset
});

Sound::Sound(int format, uint32_t sampleRate, uint16_t numberOfChannels, const std::string & filePath)
	: GameFile(filePath)
	, m_data(std::make_unique<ByteBuffer>())
	, m_info({
		0,
		static_cast<int>(sampleRate),
		static_cast<int>(numberOfChannels),
		format,
		0,
		1
	})
	, m_fileHandle(createSoundFileHandle(sf_open_virtual(&BYTE_BUFFER_VIRTUAL_SOUND_OUTPUT_CALLBACKS, SFM_WRITE, &m_info, m_data.get()))) {
	if(m_fileHandle == nullptr) {
		spdlog::error("Failed to create sound file handle for '{}' format with {} Hz sample rate and {} audio channel{} with error: {}", getFormatName(format), sampleRate, numberOfChannels, numberOfChannels == 1 ? "" : "s", sf_strerror(nullptr));
	}
}

Sound::Sound(FileHandle fileHandle, SF_INFO && info, std::unique_ptr<ByteBuffer> data)
	: GameFile()
	, m_data(std::move(data))
	, m_info(std::move(info))
	, m_fileHandle(std::move(fileHandle)) { }

Sound::Sound(Sound && sound) noexcept
	: GameFile(std::move(sound))
	, m_data(std::move(sound.m_data))
	, m_info(std::move(sound.m_info))
	, m_fileHandle(std::move(sound.m_fileHandle)) { }

Sound & Sound::operator = (Sound && sound) noexcept {
	if(this != &sound) {
		GameFile::operator = (std::move(sound));

		m_data = std::move(sound.m_data);
		m_info = std::move(sound.m_info);
		m_fileHandle = std::move(sound.m_fileHandle);
	}

	return *this;
}

Sound::~Sound() { }

std::chrono::milliseconds Sound::getDuration() const {
	return std::chrono::milliseconds(static_cast<int64_t>((static_cast<double>(m_info.frames) / static_cast<double>(m_info.samplerate)) * 1000.0));
}

uint32_t Sound::getBitrate() const {
	if(!isValid()) {
		return 0;
	}

	return static_cast<uint32_t>(sf_current_byterate(m_fileHandle.get())) * 8;
}

int Sound::numberOfChannels() const {
	return m_info.channels;
}

int Sound::numberOfFrames() const {
	return m_info.frames;
}

int Sound::numberOfSections() const {
	return m_info.sections;
}

int Sound::getSampleRate() const {
	return m_info.samplerate;
}

bool Sound::isSeekable() const {
	return m_info.seekable != 0;
}

const SF_INFO & Sound::getInfo() const {
	return m_info;
}

const ByteBuffer * Sound::getData() const {
	return m_data.get();
}

std::string Sound::getFormatName() const {
	return getFormatName(m_info.format);
}

std::string Sound::getTypeName() const {
	return getTypeName(m_info.format);
}

std::string Sound::getSubTypeName() const {
	return getSubTypeName(m_info.format);
}

std::string Sound::getTitle() const {
	return getProperty(PropertyType::Title);
}

std::string Sound::getCopyright() const {
	return getProperty(PropertyType::Copyright);
}

std::string Sound::getSoftware() const {
	return getProperty(PropertyType::Software);
}

std::string Sound::getArtist() const {
	return getProperty(PropertyType::Artist);
}

std::string Sound::getComment() const {
	return getProperty(PropertyType::Comment);
}

std::string Sound::getDate() const {
	return getProperty(PropertyType::Date);
}

std::string Sound::getAlbum() const {
	return getProperty(PropertyType::Album);
}

std::string Sound::getLicense() const {
	return getProperty(PropertyType::License);
}

std::string Sound::getTrackNumber() const {
	return getProperty(PropertyType::TrackNumber);
}

std::string Sound::getGenre() const {
	return getProperty(PropertyType::Genre);
}

std::string Sound::getProperty(PropertyType propertyType) const {
	if(!isValid()) {
		return {};
	}

	const char * propertyValue = sf_get_string(m_fileHandle.get(), magic_enum::enum_integer(propertyType));

	if(propertyValue == nullptr) {
		return {};
	}

	return propertyValue;
}

bool Sound::setTitle(std::string value) const {
	return setProperty(PropertyType::Title, value);
}

bool Sound::setCopyright(std::string value) const {
	return setProperty(PropertyType::Copyright, value);
}

bool Sound::setSoftware(std::string value) const {
	return setProperty(PropertyType::Software, value);
}

bool Sound::setArtist(std::string value) const {
	return setProperty(PropertyType::Artist, value);
}

bool Sound::setComment(std::string value) const {
	return setProperty(PropertyType::Comment, value);
}

bool Sound::setDate(std::string value) const {
	return setProperty(PropertyType::Date, value);
}

bool Sound::setAlbum(std::string value) const {
	return setProperty(PropertyType::Album, value);
}

bool Sound::setLicense(std::string value) const {
	return setProperty(PropertyType::License, value);
}

bool Sound::setTrackNumber(std::string value) const {
	return setProperty(PropertyType::TrackNumber, value);
}

bool Sound::setGenre(std::string value) const {
	return setProperty(PropertyType::Genre, value);
}

bool Sound::setProperty(PropertyType propertyType, std::string propertyValue) const {
	if(!isValid()) {
		return false;
	}

	return isSuccess(sf_set_string(m_fileHandle.get(), magic_enum::enum_integer(propertyType), propertyValue.c_str()), fmt::format("Failed to set '{}' property value to '{}'.", Utilities::toCapitalCase(magic_enum::enum_name(propertyType)), propertyValue));
}

Sound::FileHandle Sound::readFrom(const ByteBuffer & byteBuffer, SF_INFO & info) {
	FileHandle fileHandle(createSoundFileHandle(sf_open_virtual(&BYTE_BUFFER_VIRTUAL_SOUND_INPUT_CALLBACKS, SFM_READ, &info, const_cast<ByteBuffer *>(&byteBuffer))));

	if(fileHandle == nullptr) {
		spdlog::error("Failed to read sound from byte buffer with error: {}", sf_strerror(nullptr));
		return nullptr;
	}

	return fileHandle;
}

Sound::FileHandle Sound::loadFrom(const std::string & filePath, SF_INFO & info) {
	FileHandle fileHandle(createSoundFileHandle(sf_open(filePath.c_str(), SFM_READ, &info)));

	if(fileHandle == nullptr) {
		spdlog::error("Failed to read sound from file '{}' with error: {}", filePath, sf_strerror(nullptr));
		return nullptr;
	}

	return fileHandle;
}

bool Sound::writeTo(ByteBuffer & byteBuffer) const {
	if(!isValid()) {
		return false;
	}

	SF_INFO soundFileInfo(m_info);
	FileHandle outputFileHandle(createSoundFileHandle(sf_open_virtual(&BYTE_BUFFER_VIRTUAL_SOUND_OUTPUT_CALLBACKS, SFM_WRITE, &soundFileInfo, &byteBuffer)));

	if(outputFileHandle == nullptr) {
		spdlog::error("Failed to create sound file handle for writing to byte buffer with error: {}", sf_strerror(nullptr));
		return false;
	}

	if(m_data != nullptr) {
		m_data->setReadOffset(0);

		if(sf_seek(m_fileHandle.get(), 0, SEEK_SET) == -1) {
			return false;
		}
	}

	static constexpr sf_count_t BUFFER_LENGTH = 4096;
	short buffer[BUFFER_LENGTH];
	int numberOfItemsRead = 0;

	while((numberOfItemsRead = sf_read_short(m_fileHandle.get(), buffer, BUFFER_LENGTH)) > 0) {
		sf_write_short(outputFileHandle.get(), buffer, numberOfItemsRead);
	}

	sf_write_sync(m_fileHandle.get());

	outputFileHandle.reset();

	if(m_data != nullptr) {
		m_data->setReadOffset(0);

		if(sf_seek(m_fileHandle.get(), 0, SEEK_SET) == -1) {
			return false;
		}
	}

	return true;
}

void Sound::addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const {
	if(!isValid()) {
		return;
	}

	std::string subTypeName(getSubTypeName(m_info.format));

	metadata.push_back({ "Type", getTypeName(m_info.format) });

	if(!subTypeName.empty()) {
		metadata.push_back({ "Sub-Type", subTypeName });
	}

	metadata.push_back({ "Duration", fmt::format("{:.2f} Seconds", getDuration().count() / 1000.0) });
	metadata.push_back({ "Number of Channels", std::to_string(m_info.channels) });
	metadata.push_back({ "Number of Frames", std::to_string(m_info.frames) });
	metadata.push_back({ "Number of Sections", std::to_string(m_info.sections) });
	metadata.push_back({ "Sample Rate", fmt::format("{} Hz ", m_info.samplerate) });
	metadata.push_back({ "Bitrate", std::to_string(getBitrate()) });
	metadata.push_back({ "Seekable", m_info.seekable != 0 ? "Y" : "N" });

	std::string propertyValue;

	for(const auto & propertyType : magic_enum::enum_values<PropertyType>()) {
		propertyValue = getProperty(propertyType);

		if(!propertyValue.empty()) {
			metadata.push_back({ Utilities::toCapitalCase(magic_enum::enum_name(propertyType)), propertyValue });
		}
	}
}

size_t Sound::getSizeInBytes() const {

	ByteBuffer data;

	if(!writeTo(data)) {
		return 0;
	}

	return data.getSize();
}

bool Sound::isValid(bool verifyParent) const {
	return m_fileHandle != nullptr &&
		   sf_format_check(&m_info) != 0;
}

bool Sound::isValid(const Sound * sound, bool verifyParent) {
	return sound != nullptr &&
		   sound->isValid(verifyParent);
}

std::string Sound::getFormatName(int format) {
	std::string typeName(getTypeName(format));
	std::string subTypeName(getSubTypeName(format));

	if(subTypeName.empty()) {
		return typeName;
	}

	return typeName + " / " + subTypeName;
}

std::string Sound::getTypeName(int format) {
	switch(format & SF_FORMAT_TYPEMASK) {
		case SF_FORMAT_WAV:
			return "Microsoft Waveform (WAV)";
		case SF_FORMAT_AIFF:
			return "Apple/SGI Audio Interchange File Format (AIFF)";
		case SF_FORMAT_AU:
			return "Sun/NeXT AU";
		case SF_FORMAT_RAW:
			return "Raw Pulse-Code Modulation (PCM)";
		case SF_FORMAT_PAF:
			return "Ensoniq PARIS";
		case SF_FORMAT_SVX:
			return "Amiga IFF / SVX8 / SV16";
		case SF_FORMAT_NIST:
			return "Institute of Standards and Technology's Speech Header Resources (Sphere NIST)";
		case SF_FORMAT_VOC:
			return "Creative Voice (VOC)";
		case SF_FORMAT_IRCAM:
			return "Berkeley/IRCAM/CARL";
		case SF_FORMAT_W64:
			return "Sonic Foundry's 64 bit RIFF/WAV";
		case SF_FORMAT_MAT4:
			return "Matlab (tm) V4.2 / GNU Octave 2.0";
		case SF_FORMAT_MAT5:
			return "Matlab (tm) V5.0 / GNU Octave 2.1";
		case SF_FORMAT_PVF:
			return "Portable Voice Format";
		case SF_FORMAT_XI:
			return "Fasttracker 2 Extended Instrument";
		case SF_FORMAT_HTK:
			return "HMM Tool Kit";
		case SF_FORMAT_SDS:
			return "MIDI Sample Dump Standard";
		case SF_FORMAT_AVR:
			return "Audio Visual Research";
		case SF_FORMAT_WAVEX:
			return "MS WAVE with WAVEFORMATEX";
		case SF_FORMAT_SD2:
			return "Sound Designer 2";
		case SF_FORMAT_FLAC:
			return "Free Losseless Audio Codec (FLAC)";
		case SF_FORMAT_CAF:
			return "Core Audio File";
		case SF_FORMAT_WVE:
			return "Psion WVE";
		case SF_FORMAT_OGG:
			return "Xiph OGG";
		case SF_FORMAT_MPC2K:
			return "Akai MPC 2000 Sampler";
		case SF_FORMAT_RF64:
			return "RF64 WAV";
		case SF_FORMAT_MPEG:
			return "MPEG-1/2";
	}

	return {};
}

std::string Sound::getSubTypeName(int format) {
	switch(format & SF_FORMAT_SUBMASK) {
		case SF_FORMAT_PCM_S8:
			return "Signed 8 Bit Data";
		case SF_FORMAT_PCM_16:
			return "Signed 16 Bit Data";
		case SF_FORMAT_PCM_24:
			return "Signed 24 Bit Data";
		case SF_FORMAT_PCM_32:
			return "Signed 32 Bit Data";
		case SF_FORMAT_PCM_U8:
			return "Unsigned 8 Bit Data";
		case SF_FORMAT_FLOAT:
			return "32 Bit Float Data";
		case SF_FORMAT_DOUBLE:
			return "64 Bit Float Data";
		case SF_FORMAT_ULAW:
			return "U-Law Encoded";
		case SF_FORMAT_ALAW:
			return "A-Law Encoded";
		case SF_FORMAT_IMA_ADPCM:
			return "IMA ADPCM";
		case SF_FORMAT_MS_ADPCM:
			return "Microsoft ADPCM";
		case SF_FORMAT_GSM610:
			return "GSM 6.10 Encoding";
		case SF_FORMAT_VOX_ADPCM:
			return "OKI / Dialogix ADPCM";
		case SF_FORMAT_NMS_ADPCM_16:
			return "16kbs NMS G721-Variant Encoding";
		case SF_FORMAT_NMS_ADPCM_24:
			return "24kbs NMS G721-Variant Encoding";
		case SF_FORMAT_NMS_ADPCM_32:
			return "32kbs NMS G721-Variant Encoding";
		case SF_FORMAT_G721_32:
			return "32kbs G721 ADPCM Encoding";
		case SF_FORMAT_G723_24:
			return "24kbs G723 ADPCM Encoding";
		case SF_FORMAT_G723_40:
			return "40kbs G723 ADPCM Encoding";
		case SF_FORMAT_DWVW_12:
			return "12 Bit Delta Width Variable Word Encoding";
		case SF_FORMAT_DWVW_16:
			return "16 Bit Delta Width Variable Word Encoding";
		case SF_FORMAT_DWVW_24:
			return "24 Bit Delta Width Variable Word Encoding";
		case SF_FORMAT_DWVW_N:
			return "N Bit Delta Width Variable Word Encoding";
		case SF_FORMAT_DPCM_8:
			return "8 Bit Differential PCM (XI only)";
		case SF_FORMAT_DPCM_16:
			return "16 bit Differential PCM (XI only)";
		case SF_FORMAT_VORBIS:
			return "Xiph Vorbis Encoding";
		case SF_FORMAT_OPUS:
			return "Xiph/Skype Opus Encoding";
		case SF_FORMAT_ALAC_16:
			return "Apple Lossless Audio Codec (16 bit)";
		case SF_FORMAT_ALAC_20:
			return "Apple Lossless Audio Codec (20 bit)";
		case SF_FORMAT_ALAC_24:
			return "Apple Lossless Audio Codec (24 bit)";
		case SF_FORMAT_ALAC_32:
			return "Apple Lossless Audio Codec (32 bit)";
		case SF_FORMAT_MPEG_LAYER_I:
			return "MPEG-1 Audio Layer I";
		case SF_FORMAT_MPEG_LAYER_II:
			return "MPEG-1 Audio Layer II";
		case SF_FORMAT_MPEG_LAYER_III:
			return "MPEG-2 Audio Layer III";
	}

	return {};
}

Sound::FileHandle Sound::createSoundFileHandle(SNDFILE * soundFile) {
	return FileHandle(soundFile, [](SNDFILE * soundFile) {
		isSuccess(sf_close(soundFile), "Failed to clean up sound file handle.");
	});
}

bool Sound::isSuccess(int errorCode, const std::string & errorMessage) {
	if(errorCode != SF_ERR_NO_ERROR) {
		if(!errorMessage.empty()) {
			spdlog::error("{} {}", errorMessage, sf_error_number(errorCode));
		}

		return false;
	}

	return true;
}

bool Sound::operator == (const Sound & sound) const {
	return m_fileHandle.get() == sound.m_fileHandle.get();
}

bool Sound::operator != (const Sound & sound) const {
	return !operator == (sound);
}
