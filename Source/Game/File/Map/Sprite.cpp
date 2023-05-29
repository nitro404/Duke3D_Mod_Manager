#include "Sprite.h"

#include "BuildConstants.h"

#include <ByteBuffer.h>
#include <Utilities/RapidJSONUtilities.h>

#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <string>

static const std::string JSON_ATTRIBUTES_PROPERTY_NAME("attributes");
static const std::string JSON_TILE_NUMBER_PROPERTY_NAME("tileNumber");
static const std::string JSON_SHADE_PROPERTY_NAME("shade");
static const std::string JSON_PALETTE_LOOKUP_TABLE_NUMBER_PROPERTY_NAME("paletteLookupTableNumber");
static const std::string JSON_CLIPPING_DISTANCE_PROPERTY_NAME("clippingDistance");
static const std::string JSON_FILLER_PROPERTY_NAME("filler");
static const std::string JSON_X_REPEAT_PROPERTY_NAME("xRepeat");
static const std::string JSON_Y_REPEAT_PROPERTY_NAME("yRepeat");
static const std::string JSON_X_OFFSET_PROPERTY_NAME("xOffset");
static const std::string JSON_Y_OFFSET_PROPERTY_NAME("yOffset");
static const std::string JSON_STATUS_NUMBER_PROPERTY_NAME("statusNumber");
static const std::string JSON_OWNER_PROPERTY_NAME("owner");
static const std::string JSON_VELOCITY_PROPERTY_NAME("velocity");

static const std::string JSON_BLOCK_CLIPPING_ATTRIBUTE_PROPERTY_NAME("blockClipping");
static const std::string JSON_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME("translucent");
static const std::string JSON_X_FLIPPED_ATTRIBUTE_PROPERTY_NAME("xFlipped");
static const std::string JSON_Y_FLIPPED_ATTRIBUTE_PROPERTY_NAME("yFlipped");
static const std::string JSON_DRAW_TYPE_ATTRIBUTE_PROPERTY_NAME("drawType");
static const std::string JSON_ONE_SIDED_ATTRIBUTE_PROPERTY_NAME("oneSided");
static const std::string JSON_CENTERED_ATTRIBUTE_PROPERTY_NAME("centered");
static const std::string JSON_BLOCK_HITSCAN_ATTRIBUTE_PROPERTY_NAME("blockHitscan");
static const std::string JSON_REVERSE_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME("reverseTranslucent");
static const std::string JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME("reserved");
static const std::string JSON_INVISIBLE_ATTRIBUTE_PROPERTY_NAME("invisible");

Sprite::Sprite()
	: SectorItem(0, 0, 0, 0, 0)
	, TaggedItem(0, 0, 0)
	, TexturedItem(0, 0, 0)
	, m_attributes({ 0 })
	, m_clippingDistance(0)
	, m_filler(0)
	, m_xRepeat(0)
	, m_yRepeat(0)
	, m_xOffset(0)
	, m_yOffset(0)
	, m_statusNumber(0)
	, m_owner(0)
	, m_velocity(0, 0, 0) { }

Sprite::Sprite(int32_t xPosition, int32_t yPosition, int32_t zPosition, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, Velocity3D velocity, uint16_t lowTag, uint16_t highTag, uint16_t extra)
	: SectorItem(xPosition, yPosition, zPosition, angle, sectorIndex)
	, TaggedItem(lowTag, highTag, extra)
	, TexturedItem(tileNumber, shade, paletteLookupTableNumber)
	, m_attributes(attributes)
	, m_clippingDistance(clippingDistance)
	, m_filler(filler)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xOffset(xOffset)
	, m_yOffset(yOffset)
	, m_statusNumber(statusNumber)
	, m_owner(owner)
	, m_velocity(velocity) { }

Sprite::Sprite(int32_t xPosition, int32_t yPosition, int32_t zPosition, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, Velocity3D velocity, TaggedItem taggedItem)
	: SectorItem(xPosition, yPosition, zPosition, angle, sectorIndex)
	, TaggedItem(taggedItem)
	, TexturedItem(tileNumber, shade, paletteLookupTableNumber)
	, m_attributes(attributes)
	, m_clippingDistance(clippingDistance)
	, m_filler(filler)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xOffset(xOffset)
	, m_yOffset(yOffset)
	, m_statusNumber(statusNumber)
	, m_owner(owner)
	, m_velocity(velocity) { }

Sprite::Sprite(Point3D position, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, int16_t xVelocity, int16_t yVelocity, int16_t zVelocity, uint16_t lowTag, uint16_t highTag, uint16_t extra)
	: SectorItem(position, angle, sectorIndex)
	, TaggedItem(lowTag, highTag, extra)
	, TexturedItem(tileNumber, shade, paletteLookupTableNumber)
	, m_attributes(attributes)
	, m_clippingDistance(clippingDistance)
	, m_filler(filler)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xOffset(xOffset)
	, m_yOffset(yOffset)
	, m_statusNumber(statusNumber)
	, m_owner(owner)
	, m_velocity(xVelocity, yVelocity, zVelocity) { }

Sprite::Sprite(Point3D position, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, int16_t xVelocity, int16_t yVelocity, int16_t zVelocity, TaggedItem taggedItem)
	: SectorItem(position, angle, sectorIndex)
	, TaggedItem(taggedItem)
	, TexturedItem(tileNumber, shade, paletteLookupTableNumber)
	, m_attributes(attributes)
	, m_clippingDistance(clippingDistance)
	, m_filler(filler)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xOffset(xOffset)
	, m_yOffset(yOffset)
	, m_statusNumber(statusNumber)
	, m_owner(owner)
	, m_velocity(xVelocity, yVelocity, zVelocity) { }

Sprite::Sprite(Point3D position, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, Velocity3D velocity, uint16_t lowTag, uint16_t highTag, uint16_t extra)
	: SectorItem(position, angle, sectorIndex)
	, TaggedItem(lowTag, highTag, extra)
	, TexturedItem(tileNumber, shade, paletteLookupTableNumber)
	, m_attributes(attributes)
	, m_clippingDistance(clippingDistance)
	, m_filler(filler)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xOffset(xOffset)
	, m_yOffset(yOffset)
	, m_statusNumber(statusNumber)
	, m_owner(owner)
	, m_velocity(velocity) { }

Sprite::Sprite(Point3D position, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, Velocity3D velocity, TaggedItem taggedItem)
	: SectorItem(position, angle, sectorIndex)
	, TaggedItem(taggedItem)
	, TexturedItem(tileNumber, shade, paletteLookupTableNumber)
	, m_attributes(attributes)
	, m_clippingDistance(clippingDistance)
	, m_filler(filler)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xOffset(xOffset)
	, m_yOffset(yOffset)
	, m_statusNumber(statusNumber)
	, m_owner(owner)
	, m_velocity(velocity) { }

Sprite::Sprite(int32_t xPosition, int32_t yPosition, int32_t zPosition, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, Velocity3D velocity, uint16_t lowTag, uint16_t highTag, uint16_t extra)
	: SectorItem(xPosition, yPosition, zPosition, angle, sectorIndex)
	, TaggedItem(lowTag, highTag, extra)
	, TexturedItem(texturedItem)
	, m_attributes(attributes)
	, m_clippingDistance(clippingDistance)
	, m_filler(filler)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xOffset(xOffset)
	, m_yOffset(yOffset)
	, m_statusNumber(statusNumber)
	, m_owner(owner)
	, m_velocity(velocity) { }

