#include "PaletteJASC.h"

#include <ByteBuffer.h>
#include <Utilities/NumberUtilities.h>
#include <Utilities/StringUtilities.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

PaletteJASC::PaletteJASC(const std::string & filePath)
	: Palette(filePath)
	, m_colourTable(std::make_shared<ColourTable>())
	, m_version(VERSION) {
	updateParent();
}

PaletteJASC::PaletteJASC(std::unique_ptr<ColourTable> colourTable, const std::string & version, const std::string & filePath)
	: Palette(filePath)
	, m_colourTable(colourTable != nullptr ? std::shared_ptr<ColourTable>(colourTable.release()) : std::make_shared<ColourTable>())
	, m_version(version) {
	updateParent();
}

PaletteJASC::PaletteJASC(PaletteJASC && palette) noexcept
	: Palette(palette)
	, m_colourTable(std::move(palette.m_colourTable))
	, m_version(std::move(palette.m_version)) {
	updateParent();
}

PaletteJASC::PaletteJASC(const PaletteJASC & palette)
	: Palette(palette)
	, m_colourTable(palette.m_colourTable)
	, m_version(palette.m_version) {
	updateParent();
}

PaletteJASC & PaletteJASC::operator = (PaletteJASC && palette) noexcept {
	if(this != &palette) {
		Palette::operator = (palette);

		m_colourTable = std::move(palette.m_colourTable);
		m_version = std::move(palette.m_version);

		updateParent();
	}

	return *this;
}

PaletteJASC & PaletteJASC::operator = (const PaletteJASC & palette) {
	Palette::operator = (palette);

	m_colourTable = palette.m_colourTable;
	m_version = palette.m_version;

	updateParent();

	return *this;
}

PaletteJASC::~PaletteJASC() { }

const std::string & PaletteJASC::getVersion() const {
	return m_version;
}

std::shared_ptr<ColourTable> PaletteJASC::getColourTable(uint8_t colourTableIndex) const {
	if(colourTableIndex != 0) {
		return nullptr;
	}

	return m_colourTable;
}

std::unique_ptr<PaletteJASC> PaletteJASC::readFrom(const ByteBuffer & byteBuffer) {
	bool error = false;

	std::string magic(byteBuffer.readString(MAGIC_SIZE_BYTES, &error));

	if(error) {
		spdlog::error("Paint Shop Pro JASC palette is missing '{}' magic identifier.", MAGIC);
		return nullptr;
	}

	if(!Utilities::areStringsEqual(magic, MAGIC)) {
		spdlog::error("Paint Shop Pro JASC palette has invalid magic identifier '{}', expected '{}'.", magic, MAGIC);
		return nullptr;
	}

	if(!byteBuffer.skipToNextLine()) {
		spdlog::error("Truncated Paint Shop Pro JASC palette data, missing version specifier.");
		return nullptr;
	}

	std::string version(byteBuffer.readLine(&error));

	if(error) {
		spdlog::error("Failed to read Paint Shop Pro JASC palette version data.");
		return nullptr;
	}

	if(!Utilities::areStringsEqual(version, VERSION)) {
		spdlog::error("Unsupported Paint Shop Pro JASC palette version '{}', only version '{}' is supported.", version, VERSION);
		return nullptr;
	}

	std::string colourCountData(byteBuffer.readLine(&error));

	if(error) {
		spdlog::error("Paint Shop Pro JASC palette is missing number of colours information.");
		return nullptr;
	}

	uint16_t colourCount = Utilities::parseUnsignedShort(colourCountData, &error);

	if(error) {
		spdlog::error("Invalid Paint Shop Pro JASC palette number of colours: {}.", colourCountData);
		return nullptr;
	}

	if(colourCount > ColourTable::NUMBER_OF_COLOURS) {
		spdlog::error("Invalid Paint Shop Pro JASC palette number of colours: {}, expected unsigned integer less than or equal to {}.", colourCount, ColourTable::NUMBER_OF_COLOURS);
		return nullptr;
	}

	std::string colourData;
	size_t colourChannelStartOffset = 0;
	size_t colourChannelEndOffset = std::numeric_limits<size_t>::max();
	Colour colour;
	std::vector<Colour> colours;
	colours.reserve(colourCount);

	for(size_t i = 0; i < colourCount; i++) {
		colourChannelStartOffset = 0;

		colourData = Utilities::trimString(byteBuffer.readLine(&error));

		if(error) {
			spdlog::error("Failed to read Paint Shop Pro JASC palette colour #{} data.", i + 1);
			return nullptr;
		}

		colourChannelEndOffset = colourData.find_first_of(" \t");

		if(colourChannelEndOffset == std::string::npos) {
			spdlog::error("Paint Shop Pro JASC palette colour #{} data is malformed, failed to read red colour channel from: '{}'.", i + 1, colourData);
			return nullptr;
		}

		colourChannelEndOffset--;

		colour.r = Utilities::parseUnsignedByte(std::string(colourData.data() + colourChannelStartOffset, colourChannelEndOffset - colourChannelStartOffset + 1), &error);

		if(error) {
			spdlog::error("Invalid Paint Shop Pro JASC palette colour #{} red channel value in colour data: '{}'.", i + 1, colourData);
			return nullptr;
		}

		colourChannelStartOffset = colourData.find_first_not_of(" \t", colourChannelEndOffset + 1);

		if(colourChannelStartOffset == std::string::npos) {
			spdlog::error("Paint Shop Pro JASC palette colour #{} data is malformed, failed to locate start of green colour channel from: '{}'.", i + 1, colourData);
			return nullptr;
		}

		colourChannelEndOffset = colourData.find_first_of(" \t", colourChannelStartOffset);

		if(colourChannelEndOffset == std::string::npos) {
			spdlog::error("Paint Shop Pro JASC palette colour #{} data is malformed, failed to locate end of green colour channel from: '{}'.", i + 1, colourData);
			return nullptr;
		}

		colourChannelEndOffset--;

		colour.g = Utilities::parseUnsignedByte(std::string(colourData.data() + colourChannelStartOffset, colourChannelEndOffset - colourChannelStartOffset + 1), &error);

		if(error) {
			spdlog::error("Invalid Paint Shop Pro JASC palette colour #{} green channel value in colour data: '{}'.", i + 1, colourData);
			return nullptr;
		}

		colourChannelStartOffset = colourData.find_first_not_of(" \t", colourChannelEndOffset + 1);

		if(colourChannelStartOffset == std::string::npos) {
			spdlog::error("Paint Shop Pro JASC palette colour #{} data is malformed, failed to locate start of blue colour channel from: '{}'.", i + 1, colourData);
			return nullptr;
		}

		colour.b = Utilities::parseUnsignedByte(std::string(colourData.data() + colourChannelStartOffset, colourData.length() - colourChannelStartOffset), &error);

		if(error) {
			spdlog::error("Invalid Paint Shop Pro JASC palette colour #{} blue channel value in colour data: '{}'.", i + 1, colourData);
			return nullptr;
		}

		colours.emplace_back(colour);
	}

	return std::make_unique<PaletteJASC>(std::make_unique<ColourTable>(std::move(colours)), version);
}

