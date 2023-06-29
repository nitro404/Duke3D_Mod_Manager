#include "PaletteGPL.h"

#include <ByteBuffer.h>
#include <Utilities/NumberUtilities.h>
#include <Utilities/StringUtilities.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <sstream>

PaletteGPL::PaletteGPL(const std::string & filePath)
	: Palette(filePath)
	, m_colourTable(std::make_shared<ColourTable>())
	, m_colourNames(m_colourTable->numberOfColours()) {
	updateParent();
}

PaletteGPL::PaletteGPL(std::unique_ptr<ColourTable> colourTable, std::vector<std::string> && colourNames, std::optional<uint8_t> columnCount, std::vector<std::string> && comments, const std::string & filePath)
	: Palette(filePath)
	, m_colourTable(colourTable != nullptr ? std::shared_ptr<ColourTable>(colourTable.release()) : std::make_shared<ColourTable>())
	, m_colourNames(std::move(colourNames))
	, m_numberOfColumns(columnCount)
	, m_comments(std::move(comments)) {
	m_colourNames.resize(m_colourTable->numberOfColours());
	updateParent();
}

PaletteGPL::PaletteGPL(const ColourTable & colourTable, const std::vector<std::string> & colourNames, std::optional<uint8_t> columnCount, const std::vector<std::string> & comments, const std::string & filePath)
	: Palette(filePath)
	, m_colourTable(std::make_shared<ColourTable>(colourTable))
	, m_colourNames(colourNames)
	, m_numberOfColumns(columnCount)
	, m_comments(comments) {
	m_colourNames.resize(m_colourTable->numberOfColours());
	updateParent();
}

PaletteGPL::PaletteGPL(PaletteGPL && palette) noexcept
	: Palette(std::move(palette))
	, m_colourTable(std::move(palette.m_colourTable))
	, m_colourNames(std::move(palette.m_colourNames))
	, m_numberOfColumns(palette.m_numberOfColumns)
	, m_comments(std::move(palette.m_comments)) {
	updateParent();
}

PaletteGPL::PaletteGPL(const PaletteGPL & palette)
	: Palette(palette)
	, m_colourTable(palette.m_colourTable)
	, m_colourNames(palette.m_colourNames)
	, m_numberOfColumns(palette.m_numberOfColumns)
	, m_comments(palette.m_comments) {
	updateParent();
}

PaletteGPL & PaletteGPL::operator = (PaletteGPL && palette) noexcept {
	if(this != &palette) {
		Palette::operator = (std::move(palette));

		m_colourTable = std::move(palette.m_colourTable);
		m_colourNames = std::move(palette.m_colourNames);
		m_numberOfColumns = palette.m_numberOfColumns;
		m_comments = std::move(palette.m_comments);

		updateParent();
	}

	return *this;
}

PaletteGPL & PaletteGPL::operator = (const PaletteGPL & palette) {
	Palette::operator = (palette);

	m_colourTable = palette.m_colourTable;
	m_colourNames = palette.m_colourNames;
	m_numberOfColumns = palette.m_numberOfColumns;
	m_comments = palette.m_comments;

	updateParent();

	return *this;
}

PaletteGPL::~PaletteGPL() { }

std::shared_ptr<ColourTable> PaletteGPL::getColourTable(uint8_t colourTableIndex) const {
	if(colourTableIndex != 0) {
		return nullptr;
	}

	return m_colourTable;
}

size_t PaletteGPL::numberOfNamedColours() const {
	size_t namedColourCount = 0;

	for(const std::string & colourName : m_colourNames) {
		if(!colourName.empty()) {
			namedColourCount++;
		}
	}

	return namedColourCount;
}

bool PaletteGPL::hasColourName(const std::string & colourName) const {
	return indexOfColourName(colourName) != std::numeric_limits<size_t>::max();
}

