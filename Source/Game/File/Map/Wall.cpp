#include "Wall.h"

#include <ByteBuffer.h>
#include <Utilities/RapidJSONUtilities.h>

#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <string>

static const std::string JSON_BLOCK_CLIPPING_ATTRIBUTE_PROPERTY_NAME("blockClipping");
static const std::string JSON_INVISIBLE_WALL_BOTTOM_SWAPPED_ATTRIBUTE_PROPERTY_NAME("invisibleWallBottomSwapped");
static const std::string JSON_TEXTURE_ALIGN_BOTTOM_ATTRIBUTE_PROPERTY_NAME("textureAlignBottom");
static const std::string JSON_X_FLIPPED_ATTRIBUTE_PROPERTY_NAME("xFlipped");
static const std::string JSON_MASKED_ATTRIBUTE_PROPERTY_NAME("masked");
static const std::string JSON_ONE_WAY_ATTRIBUTE_PROPERTY_NAME("oneWay");
static const std::string JSON_BLOCK_HITSCAN_ATTRIBUTE_PROPERTY_NAME("blockHitscan");
static const std::string JSON_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME("translucent");
static const std::string JSON_Y_FLIPPED_ATTRIBUTE_PROPERTY_NAME("yFlipped");
static const std::string JSON_REVERSE_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME("reverseTranslucent");
static const std::string JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME("reserved");

static const std::string JSON_POSITION_PROPERTY_NAME("position");
static const std::string JSON_NEXT_WALL_INDEX_PROPERTY_NAME("nextWallIndex");
static const std::string JSON_ADJACENT_WALL_INDEX_PROPERTY_NAME("adjacentWallIndex");
static const std::string JSON_NEXT_SECTOR_INDEX_PROPERTY_NAME("nextSectorIndex");
static const std::string JSON_ATTRIBUTES_PROPERTY_NAME("attributes");
static const std::string JSON_MASKED_TILE_NUMBER_PROPERTY_NAME("maskedTileNumber");
static const std::string JSON_X_REPEAT_PROPERTY_NAME("xRepeat");
static const std::string JSON_Y_REPEAT_PROPERTY_NAME("yRepeat");
static const std::string JSON_X_PANNING_PROPERTY_NAME("xPanning");
static const std::string JSON_Y_PANNING_PROPERTY_NAME("yPanning");
static const std::string JSON_TAG_INFORMATION_PROPERTY_NAME("tagInformation");

Wall::Wall()
	: TaggedItem(0, 0, 0)
	, TexturedItem(0, 0, 0)
	, m_position(0, 0)
	, m_nextWallIndex(0)
	, m_adjacentWallIndex(0)
	, m_nextSectorIndex(0)
	, m_attributes({ 0 })
	, m_maskedTileNumber(0)
	, m_xRepeat(0)
	, m_yRepeat(0)
	, m_xPanning(0)
	, m_yPanning(0) { }

Wall::Wall(const Point2D & position, uint16_t nextWallIndex, uint16_t adjacentWallIndex, uint16_t nextSectorIndex, Attributes attributes, TexturedItem texturedItem, uint16_t maskedTileNumber, uint8_t xRepeat, uint8_t yRepeat, uint8_t xPanning, uint8_t yPanning, TaggedItem taggedItem)
	: TaggedItem(taggedItem)
	, TexturedItem(texturedItem)
	, m_position(position)
	, m_nextWallIndex(nextWallIndex)
	, m_adjacentWallIndex(adjacentWallIndex)
	, m_nextSectorIndex(nextSectorIndex)
	, m_attributes(attributes)
	, m_maskedTileNumber(maskedTileNumber)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xPanning(xPanning)
	, m_yPanning(yPanning) { }

Wall::Wall(const Point2D & position, uint16_t nextWallIndex, uint16_t adjacentWallIndex, uint16_t nextSectorIndex, Attributes attributes, TexturedItem texturedItem, uint16_t maskedTileNumber, uint8_t xRepeat, uint8_t yRepeat, uint8_t xPanning, uint8_t yPanning, uint16_t lowTag, uint16_t highTag, uint16_t extra)
	: TaggedItem(lowTag, highTag, extra)
	, TexturedItem(texturedItem)
	, m_position(position)
	, m_nextWallIndex(nextWallIndex)
	, m_adjacentWallIndex(adjacentWallIndex)
	, m_nextSectorIndex(nextSectorIndex)
	, m_attributes(attributes)
	, m_maskedTileNumber(maskedTileNumber)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xPanning(xPanning)
	, m_yPanning(yPanning) { }

Wall::Wall(int32_t xPosition, int32_t yPosition, uint16_t nextWallIndex, uint16_t adjacentWallIndex, uint16_t nextSectorIndex, Attributes attributes, TexturedItem texturedItem, uint16_t maskedTileNumber, uint8_t xRepeat, uint8_t yRepeat, uint8_t xPanning, uint8_t yPanning, TaggedItem taggedItem)
	: TaggedItem(taggedItem)
	, TexturedItem(texturedItem)
	, m_position(xPosition, yPosition)
	, m_nextWallIndex(nextWallIndex)
	, m_adjacentWallIndex(adjacentWallIndex)
	, m_nextSectorIndex(nextSectorIndex)
	, m_attributes(attributes)
	, m_maskedTileNumber(maskedTileNumber)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xPanning(xPanning)
	, m_yPanning(yPanning) { }