Sprite::Sprite(int32_t xPosition, int32_t yPosition, int32_t zPosition, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, Velocity3D velocity, TaggedItem taggedItem)
	: SectorItem(xPosition, yPosition, zPosition, angle, sectorIndex)
	, TaggedItem(taggedItem)
	, TexturedItem(texturedItem)
	, m_attributes(attributes)
	, m_clippingDistance(clippingDistance)
	, m_filler(filler)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xOffset(xOffset)
	, m_yOffset(yOffset)
	, m_statusNumber(statusNumber)
	, m_owner(owner)
	, m_velocity(velocity) { }

Sprite::Sprite(Point3D position, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, int16_t xVelocity, int16_t yVelocity, int16_t zVelocity, uint16_t lowTag, uint16_t highTag, uint16_t extra)
	: SectorItem(position, angle, sectorIndex)
	, TaggedItem(lowTag, highTag, extra)
	, TexturedItem(texturedItem)
	, m_attributes(attributes)
	, m_clippingDistance(clippingDistance)
	, m_filler(filler)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xOffset(xOffset)
	, m_yOffset(yOffset)
	, m_statusNumber(statusNumber)
	, m_owner(owner)
	, m_velocity(xVelocity, yVelocity, zVelocity) { }

Sprite::Sprite(Point3D position, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, int16_t xVelocity, int16_t yVelocity, int16_t zVelocity, TaggedItem taggedItem)
	: SectorItem(position, angle, sectorIndex)
	, TaggedItem(taggedItem)
	, TexturedItem(texturedItem)
	, m_attributes(attributes)
	, m_clippingDistance(clippingDistance)
	, m_filler(filler)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xOffset(xOffset)
	, m_yOffset(yOffset)
	, m_statusNumber(statusNumber)
	, m_owner(owner)
	, m_velocity(xVelocity, yVelocity, zVelocity) { }

Sprite::Sprite(Point3D position, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, Velocity3D velocity, uint16_t lowTag, uint16_t highTag, uint16_t extra)
	: SectorItem(position, angle, sectorIndex)
	, TaggedItem(lowTag, highTag, extra)
	, TexturedItem(texturedItem)
	, m_attributes(attributes)
	, m_clippingDistance(clippingDistance)
	, m_filler(filler)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xOffset(xOffset)
	, m_yOffset(yOffset)
	, m_statusNumber(statusNumber)
	, m_owner(owner)
	, m_velocity(velocity) { }

Sprite::Sprite(Point3D position, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, Velocity3D velocity, TaggedItem taggedItem)
	: SectorItem(position, angle, sectorIndex)
	, TaggedItem(taggedItem)
	, TexturedItem(texturedItem)
	, m_attributes(attributes)
	, m_clippingDistance(clippingDistance)
	, m_filler(filler)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xOffset(xOffset)
	, m_yOffset(yOffset)
	, m_statusNumber(statusNumber)
	, m_owner(owner)
	, m_velocity(velocity) { }

Sprite::Sprite(SectorItem sectorItem, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, int16_t statusNumber, int16_t owner, int16_t xVelocity, int16_t yVelocity, int16_t zVelocity, uint16_t lowTag, uint16_t highTag, uint16_t extra)
	: SectorItem(sectorItem)
	, TaggedItem(lowTag, highTag, extra)
	, TexturedItem(tileNumber, shade, paletteLookupTableNumber)
	, m_attributes(attributes)
	, m_clippingDistance(clippingDistance)
	, m_filler(filler)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xOffset(xOffset)
	, m_yOffset(yOffset)
	, m_statusNumber(statusNumber)
	, m_owner(owner)
	, m_velocity(xVelocity, yVelocity, zVelocity) { }

Sprite::Sprite(SectorItem sectorItem, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, int16_t statusNumber, int16_t owner, int16_t xVelocity, int16_t yVelocity, int16_t zVelocity, TaggedItem taggedItem)
	: SectorItem(sectorItem)
	, TaggedItem(taggedItem)
	, TexturedItem(tileNumber, shade, paletteLookupTableNumber)
	, m_attributes(attributes)
	, m_clippingDistance(clippingDistance)
	, m_filler(filler)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xOffset(xOffset)
	, m_yOffset(yOffset)
	, m_statusNumber(statusNumber)
	, m_owner(owner)
	, m_velocity(xVelocity, yVelocity, zVelocity) { }

Sprite::Sprite(SectorItem sectorItem, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, int16_t statusNumber, int16_t owner, Velocity3D velocity, uint16_t lowTag, uint16_t highTag, uint16_t extra)
	: SectorItem(sectorItem)
	, TaggedItem(lowTag, highTag, extra)
	, TexturedItem(tileNumber, shade, paletteLookupTableNumber)
	, m_attributes(attributes)
	, m_clippingDistance(clippingDistance)
	, m_filler(filler)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xOffset(xOffset)
	, m_yOffset(yOffset)
	, m_statusNumber(statusNumber)
	, m_owner(owner)
	, m_velocity(velocity) { }

Sprite::Sprite(SectorItem sectorItem, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, int16_t statusNumber, int16_t owner, Velocity3D velocity, TaggedItem taggedItem)
	: SectorItem(sectorItem)
	, TaggedItem(taggedItem)
	, TexturedItem(tileNumber, shade, paletteLookupTableNumber)
	, m_attributes(attributes)
	, m_clippingDistance(clippingDistance)
	, m_filler(filler)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xOffset(xOffset)
	, m_yOffset(yOffset)
	, m_statusNumber(statusNumber)
	, m_owner(owner)
	, m_velocity(velocity) { }

Sprite::Sprite(SectorItem sectorItem, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, int16_t statusNumber, int16_t owner, int16_t xVelocity, int16_t yVelocity, int16_t zVelocity, uint16_t lowTag, uint16_t highTag, uint16_t extra)
	: SectorItem(sectorItem)
	, TaggedItem(lowTag, highTag, extra)
	, TexturedItem(texturedItem)
	, m_attributes(attributes)
	, m_clippingDistance(clippingDistance)
	, m_filler(filler)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xOffset(xOffset)
	, m_yOffset(yOffset)
	, m_statusNumber(statusNumber)
	, m_owner(owner)
	, m_velocity(xVelocity, yVelocity, zVelocity) { }

Sprite::Sprite(SectorItem sectorItem, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, int16_t statusNumber, int16_t owner, int16_t xVelocity, int16_t yVelocity, int16_t zVelocity, TaggedItem taggedItem)
	: SectorItem(sectorItem)
	, TaggedItem(taggedItem)
	, TexturedItem(texturedItem)
	, m_attributes(attributes)
	, m_clippingDistance(clippingDistance)
	, m_filler(filler)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xOffset(xOffset)
	, m_yOffset(yOffset)
	, m_statusNumber(statusNumber)
	, m_owner(owner)
	, m_velocity(xVelocity, yVelocity, zVelocity) { }

Sprite::Sprite(SectorItem sectorItem, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, int16_t statusNumber, int16_t owner, Velocity3D velocity, uint16_t lowTag, uint16_t highTag, uint16_t extra)
	: SectorItem(sectorItem)
	, TaggedItem(lowTag, highTag, extra)
	, TexturedItem(texturedItem)
	, m_attributes(attributes)
	, m_clippingDistance(clippingDistance)
	, m_filler(filler)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xOffset(xOffset)
	, m_yOffset(yOffset)
	, m_statusNumber(statusNumber)
	, m_owner(owner)
	, m_velocity(velocity) { }

