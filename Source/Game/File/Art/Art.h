#ifndef _ART_H_
#define _ART_H_

#include "../GameFile.h"
#include "Tile.h"

#include <Endianness.h>

#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class ByteBuffer;

class Art final : public GameFile {
public:
	Art(const std::string & filePath = {});
	Art(uint16_t artNumber, uint32_t tileCount);
	Art(uint16_t artNumber, uint32_t tileCount = DEFAULT_NUMBER_OF_TILES, std::unique_ptr<ByteBuffer> trailingData = nullptr);
	Art(uint16_t artNumber, uint32_t tileCount, ByteBuffer && trailingData);
	Art(uint32_t localTileStart, uint32_t localTileEnd, uint32_t legacyTileCount, std::vector<std::unique_ptr<Tile>> tiles, std::unique_ptr<ByteBuffer> trailingData = nullptr);
	Art(uint32_t localTileStart, uint32_t localTileEnd, uint32_t legacyTileCount, std::vector<std::unique_ptr<Tile>> tiles, ByteBuffer && trailingData);
	Art(Art && art) noexcept;
	Art(const Art & art);
	Art & operator = (Art && art) noexcept;
	Art & operator = (const Art & art);
	virtual ~Art();

	uint32_t getVersion() const;
	void setVersion(uint32_t version);
	uint16_t getNumber() const;
	void setNumber(uint16_t number);
	uint32_t getLocalTileStart() const;
	void setLocalTileStart(uint32_t localTileStart);
	uint32_t getLocalTileEnd() const;
	bool setLocalTileEnd(uint32_t localTileEnd);
	uint32_t getLegacyTileCount() const;
	void setLegacyTileCount(uint32_t legacyTileCount);
	bool hasTrailingData() const;
	std::shared_ptr<ByteBuffer> getTailingData() const;
	void setTrailingData(std::unique_ptr<ByteBuffer> trailingData);
	void clearTrailingData();

	size_t numberOfTiles() const;
	void setNumberOfTiles(uint32_t tileCount);
	bool hasTile(const Tile & tile) const;
	bool hasTileWithNumber(uint32_t number) const;
	bool hasNonEmptyTile() const;
	bool hasEmptyTile() const;
	size_t indexOfTile(const Tile & tile) const;
	uint32_t numberOfNonEmptyTiles() const;
	uint32_t numberOfEmptyTiles() const;
	std::shared_ptr<Tile> getTileByIndex(size_t index) const;
	std::shared_ptr<Tile> getTileWithNumber(uint32_t number) const;
	const std::vector<std::shared_ptr<Tile>> & getTiles() const;
	bool setTile(std::unique_ptr<Tile> tile);
	bool setTile(std::unique_ptr<Tile> tile, uint32_t tileNumber);
	bool replaceTile(const Tile & tile);
	bool replaceTile(const Tile & tile, uint32_t tileNumber);
	bool clearTile(uint32_t tileNumber);
	std::vector<std::shared_ptr<Tile>> getNonEmptyEmptyTiles() const;
	std::vector<std::shared_ptr<Tile>> getEmptyTiles() const;

	static std::unique_ptr<Art> readFrom(const ByteBuffer & byteBuffer);
	virtual bool writeTo(ByteBuffer & byteBuffer) const override;

	rapidjson::Document toJSON() const;
	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	bool addToJSONObject(rapidjson::Value & value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<Art> parseFrom(const rapidjson::Value & artValue);

	static std::unique_ptr<Art> loadFrom(const std::string & filePath);
	static std::unique_ptr<Art> loadFromArt(const std::string & filePath);
	static std::unique_ptr<Art> loadFromJSON(const std::string & filePath);
	virtual bool saveTo(const std::string & filePath, bool overwrite = true) const override;
	bool saveToArt(const std::string & filePath, bool overwrite = true) const;
	bool saveToJSON(const std::string & filePath, bool overwrite = true) const;

	virtual void addMetadata(std::vector<std::pair<std::string, std::string>> & metadata) const override;
	virtual Endianness getEndianness() const override;
	virtual uint64_t getSizeInBytes() const override;

	virtual bool isValid(bool verifyParent = true) const override;
	static bool isValid(const Art * art, bool verifyParent = true);

	bool operator == (const Art & art) const;
	bool operator != (const Art & art) const;

	static constexpr Endianness ENDIANNESS = Endianness::LittleEndian;
	static constexpr uint32_t DEFAULT_VERSION = 1;
	static constexpr uint32_t DEFAULT_NUMBER_OF_TILES = 256;

private:
	void autosize();
	void updateParent();

	uint32_t m_version;
	uint32_t m_localTileStart;
	uint32_t m_localTileEnd;
	uint32_t m_legacyTileCount;
	std::vector<std::shared_ptr<Tile>> m_tiles;
	std::shared_ptr<ByteBuffer> m_trailingData;
};

#endif // _ART_H_
