#include "Palette.h"

#include <ByteBuffer.h>

#include <spdlog/spdlog.h>

const Palette::ColourTable Palette::ColourTable::EMPTY_COLOUR_TABLE;

Palette::ColourTable::ColourTable(uint16_t numberOfColours, std::optional<uint8_t> transparentColourIndex, bool alpha, Palette * parent)
	: m_transparentColourIndex(transparentColourIndex)
	, m_alpha(alpha)
	, m_parent(parent) {
	m_colours.resize(numberOfColours, Colour::INVISIBLE);
}

Palette::ColourTable::ColourTable(const Colour & fillColour, uint16_t numberOfColours, std::optional<uint8_t> transparentColourIndex, bool alpha, Palette * parent)
	: m_transparentColourIndex(transparentColourIndex)
	, m_alpha(alpha)
	, m_parent(parent) {
	m_colours.resize(numberOfColours, fillColour);
}

Palette::ColourTable::ColourTable(std::vector<Colour> && colours, std::optional<uint8_t> transparentColourIndex, bool alpha, Palette * parent)
	: m_colours(std::move(colours))
	, m_transparentColourIndex(transparentColourIndex)
	, m_alpha(alpha)
	, m_parent(parent) { }

Palette::ColourTable::ColourTable(const std::vector<Colour> & colours, std::optional<uint8_t> transparentColourIndex, bool alpha, Palette * parent)
	: m_colours(colours)
	, m_transparentColourIndex(transparentColourIndex)
	, m_alpha(alpha)
	, m_parent(parent) { }

Palette::ColourTable::ColourTable(ColourTable && colourTable) noexcept
	: m_colours(std::move(colourTable.m_colours))
	, m_transparentColourIndex(std::move(colourTable.m_transparentColourIndex))
	, m_alpha(colourTable.m_alpha)
	, m_name(std::move(colourTable.m_name))
	, m_parent(nullptr) { }

Palette::ColourTable::ColourTable(const ColourTable & colourTable)
	: m_colours(colourTable.m_colours)
	, m_transparentColourIndex(colourTable.m_transparentColourIndex)
	, m_alpha(colourTable.m_alpha)
	, m_name(colourTable.m_name)
	, m_parent(nullptr) { }

Palette::ColourTable & Palette::ColourTable::operator = (ColourTable && colourTable) noexcept {
	if(this != &colourTable) {
		m_colours = std::move(colourTable.m_colours);
		m_transparentColourIndex = std::move(colourTable.m_transparentColourIndex);
		m_alpha = colourTable.m_alpha;
		m_name = std::move(colourTable.m_name);
	}

	return *this;
}

Palette::ColourTable & Palette::ColourTable::operator = (const ColourTable & colourTable) {
	m_colours = std::move(colourTable.m_colours);
	m_transparentColourIndex = std::move(colourTable.m_transparentColourIndex);
	m_alpha = colourTable.m_alpha;
	m_name = std::move(colourTable.m_name);

	return *this;
}

Palette::ColourTable::~ColourTable() = default;

bool Palette::ColourTable::hasTransparentColourIndex() const {
	return m_transparentColourIndex.has_value();
}

uint16_t Palette::ColourTable::numberOfColours() const {
	return static_cast<uint16_t>(m_colours.size());
}

const Colour & Palette::ColourTable::getColour(uint8_t colourIndex, bool * error) const {
	if(colourIndex >= m_colours.size()) {
		if(error != nullptr) {
			*error = true;
		}

		return Colour::INVISIBLE;
	}

	return m_colours[colourIndex];
}

const std::vector<Colour> & Palette::ColourTable::getColours() const {
	return m_colours;
}

bool Palette::ColourTable::setColour(uint8_t colourIndex, const Colour & colour) {
	if(colourIndex >= m_colours.size()) {
		return false;
	}

	m_colours[colourIndex].setColour(colour);

	return true;
}

