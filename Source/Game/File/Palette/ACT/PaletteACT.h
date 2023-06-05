#ifndef _PALETTE_ACT_H_
#define _PALETTE_ACT_H_

#include "../Palette.h"

class PaletteACT final : public Palette {
public:
	PaletteACT(const std::string & filePath = {});
	PaletteACT(std::unique_ptr<ColourTable> colours, const std::string & filePath = {});
	PaletteACT(PaletteACT && palette) noexcept;
	PaletteACT(const PaletteACT & palette);
	PaletteACT & operator = (PaletteACT && palette) noexcept;
	PaletteACT & operator = (const PaletteACT & palette);
	virtual ~PaletteACT();

	virtual std::shared_ptr<ColourTable> getColourTable(uint8_t colourTableIndex = 0) const override;
	static std::unique_ptr<PaletteACT> readFrom(const ByteBuffer & byteBuffer);
	static std::unique_ptr<PaletteACT> loadFrom(const std::string & filePath);
	virtual bool writeTo(ByteBuffer & byteBuffer) const override;
	virtual Endianness getEndianness() const override;
	virtual size_t getSizeInBytes() const override;

	bool operator == (const PaletteACT & palette) const;
	bool operator != (const PaletteACT & palette) const;

	static constexpr Endianness ENDIANNESS = Endianness::BigEndian;
	static constexpr uint8_t BYTES_PER_COLOUR = 3;

private:
	void updateParent();

	std::shared_ptr<ColourTable> m_colourTable;
};

#endif // _PALETTE_ACT_H_
