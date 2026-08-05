#include "PaletteKPL.h"

#include <ByteBuffer.h>

#include <spdlog/spdlog.h>

PaletteKPL::PaletteKPL(const std::string & filePath)
	: Palette(filePath)
	, m_colourTable(std::make_shared<ColourTable>()) {
	updateParent();
}

PaletteKPL::PaletteKPL(std::unique_ptr<ColourTable> colourTable, const std::string & filePath)
	: Palette(filePath)
	, m_colourTable(colourTable != nullptr ? std::move(colourTable) : std::make_shared<ColourTable>()) {
	updateParent();
}

PaletteKPL::PaletteKPL(PaletteKPL && palette) noexcept
	: Palette(std::move(palette))
	, m_colourTable(std::move(palette.m_colourTable)) {
	updateParent();
}

PaletteKPL::PaletteKPL(const PaletteKPL & palette)
	: Palette(palette)
	, m_colourTable(palette.m_colourTable) {
	updateParent();
}

PaletteKPL & PaletteKPL::operator = (PaletteKPL && palette) noexcept {
	if(this != &palette) {
		Palette::operator = (std::move(palette));

		m_colourTable = std::move(palette.m_colourTable);

		updateParent();
	}

	return *this;
}

PaletteKPL & PaletteKPL::operator = (const PaletteKPL & palette) {
	Palette::operator = (palette);

	m_colourTable = palette.m_colourTable;

	updateParent();

	return *this;
}

PaletteKPL::~PaletteKPL() { }

std::shared_ptr<ColourTable> PaletteKPL::getColourTable(uint8_t colourTableIndex) const {
	if(colourTableIndex != 0) {
		return nullptr;
	}

	return m_colourTable;
}

std::unique_ptr<PaletteKPL> PaletteKPL::readFrom(const ByteBuffer & byteBuffer) {
	// TODO:
	return nullptr;
}

std::unique_ptr<PaletteKPL> PaletteKPL::loadFrom(const std::string & filePath) {
	std::unique_ptr<ByteBuffer> paletteData(ByteBuffer::readFrom(filePath));

	if(paletteData == nullptr) {
		spdlog::error("Failed to read Krita palette data from file: '{}'.", filePath);
		return nullptr;
	}

	std::unique_ptr<PaletteKPL> palette(PaletteKPL::readFrom(*paletteData));

	if(palette == nullptr) {
		spdlog::error("Failed to parse Krita palette data from file contents: '{}'.", filePath);
		return nullptr;
	}

	return palette;
}

bool PaletteKPL::writeTo(ByteBuffer & byteBuffer) const {
	// TODO:
	return false;
}

Endianness PaletteKPL::getEndianness() const {
	return ENDIANNESS;
}

size_t PaletteKPL::getSizeInBytes() const {
	// TODO:
	return 0u;
}

void PaletteKPL::updateParent() {
	m_colourTable->setParent(this);
}

bool PaletteKPL::operator == (const PaletteKPL & palette) const {
	if(this == &palette) {
		return true;
	}

	return *m_colourTable == *palette.m_colourTable;
}

bool PaletteKPL::operator != (const PaletteKPL & palette) const {
	return !operator == (palette);
}
