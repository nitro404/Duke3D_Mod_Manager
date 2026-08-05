#include "PaletteCSS.h"

#include <ByteBuffer.h>

#include <spdlog/spdlog.h>

PaletteCSS::PaletteCSS(const std::string & filePath)
	: Palette(filePath)
	, m_colourTable(std::make_shared<ColourTable>()) {
	updateParent();
}

PaletteCSS::PaletteCSS(std::unique_ptr<ColourTable> colourTable, const std::string & filePath)
	: Palette(filePath)
	, m_colourTable(colourTable != nullptr ? std::move(colourTable) : std::make_shared<ColourTable>()) {
	updateParent();
}

PaletteCSS::PaletteCSS(PaletteCSS && palette) noexcept
	: Palette(std::move(palette))
	, m_colourTable(std::move(palette.m_colourTable)) {
	updateParent();
}

PaletteCSS::PaletteCSS(const PaletteCSS & palette)
	: Palette(palette)
	, m_colourTable(palette.m_colourTable) {
	updateParent();
}

PaletteCSS & PaletteCSS::operator = (PaletteCSS && palette) noexcept {
	if(this != &palette) {
		Palette::operator = (std::move(palette));

		m_colourTable = std::move(palette.m_colourTable);

		updateParent();
	}

	return *this;
}

PaletteCSS & PaletteCSS::operator = (const PaletteCSS & palette) {
	Palette::operator = (palette);

	m_colourTable = palette.m_colourTable;

	updateParent();

	return *this;
}

PaletteCSS::~PaletteCSS() { }

std::shared_ptr<ColourTable> PaletteCSS::getColourTable(uint8_t colourTableIndex) const {
	if(colourTableIndex != 0) {
		return nullptr;
	}

	return m_colourTable;
}

std::unique_ptr<PaletteCSS> PaletteCSS::readFrom(const ByteBuffer & byteBuffer) {
	// TODO:
	return nullptr;
}

std::unique_ptr<PaletteCSS> PaletteCSS::loadFrom(const std::string & filePath) {
	std::unique_ptr<ByteBuffer> paletteData(ByteBuffer::readFrom(filePath));

	if(paletteData == nullptr) {
		spdlog::error("Failed to read CSS palette data from file: '{}'.", filePath);
		return nullptr;
	}

	std::unique_ptr<PaletteCSS> palette(PaletteCSS::readFrom(*paletteData));

	if(palette == nullptr) {
		spdlog::error("Failed to parse CSS palette data from file contents: '{}'.", filePath);
		return nullptr;
	}

	return palette;
}

bool PaletteCSS::writeTo(ByteBuffer & byteBuffer) const {
	// TODO:
	return false;
}

Endianness PaletteCSS::getEndianness() const {
	return {};
}

size_t PaletteCSS::getSizeInBytes() const {
	// TODO:
	return 0u;
}

void PaletteCSS::updateParent() {
	m_colourTable->setParent(this);
}

bool PaletteCSS::operator == (const PaletteCSS & palette) const {
	if(this == &palette) {
		return true;
	}

	return *m_colourTable == *palette.m_colourTable;
}

bool PaletteCSS::operator != (const PaletteCSS & palette) const {
	return !operator == (palette);
}
