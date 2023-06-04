#include "Palette.h"

#include <fmt/core.h>

Palette::Palette(uint8_t bytesPerColour, const std::string & filePath)
	: GameFile(filePath)
	, m_bytesPerColour(bytesPerColour) { }

Palette::Palette(Palette && palette) noexcept
	: GameFile(palette)
	, m_bytesPerColour(palette.m_bytesPerColour) { }

Palette::Palette(const Palette & palette)
	: GameFile(palette)
	, m_bytesPerColour(palette.m_bytesPerColour) { }

Palette & Palette::operator = (Palette && palette) noexcept {
	if(this != &palette) {
		GameFile::operator = (palette);

		m_bytesPerColour = palette.m_bytesPerColour;
	}

	return *this;
}

Palette & Palette::operator = (const Palette & palette) {
	GameFile::operator = (palette);

	m_bytesPerColour = palette.m_bytesPerColour;

	return *this;
}

Palette::~Palette() { }

uint8_t Palette::numberOfBytesPerColour() const {
	return m_bytesPerColour;
}

std::optional<Colour> Palette::getColour(uint8_t x, uint8_t y, uint8_t colourTableIndex) const {
	bool error = false;

	Colour colour(getColour(x, y, colourTableIndex, &error));

	if(error) {
		return {};
	}

	return colour;
}

const Colour & Palette::lookupColour(uint8_t colourIndex, uint8_t colourTableIndex, bool * error) const {
	return getColour(colourIndex % Palette::PALETTE_WIDTH, static_cast<uint8_t>(floor(colourIndex / Palette::PALETTE_HEIGHT)), colourTableIndex, error);
}

std::optional<Colour> Palette::lookupColour(uint8_t colourIndex, uint8_t colourTableIndex) const {
	bool error = false;

	Colour colour(lookupColour(colourIndex, colourTableIndex, &error));

	if(error) {
		return {};
	}

	return colour;
}

bool Palette::updateColour(uint8_t colourIndex, const Colour & colour, uint8_t colourTableIndex) {
	return updateColour(colourIndex % Palette::PALETTE_WIDTH, static_cast<uint8_t>(floor(colourIndex / Palette::PALETTE_HEIGHT)), colour, colourTableIndex);
}

std::vector<Colour> Palette::getColourTableData(uint8_t colourTableIndex) const {
	std::vector<Colour> colours;
	colours.reserve(NUMBER_OF_COLOURS);

	bool error = false;

	for(uint16_t i = 0; i < NUMBER_OF_COLOURS; i++) {
		colours.emplace_back(lookupColour(i, colourTableIndex, &error));

		if(error) {
			return {};
		}
	}

	return colours;
}

std::vector<Colour> Palette::getAllColourTableData(uint8_t colourTableIndex) const {
	uint8_t colourTableCount = numberOfColourTables();
	std::vector<Colour> colours;
	colours.reserve(NUMBER_OF_COLOURS * colourTableCount);

	bool error = false;

	for(uint8_t i = 0; i < colourTableCount; i++) {
		for(uint16_t j = 0; j < NUMBER_OF_COLOURS; j++) {
			colours.emplace_back(lookupColour(j, i, &error));

			if(error) {
				return {};
			}
		}
	}

	return colours;
}

bool Palette::updateColourTableData(uint8_t colourTableIndex, const std::vector<Colour> & colourData, uint64_t colourDataOffset) {
	if(colourTableIndex >= numberOfColourTables() || colourData.size() - colourDataOffset < NUMBER_OF_COLOURS) {
		return false;
	}

	for(int j = 0; j < PALETTE_HEIGHT; j++) {
		for(int i = 0; i < PALETTE_WIDTH; i++) {
			if(!updateColour(i, j, colourData[colourDataOffset + i + (j * Palette::PALETTE_HEIGHT)], colourTableIndex)) {
				return false;
			}
		}
	}

	return true;
}

bool Palette::updateAllColourTablesData(const std::vector<Colour> & colourData) {
	uint8_t colourTableCount = numberOfColourTables();

	if(colourData.size() != colourTableCount * NUMBER_OF_COLOURS) {
		return false;
	}

	for(uint8_t i = 0; i < colourTableCount; i++) {
		if(!updateColourTableData(i, colourData, i * NUMBER_OF_COLOURS)) {
			return false;
		}
	}

	return true;
}

bool Palette::fillColourTableWithColour(const Colour & colour, uint8_t colourTableIndex) {
	for(int j = 0; j < PALETTE_HEIGHT; j++) {
		for(int i = 0; i < PALETTE_WIDTH; i++) {
			if(!updateColour(i, j, colour, colourTableIndex)) {
				return false;
			}
		}
	}

	return true;
}

bool Palette::fillAllColourTablesWithColour(const Colour & colour) {
	uint8_t colourTableCount = numberOfColourTables();

	for(uint8_t i = 0; i < colourTableCount; i++) {
		if(!fillColourTableWithColour(colour, i)) {
			return false;
		}
	}

	return true;
}

uint8_t Palette::numberOfColourTables() const {
	return 1;
}

std::string Palette::getColourTableDescription(uint8_t colourTableIndex) const {
	if(colourTableIndex != 0) {
		return {};
	}

	return "Default";
}

void Palette::addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const {
	uint8_t colourTableCount = numberOfColourTables();

	metadata.push_back({ "Bytes Per Colour", std::to_string(numberOfBytesPerColour()) });
	metadata.push_back({ "Number of Colour Tables", std::to_string(colourTableCount) });

	if(colourTableCount == 1) {
		std::optional<uint16_t> optionalNumberOfColours(numberOfColours());
		std::optional<uint16_t> optionalTransparentColourIndex(getTransparentColourIndex());

		metadata.push_back({ "Number of Colours", optionalNumberOfColours.has_value() ? std::to_string(optionalNumberOfColours.value()) : "N/A" });
		metadata.push_back({ "Transparent Colour Index", optionalTransparentColourIndex.has_value() ? std::to_string(optionalTransparentColourIndex.value()) : "N/A" });
	}
	else {
		for(uint8_t i = 0; i < colourTableCount; i++) {
			std::string colourTableDescription(getColourTableDescription(i));
			std::optional<uint16_t> optionalNumberOfColours(numberOfColours(i));
			std::optional<uint16_t> optionalTransparentColourIndex(getTransparentColourIndex(i));

			metadata.push_back({ fmt::format("Colour Table #{}", i + 1), fmt::format("'{}' Number of Colours: {} Transparent Colour Index: {}", colourTableDescription.empty() ? "Untitled" : colourTableDescription, optionalNumberOfColours.has_value() ? std::to_string(optionalNumberOfColours.value()) : "N/A", optionalTransparentColourIndex.has_value() ? std::to_string(optionalTransparentColourIndex.value()) : "N/A") });
		}
	}
}
