#ifndef _PALETTE_PAL_H_
#define _PALETTE_PAL_H_

#include "../Palette.h"

#include <BitmaskOperators.h>

#include <array>
#include <string>

class PalettePAL final : public Palette {
public:
	enum class ColourFlag : uint8_t {
		None = 0,
		Reserved = 1,
		Explicit = 1 << 1,
		NoCollapse = 1 << 2
	};

	PalettePAL(const std::string & filePath = {});
	PalettePAL(std::vector<Colour> colours, std::vector<ColourFlag> colourFlags = {}, uint16_t version = PAL_VERSION, const std::string & filePath = {});
	PalettePAL(PalettePAL && palette) noexcept;
	PalettePAL(const PalettePAL & palette);
	PalettePAL & operator = (PalettePAL && palette) noexcept;
	PalettePAL & operator = (const PalettePAL & palette);
	virtual ~PalettePAL();

	uint16_t getVersion() const;
	const std::vector<ColourFlag> & getColourFlags() const;
	size_t getDocumentSizeInBytes() const;
	size_t getPaletteChunkSizeInBytes() const;
	virtual std::optional<uint16_t> numberOfColours(uint8_t colourTableIndex = 0) const override;
	virtual std::optional<uint16_t> getTransparentColourIndex(uint8_t colourTableIndex = 0) const override;
	virtual const Colour & lookupColour(uint8_t colourIndex, uint8_t colourTableIndex, bool * error = nullptr) const override;
	virtual bool updateColour(uint8_t colourIndex, const Colour & colour, uint8_t colourTableIndex = 0) override;
	static std::unique_ptr<PalettePAL> readFrom(const ByteBuffer & byteBuffer);
	static std::unique_ptr<PalettePAL> loadFrom(const std::string & filePath);
	virtual bool writeTo(ByteBuffer & byteBuffer) const override;
	virtual void addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const override;
	virtual Endianness getEndianness() const override;
	virtual size_t getSizeInBytes() const override;
	virtual bool isValid(bool verifyParent = false) const override;

	static constexpr Endianness ENDIANNESS = Endianness::LittleEndian;
	static inline const std::string RIFF_SIGNATURE = "RIFF";
	static inline const std::string PAL_FORM_TYPE = "PAL ";
	static inline const std::string RGB_CHUNK_TYPE = "data";
	static constexpr uint8_t BYTES_PER_COLOUR = 4;
	static constexpr uint16_t PAL_VERSION = 768;
	static constexpr uint8_t RIFF_SIGNATURE_SIZE_BYTES = 4;
	static constexpr uint8_t FORM_TYPE_SIZE_BYTES = 4;
	static constexpr uint8_t CHUNK_TYPE_SIZE_BYTES = 4;

private:
	uint16_t m_version;
	std::vector<Colour> m_colours;
	std::vector<ColourFlag> m_colourFlags;
};

template<>
struct BitmaskOperators<PalettePAL::ColourFlag> {
	static const bool enabled = true;
};

#endif // _PALETTE_PAL_H_
