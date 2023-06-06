#ifndef _PALETTE_JASC_H_
#define _PALETTE_JASC_H_

#include "../Palette.h"

#include <string>

class PaletteJASC final : public Palette {
public:
	PaletteJASC(const std::string & filePath = {});
	PaletteJASC(std::unique_ptr<ColourTable> colours, const std::string & version = VERSION, const std::string & filePath = {});
	PaletteJASC(PaletteJASC && palette) noexcept;
	PaletteJASC(const PaletteJASC & palette);
	PaletteJASC & operator = (PaletteJASC && palette) noexcept;
	PaletteJASC & operator = (const PaletteJASC & palette);
	virtual ~PaletteJASC();

	const std::string & getVersion() const;
	virtual std::shared_ptr<ColourTable> getColourTable(uint8_t colourTableIndex = 0) const override;
	static std::unique_ptr<PaletteJASC> readFrom(const ByteBuffer & byteBuffer);
	static std::unique_ptr<PaletteJASC> loadFrom(const std::string & filePath);
	virtual bool writeTo(ByteBuffer & byteBuffer) const override;
	virtual void addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const override;
	virtual Endianness getEndianness() const override;
	virtual size_t getSizeInBytes() const override;

	bool operator == (const PaletteJASC & palette) const;
	bool operator != (const PaletteJASC & palette) const;

	static inline const std::string MAGIC = "JASC-PAL";
	static inline const std::string VERSION = "0100";
	static constexpr size_t MAGIC_SIZE_BYTES = 8;

private:
	void updateParent();

	std::shared_ptr<ColourTable> m_colourTable;
	std::string m_version;
};

#endif // _PALETTE_JASC_H_