Sprite::Sprite(SectorItem sectorItem, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, int16_t statusNumber, int16_t owner, Velocity3D velocity, TaggedItem taggedItem)
	: SectorItem(sectorItem)
	, TaggedItem(taggedItem)
	, TexturedItem(texturedItem)
	, m_attributes(attributes)
	, m_clippingDistance(clippingDistance)
	, m_filler(filler)
	, m_xRepeat(xRepeat)
	, m_yRepeat(yRepeat)
	, m_xOffset(xOffset)
	, m_yOffset(yOffset)
	, m_statusNumber(statusNumber)
	, m_owner(owner)
	, m_velocity(velocity) { }

Sprite::Sprite(Sprite && sprite) noexcept
	: SectorItem(std::move(sprite))
	, TaggedItem(std::move(sprite))
	, TexturedItem(std::move(sprite))
	, m_attributes(std::move(sprite.m_attributes))
	, m_clippingDistance(sprite.m_clippingDistance)
	, m_filler(sprite.m_filler)
	, m_xRepeat(sprite.m_xRepeat)
	, m_yRepeat(sprite.m_yRepeat)
	, m_xOffset(sprite.m_xOffset)
	, m_yOffset(sprite.m_yOffset)
	, m_statusNumber(sprite.m_statusNumber)
	, m_owner(sprite.m_owner)
	, m_velocity(std::move(sprite.m_velocity)) { }

Sprite::Sprite(const Sprite & sprite)
	: SectorItem(sprite)
	, TaggedItem(sprite)
	, TexturedItem(sprite)
	, m_attributes(sprite.m_attributes)
	, m_clippingDistance(sprite.m_clippingDistance)
	, m_filler(sprite.m_filler)
	, m_xRepeat(sprite.m_xRepeat)
	, m_yRepeat(sprite.m_yRepeat)
	, m_xOffset(sprite.m_xOffset)
	, m_yOffset(sprite.m_yOffset)
	, m_statusNumber(sprite.m_statusNumber)
	, m_owner(sprite.m_owner)
	, m_velocity(sprite.m_velocity) { }

Sprite & Sprite::operator = (Sprite && sprite) noexcept {
	if(this != &sprite) {
		SectorItem::operator = (std::move(sprite));
		TaggedItem::operator = (std::move(sprite));
		TexturedItem::operator = (std::move(sprite));

		m_attributes = std::move(sprite.m_attributes);
		m_clippingDistance = sprite.m_clippingDistance;
		m_filler = sprite.m_filler;
		m_xRepeat = sprite.m_xRepeat;
		m_yRepeat = sprite.m_yRepeat;
		m_xOffset = sprite.m_xOffset;
		m_yOffset = sprite.m_yOffset;
		m_statusNumber = sprite.m_statusNumber;
		m_owner = sprite.m_owner;
		m_velocity = std::move(sprite.m_velocity);
	}

	return *this;
}

Sprite & Sprite::operator = (const Sprite & sprite) {
	SectorItem::operator = (sprite);
	TaggedItem::operator = (sprite);
		TexturedItem::operator = (sprite);

	m_attributes = sprite.m_attributes;
	m_clippingDistance = sprite.m_clippingDistance;
	m_filler = sprite.m_filler;
	m_xRepeat = sprite.m_xRepeat;
	m_yRepeat = sprite.m_yRepeat;
	m_xOffset = sprite.m_xOffset;
	m_yOffset = sprite.m_yOffset;
	m_statusNumber = sprite.m_statusNumber;
	m_owner = sprite.m_owner;
	m_velocity = sprite.m_velocity;

	return *this;
}

Sprite::~Sprite() { }

bool Sprite::isClippingBlocked() const {
	return m_attributes.blockClipping;
}

void Sprite::setBlockClipping(bool clippingBlocked) {
	m_attributes.blockClipping = clippingBlocked;
}

bool Sprite::isTranslucent() const {
	return m_attributes.translucent;
}

void Sprite::setTranslucent(bool translucent) {
	m_attributes.translucent = translucent;
}

bool Sprite::isXFlipped() const {
	return m_attributes.xFlipped;
}

void Sprite::setXFlipped(bool xFlipped) {
	m_attributes.xFlipped = xFlipped;
}

bool Sprite::isYFlipped() const {
	return m_attributes.yFlipped;
}

void Sprite::setYFlipped(bool yFlipped) {
	m_attributes.yFlipped = yFlipped;
}

Sprite::DrawType Sprite::getDrawType() const {
	return m_attributes.drawType;
}

void Sprite::setDrawType(DrawType drawType) {
	m_attributes.drawType = drawType;
}

bool Sprite::isOneSided() const {
	return m_attributes.oneSided;
}

void Sprite::setOneSided(bool oneSided) {
	m_attributes.oneSided = oneSided;
}

bool Sprite::isCentered() const {
	return m_attributes.centered;
}

void Sprite::setCentered(bool centered) {
	m_attributes.centered = centered;
}

bool Sprite::isHitscanBlocked() const {
	return m_attributes.blockHitscan;
}

void Sprite::setBlockHitscan(bool hitscanBlocked) {
	m_attributes.blockHitscan = hitscanBlocked;
}

bool Sprite::isReverseTranslucent() const {
	return m_attributes.reverseTranslucent;
}

void Sprite::setReverseTranslucent(bool reverseTranslucent) {
	m_attributes.reverseTranslucent = reverseTranslucent;
}

uint8_t Sprite::getReserved() const {
	return m_attributes.reserved;
}

bool Sprite::setReserved(uint8_t reserved) {
	if(reserved > 31) {
		return false;
	}

	m_attributes.reserved = reserved;

	return true;
}

bool Sprite::isInvisible() const {
	return m_attributes.invisible;
}

void Sprite::setInvisible(bool invisible) {
	m_attributes.invisible = invisible;
}

Sprite::Attributes & Sprite::getAttributes() {
	return m_attributes;
}

const Sprite::Attributes & Sprite::getAttributes() const {
	return m_attributes;
}

void Sprite::setAttributes(Attributes attributes) {
	m_attributes = attributes;
}

uint8_t Sprite::getClippingDistance() const {
	return m_clippingDistance;
}

void Sprite::setClippingDistance(uint8_t clippingDistance) {
	m_clippingDistance = clippingDistance;
}

uint8_t Sprite::getFiller() const {
	return m_filler;
}

void Sprite::setFiller(uint8_t filler) {
	m_filler = filler;
}

uint8_t Sprite::getXRepeat() const {
	return m_xRepeat;
}

void Sprite::setXRepeat(uint8_t xRepeat) {
	m_xRepeat = xRepeat;
}

uint8_t Sprite::getYRepeat() const {
	return m_yRepeat;
}

void Sprite::setYRepeat(uint8_t yRepeat) {
	m_yRepeat = yRepeat;
}

int8_t Sprite::getXOffset() const {
	return m_xOffset;
}

void Sprite::setXOffset(int8_t xOffset) {
	m_xOffset = xOffset;
}

int8_t Sprite::getYOffset() const {
	return m_yOffset;
}

void Sprite::setYOffset(int8_t yOffset) {
	m_yOffset = yOffset;
}

int16_t Sprite::getStatusNumber() const {
	return m_statusNumber;
}