Wall::Wall(int32_t xPosition, int32_t yPosition, uint16_t nextWallIndex, uint16_t adjacentWallIndex, uint16_t nextSectorIndex, Attributes attributes, TexturedItem texturedItem, uint16_t maskedTileNumber, uint8_t xRepeat, uint8_t yRepeat, uint8_t xPanning, uint8_t yPanning, uint16_t lowTag, uint16_t highTag, uint16_t extra)
	: TaggedItem(lowTag, highTag, extra)
	, TexturedItem(texturedItem)
	, m_position(xPosition, yPosition)
	, m_nextWallIndex(nextWallIndex)
	, m_adjacentWallIndex(adjacentWallIndex)
	, m_nextSectorIndex(nextSectorIndex)
	, m_attributes(attributes)
	, m_maskedTileNumber(maskedTileNumber)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xPanning(xPanning)
	, m_yPanning(yPanning) { }

Wall::Wall(const Point2D & position, uint16_t nextWallIndex, uint16_t adjacentWallIndex, uint16_t nextSectorIndex, Attributes attributes, uint16_t tileNumber, uint16_t maskedTileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t xRepeat, uint8_t yRepeat, uint8_t xPanning, uint8_t yPanning, TaggedItem taggedItem)
	: TaggedItem(taggedItem)
	, TexturedItem(tileNumber, shade, paletteLookupTableNumber)
	, m_position(position)
	, m_nextWallIndex(nextWallIndex)
	, m_adjacentWallIndex(adjacentWallIndex)
	, m_nextSectorIndex(nextSectorIndex)
	, m_attributes(attributes)
	, m_maskedTileNumber(maskedTileNumber)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xPanning(xPanning)
	, m_yPanning(yPanning) { }

Wall::Wall(const Point2D & position, uint16_t nextWallIndex, uint16_t adjacentWallIndex, uint16_t nextSectorIndex, Attributes attributes, uint16_t tileNumber, uint16_t maskedTileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t xRepeat, uint8_t yRepeat, uint8_t xPanning, uint8_t yPanning, uint16_t lowTag, uint16_t highTag, uint16_t extra)
	: TaggedItem(lowTag, highTag, extra)
	, TexturedItem(tileNumber, shade, paletteLookupTableNumber)
	, m_position(position)
	, m_nextWallIndex(nextWallIndex)
	, m_adjacentWallIndex(adjacentWallIndex)
	, m_nextSectorIndex(nextSectorIndex)
	, m_attributes(attributes)
	, m_maskedTileNumber(maskedTileNumber)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xPanning(xPanning)
	, m_yPanning(yPanning) { }

Wall::Wall(int32_t xPosition, int32_t yPosition, uint16_t nextWallIndex, uint16_t adjacentWallIndex, uint16_t nextSectorIndex, Attributes attributes, uint16_t tileNumber, uint16_t maskedTileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t xRepeat, uint8_t yRepeat, uint8_t xPanning, uint8_t yPanning, TaggedItem taggedItem)
	: TaggedItem(taggedItem)
	, TexturedItem(tileNumber, shade, paletteLookupTableNumber)
	, m_position(xPosition, yPosition)
	, m_nextWallIndex(nextWallIndex)
	, m_adjacentWallIndex(adjacentWallIndex)
	, m_nextSectorIndex(nextSectorIndex)
	, m_attributes(attributes)
	, m_maskedTileNumber(maskedTileNumber)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xPanning(xPanning)
	, m_yPanning(yPanning) { }


Wall::Wall(int32_t xPosition, int32_t yPosition, uint16_t nextWallIndex, uint16_t adjacentWallIndex, uint16_t nextSectorIndex, Attributes attributes, uint16_t tileNumber, uint16_t maskedTileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t xRepeat, uint8_t yRepeat, uint8_t xPanning, uint8_t yPanning, uint16_t lowTag, uint16_t highTag, uint16_t extra)
	: TaggedItem(lowTag, highTag, extra)
	, TexturedItem(tileNumber, shade, paletteLookupTableNumber)
	, m_position(xPosition, yPosition)
	, m_nextWallIndex(nextWallIndex)
	, m_adjacentWallIndex(adjacentWallIndex)
	, m_nextSectorIndex(nextSectorIndex)
	, m_attributes(attributes)
	, m_maskedTileNumber(maskedTileNumber)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xPanning(xPanning)
	, m_yPanning(yPanning) { }

Wall::Wall(Wall && wall) noexcept
	: TaggedItem(std::move(wall))
	, TexturedItem(std::move(wall))
	, m_position(std::move(wall.m_position))
	, m_nextWallIndex(wall.m_nextWallIndex)
	, m_adjacentWallIndex(wall.m_adjacentWallIndex)
	, m_nextSectorIndex(wall.m_nextSectorIndex)
	, m_attributes(std::move(wall.m_attributes))
	, m_maskedTileNumber(wall.m_maskedTileNumber)
	, m_xRepeat(wall.m_xRepeat)
	, m_yRepeat(wall.m_yRepeat)
	, m_xPanning(wall.m_xPanning)
	, m_yPanning(wall.m_yPanning) { }

Wall::Wall(const Wall & wall)
	: TaggedItem(wall)
	, TexturedItem(wall)
	, m_position(wall.m_position)
	, m_nextWallIndex(wall.m_nextWallIndex)
	, m_adjacentWallIndex(wall.m_adjacentWallIndex)
	, m_nextSectorIndex(wall.m_nextSectorIndex)
	, m_attributes(wall.m_attributes)
	, m_maskedTileNumber(wall.m_maskedTileNumber)
	, m_xRepeat(wall.m_xRepeat)
	, m_yRepeat(wall.m_yRepeat)
	, m_xPanning(wall.m_xPanning)
	, m_yPanning(wall.m_yPanning) { }

