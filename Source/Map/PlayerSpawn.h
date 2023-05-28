#ifndef _PLAYER_SPAWN_H_
#define _PLAYER_SPAWN_H_

#include "SectorItem.h"

#include <Point3D.h>

#include <cstdint>
#include <optional>
#include <string>

class ByteBuffer;

class PlayerSpawn final
	: public SectorItem {
public:
	PlayerSpawn();
	PlayerSpawn(int32_t xPosition, int32_t yPosition, int32_t zPosition, int16_t angle, uint16_t sectorIndex);
	PlayerSpawn(int32_t position[3], int16_t angle, uint16_t sectorIndex);
	PlayerSpawn(const Point3D & position, int16_t angle, uint16_t sectorIndex);
	PlayerSpawn(PlayerSpawn && playerSpawn) noexcept;
	PlayerSpawn(const PlayerSpawn & playerSpawn);
	PlayerSpawn(const SectorItem & sectorItem);
	PlayerSpawn & operator = (PlayerSpawn && playerSpawn) noexcept;
	PlayerSpawn & operator = (const PlayerSpawn & playerSpawn);
	virtual ~PlayerSpawn();

	static PlayerSpawn getFrom(const ByteBuffer & byteBuffer, size_t offset, bool * error);
	static std::optional<PlayerSpawn> getFrom(const ByteBuffer & byteBuffer, size_t offset);
	static PlayerSpawn readFrom(const ByteBuffer & byteBuffer, bool * error);
	static std::optional<PlayerSpawn> readFrom(const ByteBuffer & byteBuffer);
	bool putIn(ByteBuffer & byteBuffer, size_t offset) const;
	bool insertIn(ByteBuffer & byteBuffer, size_t offset) const;
	bool writeTo(ByteBuffer & byteBuffer) const;

	static PlayerSpawn parseFrom(const rapidjson::Value & sectorItemValue, bool * error);
	static std::optional<PlayerSpawn> parseFrom(const rapidjson::Value & sectorItemValue);

	std::string toString() const;

	bool operator == (const PlayerSpawn & playerSpawn) const;
	bool operator != (const PlayerSpawn & playerSpawn) const;

	static constexpr size_t SIZE_BYTES = Point3D::SIZE_BYTES + (sizeof(int16_t) * 2);
	static const Point3D DEFAULT_POSITION;
	static constexpr int16_t DEFAULT_ANGLE = 1536;
};

#endif // _PLAYER_SPAWN_H_
