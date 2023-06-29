#ifndef _PARTITION_H_
#define _PARTITION_H_

#include "TexturedItem.h"

#include <rapidjson/document.h>

#include <cstdint>
#include <memory>
#include <optional>

class Partition final : public TexturedItem {
public:
	enum class Type : uint8_t {
		Floor,
		Ceiling
	};

	struct Attributes final {
		union {
			struct {
				bool parallaxed        : 1;
				bool sloped            : 1;
				bool swapTextureXY     : 1;
				bool doubleSmooshiness : 1;
				bool xFlipped          : 1;
				bool yFlipped          : 1;
				bool textureAlign      : 1;
				uint16_t reserved      : 9;
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

	Partition(Type type);
	Partition(Type type, int32_t height, Attributes attributes, TexturedItem texturedItem, int16_t slope, uint8_t xPanning, uint8_t yPanning);
	Partition(Type type, int32_t height, Attributes attributes, uint16_t tileNumber, int16_t slope, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t xPanning, uint8_t yPanning);
	Partition(Partition && partition) noexcept;
	Partition(const Partition & partition);
	Partition & operator = (Partition && partition) noexcept;
	Partition & operator = (const Partition & partition);
	virtual ~Partition();

	Type getType() const;
	void setType(Type type);
	bool isParallaxed() const;
	void setParallaxed(bool parallaxed);
	bool isSloped() const;
	void setSloped(bool sloped);
	bool isTextureSwappedXY() const;
	void setTextureSwappedXY(bool swapTextureXY);
	bool isSmooshinessDoubled() const;
	void setSmooshinessDoubled(bool doubleSmooshiness);
	bool isXFlipped() const;
	void setXFlipped(bool xFlipped);
	bool isYFlipped() const;
	void setYFlipped(bool yFlipped);
	bool isTextureAligned() const;
	void setTextureAligned(bool textureAlign);
	uint16_t getReserved() const;
	bool setReserved(uint16_t reserved);
	Attributes & getAttributes();
	const Attributes & getAttributes() const;
	void setAttributes(Attributes attributes);
	int32_t getHeight() const;
	void setHeight(int32_t height);
	int16_t getSlope() const;
	void setSlope(int16_t slope);
	uint8_t getXPanning() const;
	void setXPanning(uint8_t xPanning);
	uint8_t getYPanning() const;
	void setYPanning(uint8_t yPanning);

	static size_t getSizeInBytes(uint32_t mapVersion);

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static std::unique_ptr<Partition> parseFrom(const rapidjson::Value & partitionValue, Type type);

	bool operator == (const Partition & partition) const;
	bool operator != (const Partition & partition) const;

private:
	Type m_type;
	int32_t m_height;
	Attributes m_attributes;
	int16_t m_slope;
	uint8_t m_xPanning;
	uint8_t m_yPanning;
};

#endif // _PARTITION_H_
