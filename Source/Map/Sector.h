#ifndef _SECTOR_H_
#define _SECTOR_H_

#include "Partition.h"
#include "TaggedItem.h"

#include <cstdint>
#include <memory>
#include <vector>

class ByteBuffer;

class Sector final : public TaggedItem {
public:
	Sector();
	Sector(uint16_t firstWallIndex, uint16_t numberOfWalls, int32_t ceilingHeight, Partition::Attributes ceilingAttributes, int16_t ceilingTileNumber, int16_t ceilingSlope, int8_t ceilingShade, uint8_t ceilingPaletteLookupTableNumber, uint8_t ceilingXPanning, uint8_t ceilingYPanning, int32_t floorHeight, Partition::Attributes floorAttributes, int16_t floorTileNumber, int16_t floorSlope, int8_t floorShade, uint8_t floorPaletteLookupTableNumber, uint8_t floorXPanning, uint8_t floorYPanning, uint8_t visibility, TaggedItem taggedItem, uint8_t filler, const std::vector<uint8_t> & trailingData);
	Sector(uint16_t firstWallIndex, uint16_t numberOfWalls, int32_t ceilingHeight, Partition::Attributes ceilingAttributes, int16_t ceilingTileNumber, int16_t ceilingSlope, int8_t ceilingShade, uint8_t ceilingPaletteLookupTableNumber, uint8_t ceilingXPanning, uint8_t ceilingYPanning, int32_t floorHeight, Partition::Attributes floorAttributes, int16_t floorTileNumber, int16_t floorSlope, int8_t floorShade, uint8_t floorPaletteLookupTableNumber, uint8_t floorXPanning, uint8_t floorYPanning, uint8_t visibility, uint16_t lowTag, uint16_t highTag, uint16_t extra, uint8_t filler, const std::vector<uint8_t> & trailingData);
	Sector(uint16_t firstWallIndex, uint16_t numberOfWalls, int32_t ceilingHeight, Partition::Attributes ceilingAttributes, TexturedItem texturedCeilingItem, int16_t ceilingSlope, uint8_t ceilingXPanning, uint8_t ceilingYPanning, int32_t floorHeight, Partition::Attributes floorAttributes, TexturedItem texturedFloorItem, int16_t floorSlope, uint8_t floorXPanning, uint8_t floorYPanning, uint8_t visibility, TaggedItem taggedItem, uint8_t filler, const std::vector<uint8_t> & trailingData);
	Sector(uint16_t firstWallIndex, uint16_t numberOfWalls, int32_t ceilingHeight, Partition::Attributes ceilingAttributes, TexturedItem texturedCeilingItem, int16_t ceilingSlope, uint8_t ceilingXPanning, uint8_t ceilingYPanning, int32_t floorHeight, Partition::Attributes floorAttributes, TexturedItem texturedFloorItem, int16_t floorSlope, uint8_t floorXPanning, uint8_t floorYPanning, uint8_t visibility, uint16_t lowTag, uint16_t highTag, uint16_t extra, uint8_t filler, const std::vector<uint8_t> & trailingData);
	Sector(uint16_t firstWallIndex, uint16_t numberOfWalls, std::unique_ptr<Partition> ceiling, std::unique_ptr<Partition> floor, uint8_t visibility, TaggedItem taggedItem, uint8_t filler, const std::vector<uint8_t> & trailingData);
	Sector(uint16_t firstWallIndex, uint16_t numberOfWalls, std::unique_ptr<Partition> ceiling, std::unique_ptr<Partition> floor, uint8_t visibility, uint16_t lowTag, uint16_t highTag, uint16_t extra, uint8_t filler, const std::vector<uint8_t> & trailingData);
	Sector(Sector && sector) noexcept;
	Sector(const Sector & sector);
	Sector & operator = (Sector && sector) noexcept;
	Sector & operator = (const Sector & sector);
	virtual ~Sector();

