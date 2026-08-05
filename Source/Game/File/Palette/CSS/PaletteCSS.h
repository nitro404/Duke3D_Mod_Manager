#ifndef _PALETTE_CSS_H_
#define _PALETTE_CSS_H_

#include "../Palette.h"

class PaletteCSS final : public Palette {
public:
	PaletteCSS(const std::string & filePath = {});
	PaletteCSS(std::unique_ptr<ColourTable> colours, const std::string & filePath = {});
	PaletteCSS(PaletteCSS && palette) noexcept;
	PaletteCSS(const PaletteCSS & palette);
	PaletteCSS & operator = (PaletteCSS && palette) noexcept;
	PaletteCSS & operator = (const PaletteCSS & palette);
	~PaletteCSS() override;

	virtual std::shared_ptr<ColourTable> getColourTable(uint8_t colourTableIndex = 0) const override;
	static std::unique_ptr<PaletteCSS> readFrom(const ByteBuffer & byteBuffer);
	static std::unique_ptr<PaletteCSS> loadFrom(const std::string & filePath);
	virtual bool writeTo(ByteBuffer & byteBuffer) const override;
	virtual Endianness getEndianness() const override;
	virtual size_t getSizeInBytes() const override;

	bool operator == (const PaletteCSS & palette) const;
	bool operator != (const PaletteCSS & palette) const;

private:
	void updateParent();

	std::shared_ptr<ColourTable> m_colourTable;
};

#endif // _PALETTE_CSS_H_
