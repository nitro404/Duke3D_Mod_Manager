#ifndef _WALL_H_
#define _WALL_H_

#include "TaggedItem.h"
#include "TexturedItem.h"

#include <Point2D.h>

#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <optional>

class ByteBuffer;

class Wall final
	: public TaggedItem
	, public TexturedItem {
public:
	struct Attributes final {
		union {
			struct {
				bool blockClipping              : 1;
				bool invisibleWallBottomSwapped : 1;
				bool textureAlignBottom         : 1;
				bool xFlipped                   : 1;
				bool masked                     : 1;
				bool oneWay                     : 1;
				bool blockHitscan               : 1;
				bool translucent                : 1;
				bool yFlipped                   : 1;
				bool reverseTranslucent         : 1;
				uint8_t reserved                : 6;
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

	Wall();
	Wall(const Point2D & position, uint16_t nextWallIndex, uint16_t adjacentWallIndex, uint16_t nextSectorIndex, Attributes attributes, TexturedItem texturedItem, uint16_t maskedTileNumber, uint8_t xRepeat, uint8_t yRepeat, uint8_t xPanning, uint8_t yPanning, TaggedItem taggedItem);
	Wall(const Point2D & position, uint16_t nextWallIndex, uint16_t adjacentWallIndex, uint16_t nextSectorIndex, Attributes attributes, TexturedItem texturedItem, uint16_t maskedTileNumber, uint8_t xRepeat, uint8_t yRepeat, uint8_t xPanning, uint8_t yPanning, uint16_t lowTag, uint16_t highTag, uint16_t extra);
	Wall(int32_t xPosition, int32_t yPosition, uint16_t nextWallIndex, uint16_t adjacentWallIndex, uint16_t nextSectorIndex, Attributes attributes, TexturedItem texturedItem, uint16_t maskedTileNumber, uint8_t xRepeat, uint8_t yRepeat, uint8_t xPanning, uint8_t yPanning, TaggedItem taggedItem);
	Wall(int32_t xPosition, int32_t yPosition, uint16_t nextWallIndex, uint16_t adjacentWallIndex, uint16_t nextSectorIndex, Attributes attributes, TexturedItem texturedItem, uint16_t maskedTileNumber, uint8_t xRepeat, uint8_t yRepeat, uint8_t xPanning, uint8_t yPanning, uint16_t lowTag, uint16_t highTag, uint16_t extra);
	Wall(const Point2D & position, uint16_t nextWallIndex, uint16_t adjacentWallIndex, uint16_t nextSectorIndex, Attributes attributes, uint16_t tileNumber, uint16_t maskedTileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t xRepeat, uint8_t yRepeat, uint8_t xPanning, uint8_t yPanning, TaggedItem taggedItem);
	Wall(const Point2D & position, uint16_t nextWallIndex, uint16_t adjacentWallIndex, uint16_t nextSectorIndex, Attributes attributes, uint16_t tileNumber, uint16_t maskedTileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t xRepeat, uint8_t yRepeat, uint8_t xPanning, uint8_t yPanning, uint16_t lowTag, uint16_t highTag, uint16_t extra);
	Wall(int32_t xPosition, int32_t yPosition, uint16_t nextWallIndex, uint16_t adjacentWallIndex, uint16_t nextSectorIndex, Attributes attributes, uint16_t tileNumber, uint16_t maskedTileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t xRepeat, uint8_t yRepeat, uint8_t xPanning, uint8_t yPanning, TaggedItem taggedItem);
	Wall(int32_t xPosition, int32_t yPosition, uint16_t nextWallIndex, uint16_t adjacentWallIndex, uint16_t nextSectorIndex, Attributes attributes, uint16_t tileNumber, uint16_t maskedTileNumber, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t xRepeat, uint8_t yRepeat, uint8_t xPanning, uint8_t yPanning, uint16_t lowTag, uint16_t highTag, uint16_t extra);
	Wall(Wall && sector) noexcept;
	Wall(const Wall & wall);
	Wall & operator = (Wall && wall) noexcept;
	Wall & operator = (const Wall & wall);
	virtual ~Wall();

	bool isClippingBlocked() const;
	void setBlockClipping(bool blockClipping);
	bool isInvisibleWallBottomSwapped() const;
	void setInvisibleWallBottomSwapped(bool invisibleWallBottomSwapped);
	bool isTextureBottomAligned() const;
	void setTextureBottomAligned(bool textureAlignBottom);
	bool isXFlipped() const;
	void setXFlipped(bool xFlipped);
	bool isYFlipped() const;
	void setYFlipped(bool yFlipped);
	bool isMasked() const;
	void setMasked(bool masked);
	bool isOneWay() const;
	void setOneWay(bool oneWay);
	bool isHitscanBlocked() const;
	void setBlockHitscan(bool blockHitscan);
	bool isTranslucent() const;
	void setTranslucent(bool translucent);
	bool isReverseTranslucent() const;
	void setReverseTranslucent(bool reverseTranslucent);
	uint8_t getReserved() const;
	bool setReserved(uint8_t reserved);
	Attributes & getAttributes();
	const Attributes & getAttributes() const;
	void setAttributes(Attributes attributes);
	const Point2D & getPosition() const;
	void setPosition(const Point2D & position);
	uint16_t getNextWallIndex() const;
	void setNextWallIndex(uint16_t nextWallIndex);
	uint16_t getAdjacentWallIndex() const;
	void setAdjacentWallIndex(uint16_t adjacentWallIndex);
	uint16_t getNextSectorIndex() const;
	void setNextSectorIndex(uint16_t nextSectorIndex);
	uint16_t getMaskedTileNumber() const;
	void setMaskedTileNumber(uint16_t maskedTileNumber);
	uint8_t getXRepeat() const;
	void setXRepeat(uint8_t xRepeat);
	uint8_t getYRepeat() const;
	void setYRepeat(uint8_t yRepeat);
	uint8_t getXPanning() const;
	void setXPanning(uint8_t xPanning);
	uint8_t getYPanning() const;
	void setYPanning(uint8_t yPanning);

	static std::unique_ptr<Wall> getFrom(const ByteBuffer & byteBuffer, size_t offset, uint32_t mapVersion);
	static std::unique_ptr<Wall> readFrom(const ByteBuffer & byteBuffer, uint32_t mapVersion);
	bool putIn(ByteBuffer & byteBuffer, size_t offset, uint32_t mapVersion) const;
	bool insertIn(ByteBuffer & byteBuffer, size_t offset, uint32_t mapVersion) const;
	bool writeTo(ByteBuffer & byteBuffer, uint32_t mapVersion) const;

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<Wall> parseFrom(const rapidjson::Value & wallValue);

	bool operator == (const Wall & wall) const;
	bool operator != (const Wall & wall) const;

	static constexpr size_t SIZE_BYTES = TaggedItem::SIZE_BYTES + Point2D::SIZE_BYTES + (sizeof(int16_t) * 6) + sizeof(int8_t) + (sizeof(uint8_t) * 5);

private:
	Point2D m_position;
	uint16_t m_nextWallIndex;
	uint16_t m_adjacentWallIndex;
	uint16_t m_nextSectorIndex;
	Attributes m_attributes;
	uint16_t m_maskedTileNumber;
	uint8_t m_xRepeat;
	uint8_t m_yRepeat;
	uint8_t m_xPanning;
	uint8_t m_yPanning;
};

#endif // _WALL_H_
