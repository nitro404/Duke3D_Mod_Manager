#include "PaletteACT.h"

#include <ByteBuffer.h>

#include <spdlog/spdlog.h>

static size_t getColourIndex(uint8_t x, uint8_t y) {
	if(x >= Palette::PALETTE_WIDTH || y >= Palette::PALETTE_HEIGHT) {
		return std::numeric_limits<size_t>::max();
	}

	return (static_cast<size_t>(x)) + (static_cast<size_t>(y) * Palette::PALETTE_HEIGHT);
}

PaletteACT::PaletteACT(const std::string & filePath)
	: Palette(BYTES_PER_COLOUR, filePath) { }

PaletteACT::PaletteACT(std::array<Colour, NUMBER_OF_COLOURS> colours, const std::string & filePath)
	: Palette(BYTES_PER_COLOUR, filePath)
	, m_colours(std::move(colours)) { }

PaletteACT::PaletteACT(PaletteACT && palette) noexcept
	: Palette(palette)
	, m_colours(std::move(palette.m_colours)) { }

PaletteACT::PaletteACT(const PaletteACT & palette)
	: Palette(palette)
	, m_colours(palette.m_colours) { }

PaletteACT & PaletteACT::operator = (PaletteACT && palette) noexcept {
	if(this != &palette) {
		Palette::operator = (palette);

		m_colours = std::move(palette.m_colours);
	}

	return *this;
}

PaletteACT & PaletteACT::operator = (const PaletteACT & palette) {
	Palette::operator = (palette);

	m_colours = palette.m_colours;

	return *this;
}

PaletteACT::~PaletteACT() { }

Colour PaletteACT::getColour(uint8_t x, uint8_t y, uint8_t colourTableIndex, bool * error) const {
	if(colourTableIndex != 0) {
		return {};
	}

	size_t colourIndex = getColourIndex(x, y);

	if(colourIndex == std::numeric_limits<size_t>::max()) {
		return {};
	}

	return m_colours[colourIndex];
}

bool PaletteACT::updateColour(uint8_t x, uint8_t y, const Colour & colour, uint8_t colourTableIndex) {
	if(colourTableIndex != 0) {
		return false;
	}

	size_t colourIndex = getColourIndex(x, y);

	if(colourIndex == std::numeric_limits<size_t>::max()) {
		return false;
	}

	m_colours[colourIndex] = colour;

	return true;
}

std::unique_ptr<PaletteACT> PaletteACT::readFrom(const ByteBuffer & byteBuffer) {
	if(!byteBuffer.canReadBytes(SIZE_BYTES)) {
		return nullptr;
	}

	std::array<Colour, NUMBER_OF_COLOURS> colours;

	bool error = false;

	for(uint16_t i = 0; i < NUMBER_OF_COLOURS; i++) {
		colours[i] = Colour::readFrom(byteBuffer, false, &error);

		if(error) {
			return nullptr;
		}
	}

	return std::make_unique<PaletteACT>(colours);
}

std::unique_ptr<PaletteACT> PaletteACT::loadFrom(const std::string & filePath) {
	std::unique_ptr<ByteBuffer> paletteData(ByteBuffer::readFrom(filePath));

	if(paletteData == nullptr) {
		spdlog::error("Failed to read ACT palette binary data from file: '{}'.", filePath);
		return nullptr;
	}

	std::unique_ptr<PaletteACT> palette(PaletteACT::readFrom(*paletteData));

	if(palette == nullptr) {
		spdlog::error("Failed to parse ACT palette binary data from file contents: '{}'.", filePath);
		return nullptr;
	}

	return palette;
}

bool PaletteACT::writeTo(ByteBuffer & byteBuffer) const {
	for(uint16_t i = 0; i < m_colours.size(); i++) {
		if(!m_colours[i].writeTo(byteBuffer, false)) {
			return false;
		}
	}

	return true;
}

Endianness PaletteACT::getEndianness() const {
	return Endianness::LittleEndian;
}

size_t PaletteACT::getSizeInBytes() const {
	return SIZE_BYTES;
}

bool PaletteACT::isValid(bool verifyParent) const {
	return true;
}
