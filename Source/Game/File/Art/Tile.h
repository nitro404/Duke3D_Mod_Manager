#ifndef _TILE_H_
#define _TILE_H_

#include <rapidjson/document.h>

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>

class Art;
class ByteBuffer;

class Tile final {
public:
	enum class AnimationType : uint8_t {
		None = 0,
		Oscillating = 1,
		Forward = 2,
		Backward = 3,
		Default = None
	};

	struct Attributes final {
		union {
			struct {
				uint8_t numberOfAnimatedFrames : 6;
				AnimationType animationType    : 2;
				int8_t xOffset                 : 8;
				int8_t yOffset                 : 8;
				uint8_t animationSpeed         : 4;
				uint8_t extra                  : 4; // note: extra value is not used
			};

			uint32_t rawValue;
		};

		rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
		bool addToJSONObject(rapidjson::Value & value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
		static Attributes parseFrom(const rapidjson::Value & attributesValue, bool * error);
		static std::optional<Attributes> parseFrom(const rapidjson::Value & attributesValue);

		bool operator == (const Attributes & attributes) const;
		bool operator != (const Attributes & attributes) const;
	};

	struct TileNumberComparator final {
	public:
		bool operator () (uint32_t tileNumberA, uint32_t tileNumberB) const;
	};

	Tile(Art * parent = nullptr);
	Tile(uint32_t number, Art * parent = nullptr);
	Tile(uint32_t number, uint16_t width, uint16_t height, const Attributes & attributes, std::unique_ptr<ByteBuffer> data = nullptr, Art * parent = nullptr);
	Tile(uint32_t number, uint16_t width, uint16_t height, const Attributes & attributes, ByteBuffer && data, Art * parent = nullptr);
	Tile(Tile && tile) noexcept;
	Tile(const Tile & tile);
	Tile & operator = (Tile && tile) noexcept;
	Tile & operator = (const Tile & tile);
	~Tile();

	int8_t getXOffset() const;
	void setXOffset(int8_t xOffset);
	int8_t getYOffset() const;
	void setYOffset(int8_t yOffset);
	uint8_t getNumberOfAnimatedFrames() const;
	bool setNumberOfAnimatedFrames(uint8_t numberOfAnimatedFrames);
	AnimationType getAnimationType() const;
	void setAnimationType(AnimationType animationType);
	uint8_t getAnimationSpeed() const;
	bool setAnimationSpeed(uint8_t animationSpeed);
	uint8_t getExtra() const;
	bool setExtra(uint8_t extra);
	Attributes & getAttributes();
	const Attributes & getAttributes() const;
	void setAttributes(const Attributes & attributes);
	bool setAttributes(int8_t xOffset, int8_t yOffset, uint8_t numberOfAnimatedFrames, AnimationType animationType, uint8_t animationSpeed, uint8_t extra);
	uint32_t getNumber() const;
	void setNumber(uint32_t number);
	bool hasName() const;
	const std::string & getName() const;
	bool isEmpty() const;
	bool isNotEmpty() const;
	uint16_t getWidth() const;
	void setWidth(uint16_t width);
	uint16_t getHeight() const;
	void setHeight(uint16_t height);
	size_t getSize() const;
	std::shared_ptr<ByteBuffer> getData() const;
	void setData(std::unique_ptr<ByteBuffer> data);
	void clearData();
	void clear();
	Art * getParent();
	const Art * getParent() const;
	void setParent(Art * art);
	void clearParent();

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<Tile> parseFrom(const rapidjson::Value & tileValue);

	bool operator == (const Tile & tile) const;
	bool operator != (const Tile & tile) const;

	static const std::map<uint32_t, std::string, TileNumberComparator> NAMES;
	static const std::map<uint32_t, uint8_t> SPECIAL_TILE_PALETTE_LOOKUP_TABLE_NUMBERS;

private:
	uint32_t m_number;
	uint16_t m_width;
	uint16_t m_height;
	Attributes m_attributes;
	std::shared_ptr<ByteBuffer> m_data;
	Art * m_parent;
};

#endif // _TILE_H_
