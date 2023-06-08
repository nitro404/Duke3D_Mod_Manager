#include "Art.h"

#include <ByteBuffer.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

#include <filesystem>

static const std::string JSON_VERSION_PROPERTY_NAME("version");
static const std::string JSON_LOCAL_TILE_START_PROPERTY_NAME("localTileStart");
static const std::string JSON_LOCAL_TILE_END_PROPERTY_NAME("localTileEnd");
static const std::string JSON_LEGACY_TILE_COUNT_PROPERTY_NAME("legacyTileCount");
static const std::string JSON_TILES_PROPERTY_NAME("tiles");
static const std::string JSON_TRAILING_DATA_PROPERTY_NAME("trailingData");

static constexpr bool BASE_64_ENCODE_DATA = false;

Art::Art()
	: m_version(DEFAULT_VERSION)
	, m_localTileStart(0)
	, m_localTileEnd(DEFAULT_NUMBER_OF_TILES - 1)
	, m_legacyTileCount(0) { }

Art::Art(uint16_t artNumber, uint32_t tileCount)
	: m_version(DEFAULT_VERSION)
	, m_localTileStart(artNumber * tileCount)
	, m_localTileEnd(m_localTileStart + tileCount - 1)
	, m_legacyTileCount(0) {
	for(uint32_t i = 0; i < tileCount; i++) {
		m_tiles.push_back(std::make_shared<Tile>(m_localTileStart + i, this));
	}
}

Art::Art(uint16_t artNumber, uint32_t tileCount, std::unique_ptr<ByteBuffer> trailingData)
	: m_version(DEFAULT_VERSION)
	, m_localTileStart(artNumber * tileCount)
	, m_localTileEnd(m_localTileStart + tileCount - 1)
	, m_legacyTileCount(0)
	, m_trailingData(std::shared_ptr<ByteBuffer>(trailingData.release())) {
	for(uint32_t i = 0; i < tileCount; i++) {
		m_tiles.push_back(std::make_shared<Tile>(m_localTileStart + i, this));
	}
}

Art::Art(uint16_t artNumber, uint32_t tileCount, ByteBuffer && trailingData)
	: m_version(DEFAULT_VERSION)
	, m_localTileStart(artNumber * tileCount)
	, m_localTileEnd(m_localTileStart + tileCount - 1)
	, m_legacyTileCount(0)
	, m_trailingData(trailingData.isNotEmpty() ? std::make_shared<ByteBuffer>(std::move(trailingData)) : nullptr) {
	for(uint32_t i = 0; i < tileCount; i++) {
		m_tiles.push_back(std::make_shared<Tile>(m_localTileStart + i, this));
	}
}

Art::Art(uint32_t localTileStart, uint32_t localTileEnd, uint32_t legacyTileCount, std::vector<std::unique_ptr<Tile>> tiles, std::unique_ptr<ByteBuffer> trailingData)
	: m_version(DEFAULT_VERSION)
	, m_localTileStart(localTileStart)
	, m_localTileEnd(localTileEnd)
	, m_legacyTileCount(legacyTileCount)
	, m_trailingData(std::shared_ptr<ByteBuffer>(trailingData.release())) {
	m_tiles.reserve(tiles.size());

	for(std::unique_ptr<Tile> & tile : tiles) {
		m_tiles.push_back(std::shared_ptr<Tile>(tile.release()));
	}

	autosize();
	updateParent();
}

Art::Art(uint32_t localTileStart, uint32_t localTileEnd, uint32_t legacyTileCount, std::vector<std::unique_ptr<Tile>> tiles, ByteBuffer && trailingData)
	: m_version(DEFAULT_VERSION)
	, m_localTileStart(localTileStart)
	, m_localTileEnd(localTileEnd)
	, m_legacyTileCount(legacyTileCount)
	, m_trailingData(trailingData.isNotEmpty() ? std::make_shared<ByteBuffer>(std::move(trailingData)) : nullptr) {
	m_tiles.reserve(tiles.size());

	for(std::unique_ptr<Tile> & tile : tiles) {
		m_tiles.push_back(std::shared_ptr<Tile>(tile.release()));
	}

	autosize();
	updateParent();
}

