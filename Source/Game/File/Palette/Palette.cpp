#include "Palette.h"

#include <Utilities/StringUtilities.h>

#include <fmt/core.h>

static size_t getColourIndex(uint8_t x, uint8_t y) {
	if(x >= Palette::PALETTE_WIDTH || y >= Palette::PALETTE_HEIGHT) {
		return std::numeric_limits<size_t>::max();
	}

	return (static_cast<size_t>(x)) + (static_cast<size_t>(y) * Palette::PALETTE_HEIGHT);
}

Palette::Palette(const std::string & filePath)
	: GameFile(filePath) { }

Palette::Palette(Palette && palette) noexcept
	: GameFile(palette) { }

Palette::Palette(const Palette & palette)
	: GameFile(palette) { }

Palette & Palette::operator = (Palette && palette) noexcept {
	if(this != &palette) {
		GameFile::operator = (palette);
	}

	return *this;
}

Palette & Palette::operator = (const Palette & palette) {
	GameFile::operator = (palette);

	return *this;
}

Palette::~Palette() { }

bool Palette::hasTransparentColourIndex(uint8_t colourTableIndex) const {
	std::shared_ptr<ColourTable> colourTable(getColourTable(colourTableIndex));

	if(colourTable == nullptr) {
		return {};
	}

	return colourTable->hasTransparentColourIndex();
}

std::optional<uint8_t> Palette::getTransparentColourIndex(uint8_t colourTableIndex) const {
	std::shared_ptr<ColourTable> colourTable(getColourTable(colourTableIndex));

	if(colourTable == nullptr) {
		return {};
	}

	return colourTable->getTransparentColourIndex();
}

bool Palette::setTransparentColourIndex(uint8_t transparentColourIndex, uint8_t colourTableIndex) const {
	std::shared_ptr<ColourTable> colourTable(getColourTable(colourTableIndex));

	if(colourTable == nullptr) {
		return false;
	}

	colourTable->setTransparentColourIndex(transparentColourIndex);

	return true;
}

bool Palette::clearTransparentColourIndex(uint8_t colourTableIndex) const {
	std::shared_ptr<ColourTable> colourTable(getColourTable(colourTableIndex));

	if(colourTable == nullptr) {
		return false;
	}

	colourTable->clearTransparentColourIndex();

	return true;
}

std::optional<uint16_t> Palette::numberOfColours(uint8_t colourTableIndex) const {
	std::shared_ptr<ColourTable> colourTable(getColourTable(colourTableIndex));

	if(colourTable == nullptr) {
		return {};
	}

	return colourTable->numberOfColours();
}

bool Palette::setNumberOfColours(uint16_t colourCount, uint8_t colourTableIndex) const {
	std::shared_ptr<ColourTable> colourTable(getColourTable(colourTableIndex));

	if(colourTable == nullptr) {
		return {};
	}

	return colourTable->setNumberOfColours(colourCount);
}

const Colour & Palette::getColour(uint8_t colourIndex, uint8_t colourTableIndex, bool * error) const {
	std::shared_ptr<ColourTable> colourTable(getColourTable(colourTableIndex));

	if(colourTable == nullptr) {
		if(error != nullptr) {
			*error = true;
		}

		return Colour::INVISIBLE;
	}

	return (*colourTable)[colourIndex];
}

std::optional<Colour> Palette::getColour(uint8_t colourIndex, uint8_t colourTableIndex) const {
	bool error = false;

	Colour colour(getColour(colourIndex, colourTableIndex, &error));

	if(error) {
		return {};
	}

	return colour;
}

const Colour & Palette::getColour(uint8_t x, uint8_t y, uint8_t colourTableIndex, bool * error) const {
	uint8_t colourIndex = getColourIndex(x, y);

	if(colourIndex >= NUMBER_OF_COLOURS) {
		if(error != nullptr) {
			*error = true;
		}

		return Colour::INVISIBLE;
	}

	return getColour(colourIndex, colourTableIndex, error);
}

std::optional<Colour> Palette::getColour(uint8_t x, uint8_t y, uint8_t colourTableIndex) const {
	bool error = false;

	Colour colour(getColour(x, y, colourTableIndex, &error));

	if(error) {
		return {};
	}

	return colour;
}

bool Palette::setColour(uint8_t colourIndex, const Colour & colour, uint8_t colourTableIndex) {
	std::shared_ptr<ColourTable> colourTable(getColourTable(colourTableIndex));

	if(colourTable == nullptr) {
		return false;
	}

	return colourTable->setColour(colourIndex, colour);
}

