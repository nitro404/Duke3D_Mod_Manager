#include "Sector.h"

#include <ByteBuffer.h>
#include <Utilities/RapidJSONUtilities.h>

#include <spdlog/spdlog.h>

#include <string>

static const std::string JSON_FIRST_WALL_INDEX_PROPERTY_NAME("firstWallIndex");
static const std::string JSON_NUMBER_OF_WALLS_PROPERTY_NAME("numberOfWalls");
static const std::string JSON_CEILING_PROPERTY_NAME("ceiling");
static const std::string JSON_FLOOR_PROPERTY_NAME("floor");
static const std::string JSON_VISIBILITY_PROPERTY_NAME("visibility");
static const std::string JSON_FILLER_PROPERTY_NAME("filler");
static const std::string JSON_TRAILING_DATA_PROPERTY_NAME("trailingData");

Sector::Sector()
	: TaggedItem(0, 0, 0)
	, m_firstWallIndex(0)
	, m_numberOfWalls(0)
	, m_visibility(0)
	, m_filler(0) { }

Sector::Sector(uint16_t firstWallIndex, uint16_t numberOfWalls, int32_t ceilingHeight, Partition::Attributes ceilingAttributes, int16_t ceilingTileNumber, int16_t ceilingSlope, int8_t ceilingShade, uint8_t ceilingPaletteLookupTableNumber, uint8_t ceilingXPanning, uint8_t ceilingYPanning, int32_t floorHeight, Partition::Attributes floorAttributes, int16_t floorTileNumber, int16_t floorSlope, int8_t floorShade, uint8_t floorPaletteLookupTableNumber, uint8_t floorXPanning, uint8_t floorYPanning, uint8_t visibility, TaggedItem taggedItem, uint8_t filler, const std::vector<uint8_t> & trailingData)
	: TaggedItem(taggedItem)
	, m_firstWallIndex(firstWallIndex)
	, m_numberOfWalls(numberOfWalls)
	, m_ceiling(std::make_shared<Partition>(Partition::Type::Ceiling, ceilingHeight, ceilingAttributes, ceilingTileNumber, ceilingSlope, ceilingShade, ceilingPaletteLookupTableNumber, ceilingXPanning, ceilingYPanning))
	, m_floor(std::make_shared<Partition>(Partition::Type::Floor, floorHeight, floorAttributes, floorTileNumber, floorSlope, floorShade, floorPaletteLookupTableNumber, floorXPanning, floorYPanning))
	, m_visibility(visibility)
	, m_filler(filler)
	, m_trailingData(trailingData) { }

Sector::Sector(uint16_t firstWallIndex, uint16_t numberOfWalls, int32_t ceilingHeight, Partition::Attributes ceilingAttributes, int16_t ceilingTileNumber, int16_t ceilingSlope, int8_t ceilingShade, uint8_t ceilingPaletteLookupTableNumber, uint8_t ceilingXPanning, uint8_t ceilingYPanning, int32_t floorHeight, Partition::Attributes floorAttributes, int16_t floorTileNumber, int16_t floorSlope, int8_t floorShade, uint8_t floorPaletteLookupTableNumber, uint8_t floorXPanning, uint8_t floorYPanning, uint8_t visibility, uint16_t lowTag, uint16_t highTag, uint16_t extra, uint8_t filler, const std::vector<uint8_t> & trailingData)
	: TaggedItem(lowTag, highTag, extra)
	, m_firstWallIndex(firstWallIndex)
	, m_numberOfWalls(numberOfWalls)
	, m_ceiling(std::make_shared<Partition>(Partition::Type::Ceiling, ceilingHeight, ceilingAttributes, ceilingTileNumber, ceilingSlope, ceilingShade, ceilingPaletteLookupTableNumber, ceilingXPanning, ceilingYPanning))
	, m_floor(std::make_shared<Partition>(Partition::Type::Floor, floorHeight, floorAttributes, floorTileNumber, floorSlope, floorShade, floorPaletteLookupTableNumber, floorXPanning, floorYPanning))
	, m_visibility(visibility)
	, m_filler(filler)
	, m_trailingData(trailingData) { }

Sector::Sector(uint16_t firstWallIndex, uint16_t numberOfWalls, int32_t ceilingHeight, Partition::Attributes ceilingAttributes, TexturedItem texturedCeilingItem, int16_t ceilingSlope, uint8_t ceilingXPanning, uint8_t ceilingYPanning, int32_t floorHeight, Partition::Attributes floorAttributes, TexturedItem texturedFloorItem, int16_t floorSlope, uint8_t floorXPanning, uint8_t floorYPanning, uint8_t visibility, TaggedItem taggedItem, uint8_t filler, const std::vector<uint8_t> & trailingData)
	: TaggedItem(taggedItem)
	, m_firstWallIndex(firstWallIndex)
	, m_numberOfWalls(numberOfWalls)
	, m_ceiling(std::make_shared<Partition>(Partition::Type::Ceiling, ceilingHeight, ceilingAttributes, texturedCeilingItem, ceilingSlope, ceilingXPanning, ceilingYPanning))
	, m_floor(std::make_shared<Partition>(Partition::Type::Floor, floorHeight, floorAttributes, texturedFloorItem, floorSlope, floorXPanning, floorYPanning))
	, m_visibility(visibility)
	, m_filler(filler)
	, m_trailingData(trailingData) { }