Art::Art(Art && art) noexcept
	: m_version(art.m_version)
	, m_localTileStart(art.m_localTileEnd)
	, m_localTileEnd(art.m_localTileStart)
	, m_legacyTileCount(art.m_legacyTileCount)
	, m_tiles(std::move(art.m_tiles))
	, m_trailingData(std::move(art.m_trailingData)) {
	autosize();
	updateParent();
}

Art::Art(const Art & art)
	: m_version(art.m_version)
	, m_localTileStart(art.m_localTileEnd)
	, m_localTileEnd(art.m_localTileStart)
	, m_legacyTileCount(art.m_legacyTileCount)
	, m_trailingData(art.m_trailingData != nullptr ? std::make_shared<ByteBuffer>(*art.m_trailingData) : nullptr) {
	m_tiles.clear();
	m_tiles.reserve(art.m_tiles.size());

	for(const std::shared_ptr<Tile> & tile : art.m_tiles) {
		m_tiles.emplace_back(std::make_shared<Tile>(*tile));
	}

	autosize();
	updateParent();
}

Art & Art::operator = (Art && art) noexcept {
	if(this != &art) {
		m_version = art.m_version;
		m_localTileStart = art.m_localTileEnd;
		m_localTileEnd = art.m_localTileStart;
		m_legacyTileCount = art.m_legacyTileCount;
		m_tiles = std::move(art.m_tiles);

		if(art.m_trailingData != nullptr) {
			m_trailingData = std::move(art.m_trailingData);
		}
		else {
			m_trailingData.reset();
		}

		autosize();
		updateParent();
	}

	return *this;
}

Art & Art::operator = (const Art & art) {
	m_tiles.clear();
	m_tiles.reserve(art.m_tiles.size());

	m_version = art.m_version;
	m_localTileStart = art.m_localTileEnd;
	m_localTileEnd = art.m_localTileStart;
	m_legacyTileCount = art.m_legacyTileCount;

	if(art.m_trailingData != nullptr) {
		m_trailingData = std::make_shared<ByteBuffer>(*art.m_trailingData);
	}
	else {
		m_trailingData.reset();
	}

	for(const std::shared_ptr<Tile> & tile : art.m_tiles) {
		m_tiles.emplace_back(std::make_shared<Tile>(*tile));
	}

	autosize();
	updateParent();

	return *this;
}

Art::~Art() = default;

uint32_t Art::getVersion() const {
	return m_version;
}

void Art::setVersion(uint32_t version) {
	m_version = version;
}

uint16_t Art::getNumber() const {
	return static_cast<uint16_t>(floor(m_localTileStart / numberOfTiles()));
}

void Art::setNumber(uint16_t number) {
	setLocalTileStart(number * numberOfTiles());
}

uint32_t Art::getLocalTileStart() const {
	return m_localTileStart;
}

void Art::setLocalTileStart(uint32_t localTileStart) {
	if(m_localTileStart == localTileStart) {
		return;
	}

	size_t tileCount = numberOfTiles();

	m_localTileStart = localTileStart;
	m_localTileEnd = m_localTileStart + tileCount - 1;

	for(size_t i = 0; i < m_tiles.size(); i++) {
		m_tiles[i]->setNumber(m_localTileStart + i);
	}
}

uint32_t Art::getLocalTileEnd() const {
	return m_localTileEnd;
}

bool Art::setLocalTileEnd(uint32_t localTileEnd) {
	if(m_localTileEnd == localTileEnd) {
		return true;
	}

	if(localTileEnd < m_localTileStart) {
		spdlog::error("Invalid art local tile end value: {}, expected number greater than or equal to the local tile start value: {}.", localTileEnd, m_localTileStart);
		return false;
	}

	m_localTileEnd = localTileEnd;

	autosize();

	return true;
}

uint32_t Art::getLegacyTileCount() const {
	return m_legacyTileCount;
}

void Art::setLegacyTileCount(uint32_t legacyTileCount) {
	m_legacyTileCount = legacyTileCount;
}

bool Art::hasTrailingData() const {
	return m_trailingData != nullptr &&
		   m_trailingData->isNotEmpty();
}

