#ifndef _TAGGED_ITEM_H_
#define _TAGGED_ITEM_H_

#include <rapidjson/document.h>

#include <cstdint>
#include <optional>

class ByteBuffer;

class TaggedItem {
public:
	TaggedItem();
	TaggedItem(uint16_t lowTag, uint16_t highTag, uint16_t extra);
	TaggedItem(const TaggedItem & taggedItem);
	TaggedItem & operator = (const TaggedItem & taggedItem);
	virtual ~TaggedItem();

	bool hasLowTag() const;
	uint16_t getLowTag() const;
	void setLowTag(uint16_t lowTag);
	bool hasHighTag() const;
	uint16_t getHighTag() const;
	void setHighTag(uint16_t highTag);
	uint16_t getExtra() const;
	void setExtra(uint16_t extra);

	static TaggedItem getFrom(const ByteBuffer & byteBuffer, size_t offset, bool * error);
	static std::optional<TaggedItem> getFrom(const ByteBuffer & byteBuffer, size_t offset);
	static TaggedItem readFrom(const ByteBuffer & byteBuffer, bool * error);
	static std::optional<TaggedItem> readFrom(const ByteBuffer & byteBuffer);
	bool putIn(ByteBuffer & byteBuffer, size_t offset) const;
	bool insertIn(ByteBuffer & byteBuffer, size_t offset) const;
	bool writeTo(ByteBuffer & byteBuffer) const;

	rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	bool addToJSONObject(rapidjson::Value & value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const;
	static TaggedItem parseFrom(const rapidjson::Value & tagInformationValue, bool * error);
	static std::optional<TaggedItem> parseFrom(const rapidjson::Value & tagInformationValue);

	bool operator == (const TaggedItem & taggedItem) const;
	bool operator != (const TaggedItem & taggedItem) const;

	static constexpr size_t SIZE_BYTES = (sizeof(uint16_t) * 2) + sizeof(int16_t);

protected:
	uint16_t m_lowTag;
	uint16_t m_highTag;
	uint16_t m_extra;
};

#endif // _TAGGED_ITEM_H_