bool Palette::ColourTable::setColours(const ColourTable & colourTable) {
	if(!colourTable.isValid()) {
		return false;
	}

	m_colours = colourTable.m_colours;
	m_transparentColourIndex = colourTable.m_transparentColourIndex;

	return true;
}

void Palette::ColourTable::fillWithColour(const Colour & colour) {
	for(Colour & colour : m_colours) {
		colour.setColour(colour);
	}
}

bool Palette::ColourTable::setNumberOfColours(uint16_t colourCount, const Colour & fillColour) {
	if(colourCount > NUMBER_OF_COLOURS) {
		return false;
	}

	m_colours.resize(colourCount, fillColour);

	if(m_transparentColourIndex.has_value() && m_transparentColourIndex.value() >= m_colours.size()) {
		m_transparentColourIndex.reset();
	}

	return true;
}

const std::optional<uint8_t> & Palette::ColourTable::getTransparentColourIndex() const {
	return m_transparentColourIndex;
}

void Palette::ColourTable::setTransparentColourIndex(uint8_t transparentColourIndex) {
	m_transparentColourIndex = transparentColourIndex;
}

void Palette::ColourTable::clearTransparentColourIndex() {
	m_transparentColourIndex.reset();
}

bool Palette::ColourTable::hasAlphaChannel() const {
	return m_alpha;
}

void Palette::ColourTable::setHasAlphaChannel(bool alpha) {
	m_alpha = alpha;
}

const std::string & Palette::ColourTable::getName() const {
	return m_name;
}

void Palette::ColourTable::setName(const std::string & name) {
	m_name = name;
}

void Palette::ColourTable::clearName() {
	m_name = "";
}

std::unique_ptr<Palette::ColourTable> Palette::ColourTable::getFrom(const ByteBuffer & byteBuffer, size_t offset, uint16_t numberOfColours, bool alpha, Colour::ByteOrder byteOrder) {
	bool error = false;
	size_t newOffset = offset;
	size_t bytesPerPixel = alpha ? 4 : 3;
	size_t totalByteCount = numberOfColours * bytesPerPixel;

	if(byteBuffer.getSize() - offset < totalByteCount) {
		spdlog::error("Colour table data is truncated, expected {} bytes but found {}.", totalByteCount, byteBuffer.getSize() - offset);
		return nullptr;
	}

	std::vector<Colour> colours;
	colours.reserve(numberOfColours);

	for(uint16_t i = 0; i < numberOfColours; i++) {
		colours.emplace_back(Colour::getFrom(byteBuffer, newOffset, alpha, byteOrder, &error));

		if(error) {
			spdlog::error("Failed to read colour table colour #{} from binary data.", i + 1);
			return nullptr;
		}

		newOffset += bytesPerPixel;
	}

	return std::make_unique<ColourTable>(std::move(colours));
}

std::unique_ptr<Palette::ColourTable> Palette::ColourTable::readFrom(const ByteBuffer & byteBuffer, uint16_t numberOfColours, bool alpha, Colour::ByteOrder byteOrder) {
	std::unique_ptr<ColourTable> colourTable(getFrom(byteBuffer, byteBuffer.getReadOffset(), numberOfColours, alpha, byteOrder));

	if(colourTable != nullptr) {
		byteBuffer.skipReadBytes(numberOfColours * (alpha ? 4 : 3));
	}

	return colourTable;
}

bool Palette::ColourTable::putIn(ByteBuffer & byteBuffer, size_t offset, const std::optional<Colour> & paddingColour, Colour::ByteOrder byteOrder) const {
	return putIn(byteBuffer, offset, m_alpha, paddingColour, byteOrder);
}

