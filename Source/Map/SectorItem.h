#ifndef _SECTOR_ITEM_H_
#define _SECTOR_ITEM_H_

#include <Point3D.h>

#include <rapidjson/document.h>

#include <cstdint>
#include <optional>

class SectorItem {
public:
	SectorItem();
	SectorItem(int32_t xPosition, int32_t yPosition, int32_t zPosition, int16_t angle, uint16_t sectorIndex);
	SectorItem(int32_t position[3], int16_t angle, uint16_t sectorIndex);
	SectorItem(const Point3D & position, int16_t angle, uint16_t sectorIndex);
	SectorItem(SectorItem && sectorItem) noexcept;
	SectorItem(const SectorItem & sectorItem);
	SectorItem & operator = (SectorItem && sectorItem) noexcept;
	SectorItem & operator = (const SectorItem & sectorItem);
	virtual ~SectorItem();

	int32_t getX() const;
	void setX(int32_t x);
	int32_t getY() const;
	void setY(int32_t y);
	int32_t getZ() const;
	void setZ(int32_t z);
	const Point3D & getPosition() const;
	void setPosition(int32_t x, int32_t y, int32_t z);
	void setPosition(int32_t position[3]);
	void setPosition(const Point3D & position);
	int16_t getAngle() const;
	bool setAngle(int16_t angle);
	double getAngleDegrees() const;
	bool setAngleDegrees(double angle);
	double getAngleRadians() const;
	bool setAngleRadians(double angle);
	uint16_t getSectorIndex() const;
	void setSectorIndex(uint16_t sectorIndex);

	static int16_t angleFromDegrees(double angleDegrees, bool * error);
	static int16_t angleFromRadians(double angleRadians, bool * error);
	static std::optional<int16_t> angleFromDegrees(double angleDegrees);
	static std::optional<int16_t> angleFromRadians(double angleRadians);
	static double angleToDegrees(int16_t angle);
	static double angleToRadians(int16_t angle);

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	bool addToJSONObject(rapidjson::Value & value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static SectorItem parseFrom(const rapidjson::Value & sectorItemValue, bool * error);
	static std::optional<SectorItem> parseFrom(const rapidjson::Value & sectorItemValue);

	bool operator == (const SectorItem & sectorItem) const;
	bool operator != (const SectorItem & sectorItem) const;

	static constexpr size_t SIZE_BYTES = Point3D::SIZE_BYTES + (sizeof(int16_t) * 2);

protected:
	Point3D m_position;
	int16_t m_angle;
	uint16_t m_sectorIndex;
};

#endif // _ROTATABLE_H_