std::shared_ptr<ByteBuffer> Art::getTailingData() const {
	return m_trailingData;
}

void Art::setTrailingData(std::unique_ptr<ByteBuffer> trailingData) {
	if(trailingData == nullptr) {
		clearTrailingData();
		return;
	}

	m_trailingData = std::shared_ptr<ByteBuffer>(trailingData.release());
}

void Art::clearTrailingData() {
	m_trailingData.reset();
}

size_t Art::numberOfTiles() const {
	return m_tiles.size();
}

void Art::setNumberOfTiles(uint32_t tileCount) {
	setLocalTileStart(getNumber() * tileCount);
	setLocalTileEnd(m_localTileStart + tileCount - 1);
}

bool Art::hasTile(const Tile & tile) const {
	return indexOfTile(tile) != std::numeric_limits<size_t>::max();
}

bool Art::hasTileWithNumber(uint32_t number) const {
	return number >= m_localTileStart && number <= m_localTileEnd;
}

bool Art::hasNonEmptyTile() const {
	for(const std::shared_ptr<Tile> & tile : m_tiles) {
		if(tile->isNotEmpty()) {
			return true;
		}
	}

	return false;
}

bool Art::hasEmptyTile() const {
	for(const std::shared_ptr<Tile> & tile : m_tiles) {
		if(tile->isEmpty()) {
			return true;
		}
	}

	return false;
}

size_t Art::indexOfTile(const Tile & tile) const {
	if(tile.getNumber() < m_localTileStart || tile.getNumber() > m_localTileEnd) {
		return std::numeric_limits<size_t>::max();
	}

	return m_tiles[tile.getNumber() - m_localTileStart].get() == &tile;
}

uint32_t Art::numberOfNonEmptyTiles() const {
	uint32_t nonEmptyTileCount = 0;

	for(const std::shared_ptr<Tile> & tile : m_tiles) {
		if(tile->isNotEmpty()) {
			nonEmptyTileCount++;
		}
	}

	return nonEmptyTileCount;
}

uint32_t Art::numberOfEmptyTiles() const {
	uint32_t emptyTileCount = 0;

	for(const std::shared_ptr<Tile> & tile : m_tiles) {
		if(tile->isEmpty()) {
			emptyTileCount++;
		}
	}

	return emptyTileCount;
}

std::shared_ptr<Tile> Art::getTileByIndex(size_t index) const {
	if(index >= m_tiles.size()) {
		return nullptr;
	}

	return m_tiles[index];
}

std::shared_ptr<Tile> Art::getTileWithNumber(uint32_t number) const {
	if(number < m_localTileStart) {
		return nullptr;
	}

	return getTileByIndex(number - m_localTileStart);
}

const std::vector<std::shared_ptr<Tile>> & Art::getTiles() const {
	return m_tiles;
}

bool Art::setTile(std::unique_ptr<Tile> tile) {
	if(tile == nullptr) {
		return false;
	}

	uint32_t tileNumber = tile->getNumber();

	return setTile(std::move(tile), tileNumber);
}

bool Art::setTile(std::unique_ptr<Tile> tile, uint32_t tileNumber) {
	if(tileNumber < m_localTileStart || tileNumber > m_localTileEnd) {
		return false;
	}

	uint32_t tileIndex = tileNumber - m_localTileStart;

	tile->setNumber(tileNumber);
	tile->setParent(this);
	m_tiles[tileIndex] = std::move(tile);

	return true;
}

bool Art::replaceTile(const Tile & tile) {
	if(tile == nullptr) {
		return false;
	}

	return replaceTile(tile, tile.getNumber());
}

bool Art::replaceTile(const Tile & tile, uint32_t tileNumber) {
	if(tileNumber < m_localTileStart || tileNumber > m_localTileEnd) {
		return false;
	}

	uint32_t tileIndex = tileNumber - m_localTileStart;

	std::unique_ptr<Tile> newTile(std::make_unique<Tile>(tile));
	newTile->setNumber(tileNumber);
	newTile->setParent(this);
	m_tiles[tileIndex] = std::move(newTile);

	return true;
}

