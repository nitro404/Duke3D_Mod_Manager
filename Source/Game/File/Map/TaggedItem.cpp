#include "TaggedItem.h"

#include "ByteBuffer.h"
#include "Utilities/RapidJSONUtilities.h"

#include <spdlog/spdlog.h>

#include <string>

static const std::string JSON_LOW_TAG_PROPERTY_NAME("lowTag");
static const std::string JSON_HIGH_TAG_PROPERTY_NAME("highTag");
static const std::string JSON_EXTRA_PROPERTY_NAME("extra");

TaggedItem::TaggedItem()
	: m_lowTag(0)
	, m_highTag(0)
	, m_extra(0) { }

TaggedItem::TaggedItem(uint16_t lowTag, uint16_t highTag, uint16_t extra)
	: m_lowTag(lowTag)
	, m_highTag(highTag)
	, m_extra(extra) { }

TaggedItem::TaggedItem(const TaggedItem & taggedItem)
	: m_lowTag(taggedItem.m_lowTag)
	, m_highTag(taggedItem.m_highTag)
	, m_extra(taggedItem.m_extra) { }

TaggedItem & TaggedItem::operator = (const TaggedItem & taggedItem) {
	m_lowTag = taggedItem.m_lowTag;
	m_highTag = taggedItem.m_highTag;
	m_extra = taggedItem.m_extra;

	return *this;
}

TaggedItem::~TaggedItem() { }

bool TaggedItem::hasLowTag() const {
	return m_lowTag != 0;
}

uint16_t TaggedItem::getLowTag() const {
	return m_lowTag;
}

void TaggedItem::setLowTag(uint16_t lowTag) {
	m_lowTag = lowTag;
}

bool TaggedItem::hasHighTag() const {
	return m_highTag != 0;
}

uint16_t TaggedItem::getHighTag() const {
	return m_highTag;
}

void TaggedItem::setHighTag(uint16_t highTag) {
	m_highTag = highTag;
}

uint16_t TaggedItem::getExtra() const {
	return m_extra;
}

void TaggedItem::setExtra(uint16_t extra) {
	m_extra = extra;
}

