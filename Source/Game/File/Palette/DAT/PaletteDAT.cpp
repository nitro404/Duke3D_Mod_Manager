#include "PaletteDAT.h"

#include <ByteBuffer.h>

#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <array>

static const std::array<std::pair<std::string /* description */, bool /* hasTransparentColour */>, 5> DEFAULT_ALTERNATE_COLOUR_TABLE_INFO({
	std::make_pair("Underwater", true),
	std::make_pair("Night Vision", true),
	std::make_pair("Title Screen", false),
	std::make_pair("3D Realms Logo", false),
	std::make_pair("Episode 1 Ending Animation", false)
});

std::optional<PaletteDAT::Type> PaletteDAT::determineType(const ByteBuffer & byteBuffer) {
	// http://advsys.net/ken/palette.txt
	//
	// The original vanilla PALETTE.DAT file has an additional 2 rows of translucency table data
	// that was not truncated off and serves no purpose. Custom palette files may not contain
	// this data, so we need to handle this edge case accordingly when determining the data type.
	static constexpr size_t NUMBER_OF_LEGACY_TRAILING_PALETTE_BYTES = 8192; // (32 * 256)

	byteBuffer.setEndianness(ENDIANNESS);

	// Test for PALETTE.DAT file structure
	if(byteBuffer.numberOfBytesRemaining() >= (ColourTable::NUMBER_OF_COLOURS * BYTES_PER_COLOUR) + (sizeof(uint16_t))) {
		std::optional<uint16_t> optionalNumberOfShadeTables(byteBuffer.getUnsignedShort(byteBuffer.getReadOffset() + (ColourTable::NUMBER_OF_COLOURS * BYTES_PER_COLOUR)));

		size_t expectedNumberOfBytes = (ColourTable::NUMBER_OF_COLOURS * BYTES_PER_COLOUR) + sizeof(uint16_t) + (static_cast<size_t>(optionalNumberOfShadeTables.value()) * ColourTable::NUMBER_OF_COLOURS) + (ColourTable::NUMBER_OF_COLOURS * ColourTable::NUMBER_OF_COLOURS);

		if(optionalNumberOfShadeTables.has_value() && (byteBuffer.numberOfBytesRemaining() == expectedNumberOfBytes || byteBuffer.numberOfBytesRemaining() == expectedNumberOfBytes + NUMBER_OF_LEGACY_TRAILING_PALETTE_BYTES)) {
			return Type::Palette;
		}
	}

	// Test for LOOKUP.DAT file structure
	std::optional<uint8_t> optionalNumberOfSwapTables(byteBuffer.getUnsignedByte(byteBuffer.getReadOffset()));

	if(optionalNumberOfSwapTables.has_value()) {
		size_t expectedNumberOfBytes = (sizeof(uint8_t) + static_cast<size_t>(optionalNumberOfSwapTables.value()) * (sizeof(uint8_t) + (ColourTable::NUMBER_OF_COLOURS * sizeof(uint8_t))));

		// Since there is no indicator of how many global palettes are at the end of the file,
		// we must calculate how many bytes are left over and see if this is divisible by the
		// number of colours (256) multiplied by the number of bytes per colour (3).
		size_t leftoverBytes = byteBuffer.numberOfBytesRemaining() - expectedNumberOfBytes;

		if(leftoverBytes % (ColourTable::NUMBER_OF_COLOURS * BYTES_PER_COLOUR) == 0) {
			return Type::Lookup;
		}
	}

	return {};
}

PaletteDAT::PaletteDAT(Type type, const std::string & filePath)
	: Palette(filePath)
	, m_type(type)
	, m_trailingData(std::make_unique<ByteBuffer>()) {
	switch(m_type) {
		case Type::Palette: {
			m_translucencyTable = std::make_shared<TranslucencyTable>();

			break;
		}

		case Type::Lookup: {
			break;
		}
	}

	updateParent();
}