bool Art::clearTile(uint32_t tileNumber) {
	std::shared_ptr<Tile> tile(getTileWithNumber(tileNumber));

	if(tile == nullptr) {
		return false;
	}

	tile->clear();

	return true;
}

std::vector<std::shared_ptr<Tile>> Art::getNonEmptyEmptyTiles() const {
	std::vector<std::shared_ptr<Tile>> nonEmptyTiles;

	for(const std::shared_ptr<Tile> & tile : m_tiles) {
		if(tile->isNotEmpty()) {
			nonEmptyTiles.push_back(tile);
		}
	}

	return nonEmptyTiles;
}

std::vector<std::shared_ptr<Tile>> Art::getEmptyTiles() const {
	std::vector<std::shared_ptr<Tile>> emptyTiles;

	for(const std::shared_ptr<Tile> & tile : m_tiles) {
		if(tile->isEmpty()) {
			emptyTiles.push_back(tile);
		}
	}

	return emptyTiles;
}

std::unique_ptr<Art> Art::readFrom(const ByteBuffer & byteBuffer) {
	bool error = false;

	byteBuffer.setEndianness(ENDIANNESS);

	uint32_t version = byteBuffer.readUnsignedInteger(&error);

	// some converted art files have a bad header structure and require additional processing
	if(version == 0x4c495542 && !error) {
		version = byteBuffer.readUnsignedInteger(&error);

		if(version == 0x54524144 && !error) {
			version = byteBuffer.readUnsignedInteger(&error);
		}
	}

	if(error) {
		spdlog::error("Failed to read art version.");
		return nullptr;
	}

	if(version != DEFAULT_VERSION) {
		spdlog::error("Invalid or unsupported art file version: {}, expected version: {}.", version, DEFAULT_VERSION);
		return nullptr;
	}

	uint32_t legacyTileCount = byteBuffer.readUnsignedInteger(&error);

	if(error) {
		spdlog::error("Failed to read art legacy tile count value.");
		return nullptr;
	}

	uint32_t localTileStart = byteBuffer.readUnsignedInteger(&error);

	if(error) {
		spdlog::error("Failed to read art local tile start value.");
		return nullptr;
	}

	uint32_t localTileEnd = byteBuffer.readUnsignedInteger(&error);

	if(error) {
		spdlog::error("Failed to read art local tile end value.");
		return nullptr;
	}

	if(localTileStart > localTileEnd) {
		spdlog::error("Art local tile start value of {} exceeds local tile end value of {}.", localTileStart, localTileEnd);
		return nullptr;
	}

	uint32_t tileCount = localTileEnd - localTileStart + 1;

	std::vector<uint16_t> tileWidths;
	std::vector<uint16_t> tileHeights;
	std::vector<Tile::Attributes> tileAttributes;

	for(uint32_t i = 0; i < tileCount; i++) {
		tileWidths.push_back(byteBuffer.readUnsignedShort(&error));

		if(error) {
			spdlog::error("Failed to read art tile #{} width value.", tileWidths.size() + 1);
			return nullptr;
		}
	}

	for(uint32_t i = 0; i < tileCount; i++) {
		tileHeights.push_back(byteBuffer.readUnsignedShort(&error));

		if(error) {
			spdlog::error("Failed to read art tile #{} height value.", tileHeights.size() + 1);
			return nullptr;
		}
	}

	for(uint32_t i = 0; i < tileCount; i++) {
		Tile::Attributes tileAttribute;
		tileAttribute.rawValue = byteBuffer.readUnsignedInteger(&error);

		if(error) {
			spdlog::error("Failed to read art tile #{} attributes.", tileAttributes.size() + 1);
			return nullptr;
		}

		tileAttributes.push_back(tileAttribute);
	}

	std::vector<std::unique_ptr<Tile>> tiles;

	for(uint32_t i = 0; i < tileCount; i++) {
		uint64_t pixelCount = static_cast<uint64_t>(tileWidths[i]) * tileHeights[i];

		std::unique_ptr<std::vector<uint8_t>> tileBytes(byteBuffer.readBytes(pixelCount));

		if(tileBytes == nullptr) {
			spdlog::error("Failed to read art tile #{} pixel data.", tiles.size() + 1);
			return nullptr;
		}

		tiles.emplace_back(std::make_unique<Tile>(localTileStart + i, tileWidths[i], tileHeights[i], tileAttributes[i], std::make_unique<ByteBuffer>(std::move(tileBytes))));
	}

	std::unique_ptr<ByteBuffer> trailingData(byteBuffer.getRemainingBytes());

	if(trailingData != nullptr && trailingData->isNotEmpty()) {
		spdlog::info("Art data has an additional {} trailing bytes.");
	}

	return std::make_unique<Art>(localTileStart, localTileEnd, legacyTileCount, std::move(tiles), std::move(trailingData));
}