TaggedItem TaggedItem::getFrom(const ByteBuffer & byteBuffer, size_t offset, bool * error) {
	if(offset + SIZE_BYTES > byteBuffer.getSize()) {
		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	size_t newOffset = offset;

	std::optional<uint16_t> optionalLowTag(byteBuffer.getUnsignedShort(newOffset));

	if(!optionalLowTag.has_value()) {
		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	newOffset += sizeof(uint16_t);

	std::optional<uint16_t> optionalHighTag(byteBuffer.getUnsignedShort(newOffset));

	if(!optionalHighTag.has_value()) {
		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	newOffset += sizeof(uint16_t);

	std::optional<uint16_t> optionalExtra(byteBuffer.getUnsignedShort(newOffset));

	if(!optionalExtra.has_value()) {
		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	return TaggedItem(std::move(optionalLowTag.value()), optionalHighTag.value(), optionalExtra.value());
}

std::optional<TaggedItem> TaggedItem::getFrom(const ByteBuffer & byteBuffer, size_t offset) {
	bool error = false;

	TaggedItem value(getFrom(byteBuffer, offset, &error));

	if(error) {
		return {};
	}

	return value;
}

TaggedItem TaggedItem::readFrom(const ByteBuffer & byteBuffer, bool * error) {
	bool internalError = false;

	TaggedItem value(getFrom(byteBuffer, byteBuffer.getReadOffset(), error));

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

std::optional<TaggedItem> TaggedItem::readFrom(const ByteBuffer & byteBuffer) {
	bool error = false;

	TaggedItem value(readFrom(byteBuffer, &error));

	if(error) {
		return {};
	}

	return value;
}

bool TaggedItem::putIn(ByteBuffer & byteBuffer, size_t offset) const {
	size_t newOffset = offset;

	if(!byteBuffer.putUnsignedShort(m_lowTag, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(!byteBuffer.putUnsignedShort(m_highTag, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(!byteBuffer.putUnsignedShort(m_extra, newOffset)) {
		return false;
	}

	return true;
}

bool TaggedItem::insertIn(ByteBuffer & byteBuffer, size_t offset) const {
	size_t newOffset = offset;

	if(!byteBuffer.insertUnsignedShort(m_lowTag, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(!byteBuffer.insertUnsignedShort(m_highTag, newOffset)) {
		return false;
	}

	newOffset += sizeof(uint16_t);

	if(!byteBuffer.insertUnsignedShort(m_extra, newOffset)) {
		return false;
	}

	return true;
}

bool TaggedItem::writeTo(ByteBuffer & byteBuffer) const {
	if(!byteBuffer.writeUnsignedShort(m_lowTag)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedShort(m_highTag)) {
		return false;
	}

	if(!byteBuffer.writeUnsignedShort(m_extra)) {
		return false;
	}

	return true;
}

rapidjson::Value TaggedItem::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value tagInformationValue(rapidjson::kObjectType);

	addToJSONObject(tagInformationValue, allocator);

	return tagInformationValue;
}

bool TaggedItem::addToJSONObject(rapidjson::Value & value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	if(!value.IsObject()) {
		return false;
	}

	value.AddMember(rapidjson::StringRef(JSON_LOW_TAG_PROPERTY_NAME.c_str()), rapidjson::Value(m_lowTag), allocator);

	value.AddMember(rapidjson::StringRef(JSON_HIGH_TAG_PROPERTY_NAME.c_str()), rapidjson::Value(m_highTag), allocator);

	value.AddMember(rapidjson::StringRef(JSON_EXTRA_PROPERTY_NAME.c_str()), rapidjson::Value(m_extra), allocator);

	return true;
}

TaggedItem TaggedItem::parseFrom(const rapidjson::Value & tagInformationValue, bool * error) {
	if(!tagInformationValue.IsObject()) {
		spdlog::error("Invalid tag information type: '{}', expected 'object'.", Utilities::typeToString(tagInformationValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse low tag
	if(!tagInformationValue.HasMember(JSON_LOW_TAG_PROPERTY_NAME.c_str())) {
		spdlog::error("Tag information is missing '{}' property.", JSON_LOW_TAG_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & lowTagValue = tagInformationValue[JSON_LOW_TAG_PROPERTY_NAME.c_str()];

	if(!lowTagValue.IsUint()) {
		spdlog::error("Tag information has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_LOW_TAG_PROPERTY_NAME, Utilities::typeToString(lowTagValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	uint32_t lowTag = lowTagValue.GetUint();

	if(lowTag > std::numeric_limits<uint16_t>::max()) {
		spdlog::error("Invalid tag information low tag: {}, expected a value between {} and {}, inclusively.", lowTag, 0, std::numeric_limits<uint16_t>::max());

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse high tag
	if(!tagInformationValue.HasMember(JSON_HIGH_TAG_PROPERTY_NAME.c_str())) {
		spdlog::error("Tag information is missing '{}' property.", JSON_HIGH_TAG_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & highTagValue = tagInformationValue[JSON_HIGH_TAG_PROPERTY_NAME.c_str()];

	if(!highTagValue.IsUint()) {
		spdlog::error("Tag information has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_HIGH_TAG_PROPERTY_NAME, Utilities::typeToString(highTagValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	uint32_t highTag = highTagValue.GetUint();

	if(highTag > std::numeric_limits<uint16_t>::max()) {
		spdlog::error("Invalid tag information high tag: {}, expected a value between {} and {}, inclusively.", highTag, 0, std::numeric_limits<uint16_t>::max());

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse extra
	if(!tagInformationValue.HasMember(JSON_EXTRA_PROPERTY_NAME.c_str())) {
		spdlog::error("Tag information is missing '{}' property.", JSON_EXTRA_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & extraValue = tagInformationValue[JSON_EXTRA_PROPERTY_NAME.c_str()];

	if(!extraValue.IsUint()) {
		spdlog::error("Tag information has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_EXTRA_PROPERTY_NAME, Utilities::typeToString(extraValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	uint32_t extra = extraValue.GetUint();

	if(extra > std::numeric_limits<uint16_t>::max()) {
		spdlog::error("Invalid tag information extra: {}, expected a value between 0 and {}, inclusively.", extra, std::numeric_limits<int16_t>::max());

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	return TaggedItem(static_cast<uint16_t>(lowTag), static_cast<uint16_t>(highTag), static_cast<uint16_t>(extra));
}

std::optional<TaggedItem> TaggedItem::parseFrom(const rapidjson::Value & tagInformationValue) {
	bool error = false;

	TaggedItem value(parseFrom(tagInformationValue, &error));

	if(error) {
		return {};
	}

	return value;
}

bool TaggedItem::operator == (const TaggedItem & taggedItem) const {
	return m_lowTag == taggedItem.m_lowTag &&
		   m_highTag == taggedItem.m_highTag &&
		   m_extra == taggedItem.m_extra;
}

bool TaggedItem::operator != (const TaggedItem & taggedItem) const {
	return !operator == (taggedItem);
}