bool Palette::ColourTable::putIn(ByteBuffer & byteBuffer, size_t offset, bool alpha, const std::optional<Colour> & paddingColour, Colour::ByteOrder byteOrder) const {
	size_t newOffset = offset;
	size_t bytesPerPixel = m_alpha ? 4 : 3;

	for(const Colour & colour : m_colours) {
		if(!colour.putIn(byteBuffer, newOffset, alpha, byteOrder)) {
			return false;
		}

		newOffset += bytesPerPixel;
	}

	if(paddingColour.has_value()) {
		uint16_t paddingAmount = NUMBER_OF_COLOURS - m_colours.size();

		for(uint16_t i = 0; i < paddingAmount; i++) {
			if(!paddingColour->putIn(byteBuffer, newOffset, alpha, byteOrder)) {
				return false;
			}

			newOffset += bytesPerPixel;
		}
	}

	return true;
}

bool Palette::ColourTable::insertIn(ByteBuffer & byteBuffer, size_t offset, const std::optional<Colour> & paddingColour, Colour::ByteOrder byteOrder) const {
	return insertIn(byteBuffer, offset, m_alpha, paddingColour, byteOrder);
}

bool Palette::ColourTable::insertIn(ByteBuffer & byteBuffer, size_t offset, bool alpha, const std::optional<Colour> & paddingColour, Colour::ByteOrder byteOrder) const {
	size_t newOffset = offset;
	size_t bytesPerPixel = m_alpha ? 4 : 3;

	for(const Colour & colour : m_colours) {
		if(!colour.insertIn(byteBuffer, newOffset, alpha, byteOrder)) {
			return false;
		}

		newOffset += bytesPerPixel;
	}

	if(paddingColour.has_value()) {
		uint16_t paddingAmount = NUMBER_OF_COLOURS - m_colours.size();

		for(uint16_t i = 0; i < paddingAmount; i++) {
			if(!paddingColour->insertIn(byteBuffer, newOffset, alpha, byteOrder)) {
				return false;
			}

			newOffset += bytesPerPixel;
		}
	}

	return true;
}

bool Palette::ColourTable::writeTo(ByteBuffer & byteBuffer, const std::optional<Colour> & paddingColour, Colour::ByteOrder byteOrder) const {
	return writeTo(byteBuffer, m_alpha, paddingColour, byteOrder);
}

bool Palette::ColourTable::writeTo(ByteBuffer & byteBuffer, bool alpha, const std::optional<Colour> & paddingColour, Colour::ByteOrder byteOrder) const {
	for(const Colour & colour : m_colours) {
		if(!colour.writeTo(byteBuffer, alpha, byteOrder)) {
			return false;
		}
	}

	if(paddingColour.has_value()) {
		uint16_t paddingAmount = NUMBER_OF_COLOURS - m_colours.size();

		for(uint16_t i = 0; i < paddingAmount; i++) {
			if(!paddingColour->writeTo(byteBuffer, alpha, byteOrder)) {
				return false;
			}
		}
	}

	return true;
}

bool Palette::ColourTable::isParentValid() const {
	return m_parent != nullptr &&
		   m_parent->isValid();
}

Palette * Palette::ColourTable::getParent() const {
	return m_parent;
}

void Palette::ColourTable::setParent(Palette * palette) {
	m_parent = palette;
}

void Palette::ColourTable::clearParent() {
	m_parent = nullptr;
}

bool Palette::ColourTable::isValid() const {
	return m_colours.size() <= NUMBER_OF_COLOURS;
}

bool Palette::ColourTable::isValid(const ColourTable * colourTable) {
	return colourTable != nullptr &&
		   colourTable->isValid();
}

const Colour & Palette::ColourTable::operator[] (size_t colourIndex) const {
	if(colourIndex >= m_colours.size()) {
		return Colour::INVISIBLE;
	}

	return m_colours[colourIndex];
}

bool Palette::ColourTable::operator == (const ColourTable & colourTable) const {
	return m_colours == colourTable.m_colours;
}

bool Palette::ColourTable::operator != (const ColourTable & colourTable) const {
	return !operator == (colourTable);
}