Sector::Sector(uint16_t firstWallIndex, uint16_t numberOfWalls, int32_t ceilingHeight, Partition::Attributes ceilingAttributes, TexturedItem texturedCeilingItem, int16_t ceilingSlope, uint8_t ceilingXPanning, uint8_t ceilingYPanning, int32_t floorHeight, Partition::Attributes floorAttributes, TexturedItem texturedFloorItem, int16_t floorSlope, uint8_t floorXPanning, uint8_t floorYPanning, uint8_t visibility, uint16_t lowTag, uint16_t highTag, uint16_t extra, uint8_t filler, const std::vector<uint8_t> & trailingData)
	: TaggedItem(lowTag, highTag, extra)
	, m_firstWallIndex(firstWallIndex)
	, m_numberOfWalls(numberOfWalls)
	, m_ceiling(std::make_shared<Partition>(Partition::Type::Ceiling, ceilingHeight, ceilingAttributes, texturedCeilingItem, ceilingSlope, ceilingXPanning, ceilingYPanning))
	, m_floor(std::make_shared<Partition>(Partition::Type::Floor, floorHeight, floorAttributes, texturedFloorItem, floorSlope, floorXPanning, floorYPanning))
	, m_visibility(visibility)
	, m_filler(filler)
	, m_trailingData(trailingData) { }

Sector::Sector(uint16_t firstWallIndex, uint16_t numberOfWalls, std::unique_ptr<Partition> ceiling, std::unique_ptr<Partition> floor, uint8_t visibility, TaggedItem taggedItem, uint8_t filler, const std::vector<uint8_t> & trailingData)
	: TaggedItem(taggedItem)
	, m_firstWallIndex(firstWallIndex)
	, m_numberOfWalls(numberOfWalls)
	, m_ceiling(std::move(ceiling))
	, m_floor(std::move(floor))
	, m_visibility(visibility)
	, m_filler(filler)
	, m_trailingData(trailingData) {
	m_ceiling->setType(Partition::Type::Ceiling);
	m_floor->setType(Partition::Type::Floor);
}

Sector::Sector(uint16_t firstWallIndex, uint16_t numberOfWalls, std::unique_ptr<Partition> ceiling, std::unique_ptr<Partition> floor, uint8_t visibility, uint16_t lowTag, uint16_t highTag, uint16_t extra, uint8_t filler, const std::vector<uint8_t> & trailingData)
	: TaggedItem(lowTag, highTag, extra)
	, m_firstWallIndex(firstWallIndex)
	, m_numberOfWalls(numberOfWalls)
	, m_ceiling(std::move(ceiling))
	, m_floor(std::move(floor))
	, m_visibility(visibility)
	, m_filler(filler)
	, m_trailingData(trailingData) {
	m_ceiling->setType(Partition::Type::Ceiling);
	m_floor->setType(Partition::Type::Floor);
}

Sector::Sector(Sector && sector) noexcept
	: TaggedItem(std::move(sector))
	, m_firstWallIndex(sector.m_firstWallIndex)
	, m_numberOfWalls(sector.m_numberOfWalls)
	, m_ceiling(std::move(sector.m_ceiling))
	, m_floor(std::move(sector.m_floor))
	, m_visibility(sector.m_visibility)
	, m_filler(sector.m_filler)
	, m_trailingData(std::move(sector.m_trailingData)) { }

Sector::Sector(const Sector & sector)
	: TaggedItem(sector)
	, m_firstWallIndex(sector.m_firstWallIndex)
	, m_numberOfWalls(sector.m_numberOfWalls)
	, m_ceiling(std::make_shared<Partition>(*sector.m_ceiling))
	, m_floor(std::make_shared<Partition>(*sector.m_floor))
	, m_visibility(sector.m_visibility)
	, m_filler(sector.m_filler)
	, m_trailingData(sector.m_trailingData) { }

Sector & Sector::operator = (Sector && sector) noexcept {
	if(this != &sector) {
		TaggedItem::operator = (std::move(sector));

		m_firstWallIndex = sector.m_firstWallIndex;
		m_numberOfWalls = sector.m_numberOfWalls;
		m_ceiling = std::move(sector.m_ceiling);
		m_floor = std::move(sector.m_floor);
		m_visibility = sector.m_visibility;
		m_filler = sector.m_filler;
		m_trailingData = std::move(sector.m_trailingData);
	}

	return *this;
}

Sector & Sector::operator = (const Sector & sector) {
	TaggedItem::operator = (sector);

	m_firstWallIndex = sector.m_firstWallIndex;
	m_numberOfWalls = sector.m_numberOfWalls;
	m_ceiling = std::make_shared<Partition>(*sector.m_ceiling);
	m_floor = std::make_shared<Partition>(*sector.m_floor);
	m_visibility = sector.m_visibility;
	m_filler = sector.m_filler;
	m_trailingData = sector.m_trailingData;

	return *this;
}

Sector::~Sector() { }

bool Sector::isCeilingParallaxed() const {
	return m_ceiling->isParallaxed();
}

void Sector::setCeilingParallaxed(bool parallaxed) {
	m_ceiling->setParallaxed(parallaxed);
}

bool Sector::isCeilingSloped() const {
	return m_ceiling->isSloped();
}

void Sector::setCeilingSloped(bool sloped) {
	m_ceiling->setSloped(sloped);
}

bool Sector::isCeilingTextureSwappedXY() const {
	return m_ceiling->isTextureSwappedXY();
}

void Sector::setCeilingTextureSwappedXY(bool swapTextureXY) {
	m_ceiling->setTextureSwappedXY(swapTextureXY);
}

bool Sector::isCeilingSmooshinessDoubled() const {
	return m_ceiling->isSmooshinessDoubled();
}

void Sector::setCeilingSmooshinessDoubled(bool doubleSmooshiness) {
	m_ceiling->setSmooshinessDoubled(doubleSmooshiness);
}

bool Sector::isCeilingXFlipped() const {
	return m_ceiling->isXFlipped();
}

void Sector::setCeilingXFlipped(bool xFlipped) {
	m_ceiling->setXFlipped(xFlipped);
}

bool Sector::isCeilingYFlipped() const {
	return m_ceiling->isYFlipped();
}

void Sector::setCeilingYFlipped(bool yFlipped) {
	m_ceiling->setYFlipped(yFlipped);
}

