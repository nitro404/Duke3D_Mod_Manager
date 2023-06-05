#include "PaletteACT.h"

#include <ByteBuffer.h>

#include <spdlog/spdlog.h>

PaletteACT::PaletteACT(const std::string & filePath)
	: Palette(filePath)
	, m_colourTable(std::make_shared<ColourTable>()) {
	updateParent();
}

PaletteACT::PaletteACT(std::unique_ptr<ColourTable> colourTable, const std::string & filePath)
	: Palette(filePath)
	, m_colourTable(colourTable != nullptr ? std::shared_ptr<ColourTable>(colourTable.release()) : std::make_shared<ColourTable>()) {
	updateParent();
}

PaletteACT::PaletteACT(PaletteACT && palette) noexcept
	: Palette(palette)
	, m_colourTable(std::move(palette.m_colourTable)) {
	updateParent();
}

PaletteACT::PaletteACT(const PaletteACT & palette)
	: Palette(palette)
	, m_colourTable(palette.m_colourTable) {
	updateParent();
}

PaletteACT & PaletteACT::operator = (PaletteACT && palette) noexcept {
	if(this != &palette) {
		Palette::operator = (palette);

		m_colourTable = std::move(palette.m_colourTable);

		updateParent();
	}

	return *this;
}

PaletteACT & PaletteACT::operator = (const PaletteACT & palette) {
	Palette::operator = (palette);

	m_colourTable = palette.m_colourTable;

	updateParent();

	return *this;
}

PaletteACT::~PaletteACT() { }

std::shared_ptr<Palette::ColourTable> PaletteACT::getColourTable(uint8_t colourTableIndex) const {
	if(colourTableIndex != 0) {
		return nullptr;
	}

	return m_colourTable;
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

	std::optional<uint8_t> optionalTransparentColourIndex;
	size_t bytesRemaining = byteBuffer.numberOfBytesRemaining();

	if(bytesRemaining == 4) {
		uint16_t numberOfColours = byteBuffer.readUnsignedShort(&error);

		if(error) {
			spdlog::error("Failed to read Adobe Photoshop ACT palette number of colours value.");
			return nullptr;
		}

		if(colours.size() != numberOfColours) {
			spdlog::debug("Resizing Adobe Photoshop ACT palette colour list from {} to {} colours.", colours.size(), numberOfColours);

			colours.resize(numberOfColours);
		}

		std::optional<uint16_t> optionalFullTransparentColourIndex(byteBuffer.readUnsignedShort(&error));

		if(error) {
			spdlog::error("Missing Adobe Photoshop ACT palette transparent colour index.");
			return nullptr;
		}

		if(optionalFullTransparentColourIndex.value() < numberOfColours) {
			optionalTransparentColourIndex = static_cast<uint8_t>(optionalFullTransparentColourIndex.value());
		}
		else if(optionalFullTransparentColourIndex.value() != std::numeric_limits<uint16_t>::max()) {
			// no transparent colour index is denoted using a value of 65535, so any other value is considered as invalid
			spdlog::error("Adobe Photoshop ACT palette transparent colour index of {} must be less than the number of colours ({}).", optionalFullTransparentColourIndex.value(), numberOfColours);
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

	return std::make_unique<PaletteACT>(std::make_unique<ColourTable>(std::move(colours), optionalTransparentColourIndex));
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

	if(!m_colourTable->writeTo(byteBuffer, false, Colour::BLACK)) {
		return false;
	}

	if(m_colourTable->hasTransparentColourIndex() || m_colourTable->numberOfColours() != NUMBER_OF_COLOURS) {
		if(!byteBuffer.writeUnsignedShort(m_colourTable->numberOfColours())) {
			return false;
		}

		if(m_colourTable->hasTransparentColourIndex()) {
			if(!byteBuffer.writeUnsignedShort(m_colourTable->getTransparentColourIndex().value())) {
				return false;
			}
		}
		else {
			if(!byteBuffer.writeUnsignedShort(std::numeric_limits<uint16_t>::max())) {
				return false;
			}
		}
	}

	return true;
}

Endianness PaletteACT::getEndianness() const {
	return ENDIANNESS;
}

size_t PaletteACT::getSizeInBytes() const {
	return NUMBER_OF_COLOURS * BYTES_PER_COLOUR + (m_colourTable->hasTransparentColourIndex() || m_colourTable->numberOfColours() != NUMBER_OF_COLOURS ? (sizeof(uint16_t) * 2) : 0);
}

void PaletteACT::updateParent() {
	m_colourTable->setParent(this);
}

bool PaletteACT::operator == (const PaletteACT & palette) const {
	return *m_colourTable == *palette.m_colourTable;
}

bool PaletteACT::operator != (const PaletteACT & palette) const {
	return !operator == (palette);
}