PaletteDAT::PaletteDAT(std::unique_ptr<ColourTable> colourTable, std::vector<std::unique_ptr<ShadeTable>> shadeTables, std::unique_ptr<TranslucencyTable> translucencyTable, std::unique_ptr<ByteBuffer> trailingData, const std::string & filePath)
	: Palette(filePath)
	, m_type(Type::Palette)
	, m_colourTables({ colourTable != nullptr ? std::shared_ptr<ColourTable>(colourTable.release()) : std::make_shared<ColourTable>() })
	, m_translucencyTable(std::move(translucencyTable))
	, m_trailingData(trailingData != nullptr ? std::move(trailingData) : std::make_unique<ByteBuffer>()) {
	m_shadeTables.reserve(shadeTables.size());

	for(std::unique_ptr<ShadeTable> & shadeTable : shadeTables) {
		m_shadeTables.emplace_back(std::shared_ptr<ShadeTable>(shadeTable.release()));
	}
}

PaletteDAT::PaletteDAT(std::vector<std::unique_ptr<ColourTable>> colourTables, std::vector<std::unique_ptr<SwapTable>> swapTables, std::unique_ptr<ByteBuffer> trailingData, const std::string & filePath)
	: Palette(filePath)
	, m_type(Type::Lookup)
	, m_trailingData(trailingData != nullptr ? std::move(trailingData) : std::make_unique<ByteBuffer>()) {
	m_colourTables.reserve(colourTables.size());
	m_swapTables.reserve(swapTables.size());

	for(std::unique_ptr<ColourTable> & colourTable : colourTables) {
		m_colourTables.emplace_back(std::shared_ptr<ColourTable>(colourTable.release()));
	}

	for(std::unique_ptr<SwapTable> & swapTable : swapTables) {
		m_swapTables.emplace_back(std::shared_ptr<SwapTable>(swapTable.release()));
	}
}

PaletteDAT::PaletteDAT(PaletteDAT && palette) noexcept
	: Palette(palette)
	, m_type(palette.m_type)
	, m_trailingData(std::move(palette.m_trailingData)) {
	updateParent();
}

PaletteDAT::PaletteDAT(const PaletteDAT & palette)
	: Palette(palette)
	, m_type(palette.m_type)
	, m_trailingData(std::make_unique<ByteBuffer>(*palette.m_trailingData)) {
	updateParent();
}

PaletteDAT & PaletteDAT::operator = (PaletteDAT && palette) noexcept {
	if(this != &palette) {
		Palette::operator = (palette);

		m_type = palette.m_type;
		m_trailingData = std::move(palette.m_trailingData);

		updateParent();
	}

	return *this;
}

PaletteDAT & PaletteDAT::operator = (const PaletteDAT & palette) {
	Palette::operator = (palette);

	m_type = palette.m_type;
	m_trailingData = std::make_unique<ByteBuffer>(*palette.m_trailingData);

	updateParent();

	return *this;
}

PaletteDAT::~PaletteDAT() { }

PaletteDAT::Type PaletteDAT::getType() const {
	return m_type;
}

bool PaletteDAT::hasTrailingData() const {
	return !m_trailingData->isEmpty();
}

const ByteBuffer & PaletteDAT::getTrailingData() const {
	return *m_trailingData;
}

void PaletteDAT::clearTrailingData() {
	m_trailingData->clear();
}

bool PaletteDAT::hasTranslucencyTable() const {
	return m_translucencyTable != nullptr;
}

std::shared_ptr<PaletteDAT::TranslucencyTable> PaletteDAT::getTranslucencyTable() const {
	return m_translucencyTable;
}

size_t PaletteDAT::numberOfShadeTables() const {
	return m_shadeTables.size();
}

bool PaletteDAT::hasShadeTable(const ShadeTable & shadeTable) const {
	return indexOfShadeTable(shadeTable) != std::numeric_limits<size_t>::max();
}

