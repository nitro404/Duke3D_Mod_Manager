#include "Map.h"

#include <ByteBuffer.h>
#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <string>

static const std::string JSON_VERSION_PROPERTY_NAME("version");
static const std::string JSON_PLAYER_SPAWN_PROPERTY_NAME("playerSpawn");
static const std::string JSON_SECTORS_PROPERTY_NAME("sectors");
static const std::string JSON_WALLS_PROPERTY_NAME("walls");
static const std::string JSON_SPRITES_PROPERTY_NAME("sprites");
static const std::string JSON_TRAILING_DATA_PROPERTY_NAME("trailingData");

Map::Map()
	: m_version(DEFAULT_VERSION) { }

Map::Map(uint32_t version, const PlayerSpawn & playerSpawn, std::vector<std::unique_ptr<Sector>> sectors, std::vector<std::unique_ptr<Wall>> walls, std::vector<std::unique_ptr<Sprite>> sprites, std::unique_ptr<ByteBuffer> trailingData)
	: m_version(version)
	, m_playerSpawn(playerSpawn)
	, m_trailingData(trailingData != nullptr ? std::move(trailingData) : std::make_unique<ByteBuffer>()) {
	m_sectors.reserve(sectors.size());

	for(std::unique_ptr<Sector> & sector : sectors) {
		m_sectors.push_back(std::shared_ptr<Sector>(sector.release()));
	}

	m_walls.reserve(walls.size());

	for(std::unique_ptr<Wall> & wall : walls) {
		m_walls.push_back(std::shared_ptr<Wall>(wall.release()));
	}

	m_sprites.reserve(sprites.size());

	for(std::unique_ptr<Sprite> & sprite : sprites) {
		m_sprites.push_back(std::shared_ptr<Sprite>(sprite.release()));
	}
}

Map::Map(Map && map) noexcept
	: m_version(map.m_version)
	, m_playerSpawn(std::move(map.m_playerSpawn))
	, m_trailingData(std::move(map.m_trailingData))
	, m_sectors(std::move(map.m_sectors))
	, m_walls(std::move(map.m_walls))
	, m_sprites(std::move(map.m_sprites)) { }

Map::Map(const Map & map)
	: m_version(map.m_version)
	, m_playerSpawn(map.m_playerSpawn)
	, m_trailingData(map.m_trailingData) {
	m_sectors.reserve(map.m_sectors.size());
	m_walls.reserve(map.m_walls.size());
	m_sprites.reserve(map.m_sprites.size());

	for(const std::shared_ptr<Sector> sector : map.m_sectors) {
		m_sectors.emplace_back(std::make_shared<Sector>(*sector));
	}

	for(const std::shared_ptr<Wall> wall : map.m_walls) {
		m_walls.emplace_back(std::make_shared<Wall>(*wall));
	}

	for(const std::shared_ptr<Sprite> sprite : map.m_sprites) {
		m_sprites.emplace_back(std::make_shared<Sprite>(*sprite));
	}
}

Map & Map::operator = (Map && map) noexcept {
	if(this != &map) {
		m_version = map.m_version;
		m_playerSpawn = std::move(map.m_playerSpawn);
		m_trailingData = std::move(map.m_trailingData);
		m_sectors = std::move(map.m_sectors);
		m_walls = std::move(map.m_walls);
		m_sprites = std::move(map.m_sprites);
	}

	return *this;
}

Map & Map::operator = (const Map & map) {
	m_sectors.clear();
	m_walls.clear();
	m_sprites.clear();

	m_sectors.reserve(map.m_sectors.size());
	m_walls.reserve(map.m_walls.size());
	m_sprites.reserve(map.m_sprites.size());

	m_version = map.m_version;
	m_playerSpawn = map.m_playerSpawn;
	m_trailingData = map.m_trailingData;

	for(const std::shared_ptr<Sector> sector : map.m_sectors) {
		m_sectors.emplace_back(std::make_shared<Sector>(*sector));
	}

	for(const std::shared_ptr<Wall> wall : map.m_walls) {
		m_walls.emplace_back(std::make_shared<Wall>(*wall));
	}

	for(const std::shared_ptr<Sprite> sprite : map.m_sprites) {
		m_sprites.emplace_back(std::make_shared<Sprite>(*sprite));
	}

	return *this;
}