bool Sector::isCeilingTextureAligned() const {
	return m_ceiling->isTextureAligned();
}

void Sector::setCeilingTextureAligned(bool textureAlign) {
	m_ceiling->setTextureAligned(textureAlign);
}

uint16_t Sector::getCeilingReserved() const {
	return m_ceiling->getReserved();
}

bool Sector::setCeilingReserved(uint16_t reserved) {
	return m_ceiling->setReserved(reserved);
}

Partition::Attributes Sector::getCeilingAttributes() const {
	return m_ceiling->getAttributes();
}

void Sector::setCeilingAttributes(Partition::Attributes attributes) {
	m_ceiling->setAttributes(attributes);
}

int32_t Sector::getCeilingHeight() const {
	return m_ceiling->getHeight();
}

void Sector::setCeilingHeight(int32_t height) {
	m_ceiling->setHeight(height);
}

int16_t Sector::getCeilingSlope() const {
	return m_ceiling->getSlope();
}

void Sector::setCeilingSlope(int16_t slope) {
	m_ceiling->setSlope(slope);
}

uint8_t Sector::getCeilingXPanning() const {
	return m_ceiling->getXPanning();
}

void Sector::setCeilingXPanning(uint8_t xPanning) {
	m_ceiling->setXPanning(xPanning);
}

uint8_t Sector::getCeilingYPanning() const {
	return m_ceiling->getYPanning();
}

void Sector::setCeilingYPanning(uint8_t yPanning) {
	m_ceiling->setYPanning(yPanning);
}

std::shared_ptr<Partition> Sector::getCeiling() const {
	return m_ceiling;
}

void Sector::setCeiling(const Partition & ceiling) {
	std::shared_ptr<Partition> newCeiling(std::make_shared<Partition>(ceiling));
	newCeiling->setType(Partition::Type::Ceiling);

	m_ceiling = std::move(newCeiling);
}

void Sector::setCeiling(std::unique_ptr<Partition> ceiling) {
	if(ceiling == nullptr) {
		return;
	}

	ceiling->setType(Partition::Type::Ceiling);

	m_ceiling = std::move(ceiling);
}

bool Sector::isFloorParallaxed() const {
	return m_floor->isParallaxed();
}

void Sector::setFloorParallaxed(bool parallaxed) {
	m_floor->setParallaxed(parallaxed);
}

bool Sector::isFloorSloped() const {
	return m_floor->isSloped();
}

void Sector::setFloorSloped(bool sloped) {
	m_floor->setSloped(sloped);
}

bool Sector::isFloorTextureSwappedXY() const {
	return m_floor->isTextureSwappedXY();
}

void Sector::setFloorTextureSwappedXY(bool swapTextureXY) {
	m_floor->setTextureSwappedXY(swapTextureXY);
}

bool Sector::isFloorSmooshinessDoubled() const {
	return m_floor->isSmooshinessDoubled();
}

void Sector::setFloorSmooshinessDoubled(bool doubleSmooshiness) {
	m_floor->setSmooshinessDoubled(doubleSmooshiness);
}

bool Sector::isFloorXFlipped() const {
	return m_floor->isXFlipped();
}

void Sector::setFloorXFlipped(bool xFlipped) {
	m_floor->setXFlipped(xFlipped);
}

bool Sector::isFloorYFlipped() const {
	return m_floor->isYFlipped();
}

void Sector::setFloorYFlipped(bool yFlipped) {
	m_floor->setYFlipped(yFlipped);
}

bool Sector::isFloorTextureAligned() const {
	return m_floor->isTextureAligned();
}

void Sector::setFloorTextureAligned(bool textureAlign) {
	m_floor->setTextureAligned(textureAlign);
}

uint16_t Sector::getFloorReserved() const {
	return m_floor->getReserved();
}

bool Sector::setFloorReserved(uint16_t reserved) {
	return m_floor->setReserved(reserved);
}

Partition::Attributes Sector::getFloorAttributes() const {
	return m_floor->getAttributes();
}

void Sector::setFloorAttributes(Partition::Attributes attributes) {
	m_floor->setAttributes(attributes);
}

int32_t Sector::getFloorHeight() const {
	return m_floor->getHeight();
}

void Sector::setFloorHeight(int32_t height) {
	m_floor->setHeight(height);
}

int16_t Sector::getFloorSlope() const {
	return m_floor->getSlope();
}

void Sector::setFloorSlope(int16_t slope) {
	m_floor->setSlope(slope);
}

uint8_t Sector::getFloorXPanning() const {
	return m_floor->getXPanning();
}

void Sector::setFloorXPanning(uint8_t xPanning) {
	m_floor->setXPanning(xPanning);
}

uint8_t Sector::getFloorYPanning() const {
	return m_floor->getYPanning();
}

void Sector::setFloorYPanning(uint8_t yPanning) {
	m_floor->setYPanning(yPanning);
}

std::shared_ptr<Partition> Sector::getFloor() const {
	return m_floor;
}

void Sector::setFloor(const Partition & floor) {
	std::shared_ptr<Partition> newFloor(std::make_shared<Partition>(floor));
	newFloor->setType(Partition::Type::Floor);

	m_floor = std::move(newFloor);
}

void Sector::setFloor(std::unique_ptr<Partition> floor) {
	if(floor == nullptr) {
		return;
	}

	floor->setType(Partition::Type::Floor);

	m_floor = std::move(floor);
}

uint16_t Sector::getFirstWallIndex() const {
	return m_firstWallIndex;
}

void Sector::setFirstWallIndex(uint16_t firstWallIndex) {
	m_firstWallIndex = firstWallIndex;
}

uint16_t Sector::getNumberOfWalls() const {
	return m_numberOfWalls;
}

void Sector::setNumberOfWalls(uint16_t numberOfWalls) {
	m_numberOfWalls = numberOfWalls;
}

uint8_t Sector::getVisibility() const {
	return m_visibility;
}