size_t PaletteGPL::indexOfColourName(const std::string & colourName) const {
	std::vector<std::string>::const_iterator colourNameIterator(std::find(m_colourNames.cbegin(), m_colourNames.cend(), colourName));

	if(colourNameIterator == m_colourNames.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return colourNameIterator - m_colourNames.cbegin();
}

const std::string & PaletteGPL::getColourName(size_t index) const {
	if(index >= m_colourNames.size()) {
		return Utilities::emptyString;
	}

	return m_colourNames[index];
}

const std::vector<std::string> & PaletteGPL::getColourNames() const {
	return m_colourNames;
}

bool PaletteGPL::setColourName(size_t index, const std::string & colourName) {
	if(index >= m_colourNames.size()) {
		return false;
	}

	m_colourNames[index] = Utilities::trimString(colourName);

	return true;
}

bool PaletteGPL::clearColourName(size_t index) {
	if(index >= m_colourNames.size()) {
		return false;
	}

	m_colourNames[index] = "";

	return true;
}

void PaletteGPL::clearColourNames() {
	m_colourNames.clear();
}

bool PaletteGPL::hasNumberOfColumns() const {
	return m_numberOfColumns.has_value();
}

const std::optional<uint8_t> PaletteGPL::numberOfColumns() const {
	return m_numberOfColumns;
}

void PaletteGPL::setNumberOfColumns(uint8_t columnCount) {
	m_numberOfColumns = columnCount;
}

void PaletteGPL::clearNumberOfColumns() {
	m_numberOfColumns.reset();
}

bool PaletteGPL::hasComments() const {
	return !m_comments.empty();
}

size_t PaletteGPL::numberOfComments() const {
	return m_comments.size();
}

bool PaletteGPL::hasComment(const std::string & comment) const {
	return indexOfComment(comment) != std::numeric_limits<size_t>::max();
}

size_t PaletteGPL::indexOfComment(const std::string & comment) const {
	std::vector<std::string>::const_iterator commentIterator(std::find(m_comments.cbegin(), m_comments.cend(), comment));

	if(commentIterator == m_comments.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return commentIterator - m_comments.cbegin();
}

const std::string & PaletteGPL::getComment(size_t index) const {
	if(index >= m_comments.size()) {
		return Utilities::emptyString;
	}

	return m_comments[index];
}

const std::vector<std::string> & PaletteGPL::getComments() const {
	return m_comments;
}

void PaletteGPL::addComment(const std::string & comment) {
	m_comments.push_back(Utilities::trimString(comment));
}

void PaletteGPL::addComments(const std::vector<std::string> & comments) {
	for(const std::string & comment : comments) {
		addComment(comment);
	}
}

void PaletteGPL::setComments(const std::vector<std::string> & comments) {
	m_comments = comments;
}

void PaletteGPL::setComments(std::vector<std::string> && comments) {
	m_comments = std::move(comments);
}

bool PaletteGPL::insertComment(size_t index, const std::string & comment) {
	if(index > m_comments.size()) {
		return false;
	}
	else if(index == m_comments.size()) {
		m_comments.push_back(comment);
	}
	else {
		m_comments.insert(m_comments.begin() + index, comment);
	}

	return true;
}

bool PaletteGPL::replaceComment(size_t index, const std::string & newComment) {
	if(index >= m_comments.size()) {
		return false;
	}

	m_comments[index] = newComment;

	return true;
}

bool PaletteGPL::replaceComment(const std::string & comment, const std::string & newComment) {
	return replaceComment(indexOfComment(comment), newComment);
}

bool PaletteGPL::removeComment(size_t index) {
	if(index >= m_comments.size()) {
		return false;
	}

	m_comments.erase(m_comments.begin() + index);

	return true;
}

bool PaletteGPL::removeComment(const std::string & comment) {
	return removeComment(indexOfComment(comment));
}

void PaletteGPL::clearComments() {
	m_comments.clear();
}

std::unique_ptr<PaletteGPL> PaletteGPL::readFrom(const ByteBuffer & byteBuffer) {
	bool error = false;

	std::string magic(byteBuffer.readString(MAGIC_SIZE_BYTES, &error));

	if(error) {
		spdlog::error("GIMP GPL palette is missing '{}' magic identifier.", MAGIC);
		return nullptr;
	}

	if(!Utilities::areStringsEqual(magic, MAGIC)) {
		spdlog::error("GIMP GPL palette has invalid magic identifier '{}', expected '{}'.", magic, MAGIC);
		return nullptr;
	}

	if(!byteBuffer.skipToNextLine()) {
		spdlog::error("Truncated GIMP GPL palette data, missing name.");
		return nullptr;
	}

	std::string nameData(byteBuffer.readLine(&error));

	if(error) {
		spdlog::error("Failed to read GIMP GPL palette name.");
		return nullptr;
	}

	size_t nameDataSeparatorIndex = nameData.find_first_of(":");

	if(nameDataSeparatorIndex == std::string::npos) {
		spdlog::error("Failed to locate GIMP GPL palette name separator.");
		return nullptr;
	}

	size_t nameEndIndex = nameDataSeparatorIndex - 1;

	std::string_view nameKey(nameData.data(), nameEndIndex + 1);

	if(!Utilities::areStringsEqual(nameKey, NAME_KEY)) {
		spdlog::error("Invalid GIMP GPL palette name key: '{}', expected '{}'.", nameKey, NAME_KEY);
		return nullptr;
	}

	std::string name(Utilities::trimString(std::string_view(nameData.data() + nameDataSeparatorIndex + 1, nameData.length() - nameDataSeparatorIndex - 1)));

	std::optional<uint8_t> optionalNumberOfColoumns;

	if(byteBuffer.getByte(byteBuffer.getReadOffset()) != COMMENT_CHARACTER) {
		std::string columnsCountData(byteBuffer.readLine(&error));

		if(error) {
			spdlog::error("Failed to read GIMP GPL palette number of columns value.");
			return nullptr;
		}

		size_t columnsCountSeparatorIndex = columnsCountData.find_first_of(":");

		if(columnsCountSeparatorIndex == std::string::npos) {
			spdlog::error("Failed to locate GIMP GPL palette number of columns value separator.");
			return nullptr;
		}

		size_t columnsCountEndIndex = columnsCountSeparatorIndex - 1;

		std::string_view numberOfColumnsKey(columnsCountData.data(), columnsCountEndIndex + 1);

		if(!Utilities::areStringsEqual(numberOfColumnsKey, NUMBER_OF_COLUMNS_KEY)) {
			spdlog::error("Invalid GIMP GPL palette number of columns key: '{}', expected '{}'.", numberOfColumnsKey, NUMBER_OF_COLUMNS_KEY);
			return nullptr;
		}

		std::string columnsCountValue(Utilities::trimString(std::string_view(columnsCountData.data() + columnsCountSeparatorIndex + 1, columnsCountData.length() - columnsCountSeparatorIndex - 1)));

		optionalNumberOfColoumns = Utilities::parseUnsignedByte(columnsCountValue, &error);

		if(error) {
			spdlog::error("Failed to read GIMP GPL palette number of columns number.");
			return nullptr;
		}
	}

	std::string commentData;
	std::string comment;
	std::vector<std::string> comments;

	while(byteBuffer.canReadBytes(1) && byteBuffer.getByte(byteBuffer.getReadOffset()) == COMMENT_CHARACTER) {
		commentData = byteBuffer.readLine(&error);

		if(error) {
			spdlog::error("Failed to read GIMP GPL palette comment line #{}.", comments.size() + 1);
			return nullptr;
		}

		comment = Utilities::trimString(std::string_view(commentData.data() + 1, commentData.length() - 1));

		if(comment.empty()) {
			continue;
		}

		comments.emplace_back(std::move(comment));
	}

	std::string colourData;
	size_t colourChannelStartOffset = 0;
	size_t colourChannelEndOffset = std::numeric_limits<size_t>::max();
	Colour colour;
	std::vector<Colour> colours;
	colours.reserve(ColourTable::NUMBER_OF_COLOURS);
	std::vector<std::string> colourNames;
	colourNames.reserve(ColourTable::NUMBER_OF_COLOURS);

	while(true) {
		if(!byteBuffer.hasMoreLines()) {
			break;
		}

		if(colours.size() >= ColourTable::NUMBER_OF_COLOURS) {
			spdlog::error("GIMP GPL palettes with more than {} colours are not supported, additional colours will be discarded.", ColourTable::NUMBER_OF_COLOURS);
			break;
		}

		colourData = Utilities::trimString(byteBuffer.readLine(&error));

		if(error) {
			spdlog::error("Failed to read GIMP GPL palette colour #{} data.", colours.size() + 1);
			return nullptr;
		}

		colourChannelStartOffset = colourData.find_first_not_of(" \t");

		if(colourChannelStartOffset == std::string::npos) {
			spdlog::error("GIMP GPL palette colour #{} data is malformed, failed to locate start of red colour channel from: '{}'.", colours.size() + 1, colourData);
			return nullptr;
		}

		colourChannelEndOffset = colourData.find_first_of(" \t", colourChannelStartOffset);

		if(colourChannelEndOffset == std::string::npos) {
			spdlog::error("GIMP GPL palette colour #{} data is malformed, failed to read red colour channel from: '{}'.", colours.size() + 1, colourData);
			return nullptr;
		}

		colourChannelEndOffset--;

		colour.r = Utilities::parseUnsignedByte(std::string(colourData.data() + colourChannelStartOffset, colourChannelEndOffset - colourChannelStartOffset + 1), &error);

		if(error) {
			spdlog::error("Invalid GIMP GPL palette colour #{} red channel value in colour data: '{}'.", colours.size() + 1, colourData);
			return nullptr;
		}

		colourChannelStartOffset = colourData.find_first_not_of(" \t", colourChannelEndOffset + 1);

		if(colourChannelStartOffset == std::string::npos) {
			spdlog::error("GIMP GPL palette colour #{} data is malformed, failed to locate start of green colour channel from: '{}'.", colours.size() + 1, colourData);
			return nullptr;
		}

		colourChannelEndOffset = colourData.find_first_of(" \t", colourChannelStartOffset);

		if(colourChannelEndOffset == std::string::npos) {
			spdlog::error("GIMP GPL palette colour #{} data is malformed, failed to locate end of green colour channel from: '{}'.", colours.size() + 1, colourData);
			return nullptr;
		}

		colourChannelEndOffset--;

		colour.g = Utilities::parseUnsignedByte(std::string(colourData.data() + colourChannelStartOffset, colourChannelEndOffset - colourChannelStartOffset + 1), &error);

		if(error) {
			spdlog::error("Invalid GIMP GPL palette colour #{} green channel value in colour data: '{}'.", colours.size() + 1, colourData);
			return nullptr;
		}

		colourChannelStartOffset = colourData.find_first_not_of(" \t", colourChannelEndOffset + 1);

		if(colourChannelStartOffset == std::string::npos) {
			spdlog::error("GIMP GPL palette colour #{} data is malformed, failed to locate start of blue colour channel from: '{}'.", colours.size() + 1, colourData);
			return nullptr;
		}

		colourChannelEndOffset = colourData.find_first_of(" \t", colourChannelStartOffset);

		bool hasName = false;

		if(colourChannelEndOffset == std::string::npos) {
			colourChannelEndOffset = colourData.length() - 1;
		}
		else {
			colourChannelEndOffset--;
			hasName = true;
		}

		colour.b = Utilities::parseUnsignedByte(std::string(colourData.data() + colourChannelStartOffset, colourChannelEndOffset - colourChannelStartOffset + 1), &error);

		if(error) {
			spdlog::error("Invalid GIMP GPL palette colour #{} blue channel value in colour data: '{}'.", colours.size() + 1, colourData);
			return nullptr;
		}

		colours.emplace_back(colour);

		if(hasName) {
			colourNames.emplace_back(Utilities::trimString(std::string_view(colourData.data() + colourChannelEndOffset + 1, colourData.length() - colourChannelEndOffset - 1)));
		}
		else {
			colourNames.push_back(Utilities::emptyString);
		}
	}

	std::unique_ptr<ColourTable> colourTable(std::make_unique<ColourTable>(std::move(colours)));
	colourTable->setName(name);

	return std::make_unique<PaletteGPL>(std::move(colourTable), std::move(colourNames), optionalNumberOfColoumns, std::move(comments));
}

std::unique_ptr<PaletteGPL> PaletteGPL::loadFrom(const std::string & filePath) {
	std::unique_ptr<ByteBuffer> paletteData(ByteBuffer::readFrom(filePath));

	if(paletteData == nullptr) {
		spdlog::error("Failed to read GIMP GPL palette binary data from file: '{}'.", filePath);
		return nullptr;
	}

	std::unique_ptr<PaletteGPL> palette(PaletteGPL::readFrom(*paletteData));

	if(palette == nullptr) {
		spdlog::error("Failed to parse GIMP GPL palette binary data from file contents: '{}'.", filePath);
		return nullptr;
	}

	return palette;
}

bool PaletteGPL::writeTo(ByteBuffer & byteBuffer) const {
	byteBuffer.writeLine(MAGIC);

	byteBuffer.writeLine(NAME_KEY + ": " + m_colourTable->getName());

	if(m_numberOfColumns.has_value()) {
		byteBuffer.writeLine(NUMBER_OF_COLUMNS_KEY + ": " + std::to_string(m_numberOfColumns.value()));
	}

	if(!m_comments.empty()) {
		byteBuffer.writeLine(std::string(1, COMMENT_CHARACTER));

		for(const std::string & comment : m_comments) {
			byteBuffer.writeLine(fmt::format("{}{}{}", COMMENT_CHARACTER, std::string(COMMENT_PADDING, ' '), comment));
		}

		byteBuffer.writeLine(std::string(1, COMMENT_CHARACTER));
	}
	else {
		byteBuffer.writeLine(std::string(1, COMMENT_CHARACTER));
	}

	for(size_t i = 0; i < m_colourTable->numberOfColours(); i++) {
		const Colour & colour = m_colourTable->getColour(i);
		std::stringstream colourStream;

		for(uint8_t j = 0; j < 3; j++) {
			if(j != 0) {
				colourStream << ' ';
			}

			colourStream << std::string(3 - Utilities::unsignedByteLength(colour.c[j]), ' ') << std::to_string(colour.c[j]);
		}

		if(!m_colourNames[i].empty()) {
			colourStream << '\t' << m_colourNames[i];
		}

		byteBuffer.writeLine(colourStream.str());
	}

	return true;
}

void PaletteGPL::addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const {
	Palette::addMetadata(metadata);

	metadata.push_back({ "Number of Columns", m_numberOfColumns.has_value() ? std::to_string(m_numberOfColumns.value()) : "N/A" });
	metadata.push_back({ "Number of Named Colours", std::to_string(numberOfNamedColours()) });
	metadata.push_back({ "Number of Comments", std::to_string(m_comments.size()) });

	for(size_t i = 0; i < m_comments.size(); i++) {
		metadata.push_back({ fmt::format("Comment #{}", i + 1), m_comments[i] });
	}
}

Endianness PaletteGPL::getEndianness() const {
	return {};
}

size_t PaletteGPL::getSizeInBytes() const {
	size_t sizeBytes = MAGIC.length() + 1 + NAME_KEY.length() + 2 + m_colourTable->getName().length() + 1;

	if(m_numberOfColumns.has_value()) {
		sizeBytes += NUMBER_OF_COLUMNS_KEY.length() + 2 + Utilities::unsignedByteLength(m_numberOfColumns.value()) + 1;
	}

	if(m_comments.empty()) {
		sizeBytes += 2;
	}
	else {
		sizeBytes += 2;

		for(const std::string & comment : m_comments) {
			sizeBytes += 1 + COMMENT_PADDING + comment.length() + 1;
		}

		sizeBytes += 2;
	}

	for(size_t i = 0; i < m_colourTable->numberOfColours(); i++) {
		const Colour & colour = m_colourTable->getColour(i);

		sizeBytes += 12;

		if(!m_colourNames[i].empty()) {
			sizeBytes += 1 + m_colourNames[i].length();
		}
	}

	return sizeBytes;
}

void PaletteGPL::updateParent() {
	m_colourTable->setParent(this);
}

bool PaletteGPL::operator == (const PaletteGPL & palette) const {
	return *m_colourTable == *palette.m_colourTable &&
		   m_colourNames == palette.m_colourNames;
}

bool PaletteGPL::operator != (const PaletteGPL & palette) const {
	return !operator == (palette);
}