bool Palette::setColour(uint8_t x, uint8_t y, const Colour & colour, uint8_t colourTableIndex) {
	uint8_t colourIndex = getColourIndex(x, y);

	if(colourIndex >= NUMBER_OF_COLOURS) {
		return false;
	}

	return setColour(colourIndex, colour, colourTableIndex);
}

std::vector<std::shared_ptr<Palette::ColourTable>> Palette::getAllColourTables() const {
	bool error = false;
	uint8_t colourTableCount = numberOfColourTables();
	std::shared_ptr<ColourTable> colourTable;
	std::vector<std::shared_ptr<ColourTable>> colourTables;

	for(uint8_t i = 0; i < colourTableCount; i++) {
		colourTable = getColourTable(i);

		if(colourTable == nullptr) {
			return {};
		}

		colourTables.emplace_back(colourTable);
	}

	return colourTables;
}

bool Palette::updateColourTable(uint8_t colourTableIndex, const ColourTable & colourTable) {
	std::shared_ptr<ColourTable> destinationColourTable(getColourTable(colourTableIndex));

	if(destinationColourTable == nullptr) {
		return false;
	}

	return destinationColourTable->setColours(colourTable);
}

bool Palette::updateAllColourTables(const std::vector<std::shared_ptr<const ColourTable>> & colourTables) {
	uint8_t colourTableCount = numberOfColourTables();

	if(colourTables.size() != colourTableCount) {
		return false;
	}

	for(uint8_t i = 0; i < colourTableCount; i++) {
		if(!updateColourTable(i, *colourTables[i])) {
			return false;
		}
	}

	return true;
}

bool Palette::fillColourTableWithColour(const Colour & colour, uint8_t colourTableIndex) {
	std::shared_ptr<ColourTable> colourTable(getColourTable(colourTableIndex));

	if(colourTable == nullptr) {
		return false;
	}

	colourTable->fillWithColour(colour);

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

const std::string & Palette::getColourTableName(uint8_t colourTableIndex) const {
	std::shared_ptr<ColourTable> colourTable(getColourTable(colourTableIndex));

	if(colourTable == nullptr) {
		return Utilities::emptyString;
	}

	return colourTable->getName();
}

void Palette::addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const {
	uint8_t colourTableCount = numberOfColourTables();

	metadata.push_back({ "Number of Colour Tables", std::to_string(colourTableCount) });

	if(colourTableCount == 1) {
		std::string colourTableName(getColourTableName());
		std::optional<uint16_t> optionalNumberOfColours(numberOfColours());
		std::optional<uint8_t> optionalTransparentColourIndex(getTransparentColourIndex());

		if(!colourTableName.empty()) {
			metadata.push_back({ "Palette Name", colourTableName });
		}

		metadata.push_back({ "Number of Colours", optionalNumberOfColours.has_value() ? std::to_string(optionalNumberOfColours.value()) : "N/A" });
		metadata.push_back({ "Transparent Colour Index", optionalTransparentColourIndex.has_value() ? std::to_string(optionalTransparentColourIndex.value()) : "N/A" });
	}
	else {
		for(uint8_t i = 0; i < colourTableCount; i++) {
			std::string colourTableName(getColourTableName(i));
			std::optional<uint16_t> optionalNumberOfColours(numberOfColours(i));
			std::optional<uint8_t> optionalTransparentColourIndex(getTransparentColourIndex(i));

			metadata.push_back({ fmt::format("Colour Table #{}", i + 1), fmt::format("'{}' Number of Colours: {} Transparent Colour Index: {}", colourTableName.empty() ? "Untitled" : colourTableName, optionalNumberOfColours.has_value() ? std::to_string(optionalNumberOfColours.value()) : "N/A", optionalTransparentColourIndex.has_value() ? std::to_string(optionalTransparentColourIndex.value()) : "N/A") });
		}
	}
}

bool Palette::isValid(bool verifyParent) const {
	uint8_t colourTableCount = numberOfColourTables();

	if(colourTableCount == 0) {
		return false;
	}

	for(uint8_t i = 0; i < colourTableCount; i++) {
		std::shared_ptr<ColourTable> colourTable(getColourTable(i));

		if(!ColourTable::isValid(colourTable.get())) {
			return false;
		}

		if(verifyParent) {
			if(colourTable->getParent() != this) {
				return false;
			}
		}
	}

	return true;
}

bool Palette::isValid(const Palette * palette, bool verifyParent) {
	return palette != nullptr &&
		   palette->isValid();
}