void Sector::setVisibility(uint8_t visibility) {
	m_visibility = visibility;
}

uint8_t Sector::getFiller() const {
	return m_filler;
}

void Sector::setFiller(uint8_t filler) {
	m_filler = filler;
}

const std::vector<uint8_t> & Sector::getTrailingData() const {
	return m_trailingData;
}

bool Sector::setTrailingData(const std::vector<uint8_t> & trailingData) {
	if(trailingData.size() != 3) {
		return false;
	}

	m_trailingData = trailingData;

	return true;
}

void Sector::clearTrailingData() {
	m_trailingData.clear();
}

std::unique_ptr<Sector> Sector::getFrom(const ByteBuffer & byteBuffer, size_t offset, uint32_t mapVersion) {
	int16_t ceilingTileNumber = 0;
	int16_t ceilingSlope = 0;
	int8_t ceilingShade = 0;
	uint8_t ceilingXPanning = 0;
	uint8_t ceilingYPanning = 0;
	uint16_t rawCeilingAttributes = 0;
	uint8_t ceilingPaletteLookupTableNumber = 0;
	int16_t floorTileNumber = 0;
	int16_t floorSlope = 0;
	int8_t floorShade = 0;
	uint8_t floorXPanning = 0;
	uint8_t floorYPanning = 0;
	uint16_t rawFloorAttributes = 0;
	uint8_t floorPaletteLookupTableNumber = 0;
	uint8_t filler = 0;
	std::unique_ptr<std::vector<uint8_t>> trailingData;
	bool error = false;
	size_t newOffset = offset;

	uint16_t firstWallIndex = byteBuffer.getUnsignedShort(newOffset, &error);

	if(error) {
		return nullptr;
	}

	newOffset += sizeof(uint16_t);

	uint16_t numberOfWalls = byteBuffer.getUnsignedShort(newOffset, &error);

	if(error) {
		return nullptr;
	}

	newOffset += sizeof(uint16_t);

	if(mapVersion == 6) {
		ceilingTileNumber = byteBuffer.getShort(newOffset, &error);

		if(error || ceilingTileNumber < 0) {
			return nullptr;
		}

		newOffset += sizeof(int16_t);

		floorTileNumber = byteBuffer.getShort(newOffset, &error);

		if(error || floorTileNumber < 0) {
			return nullptr;
		}

		newOffset += sizeof(int16_t);

		ceilingSlope = byteBuffer.getShort(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(int16_t);

		floorSlope = byteBuffer.getShort(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(int16_t);
	}

	int32_t ceilingHeight = byteBuffer.getInteger(newOffset, &error);

	if(error) {
		return nullptr;
	}

	newOffset += sizeof(int32_t);

	int32_t floorHeight = byteBuffer.getInteger(newOffset, &error);

	if(error) {
		return nullptr;
	}

	newOffset += sizeof(int32_t);

	if(mapVersion == 6) {
		ceilingShade = byteBuffer.getByte(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(int8_t);

		floorShade = byteBuffer.getByte(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(int8_t);

		ceilingXPanning = byteBuffer.getUnsignedByte(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint8_t);

		floorXPanning = byteBuffer.getUnsignedByte(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint8_t);

		ceilingYPanning = byteBuffer.getUnsignedByte(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint8_t);

		floorYPanning = byteBuffer.getUnsignedByte(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint8_t);
	}

	if(mapVersion == 6) {
		rawCeilingAttributes = byteBuffer.getUnsignedByte(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint8_t);

		rawFloorAttributes = byteBuffer.getUnsignedByte(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint8_t);
	}
	else {
		rawCeilingAttributes = byteBuffer.getUnsignedShort(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint16_t);

		rawFloorAttributes = byteBuffer.getUnsignedShort(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint16_t);

		ceilingTileNumber = byteBuffer.getShort(newOffset, &error);

		if(error || ceilingTileNumber < 0) {
			return nullptr;
		}

		newOffset += sizeof(int16_t);

		ceilingSlope = byteBuffer.getShort(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(int16_t);

		ceilingShade = byteBuffer.getByte(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(int8_t);

		ceilingPaletteLookupTableNumber = byteBuffer.getUnsignedByte(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint8_t);

		ceilingXPanning = byteBuffer.getUnsignedByte(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint8_t);

		ceilingYPanning = byteBuffer.getUnsignedByte(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint8_t);

		floorTileNumber = byteBuffer.getShort(newOffset, &error);

		if(error || floorTileNumber < 0) {
			return nullptr;
		}

		newOffset += sizeof(int16_t);

		floorSlope = byteBuffer.getShort(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(int16_t);

		floorShade = byteBuffer.getByte(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(int8_t);

		floorPaletteLookupTableNumber = byteBuffer.getUnsignedByte(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint8_t);

		floorXPanning = byteBuffer.getUnsignedByte(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint8_t);

		floorYPanning = byteBuffer.getUnsignedByte(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint8_t);
	}

	if(mapVersion == 6) {
		ceilingPaletteLookupTableNumber = byteBuffer.getUnsignedByte(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint8_t);

		floorPaletteLookupTableNumber = byteBuffer.getUnsignedByte(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint8_t);
	}

	uint8_t visibility = byteBuffer.getUnsignedByte(newOffset, &error);

	if(error) {
		return nullptr;
	}

	newOffset += sizeof(uint8_t);

	if(mapVersion != 6) {
		filler = byteBuffer.getUnsignedByte(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint8_t);
	}

	TaggedItem taggedItem(TaggedItem::getFrom(byteBuffer, newOffset, &error));

	if(error) {
		return nullptr;
	}

	newOffset += TaggedItem::SIZE_BYTES;

	if(mapVersion == 6) {
		trailingData = byteBuffer.getBytes(3, newOffset);

		if(trailingData == nullptr) {
			return nullptr;
		}
	}
	else {
		trailingData = std::make_unique<std::vector<uint8_t>>();
	}

	Partition::Attributes ceilingAttributes;
	ceilingAttributes.rawValue = rawCeilingAttributes;

	Partition::Attributes floorAttributes;
	floorAttributes.rawValue = rawFloorAttributes;

	return std::make_unique<Sector>(firstWallIndex, numberOfWalls, ceilingHeight, ceilingAttributes, ceilingTileNumber, ceilingSlope, ceilingShade, ceilingPaletteLookupTableNumber, ceilingXPanning, ceilingYPanning, floorHeight, floorAttributes, floorTileNumber, floorSlope, floorShade, floorPaletteLookupTableNumber, floorXPanning, floorYPanning, visibility, taggedItem, filler, *trailingData);
}

std::unique_ptr<Sector> Sector::readFrom(const ByteBuffer & byteBuffer, uint32_t mapVersion) {
	std::unique_ptr<Sector> value(getFrom(byteBuffer, byteBuffer.getReadOffset(), mapVersion));

	if(value != nullptr) {
		byteBuffer.skipReadBytes(getSizeInBytes(mapVersion));
	}

	return value;
}

bool Sector::putIn(ByteBuffer & byteBuffer, size_t offset, uint32_t mapVersion) const {
	if(m_ceiling == nullptr || m_floor == nullptr) {
		return false;
	}

	size_t newOffset = offset;

	if(!byteBuffer.putUnsignedShort(m_firstWallIndex, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(!byteBuffer.putUnsignedShort(m_numberOfWalls, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(mapVersion == 6) {
		if(!byteBuffer.putShort(m_ceiling->getTileNumber(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);

		if(!byteBuffer.putShort(m_floor->getTileNumber(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);

		if(!byteBuffer.putShort(m_ceiling->getSlope(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);

		if(!byteBuffer.putShort(m_floor->getSlope(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);
	}

	if(!byteBuffer.putInteger(m_ceiling->getHeight(), newOffset)) {
		return false;
	}

	newOffset += sizeof(int32_t);

	if(!byteBuffer.putInteger(m_floor->getHeight(), newOffset)) {
		return false;
	}

	newOffset += sizeof(int32_t);

	if(mapVersion == 6) {
		if(!byteBuffer.putByte(m_ceiling->getShade(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int8_t);

		if(!byteBuffer.putByte(m_floor->getShade(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int8_t);

		if(!byteBuffer.putUnsignedByte(m_ceiling->getXPanning(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);

		if(!byteBuffer.putUnsignedByte(m_floor->getXPanning(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);

		if(!byteBuffer.putUnsignedByte(m_ceiling->getYPanning(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);

		if(!byteBuffer.putUnsignedByte(m_floor->getYPanning(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);
	}

	if(mapVersion == 6) {
		if(!byteBuffer.putUnsignedByte(static_cast<uint8_t>(m_ceiling->getAttributes().rawValue), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);

		if(!byteBuffer.putUnsignedByte(static_cast<uint8_t>(m_floor->getAttributes().rawValue), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);
	}
	else {
		if(!byteBuffer.putUnsignedShort(m_ceiling->getAttributes().rawValue, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint16_t);

		if(!byteBuffer.putUnsignedShort(m_floor->getAttributes().rawValue, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint16_t);

		if(!byteBuffer.putShort(m_ceiling->getTileNumber(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);

		if(!byteBuffer.putShort(m_ceiling->getSlope(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);

		if(!byteBuffer.putByte(m_ceiling->getShade(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int8_t);

		if(!byteBuffer.putUnsignedByte(m_ceiling->getPaletteLookupTableNumber(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);

		if(!byteBuffer.putUnsignedByte(m_ceiling->getXPanning(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);

		if(!byteBuffer.putUnsignedByte(m_ceiling->getYPanning(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);

		if(!byteBuffer.putShort(m_floor->getTileNumber(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);

		if(!byteBuffer.putShort(m_floor->getSlope(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);

		if(!byteBuffer.putByte(m_floor->getShade(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int8_t);

		if(!byteBuffer.putUnsignedByte(m_floor->getPaletteLookupTableNumber(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);

		if(!byteBuffer.putUnsignedByte(m_floor->getXPanning(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);

		if(!byteBuffer.putUnsignedByte(m_floor->getYPanning(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);
	}

	if(mapVersion == 6) {
		if(!byteBuffer.putUnsignedByte(m_ceiling->getPaletteLookupTableNumber(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);

		if(!byteBuffer.putUnsignedByte(m_floor->getPaletteLookupTableNumber(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);
	}

	if(!byteBuffer.putUnsignedByte(m_visibility, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(mapVersion != 6) {
		if(!byteBuffer.putUnsignedByte(m_filler, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);
	}

	if(!TaggedItem::putIn(byteBuffer, newOffset)) {
		return false;
	}

	newOffset += TaggedItem::SIZE_BYTES;

	if(mapVersion == 6) {
		if(!byteBuffer.putBytes(m_trailingData, newOffset)) {
			return false;
		}
	}

	return true;
}

bool Sector::insertIn(ByteBuffer & byteBuffer, size_t offset, uint32_t mapVersion) const {
	if(m_ceiling == nullptr || m_floor == nullptr) {
		return false;
	}

	size_t newOffset = offset;

	if(!byteBuffer.insertUnsignedShort(m_firstWallIndex, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(!byteBuffer.insertUnsignedShort(m_numberOfWalls, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(mapVersion == 6) {
		if(!byteBuffer.insertShort(m_ceiling->getTileNumber(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);

		if(!byteBuffer.insertShort(m_floor->getTileNumber(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);

		if(!byteBuffer.insertShort(m_ceiling->getSlope(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);

		if(!byteBuffer.insertShort(m_floor->getSlope(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);
	}

	if(!byteBuffer.insertInteger(m_ceiling->getHeight(), newOffset)) {
		return false;
	}

	newOffset += sizeof(int32_t);

	if(!byteBuffer.insertInteger(m_floor->getHeight(), newOffset)) {
		return false;
	}

	newOffset += sizeof(int32_t);

	if(mapVersion == 6) {
		if(!byteBuffer.insertByte(m_ceiling->getShade(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int8_t);

		if(!byteBuffer.insertByte(m_floor->getShade(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int8_t);

		if(!byteBuffer.insertUnsignedByte(m_ceiling->getXPanning(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);

		if(!byteBuffer.insertUnsignedByte(m_floor->getXPanning(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);

		if(!byteBuffer.insertUnsignedByte(m_ceiling->getYPanning(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);

		if(!byteBuffer.insertUnsignedByte(m_floor->getYPanning(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);
	}

	if(mapVersion == 6) {
		if(!byteBuffer.insertUnsignedByte(static_cast<uint8_t>(m_ceiling->getAttributes().rawValue), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);

		if(!byteBuffer.insertUnsignedByte(static_cast<uint8_t>(m_floor->getAttributes().rawValue), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);
	}
	else {
		if(!byteBuffer.insertUnsignedShort(m_ceiling->getAttributes().rawValue, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint16_t);

		if(!byteBuffer.insertUnsignedShort(m_floor->getAttributes().rawValue, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint16_t);

		if(!byteBuffer.insertShort(m_ceiling->getTileNumber(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);

		if(!byteBuffer.insertShort(m_ceiling->getSlope(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);

		if(!byteBuffer.insertByte(m_ceiling->getShade(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int8_t);

		if(!byteBuffer.insertUnsignedByte(m_ceiling->getPaletteLookupTableNumber(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);

		if(!byteBuffer.insertUnsignedByte(m_ceiling->getXPanning(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);

		if(!byteBuffer.insertUnsignedByte(m_ceiling->getYPanning(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);

		if(!byteBuffer.insertShort(m_floor->getTileNumber(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);

		if(!byteBuffer.insertShort(m_floor->getSlope(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);

		if(!byteBuffer.insertByte(m_floor->getShade(), newOffset)) {
			return false;
		}

		newOffset += sizeof(int8_t);

		if(!byteBuffer.insertUnsignedByte(m_floor->getPaletteLookupTableNumber(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);

		if(!byteBuffer.insertUnsignedByte(m_floor->getXPanning(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);

		if(!byteBuffer.insertUnsignedByte(m_floor->getYPanning(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);
	}

	if(mapVersion == 6) {
		if(!byteBuffer.insertUnsignedByte(m_ceiling->getPaletteLookupTableNumber(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);

		if(!byteBuffer.insertUnsignedByte(m_floor->getPaletteLookupTableNumber(), newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);
	}

	if(!byteBuffer.insertUnsignedByte(m_visibility, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(mapVersion != 6) {
		if(!byteBuffer.insertUnsignedByte(m_filler, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);
	}

	if(!TaggedItem::insertIn(byteBuffer, newOffset)) {
		return false;
	}

	newOffset += TaggedItem::SIZE_BYTES;

	if(mapVersion == 6) {
		if(!byteBuffer.insertBytes(m_trailingData, newOffset)) {
			return false;
		}
	}

	return true;
}

bool Sector::writeTo(ByteBuffer & byteBuffer, uint32_t mapVersion) const {
	if(m_ceiling == nullptr || m_floor == nullptr) {
		return false;
	}

	if(!byteBuffer.writeUnsignedShort(m_firstWallIndex)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedShort(m_numberOfWalls)) {
		return false;
	}

	if(mapVersion == 6) {
		if(!byteBuffer.writeShort(m_ceiling->getTileNumber())) {
			return false;
		}

		if(!byteBuffer.writeShort(m_floor->getTileNumber())) {
			return false;
		}

		if(!byteBuffer.writeShort(m_ceiling->getSlope())) {
			return false;
		}

		if(!byteBuffer.writeShort(m_floor->getSlope())) {
			return false;
		}
	}

	if(!byteBuffer.writeInteger(m_ceiling->getHeight())) {
		return false;
	}

	if(!byteBuffer.writeInteger(m_floor->getHeight())) {
		return false;
	}

	if(mapVersion == 6) {
		if(!byteBuffer.writeByte(m_ceiling->getShade())) {
			return false;
		}

		if(!byteBuffer.writeByte(m_floor->getShade())) {
			return false;
		}

		if(!byteBuffer.writeUnsignedByte(m_ceiling->getXPanning())) {
			return false;
		}

		if(!byteBuffer.writeUnsignedByte(m_floor->getXPanning())) {
			return false;
		}

		if(!byteBuffer.writeUnsignedByte(m_ceiling->getYPanning())) {
			return false;
		}

		if(!byteBuffer.writeUnsignedByte(m_floor->getYPanning())) {
			return false;
		}
	}

	if(mapVersion == 6) {
		if(!byteBuffer.writeUnsignedByte(static_cast<uint8_t>(m_ceiling->getAttributes().rawValue))) {
			return false;
		}

		if(!byteBuffer.writeUnsignedByte(static_cast<uint8_t>(m_floor->getAttributes().rawValue))) {
			return false;
		}
	}
	else {
		if(!byteBuffer.writeUnsignedShort(m_ceiling->getAttributes().rawValue)) {
			return false;
		}

		if(!byteBuffer.writeUnsignedShort(m_floor->getAttributes().rawValue)) {
			return false;
		}

		if(!byteBuffer.writeShort(m_ceiling->getTileNumber())) {
			return false;
		}

		if(!byteBuffer.writeShort(m_ceiling->getSlope())) {
			return false;
		}

		if(!byteBuffer.writeByte(m_ceiling->getShade())) {
			return false;
		}

		if(!byteBuffer.writeUnsignedByte(m_ceiling->getPaletteLookupTableNumber())) {
			return false;
		}

		if(!byteBuffer.writeUnsignedByte(m_ceiling->getXPanning())) {
			return false;
		}

		if(!byteBuffer.writeUnsignedByte(m_ceiling->getYPanning())) {
			return false;
		}

		if(!byteBuffer.writeShort(m_floor->getTileNumber())) {
			return false;
		}

		if(!byteBuffer.writeShort(m_floor->getSlope())) {
			return false;
		}

		if(!byteBuffer.writeByte(m_floor->getShade())) {
			return false;
		}

		if(!byteBuffer.writeUnsignedByte(m_floor->getPaletteLookupTableNumber())) {
			return false;
		}

		if(!byteBuffer.writeUnsignedByte(m_floor->getXPanning())) {
			return false;
		}

		if(!byteBuffer.writeUnsignedByte(m_floor->getYPanning())) {
			return false;
		}
	}

	if(mapVersion == 6) {
		if(!byteBuffer.writeUnsignedByte(m_ceiling->getPaletteLookupTableNumber())) {
			return false;
		}

		if(!byteBuffer.writeUnsignedByte(m_floor->getPaletteLookupTableNumber())) {
			return false;
		}
	}

	if(!byteBuffer.writeUnsignedByte(m_visibility)) {
		return false;
	}

	if(mapVersion != 6) {
		if(!byteBuffer.writeUnsignedByte(m_filler)) {
			return false;
		}
	}

	if(!TaggedItem::writeTo(byteBuffer)) {
		return false;
	}

	if(mapVersion == 6) {
		if(!byteBuffer.writeBytes(m_trailingData)) {
			return false;
		}
	}

	return true;
}

rapidjson::Value Sector::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	if(m_ceiling == nullptr || m_floor == nullptr) {
		return {};
	}

	rapidjson::Value sectorValue(rapidjson::kObjectType);

	sectorValue.AddMember(rapidjson::StringRef(JSON_FIRST_WALL_INDEX_PROPERTY_NAME.c_str()), rapidjson::Value(m_firstWallIndex), allocator);

	sectorValue.AddMember(rapidjson::StringRef(JSON_NUMBER_OF_WALLS_PROPERTY_NAME.c_str()), rapidjson::Value(m_numberOfWalls), allocator);

	rapidjson::Value ceilingValue(m_ceiling->toJSON(allocator));
	sectorValue.AddMember(rapidjson::StringRef(JSON_CEILING_PROPERTY_NAME.c_str()), ceilingValue, allocator);

	rapidjson::Value floorValue(m_floor->toJSON(allocator));
	sectorValue.AddMember(rapidjson::StringRef(JSON_FLOOR_PROPERTY_NAME.c_str()), floorValue, allocator);

	sectorValue.AddMember(rapidjson::StringRef(JSON_VISIBILITY_PROPERTY_NAME.c_str()), rapidjson::Value(m_visibility), allocator);

	TaggedItem::addToJSONObject(sectorValue, allocator);

	sectorValue.AddMember(rapidjson::StringRef(JSON_FILLER_PROPERTY_NAME.c_str()), rapidjson::Value(m_filler), allocator);

	if(!m_trailingData.empty()) {
		rapidjson::Value trailingDataValue(rapidjson::kArrayType);
		trailingDataValue.Reserve(m_trailingData.size(), allocator);

		for(uint8_t b : m_trailingData) {
			trailingDataValue.PushBack(rapidjson::Value(b), allocator);
		}

		sectorValue.AddMember(rapidjson::StringRef(JSON_TRAILING_DATA_PROPERTY_NAME.c_str()), trailingDataValue, allocator);
	}

	return sectorValue;
}

std::unique_ptr<Sector> Sector::parseFrom(const rapidjson::Value & sectorValue) {
	if(!sectorValue.IsObject()) {
		spdlog::error("Invalid sector type: '{}', expected 'object'.", Utilities::typeToString(sectorValue.GetType()));
		return nullptr;
	}

	// parse first wall index
	if(!sectorValue.HasMember(JSON_FIRST_WALL_INDEX_PROPERTY_NAME.c_str())) {
		spdlog::error("Sector is missing '{}' property.", JSON_FIRST_WALL_INDEX_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & firstWallIndexValue = sectorValue[JSON_FIRST_WALL_INDEX_PROPERTY_NAME.c_str()];

	if(!firstWallIndexValue.IsUint()) {
		spdlog::error("Sector has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_FIRST_WALL_INDEX_PROPERTY_NAME, Utilities::typeToString(firstWallIndexValue.GetType()));
		return nullptr;
	}

	uint32_t firstWallIndex = firstWallIndexValue.GetUint();

	if(firstWallIndex > std::numeric_limits<uint16_t>::max()) {
		spdlog::error("Invalid sector first wall index: {}, expected a value between 0 and {}, inclusively.", firstWallIndex, std::numeric_limits<uint16_t>::max());
		return nullptr;
	}

	// parse number of walls
	if(!sectorValue.HasMember(JSON_NUMBER_OF_WALLS_PROPERTY_NAME.c_str())) {
		spdlog::error("Sector is missing '{}' property.", JSON_NUMBER_OF_WALLS_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & numberOfWallsValue = sectorValue[JSON_NUMBER_OF_WALLS_PROPERTY_NAME.c_str()];

	if(!numberOfWallsValue.IsUint()) {
		spdlog::error("Sector has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_NUMBER_OF_WALLS_PROPERTY_NAME, Utilities::typeToString(numberOfWallsValue.GetType()));
		return nullptr;
	}

	uint32_t numberOfWalls = numberOfWallsValue.GetUint();

	if(numberOfWalls > std::numeric_limits<uint16_t>::max()) {
		spdlog::error("Invalid sector number of walls: {}, expected a value between 0 and {}, inclusively.", numberOfWalls, std::numeric_limits<uint16_t>::max());
		return nullptr;
	}

	// parse ceiling
	if(!sectorValue.HasMember(JSON_CEILING_PROPERTY_NAME.c_str())) {
		spdlog::error("Sector is missing '{}' property.", JSON_CEILING_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & ceilingPartitionValue = sectorValue[JSON_CEILING_PROPERTY_NAME.c_str()];

	std::unique_ptr<Partition> ceiling(Partition::parseFrom(ceilingPartitionValue, Partition::Type::Ceiling));

	if(ceiling == nullptr) {
		return nullptr;
	}

	// parse floor
	if(!sectorValue.HasMember(JSON_FLOOR_PROPERTY_NAME.c_str())) {
		spdlog::error("Sector is missing '{}' property.", JSON_FLOOR_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & floorPartitionValue = sectorValue[JSON_FLOOR_PROPERTY_NAME.c_str()];

	std::unique_ptr<Partition> floor(Partition::parseFrom(floorPartitionValue, Partition::Type::Floor));

	if(floor == nullptr) {
		return nullptr;
	}

	// parse visibility
	if(!sectorValue.HasMember(JSON_VISIBILITY_PROPERTY_NAME.c_str())) {
		spdlog::error("Sector is missing '{}' property.", JSON_VISIBILITY_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & visibilityValue = sectorValue[JSON_VISIBILITY_PROPERTY_NAME.c_str()];

	if(!visibilityValue.IsUint()) {
		spdlog::error("Sector has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_VISIBILITY_PROPERTY_NAME, Utilities::typeToString(visibilityValue.GetType()));
		return nullptr;
	}

	uint32_t visibility = visibilityValue.GetUint();

	if(visibility > std::numeric_limits<uint8_t>::max()) {
		spdlog::error("Invalid sector visibility: {}, expected a value between 0 and {}, inclusively.", visibility, std::numeric_limits<uint8_t>::max());
		return nullptr;
	}

	// parse tags
	std::optional<TaggedItem> optionalTaggedItem(TaggedItem::parseFrom(sectorValue));

	if(!optionalTaggedItem.has_value()) {
		return nullptr;
	}

	// parse filler
	if(!sectorValue.HasMember(JSON_FILLER_PROPERTY_NAME.c_str())) {
		spdlog::error("Sector is missing '{}' property.", JSON_FILLER_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & fillerValue = sectorValue[JSON_FILLER_PROPERTY_NAME.c_str()];

	if(!fillerValue.IsUint()) {
		spdlog::error("Sector has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_FILLER_PROPERTY_NAME, Utilities::typeToString(fillerValue.GetType()));
		return nullptr;
	}

	uint32_t filler = fillerValue.GetUint();

	if(filler > std::numeric_limits<uint8_t>::max()) {
		spdlog::error("Invalid sector filler: {}, expected a value between 0 and {}, inclusively.", filler, std::numeric_limits<uint8_t>::max());
		return nullptr;
	}

	// parse trailing data
	std::vector<uint8_t> trailingData;

	if(sectorValue.HasMember(JSON_TRAILING_DATA_PROPERTY_NAME.c_str())) {
		const rapidjson::Value & trailingDataValue = sectorValue[JSON_TRAILING_DATA_PROPERTY_NAME.c_str()];

		if(!trailingDataValue.IsArray()) {
			spdlog::error("Sector has an invalid '{}' property type: '{}', expected 'array'.", JSON_TRAILING_DATA_PROPERTY_NAME, Utilities::typeToString(trailingDataValue.GetType()));
			return nullptr;
		}

		for(rapidjson::Value::ConstValueIterator i = trailingDataValue.Begin(); i != trailingDataValue.End(); ++i) {
			const rapidjson::Value & trailingByteValue = *i;

			if(!trailingByteValue.IsUint()) {
				spdlog::error("Invalid sector trailing data byte #{} type: '{}', expected unsigned integer 'number'." + (trailingData.size() + 1), Utilities::typeToString(trailingByteValue.GetType()));
				return nullptr;
			}

			uint32_t trailingByte = trailingByteValue.GetUint();

			if(trailingByte > std::numeric_limits<uint8_t>::max()) {
				spdlog::error("Invalid sector trailing data byte #{} value: {}, expected unsigned integer 'number' between 0 and {}, inclusively." + (trailingData.size() + 1), trailingByte, std::numeric_limits<uint8_t>::max());
				return nullptr;
			}

			trailingData.push_back(static_cast<uint8_t>(trailingByte));
		}
	}

	return std::make_unique<Sector>(static_cast<uint16_t>(firstWallIndex), static_cast<uint16_t>(numberOfWalls), std::move(ceiling), std::move(floor), static_cast<uint8_t>(visibility), optionalTaggedItem.value(), static_cast<uint8_t>(filler), trailingData);
}

size_t Sector::getSizeInBytes(uint32_t mapVersion) {
	return TaggedItem::SIZE_BYTES + (Partition::getSizeInBytes(mapVersion) * 2) + (sizeof(int16_t) * 2) + (sizeof(uint8_t) * 2) + (mapVersion == 6 ? sizeof(uint8_t) * 2 : 0);
}

bool Sector::operator == (const Sector & sector) const {
	if(m_ceiling == nullptr ||
	   m_floor == nullptr ||
	   sector.m_ceiling == nullptr ||
	   sector.m_floor == nullptr) {
		return false;
	}

	return TaggedItem::operator == (sector) &&
		   m_firstWallIndex == sector.m_firstWallIndex &&
		   m_numberOfWalls == sector.m_numberOfWalls &&
		   *m_ceiling == *sector.m_ceiling &&
		   *m_floor == *sector.m_floor &&
		   m_visibility == sector.m_visibility &&
		   m_filler == sector.m_filler &&
		   m_trailingData ==  sector.m_trailingData;
}

bool Sector::operator != (const Sector & sector) const {
	return !operator == (sector);
}
