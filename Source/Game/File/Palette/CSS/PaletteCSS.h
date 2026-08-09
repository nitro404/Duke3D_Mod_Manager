#ifndef _PALETTE_CSS_H_
#define _PALETTE_CSS_H_

#include "../Palette.h"

class PaletteCSS final : public Palette {
public:
	enum class Format {
		RGB,
		Hexadecimal
	};

	PaletteCSS(Format format = DEFAULT_FORMAT, std::string_view comment = {}, const std::string & filePath = {});
	PaletteCSS(std::unique_ptr<ColourTable> colours, Format format = DEFAULT_FORMAT, std::string_view comment = {}, const std::string & filePath = {});
	PaletteCSS(PaletteCSS && palette) noexcept;
	PaletteCSS(const PaletteCSS & palette);
	PaletteCSS & operator = (PaletteCSS && palette) noexcept;
	PaletteCSS & operator = (const PaletteCSS & palette);
	~PaletteCSS() override;

	Format getFormat() const;
	void setFormat(Format format);
	bool hasComment() const;
	const std::string & getComment() const;
	void setComment(std::string_view comment);
	void clearComment();

	static std::unique_ptr<PaletteCSS> readFrom(const ByteBuffer & byteBuffer);
	static std::unique_ptr<PaletteCSS> loadFrom(const std::string & filePath);

	// Palette Virtuals
	std::shared_ptr<ColourTable> getColourTable(uint8_t colourTableIndex = 0) const override;
	bool writeTo(ByteBuffer & byteBuffer) const override;
	Endianness getEndianness() const override;
	size_t getSizeInBytes() const override;

	bool operator == (const PaletteCSS & palette) const;
	bool operator != (const PaletteCSS & palette) const;

	static constexpr Format DEFAULT_FORMAT = Format::RGB;

private:
	void updateParent();

	Format m_format;
	std::string m_comment;
	std::shared_ptr<ColourTable> m_colourTable;
};

#endif // _PALETTE_CSS_H_