Wall & Wall::operator = (Wall && wall) noexcept {
	if(this != &wall) {
		TaggedItem::operator = (std::move(wall));
		TexturedItem::operator = (std::move(wall));

		m_position = std::move(wall.m_position);
		m_nextWallIndex = wall.m_nextWallIndex;
		m_adjacentWallIndex = wall.m_adjacentWallIndex;
		m_nextSectorIndex = wall.m_nextSectorIndex;
		m_attributes = std::move(wall.m_attributes);
		m_maskedTileNumber = wall.m_maskedTileNumber;
		m_xRepeat = wall.m_xRepeat;
		m_yRepeat = wall.m_yRepeat;
		m_xPanning = wall.m_xPanning;
		m_yPanning = wall.m_yPanning;
	}

	return *this;
}

Wall & Wall::operator = (const Wall & wall) {
	TaggedItem::operator = (wall);
	TexturedItem::operator = (wall);

	m_position = wall.m_position;
	m_nextWallIndex = wall.m_nextWallIndex;
	m_adjacentWallIndex = wall.m_adjacentWallIndex;
	m_nextSectorIndex = wall.m_nextSectorIndex;
	m_attributes = wall.m_attributes;
	m_maskedTileNumber = wall.m_maskedTileNumber;
	m_xRepeat = wall.m_xRepeat;
	m_yRepeat = wall.m_yRepeat;
	m_xPanning = wall.m_xPanning;
	m_yPanning = wall.m_yPanning;

	return *this;
}

Wall::~Wall() { }

bool Wall::isClippingBlocked() const {
	return m_attributes.blockClipping;
}

void Wall::setBlockClipping(bool blockClipping) {
	m_attributes.blockClipping = blockClipping;
}

bool Wall::isInvisibleWallBottomSwapped() const {
	return m_attributes.invisibleWallBottomSwapped;
}

void Wall::setInvisibleWallBottomSwapped(bool invisibleWallBottomSwapped) {
	m_attributes.invisibleWallBottomSwapped = invisibleWallBottomSwapped;
}

bool Wall::isTextureBottomAligned() const {
	return m_attributes.textureAlignBottom;
}

void Wall::setTextureBottomAligned(bool textureAlignBottom) {
	m_attributes.textureAlignBottom = textureAlignBottom;
}

bool Wall::isXFlipped() const {
	return m_attributes.xFlipped;
}

void Wall::setXFlipped(bool xFlipped) {
	m_attributes.xFlipped = xFlipped;
}

bool Wall::isYFlipped() const {
	return m_attributes.yFlipped;
}

void Wall::setYFlipped(bool yFlipped) {
	m_attributes.yFlipped = yFlipped;
}

bool Wall::isMasked() const {
	return m_attributes.masked;
}

void Wall::setMasked(bool masked) {
	m_attributes.masked = masked;
}

bool Wall::isOneWay() const {
	return m_attributes.oneWay;
}

void Wall::setOneWay(bool oneWay) {
	m_attributes.oneWay = oneWay;
}

bool Wall::isHitscanBlocked() const {
	return m_attributes.blockHitscan;
}

void Wall::setBlockHitscan(bool blockHitscan) {
	m_attributes.blockHitscan = blockHitscan;
}

bool Wall::isTranslucent() const {
	return m_attributes.translucent;
}

void Wall::setTranslucent(bool translucent) {
	m_attributes.translucent = translucent;
}

bool Wall::isReverseTranslucent() const {
	return m_attributes.reverseTranslucent;
}

void Wall::setReverseTranslucent(bool reverseTranslucent) {
	m_attributes.reverseTranslucent = reverseTranslucent;
}

uint8_t Wall::getReserved() const {
	return m_attributes.reserved;
}

bool Wall::setReserved(uint8_t reserved) {
	if(reserved > 63) {
		return false;
	}

	m_attributes.reserved = reserved;

	return true;
}

Wall::Attributes & Wall::getAttributes() {
	return m_attributes;
}

const Wall::Attributes & Wall::getAttributes() const {
	return m_attributes;
}

void Wall::setAttributes(Attributes attributes) {
	m_attributes = attributes;
}

const Point2D & Wall::getPosition() const {
	return m_position;
}

void Wall::setPosition(const Point2D & position) {
	m_position = position;
}

uint16_t Wall::getNextWallIndex() const {
	return m_nextWallIndex;
}

void Wall::setNextWallIndex(uint16_t nextWallIndex) {
	m_nextWallIndex = nextWallIndex;
}

uint16_t Wall::getAdjacentWallIndex() const {
	return m_adjacentWallIndex;
}

void Wall::setAdjacentWallIndex(uint16_t adjacentWallIndex) {
	m_adjacentWallIndex = adjacentWallIndex;
}

uint16_t Wall::getNextSectorIndex() const {
	return m_nextSectorIndex;
}

void Wall::setNextSectorIndex(uint16_t nextSectorIndex) {
	m_nextSectorIndex = nextSectorIndex;
}

uint16_t Wall::getMaskedTileNumber() const {
	return m_maskedTileNumber;
}

void Wall::setMaskedTileNumber(uint16_t maskedTileNumber) {
	m_maskedTileNumber = maskedTileNumber;
}

uint8_t Wall::getXRepeat() const {
	return m_xRepeat;
}

void Wall::setXRepeat(uint8_t xRepeat) {
	m_xRepeat = xRepeat;
}

uint8_t Wall::getYRepeat() const {
	return m_yRepeat;
}

void Wall::setYRepeat(uint8_t yRepeat) {
	m_yRepeat = yRepeat;
}

uint8_t Wall::getXPanning() const {
	return m_xPanning;
}

void Wall::setXPanning(uint8_t xPanning) {
	m_xPanning = xPanning;
}

uint8_t Wall::getYPanning() const {
	return m_yPanning;
}

void Wall::setYPanning(uint8_t yPanning) {
	m_yPanning = yPanning;
}

