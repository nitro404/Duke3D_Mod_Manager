#ifndef _SOUND_WAV_H_
#define _SOUND_WAV_H_

#include "../Sound.h"

class SoundWAV final : public Sound {
public:
	enum class SubType {
		PCMUnsigned8Bit,
		PCMSigned16Bit,
		PCMSigned24Bit,
		PCMSigned32Bit,
		Float,
		Double,
		ULaw,
		ALaw,
		IMAADPCM,
		MSADPCM,
		GSM610,
		G721ADPCM32kbps
	};

	SoundWAV(SubType subType, uint32_t sampleRate, uint16_t numberOfChannels, const std::string & filePath = {});
	SoundWAV(FileHandle fileHandle, SF_INFO && info, std::unique_ptr<ByteBuffer> data = nullptr);
	SoundWAV(SoundWAV && sound) noexcept;
	SoundWAV & operator = (SoundWAV && sound) noexcept;
	virtual ~SoundWAV();

	static std::unique_ptr<SoundWAV> readFrom(const ByteBuffer & byteBuffer);
	static std::unique_ptr<SoundWAV> loadFrom(const std::string & filePath);

	virtual Endianness getEndianness() const override;
	virtual bool isValid(bool verifyParent = true) const override;

	static std::string getSubTypeName(SubType subType);
	static int getFormatForSubType(SubType subType);

	static constexpr Endianness ENDIANNESS = Endianness::LittleEndian;
};

#endif // _SOUND_WAV_H_
