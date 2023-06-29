#include "PlayerSpawn.h"

#include "BuildConstants.h"

#include <ByteBuffer.h>
#include <Utilities/RapidJSONUtilities.h>

#include <spdlog/spdlog.h>

const Point3D PlayerSpawn::DEFAULT_POSITION(32768, 32768, 0);

PlayerSpawn::PlayerSpawn()
	: SectorItem(DEFAULT_POSITION, DEFAULT_ANGLE, 0) { }

PlayerSpawn::PlayerSpawn(int32_t xPosition, int32_t yPosition, int32_t zPosition, int16_t angle, uint16_t sectorIndex)
	: SectorItem(xPosition, yPosition, zPosition, angle, sectorIndex) { }

PlayerSpawn::PlayerSpawn(int32_t position[3], int16_t angle, uint16_t sectorIndex)
	: SectorItem(position, angle, sectorIndex) { }

PlayerSpawn::PlayerSpawn(const Point3D & position, int16_t angle, uint16_t sectorIndex)
	: SectorItem(position, angle, sectorIndex) { }

PlayerSpawn::PlayerSpawn(PlayerSpawn && playerSpawn) noexcept
	: SectorItem(std::move(playerSpawn)) { }

PlayerSpawn::PlayerSpawn(const PlayerSpawn & playerSpawn)
	: SectorItem(playerSpawn) { }

PlayerSpawn::PlayerSpawn(const SectorItem & sectorItem)
	: SectorItem(sectorItem) { }

PlayerSpawn & PlayerSpawn::operator = (PlayerSpawn && playerSpawn) noexcept {
	if(this != &playerSpawn) {
		SectorItem::operator = (std::move(playerSpawn));
	}

	return *this;
}

PlayerSpawn & PlayerSpawn::operator = (const PlayerSpawn & playerSpawn) {
	SectorItem::operator = (playerSpawn);

	return *this;
}

PlayerSpawn::~PlayerSpawn() { }

PlayerSpawn PlayerSpawn::getFrom(const ByteBuffer & byteBuffer, size_t offset, bool * error) {
	if(offset + SIZE_BYTES > byteBuffer.getSize()) {
		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	size_t newOffset = offset;

	std::optional<Point3D> optionalPosition(Point3D::getFrom(byteBuffer, newOffset));

	if(!optionalPosition.has_value()) {
		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	newOffset += Point3D::SIZE_BYTES;

	std::optional<int16_t> optionalAngle(byteBuffer.getShort(newOffset));

	if(!optionalAngle.has_value()) {
		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	newOffset += sizeof(int16_t);

	std::optional<uint16_t> optionalSectorIndex(byteBuffer.getUnsignedShort(newOffset));

	if(!optionalSectorIndex.has_value()) {
		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	return PlayerSpawn(std::move(optionalPosition.value()), optionalAngle.value(), optionalSectorIndex.value());
}

std::optional<PlayerSpawn> PlayerSpawn::getFrom(const ByteBuffer & byteBuffer, size_t offset) {
	bool error = false;

	PlayerSpawn value(getFrom(byteBuffer, offset, &error));

	if(error) {
		return {};
	}

	return std::move(value);
}

PlayerSpawn PlayerSpawn::readFrom(const ByteBuffer & byteBuffer, bool * error) {
	bool internalError = false;

	PlayerSpawn value(getFrom(byteBuffer, byteBuffer.getReadOffset(), error));

	if(internalError) {
		if(error != nullptr) {
			*error = true;
		}
	}
	else {
		byteBuffer.skipReadBytes(SIZE_BYTES);
	}

	return value;
}

std::optional<PlayerSpawn> PlayerSpawn::readFrom(const ByteBuffer & byteBuffer) {
	bool error = false;

	PlayerSpawn value(readFrom(byteBuffer, &error));

	if(error) {
		return {};
	}

	return std::move(value);
}

bool PlayerSpawn::putIn(ByteBuffer & byteBuffer, size_t offset) const {
	size_t newOffset = offset;

	if(!m_position.putIn(byteBuffer, newOffset)) {
		return false;
	}

	newOffset += Point3D::SIZE_BYTES;

	if(!byteBuffer.putShort(m_angle, newOffset)) {
		return false;
	}

	newOffset += sizeof(int16_t);

	if(!byteBuffer.putUnsignedShort(m_sectorIndex, newOffset)) {
		return false;
	}

	return true;
}

bool PlayerSpawn::insertIn(ByteBuffer & byteBuffer, size_t offset) const {
	size_t newOffset = offset;

	if(!m_position.insertIn(byteBuffer, newOffset)) {
		return false;
	}

	newOffset += Point3D::SIZE_BYTES;

	if(!byteBuffer.insertShort(m_angle, newOffset)) {
		return false;
	}

	newOffset += sizeof(int16_t);

	if(!byteBuffer.insertUnsignedShort(m_sectorIndex, newOffset)) {
		return false;
	}

	return true;
}

bool PlayerSpawn::writeTo(ByteBuffer & byteBuffer) const {
	if(!m_position.writeTo(byteBuffer)) {
		return false;
	}

	if(!byteBuffer.writeShort(m_angle)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedShort(m_sectorIndex)) {
		return false;
	}

	return true;
}

PlayerSpawn PlayerSpawn::parseFrom(const rapidjson::Value & sectorItemValue, bool * error) {
	return PlayerSpawn(SectorItem::parseFrom(sectorItemValue, error));
}

std::optional<PlayerSpawn> PlayerSpawn::parseFrom(const rapidjson::Value & sectorItemValue) {
	bool error = false;

	PlayerSpawn value(parseFrom(sectorItemValue, &error));

	if(error) {
		return {};
	}

	return std::move(value);
}

std::string PlayerSpawn::toString() const {
	return fmt::format("Player Spawn Position: {}, Angle: {}, Sector Index: {}", m_position.toString(), m_angle, m_sectorIndex);
}

bool PlayerSpawn::operator == (const PlayerSpawn & playerSpawn) const {
	return SectorItem::operator == (playerSpawn);
}

bool PlayerSpawn::operator != (const PlayerSpawn & playerSpawn) const {
	return !operator == (playerSpawn);
}