void Sprite::setStatusNumber(int16_t statusNumber) {
	m_statusNumber = statusNumber;
}

int16_t Sprite::getOwner() const {
	return m_owner;
}

void Sprite::setOwner(int16_t owner) {
	m_owner = owner;
}

Velocity3D & Sprite::getVelocity() {
	return m_velocity;
}

const Velocity3D & Sprite::getVelocity() const {
	return m_velocity;
}

void Sprite::setVelocity(Velocity3D velocity) {
	m_velocity = velocity;
}

// TODO: convert from java:
/*
bool Sprite::isSpecialSprite() const {
	return SpecialSprite.getSpecialSpriteByTileNumber(m_tileNumber).isValid();
}

bool Sprite::hasPrimarySound() const {
	if(getSpecialSprite() == SpecialSprite.MusicAndSFX) {
		return getLowTag() > 0 && getLowTag() < ECHO_EFFECT_OFFSET;
	}
	else if(isSwitchWithSoundOverride()) {
		return getHighTag() > 0 && getHighTag() < BuildConstants.MAX_NUMBER_OF_SOUNDS;
	}

	return false;
}

bool Sprite::hasSecondarySound() const {
	if(getSpecialSprite() != SpecialSprite.MusicAndSFX) {
		return false;
	}

	Sector sector = getSector();
	
	if(sector == null || !sector.hasLowTag()) {
		return false;
	}

	SpecialSectorTag specialSectorTag = SpecialSectorTag.getSpecialSectorTagByTagNumber(sector.getLowTag());

	if(!specialSectorTag.isValid() || !specialSectorTag.hasMultipleSounds()) {
		return false;
	}

	return true;
}

bool Sprite::isActivationSoundSprite() const {
	if(getSpecialSprite() != SpecialSprite.MusicAndSFX) {
		return false;
	}

	Sector sector = getSector();
	
	if(sector == null || !sector.hasLowTag()) {
		return false;
	}

	return true;
}

bool Sprite::isAmbientSoundSprite() {
	if(getSpecialSprite() != SpecialSprite.MusicAndSFX) {
		return false;
	}

	Sector sector = getSector();

	if(sector == null || sector.hasLowTag()) {
		return false;
	}

	return getLowTag() < ECHO_EFFECT_OFFSET;
}

bool Sprite::isEchoEffectSoundSprite() {
	if(getSpecialSprite() != SpecialSprite.MusicAndSFX) {
		return false;
	}

	Sector sector = getSector();

	if(sector == null || sector.hasLowTag()) {
		return false;
	}

	return getLowTag() >= ECHO_EFFECT_OFFSET &&
		   getLowTag() <= ECHO_EFFECT_OFFSET + MAX_ECHO_EFFECT;
}

bool Sprite::isSwitch() {
	return SwitchSprite.getSwitchSpriteByTileNumber(m_tileNumber).isValid();
}

bool Sprite::isSwitchWithSoundOverride() {
	return SwitchSprite.getSwitchSpriteByTileNumber(m_tileNumber).hasSoundOverride();
}

bool Sprite::isSwitchWithCustomSound() {
	return SwitchSprite.getSwitchSpriteByTileNumber(m_tileNumber).hasSoundOverride() &&
		   getHighTag() > 0 && getHighTag() <= BuildConstants.MAX_NUMBER_OF_SOUNDS;
}

SpecialSpriteType Sprite::getSpecialSprite() {
	return SpecialSprite.getSpecialSpriteByTileNumber(m_tileNumber);
}

int16_t Sprite::getPrimarySoundNumber() const {
	if(getSpecialSprite() == SpecialSprite.MusicAndSFX) {
		Sector sector = getSector();
		
		if(sector == null || getLowTag() >= ECHO_EFFECT_OFFSET) {
			return -1;
		}

		return (short) getLowTag();
	}
	else if(isSwitchWithSoundOverride()) {
		return (short) getHighTag();
	}

	return -1;
}

bool Sprite::setPrimarySoundNumber(short soundNumber) {
	if(soundNumber < 0 || soundNumber >= BuildConstants.MAX_NUMBER_OF_SOUNDS) {
		throw new IllegalArgumentException("Sound number value of " + soundNumber + " exceeds maximum number of sounds.");
	}

	if(getSpecialSprite() == SpecialSprite.MusicAndSFX) {
		setLowTag(soundNumber);

		return true;
	}
	else if(isSwitchWithSoundOverride()) {
		setHighTag(soundNumber);

		return true;
	}

	return false;
}

short Sprite::getSecondarySoundNumber() const {
	if(getSpecialSprite() != SpecialSprite.MusicAndSFX) {
		return -1;
	}

	Sector sector = getSector();
	
	if(sector == null || !sector.hasLowTag()) {
		return -1;
	}

	SpecialSectorTag specialSectorTag = SpecialSectorTag.getSpecialSectorTagByTagNumber(sector.getLowTag());

	if(!specialSectorTag.isValid() || !specialSectorTag.hasMultipleSounds()) {
		return -1;
	}

	return (short) getHighTag();
}

bool Sprite::setSecondarySoundNumber(short soundNumber) {
	if(soundNumber < 0 || soundNumber >= BuildConstants.MAX_NUMBER_OF_SOUNDS) {
		throw new IllegalArgumentException("Sound number value of " + soundNumber + " exceeds maximum number of sounds.");
	}

	if(getSpecialSprite() != SpecialSprite.MusicAndSFX) {
		return false;
	}

	Sector sector = getSector();

	if(sector == null || !sector.hasLowTag()) {
		return false;
	}

	SpecialSectorTag specialSectorTag = SpecialSectorTag.getSpecialSectorTagByTagNumber(sector.getLowTag());

	if(!specialSectorTag.isValid() || !specialSectorTag.hasMultipleSounds()) {
		return false;
	}

	setHighTag(soundNumber);

	return true;
}
*/

