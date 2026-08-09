#include "ColourTable.h"

#include "../GameFile.h"

#include <ByteBuffer.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

const ColourTable ColourTable::EMPTY_COLOUR_TABLE;

ColourTable::ColourTable(uint16_t numberOfColours, std::optional<uint8_t> transparentColourIndex, bool alpha, std::string_view name, GameFile * parent)
	: m_transparentColourIndex(transparentColourIndex)
	, m_alpha(alpha)
	, m_name(name)
	, m_modified(false)
	, m_parent(parent) {
	m_colours.resize(numberOfColours, Colour::INVISIBLE);
}

ColourTable::ColourTable(const Colour & fillColour, uint16_t numberOfColours, std::optional<uint8_t> transparentColourIndex, bool alpha, std::string_view name, GameFile * parent)
	: m_transparentColourIndex(transparentColourIndex)
	, m_alpha(alpha)
	, m_name(name)
	, m_modified(false)
	, m_parent(parent) {
	m_colours.resize(numberOfColours, fillColour);
}

ColourTable::ColourTable(std::vector<Colour> && colours, std::optional<uint8_t> transparentColourIndex, bool alpha, std::string_view name, GameFile * parent)
	: m_colours(std::move(colours))
	, m_transparentColourIndex(transparentColourIndex)
	, m_alpha(alpha)
	, m_name(name)
	, m_modified(false)
	, m_parent(parent) { }

ColourTable::ColourTable(const std::vector<Colour> & colours, std::optional<uint8_t> transparentColourIndex, bool alpha, std::string_view name, GameFile * parent)
	: m_colours(colours)
	, m_transparentColourIndex(transparentColourIndex)
	, m_alpha(alpha)
	, m_name(name)
	, m_modified(false)
	, m_parent(parent) { }

ColourTable::ColourTable(ColourTable && colourTable) noexcept
	: m_colours(std::move(colourTable.m_colours))
	, m_transparentColourIndex(std::move(colourTable.m_transparentColourIndex))
	, m_alpha(colourTable.m_alpha)
	, m_name(std::move(colourTable.m_name))
	, m_modified(false)
	, m_parent(nullptr) { }

ColourTable::ColourTable(const ColourTable & colourTable)
	: m_colours(colourTable.m_colours)
	, m_transparentColourIndex(colourTable.m_transparentColourIndex)
	, m_alpha(colourTable.m_alpha)
	, m_name(colourTable.m_name)
	, m_modified(false)
	, m_parent(nullptr) { }

ColourTable & ColourTable::operator = (ColourTable && colourTable) noexcept {
	if(this != &colourTable) {
		m_colours = std::move(colourTable.m_colours);
		m_transparentColourIndex = std::move(colourTable.m_transparentColourIndex);
		m_alpha = colourTable.m_alpha;
		m_name = std::move(colourTable.m_name);

		setModified(true);
	}

	return *this;
}

ColourTable & ColourTable::operator = (const ColourTable & colourTable) {
	m_colours = std::move(colourTable.m_colours);
	m_transparentColourIndex = std::move(colourTable.m_transparentColourIndex);
	m_alpha = colourTable.m_alpha;
	m_name = std::move(colourTable.m_name);

	setModified(true);

	return *this;
}

ColourTable::~ColourTable() = default;

bool ColourTable::isModified() const {
	return m_modified;
}

void ColourTable::setModified(bool value) {
	m_modified = value;

	modified(*this);
}

bool ColourTable::hasTransparentColourIndex() const {
	return m_transparentColourIndex.has_value();
}

uint16_t ColourTable::numberOfColours() const {
	return static_cast<uint16_t>(m_colours.size());
}

const Colour & ColourTable::getColour(uint8_t colourIndex, bool * error) const {
	if(colourIndex >= m_colours.size()) {
		if(error != nullptr) {
			*error = true;
		}

		return Colour::INVISIBLE;
	}

	return m_colours[colourIndex];
}

const std::vector<Colour> & ColourTable::getColours() const {
	return m_colours;
}

bool ColourTable::setColour(uint8_t colourIndex, const Colour & colour) {
	if(colourIndex >= m_colours.size()) {
		return false;
	}
	else if(m_colours[colourIndex] == colour) {
		return true;
	}

	m_colours[colourIndex].setColour(colour);

	setModified(true);

	return true;
}

bool ColourTable::setColour(uint8_t colourIndex, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	if(colourIndex >= m_colours.size()) {
		return false;
	}
	else if(m_colours[colourIndex].r == r &&
			m_colours[colourIndex].g == g &&
			m_colours[colourIndex].b == b &&
			m_colours[colourIndex].a == a) {
		return true;
	}

	m_colours[colourIndex].setColour(r, g, b, a);

	setModified(true);

	return true;
}

bool ColourTable::setColours(const ColourTable & colourTable) {
	if(!colourTable.isValid()) {
		return false;
	}
	else if(m_colours == colourTable.m_colours &&
			m_transparentColourIndex == colourTable.m_transparentColourIndex) {
		return true;
	}

	m_colours = colourTable.m_colours;
	m_transparentColourIndex = colourTable.m_transparentColourIndex;

	setModified(true);

	return true;
}

