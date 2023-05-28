#ifndef _TEXTURED_ITEM_H_
#define _TEXTURED_ITEM_H_

#include <rapidjson/document.h>

#include <cstdint>
#include <optional>

class TexturedItem {
public:
	TexturedItem();
	TexturedItem(uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber);
	TexturedItem(const TexturedItem & texturedItem);
	TexturedItem & operator = (const TexturedItem & texturedItem);
	virtual ~TexturedItem();

	uint16_t getTileNumber() const;
	void setTileNumber(uint16_t tileNumber);
	int8_t getShade() const;
	void setShade(int8_t shade);
	uint8_t getPaletteLookupTableNumber() const;
	void setPaletteLookupTableNumber(uint8_t paletteLookupTableNumber);

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	bool addToJSONObject(rapidjson::Value & value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static TexturedItem parseFrom(const rapidjson::Value & texturedItemValue, bool * error);
	static std::optional<TexturedItem> parseFrom(const rapidjson::Value & texturedItemValue);

	bool operator == (const TexturedItem & texturedItem) const;
	bool operator != (const TexturedItem & texturedItem) const;

	static constexpr size_t SIZE_BYTES = sizeof(int16_t) + sizeof(int8_t) + sizeof(uint8_t);

protected:
	uint16_t m_tileNumber;
	int8_t m_shade;
	uint8_t m_paletteLookupTableNumber;
};

#endif // _TEXTURED_ITEM_H_
