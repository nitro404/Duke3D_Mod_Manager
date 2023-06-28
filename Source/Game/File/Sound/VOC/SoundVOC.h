#ifndef _SOUND_VOC_H_
#define _SOUND_VOC_H_

#include "../Sound.h"

class SoundVOC final : public Sound {
public:
	enum class SubType {
		PCMUnsigned8Bit,
		PCMSigned16Bit,
		ULaw,
		ALaw
	};

	SoundVOC(SubType subType, uint32_t sampleRate, uint16_t numberOfChannels, const std::string & filePath = {});
	SoundVOC(FileHandle fileHandle, SF_INFO && info, std::unique_ptr<ByteBuffer> data = nullptr);
	SoundVOC(SoundVOC && sound) noexcept;
	SoundVOC & operator = (SoundVOC && sound) noexcept;
	virtual ~SoundVOC();

	static std::unique_ptr<SoundVOC> readFrom(const ByteBuffer & byteBuffer);
	static std::unique_ptr<SoundVOC> loadFrom(const std::string & filePath);

	virtual Endianness getEndianness() const override;
	virtual bool isValid(bool verifyParent = true) const override;

	static std::string getSubTypeName(SubType subType);
	static int getFormatForSubType(SubType subType);

	static constexpr Endianness ENDIANNESS = Endianness::LittleEndian;
};

#endif // _SOUND_VOC_H_
