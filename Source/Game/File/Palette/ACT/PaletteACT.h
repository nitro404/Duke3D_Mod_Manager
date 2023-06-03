#ifndef _PALETTE_ACT_H_
#define _PALETTE_ACT_H_

#include "../Palette.h"

#include <array>

class PaletteACT final : public Palette {
public:
	PaletteACT(const std::string & filePath = {});
	PaletteACT(std::array<Colour, NUMBER_OF_COLOURS> colours, const std::string & filePath = {});
	PaletteACT(PaletteACT && palette) noexcept;
	PaletteACT(const PaletteACT & palette);
	PaletteACT & operator = (PaletteACT && palette) noexcept;
	PaletteACT & operator = (const PaletteACT & palette);
	virtual ~PaletteACT();

	virtual Colour getColour(uint8_t x, uint8_t y, uint8_t colourTableIndex, bool * error = nullptr) const override;
	virtual bool updateColour(uint8_t x, uint8_t y, const Colour & colour, uint8_t colourTableIndex = 0) override;
	static std::unique_ptr<PaletteACT> readFrom(const ByteBuffer & byteBuffer);
	static std::unique_ptr<PaletteACT> loadFrom(const std::string & filePath);
	virtual bool writeTo(ByteBuffer & byteBuffer) const override;
	virtual Endianness getEndianness() const override;
	virtual size_t getSizeInBytes() const override;
	virtual bool isValid(bool verifyParent = false) const override;

	static constexpr uint8_t BYTES_PER_COLOUR = 3;
	static constexpr uint64_t SIZE_BYTES = NUMBER_OF_COLOURS * BYTES_PER_COLOUR;

private:
	std::array<Colour, NUMBER_OF_COLOURS> m_colours;
};

#endif // _PALETTE_ACT_H_