std::unique_ptr<Wall> Wall::getFrom(const ByteBuffer & byteBuffer, size_t offset, uint32_t mapVersion) {
	uint16_t nextSectorIndex = 0;
	uint16_t rawAttributesValue = 0;
	bool error = false;
	size_t newOffset = offset;

	Point2D position(Point2D::getFrom(byteBuffer, newOffset, &error));

	if(error) {
		return nullptr;
	}

	newOffset += Point2D::SIZE_BYTES;

	uint16_t nextWallIndex = byteBuffer.getUnsignedShort(newOffset, &error);

	if(error) {
		return nullptr;
	}

	newOffset += sizeof(uint16_t);

	if(mapVersion == 6) {
		nextSectorIndex = byteBuffer.getUnsignedShort(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint16_t);
	}

	uint16_t adjacentWallIndex = byteBuffer.getUnsignedShort(newOffset, &error);

	if(error) {
		return nullptr;
	}

	newOffset += sizeof(uint16_t);

	if(mapVersion != 6) {
		nextSectorIndex = byteBuffer.getUnsignedShort(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint16_t);

		rawAttributesValue = byteBuffer.getUnsignedShort(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint16_t);
	}

	uint16_t tileNumber = byteBuffer.getUnsignedShort(newOffset, &error);

	if(error) {
		return nullptr;
	}

	newOffset += sizeof(uint16_t);

	uint16_t maskedTileNumber = byteBuffer.getUnsignedShort(newOffset, &error);

	if(error) {
		return nullptr;
	}

	newOffset += sizeof(uint16_t);

	int8_t shade = byteBuffer.getByte(newOffset, &error);

	if(error) {
		return nullptr;
	}

	newOffset += sizeof(int8_t);

	uint8_t paletteLookupTableNumber = byteBuffer.getUnsignedByte(newOffset, &error);

	if(error) {
		return nullptr;
	}

	newOffset += sizeof(uint8_t);

	if(mapVersion == 6) {
		rawAttributesValue = byteBuffer.getUnsignedShort(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint16_t);
	}

	uint8_t xRepeat = byteBuffer.getUnsignedByte(newOffset, &error);

	if(error) {
		return nullptr;
	}

	newOffset += sizeof(uint8_t);

	uint8_t yRepeat = byteBuffer.getUnsignedByte(newOffset, &error);

	if(error) {
		return nullptr;
	}

	newOffset += sizeof(uint8_t);

	uint8_t xPanning = byteBuffer.getUnsignedByte(newOffset, &error);

	if(error) {
		return nullptr;
	}

	newOffset += sizeof(uint8_t);

	uint8_t yPanning = byteBuffer.getUnsignedByte(newOffset, &error);

	if(error) {
		return nullptr;
	}

	newOffset += sizeof(uint8_t);

	TaggedItem taggedItem(TaggedItem::getFrom(byteBuffer, newOffset, &error));

	if(error) {
		return nullptr;
	}

	Attributes attributes;
	attributes.rawValue = rawAttributesValue;

	return std::make_unique<Wall>(position, nextWallIndex, adjacentWallIndex, nextSectorIndex, attributes, tileNumber, maskedTileNumber, shade, paletteLookupTableNumber, xRepeat, yRepeat, xPanning, yPanning, taggedItem);
}

std::unique_ptr<Wall> Wall::readFrom(const ByteBuffer & byteBuffer, uint32_t mapVersion) {
	std::unique_ptr<Wall> value(getFrom(byteBuffer, byteBuffer.getReadOffset(), mapVersion));

	if(value != nullptr) {
		byteBuffer.skipReadBytes(SIZE_BYTES);
	}

	return value;
}