bool Art::writeTo(ByteBuffer & byteBuffer) const {
	byteBuffer.setEndianness(ENDIANNESS);

	if(!byteBuffer.writeUnsignedInteger(m_version)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedInteger(m_legacyTileCount)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedInteger(m_localTileStart)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedInteger(m_localTileEnd)) {
		return false;
	}

	for(const std::shared_ptr<Tile> & tile : m_tiles) {
		if(!byteBuffer.writeUnsignedShort(tile->getWidth())) {
			return false;
		}
	}

	for(const std::shared_ptr<Tile> & tile : m_tiles) {
		if(!byteBuffer.writeUnsignedShort(tile->getHeight())) {
			return false;
		}
	}

	for(const std::shared_ptr<Tile> & tile : m_tiles) {
		if(!byteBuffer.writeUnsignedInteger(tile->getAttributes().rawValue)) {
			return false;
		}
	}

	for(const std::shared_ptr<Tile> & tile : m_tiles) {
		if(tile->isEmpty()) {
			continue;
		}

		if(!byteBuffer.writeBytes(*tile->getData())) {
			return false;
		}
	}

	return true;
}

rapidjson::Document Art::toJSON() const {
	rapidjson::Document artDocument(rapidjson::kObjectType);

	addToJSONObject(artDocument, artDocument.GetAllocator());

	return artDocument;
}

rapidjson::Value Art::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value artValue(rapidjson::kObjectType);

	addToJSONObject(artValue, allocator);

	return artValue;
}

bool Art::addToJSONObject(rapidjson::Value & value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	if(!value.IsObject()) {
		return false;
	}

	value.AddMember(rapidjson::StringRef(JSON_VERSION_PROPERTY_NAME.c_str()), rapidjson::Value(m_version), allocator);

	value.AddMember(rapidjson::StringRef(JSON_LOCAL_TILE_START_PROPERTY_NAME.c_str()), rapidjson::Value(m_localTileStart), allocator);

	value.AddMember(rapidjson::StringRef(JSON_LOCAL_TILE_END_PROPERTY_NAME.c_str()), rapidjson::Value(m_localTileEnd), allocator);

	value.AddMember(rapidjson::StringRef(JSON_LEGACY_TILE_COUNT_PROPERTY_NAME.c_str()), rapidjson::Value(m_legacyTileCount), allocator);

	rapidjson::Value tilesValue(rapidjson::kArrayType);

	for(const std::shared_ptr<Tile> & tile : m_tiles) {
		rapidjson::Value tileValue(tile->toJSON(allocator));
		tilesValue.PushBack(tileValue, allocator);
	}

	value.AddMember(rapidjson::StringRef(JSON_TILES_PROPERTY_NAME.c_str()), tilesValue, allocator);

	if(m_trailingData != nullptr && m_trailingData->isNotEmpty()) {
		if(BASE_64_ENCODE_DATA) {
			std::string base64TrailingData(m_trailingData->toBase64());
			rapidjson::Value trailingDataValue(base64TrailingData.c_str(), allocator);
			value.AddMember(rapidjson::StringRef(JSON_TRAILING_DATA_PROPERTY_NAME.c_str()), trailingDataValue, allocator);
		}
		else {
			rapidjson::Value trailingDataValue(rapidjson::kArrayType);

			for(size_t i = 0; i < m_trailingData->getSize(); i++) {
				trailingDataValue.PushBack(rapidjson::Value((*m_trailingData)[i]), allocator);
			}

			value.AddMember(rapidjson::StringRef(JSON_TRAILING_DATA_PROPERTY_NAME.c_str()), trailingDataValue, allocator);
		}
	}

	return true;
}

