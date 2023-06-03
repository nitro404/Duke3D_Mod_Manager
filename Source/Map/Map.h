#ifndef _MAP_H_
#define _MAP_H_

#include "File/GameFile.h"
#include "PlayerSpawn.h"
#include "Sector.h"
#include "Sprite.h"
#include "Wall.h"

#include <Endianness.h>

#include <rapidjson/document.h>

#include <cstdint>
#include <vector>

class Map final : public GameFile {
public:
	Map();
	Map(uint32_t version, const PlayerSpawn & playerSpawn, std::vector<std::unique_ptr<Sector>> sectors, std::vector<std::unique_ptr<Wall>> walls, std::vector<std::unique_ptr<Sprite>> sprites, std::unique_ptr<ByteBuffer> trailingData = nullptr);
	Map(Map && map) noexcept;
	Map(const Map & map);
	Map & operator = (Map && map) noexcept;
	Map & operator = (const Map & map);
	virtual ~Map();

	uint32_t getVersion() const;
	PlayerSpawn & getPlayerSpawn();
	const PlayerSpawn & getPlayerSpawn() const;
	void setPlayerSpawn(const PlayerSpawn & playerSpawn);
	int32_t getPlayerSpawnX() const;
	void setPlayerSpawnX(int32_t x);
	int32_t getPlayerSpawnY() const;
	void setPlayerSpawnY(int32_t y);
	int32_t getPlayerSpawnZ() const;
	void setPlayerSpawnZ(int32_t z);
	const Point3D & getPlayerSpawnPosition() const;
	void setPlayerSpawnPosition(int32_t x, int32_t y, int32_t z);
	void setPlayerSpawnPosition(int32_t position[3]);
	void setPlayerSpawnPosition(const Point3D & position);
	int16_t getPlayerSpawnAngle() const;
	bool setPlayerSpawnAngle(int16_t angle);
	double getPlayerSpawnAngleDegrees() const;
	bool setPlayerSpawnAngleDegrees(double angleDegrees);
	double getPlayerSpawnAngleRadians() const;
	bool setPlayerSpawnAngleRadians(double angleRadians);
	uint16_t getPlayerSpawnSectorIndex() const;
	void setPlayerSpawnSectorIndex(uint16_t sectorIndex);

	size_t numberOfSectors() const;
	bool hasSector(const Sector & sector) const;
	size_t indexOfSector(const Sector & sector) const;
	std::shared_ptr<Sector> getSector(size_t index) const;
	const std::vector<std::shared_ptr<Sector>> & getSectors() const;

	size_t numberOfWalls() const;
	bool hasWall(const Wall & wall) const;
	size_t indexOfWall(const Wall & wall) const;
	std::shared_ptr<Wall> getWall(size_t index) const;
	const std::vector<std::shared_ptr<Wall>> & getWalls() const;

	size_t numberOfSprites() const;
	bool hasSprite(const Sprite & sprite) const;
	size_t indexOfSprite(const Sprite & sprite) const;
	std::shared_ptr<Sprite> getSprite(size_t index) const;
	const std::vector<std::shared_ptr<Sprite>> & getSprites() const;

	std::shared_ptr<ByteBuffer> getTrailingData() const;
	void setTrailingData(std::unique_ptr<ByteBuffer> trailingData);
	void clearTrailingData();

	static std::unique_ptr<Map> readFrom(const ByteBuffer & byteBuffer);
	virtual bool writeTo(ByteBuffer & byteBuffer) const override;

	rapidjson::Document toJSON() const;
	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	bool addToJSONObject(rapidjson::Value & value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<Map> parseFrom(const rapidjson::Value & mapValue);

	static std::unique_ptr<Map> loadFrom(const std::string & filePath);
	static std::unique_ptr<Map> loadFromMap(const std::string & filePath);
	static std::unique_ptr<Map> loadFromJSON(const std::string & filePath);
	virtual bool saveTo(const std::string & filePath, bool overwrite = true) const override;
	bool saveToMap(const std::string & filePath, bool overwrite = true) const;
	bool saveToJSON(const std::string & filePath, bool overwrite = true) const;

	virtual void addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const override;
	virtual Endianness getEndianness() const override;
	virtual size_t getSizeInBytes() const override;

	virtual bool isValid(bool verifyParent = true) const override;
	static bool isValid(const Map * map, bool verifyParent = true);

	bool operator == (const Map & map) const;
	bool operator != (const Map & map) const;

	static constexpr uint32_t DEFAULT_VERSION = 7;
	static constexpr Endianness ENDIANNESS = Endianness::LittleEndian;

private:
	uint32_t m_version;
	PlayerSpawn m_playerSpawn;
	std::vector<std::shared_ptr<Sector>> m_sectors;
	std::vector<std::shared_ptr<Wall>> m_walls;
	std::vector<std::shared_ptr<Sprite>> m_sprites;
	std::shared_ptr<ByteBuffer> m_trailingData;
};

#endif // _MAP_H_