bool Wall::putIn(ByteBuffer & byteBuffer, size_t offset, uint32_t mapVersion) const {
	size_t newOffset = offset;

	if(!m_position.putIn(byteBuffer, newOffset)) {
		return false;
	}

	newOffset += Point2D::SIZE_BYTES;

	if(!byteBuffer.putUnsignedShort(m_nextWallIndex, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(mapVersion == 6) {
		if(!byteBuffer.putUnsignedShort(m_nextSectorIndex, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint16_t);
	}

	if(!byteBuffer.putUnsignedShort(m_adjacentWallIndex, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(mapVersion != 6) {
		if(!byteBuffer.putUnsignedShort(m_nextSectorIndex, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint16_t);

		if(!byteBuffer.putUnsignedShort(m_attributes.rawValue, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint16_t);
	}

	if(!byteBuffer.putUnsignedShort(m_tileNumber, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(!byteBuffer.putUnsignedShort(m_maskedTileNumber, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(!byteBuffer.putByte(m_shade, newOffset)) {
		return false;
	}

	newOffset += sizeof(int8_t);

	if(!byteBuffer.putUnsignedByte(m_paletteLookupTableNumber, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(mapVersion == 6) {
		if(!byteBuffer.putUnsignedShort(m_attributes.rawValue, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint16_t);
	}

	if(!byteBuffer.putUnsignedByte(m_xRepeat, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(!byteBuffer.putUnsignedByte(m_yRepeat, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(!byteBuffer.putUnsignedByte(m_xPanning, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(!byteBuffer.putUnsignedByte(m_yPanning, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(!TaggedItem::putIn(byteBuffer, newOffset)) {
		return false;
	}

	return true;
}

bool Wall::insertIn(ByteBuffer & byteBuffer, size_t offset, uint32_t mapVersion) const {
	size_t newOffset = offset;

	if(!m_position.insertIn(byteBuffer, newOffset)) {
		return false;
	}

	newOffset += Point2D::SIZE_BYTES;

	if(!byteBuffer.insertUnsignedShort(m_nextWallIndex, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(mapVersion == 6) {
		if(!byteBuffer.insertUnsignedShort(m_nextSectorIndex, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint16_t);
	}

	if(!byteBuffer.insertUnsignedShort(m_adjacentWallIndex, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(mapVersion != 6) {
		if(!byteBuffer.insertUnsignedShort(m_nextSectorIndex, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint16_t);

		if(!byteBuffer.insertUnsignedShort(m_attributes.rawValue, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint16_t);
	}

	if(!byteBuffer.insertUnsignedShort(m_tileNumber, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(!byteBuffer.insertUnsignedShort(m_maskedTileNumber, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(!byteBuffer.insertByte(m_shade, newOffset)) {
		return false;
	}

	newOffset += sizeof(int8_t);

	if(!byteBuffer.insertUnsignedByte(m_paletteLookupTableNumber, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(mapVersion == 6) {
		if(!byteBuffer.insertUnsignedShort(m_attributes.rawValue, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint16_t);
	}

	if(!byteBuffer.insertUnsignedByte(m_xRepeat, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(!byteBuffer.insertUnsignedByte(m_yRepeat, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(!byteBuffer.insertUnsignedByte(m_xPanning, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(!byteBuffer.insertUnsignedByte(m_yPanning, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(!TaggedItem::insertIn(byteBuffer, newOffset)) {
		return false;
	}

	return true;
}

bool Wall::writeTo(ByteBuffer & byteBuffer, uint32_t mapVersion) const {
	if(!m_position.writeTo(byteBuffer)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedShort(m_nextWallIndex)) {
		return false;
	}

	if(mapVersion == 6) {
		if(!byteBuffer.writeUnsignedShort(m_nextSectorIndex)) {
			return false;
		}
	}

	if(!byteBuffer.writeUnsignedShort(m_adjacentWallIndex)) {
		return false;
	}

	if(mapVersion != 6) {
		if(!byteBuffer.writeUnsignedShort(m_nextSectorIndex)) {
			return false;
		}

		if(!byteBuffer.writeUnsignedShort(m_attributes.rawValue)) {
			return false;
		}
	}

	if(!byteBuffer.writeUnsignedShort(m_tileNumber)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedShort(m_maskedTileNumber)) {
		return false;
	}

	if(!byteBuffer.writeByte(m_shade)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedByte(m_paletteLookupTableNumber)) {
		return false;
	}

	if(mapVersion == 6) {
		if(!byteBuffer.writeUnsignedShort(m_attributes.rawValue)) {
			return false;
		}
	}

	if(!byteBuffer.writeUnsignedByte(m_xRepeat)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedByte(m_yRepeat)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedByte(m_xPanning)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedByte(m_yPanning)) {
		return false;
	}

	if(!TaggedItem::writeTo(byteBuffer)) {
		return false;
	}

	return true;
}

rapidjson::Value Wall::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value wallValue(rapidjson::kObjectType);

	rapidjson::Value positionValue(m_position.toJSON(allocator));
	wallValue.AddMember(rapidjson::StringRef(JSON_POSITION_PROPERTY_NAME.c_str()), positionValue, allocator);

	wallValue.AddMember(rapidjson::StringRef(JSON_NEXT_WALL_INDEX_PROPERTY_NAME.c_str()), rapidjson::Value(m_nextWallIndex), allocator);

	wallValue.AddMember(rapidjson::StringRef(JSON_ADJACENT_WALL_INDEX_PROPERTY_NAME.c_str()), rapidjson::Value(m_adjacentWallIndex), allocator);

	wallValue.AddMember(rapidjson::StringRef(JSON_NEXT_SECTOR_INDEX_PROPERTY_NAME.c_str()), rapidjson::Value(m_nextSectorIndex), allocator);

	rapidjson::Value attributesValue(m_attributes.toJSON(allocator));
	wallValue.AddMember(rapidjson::StringRef(JSON_ATTRIBUTES_PROPERTY_NAME.c_str()), attributesValue, allocator);

	TexturedItem::addToJSONObject(wallValue, allocator);

	wallValue.AddMember(rapidjson::StringRef(JSON_MASKED_TILE_NUMBER_PROPERTY_NAME.c_str()), rapidjson::Value(m_maskedTileNumber), allocator);

	wallValue.AddMember(rapidjson::StringRef(JSON_X_REPEAT_PROPERTY_NAME.c_str()), rapidjson::Value(m_xRepeat), allocator);

	wallValue.AddMember(rapidjson::StringRef(JSON_Y_REPEAT_PROPERTY_NAME.c_str()), rapidjson::Value(m_yRepeat), allocator);

	wallValue.AddMember(rapidjson::StringRef(JSON_X_PANNING_PROPERTY_NAME.c_str()), rapidjson::Value(m_xPanning), allocator);

	wallValue.AddMember(rapidjson::StringRef(JSON_Y_PANNING_PROPERTY_NAME.c_str()), rapidjson::Value(m_yPanning), allocator);

	TaggedItem::addToJSONObject(wallValue, allocator);

	return wallValue;
}

std::unique_ptr<Wall> Wall::parseFrom(const rapidjson::Value & wallValue) {
	if(!wallValue.IsObject()) {
		spdlog::error("Invalid wall type: '{}', expected 'object'.", Utilities::typeToString(wallValue.GetType()));
		return nullptr;
	}

	// parse position
	if(!wallValue.HasMember(JSON_POSITION_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall is missing '{}' property.", JSON_POSITION_PROPERTY_NAME);
		return nullptr;
	}

	std::optional<Point2D> optionalPosition(Point2D::parseFrom(wallValue[JSON_POSITION_PROPERTY_NAME.c_str()]));

	if(!optionalPosition.has_value()) {
		return nullptr;
	}

	// parse next wall index
	if(!wallValue.HasMember(JSON_NEXT_WALL_INDEX_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall is missing '{}' property.", JSON_NEXT_WALL_INDEX_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & nextWallIndexValue = wallValue[JSON_NEXT_WALL_INDEX_PROPERTY_NAME.c_str()];

	if(!nextWallIndexValue.IsUint()) {
		spdlog::error("Wall has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_NEXT_WALL_INDEX_PROPERTY_NAME, Utilities::typeToString(nextWallIndexValue.GetType()));
		return nullptr;
	}

	uint32_t nextWallIndex = nextWallIndexValue.GetUint();

	if(nextWallIndex > std::numeric_limits<uint16_t>::max()) {
		spdlog::error("Invalid wall next wall index: {}, expected a value between 0 and {}, inclusively.", nextWallIndex, std::numeric_limits<uint16_t>::max());
		return nullptr;
	}

	// parse adjacent wall index
	if(!wallValue.HasMember(JSON_ADJACENT_WALL_INDEX_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall is missing '{}' property.", JSON_ADJACENT_WALL_INDEX_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & adjacentWallIndexValue = wallValue[JSON_ADJACENT_WALL_INDEX_PROPERTY_NAME.c_str()];

	if(!adjacentWallIndexValue.IsUint()) {
		spdlog::error("Wall has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_ADJACENT_WALL_INDEX_PROPERTY_NAME, Utilities::typeToString(adjacentWallIndexValue.GetType()));
		return nullptr;
	}

	uint32_t adjacentWallIndex = adjacentWallIndexValue.GetUint();

	if(adjacentWallIndex > std::numeric_limits<uint16_t>::max()) {
		spdlog::error("Invalid wall adjacent wall index: {}, expected a value between 0 and {}, inclusively.", adjacentWallIndex, std::numeric_limits<uint16_t>::max());
		return nullptr;
	}

	// parse next sector index
	if(!wallValue.HasMember(JSON_NEXT_SECTOR_INDEX_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall is missing '{}' property.", JSON_NEXT_SECTOR_INDEX_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & nextSectorIndexValue = wallValue[JSON_NEXT_SECTOR_INDEX_PROPERTY_NAME.c_str()];

	if(!nextSectorIndexValue.IsUint()) {
		spdlog::error("Wall has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_NEXT_SECTOR_INDEX_PROPERTY_NAME, Utilities::typeToString(nextSectorIndexValue.GetType()));
		return nullptr;
	}

	uint32_t nextSectorIndex = nextSectorIndexValue.GetUint();

	if(nextSectorIndex > std::numeric_limits<uint16_t>::max()) {
		spdlog::error("Invalid wall next sector index: {}, expected a value between 0 and {}, inclusively.", nextSectorIndex, std::numeric_limits<uint16_t>::max());
		return nullptr;
	}

	// parse attributes
	if(!wallValue.HasMember(JSON_ATTRIBUTES_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall is missing '{}' property.", JSON_ATTRIBUTES_PROPERTY_NAME);
		return nullptr;
	}

	std::optional<Attributes> optionalAttributes(Attributes::parseFrom(wallValue[JSON_ATTRIBUTES_PROPERTY_NAME.c_str()]));

	if(!optionalAttributes.has_value()) {
		return nullptr;
	}

	// parse textured item information
	std::optional<TexturedItem> optionalTexturedItem(TexturedItem::parseFrom(wallValue));

	if(!optionalTexturedItem.has_value()) {
		return nullptr;
	}

	// parse masked tile number
	if(!wallValue.HasMember(JSON_MASKED_TILE_NUMBER_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall is missing '{}' property.", JSON_MASKED_TILE_NUMBER_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & maskedTileNumberValue = wallValue[JSON_MASKED_TILE_NUMBER_PROPERTY_NAME.c_str()];

	if(!maskedTileNumberValue.IsUint()) {
		spdlog::error("Wall has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_MASKED_TILE_NUMBER_PROPERTY_NAME, Utilities::typeToString(maskedTileNumberValue.GetType()));
		return nullptr;
	}

	uint32_t maskedTileNumber = maskedTileNumberValue.GetUint();

	if(maskedTileNumber > std::numeric_limits<uint16_t>::max()) {
		spdlog::error("Invalid wall masked tile number: {}, expected a value between 0 and {}, inclusively.", maskedTileNumber, std::numeric_limits<uint16_t>::max());
		return nullptr;
	}

	// parse x repeat
	if(!wallValue.HasMember(JSON_X_REPEAT_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall is missing '{}' property.", JSON_X_REPEAT_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & xRepeatValue = wallValue[JSON_X_REPEAT_PROPERTY_NAME.c_str()];

	if(!xRepeatValue.IsUint()) {
		spdlog::error("Wall has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_X_REPEAT_PROPERTY_NAME, Utilities::typeToString(xRepeatValue.GetType()));
		return nullptr;
	}

	uint32_t xRepeat = xRepeatValue.GetUint();

	if(xRepeat > std::numeric_limits<uint8_t>::max()) {
		spdlog::error("Invalid wall x repeat: {}, expected a value between 0 and {}, inclusively.", xRepeat, std::numeric_limits<uint8_t>::max());
		return nullptr;
	}

	// parse y repeat
	if(!wallValue.HasMember(JSON_Y_REPEAT_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall is missing '{}' property.", JSON_Y_REPEAT_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & yRepeatValue = wallValue[JSON_Y_REPEAT_PROPERTY_NAME.c_str()];

	if(!yRepeatValue.IsUint()) {
		spdlog::error("Wall has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_Y_REPEAT_PROPERTY_NAME, Utilities::typeToString(yRepeatValue.GetType()));
		return nullptr;
	}

	uint32_t yRepeat = yRepeatValue.GetUint();

	if(yRepeat > std::numeric_limits<uint8_t>::max()) {
		spdlog::error("Invalid wall y repeat: {}, expected a value between 0 and {}, inclusively.", yRepeat, std::numeric_limits<uint8_t>::max());
		return nullptr;
	}

	// parse x panning
	if(!wallValue.HasMember(JSON_X_PANNING_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall is missing '{}' property.", JSON_X_PANNING_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & xPanningValue = wallValue[JSON_X_PANNING_PROPERTY_NAME.c_str()];

	if(!xPanningValue.IsUint()) {
		spdlog::error("Wall has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_X_PANNING_PROPERTY_NAME, Utilities::typeToString(xPanningValue.GetType()));
		return nullptr;
	}

	uint32_t xPanning = xPanningValue.GetUint();

	if(xPanning > std::numeric_limits<uint8_t>::max()) {
		spdlog::error("Invalid wall x panning: {}, expected a value between 0 and {}, inclusively.", xPanning, std::numeric_limits<uint8_t>::max());
		return nullptr;
	}

	// parse y panning
	if(!wallValue.HasMember(JSON_Y_PANNING_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall is missing '{}' property.", JSON_Y_PANNING_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & yPanningValue = wallValue[JSON_Y_PANNING_PROPERTY_NAME.c_str()];

	if(!yPanningValue.IsUint()) {
		spdlog::error("Wall has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_Y_PANNING_PROPERTY_NAME, Utilities::typeToString(yPanningValue.GetType()));
		return nullptr;
	}

	uint32_t yPanning = yPanningValue.GetUint();

	if(yPanning > std::numeric_limits<uint8_t>::max()) {
		spdlog::error("Invalid wall y panning: {}, expected a value between 0 and {}, inclusively.", yPanning, std::numeric_limits<uint8_t>::max());
		return nullptr;
	}

	// parse tags
	std::optional<TaggedItem> optionalTaggedItem(TaggedItem::parseFrom(wallValue));

	if(!optionalTaggedItem.has_value()) {
		return nullptr;
	}

	return std::make_unique<Wall>(optionalPosition.value(), static_cast<uint16_t>(nextWallIndex), static_cast<uint16_t>(adjacentWallIndex), static_cast<uint16_t>(nextSectorIndex), optionalAttributes.value(), optionalTexturedItem.value(), static_cast<uint16_t>(maskedTileNumber), static_cast<uint8_t>(xRepeat), static_cast<uint8_t>(yRepeat), static_cast<uint8_t>(xPanning), static_cast<uint8_t>(yPanning), optionalTaggedItem->getLowTag(), optionalTaggedItem->getHighTag(), optionalTaggedItem->getExtra());
}

bool Wall::operator == (const Wall & wall) const {
	return TaggedItem::operator == (wall) &&
		   TexturedItem::operator == (wall) &&
		   m_position == wall.m_position &&
		   m_nextWallIndex == wall.m_nextWallIndex &&
		   m_adjacentWallIndex == wall.m_adjacentWallIndex &&
		   m_nextSectorIndex == wall.m_nextSectorIndex &&
		   m_attributes == wall.m_attributes &&
		   m_maskedTileNumber == wall.m_maskedTileNumber &&
		   m_xRepeat == wall.m_xRepeat &&
		   m_yRepeat == wall.m_yRepeat &&
		   m_xPanning == wall.m_xPanning &&
		   m_yPanning == wall.m_yPanning;
}

bool Wall::operator != (const Wall & wall) const {
	return !operator == (wall);
}

rapidjson::Value Wall::Attributes::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value attributesValue(rapidjson::kObjectType);

	addToJSONObject(attributesValue, allocator);

	return attributesValue;
}

bool Wall::Attributes::addToJSONObject(rapidjson::Value & value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	if(!value.IsObject()) {
		return false;
	}

	value.AddMember(rapidjson::StringRef(JSON_BLOCK_CLIPPING_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(blockClipping), allocator);

	value.AddMember(rapidjson::StringRef(JSON_INVISIBLE_WALL_BOTTOM_SWAPPED_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(invisibleWallBottomSwapped), allocator);

	value.AddMember(rapidjson::StringRef(JSON_TEXTURE_ALIGN_BOTTOM_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(textureAlignBottom), allocator);

	value.AddMember(rapidjson::StringRef(JSON_X_FLIPPED_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(xFlipped), allocator);

	value.AddMember(rapidjson::StringRef(JSON_MASKED_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(masked), allocator);

	value.AddMember(rapidjson::StringRef(JSON_ONE_WAY_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(oneWay), allocator);

	value.AddMember(rapidjson::StringRef(JSON_BLOCK_HITSCAN_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(blockHitscan), allocator);

	value.AddMember(rapidjson::StringRef(JSON_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(translucent), allocator);

	value.AddMember(rapidjson::StringRef(JSON_Y_FLIPPED_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(yFlipped), allocator);

	value.AddMember(rapidjson::StringRef(JSON_REVERSE_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(reverseTranslucent), allocator);

	value.AddMember(rapidjson::StringRef(JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(reserved), allocator);

	return true;
}

Wall::Attributes Wall::Attributes::parseFrom(const rapidjson::Value & attributesValue, bool * error) {
	if(!attributesValue.IsObject()) {
		spdlog::error("Invalid wall attributes type: '{}', expected 'object'.", Utilities::typeToString(attributesValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse block clipping attribute
	if(!attributesValue.HasMember(JSON_BLOCK_CLIPPING_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall attribute '{}' is missing.", JSON_BLOCK_CLIPPING_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & blockClippingValue = attributesValue[JSON_BLOCK_CLIPPING_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!blockClippingValue.IsBool()) {
		spdlog::error("Wall has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_BLOCK_CLIPPING_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(blockClippingValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool blockClipping = blockClippingValue.GetBool();

	// parse invisible wall bottom swapped attribute
	if(!attributesValue.HasMember(JSON_INVISIBLE_WALL_BOTTOM_SWAPPED_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall attribute '{}' is missing.", JSON_INVISIBLE_WALL_BOTTOM_SWAPPED_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & invisibleWallBottomSwappedValue = attributesValue[JSON_INVISIBLE_WALL_BOTTOM_SWAPPED_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!invisibleWallBottomSwappedValue.IsBool()) {
		spdlog::error("Wall has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_INVISIBLE_WALL_BOTTOM_SWAPPED_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(invisibleWallBottomSwappedValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool invisibleWallBottomSwapped = invisibleWallBottomSwappedValue.GetBool();

	// parse texture align bottom attribute
	if(!attributesValue.HasMember(JSON_TEXTURE_ALIGN_BOTTOM_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall attribute '{}' is missing.", JSON_TEXTURE_ALIGN_BOTTOM_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & textureAlignBottomValue = attributesValue[JSON_TEXTURE_ALIGN_BOTTOM_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!textureAlignBottomValue.IsBool()) {
		spdlog::error("Wall has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_TEXTURE_ALIGN_BOTTOM_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(textureAlignBottomValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool textureAlignBottom = textureAlignBottomValue.GetBool();

	// parse x flipped attribute
	if(!attributesValue.HasMember(JSON_X_FLIPPED_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall attribute '{}' is missing.", JSON_X_FLIPPED_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & xFlippedValue = attributesValue[JSON_X_FLIPPED_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!xFlippedValue.IsBool()) {
		spdlog::error("Wall has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_X_FLIPPED_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(xFlippedValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool xFlipped = xFlippedValue.GetBool();

	// parse masked attribute
	if(!attributesValue.HasMember(JSON_MASKED_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall attribute '{}' is missing.", JSON_MASKED_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & maskedValue = attributesValue[JSON_MASKED_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!maskedValue.IsBool()) {
		spdlog::error("Wall has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_MASKED_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(maskedValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool masked = maskedValue.GetBool();

	// parse one way attribute
	if(!attributesValue.HasMember(JSON_ONE_WAY_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall attribute '{}' is missing.", JSON_ONE_WAY_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & oneWayValue = attributesValue[JSON_ONE_WAY_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!oneWayValue.IsBool()) {
		spdlog::error("Wall has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_ONE_WAY_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(oneWayValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool oneWay = oneWayValue.GetBool();

	// parse block hitscan attribute
	if(!attributesValue.HasMember(JSON_BLOCK_HITSCAN_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall attribute '{}' is missing.", JSON_BLOCK_HITSCAN_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & blockHitscanValue = attributesValue[JSON_BLOCK_HITSCAN_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!blockHitscanValue.IsBool()) {
		spdlog::error("Wall has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_BLOCK_HITSCAN_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(blockHitscanValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool blockHitscan = blockHitscanValue.GetBool();

	// parse translucent attribute
	if(!attributesValue.HasMember(JSON_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall attribute '{}' is missing.", JSON_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & translucentValue = attributesValue[JSON_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!translucentValue.IsBool()) {
		spdlog::error("Wall has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(translucentValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool translucent = translucentValue.GetBool();

	// parse y flipped attribute
	if(!attributesValue.HasMember(JSON_Y_FLIPPED_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall attribute '{}' is missing.", JSON_Y_FLIPPED_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & yFlippedValue = attributesValue[JSON_Y_FLIPPED_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!yFlippedValue.IsBool()) {
		spdlog::error("Wall has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_Y_FLIPPED_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(yFlippedValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool yFlipped = yFlippedValue.GetBool();

	// parse reverse translucent attribute
	if(!attributesValue.HasMember(JSON_REVERSE_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall attribute '{}' is missing.", JSON_REVERSE_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & reverseTranslucentValue = attributesValue[JSON_REVERSE_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!reverseTranslucentValue.IsBool()) {
		spdlog::error("Wall has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_REVERSE_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(reverseTranslucentValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool reverseTranslucent = reverseTranslucentValue.GetBool();

	// parse reserved attribute
	if(!attributesValue.HasMember(JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Wall attribute '{}' is missing.", JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & reservedValue = attributesValue[JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!reservedValue.IsUint()) {
		spdlog::error("Wall has an invalid '{}' attribute type: '{}', expected unsigned integer 'number'.", JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(reservedValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	uint32_t reserved = reservedValue.GetUint();

	if(reserved > 63) {
		spdlog::error("Wall has an invalid '{}' attribute value: {}, expected integer 'number' between 0 and {}, inclusively.", JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME, reserved, 63);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	return Attributes({ blockClipping, invisibleWallBottomSwapped, textureAlignBottom, xFlipped, masked, oneWay, blockHitscan, translucent, yFlipped, reverseTranslucent, static_cast<uint8_t>(reserved) });
}

std::optional<Wall::Attributes> Wall::Attributes::parseFrom(const rapidjson::Value & attributesValue) {
	bool error = false;

	Attributes value(parseFrom(attributesValue, &error));

	if(error) {
		return {};
	}

	return value;
}

bool Wall::Attributes::operator == (const Attributes & attributes) const {
	return rawValue == attributes.rawValue;
}

bool Wall::Attributes::operator != (const Attributes & attributes) const {
	return !operator == (attributes);
}