std::unique_ptr<Sprite> Sprite::getFrom(const ByteBuffer & byteBuffer, size_t offset, uint32_t mapVersion) {
	uint16_t tileNumber = 0;
	uint16_t sectorIndex = 0;
	uint8_t filler = 0;
	int16_t statusNumber = 0;
	int16_t owner = 0;
	bool error = false;
	size_t newOffset = offset;

	Point3D position(Point3D::getFrom(byteBuffer, newOffset, &error));

	if(error) {
		return nullptr;
	}

	newOffset += Point3D::SIZE_BYTES;

	uint16_t rawAttributesValue = byteBuffer.getUnsignedShort(newOffset, &error);

	if(error) {
		return nullptr;
	}

	Attributes attributes;
	attributes.rawValue = rawAttributesValue;

	newOffset += sizeof(uint16_t);

	if(mapVersion != 6) {
		tileNumber = byteBuffer.getUnsignedShort(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint16_t);
	}

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

	uint8_t clippingDistance = byteBuffer.getUnsignedByte(newOffset, &error);

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

	int8_t xOffset = byteBuffer.getByte(newOffset, &error);

	if(error) {
		return nullptr;
	}

	newOffset += sizeof(int8_t);

	int8_t yOffset = byteBuffer.getByte(newOffset, &error);

	if(error) {
		return nullptr;
	}

	newOffset += sizeof(int8_t);

	if(mapVersion == 6) {
		tileNumber = byteBuffer.getUnsignedShort(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint16_t);
	}
	else {
		sectorIndex = byteBuffer.getUnsignedShort(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint16_t);

		statusNumber = byteBuffer.getShort(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(int16_t);
	}

	int16_t angle = byteBuffer.getShort(newOffset, &error);

	if(error) {
		return nullptr;
	}

	newOffset += sizeof(int16_t);

	if(mapVersion != 6) {
		owner = byteBuffer.getShort(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(int16_t);
	}

	Velocity3D velocity(Velocity3D::getFrom(byteBuffer, newOffset, &error));

	if(error) {
		return nullptr;
	}

	newOffset += Velocity3D::SIZE_BYTES;

	if(mapVersion == 6) {
		owner = byteBuffer.getShort(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(int16_t);

		sectorIndex = byteBuffer.getUnsignedShort(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(uint16_t);

		statusNumber = byteBuffer.getShort(newOffset, &error);

		if(error) {
			return nullptr;
		}

		newOffset += sizeof(int16_t);
	}

	TaggedItem taggedItem(TaggedItem::getFrom(byteBuffer, newOffset, &error));

	if(error) {
		return nullptr;
	}

	newOffset += TaggedItem::SIZE_BYTES;

	if(mapVersion == 6) {
		filler = byteBuffer.getUnsignedByte(newOffset, &error);

		if(error) {
			return nullptr;
		}
	}

	return std::make_unique<Sprite>(position, attributes, tileNumber, shade, paletteLookupTableNumber, clippingDistance, filler, xRepeat, yRepeat, xOffset, yOffset, sectorIndex, statusNumber, angle, owner, velocity, taggedItem);
}

std::unique_ptr<Sprite> Sprite::readFrom(const ByteBuffer & byteBuffer, uint32_t mapVersion) {
	std::unique_ptr<Sprite> value(getFrom(byteBuffer, byteBuffer.getReadOffset(), mapVersion));

	if(value != nullptr) {
		byteBuffer.skipReadBytes(SIZE_BYTES);
	}

	return value;
}

bool Sprite::putIn(ByteBuffer & byteBuffer, size_t offset, uint32_t mapVersion) const {
	size_t newOffset = offset;

	if(!m_position.putIn(byteBuffer, newOffset)) {
		return false;
	}

	newOffset += Point3D::SIZE_BYTES;

	if(!byteBuffer.putUnsignedShort(m_attributes.rawValue, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(mapVersion != 6) {
		if(!byteBuffer.putUnsignedShort(m_tileNumber, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint16_t);
	}

	if(!byteBuffer.putByte(m_shade, newOffset)) {
		return false;
	}

	newOffset += sizeof(int8_t);

	if(!byteBuffer.putUnsignedByte(m_paletteLookupTableNumber, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(!byteBuffer.putUnsignedByte(m_clippingDistance, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(mapVersion != 6) {
		if(!byteBuffer.putUnsignedByte(m_filler, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);
	}

	if(!byteBuffer.putUnsignedByte(m_xRepeat, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(!byteBuffer.putUnsignedByte(m_yRepeat, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(!byteBuffer.putByte(m_xOffset, newOffset)) {
		return false;
	}

	newOffset += sizeof(int8_t);

	if(!byteBuffer.putByte(m_yOffset, newOffset)) {
		return false;
	}

	newOffset += sizeof(int8_t);

	if(mapVersion == 6) {
		if(!byteBuffer.putUnsignedShort(m_tileNumber, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint16_t);
	}
	else {
		if(!byteBuffer.putUnsignedShort(m_sectorIndex, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint16_t);

		if(!byteBuffer.putShort(m_statusNumber, newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);
	}

	if(!byteBuffer.putShort(m_angle, newOffset)) {
		return false;
	}

	newOffset += sizeof(int16_t);

	if(mapVersion != 6) {
		if(!byteBuffer.putShort(m_owner, newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);
	}

	if(!m_velocity.putIn(byteBuffer, newOffset)) {
		return false;
	}

	newOffset += Velocity3D::SIZE_BYTES;

	if(mapVersion == 6) {
		if(!byteBuffer.putShort(m_owner, newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);

		if(!byteBuffer.putUnsignedShort(m_sectorIndex, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint16_t);

		if(!byteBuffer.putShort(m_statusNumber, newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);
	}

	if(!TaggedItem::putIn(byteBuffer, newOffset)) {
		return false;
	}

	newOffset += TaggedItem::SIZE_BYTES;

	if(mapVersion == 6) {
		if(!byteBuffer.putUnsignedByte(m_filler, newOffset)) {
			return false;
		}
	}

	return true;
}

bool Sprite::insertIn(ByteBuffer & byteBuffer, size_t offset, uint32_t mapVersion) const {
	size_t newOffset = offset;

	if(!m_position.insertIn(byteBuffer, newOffset)) {
		return false;
	}

	newOffset += Point3D::SIZE_BYTES;

	if(!byteBuffer.insertUnsignedShort(m_attributes.rawValue, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(mapVersion != 6) {
		if(!byteBuffer.insertUnsignedShort(m_tileNumber, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint16_t);
	}

	if(!byteBuffer.insertByte(m_shade, newOffset)) {
		return false;
	}

	newOffset += sizeof(int8_t);

	if(!byteBuffer.insertUnsignedByte(m_paletteLookupTableNumber, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(!byteBuffer.insertUnsignedByte(m_clippingDistance, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(mapVersion != 6) {
		if(!byteBuffer.insertUnsignedByte(m_filler, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint8_t);
	}

	if(!byteBuffer.insertUnsignedByte(m_xRepeat, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(!byteBuffer.insertUnsignedByte(m_yRepeat, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint8_t);

	if(!byteBuffer.insertByte(m_xOffset, newOffset)) {
		return false;
	}

	newOffset += sizeof(int8_t);

	if(!byteBuffer.insertByte(m_yOffset, newOffset)) {
		return false;
	}

	newOffset += sizeof(int8_t);

	if(mapVersion == 6) {
		if(!byteBuffer.insertUnsignedShort(m_tileNumber, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint16_t);
	}
	else {
		if(!byteBuffer.insertUnsignedShort(m_sectorIndex, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint16_t);

		if(!byteBuffer.insertShort(m_statusNumber, newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);
	}

	if(!byteBuffer.insertShort(m_angle, newOffset)) {
		return false;
	}

	newOffset += sizeof(int16_t);

	if(mapVersion != 6) {
		if(!byteBuffer.insertShort(m_owner, newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);
	}

	if(!m_velocity.insertIn(byteBuffer, newOffset)) {
		return false;
	}

	newOffset += Velocity3D::SIZE_BYTES;

	if(mapVersion == 6) {
		if(!byteBuffer.insertShort(m_owner, newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);

		if(!byteBuffer.insertUnsignedShort(m_sectorIndex, newOffset)) {
			return false;
		}

		newOffset += sizeof(uint16_t);

		if(!byteBuffer.insertShort(m_statusNumber, newOffset)) {
			return false;
		}

		newOffset += sizeof(int16_t);
	}

	if(!TaggedItem::insertIn(byteBuffer, newOffset)) {
		return false;
	}

	newOffset += TaggedItem::SIZE_BYTES;

	if(mapVersion == 6) {
		if(!byteBuffer.insertUnsignedByte(m_filler, newOffset)) {
			return false;
		}
	}

	return true;
}

bool Sprite::writeTo(ByteBuffer & byteBuffer, uint32_t mapVersion) const {
	if(!m_position.writeTo(byteBuffer)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedShort(m_attributes.rawValue)) {
		return false;
	}

	if(mapVersion != 6) {
		if(!byteBuffer.writeUnsignedShort(m_tileNumber)) {
			return false;
		}
	}

	if(!byteBuffer.writeByte(m_shade)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedByte(m_paletteLookupTableNumber)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedByte(m_clippingDistance)) {
		return false;
	}

	if(mapVersion != 6) {
		if(!byteBuffer.writeUnsignedByte(m_filler)) {
			return false;
		}
	}

	if(!byteBuffer.writeUnsignedByte(m_xRepeat)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedByte(m_yRepeat)) {
		return false;
	}

	if(!byteBuffer.writeByte(m_xOffset)) {
		return false;
	}

	if(!byteBuffer.writeByte(m_yOffset)) {
		return false;
	}

	if(mapVersion == 6) {
		if(!byteBuffer.writeUnsignedShort(m_tileNumber)) {
			return false;
		}
	}
	else {
		if(!byteBuffer.writeUnsignedShort(m_sectorIndex)) {
			return false;
		}

		if(!byteBuffer.writeShort(m_statusNumber)) {
			return false;
		}
	}

	if(!byteBuffer.writeShort(m_angle)) {
		return false;
	}

	if(mapVersion != 6) {
		if(!byteBuffer.writeShort(m_owner)) {
			return false;
		}
	}

	if(!m_velocity.writeTo(byteBuffer)) {
		return false;
	}

	if(mapVersion == 6) {
		if(!byteBuffer.writeShort(m_owner)) {
			return false;
		}

		if(!byteBuffer.writeUnsignedShort(m_sectorIndex)) {
			return false;
		}

		if(!byteBuffer.writeShort(m_statusNumber)) {
			return false;
		}
	}

	if(!TaggedItem::writeTo(byteBuffer)) {
		return false;
	}

	if(mapVersion == 6) {
		if(!byteBuffer.writeUnsignedByte(m_filler)) {
			return false;
		}
	}

	return true;
}

rapidjson::Value Sprite::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value spriteValue(rapidjson::kObjectType);

	SectorItem::addToJSONObject(spriteValue, allocator);

	rapidjson::Value attributesValue(m_attributes.toJSON(allocator));
	spriteValue.AddMember(rapidjson::StringRef(JSON_ATTRIBUTES_PROPERTY_NAME.c_str()), attributesValue, allocator);

	TexturedItem::addToJSONObject(spriteValue, allocator);

	spriteValue.AddMember(rapidjson::StringRef(JSON_CLIPPING_DISTANCE_PROPERTY_NAME.c_str()), rapidjson::Value(m_clippingDistance), allocator);

	spriteValue.AddMember(rapidjson::StringRef(JSON_FILLER_PROPERTY_NAME.c_str()), rapidjson::Value(m_filler), allocator);

	spriteValue.AddMember(rapidjson::StringRef(JSON_X_REPEAT_PROPERTY_NAME.c_str()), rapidjson::Value(m_xRepeat), allocator);

	spriteValue.AddMember(rapidjson::StringRef(JSON_Y_REPEAT_PROPERTY_NAME.c_str()), rapidjson::Value(m_yRepeat), allocator);

	spriteValue.AddMember(rapidjson::StringRef(JSON_X_OFFSET_PROPERTY_NAME.c_str()), rapidjson::Value(m_xOffset), allocator);

	spriteValue.AddMember(rapidjson::StringRef(JSON_Y_OFFSET_PROPERTY_NAME.c_str()), rapidjson::Value(m_yOffset), allocator);

	spriteValue.AddMember(rapidjson::StringRef(JSON_STATUS_NUMBER_PROPERTY_NAME.c_str()), rapidjson::Value(m_statusNumber), allocator);

	spriteValue.AddMember(rapidjson::StringRef(JSON_OWNER_PROPERTY_NAME.c_str()), rapidjson::Value(m_owner), allocator);

	rapidjson::Value velocityValue(m_velocity.toJSON(allocator));
	spriteValue.AddMember(rapidjson::StringRef(JSON_VELOCITY_PROPERTY_NAME.c_str()), velocityValue, allocator);

	TaggedItem::addToJSONObject(spriteValue, allocator);

	return spriteValue;
}

std::unique_ptr<Sprite> Sprite::parseFrom(const rapidjson::Value & spriteValue) {
	if(!spriteValue.IsObject()) {
		spdlog::error("Invalid sprite type: '{}', expected 'object'.", Utilities::typeToString(spriteValue.GetType()));
		return nullptr;
	}

	// parse sector item information
	std::optional<SectorItem> optionalSectorItem(SectorItem::parseFrom(spriteValue));

	if(!optionalSectorItem.has_value()) {
		return nullptr;
	}

	// parse attributes
	if(!spriteValue.HasMember(JSON_ATTRIBUTES_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite is missing '{}' property.", JSON_ATTRIBUTES_PROPERTY_NAME);
		return nullptr;
	}

	std::optional<Attributes> optionalAttributes(Attributes::parseFrom(spriteValue[JSON_ATTRIBUTES_PROPERTY_NAME.c_str()]));

	if(!optionalAttributes.has_value()) {
		return nullptr;
	}

	// parse textured item information
	std::optional<TexturedItem> optionalTexturedItem(TexturedItem::parseFrom(spriteValue));

	if(!optionalTexturedItem.has_value()) {
		return nullptr;
	}

	// parse clipping distance
	if(!spriteValue.HasMember(JSON_CLIPPING_DISTANCE_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite is missing '{}' property.", JSON_CLIPPING_DISTANCE_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & clippingDistanceValue = spriteValue[JSON_CLIPPING_DISTANCE_PROPERTY_NAME.c_str()];

	if(!clippingDistanceValue.IsUint()) {
		spdlog::error("Sprite has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_CLIPPING_DISTANCE_PROPERTY_NAME, Utilities::typeToString(clippingDistanceValue.GetType()));
		return nullptr;
	}

	uint32_t clippingDistance = clippingDistanceValue.GetUint();

	if(clippingDistance > std::numeric_limits<uint8_t>::max()) {
		spdlog::error("Invalid sprite clipping distance: {}, expected a value between 0 and {}, inclusively.", clippingDistance, std::numeric_limits<uint8_t>::max());
		return nullptr;
	}

	// parse filler
	if(!spriteValue.HasMember(JSON_FILLER_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite is missing '{}' property.", JSON_FILLER_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & fillerValue = spriteValue[JSON_FILLER_PROPERTY_NAME.c_str()];

	if(!fillerValue.IsUint()) {
		spdlog::error("Sprite has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_FILLER_PROPERTY_NAME, Utilities::typeToString(fillerValue.GetType()));
		return nullptr;
	}

	uint32_t filler = fillerValue.GetUint();

	if(filler > std::numeric_limits<uint8_t>::max()) {
		spdlog::error("Invalid sprite filler: {}, expected a value between 0 and {}, inclusively.", filler, std::numeric_limits<uint8_t>::max());
		return nullptr;
	}

	// parse x repeat
	if(!spriteValue.HasMember(JSON_X_REPEAT_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite is missing '{}' property.", JSON_X_REPEAT_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & xRepeatValue = spriteValue[JSON_X_REPEAT_PROPERTY_NAME.c_str()];

	if(!xRepeatValue.IsUint()) {
		spdlog::error("Sprite has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_X_REPEAT_PROPERTY_NAME, Utilities::typeToString(xRepeatValue.GetType()));
		return nullptr;
	}

	uint32_t xRepeat = xRepeatValue.GetUint();

	if(xRepeat > std::numeric_limits<uint8_t>::max()) {
		spdlog::error("Invalid sprite x repeat: {}, expected a value between 0 and {}, inclusively.", xRepeat, std::numeric_limits<uint8_t>::max());
		return nullptr;
	}

	// parse y repeat
	if(!spriteValue.HasMember(JSON_Y_REPEAT_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite is missing '{}' property.", JSON_Y_REPEAT_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & yRepeatValue = spriteValue[JSON_Y_REPEAT_PROPERTY_NAME.c_str()];

	if(!yRepeatValue.IsUint()) {
		spdlog::error("Sprite has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_Y_REPEAT_PROPERTY_NAME, Utilities::typeToString(yRepeatValue.GetType()));
		return nullptr;
	}

	uint32_t yRepeat = yRepeatValue.GetUint();

	if(yRepeat > std::numeric_limits<uint8_t>::max()) {
		spdlog::error("Invalid sprite y repeat: {}, expected a value between 0 and {}, inclusively.", yRepeat, std::numeric_limits<uint8_t>::max());
		return nullptr;
	}

	// parse x offset
	if(!spriteValue.HasMember(JSON_X_OFFSET_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite is missing '{}' property.", JSON_X_OFFSET_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & xOffsetValue = spriteValue[JSON_X_OFFSET_PROPERTY_NAME.c_str()];

	if(!xOffsetValue.IsInt()) {
		spdlog::error("Sprite has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_X_OFFSET_PROPERTY_NAME, Utilities::typeToString(xOffsetValue.GetType()));
		return nullptr;
	}

	int32_t xOffset = xOffsetValue.GetInt();

	if(xOffset < std::numeric_limits<int8_t>::min() || xOffset > std::numeric_limits<int8_t>::max()) {
		spdlog::error("Invalid sprite x offset: {}, expected a value between {} and {}, inclusively.", xOffset, std::numeric_limits<int8_t>::min(), std::numeric_limits<int8_t>::max());
		return nullptr;
	}

	// parse y offset
	if(!spriteValue.HasMember(JSON_Y_OFFSET_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite is missing '{}' property.", JSON_Y_OFFSET_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & yOffsetValue = spriteValue[JSON_Y_OFFSET_PROPERTY_NAME.c_str()];

	if(!yOffsetValue.IsInt()) {
		spdlog::error("Sprite has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_Y_OFFSET_PROPERTY_NAME, Utilities::typeToString(yOffsetValue.GetType()));
		return nullptr;
	}

	int32_t yOffset = yOffsetValue.GetInt();

	if(yOffset < std::numeric_limits<int8_t>::min() || yOffset > std::numeric_limits<int8_t>::max()) {
		spdlog::error("Invalid sprite y offset: {}, expected a value between {} and {}, inclusively.", yOffset, std::numeric_limits<int8_t>::min(), std::numeric_limits<int8_t>::max());
		return nullptr;
	}

	// parse status number
	if(!spriteValue.HasMember(JSON_STATUS_NUMBER_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite is missing '{}' property.", JSON_STATUS_NUMBER_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & statusNumberValue = spriteValue[JSON_STATUS_NUMBER_PROPERTY_NAME.c_str()];

	if(!statusNumberValue.IsInt()) {
		spdlog::error("Sprite has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_STATUS_NUMBER_PROPERTY_NAME, Utilities::typeToString(statusNumberValue.GetType()));
		return nullptr;
	}

	int32_t statusNumber = statusNumberValue.GetInt();

	if(statusNumber < std::numeric_limits<int16_t>::min() || statusNumber > std::numeric_limits<int16_t>::max()) {
		spdlog::error("Invalid sprite status number: {}, expected a value between {} and {}, inclusively.", statusNumber, std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max());
		return nullptr;
	}

	// parse owner
	if(!spriteValue.HasMember(JSON_OWNER_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite is missing '{}' property.", JSON_OWNER_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & ownerValue = spriteValue[JSON_OWNER_PROPERTY_NAME.c_str()];

	if(!ownerValue.IsInt()) {
		spdlog::error("Sprite has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_OWNER_PROPERTY_NAME, Utilities::typeToString(ownerValue.GetType()));
		return nullptr;
	}

	int32_t owner = ownerValue.GetInt();

	if(owner < std::numeric_limits<int16_t>::min() || owner > std::numeric_limits<int16_t>::max()) {
		spdlog::error("Invalid sprite owner: {}, expected a value between {} and {}, inclusively.", owner, std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max());
		return nullptr;
	}

	// parse velocity
	if(!spriteValue.HasMember(JSON_VELOCITY_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite is missing '{}' property.", JSON_VELOCITY_PROPERTY_NAME);
		return nullptr;
	}

	std::optional<Velocity3D> optionalVelocity(Velocity3D::parseFrom(spriteValue[JSON_VELOCITY_PROPERTY_NAME.c_str()]));

	if(!optionalVelocity.has_value()) {
		return nullptr;
	}

	// parse tags
	std::optional<TaggedItem> optionalTaggedItem(TaggedItem::parseFrom(spriteValue));

	if(!optionalTaggedItem.has_value()) {
		return nullptr;
	}

	return std::make_unique<Sprite>(optionalSectorItem.value(), optionalAttributes.value(), optionalTexturedItem.value(), static_cast<uint8_t>(clippingDistance), static_cast<uint8_t>(filler), static_cast<uint8_t>(xRepeat), static_cast<uint8_t>(yRepeat), static_cast<int8_t>(xOffset), static_cast<int8_t>(yOffset), static_cast<int16_t>(statusNumber), static_cast<int16_t>(owner), optionalVelocity.value(), optionalTaggedItem.value());
}

bool Sprite::operator == (const Sprite & sprite) const {
	return SectorItem::operator == (sprite) &&
		   TaggedItem::operator == (sprite) &&
		   TexturedItem::operator == (sprite) &&
		   m_attributes == sprite.m_attributes &&
		   m_clippingDistance == sprite.m_clippingDistance &&
		   m_filler == sprite.m_filler &&
		   m_xRepeat == sprite.m_xRepeat &&
		   m_yRepeat == sprite.m_yRepeat &&
		   m_xOffset == sprite.m_xOffset &&
		   m_yOffset == sprite.m_yOffset &&
		   m_statusNumber == sprite.m_statusNumber &&
		   m_owner == sprite.m_owner &&
		   m_velocity == sprite.m_velocity;
}

bool Sprite::operator != (const Sprite & sprite) const {
	return !operator == (sprite);
}

rapidjson::Value Sprite::Attributes::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value attributesValue(rapidjson::kObjectType);

	addToJSONObject(attributesValue, allocator);

	return attributesValue;
}

bool Sprite::Attributes::addToJSONObject(rapidjson::Value & value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	if(!value.IsObject()) {
		return false;
	}

	value.AddMember(rapidjson::StringRef(JSON_BLOCK_CLIPPING_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(blockClipping), allocator);

	value.AddMember(rapidjson::StringRef(JSON_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(translucent), allocator);

	value.AddMember(rapidjson::StringRef(JSON_X_FLIPPED_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(xFlipped), allocator);

	value.AddMember(rapidjson::StringRef(JSON_Y_FLIPPED_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(yFlipped), allocator);

	rapidjson::Value drawTypeValue(std::string(magic_enum::enum_name(drawType)).c_str(), allocator);
	value.AddMember(rapidjson::StringRef(JSON_DRAW_TYPE_ATTRIBUTE_PROPERTY_NAME.c_str()), drawTypeValue, allocator);

	value.AddMember(rapidjson::StringRef(JSON_ONE_SIDED_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(oneSided), allocator);

	value.AddMember(rapidjson::StringRef(JSON_CENTERED_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(centered), allocator);

	value.AddMember(rapidjson::StringRef(JSON_BLOCK_HITSCAN_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(blockHitscan), allocator);

	value.AddMember(rapidjson::StringRef(JSON_REVERSE_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(reverseTranslucent), allocator);

	value.AddMember(rapidjson::StringRef(JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(reserved), allocator);

	value.AddMember(rapidjson::StringRef(JSON_INVISIBLE_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(invisible), allocator);

	return true;
}

Sprite::Attributes Sprite::Attributes::parseFrom(const rapidjson::Value & attributesValue, bool * error) {
	if(!attributesValue.IsObject()) {
		spdlog::error("Invalid sprite attributes type: '{}', expected 'object'.", Utilities::typeToString(attributesValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse block clipping attribute
	if(!attributesValue.HasMember(JSON_BLOCK_CLIPPING_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite attribute '{}' is missing.", JSON_BLOCK_CLIPPING_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & blockClippingValue = attributesValue[JSON_BLOCK_CLIPPING_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!blockClippingValue.IsBool()) {
		spdlog::error("Sprite has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_BLOCK_CLIPPING_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(blockClippingValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool blockClipping = blockClippingValue.GetBool();

	// parse translucent attribute
	if(!attributesValue.HasMember(JSON_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite attribute '{}' is missing.", JSON_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & translucentValue = attributesValue[JSON_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!translucentValue.IsBool()) {
		spdlog::error("Sprite has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(translucentValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool translucent = translucentValue.GetBool();

	// parse x flipped attribute
	if(!attributesValue.HasMember(JSON_X_FLIPPED_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite attribute '{}' is missing.", JSON_X_FLIPPED_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & xFlippedValue = attributesValue[JSON_X_FLIPPED_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!xFlippedValue.IsBool()) {
		spdlog::error("Sprite has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_X_FLIPPED_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(xFlippedValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool xFlipped = xFlippedValue.GetBool();

	// parse y flipped attribute
	if(!attributesValue.HasMember(JSON_Y_FLIPPED_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite attribute '{}' is missing.", JSON_Y_FLIPPED_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & yFlippedValue = attributesValue[JSON_Y_FLIPPED_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!yFlippedValue.IsBool()) {
		spdlog::error("Sprite has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_Y_FLIPPED_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(yFlippedValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool yFlipped = yFlippedValue.GetBool();

	// parse draw type attribute
	std::optional<DrawType> optionalDrawType;

	if(!attributesValue.HasMember(JSON_DRAW_TYPE_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite attribute '{}' is missing.", JSON_DRAW_TYPE_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & drawTypeValue = attributesValue[JSON_DRAW_TYPE_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(drawTypeValue.IsUint()) {
		optionalDrawType = magic_enum::enum_cast<DrawType>(drawTypeValue.GetUint());
	}
	else if(drawTypeValue.IsString()) {
		optionalDrawType = magic_enum::enum_cast<DrawType>(drawTypeValue.GetString());
	}
	else {
		spdlog::error("Sprite has an invalid '{}' attribute type: '{}', expected 'string' or unsigned integer 'number'.", JSON_DRAW_TYPE_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(drawTypeValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	if(!optionalDrawType.has_value()) {
		spdlog::error("Sprite has an invalid '{}' attribute value: {}.", JSON_DRAW_TYPE_ATTRIBUTE_PROPERTY_NAME, Utilities::valueToString(drawTypeValue));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse one sided attribute
	if(!attributesValue.HasMember(JSON_ONE_SIDED_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite attribute '{}' is missing.", JSON_ONE_SIDED_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & oneSidedValue = attributesValue[JSON_ONE_SIDED_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!oneSidedValue.IsBool()) {
		spdlog::error("Sprite has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_ONE_SIDED_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(oneSidedValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool oneSided = oneSidedValue.GetBool();

	// parse centered attribute
	if(!attributesValue.HasMember(JSON_CENTERED_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite attribute '{}' is missing.", JSON_CENTERED_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & centeredValue = attributesValue[JSON_CENTERED_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!centeredValue.IsBool()) {
		spdlog::error("Sprite has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_CENTERED_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(centeredValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool centered = centeredValue.GetBool();

	// parse block hitscan attribute
	if(!attributesValue.HasMember(JSON_BLOCK_HITSCAN_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite attribute '{}' is missing.", JSON_BLOCK_HITSCAN_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & blockHitscanValue = attributesValue[JSON_BLOCK_HITSCAN_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!blockHitscanValue.IsBool()) {
		spdlog::error("Sprite has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_BLOCK_HITSCAN_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(blockHitscanValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool blockHitscan = blockHitscanValue.GetBool();

	// parse reverse translucent attribute
	if(!attributesValue.HasMember(JSON_REVERSE_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite attribute '{}' is missing.", JSON_REVERSE_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & reverseTranslucentValue = attributesValue[JSON_REVERSE_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!reverseTranslucentValue.IsBool()) {
		spdlog::error("Sprite has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_REVERSE_TRANSLUCENT_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(reverseTranslucentValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool reverseTranslucent = reverseTranslucentValue.GetBool();

	// parse reserved attribute
	if(!attributesValue.HasMember(JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite attribute '{}' is missing.", JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & reservedValue = attributesValue[JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!reservedValue.IsUint()) {
		spdlog::error("Sprite has an invalid '{}' attribute type: '{}', expected unsigned integer 'number'.", JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(reservedValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	uint32_t reserved = reservedValue.GetUint();

	if(reserved > 31) {
		spdlog::error("Sprite has an invalid '{}' attribute value: {}, expected integer 'number' between 0 and {}, inclusively.", JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME, reserved, 31);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse invisible attribute
	if(!attributesValue.HasMember(JSON_INVISIBLE_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Sprite attribute '{}' is missing.", JSON_INVISIBLE_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & invisibleValue = attributesValue[JSON_INVISIBLE_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!invisibleValue.IsBool()) {
		spdlog::error("Sprite has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_INVISIBLE_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(invisibleValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool invisible = invisibleValue.GetBool();

	return Attributes({ blockClipping, translucent, xFlipped, yFlipped, optionalDrawType.value(), oneSided, centered, blockHitscan, reverseTranslucent, static_cast<uint8_t>(reserved), invisible });
}

std::optional<Sprite::Attributes> Sprite::Attributes::parseFrom(const rapidjson::Value & attributesValue) {
	bool error = false;

	Attributes value(parseFrom(attributesValue, &error));

	if(error) {
		return {};
	}

	return value;
}

bool Sprite::Attributes::operator == (const Attributes & attributes) const {
	return rawValue == attributes.rawValue;
}

bool Sprite::Attributes::operator != (const Attributes & attributes) const {
	return !operator == (attributes);
}
