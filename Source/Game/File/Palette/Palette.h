#ifndef _PALETTE_H_
#define _PALETTE_H_

#include "../GameFile.h"

#include <Colour.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

class Palette : public GameFile {
public:
	Palette(Palette && palette) noexcept;
	Palette(const Palette & palette);
	Palette & operator = (Palette && palette) noexcept;
	Palette & operator = (const Palette & palette);
	virtual ~Palette();

	uint8_t numberOfBytesPerColour() const;
	virtual std::optional<uint16_t> numberOfColours(uint8_t colourTableIndex = 0) const = 0;
	virtual std::optional<uint16_t> getTransparentColourIndex(uint8_t colourTableIndex = 0) const = 0;
	virtual const Colour & getColour(uint8_t x, uint8_t y, uint8_t colourTableIndex = 0, bool * error = nullptr) const = 0;
	std::optional<Colour> getColour(uint8_t x, uint8_t y, uint8_t colourTableIndex = 0) const;
	const Colour & lookupColour(uint8_t colourIndex, uint8_t colourTableIndex = 0, bool * error = nullptr) const;
	std::optional<Colour> lookupColour(uint8_t colourIndex, uint8_t colourTableIndex = 0) const;
	virtual bool updateColour(uint8_t x, uint8_t y, const Colour & colour, uint8_t colourTableIndex = 0) = 0;
	bool updateColour(uint8_t colourIndex, const Colour & colour, uint8_t colourTableIndex = 0);
	std::vector<Colour> getColourTableData(uint8_t colourTableIndex = 0) const;
	std::vector<Colour> getAllColourTableData(uint8_t colourTableIndex = 0) const;
	bool updateColourTableData(uint8_t colourTableIndex, const std::vector<Colour> & colourData, uint64_t colourDataOffset = 0);
	bool updateAllColourTablesData(const std::vector<Colour> & colourData);
	bool fillColourTableWithColour(const Colour & colour, uint8_t colourTableIndex = 0);
	bool fillAllColourTablesWithColour(const Colour & colour);
	virtual uint8_t numberOfColourTables() const;
	virtual std::string getColourTableDescription(uint8_t colourTableIndex) const;
	virtual void addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const override;
	virtual bool isValid(bool verifyParent = false) const override;

	static constexpr uint8_t PALETTE_WIDTH = 16;
	static constexpr uint8_t PALETTE_HEIGHT = 16;
	static constexpr uint16_t NUMBER_OF_COLOURS = PALETTE_WIDTH * PALETTE_HEIGHT;

protected:
	Palette(uint8_t bytesPerColour, const std::string & filePath = {});

	uint8_t m_bytesPerColour;
};

#endif // _PALETTE_H_