Map::~Map() { }

uint32_t Map::getVersion() const {
	return m_version;
}

PlayerSpawn & Map::getPlayerSpawn() {
	return m_playerSpawn;
}

const PlayerSpawn & Map::getPlayerSpawn() const {
	return m_playerSpawn;
}

void Map::setPlayerSpawn(const PlayerSpawn & playerSpawn) {
	m_playerSpawn = playerSpawn;
}

int32_t Map::getPlayerSpawnX() const {
	return m_playerSpawn.getX();
}

void Map::setPlayerSpawnX(int32_t x) {
	m_playerSpawn.setX(x);
}

int32_t Map::getPlayerSpawnY() const {
	return m_playerSpawn.getY();
}

void Map::setPlayerSpawnY(int32_t y) {
	m_playerSpawn.setY(y);
}

int32_t Map::getPlayerSpawnZ() const {
	return m_playerSpawn.getZ();
}

void Map::setPlayerSpawnZ(int32_t z) {
	m_playerSpawn.setZ(z);
}

const Point3D & Map::getPlayerSpawnPosition() const {
	return m_playerSpawn.getPosition();
}

void Map::setPlayerSpawnPosition(int32_t x, int32_t y, int32_t z) {
	m_playerSpawn.setPosition(x, y, z);
}

void Map::setPlayerSpawnPosition(int32_t position[3]) {
	m_playerSpawn.setPosition(position);
}

void Map::setPlayerSpawnPosition(const Point3D & position) {
	m_playerSpawn.setPosition(position);
}

int16_t Map::getPlayerSpawnAngle() const {
	return m_playerSpawn.getAngle();
}

bool Map::setPlayerSpawnAngle(int16_t angle) {
	return m_playerSpawn.setAngle(angle);
}

double Map::getPlayerSpawnAngleDegrees() const {
	return m_playerSpawn.getAngleDegrees();
}

bool Map::setPlayerSpawnAngleDegrees(double angleDegrees) {
	return m_playerSpawn.setAngleDegrees(angleDegrees);
}

double Map::getPlayerSpawnAngleRadians() const {
	return m_playerSpawn.getAngleRadians();
}

bool Map::setPlayerSpawnAngleRadians(double angleRadians) {
	return m_playerSpawn.setAngleRadians(angleRadians);
}

uint16_t Map::getPlayerSpawnSectorIndex() const {
	return m_playerSpawn.getSectorIndex();
}

void Map::setPlayerSpawnSectorIndex(uint16_t sectorIndex) {
	m_playerSpawn.setSectorIndex(sectorIndex);
}

size_t Map::numberOfSectors() const {
	return m_sectors.size();
}

bool Map::hasSector(const Sector & sector) const {
	return indexOfSector(sector) != std::numeric_limits<size_t>::max();
}