	bool isCeilingParallaxed() const;
	void setCeilingParallaxed(bool parallaxed);
	bool isCeilingSloped() const;
	void setCeilingSloped(bool sloped);
	bool isCeilingTextureSwappedXY() const;
	void setCeilingTextureSwappedXY(bool swapTextureXY);
	bool isCeilingSmooshinessDoubled() const;
	void setCeilingSmooshinessDoubled(bool doubleSmooshiness);
	bool isCeilingXFlipped() const;
	void setCeilingXFlipped(bool xFlipped);
	bool isCeilingYFlipped() const;
	void setCeilingYFlipped(bool yFlipped);
	bool isCeilingTextureAligned() const;
	void setCeilingTextureAligned(bool textureAlign);
	uint16_t getCeilingReserved() const;
	bool setCeilingReserved(uint16_t reserved);
	Partition::Attributes getCeilingAttributes() const;
	void setCeilingAttributes(Partition::Attributes attributes);
	int32_t getCeilingHeight() const;
	void setCeilingHeight(int32_t height);
	int16_t getCeilingSlope() const;
	void setCeilingSlope(int16_t slope);
	uint8_t getCeilingXPanning() const;
	void setCeilingXPanning(uint8_t xPanning);
	uint8_t getCeilingYPanning() const;
	void setCeilingYPanning(uint8_t yPanning);
	std::shared_ptr<Partition> getCeiling() const;
	void setCeiling(const Partition & ceiling);
	void setCeiling(std::unique_ptr<Partition> ceiling);
	bool isFloorParallaxed() const;
	void setFloorParallaxed(bool parallaxed);
	bool isFloorSloped() const;
	void setFloorSloped(bool sloped);
	bool isFloorTextureSwappedXY() const;
	void setFloorTextureSwappedXY(bool swapTextureXY);
	bool isFloorSmooshinessDoubled() const;
	void setFloorSmooshinessDoubled(bool doubleSmooshiness);
	bool isFloorXFlipped() const;
	void setFloorXFlipped(bool xFlipped);
	bool isFloorYFlipped() const;
	void setFloorYFlipped(bool yFlipped);
	bool isFloorTextureAligned() const;
	void setFloorTextureAligned(bool textureAlign);
	uint16_t getFloorReserved() const;
	bool setFloorReserved(uint16_t reserved);
	Partition::Attributes getFloorAttributes() const;
	void setFloorAttributes(Partition::Attributes attributes);
	int32_t getFloorHeight() const;
	void setFloorHeight(int32_t height);
	int16_t getFloorSlope() const;
	void setFloorSlope(int16_t slope);
	uint8_t getFloorXPanning() const;
	void setFloorXPanning(uint8_t xPanning);
	uint8_t getFloorYPanning() const;
	void setFloorYPanning(uint8_t yPanning);
	std::shared_ptr<Partition> getFloor() const;
	void setFloor(const Partition & floor);
	void setFloor(std::unique_ptr<Partition> floor);
	uint16_t getFirstWallIndex() const;
	void setFirstWallIndex(uint16_t firstWallIndex);
	uint16_t getNumberOfWalls() const;
	void setNumberOfWalls(uint16_t numberOfWalls);
	uint8_t getVisibility() const;
	void setVisibility(uint8_t visibility);
	uint8_t getFiller() const;
	void setFiller(uint8_t filler);
	const std::vector<uint8_t> & getTrailingData() const;
	bool setTrailingData(const std::vector<uint8_t> & trailingData);
	void clearTrailingData();

	static std::unique_ptr<Sector> getFrom(const ByteBuffer & byteBuffer, size_t offset, uint32_t mapVersion);
	static std::unique_ptr<Sector> readFrom(const ByteBuffer & byteBuffer, uint32_t mapVersion);
	bool putIn(ByteBuffer & byteBuffer, size_t offset, uint32_t mapVersion) const;
	bool insertIn(ByteBuffer & byteBuffer, size_t offset, uint32_t mapVersion) const;
	bool writeTo(ByteBuffer & byteBuffer, uint32_t mapVersion) const;

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<Sector> parseFrom(const rapidjson::Value & partitionValue);

	static size_t getSizeInBytes(uint32_t mapVersion);

	bool operator == (const Sector & sector) const;
	bool operator != (const Sector & sector) const;

private:
	uint16_t m_firstWallIndex;
	uint16_t m_numberOfWalls;
	std::shared_ptr<Partition> m_ceiling;
	std::shared_ptr<Partition> m_floor;
	uint8_t m_visibility;
	uint8_t m_filler;
	std::vector<uint8_t> m_trailingData;
};

#endif // _SECTOR_H_
