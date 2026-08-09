#ifndef _PALETTE_KPL_H_
#define _PALETTE_KPL_H_

#include "../Palette.h"

#include <Archive/Zip/ZipArchive.h>

class PaletteKPL final : public Palette {
public:
	enum BitDepth {
		U8,
		U16,
		F16,
		F32
	};

	PaletteKPL(std::string_view comment = {}, uint8_t numberOfColumns = 1u, bool readOnly = DEFAULT_READ_ONLY, std::unique_ptr<ZipArchive> ZipArchive = nullptr, const std::string & filePath = {});
	PaletteKPL(std::unique_ptr<ColourTable> colours, std::string_view comment = {}, uint8_t numberOfColumns = 1u, bool readOnly = DEFAULT_READ_ONLY, std::unique_ptr<ZipArchive> ZipArchive = nullptr, const std::string & filePath = {});
	PaletteKPL(PaletteKPL && palette) noexcept;
	PaletteKPL(const PaletteKPL & palette);
	PaletteKPL & operator = (PaletteKPL && palette) noexcept;
	PaletteKPL & operator = (const PaletteKPL & palette);
	~PaletteKPL() override;

	const std::string & getName() const;
	void setName(std::string_view name);
	const std::string & getComment() const;
	void setComment(std::string_view comment);
	uint8_t getNumberOfColumns() const;
	void setNumberOfColumns(uint8_t numberOfColumns);
	bool isReadOnly() const;
	void setReadOnly(bool readOnly);

	static std::unique_ptr<PaletteKPL> readFrom(const ByteBuffer & byteBuffer);
	static std::unique_ptr<PaletteKPL> loadFrom(const std::string & filePath);

	// Palette Virtuals
	std::shared_ptr<ColourTable> getColourTable(uint8_t colourTableIndex = 0) const override;
	bool writeTo(ByteBuffer & byteBuffer) const override;
	Endianness getEndianness() const override;
	size_t getSizeInBytes() const override;

	bool operator == (const PaletteKPL & palette) const;
	bool operator != (const PaletteKPL & palette) const;

	static const std::string MIME_TYPE;
	static const std::string COLOR_SET_VERSION;
	static constexpr bool DEFAULT_READ_ONLY = false;
	static constexpr BitDepth DEFAULT_BIT_DEPTH = BitDepth::U8;

private:
	void ensureRequiredFiles() const;
	bool updateColorSet() const;
	void setModified(bool modified) const override;
	void onColourTableModified(ColourTable & colourTable);
	void connectSignals();
	void updateParent();

	std::string m_comment;
	uint8_t m_numberOfColumns;
	bool m_readOnly;
	mutable std::unique_ptr<ZipArchive> m_archive;
	std::shared_ptr<ColourTable> m_colourTable;
	boost::signals2::connection m_colourTableModifiedConnection;
};

#endif // _PALETTE_KPL_H_
