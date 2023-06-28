#include "SoundWAV.h"

#include <spdlog/spdlog.h>

SoundWAV::SoundWAV(SubType subType, uint32_t sampleRate, uint16_t numberOfChannels, const std::string & filePath)
	: Sound(getFormatForSubType(subType), sampleRate, numberOfChannels) { }

SoundWAV::SoundWAV(FileHandle fileHandle, SF_INFO && info, std::unique_ptr<ByteBuffer> data)
	: Sound(std::move(fileHandle), std::move(info), std::move(data)) { }

SoundWAV::SoundWAV(SoundWAV && sound) noexcept
	: Sound(std::move(sound)) { }

SoundWAV & SoundWAV::operator = (SoundWAV && sound) noexcept {
	if(this != &sound) {
		Sound::operator = (std::move(sound));
	}

	return *this;
}

SoundWAV::~SoundWAV() { }

std::unique_ptr<SoundWAV> SoundWAV::readFrom(const ByteBuffer & byteBuffer) {
	std::unique_ptr<ByteBuffer> data(std::make_unique<ByteBuffer>(byteBuffer));
	SF_INFO soundFileInfo;
	FileHandle fileHandle(Sound::readFrom(*data, soundFileInfo));

	if(fileHandle == nullptr) {
		return nullptr;
	}

	if((soundFileInfo.format & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
		spdlog::error("Invalid sound type: '{}', expected: '{}'.", getTypeName(soundFileInfo.format), getTypeName(SF_FORMAT_WAV));
		return nullptr;
	}

	return std::make_unique<SoundWAV>(std::move(fileHandle), std::move(soundFileInfo), std::move(data));
}

std::unique_ptr<SoundWAV> SoundWAV::loadFrom(const std::string & filePath) {
	SF_INFO soundFileInfo;
	FileHandle fileHandle(Sound::loadFrom(filePath, soundFileInfo));

	if(fileHandle == nullptr || (soundFileInfo.format & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
		return nullptr;
	}

	return std::make_unique<SoundWAV>(std::move(fileHandle), std::move(soundFileInfo));
}

Endianness SoundWAV::getEndianness() const {
	return ENDIANNESS;
}

bool SoundWAV::isValid(bool verifyParent) const {
	return Sound::isValid(verifyParent) &&
		   (m_info.format & SF_FORMAT_TYPEMASK) == SF_FORMAT_WAV;
}

std::string SoundWAV::getSubTypeName(SubType subType) {
	return Sound::getSubTypeName(getFormatForSubType(subType) & SF_FORMAT_SUBMASK);
}

int SoundWAV::getFormatForSubType(SubType subType) {
	int format = 0;

	switch(subType) {
		case SubType::PCMUnsigned8Bit: {
			format = SF_FORMAT_PCM_U8;
			break;
		}

		case SubType::PCMSigned16Bit: {
			format = SF_FORMAT_PCM_16;
			break;
		}

		case SubType::PCMSigned24Bit: {
			format = SF_FORMAT_PCM_24;
			break;
		}

		case SubType::PCMSigned32Bit: {
			format = SF_FORMAT_PCM_32;
			break;
		}

		case SubType::Float: {
			format = SF_FORMAT_FLOAT;
			break;
		}

		case SubType::Double: {
			format = SF_FORMAT_DOUBLE;
			break;
		}

		case SubType::ULaw: {
			format = SF_FORMAT_ULAW;
			break;
		}

		case SubType::ALaw: {
			format = SF_FORMAT_ALAW;
			break;
		}

		case SubType::IMAADPCM: {
			format = SF_FORMAT_IMA_ADPCM;
			break;
		}

		case SubType::MSADPCM: {
			format = SF_FORMAT_MS_ADPCM;
			break;
		}

		case SubType::GSM610: {
			format = SF_FORMAT_GSM610;
			break;
		}

		case SubType::G721ADPCM32kbps: {
			format = SF_FORMAT_NMS_ADPCM_32;
			break;
		}
	}

	return SF_FORMAT_WAV | format;
}