size_t PaletteDAT::indexOfShadeTable(const ShadeTable & shadeTable) const {
	std::vector<std::shared_ptr<ShadeTable>>::const_iterator shadeTableIterator(std::find_if(m_shadeTables.cbegin(), m_shadeTables.cend(), [&shadeTable](const std::shared_ptr<ShadeTable> & currentShadeTable) {
		return currentShadeTable.get() == &shadeTable;
	}));

	if(shadeTableIterator == m_shadeTables.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return shadeTableIterator - m_shadeTables.cbegin();
}

std::shared_ptr<PaletteDAT::ShadeTable> PaletteDAT::getShadeTable(size_t index) const {
	if(index >= m_shadeTables.size()) {
		return nullptr;
	}

	return m_shadeTables[index];
}

const std::vector<std::shared_ptr<PaletteDAT::ShadeTable>> & PaletteDAT::getShadeTables() const {
	return m_shadeTables;
}

bool PaletteDAT::addShadeTable(std::unique_ptr<ShadeTable> shadeTable) {
	if(shadeTable == nullptr) {
		return false;
	}

	m_shadeTables.emplace_back(std::shared_ptr<ShadeTable>(shadeTable.release()));

	return true;
}

void PaletteDAT::addShadeTable(const ShadeTable & shadeTable) {
	m_shadeTables.push_back(std::make_shared<ShadeTable>(shadeTable));
}

bool PaletteDAT::insertShadeRable(size_t index, std::unique_ptr<ShadeTable> shadeTable) {
	if(index > m_shadeTables.size()) {
		return false;
	}
	else if(index == m_shadeTables.size()) {
		m_shadeTables.emplace_back(std::shared_ptr<ShadeTable>(shadeTable.release()));
	}
	else {
		m_shadeTables.emplace(m_shadeTables.begin() + index, std::shared_ptr<ShadeTable>(shadeTable.release()));
	}

	return true;
}

bool PaletteDAT::insertShadeRable(size_t index, const ShadeTable & shadeTable) {
	if(index > m_shadeTables.size()) {
		return false;
	}
	else if(index == m_shadeTables.size()) {
		m_shadeTables.emplace_back(std::make_shared<ShadeTable>(shadeTable));
	}
	else {
		m_shadeTables.emplace(m_shadeTables.begin() + index, std::make_shared<ShadeTable>(shadeTable));
	}

	return true;
}

bool PaletteDAT::replaceShadeRable(size_t index, std::unique_ptr<ShadeTable> shadeTable) {
	if(index >= m_shadeTables.size()) {
		return false;
	}

	m_shadeTables[index] = std::shared_ptr<ShadeTable>(shadeTable.release());

	return true;
}

bool PaletteDAT::replaceShadeRable(size_t index, const ShadeTable & shadeTable) {
	if(index >= m_shadeTables.size()) {
		return false;
	}

	m_shadeTables[index] = std::make_shared<ShadeTable>(shadeTable);

	return true;
}

bool PaletteDAT::removeShadeTable(size_t index) {
	if(index >= m_shadeTables.size()) {
		return false;
	}

	m_shadeTables.erase(m_shadeTables.begin() + index);

	return true;
}

bool PaletteDAT::removeShadeTable(const ShadeTable & shadeTable) {
	return removeShadeTable(indexOfShadeTable(shadeTable));
}

void PaletteDAT::clearShadeTables() {
	m_shadeTables.clear();
}

size_t PaletteDAT::numberOfSwapTables() const {
	return m_swapTables.size();
}

bool PaletteDAT::hasSwapTable(const SwapTable & swapTable) const {
	return indexOfSwapTable(swapTable) != std::numeric_limits<size_t>::max();
}

bool PaletteDAT::hasSwapTableWithSwapIndex(uint8_t swapIndex) const {
	return indexOfSwapTableWithSwapIndex(swapIndex) != std::numeric_limits<size_t>::max();
}

size_t PaletteDAT::indexOfSwapTable(const SwapTable & swapTable) const {
	std::vector<std::shared_ptr<SwapTable>>::const_iterator swapTableIterator(std::find_if(m_swapTables.cbegin(), m_swapTables.cend(), [&swapTable](const std::shared_ptr<SwapTable> & currentSwapTable) {
		return currentSwapTable.get() == &swapTable;
	}));

	if(swapTableIterator == m_swapTables.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return swapTableIterator - m_swapTables.cbegin();
}

size_t PaletteDAT::indexOfSwapTableWithSwapIndex(uint8_t swapIndex) const {
	std::vector<std::shared_ptr<SwapTable>>::const_iterator swapTableIterator(std::find_if(m_swapTables.cbegin(), m_swapTables.cend(), [swapIndex](const std::shared_ptr<SwapTable> & currentSwapTable) {
		return currentSwapTable->getSwapIndex() == swapIndex;
	}));

	if(swapTableIterator == m_swapTables.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return swapTableIterator - m_swapTables.cbegin();
}

std::shared_ptr<PaletteDAT::SwapTable> PaletteDAT::getSwapTable(size_t index) const {
	if(index >= m_swapTables.size()) {
		return nullptr;
	}

	return m_swapTables[index];
}

std::shared_ptr<PaletteDAT::SwapTable> PaletteDAT::getSwapTableWithSwapIndex(uint8_t swapIndex) const {
	size_t swapTableIndex = indexOfSwapTableWithSwapIndex(swapIndex);

	if(swapTableIndex == std::numeric_limits<size_t>::max()) {
		return nullptr;
	}

	return m_swapTables[swapTableIndex];
}

const std::vector<std::shared_ptr<PaletteDAT::SwapTable>> & PaletteDAT::getSwapTables() const {
	return m_swapTables;
}

bool PaletteDAT::addSwapTable(std::unique_ptr<SwapTable> swapTable) {
	if(swapTable == nullptr || hasSwapTableWithSwapIndex(swapTable->getSwapIndex())) {
		return false;
	}

	m_swapTables.emplace_back(std::shared_ptr<SwapTable>(swapTable.release()));

	return true;
}

bool PaletteDAT::addSwapTable(const SwapTable & swapTable) {
	if(hasSwapTableWithSwapIndex(swapTable.getSwapIndex())) {
		return false;
	}

	m_swapTables.push_back(std::make_shared<SwapTable>(swapTable));

	return true;
}

bool PaletteDAT::insertSwapRable(size_t index, std::unique_ptr<SwapTable> swapTable) {
	if(index > m_swapTables.size() || hasSwapTableWithSwapIndex(swapTable->getSwapIndex())) {
		return false;
	}
	else if(index == m_swapTables.size()) {
		m_swapTables.emplace_back(std::shared_ptr<SwapTable>(swapTable.release()));
	}
	else {
		m_swapTables.emplace(m_swapTables.begin() + index, std::shared_ptr<SwapTable>(swapTable.release()));
	}

	return true;
}

bool PaletteDAT::insertSwapRable(size_t index, const SwapTable & swapTable) {
	if(index > m_swapTables.size() || hasSwapTableWithSwapIndex(swapTable.getSwapIndex())) {
		return false;
	}
	else if(index == m_swapTables.size()) {
		m_swapTables.emplace_back(std::make_shared<SwapTable>(swapTable));
	}
	else {
		m_swapTables.emplace(m_swapTables.begin() + index, std::make_shared<SwapTable>(swapTable));
	}

	return true;
}

bool PaletteDAT::replaceSwapRable(size_t index, std::unique_ptr<SwapTable> swapTable) {
	if(index >= m_swapTables.size()) {
		return false;
	}

	m_swapTables[index] = std::shared_ptr<SwapTable>(swapTable.release());

	return true;
}

bool PaletteDAT::replaceSwapRable(size_t index, const SwapTable & swapTable) {
	if(index >= m_swapTables.size()) {
		return false;
	}

	m_swapTables[index] = std::make_shared<SwapTable>(swapTable);

	return true;
}

bool PaletteDAT::removeSwapTable(size_t index) {
	if(index >= m_swapTables.size()) {
		return false;
	}

	m_swapTables.erase(m_swapTables.begin() + index);

	return true;
}

bool PaletteDAT::removeSwapTable(const SwapTable & swapTable) {
	return removeSwapTable(indexOfSwapTable(swapTable));
}

bool PaletteDAT::removeSwapTableWithIndex(uint8_t swapIndex) {
	return removeSwapTable(indexOfSwapTableWithSwapIndex(swapIndex));
}

void PaletteDAT::clearSwapTables() {
	m_swapTables.clear();
}

std::shared_ptr<ColourTable> PaletteDAT::getColourTable(uint8_t colourTableIndex) const {
	if(colourTableIndex >= m_colourTables.size()) {
		return nullptr;
	}

	return m_colourTables[colourTableIndex];
}

uint8_t PaletteDAT::numberOfColourTables() const {
	return m_colourTables.size();
}

void PaletteDAT::addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const {
	Palette::addMetadata(metadata);

	if(!isValid(false)) {
		return;
	}

	metadata.push_back({ "Data Type", std::string(magic_enum::enum_name(m_type)) });

	switch(m_type) {
		case Type::Palette: {
			metadata.push_back({ "Number of Shade Tables", std::to_string(m_shadeTables.size()) });

			if(m_trailingData->isNotEmpty()) {
				metadata.push_back({ "Number of Trailing Bytes", std::to_string(m_trailingData->getSize()) });
			}

			break;
		}

		case Type::Lookup: {
			metadata.push_back({ "Number of Swap Tables", std::to_string(m_swapTables.size()) });

			break;
		}
	}
}

std::unique_ptr<PaletteDAT> PaletteDAT::readFrom(const ByteBuffer & byteBuffer) {
	// https://wiki.eduke32.com/wiki/Palette_data_files
	// https://wiki.eduke32.com/wiki/Palette_(environment)
	// https://moddingwiki.shikadi.net/wiki/Duke_Nukem_3D_Palette_Format

	std::optional<Type> optionalType(determineType(byteBuffer));

	if(!optionalType.has_value()) {
		spdlog::error("Unknown or invalid Build Engine DAT palette type.");
		return nullptr;
	}

	switch(optionalType.value()) {
		case Type::Palette: {
			return readPaletteFrom(byteBuffer);
		}

		case Type::Lookup: {
			return readLookupFrom(byteBuffer);
		}
	}

	return nullptr;
}

std::unique_ptr<PaletteDAT> PaletteDAT::readPaletteFrom(const ByteBuffer & byteBuffer) {
	byteBuffer.setEndianness(ENDIANNESS);

	std::unique_ptr<ColourTable> colourTable(ColourTable::readFrom(byteBuffer));

	if(colourTable == nullptr) {
		spdlog::error("Failed to read Build Engine DAT regular palette colour table.");
		return nullptr;
	}

	upscaleColourTable(*colourTable);
	colourTable->setTransparentColourIndex(std::numeric_limits<uint8_t>::max());

	bool error = false;

	uint16_t shadeTableCount = byteBuffer.readUnsignedShort(&error);

	if(error) {
		spdlog::error("Failed to read Build Engine DAT regular palette shade table count.");
		return nullptr;
	}

	if(!byteBuffer.canReadBytes(static_cast<size_t>(shadeTableCount) * ColourTable::NUMBER_OF_COLOURS)) {
		spdlog::error("Build Engine DAT regular palette shade tables data is truncated.");
		return nullptr;
	}

	std::vector<std::unique_ptr<ShadeTable>> shadeTables;

	for(uint16_t i = 0 ; i < shadeTableCount; i++) {
		std::unique_ptr<ShadeTable> shadeTable(ShadeTable::readFrom(byteBuffer));

		if(shadeTable == nullptr) {
			spdlog::error("Failed to read Build Engine DAT regular palette shade table #{}.", i + 1);
			return nullptr;
		}

		shadeTables.emplace_back(std::move(shadeTable));
	}

	if(!byteBuffer.canReadBytes(TRANSLUCENCY_TABLE_SIZE_BYTES)) {
		spdlog::error("Build Engine DAT regular palette translucency table data is truncated.");
		return nullptr;
	}

	std::unique_ptr<TranslucencyTable> translucencyTable(TranslucencyTable::readFrom(byteBuffer));

	if(translucencyTable == nullptr) {
		spdlog::error("Failed to read Build Engine DAT regular palette translucency table.");
		return nullptr;
	}

	return std::make_unique<PaletteDAT>(std::move(colourTable), std::move(shadeTables), std::move(translucencyTable), std::move(byteBuffer.getRemainingBytes()));
}

std::unique_ptr<PaletteDAT> PaletteDAT::readLookupFrom(const ByteBuffer & byteBuffer) {
	byteBuffer.setEndianness(ENDIANNESS);

	bool error = false;

	uint8_t swapTableCount = byteBuffer.readUnsignedByte(&error);

	if(error) {
		spdlog::error("Failed to read Build Engine DAT lookup palette swap table count.");
		return nullptr;
	}

	if(!byteBuffer.canReadBytes(static_cast<size_t>(swapTableCount) * ColourTable::NUMBER_OF_COLOURS)) {
		spdlog::error("Build Engine DAT lookup palette swap tables data is truncated.");
		return nullptr;
	}

	std::vector<std::unique_ptr<SwapTable>> swapTables;

	for(uint8_t i = 0; i < swapTableCount; i++) {
		std::unique_ptr<SwapTable> swapTable(SwapTable::readFrom(byteBuffer));

		if(swapTable == nullptr) {
			spdlog::error("Failed to read Build Engine DAT lookup palette swap table #{}.", i + 1);
			return nullptr;
		}

		swapTables.emplace_back(std::move(swapTable));
	}

	size_t bytesRemaining = byteBuffer.numberOfBytesRemaining();
	uint8_t alternateColourTableCount = static_cast<uint8_t>(floor(bytesRemaining / (ColourTable::NUMBER_OF_COLOURS * BYTES_PER_COLOUR)));
	std::vector<std::unique_ptr<ColourTable>> alternateColourTables;
	alternateColourTables.reserve(alternateColourTableCount);

	for(uint8_t i = 0; i < alternateColourTableCount; i++) {
		std::unique_ptr<ColourTable> colourTable(ColourTable::readFrom(byteBuffer));

		if(colourTable == nullptr) {
			spdlog::error("Failed to read Build Engine DAT lookup palette alternate colour table #{}.", i + 1);
			return nullptr;
		}

		if(i < DEFAULT_ALTERNATE_COLOUR_TABLE_INFO.size()) {
			const auto & colourTableInfo = DEFAULT_ALTERNATE_COLOUR_TABLE_INFO[i];

			colourTable->setName(colourTableInfo.first);

			if(colourTableInfo.second) {
				colourTable->setTransparentColourIndex(std::numeric_limits<uint8_t>::max());
			}
		}

		upscaleColourTable(*colourTable);

		alternateColourTables.emplace_back(std::move(colourTable));
	}

	return std::make_unique<PaletteDAT>(std::move(alternateColourTables), std::move(swapTables), std::move(byteBuffer.getRemainingBytes()));
}

std::unique_ptr<PaletteDAT> PaletteDAT::loadFrom(const std::string & filePath) {
	std::unique_ptr<ByteBuffer> paletteData(ByteBuffer::readFrom(filePath));

	if(paletteData == nullptr) {
		spdlog::error("Failed to read Build Engine DAT palette binary data from file: '{}'.", filePath);
		return nullptr;
	}

	std::unique_ptr<PaletteDAT> palette(PaletteDAT::readFrom(*paletteData));

	if(palette == nullptr) {
		spdlog::error("Failed to parse Build Engine DAT palette binary data from file contents: '{}'.", filePath);
		return nullptr;
	}

	return palette;
}

bool PaletteDAT::writeTo(ByteBuffer & byteBuffer) const {
	switch(m_type) {
		case Type::Palette: {
			return writePaletteTo(byteBuffer);
		}

		case Type::Lookup: {
			return writeLookupTo(byteBuffer);
		}
	}

	return false;
}

bool PaletteDAT::writePaletteTo(ByteBuffer & byteBuffer) const {
	if(!isValid(false) ||
	   m_type != Type::Palette ||
	   m_colourTables.size() != 1) {
		spdlog::error("Failed to write invalid Build Engine DAT regular palette data.");
		return false;
	}

	byteBuffer.setEndianness(ENDIANNESS);

	if(!writeDownscaledColourTableTo(*m_colourTables.front(), byteBuffer)) {
		spdlog::error("Failed to write Build Engine DAT regular palette colour table data.");
		return false;
	}

	if(!byteBuffer.writeUnsignedShort(static_cast<uint16_t>(m_shadeTables.size()))) {
		spdlog::error("Failed to write Build Engine DAT regular palette shade table count.");
		return false;
	}

	for(const std::shared_ptr<ShadeTable> & shadeTable : m_shadeTables) {
		if(!shadeTable->writeTo(byteBuffer)) {
			spdlog::error("Failed to write Build Engine DAT regular palette colour shade table data");
			return false;
		}
	}

	if(!m_translucencyTable->writeTo(byteBuffer)) {
		spdlog::error("Failed to write Build Engine DAT regular palette translucency table data.");
		return false;
	}

	if(m_trailingData->isNotEmpty()) {
		if(!byteBuffer.writeBytes(m_trailingData->getData())) {
			spdlog::error("Failed to write Build Engine DAT regular palette trailing data.");
			return false;
		}
	}

	return true;
}

bool PaletteDAT::writeLookupTo(ByteBuffer & byteBuffer) const {
	if(!isValid(false) || m_type != Type::Lookup) {
		spdlog::error("Failed to write invalid Build Engine DAT lookup palette data.");
		return false;
	}

	byteBuffer.setEndianness(ENDIANNESS);

	if(!byteBuffer.writeUnsignedByte(static_cast<uint8_t>(m_swapTables.size()))) {
		spdlog::error("Failed to write Build Engine DAT lookup palette swap table count.");
		return false;
	}

	for(const std::shared_ptr<SwapTable> & swapTable : m_swapTables) {
		if(!swapTable->writeTo(byteBuffer)) {
			spdlog::error("Failed to write Build Engine DAT lookup palette swap table data.");
			return false;
		}
	}

	for(const std::shared_ptr<ColourTable> colourTable : m_colourTables) {
		if(!writeDownscaledColourTableTo(*colourTable, byteBuffer)) {
			spdlog::error("Failed to write Build Engine DAT lookup palette alternate colour table data.");
			return false;
		}
	}

	if(m_trailingData->isNotEmpty()) {
		if(!byteBuffer.writeBytes(m_trailingData->getData())) {
			spdlog::error("Failed to write Build Engine DAT lookup palette trailing data.");
			return false;
		}
	}

	return true;
}

bool PaletteDAT::writeDownscaledColourTableTo(const ColourTable & colourTable, ByteBuffer & byteBuffer) {
	const std::vector<Colour> & colours = colourTable.getColours();

	for(const Colour & colour : colours) {
		for(uint8_t i = 0; i < 3; i++) {
			if(!byteBuffer.writeUnsignedByte(colour.c[i] / COLOUR_SCALE)) {
				spdlog::error("Failed to write colour table data.");
				return false;
			}
		}
	}

	return true;
}

Endianness PaletteDAT::getEndianness() const {
	return ENDIANNESS;
}

size_t PaletteDAT::getSizeInBytes() const {
	switch(m_type) {
		case Type::Palette: {
			return (ColourTable::NUMBER_OF_COLOURS * BYTES_PER_COLOUR) + sizeof(uint16_t) + (m_shadeTables.size() * ColourTable::NUMBER_OF_COLOURS) + TRANSLUCENCY_TABLE_SIZE_BYTES + m_trailingData->getSize();
		}

		case Type::Lookup: {
			return sizeof(uint8_t) + (m_swapTables.size() * (sizeof(uint8_t) + ColourTable::NUMBER_OF_COLOURS)) + (m_colourTables.size() * ColourTable::NUMBER_OF_COLOURS * BYTES_PER_COLOUR) + m_trailingData->getSize();
		}
	}

	return 0;
}

bool PaletteDAT::isValid(bool verifyParent) const {
	if(!Palette::isValid(verifyParent)) {
		return false;
	}

	if(m_trailingData == nullptr) {
		return false;
	}

	switch(m_type) {
		case Type::Palette: {
			if(m_colourTables.size() != 1) {
				return false;
			}

			for(const std::shared_ptr<ShadeTable> & shadeTable : m_shadeTables) {
				if(shadeTable == nullptr) {
					return false;
				}
			}

			if(m_translucencyTable == nullptr) {
				return false;
			}

			break;
		}

		case Type::Lookup: {
			for(const std::shared_ptr<SwapTable> & swapTable : m_swapTables) {
				if(swapTable == nullptr) {
					return false;
				}
			}

			for(size_t i = 0; i < m_swapTables.size(); i++) {
				for(size_t j = i + 1; j < m_swapTables.size(); j++) {
					if(m_swapTables[i]->getSwapIndex() == m_swapTables[j]->getSwapIndex()) {
						return false;
					}
				}
			}

			break;
		}
	}

	if(verifyParent) {
		for(const std::shared_ptr<ColourTable> & colourTable : m_colourTables) {
			if(colourTable->getParent() != this) {
				return false;
			}
		}

		switch(m_type) {
			case Type::Palette: {
				for(const std::shared_ptr<ShadeTable> & shadeTable : m_shadeTables) {
					if(shadeTable->getParent() != this) {
						return false;
					}
				}

				if(m_translucencyTable->getParent() != this) {
					return false;
				}

				break;
			}

			case Type::Lookup: {
				for(const std::shared_ptr<SwapTable> & swapTable : m_swapTables) {
					if(swapTable->getParent() != this) {
						return false;
					}
				}

				break;
			}
		}
	}

	return true;
}

void PaletteDAT::upscaleColourTable(ColourTable & colourTable) {
	for(size_t i = 0; i < colourTable.numberOfColours(); i++) {
		const Colour & colour = colourTable.getColour(i);
		colourTable.setColour(i, colour.r * COLOUR_SCALE, colour.g * COLOUR_SCALE, colour.b * COLOUR_SCALE, colour.a * COLOUR_SCALE);
	}
}

void PaletteDAT::updateParent() {
	for(std::shared_ptr<ColourTable> & colourTable : m_colourTables) {
		colourTable->setParent(this);
	}

	switch(m_type) {
		case Type::Palette: {
			for(std::shared_ptr<ShadeTable> & shadeTable : m_shadeTables) {
				shadeTable->setParent(this);
			}

			m_translucencyTable->setParent(this);

			break;
		}

		case Type::Lookup: {
			for(std::shared_ptr<SwapTable> & swapTable : m_swapTables) {
				swapTable->setParent(this);
			}

			break;
		}
	}
}

bool PaletteDAT::operator == (const PaletteDAT & palette) const {
	if(m_type != palette.m_type ||
	   m_colourTables.size() != palette.m_colourTables.size() ||
	   *m_trailingData != *palette.m_trailingData) {
		return false;
	}

	for(size_t i = 0; i < m_colourTables.size(); i++) {
		if(*m_colourTables[i] != *palette.m_colourTables[i]) {
			return false;
		}
	}

	switch(m_type) {
		case Type::Palette: {
			if(m_shadeTables.size() != palette.m_shadeTables.size()) {
				return false;
			}

			for(size_t i = 0; i < m_shadeTables.size(); i++) {
				if(*m_shadeTables[i] != *palette.m_shadeTables[i]) {
					return false;
				}
			}

			if(*m_translucencyTable != *palette.m_translucencyTable) {
				return false;
			}

			break;
		}

		case Type::Lookup: {
			if(m_swapTables.size() != palette.m_swapTables.size()) {
				return false;
			}

			for(size_t i = 0; i < m_swapTables.size(); i++) {
				if(*m_swapTables[i] != *palette.m_swapTables[i]) {
					return false;
				}
			}

			break;
		}
	}

	return true;
}

bool PaletteDAT::operator != (const PaletteDAT & palette) const {
	return !operator == (palette);
}
