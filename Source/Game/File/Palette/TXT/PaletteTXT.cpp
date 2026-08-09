#include "PaletteTXT.h"

#include <ByteBuffer.h>
#include <Utilities/NumberUtilities.h>
#include <Utilities/StringUtilities.h>

#include <CSSColorParser/css_color.hpp>
#include <spdlog/spdlog.h>

PaletteTXT::PaletteTXT(const std::string & filePath)
	: Palette(filePath)
	, m_colourTable(std::make_shared<ColourTable>()) {
	updateParent();
}

PaletteTXT::PaletteTXT(std::unique_ptr<ColourTable> colourTable, const std::string & filePath)
	: Palette(filePath)
	, m_colourTable(colourTable != nullptr ? std::move(colourTable) : std::make_shared<ColourTable>()) {
	updateParent();
}

PaletteTXT::PaletteTXT(PaletteTXT && palette) noexcept
	: Palette(std::move(palette))
	, m_colourTable(std::move(palette.m_colourTable)) {
	updateParent();
}

PaletteTXT::PaletteTXT(const PaletteTXT & palette)
	: Palette(palette)
	, m_colourTable(palette.m_colourTable) {
	updateParent();
}

PaletteTXT & PaletteTXT::operator = (PaletteTXT && palette) noexcept {
	if(this != &palette) {
		Palette::operator = (std::move(palette));

		m_colourTable = std::move(palette.m_colourTable);

		updateParent();
	}

	return *this;
}

PaletteTXT & PaletteTXT::operator = (const PaletteTXT & palette) {
	Palette::operator = (palette);

	m_colourTable = palette.m_colourTable;

	updateParent();

	return *this;
}

PaletteTXT::~PaletteTXT() { }

std::shared_ptr<ColourTable> PaletteTXT::getColourTable(uint8_t colourTableIndex) const {
	if(colourTableIndex != 0) {
		return nullptr;
	}

	return m_colourTable;
}

std::unique_ptr<PaletteTXT> PaletteTXT::readFrom(const ByteBuffer & byteBuffer) {
	std::string line;
	bool error = false;
	css_colors::color colour;
	std::vector<Colour> colours;
	colours.reserve(256);

	while(true) {
		line = byteBuffer.readLine(&error);

		if(error || line.empty()) {
			break;
		}

		colour = css_colors::parse(line.data());

		if(!colour) {
			return nullptr;
		}

		auto formattedColour = colour.as<css_colors::colorspaces::srgb>();

		colours.emplace_back(static_cast<uint8_t>(formattedColour.second.first[0]), static_cast<uint8_t>(formattedColour.second.first[1]), static_cast<uint8_t>(formattedColour.second.first[2]), static_cast<uint8_t>(formattedColour.second.second * 255));
	}

	if(colours.empty()) {
		return nullptr;
	}

	return std::make_unique<PaletteTXT>(std::make_unique<ColourTable>(std::move(colours)));
}

std::unique_ptr<PaletteTXT> PaletteTXT::loadFrom(const std::string & filePath) {
	std::unique_ptr<ByteBuffer> paletteData(ByteBuffer::readFrom(filePath));

	if(paletteData == nullptr) {
		spdlog::error("Failed to read TXT palette data from file: '{}'.", filePath);
		return nullptr;
	}

	std::unique_ptr<PaletteTXT> palette(PaletteTXT::readFrom(*paletteData));

	if(palette == nullptr) {
		spdlog::error("Failed to parse TXT palette data from file contents: '{}'.", filePath);
		return nullptr;
	}

	return palette;
}

bool PaletteTXT::writeTo(ByteBuffer & byteBuffer) const {
	for(const Colour & colour : m_colourTable->getColours()) {
		if(!byteBuffer.writeString(fmt::format("#{}{}{}\n", Utilities::toHexadecimal(colour.r), Utilities::toHexadecimal(colour.g), Utilities::toHexadecimal(colour.b)))) {
			return false;
		}
	}

	return true;
}

Endianness PaletteTXT::getEndianness() const {
	return {};
}

size_t PaletteTXT::getSizeInBytes() const {
	return m_colourTable->numberOfColours() * 8u;
}

void PaletteTXT::updateParent() {
	m_colourTable->setParent(this);
}

bool PaletteTXT::operator == (const PaletteTXT & palette) const {
	if(this == &palette) {
		return true;
	}

	return *m_colourTable == *palette.m_colourTable;
}

bool PaletteTXT::operator != (const PaletteTXT & palette) const {
	return !operator == (palette);
}