void ColourTable::fillWithColour(const Colour & colour) {
	for(Colour & colour : m_colours) {
		colour.setColour(colour);
	}

	setModified(true);
}

bool ColourTable::setNumberOfColours(uint16_t colourCount, const Colour & fillColour) {
	if(colourCount > NUMBER_OF_COLOURS) {
		return false;
	}
	else if(m_colours.size() == colourCount) {
		return true;
	}

	m_colours.resize(colourCount, fillColour);

	if(m_transparentColourIndex.has_value() && m_transparentColourIndex.value() >= m_colours.size()) {
		m_transparentColourIndex.reset();
	}

	setModified(true);

	return true;
}

const std::optional<uint8_t> & ColourTable::getTransparentColourIndex() const {
	return m_transparentColourIndex;
}

void ColourTable::setTransparentColourIndex(uint8_t transparentColourIndex) {
	if(m_transparentColourIndex == transparentColourIndex) {
		return;
	}

	m_transparentColourIndex = transparentColourIndex;

	setModified(true);
}

void ColourTable::clearTransparentColourIndex() {
	if(!m_transparentColourIndex.has_value()) {
		return;
	}

	m_transparentColourIndex.reset();

	setModified(true);
}

bool ColourTable::hasAlphaChannel() const {
	return m_alpha;
}

void ColourTable::setHasAlphaChannel(bool alpha) {
	if(m_alpha == alpha) {
		return;
	}

	m_alpha = alpha;

	setModified(true);
}

bool ColourTable::hasName() const {
	return !m_name.empty();
}

const std::string & ColourTable::getName() const {
	return m_name;
}

void ColourTable::setName(std::string_view name) {
	if(Utilities::areStringsEqual(m_name, name)) {
		return;
	}

	m_name = name;

	setModified(true);
}

void ColourTable::clearName() {
	if(m_name.empty()) {
		return;
	}

	m_name.clear();

	setModified(true);
}

std::unique_ptr<ColourTable> ColourTable::getFrom(const ByteBuffer & byteBuffer, size_t offset, uint16_t numberOfColours, bool alpha, Colour::ByteOrder byteOrder) {
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

std::unique_ptr<ColourTable> ColourTable::readFrom(const ByteBuffer & byteBuffer, uint16_t numberOfColours, bool alpha, Colour::ByteOrder byteOrder) {
	std::unique_ptr<ColourTable> colourTable(getFrom(byteBuffer, byteBuffer.getReadOffset(), numberOfColours, alpha, byteOrder));

	if(colourTable != nullptr) {
		byteBuffer.skipReadBytes(numberOfColours * (alpha ? 4 : 3));
	}

	return colourTable;
}

bool ColourTable::putIn(ByteBuffer & byteBuffer, size_t offset, const std::optional<Colour> & paddingColour, Colour::ByteOrder byteOrder) const {
	return putIn(byteBuffer, offset, m_alpha, paddingColour, byteOrder);
}

bool ColourTable::putIn(ByteBuffer & byteBuffer, size_t offset, bool alpha, const std::optional<Colour> & paddingColour, Colour::ByteOrder byteOrder) const {
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

bool ColourTable::insertIn(ByteBuffer & byteBuffer, size_t offset, const std::optional<Colour> & paddingColour, Colour::ByteOrder byteOrder) const {
	return insertIn(byteBuffer, offset, m_alpha, paddingColour, byteOrder);
}

bool ColourTable::insertIn(ByteBuffer & byteBuffer, size_t offset, bool alpha, const std::optional<Colour> & paddingColour, Colour::ByteOrder byteOrder) const {
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

bool ColourTable::writeTo(ByteBuffer & byteBuffer, const std::optional<Colour> & paddingColour, Colour::ByteOrder byteOrder) const {
	return writeTo(byteBuffer, m_alpha, paddingColour, byteOrder);
}

bool ColourTable::writeTo(ByteBuffer & byteBuffer, bool alpha, const std::optional<Colour> & paddingColour, Colour::ByteOrder byteOrder) const {
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

bool ColourTable::isParentValid() const {
	return m_parent != nullptr &&
		   m_parent->isValid(false);
}

GameFile * ColourTable::getParent() const {
	return m_parent;
}

void ColourTable::setParent(GameFile * parent) {
	m_parent = parent;
}

void ColourTable::clearParent() {
	m_parent = nullptr;
}

bool ColourTable::isValid() const {
	return m_colours.size() <= NUMBER_OF_COLOURS;
}

bool ColourTable::isValid(const ColourTable * colourTable) {
	return colourTable != nullptr &&
		   colourTable->isValid();
}

const Colour & ColourTable::operator[] (size_t colourIndex) const {
	if(colourIndex >= m_colours.size()) {
		return Colour::INVISIBLE;
	}

	return m_colours[colourIndex];
}

bool ColourTable::operator == (const ColourTable & colourTable) const {
	if(this == &colourTable) {
		return true;
	}

	return m_transparentColourIndex == colourTable.m_transparentColourIndex &&
		   m_alpha == colourTable.m_alpha &&
		   m_colours == colourTable.m_colours;
}

bool ColourTable::operator != (const ColourTable & colourTable) const {
	return !operator == (colourTable);
}
