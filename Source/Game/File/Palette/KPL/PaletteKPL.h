#ifndef _PALETTE_KPL_H_
#define _PALETTE_KPL_H_

#include "../Palette.h"

class PaletteKPL final : public Palette {
public:
	PaletteKPL(const std::string & filePath = {});
	PaletteKPL(std::unique_ptr<ColourTable> colours, const std::string & filePath = {});
	PaletteKPL(PaletteKPL && palette) noexcept;
	PaletteKPL(const PaletteKPL & palette);
	PaletteKPL & operator = (PaletteKPL && palette) noexcept;
	PaletteKPL & operator = (const PaletteKPL & palette);
	~PaletteKPL() override;

	virtual std::shared_ptr<ColourTable> getColourTable(uint8_t colourTableIndex = 0) const override;
	static std::unique_ptr<PaletteKPL> readFrom(const ByteBuffer & byteBuffer);
	static std::unique_ptr<PaletteKPL> loadFrom(const std::string & filePath);
	virtual bool writeTo(ByteBuffer & byteBuffer) const override;
	virtual Endianness getEndianness() const override;
	virtual size_t getSizeInBytes() const override;

	bool operator == (const PaletteKPL & palette) const;
	bool operator != (const PaletteKPL & palette) const;

	static constexpr Endianness ENDIANNESS = Endianness::BigEndian;
	static constexpr uint8_t BYTES_PER_COLOUR = 3;

private:
	void updateParent();

	std::shared_ptr<ColourTable> m_colourTable;
};

#endif // _PALETTE_KPL_H_
