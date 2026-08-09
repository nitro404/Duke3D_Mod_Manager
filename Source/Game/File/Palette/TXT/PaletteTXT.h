#ifndef _PALETTE_TXT_H_
#define _PALETTE_TXT_H_

#include "../Palette.h"

class PaletteTXT final : public Palette {
public:
	PaletteTXT(const std::string & filePath = {});
	PaletteTXT(std::unique_ptr<ColourTable> colours, const std::string & filePath = {});
	PaletteTXT(PaletteTXT && palette) noexcept;
	PaletteTXT(const PaletteTXT & palette);
	PaletteTXT & operator = (PaletteTXT && palette) noexcept;
	PaletteTXT & operator = (const PaletteTXT & palette);
	~PaletteTXT() override;

	virtual std::shared_ptr<ColourTable> getColourTable(uint8_t colourTableIndex = 0) const override;
	static std::unique_ptr<PaletteTXT> readFrom(const ByteBuffer & byteBuffer);
	static std::unique_ptr<PaletteTXT> loadFrom(const std::string & filePath);
	virtual bool writeTo(ByteBuffer & byteBuffer) const override;
	virtual Endianness getEndianness() const override;
	virtual size_t getSizeInBytes() const override;

	bool operator == (const PaletteTXT & palette) const;
	bool operator != (const PaletteTXT & palette) const;

private:
	void updateParent();

	std::shared_ptr<ColourTable> m_colourTable;
};

#endif // _PALETTE_TXT_H_
