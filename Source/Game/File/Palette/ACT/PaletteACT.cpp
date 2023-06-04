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

PaletteACT::PaletteACT(std::vector<Colour> colours, std::optional<uint16_t> transparentColourIndex, const std::string & filePath)
	: Palette(BYTES_PER_COLOUR, filePath)
	, m_colours(std::move(colours))
	, m_transparentColourIndex(transparentColourIndex) { }

PaletteACT::PaletteACT(PaletteACT && palette) noexcept
	: Palette(palette)
	, m_colours(std::move(palette.m_colours))
	, m_transparentColourIndex(palette.m_transparentColourIndex) { }

PaletteACT::PaletteACT(const PaletteACT & palette)
	: Palette(palette)
	, m_colours(palette.m_colours)
	, m_transparentColourIndex(palette.m_transparentColourIndex) { }

PaletteACT & PaletteACT::operator = (PaletteACT && palette) noexcept {
	if(this != &palette) {
		Palette::operator = (palette);

		m_colours = std::move(palette.m_colours);
		m_transparentColourIndex = palette.m_transparentColourIndex;
	}

	return *this;
}

PaletteACT & PaletteACT::operator = (const PaletteACT & palette) {
	Palette::operator = (palette);

	m_colours = palette.m_colours;
	m_transparentColourIndex = palette.m_transparentColourIndex;

	return *this;
}

PaletteACT::~PaletteACT() { }

std::optional<uint16_t> PaletteACT::numberOfColours(uint8_t colourTableIndex) const {
	if(colourTableIndex != 0) {
		return {};
	}

	return static_cast<uint16_t>(m_colours.size());
}

std::optional<uint16_t> PaletteACT::getTransparentColourIndex(uint8_t colourTableIndex) const {
	if(colourTableIndex != 0) {
		return {};
	}

	return m_transparentColourIndex;
}

const Colour & PaletteACT::getColour(uint8_t x, uint8_t y, uint8_t colourTableIndex, bool * error) const {
	if(colourTableIndex != 0) {
		return Colour::INVISIBLE;
	}

	size_t colourIndex = getColourIndex(x, y);

	if(colourIndex == std::numeric_limits<size_t>::max()) {
		return Colour::INVISIBLE;
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
	// https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577411_pgfId-1070626
	//
	// There is no version number written in the file.
	// The file is 768 or 772 bytes long and contains 256 RGB colors.
	// The first color in the table is index zero.
	// There are three bytes per color in the order red, green, blue.
	// If the file is 772 bytes long there are 4 additional bytes remaining.
	// Two bytes for the number of colors to use.
	// Two bytes for the color index with the transparency color to use.
	// If loaded into the Colors palette, the colors will be installed in the color swatch list as RGB colors.

	byteBuffer.setEndianness(ENDIANNESS);

	size_t minimumSizeBytes = NUMBER_OF_COLOURS * BYTES_PER_COLOUR;

	if(!byteBuffer.canReadBytes(minimumSizeBytes)) {
		spdlog::error("Adobe Photoshop ACT palette binary data is truncated, expected at least {} bytes but only found {}.", minimumSizeBytes, byteBuffer.numberOfBytesRemaining());
		return nullptr;
	}

	std::vector<Colour> colours;
	colours.reserve(NUMBER_OF_COLOURS);

	bool error = false;

	for(uint16_t i = 0; i < NUMBER_OF_COLOURS; i++) {
		colours.emplace_back(Colour::readFrom(byteBuffer, false, &error));

		if(error) {
			spdlog::error("Failed to read Adobe Photoshop ACT palette colour #{} from binary data.", i + 1);
			return nullptr;
		}
	}

	std::optional<uint16_t> optionalTransparentColourIndex;
	size_t bytesRemaining = byteBuffer.numberOfBytesRemaining();

	if(bytesRemaining == 4) {
		uint16_t numberOfColours = byteBuffer.readUnsignedShort(&error);

		if(error) {
			spdlog::error("Failed to read Adobe Photoshop ACT palette number of colours value.");
			return nullptr;
		}

		if(colours.size() != numberOfColours) {
			spdlog::info("Resizing Adobe Photoshop ACT palette colour list from {} to {} colours .", colours.size(), numberOfColours);

			colours.resize(numberOfColours);
		}

		optionalTransparentColourIndex = byteBuffer.readUnsignedShort(&error);

		if(error) {
			spdlog::error("Missing Adobe Photoshop ACT palette transparent colour index.");
			return nullptr;
		}

		if(optionalTransparentColourIndex.value() >= numberOfColours) {
			spdlog::error("Adobe Photoshop ACT palette transparent colour index of {} must be less than the number of colours ({}).", optionalTransparentColourIndex.value(), numberOfColours);
			return nullptr;
		}

		size_t finalBytesRemaining = byteBuffer.numberOfBytesRemaining();

		if(finalBytesRemaining != 0) {
			spdlog::warn("Adobe Photoshop ACT palette contains an unexpected additional {} byte{} after the transparent colour index, they will be discarded.", finalBytesRemaining, finalBytesRemaining == 1 ? "" : "s");
		}
	}
	else if(bytesRemaining != 0) {
		spdlog::warn("Adobe Photoshop ACT palette contains an unexpected additional {} byte{} after the colour data, they will be discarded.", bytesRemaining == 1 ? "" : "s");
	}

	return std::make_unique<PaletteACT>(colours, optionalTransparentColourIndex);
}

std::unique_ptr<PaletteACT> PaletteACT::loadFrom(const std::string & filePath) {
	std::unique_ptr<ByteBuffer> paletteData(ByteBuffer::readFrom(filePath));

	if(paletteData == nullptr) {
		spdlog::error("Failed to read Adobe Photoshop ACT palette binary data from file: '{}'.", filePath);
		return nullptr;
	}

	std::unique_ptr<PaletteACT> palette(PaletteACT::readFrom(*paletteData));

	if(palette == nullptr) {
		spdlog::error("Failed to parse Adobe Photoshop ACT palette binary data from file contents: '{}'.", filePath);
		return nullptr;
	}

	return palette;
}

bool PaletteACT::writeTo(ByteBuffer & byteBuffer) const {
	byteBuffer.setEndianness(ENDIANNESS);

	for(uint16_t i = 0; i < m_colours.size(); i++) {
		if(!m_colours[i].writeTo(byteBuffer, false)) {
			return false;
		}
	}

	if(m_transparentColourIndex.has_value()) {
		if(!byteBuffer.writeUnsignedShort(static_cast<uint16_t>(m_colours.size()))) {
			return false;
		}

		if(!byteBuffer.writeUnsignedShort(m_transparentColourIndex.value())) {
			return false;
		}
	}

	return true;
}

Endianness PaletteACT::getEndianness() const {
	return ENDIANNESS;
}

size_t PaletteACT::getSizeInBytes() const {
	return NUMBER_OF_COLOURS * BYTES_PER_COLOUR + (m_transparentColourIndex.has_value() ? (sizeof(uint16_t) * 2) : 0);
}

bool PaletteACT::isValid(bool verifyParent) const {
	return true;
}
