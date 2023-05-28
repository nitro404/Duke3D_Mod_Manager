#ifndef _SPRITE_H_
#define _SPRITE_H_

#include "SectorItem.h"
#include "TaggedItem.h"
#include "TexturedItem.h"
#include "Velocity3D.h"

#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <optional>

class ByteBuffer;
class Velocity3D;

class Sprite final
	: public SectorItem
	, public TaggedItem
	, public TexturedItem {
public:
	enum class DrawType : uint8_t {
		Face = 0,
		Wall = 1,
		Floor = 2,
		Sloped = 3,
		Default = Face
	};

	enum class SpecialSpriteType : uint8_t {
		SectorEffector = 1,
		Activator = 2,
		Touchplate = 3,
		ActivatorLocked = 4,
		MusicAndSoundEffects = 5,
		Locators = 6,
		Cycler = 7,
		MasterSwitch = 8,
		Respawn = 9
	};

	struct Attributes final {
		union {
			struct {
				bool blockClipping      : 1;
				bool translucent        : 1;
				bool xFlipped           : 1;
				bool yFlipped           : 1;
				DrawType drawType       : 2;
				bool oneSided           : 1;
				bool centered           : 1;
				bool blockHitscan       : 1;
				bool reverseTranslucent : 1;
				uint8_t reserved        : 5; // note: unused
				bool invisible          : 1;
			};

			uint16_t rawValue;
		};

		rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
		bool addToJSONObject(rapidjson::Value & value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
		static Attributes parseFrom(const rapidjson::Value & attributesValue, bool * error);
		static std::optional<Attributes> parseFrom(const rapidjson::Value & attributesValue);

		bool operator == (const Attributes & attributes) const;
		bool operator != (const Attributes & attributes) const;
	};

	Sprite();
	Sprite(int32_t xPosition, int32_t yPosition, int32_t zPosition, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, int16_t xVelocity, int16_t yVelocity, int16_t zVelocity, uint16_t lowTag, uint16_t highTag, uint16_t extra);
	Sprite(int32_t xPosition, int32_t yPosition, int32_t zPosition, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, int16_t xVelocity, int16_t yVelocity, int16_t zVelocity, TaggedItem taggedItem);
	Sprite(int32_t xPosition, int32_t yPosition, int32_t zPosition, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, Velocity3D velocity, uint16_t lowTag, uint16_t highTag, uint16_t extra);
	Sprite(int32_t xPosition, int32_t yPosition, int32_t zPosition, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, Velocity3D velocity, TaggedItem taggedItem);
	Sprite(Point3D position, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, int16_t xVelocity, int16_t yVelocity, int16_t zVelocity, uint16_t lowTag, uint16_t highTag, uint16_t extra);
	Sprite(Point3D position, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, int16_t xVelocity, int16_t yVelocity, int16_t zVelocity, TaggedItem taggedItem);
	Sprite(Point3D position, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, Velocity3D velocity, uint16_t lowTag, uint16_t highTag, uint16_t extra);
	Sprite(Point3D position, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, Velocity3D velocity, TaggedItem taggedItem);
	Sprite(int32_t xPosition, int32_t yPosition, int32_t zPosition, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, int16_t xVelocity, int16_t yVelocity, int16_t zVelocity, uint16_t lowTag, uint16_t highTag, uint16_t extra);
	Sprite(int32_t xPosition, int32_t yPosition, int32_t zPosition, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, int16_t xVelocity, int16_t yVelocity, int16_t zVelocity, TaggedItem taggedItem);
	Sprite(int32_t xPosition, int32_t yPosition, int32_t zPosition, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, Velocity3D velocity, uint16_t lowTag, uint16_t highTag, uint16_t extra);
	Sprite(int32_t xPosition, int32_t yPosition, int32_t zPosition, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, Velocity3D velocity, TaggedItem taggedItem);
	Sprite(Point3D position, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, int16_t xVelocity, int16_t yVelocity, int16_t zVelocity, uint16_t lowTag, uint16_t highTag, uint16_t extra);
	Sprite(Point3D position, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, int16_t xVelocity, int16_t yVelocity, int16_t zVelocity, TaggedItem taggedItem);
	Sprite(Point3D position, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, Velocity3D velocity, uint16_t lowTag, uint16_t highTag, uint16_t extra);
	Sprite(Point3D position, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, uint16_t sectorIndex, int16_t statusNumber, int16_t angle, int16_t owner, Velocity3D velocity, TaggedItem taggedItem);
	Sprite(SectorItem sectorItem, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, int16_t statusNumber, int16_t owner, int16_t xVelocity, int16_t yVelocity, int16_t zVelocity, uint16_t lowTag, uint16_t highTag, uint16_t extra);
	Sprite(SectorItem sectorItem, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, int16_t statusNumber, int16_t owner, int16_t xVelocity, int16_t yVelocity, int16_t zVelocity, TaggedItem taggedItem);
	Sprite(SectorItem sectorItem, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, int16_t statusNumber, int16_t owner, Velocity3D velocity, uint16_t lowTag, uint16_t highTag, uint16_t extra);
	Sprite(SectorItem sectorItem, Attributes attributes, uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, int16_t statusNumber, int16_t owner, Velocity3D velocity, TaggedItem taggedItem);
	Sprite(SectorItem sectorItem, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, int16_t statusNumber, int16_t owner, int16_t xVelocity, int16_t yVelocity, int16_t zVelocity, uint16_t lowTag, uint16_t highTag, uint16_t extra);
	Sprite(SectorItem sectorItem, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, int16_t statusNumber, int16_t owner, int16_t xVelocity, int16_t yVelocity, int16_t zVelocity, TaggedItem taggedItem);
	Sprite(SectorItem sectorItem, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, int16_t statusNumber, int16_t owner, Velocity3D velocity, uint16_t lowTag, uint16_t highTag, uint16_t extra);
	Sprite(SectorItem sectorItem, Attributes attributes, TexturedItem texturedItem, uint8_t clippingDistance, uint8_t filler, uint8_t xRepeat, uint8_t yRepeat, int8_t xOffset, int8_t yOffset, int16_t statusNumber, int16_t owner, Velocity3D velocity, TaggedItem taggedItem);
	Sprite(Sprite && sprite) noexcept;
	Sprite(const Sprite & sprite);
	Sprite & operator = (Sprite && sprite) noexcept;
	Sprite & operator = (const Sprite & sprite);
	virtual ~Sprite();

	bool isClippingBlocked() const;
	void setBlockClipping(bool clippingBlocked);
	bool isTranslucent() const;
	void setTranslucent(bool translucent);
	bool isXFlipped() const;
	void setXFlipped(bool xFlipped);
	bool isYFlipped() const;
	void setYFlipped(bool yFlipped);
	DrawType getDrawType() const;
	void setDrawType(DrawType drawType);
	bool isOneSided() const;
	void setOneSided(bool oneSided);
	bool isCentered() const;
	void setCentered(bool centered);
	bool isHitscanBlocked() const;
	void setBlockHitscan(bool hitscanBlocked);
	bool isReverseTranslucent() const;
	void setReverseTranslucent(bool reverseTranslucent);
	uint8_t getReserved() const;
	bool setReserved(uint8_t reserved);
	bool isInvisible() const;
	void setInvisible(bool invisible);
	Attributes & getAttributes();
	const Attributes & getAttributes() const;
	void setAttributes(Attributes attributes);
	uint8_t getClippingDistance() const;
	void setClippingDistance(uint8_t clippingDistance);
	uint8_t getFiller() const;
	void setFiller(uint8_t filler);
	uint8_t getXRepeat() const;
	void setXRepeat(uint8_t xRepeat);
	uint8_t getYRepeat() const;
	void setYRepeat(uint8_t yRepeat);
	int8_t getXOffset() const;
	void setXOffset(int8_t xOffset);
	int8_t getYOffset() const;
	void setYOffset(int8_t yOffset);
	int16_t getStatusNumber() const;
	void setStatusNumber(int16_t statusNumber);
	int16_t getOwner() const;
	void setOwner(int16_t owner);
	Velocity3D & getVelocity();
	const Velocity3D & getVelocity() const;
	void setVelocity(Velocity3D velocity);

	static std::unique_ptr<Sprite> getFrom(const ByteBuffer & byteBuffer, size_t offset, uint32_t mapVersion);
	static std::unique_ptr<Sprite> readFrom(const ByteBuffer & byteBuffer, uint32_t mapVersion);
	bool putIn(ByteBuffer & byteBuffer, size_t offset, uint32_t mapVersion) const;
	bool insertIn(ByteBuffer & byteBuffer, size_t offset, uint32_t mapVersion) const;
	bool writeTo(ByteBuffer & byteBuffer, uint32_t mapVersion) const;

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<Sprite> parseFrom(const rapidjson::Value & spriteValue);

	bool operator == (const Sprite & sprite) const;
	bool operator != (const Sprite & sprite) const;

	static constexpr size_t SIZE_BYTES = SectorItem::SIZE_BYTES + TaggedItem::SIZE_BYTES + Velocity3D::SIZE_BYTES + (sizeof(int16_t) * 4) + (sizeof(int8_t) * 3) + (sizeof(uint8_t) * 5);

private:
	Attributes m_attributes;
	uint8_t m_clippingDistance;
	uint8_t m_filler; // note: unused and not present in version 6
	uint8_t m_xRepeat;
	uint8_t m_yRepeat;
	int8_t m_xOffset;
	int8_t m_yOffset;
	int16_t m_statusNumber;
	int16_t m_owner;
	Velocity3D m_velocity;
};

#endif // _SPRITE_H_
