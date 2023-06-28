#include "SoundVOC.h"

#include <spdlog/spdlog.h>

SoundVOC::SoundVOC(SubType subType, uint32_t sampleRate, uint16_t numberOfChannels, const std::string & filePath)
	: Sound(getFormatForSubType(subType), sampleRate, numberOfChannels) { }

SoundVOC::SoundVOC(FileHandle fileHandle, SF_INFO && info, std::unique_ptr<ByteBuffer> data)
	: Sound(std::move(fileHandle), std::move(info), std::move(data)) { }

SoundVOC::SoundVOC(SoundVOC && sound) noexcept
	: Sound(std::move(sound)) { }

SoundVOC & SoundVOC::operator = (SoundVOC && sound) noexcept {
	if(this != &sound) {
		Sound::operator = (std::move(sound));
	}

	return *this;
}

SoundVOC::~SoundVOC() { }

std::unique_ptr<SoundVOC> SoundVOC::readFrom(const ByteBuffer & byteBuffer) {
	std::unique_ptr<ByteBuffer> data(std::make_unique<ByteBuffer>(byteBuffer));
	SF_INFO soundFileInfo;
	FileHandle fileHandle(Sound::readFrom(*data, soundFileInfo));

	if(fileHandle == nullptr) {
		return nullptr;
	}

	if((soundFileInfo.format & SF_FORMAT_TYPEMASK) != SF_FORMAT_VOC) {
		spdlog::error("Invalid sound type: '{}', expected: '{}'.", getTypeName(soundFileInfo.format), getTypeName(SF_FORMAT_VOC));
		return nullptr;
	}

	return std::make_unique<SoundVOC>(std::move(fileHandle), std::move(soundFileInfo), std::move(data));
}

std::unique_ptr<SoundVOC> SoundVOC::loadFrom(const std::string & filePath) {
	SF_INFO soundFileInfo;
	FileHandle fileHandle(Sound::loadFrom(filePath, soundFileInfo));

	if(fileHandle == nullptr || (soundFileInfo.format & SF_FORMAT_TYPEMASK) != SF_FORMAT_VOC) {
		return nullptr;
	}

	return std::make_unique<SoundVOC>(std::move(fileHandle), std::move(soundFileInfo));
}

Endianness SoundVOC::getEndianness() const {
	return ENDIANNESS;
}

bool SoundVOC::isValid(bool verifyParent) const {
	return Sound::isValid(verifyParent) &&
		   (m_info.format & SF_FORMAT_TYPEMASK) == SF_FORMAT_VOC;
}

std::string SoundVOC::getSubTypeName(SubType subType) {
	return Sound::getSubTypeName(getFormatForSubType(subType) & SF_FORMAT_SUBMASK);
}

int SoundVOC::getFormatForSubType(SubType subType) {
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

		case SubType::ULaw: {
			format = SF_FORMAT_ULAW;
			break;
		}

		case SubType::ALaw: {
			format = SF_FORMAT_ALAW;
			break;
		}
	}

	return SF_FORMAT_VOC | format;
}