std::unique_ptr<PaletteJASC> PaletteJASC::loadFrom(const std::string & filePath) {
	std::unique_ptr<ByteBuffer> paletteData(ByteBuffer::readFrom(filePath));

	if(paletteData == nullptr) {
		spdlog::error("Failed to read Paint Shop Pro JASC palette binary data from file: '{}'.", filePath);
		return nullptr;
	}

	std::unique_ptr<PaletteJASC> palette(PaletteJASC::readFrom(*paletteData));

	if(palette == nullptr) {
		spdlog::error("Failed to parse Paint Shop Pro JASC palette binary data from file contents: '{}'.", filePath);
		return nullptr;
	}

	return palette;
}

bool PaletteJASC::writeTo(ByteBuffer & byteBuffer) const {
	byteBuffer.writeLine(MAGIC);

	byteBuffer.writeLine(VERSION);

	byteBuffer.writeLine(std::to_string(m_colourTable->numberOfColours()));

	for(const Colour & colour : m_colourTable->getColours()) {
		if(!byteBuffer.writeLine(fmt::format("{} {} {}", colour.r, colour.g, colour.b))) {
			return false;
		}
	}

	uint16_t paddingAmount = ColourTable::NUMBER_OF_COLOURS - m_colourTable->numberOfColours();

	for(uint16_t i = 0; i < paddingAmount; i++) {
		if(!byteBuffer.writeLine("0 0 0")) {
			return false;
		}
	}

	return true;
}

void PaletteJASC::addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const {
	Palette::addMetadata(metadata);

	metadata.push_back({ "Version", m_version });
}

Endianness PaletteJASC::getEndianness() const {
	return {};
}

size_t PaletteJASC::getSizeInBytes() const {
	size_t sizeBytes = MAGIC.length() + VERSION.length() + Utilities::unsignedShortLength(m_colourTable->numberOfColours()) + (sizeof(uint8_t) * 3);

	for(const Colour & colour : m_colourTable->getColours()) {
		for(uint8_t i = 0; i < 3; i++) {
			sizeBytes += Utilities::unsignedByteLength(colour.c[i]) + sizeof(uint8_t);
		}
	}

	return sizeBytes + ((ColourTable::NUMBER_OF_COLOURS - m_colourTable->numberOfColours()) * (sizeof(uint8_t) * 6));
}

void PaletteJASC::updateParent() {
	m_colourTable->setParent(this);
}

bool PaletteJASC::operator == (const PaletteJASC & palette) const {
	return *m_colourTable == *palette.m_colourTable &&
		   Utilities::areStringsEqual(m_version, palette.m_version);
}

bool PaletteJASC::operator != (const PaletteJASC & palette) const {
	return !operator == (palette);
}