size_t Map::indexOfSector(const Sector & sector) const {
	std::vector<std::shared_ptr<Sector>>::const_iterator sectorIterator(std::find_if(m_sectors.cbegin(), m_sectors.cend(), [&sector](const std::shared_ptr<Sector> & currentSector) {
		return &sector == currentSector.get();
	}));

	if(sectorIterator == m_sectors.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return sectorIterator - m_sectors.cbegin();
}

std::shared_ptr<Sector> Map::getSector(size_t index) const {
	if(index >= m_sectors.size()) {
		return nullptr;
	}

	return m_sectors[index];
}

const std::vector<std::shared_ptr<Sector>> & Map::getSectors() const {
	return m_sectors;
}

size_t Map::numberOfWalls() const {
	return m_walls.size();
}

bool Map::hasWall(const Wall & wall) const {
	return indexOfWall(wall) != std::numeric_limits<size_t>::max();
}

size_t Map::indexOfWall(const Wall & wall) const {
	std::vector<std::shared_ptr<Wall>>::const_iterator wallIterator(std::find_if(m_walls.cbegin(), m_walls.cend(), [&wall](const std::shared_ptr<Wall> & currentWall) {
		return &wall == currentWall.get();
	}));

	if(wallIterator == m_walls.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return wallIterator - m_walls.cbegin();
}

std::shared_ptr<Wall> Map::getWall(size_t index) const {
	if(index >= m_walls.size()) {
		return nullptr;
	}

	return m_walls[index];
}

const std::vector<std::shared_ptr<Wall>> & Map::getWalls() const {
	return m_walls;
}

size_t Map::numberOfSprites() const {
	return m_sprites.size();
}

bool Map::hasSprite(const Sprite & sprite) const {
	return indexOfSprite(sprite) != std::numeric_limits<size_t>::max();
}

size_t Map::indexOfSprite(const Sprite & sprite) const {
	std::vector<std::shared_ptr<Sprite>>::const_iterator spriteIterator(std::find_if(m_sprites.cbegin(), m_sprites.cend(), [&sprite](const std::shared_ptr<Sprite> & currentSprite) {
		return &sprite == currentSprite.get();
	}));

	if(spriteIterator == m_sprites.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return spriteIterator - m_sprites.cbegin();
}

std::shared_ptr<Sprite> Map::getSprite(size_t index) const {
	if(index >= m_sprites.size()) {
		return nullptr;
	}

	return m_sprites[index];
}

const std::vector<std::shared_ptr<Sprite>> & Map::getSprites() const {
	return m_sprites;
}

std::shared_ptr<ByteBuffer> Map::getTrailingData() const {
	return m_trailingData;
}

void Map::setTrailingData(std::unique_ptr<ByteBuffer> trailingData) {
	if(trailingData == nullptr) {
		clearTrailingData();
		return;
	}

	m_trailingData = std::shared_ptr<ByteBuffer>(trailingData.release());
}

void Map::clearTrailingData() {
	m_trailingData->clear();
}

std::unique_ptr<Map> Map::readFrom(const ByteBuffer & byteBuffer) {
	bool error = false;

	byteBuffer.setEndianness(ENDIANNESS);

	uint32_t version = byteBuffer.readUnsignedInteger(&error);

	if(error) {
		spdlog::error("Failed to read map version.");
		return nullptr;
	}

	PlayerSpawn playerSpawn(PlayerSpawn::readFrom(byteBuffer, &error));

	if(error) {
		spdlog::error("Failed to read player spawn position from map data.");
		return nullptr;
	}

	uint16_t numberOfSectors = byteBuffer.readUnsignedShort(&error);

	if(error) {
		spdlog::error("Failed to read number of sectors from map data.");
		return nullptr;
	}

	std::vector<std::unique_ptr<Sector>> sectors;
	sectors.reserve(numberOfSectors);

	for(uint16_t i = 0; i < numberOfSectors; i++) {
		std::unique_ptr<Sector> sector(Sector::readFrom(byteBuffer, version));

		if(sector == nullptr) {
			spdlog::error("Failed to read sector #{} from map data.", sectors.size() + 1);
			return nullptr;
		}

		sectors.push_back(std::move(sector));
	}

	uint16_t numberOfWalls = byteBuffer.readUnsignedShort(&error);

	if(error) {
		spdlog::error("Failed to read number of walls from map data.");
		return nullptr;
	}

	std::vector<std::unique_ptr<Wall>> walls;
	walls.reserve(numberOfWalls);

	for(uint16_t i = 0; i < numberOfWalls; i++) {
		std::unique_ptr<Wall> wall(Wall::readFrom(byteBuffer, version));

		if(wall == nullptr) {
			spdlog::error("Failed to read wall #{} from map data.", walls.size() + 1);
			return nullptr;
		}

		walls.push_back(std::move(wall));
	}

	uint16_t numberOfSprites = byteBuffer.readUnsignedShort(&error);

	if(error) {
		spdlog::error("Failed to read number of sprites from map data.");
		return nullptr;
	}

	std::vector<std::unique_ptr<Sprite>> sprites;
	sprites.reserve(numberOfSprites);

	for(uint16_t i = 0; i < numberOfSprites; i++) {
		std::unique_ptr<Sprite> sprite(Sprite::readFrom(byteBuffer, version));

		if(sprite == nullptr) {
			spdlog::error("Failed to read sprite #{} from map data.", sprites.size() + 1);
			return nullptr;
		}

		sprites.push_back(std::move(sprite));
	}

	return std::make_unique<Map>(version, playerSpawn, std::move(sectors), std::move(walls), std::move(sprites), byteBuffer.getRemainingBytes());
}

bool Map::writeTo(ByteBuffer & byteBuffer) const {
	byteBuffer.setEndianness(ENDIANNESS);

	if(!byteBuffer.writeUnsignedInteger(m_version)) {
		return false;
	}

	if(!m_playerSpawn.writeTo(byteBuffer)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedShort(static_cast<uint16_t>(m_sectors.size()))) {
		return false;
	}

	for(const std::shared_ptr<Sector> & sector : m_sectors) {
		if(!sector->writeTo(byteBuffer, m_version)) {
			return false;
		}
	}

	if(!byteBuffer.writeUnsignedShort(static_cast<uint16_t>(m_walls.size()))) {
		return false;
	}

	for(const std::shared_ptr<Wall> & wall : m_walls) {
		if(!wall->writeTo(byteBuffer, m_version)) {
			return false;
		}
	}

	if(!byteBuffer.writeUnsignedShort(static_cast<uint16_t>(m_sprites.size()))) {
		return false;
	}

	for(const std::shared_ptr<Sprite> & sprite : m_sprites) {
		if(!sprite->writeTo(byteBuffer, m_version)) {
			return false;
		}
	}

	if(!m_trailingData->isEmpty()) {
		if(!byteBuffer.writeBytes(*m_trailingData)) {
			return false;
		}
	}

	return true;
}

std::unique_ptr<ByteBuffer> Map::getData() const {
	std::unique_ptr<ByteBuffer> mapData(std::make_unique<ByteBuffer>());
	mapData->reserve(getSizeInBytes());

	if(!writeTo(*mapData)) {
		return nullptr;
	}

	return mapData;
}

rapidjson::Document Map::toJSON() const {
	rapidjson::Document mapDocument(rapidjson::kObjectType);

	addToJSONObject(mapDocument, mapDocument.GetAllocator());

	return mapDocument;
}

rapidjson::Value Map::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value mapValue(rapidjson::kObjectType);

	addToJSONObject(mapValue, allocator);

	return mapValue;
}

bool Map::addToJSONObject(rapidjson::Value & value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	if(!value.IsObject()) {
		return false;
	}

	value.AddMember(rapidjson::StringRef(JSON_VERSION_PROPERTY_NAME.c_str()), rapidjson::Value(m_version), allocator);

	rapidjson::Value playerSpawnValue(m_playerSpawn.toJSON(allocator));
	value.AddMember(rapidjson::StringRef(JSON_PLAYER_SPAWN_PROPERTY_NAME.c_str()), playerSpawnValue, allocator);

	rapidjson::Value sectorsValue(rapidjson::kArrayType);

	for(const std::shared_ptr<Sector> & sector : m_sectors) {
		rapidjson::Value sectorValue(sector->toJSON(allocator));
		sectorsValue.PushBack(sectorValue, allocator);
	}

	value.AddMember(rapidjson::StringRef(JSON_SECTORS_PROPERTY_NAME.c_str()), sectorsValue, allocator);

	rapidjson::Value wallsValue(rapidjson::kArrayType);

	for(const std::shared_ptr<Wall> & wall : m_walls) {
		rapidjson::Value wallValue(wall->toJSON(allocator));
		wallsValue.PushBack(wallValue, allocator);
	}

	value.AddMember(rapidjson::StringRef(JSON_WALLS_PROPERTY_NAME.c_str()), wallsValue, allocator);

	rapidjson::Value spritesValue(rapidjson::kArrayType);

	for(const std::shared_ptr<Sprite> & sprite : m_sprites) {
		rapidjson::Value spriteValue(sprite->toJSON(allocator));
		spritesValue.PushBack(spriteValue, allocator);
	}

	value.AddMember(rapidjson::StringRef(JSON_SPRITES_PROPERTY_NAME.c_str()), spritesValue, allocator);

	if(!m_trailingData->isEmpty()) {
		rapidjson::Value trailingDataValue(rapidjson::kArrayType);

		for(size_t i = 0; i < m_trailingData->getSize(); i++) {
			trailingDataValue.PushBack(rapidjson::Value((*m_trailingData)[i]), allocator);
		}

		value.AddMember(rapidjson::StringRef(JSON_TRAILING_DATA_PROPERTY_NAME.c_str()), trailingDataValue, allocator);
	}

	return true;
}

std::unique_ptr<Map> Map::parseFrom(const rapidjson::Value & mapValue) {
	if(!mapValue.IsObject()) {
		spdlog::error("Invalid map type: '{}', expected 'object'.", Utilities::typeToString(mapValue.GetType()));
		return nullptr;
	}

	// parse version
	if(!mapValue.HasMember(JSON_VERSION_PROPERTY_NAME.c_str())) {
		spdlog::error("Map is missing '{}' property.", JSON_VERSION_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & versionValue = mapValue[JSON_VERSION_PROPERTY_NAME.c_str()];

	if(!versionValue.IsUint()) {
		spdlog::error("Map has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_VERSION_PROPERTY_NAME, Utilities::typeToString(versionValue.GetType()));
		return nullptr;
	}

	uint32_t version = versionValue.GetUint();

	// parse player spawn
	if(!mapValue.HasMember(JSON_PLAYER_SPAWN_PROPERTY_NAME.c_str())) {
		spdlog::error("Map is missing '{}' property.", JSON_PLAYER_SPAWN_PROPERTY_NAME);
		return nullptr;
	}

	std::optional<PlayerSpawn> optionalPlayerSpawn(PlayerSpawn::parseFrom(mapValue[JSON_PLAYER_SPAWN_PROPERTY_NAME.c_str()]));

	if(!optionalPlayerSpawn.has_value()) {
		spdlog::error("Failed to parse map '{}' property value.", JSON_PLAYER_SPAWN_PROPERTY_NAME);
		return nullptr;
	}

	// parse sectors
	if(!mapValue.HasMember(JSON_SECTORS_PROPERTY_NAME.c_str())) {
		spdlog::error("Map is missing '{}' property.", JSON_SECTORS_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & sectorsValue = mapValue[JSON_SECTORS_PROPERTY_NAME.c_str()];

	if(!sectorsValue.IsArray()) {
		spdlog::error("Map has an invalid '{}' property type: '{}', expected 'array'.", JSON_SECTORS_PROPERTY_NAME, Utilities::typeToString(sectorsValue.GetType()));
		return nullptr;
	}

	std::vector<std::unique_ptr<Sector>> sectors;

	for(rapidjson::Value::ConstValueIterator i = sectorsValue.Begin(); i != sectorsValue.End(); ++i) {
		std::unique_ptr<Sector> sector(Sector::parseFrom(*i));

		if(sector == nullptr) {
			spdlog::error("Failed to parse map sector #{}.", sectors.size() + 1);
			return nullptr;
		}

		sectors.push_back(std::move(sector));
	}

	// parse walls
	if(!mapValue.HasMember(JSON_WALLS_PROPERTY_NAME.c_str())) {
		spdlog::error("Map is missing '{}' property.", JSON_WALLS_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & wallsValue = mapValue[JSON_WALLS_PROPERTY_NAME.c_str()];

	if(!wallsValue.IsArray()) {
		spdlog::error("Map has an invalid '{}' property type: '{}', expected 'array'.", JSON_WALLS_PROPERTY_NAME, Utilities::typeToString(wallsValue.GetType()));
		return nullptr;
	}

	std::vector<std::unique_ptr<Wall>> walls;

	for(rapidjson::Value::ConstValueIterator i = wallsValue.Begin(); i != wallsValue.End(); ++i) {
		std::unique_ptr<Wall> wall(Wall::parseFrom(*i));

		if(wall == nullptr) {
			spdlog::error("Failed to parse map wall #{}.", walls.size() + 1);
			return nullptr;
		}

		walls.push_back(std::move(wall));
	}

	// parse sprites
	if(!mapValue.HasMember(JSON_SPRITES_PROPERTY_NAME.c_str())) {
		spdlog::error("Map is missing '{}' property.", JSON_SPRITES_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & spritesValue = mapValue[JSON_SPRITES_PROPERTY_NAME.c_str()];

	if(!spritesValue.IsArray()) {
		spdlog::error("Map has an invalid '{}' property type: '{}', expected 'array'.", JSON_SPRITES_PROPERTY_NAME, Utilities::typeToString(spritesValue.GetType()));
		return nullptr;
	}

	std::vector<std::unique_ptr<Sprite>> sprites;

	for(rapidjson::Value::ConstValueIterator i = spritesValue.Begin(); i != spritesValue.End(); ++i) {
		std::unique_ptr<Sprite> sprite(Sprite::parseFrom(*i));

		if(sprite == nullptr) {
			spdlog::error("Failed to parse map sprite #{}.", sprites.size() + 1);
			return nullptr;
		}

		sprites.push_back(std::move(sprite));
	}

	// parse trailing data
	std::unique_ptr<ByteBuffer> trailingData;

	if(mapValue.HasMember(JSON_TRAILING_DATA_PROPERTY_NAME.c_str())) {
		const rapidjson::Value & trailingDataValue = mapValue[JSON_TRAILING_DATA_PROPERTY_NAME.c_str()];

		if(!trailingDataValue.IsArray()) {
			spdlog::error("Map has an invalid '{}' property type: '{}', expected 'array'.", JSON_TRAILING_DATA_PROPERTY_NAME, Utilities::typeToString(trailingDataValue.GetType()));
			return nullptr;
		}

		trailingData = std::make_unique<ByteBuffer>();
		trailingData->reserve(trailingDataValue.Size());

		for(rapidjson::Value::ConstValueIterator i = trailingDataValue.Begin(); i != trailingDataValue.End(); ++i) {
			const rapidjson::Value & trailingByteValue = *i;

			if(!trailingByteValue.IsUint()) {
				spdlog::error("Map '{}' property byte #{} has an invalid type: '{}', expected unsigned byte 'number'.", JSON_TRAILING_DATA_PROPERTY_NAME, trailingData->getSize(), Utilities::typeToString(trailingByteValue.GetType()));
				return nullptr;
			}

			uint32_t trailingByte = trailingByteValue.GetUint();

			if(trailingByte > std::numeric_limits<uint8_t>::max()) {
				spdlog::error("Map '{}' property byte #{} has an invalid value: {}, expected unsigned byte 'number' between 0 and {}, inclusively.", JSON_TRAILING_DATA_PROPERTY_NAME, trailingData->getSize(), trailingByte, std::numeric_limits<uint8_t>::max());
				return nullptr;
			}

			trailingData->writeByte(static_cast<uint8_t>(trailingByte));
		}
	}

	return std::make_unique<Map>(version, optionalPlayerSpawn.value(), std::move(sectors), std::move(walls), std::move(sprites), std::move(trailingData));
}

std::unique_ptr<Map> Map::loadFrom(const std::string & filePath) {
	std::string fileExtension(Utilities::getFileExtension(filePath));

	if(Utilities::areStringsEqualIgnoreCase(fileExtension, "json")) {
		return loadFromJSON(filePath);
	}
	else {
		return loadFromMap(filePath);
	}
}

std::unique_ptr<Map> Map::loadFromMap(const std::string & filePath) {
	std::unique_ptr<ByteBuffer> mapData(ByteBuffer::readFrom(filePath, ENDIANNESS));

	if(mapData == nullptr) {
		spdlog::error("Failed to read map binary data from file: '{}'.", filePath);
		return nullptr;
	}

	std::unique_ptr<Map> map(Map::readFrom(*mapData));

	if(map == nullptr) {
		spdlog::error("Failed to parse map binary data from file contents: '{}'.", filePath);
		return nullptr;
	}

	return map;
}

std::unique_ptr<Map> Map::loadFromJSON(const std::string & filePath) {
	std::optional<rapidjson::Document> optionalMapDocument(Utilities::loadJSONDocumentFrom(filePath));

	if(!optionalMapDocument.has_value()) {
		return nullptr;
	}

	std::unique_ptr<Map> map(parseFrom(optionalMapDocument.value()));

	if(map == nullptr) {
		spdlog::error("Failed to parse map from JSON file: '{}'.", filePath);
		return nullptr;
	}

	return map;
}

bool Map::saveTo(const std::string & filePath, bool overwrite) const {
	std::string fileExtension(Utilities::getFileExtension(filePath));

	if(Utilities::areStringsEqualIgnoreCase(fileExtension, "json")) {
		return saveToJSON(filePath, overwrite);
	}
	else {
		return saveToMap(filePath, overwrite);
	}
}

bool Map::saveToMap(const std::string & filePath, bool overwrite) const {
	if(filePath.empty()) {
		return false;
	}

	if(!overwrite && std::filesystem::exists(std::filesystem::path(filePath))) {
		spdlog::warn("File '{}' already exists, use overwrite to force write.", filePath);
		return false;
	}

	std::unique_ptr<ByteBuffer> mapData(getData());

	if(mapData == nullptr) {
		spdlog::error("Failed to serialize map data.");
		return false;
	}

	return mapData->writeTo(filePath, overwrite);
}

bool Map::saveToJSON(const std::string & filePath, bool overwrite) const {
	return Utilities::saveJSONValueTo(toJSON(), filePath, overwrite);
}

size_t Map::getSizeInBytes() const {
	return PlayerSpawn::SIZE_BYTES + (Sector::getSizeInBytes(m_version) * m_sectors.size()) + (Wall::SIZE_BYTES * m_walls.size()) + (Sprite::SIZE_BYTES * m_sprites.size()) + sizeof(int32_t) + (sizeof(uint16_t) * 3) + m_trailingData->getSize();
}

bool Map::operator == (const Map & map) const {
	if(m_version != map.m_version ||
	   m_playerSpawn != map.m_playerSpawn ||
	   *m_trailingData != *map.m_trailingData ||
	   m_sectors.size() != map.m_sectors.size() ||
	   m_walls.size() != map.m_walls.size() ||
	   m_sprites.size() != map.m_sprites.size()) {
		return false;
	}

	for(size_t i = 0; i < m_sectors.size(); i++) {
		if(*m_sectors[i] != *map.m_sectors[i]) {
			return false;
		}
	}

	for(size_t i = 0; i < m_walls.size(); i++) {
		if(*m_walls[i] != *map.m_walls[i]) {
			return false;
		}
	}

	for(size_t i = 0; i < m_sprites.size(); i++) {
		if(*m_sprites[i] != *map.m_sprites[i]) {
			return false;
		}
	}

	return true;
}

bool Map::operator != (const Map & map) const {
	return !operator == (map);
}