std::unique_ptr<Art> Art::parseFrom(const rapidjson::Value & artValue) {
	if(!artValue.IsObject()) {
		spdlog::error("Invalid art type: '{}', expected 'object'.", Utilities::typeToString(artValue.GetType()));
		return nullptr;
	}

	// parse version
	if(!artValue.HasMember(JSON_VERSION_PROPERTY_NAME.c_str())) {
		spdlog::error("Art is missing '{}' property.", JSON_VERSION_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & versionValue = artValue[JSON_VERSION_PROPERTY_NAME.c_str()];

	if(!versionValue.IsUint()) {
		spdlog::error("Art has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_VERSION_PROPERTY_NAME, Utilities::typeToString(versionValue.GetType()));
		return nullptr;
	}

	uint32_t version = versionValue.GetUint();

	// parse local tile start
	if(!artValue.HasMember(JSON_LOCAL_TILE_START_PROPERTY_NAME.c_str())) {
		spdlog::error("Art is missing '{}' property.", JSON_LOCAL_TILE_START_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & localTileStartValue = artValue[JSON_LOCAL_TILE_START_PROPERTY_NAME.c_str()];

	if(!localTileStartValue.IsUint()) {
		spdlog::error("Art has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_LOCAL_TILE_START_PROPERTY_NAME, Utilities::typeToString(localTileStartValue.GetType()));
		return nullptr;
	}

	uint32_t localTileStart = localTileStartValue.GetUint();

	// parse local tile end
	if(!artValue.HasMember(JSON_LOCAL_TILE_END_PROPERTY_NAME.c_str())) {
		spdlog::error("Art is missing '{}' property.", JSON_LOCAL_TILE_END_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & localTileEndValue = artValue[JSON_LOCAL_TILE_END_PROPERTY_NAME.c_str()];

	if(!localTileEndValue.IsUint()) {
		spdlog::error("Art has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_LOCAL_TILE_END_PROPERTY_NAME, Utilities::typeToString(localTileEndValue.GetType()));
		return nullptr;
	}

	uint32_t localTileEnd = localTileEndValue.GetUint();

	// parse legacy tile count
	if(!artValue.HasMember(JSON_LEGACY_TILE_COUNT_PROPERTY_NAME.c_str())) {
		spdlog::error("Art is missing '{}' property.", JSON_LEGACY_TILE_COUNT_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & legacyTileCountValue = artValue[JSON_LEGACY_TILE_COUNT_PROPERTY_NAME.c_str()];

	if(!legacyTileCountValue.IsUint()) {
		spdlog::error("Art has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_LEGACY_TILE_COUNT_PROPERTY_NAME, Utilities::typeToString(legacyTileCountValue.GetType()));
		return nullptr;
	}

	uint32_t legacyTileCount = legacyTileCountValue.GetUint();

	// parse tiles
	if(!artValue.HasMember(JSON_TILES_PROPERTY_NAME.c_str())) {
		spdlog::error("Art is missing '{}' property.", JSON_TILES_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & tilesValue = artValue[JSON_TILES_PROPERTY_NAME.c_str()];

	if(!tilesValue.IsArray()) {
		spdlog::error("Art has an invalid '{}' property type: '{}', expected 'array'.", JSON_TILES_PROPERTY_NAME, Utilities::typeToString(tilesValue.GetType()));
		return nullptr;
	}

	std::vector<std::unique_ptr<Tile>> tiles;

	for(rapidjson::Value::ConstValueIterator i = tilesValue.Begin(); i != tilesValue.End(); ++i) {
		std::unique_ptr<Tile> tile(Tile::parseFrom(*i));

		if(tile == nullptr) {
			spdlog::error("Failed to parse art tile #{}.", tiles.size() + 1);
			return nullptr;
		}

		tiles.push_back(std::move(tile));
	}

	// parse base 64 encoded trailing data
	if(!artValue.HasMember(JSON_TRAILING_DATA_PROPERTY_NAME.c_str())) {
		return std::make_unique<Art>(localTileStart, localTileEnd, legacyTileCount, std::move(tiles));
	}

	const rapidjson::Value & trailingDataValue = artValue[JSON_TRAILING_DATA_PROPERTY_NAME.c_str()];

	if(trailingDataValue.IsArray()) {
		std::unique_ptr<ByteBuffer> trailingData(std::make_unique<ByteBuffer>());
		trailingData->reserve(trailingDataValue.Size());

		for(rapidjson::Value::ConstValueIterator i = trailingDataValue.Begin(); i != trailingDataValue.End(); ++i) {
			const rapidjson::Value & trailingByteValue = *i;

			if(!trailingByteValue.IsUint()) {
				spdlog::error("Art '{}' property byte #{} has an invalid type: '{}', expected unsigned byte 'number'.", JSON_TRAILING_DATA_PROPERTY_NAME, trailingData->getSize(), Utilities::typeToString(trailingByteValue.GetType()));
				return nullptr;
			}

			uint32_t trailingByte = trailingByteValue.GetUint();

			if(trailingByte > std::numeric_limits<uint8_t>::max()) {
				spdlog::error("Art '{}' property byte #{} has an invalid value: {}, expected unsigned byte 'number' between 0 and {}, inclusively.", JSON_TRAILING_DATA_PROPERTY_NAME, trailingData->getSize(), trailingByte, std::numeric_limits<uint8_t>::max());
				return nullptr;
			}

			trailingData->writeByte(static_cast<uint8_t>(trailingByte));
		}

		return std::make_unique<Art>(localTileStart, localTileEnd, legacyTileCount, std::move(tiles), std::move(trailingData));
	}
	else if(trailingDataValue.IsString()) {
		std::optional<ByteBuffer> optionalTrailingData(ByteBuffer::fromBase64(trailingDataValue.GetString()));

		if(!optionalTrailingData.has_value()) {
			spdlog::error("Art has invalid base 64 encoded trailing data.");
			return nullptr;
		}

		return std::make_unique<Art>(localTileStart, localTileEnd, legacyTileCount, std::move(tiles), std::move(optionalTrailingData.value()));
	}
	else {
		spdlog::error("Art has an invalid '{}' property type: '{}', expected 'array' or 'string'.", JSON_TRAILING_DATA_PROPERTY_NAME, Utilities::typeToString(trailingDataValue.GetType()));
		return nullptr;
	}
}

std::unique_ptr<Art> Art::loadFrom(const std::string & filePath) {
	std::string fileExtension(Utilities::getFileExtension(filePath));

	if(Utilities::areStringsEqualIgnoreCase(fileExtension, "json")) {
		return loadFromJSON(filePath);
	}
	else {
		return loadFromArt(filePath);
	}
}

std::unique_ptr<Art> Art::loadFromArt(const std::string & filePath) {
	std::unique_ptr<ByteBuffer> artData(ByteBuffer::readFrom(filePath, ENDIANNESS));

	if(artData == nullptr) {
		spdlog::error("Failed to read art binary data from file: '{}'.", filePath);
		return nullptr;
	}

	std::unique_ptr<Art> art(readFrom(*artData));

	if(art == nullptr) {
		spdlog::error("Failed to parse art binary data from file contents: '{}'.", filePath);
		return nullptr;
	}

	return art;
}

std::unique_ptr<Art> Art::loadFromJSON(const std::string & filePath) {
	std::optional<rapidjson::Document> optionalArtDocument(Utilities::loadJSONDocumentFrom(filePath));

	if(!optionalArtDocument.has_value()) {
		return nullptr;
	}

	std::unique_ptr<Art> art(parseFrom(optionalArtDocument.value()));

	if(art == nullptr) {
		spdlog::error("Failed to parse art from JSON file: '{}'.", filePath);
		return nullptr;
	}

	return art;
}

bool Art::saveTo(const std::string & filePath, bool overwrite) const {
	std::string fileExtension(Utilities::getFileExtension(filePath));

	if(Utilities::areStringsEqualIgnoreCase(fileExtension, "json")) {
		return saveToJSON(filePath, overwrite);
	}
	else {
		return saveToArt(filePath, overwrite);
	}
}

bool Art::saveToArt(const std::string & filePath, bool overwrite) const {
	if(filePath.empty()) {
		return false;
	}

	if(!overwrite && std::filesystem::exists(std::filesystem::path(filePath))) {
		spdlog::warn("File '{}' already exists, use overwrite to force write.", filePath);
		return false;
	}

	std::unique_ptr<ByteBuffer> artData(getData());

	if(artData == nullptr) {
		spdlog::error("Failed to serialize art data.");
		return false;
	}

	return artData->writeTo(filePath, overwrite);
}

bool Art::saveToJSON(const std::string & filePath, bool overwrite) const {
	return Utilities::saveJSONValueTo(toJSON(), filePath, overwrite);
}

void Art::addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const {
	metadata.push_back({ "Version", std::to_string(m_version) });
	metadata.push_back({ "Art Number", std::to_string(getNumber()) });
	metadata.push_back({ "Number of Tiles", std::to_string(m_tiles.size()) });
	metadata.push_back({ "Number of Non-Empty Tiles", std::to_string(numberOfNonEmptyTiles()) });
	metadata.push_back({ "Number of Empty Tiles", std::to_string(numberOfEmptyTiles()) });
	metadata.push_back({ "Local Tile Start", std::to_string(m_localTileStart) });
	metadata.push_back({ "Local Tile End", std::to_string(m_localTileEnd) });
	metadata.push_back({ "Legacy Tile Count", std::to_string(m_legacyTileCount) });
}

Endianness Art::getEndianness() const {
	return ENDIANNESS;
}

uint64_t Art::getSizeInBytes() const {
	size_t numberOfBytes = (sizeof(uint32_t) * 4) + (m_tiles.size() * ((sizeof(uint16_t) * 2) + sizeof(uint32_t))) + (m_trailingData != nullptr ? m_trailingData->getSize() : 0);

	for(const std::shared_ptr<Tile> & tile : m_tiles) {
		numberOfBytes += tile->getSize();
	}

	return numberOfBytes;
}

void Art::autosize() {
	uint32_t oldLocalTileEnd = m_localTileStart + m_tiles.size() - 1;

	if(m_localTileEnd == oldLocalTileEnd) {
		return;
	}

	if(m_localTileEnd < oldLocalTileEnd) {
		for(uint32_t i = m_localTileEnd + 1; i < m_tiles.size(); i++) {
			m_tiles[i]->clearParent();
		}

		m_tiles.resize(m_localTileEnd + 1);
	}
	else {
		uint32_t numberOfTilesToAdd = m_localTileEnd - oldLocalTileEnd;

		for(uint32_t i = 0; i < numberOfTilesToAdd; i++) {
			m_tiles.push_back(std::make_shared<Tile>(oldLocalTileEnd + i + 1, this));
		}
	}
}

void Art::updateParent() {
	for(std::shared_ptr<Tile> & tile : m_tiles) {
		tile->setParent(this);
	}
}

bool Art::isValid(bool verifyParent) const {
	return true;
}

bool Art::isValid(const Art * art, bool verifyParent) {
	return art != nullptr && art->isValid();
}

bool Art::operator == (const Art & art) const {
	if(m_version != art.m_version ||
	   m_localTileStart != art.m_localTileStart ||
	   m_localTileEnd != art.m_localTileEnd ||
	   m_legacyTileCount != art.m_legacyTileCount ||
	   m_tiles.size() != art.m_tiles.size() ||
	   !((m_trailingData == nullptr && art.m_trailingData == nullptr) || (m_trailingData != nullptr && art.m_trailingData != nullptr && *m_trailingData != *art.m_trailingData))) {
		return false;
	}

	for(size_t i = 0; i < m_tiles.size(); i++) {
		if(*m_tiles[i] != *art.m_tiles[i]) {
			return false;
		}
	}

	return true;
}

bool Art::operator != (const Art & art) const {
	return !operator == (art);
}
