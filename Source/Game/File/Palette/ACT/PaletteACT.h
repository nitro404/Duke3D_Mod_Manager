#ifndef _PALETTE_ACT_H_
#define _PALETTE_ACT_H_

#include "../Palette.h"

class PaletteACT final : public Palette {
public:
	PaletteACT(const std::string & filePath = {});
	PaletteACT(std::vector<Colour> colours, std::optional<uint16_t> transparentColourIndex = {}, const std::string & filePath = {});
	PaletteACT(PaletteACT && palette) noexcept;
	PaletteACT(const PaletteACT & palette);
	PaletteACT & operator = (PaletteACT && palette) noexcept;
	PaletteACT & operator = (const PaletteACT & palette);
	virtual ~PaletteACT();

	virtual std::optional<uint16_t> numberOfColours(uint8_t colourTableIndex = 0) const override;
	virtual std::optional<uint16_t> getTransparentColourIndex(uint8_t colourTableIndex = 0) const override;
	virtual const Colour & lookupColour(uint8_t colourIndex, uint8_t colourTableIndex, bool * error = nullptr) const override;
	virtual bool updateColour(uint8_t colourIndex, const Colour & colour, uint8_t colourTableIndex = 0) override;
	static std::unique_ptr<PaletteACT> readFrom(const ByteBuffer & byteBuffer);
	static std::unique_ptr<PaletteACT> loadFrom(const std::string & filePath);
	virtual bool writeTo(ByteBuffer & byteBuffer) const override;
	virtual Endianness getEndianness() const override;
	virtual size_t getSizeInBytes() const override;
	virtual bool isValid(bool verifyParent = false) const override;

	static constexpr Endianness ENDIANNESS = Endianness::BigEndian;
	static constexpr uint8_t BYTES_PER_COLOUR = 3;

private:
	std::vector<Colour> m_colours;
	std::optional<uint16_t> m_transparentColourIndex;
};

#endif // _PALETTE_ACT_H_
