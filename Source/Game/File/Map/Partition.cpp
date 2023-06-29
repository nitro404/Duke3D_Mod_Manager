#include "Partition.h"

#include <ByteBuffer.h>
#include <Utilities/RapidJSONUtilities.h>

#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <string>

static const std::string JSON_PARALLAXED_ATTRIBUTE_PROPERTY_NAME("parallaxed");
static const std::string JSON_SLOPED_ATTRIBUTE_PROPERTY_NAME("sloped");
static const std::string JSON_SWAP_TEXTURE_XY_ATTRIBUTE_PROPERTY_NAME("swapTextureXY");
static const std::string JSON_DOUBLE_SMOOSHINESS_ATTRIBUTE_PROPERTY_NAME("doubleSmooshiness");
static const std::string JSON_X_FLIPPED_ATTRIBUTE_PROPERTY_NAME("xFlipped");
static const std::string JSON_Y_FLIPPED_ATTRIBUTE_PROPERTY_NAME("yFlipped");
static const std::string JSON_TEXTURE_ALIGN_ATTRIBUTE_PROPERTY_NAME("textureAlign");
static const std::string JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME("reserved");

static const std::string JSON_HEIGHT_PROPERTY_NAME("height");
static const std::string JSON_ATTRIBUTES_PROPERTY_NAME("attributes");
static const std::string JSON_SLOPE_PROPERTY_NAME("slope");
static const std::string JSON_X_PANNING_PROPERTY_NAME("xPanning");
static const std::string JSON_Y_PANNING_PROPERTY_NAME("yPanning");

Partition::Partition(Type type)
	: TexturedItem(0, 0, 0)
	, m_type(type)
	, m_height(0)
	, m_attributes({ 0 })
	, m_slope(0)
	, m_xPanning(0)
	, m_yPanning(0) { }

Partition::Partition(Type type, int32_t height, Attributes attributes, TexturedItem texturedItem, int16_t slope, uint8_t xPanning, uint8_t yPanning)
	: TexturedItem(texturedItem)
	, m_type(type)
	, m_height(height)
	, m_attributes(attributes)
	, m_slope(slope)
	, m_xPanning(xPanning)
	, m_yPanning(yPanning) { }

Partition::Partition(Type type, int32_t height, Attributes attributes, uint16_t tileNumber, int16_t slope, int8_t shade, uint8_t paletteLookupTableNumber, uint8_t xPanning, uint8_t yPanning)
	: TexturedItem(tileNumber, shade, paletteLookupTableNumber)
	, m_type(type)
	, m_height(height)
	, m_attributes(attributes)
	, m_slope(slope)
	, m_xPanning(xPanning)
	, m_yPanning(yPanning) { }

Partition::Partition(Partition && partition) noexcept
	: TexturedItem(partition)
	, m_type(partition.m_type)
	, m_height(partition.m_height)
	, m_attributes(std::move(partition.m_attributes))
	, m_slope(partition.m_slope)
	, m_xPanning(partition.m_xPanning)
	, m_yPanning(partition.m_yPanning) { }

Partition::Partition(const Partition & partition)
	: TexturedItem(partition)
	, m_type(partition.m_type)
	, m_height(partition.m_height)
	, m_attributes(partition.m_attributes)
	, m_slope(partition.m_slope)
	, m_xPanning(partition.m_xPanning)
	, m_yPanning(partition.m_yPanning) { }

Partition & Partition::operator = (Partition && partition) noexcept {
	if(this != &partition) {
		TexturedItem::operator = (std::move(partition));

		m_type = partition.m_type;
		m_height = partition.m_height;
		m_attributes = std::move(partition.m_attributes);
		m_slope = partition.m_slope;
		m_xPanning = partition.m_xPanning;
		m_yPanning = partition.m_yPanning;
	}

	return *this;
}

Partition & Partition::operator = (const Partition & partition) {
	TexturedItem::operator = (partition);

	m_type = partition.m_type;
	m_height = partition.m_height;
	m_attributes = partition.m_attributes;
	m_slope = partition.m_slope;
	m_xPanning = partition.m_xPanning;
	m_yPanning = partition.m_yPanning;

	return *this;
}

Partition::~Partition() = default;

Partition::Type Partition::getType() const {
	return m_type;
}

void Partition::setType(Type type) {
	m_type = type;
}

bool Partition::isParallaxed() const {
	return m_attributes.parallaxed;
}

void Partition::setParallaxed(bool parallaxed) {
	m_attributes.parallaxed = parallaxed;
}

bool Partition::isSloped() const {
	return m_attributes.sloped;
}

void Partition::setSloped(bool sloped) {
	m_attributes.sloped = sloped;
}

bool Partition::isTextureSwappedXY() const {
	return m_attributes.swapTextureXY;
}

void Partition::setTextureSwappedXY(bool swapTextureXY) {
	m_attributes.swapTextureXY = swapTextureXY;
}

bool Partition::isSmooshinessDoubled() const {
	return m_attributes.doubleSmooshiness;
}

void Partition::setSmooshinessDoubled(bool doubleSmooshiness) {
	m_attributes.doubleSmooshiness = doubleSmooshiness;
}

bool Partition::isXFlipped() const {
	return m_attributes.xFlipped;
}

void Partition::setXFlipped(bool xFlipped) {
	m_attributes.xFlipped = xFlipped;
}

bool Partition::isYFlipped() const {
	return m_attributes.yFlipped;
}

void Partition::setYFlipped(bool yFlipped) {
	m_attributes.yFlipped = yFlipped;
}

bool Partition::isTextureAligned() const {
	return m_attributes.textureAlign;
}

void Partition::setTextureAligned(bool textureAlign) {
	m_attributes.textureAlign = textureAlign;
}

uint16_t Partition::getReserved() const {
	return m_attributes.reserved;
}

bool Partition::setReserved(uint16_t reserved) {
	if(reserved > 511) {
		return false;
	}

	m_attributes.reserved = reserved;

	return true;
}

Partition::Attributes & Partition::getAttributes() {
	return m_attributes;
}

const Partition::Attributes & Partition::getAttributes() const {
	return m_attributes;
}

void Partition::setAttributes(Attributes attributes) {
	m_attributes = attributes;
}

int32_t Partition::getHeight() const {
	return m_height;
}

void Partition::setHeight(int32_t height) {
	m_height = height;
}

int16_t Partition::getSlope() const {
	return m_slope;
}

void Partition::setSlope(int16_t slope) {
	m_slope = slope;
}

uint8_t Partition::getXPanning() const {
	return m_xPanning;
}

void Partition::setXPanning(uint8_t xPanning) {
	m_xPanning = xPanning;
}

uint8_t Partition::getYPanning() const {
	return m_yPanning;
}

void Partition::setYPanning(uint8_t yPanning) {
	m_yPanning = yPanning;
}

size_t Partition::getSizeInBytes(uint32_t mapVersion) {
	return sizeof(uint32_t) + (sizeof(int16_t) * 2) + (sizeof(uint8_t) * (mapVersion == 6 ? 5 : 6));
}

rapidjson::Value Partition::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value partitionValue(rapidjson::kObjectType);

	partitionValue.AddMember(rapidjson::StringRef(JSON_HEIGHT_PROPERTY_NAME.c_str()), rapidjson::Value(m_height), allocator);

	rapidjson::Value attributesValue(m_attributes.toJSON(allocator));
	partitionValue.AddMember(rapidjson::StringRef(JSON_ATTRIBUTES_PROPERTY_NAME.c_str()), attributesValue, allocator);

	TexturedItem::addToJSONObject(partitionValue, allocator);

	partitionValue.AddMember(rapidjson::StringRef(JSON_SLOPE_PROPERTY_NAME.c_str()), rapidjson::Value(m_slope), allocator);

	partitionValue.AddMember(rapidjson::StringRef(JSON_X_PANNING_PROPERTY_NAME.c_str()), rapidjson::Value(m_xPanning), allocator);

	partitionValue.AddMember(rapidjson::StringRef(JSON_Y_PANNING_PROPERTY_NAME.c_str()), rapidjson::Value(m_yPanning), allocator);

	return partitionValue;
}

std::unique_ptr<Partition> Partition::parseFrom(const rapidjson::Value & partitionValue, Type type) {
	if(!partitionValue.IsObject()) {
		spdlog::error("Invalid partition type: '{}', expected 'object'.", Utilities::typeToString(partitionValue.GetType()));
		return nullptr;
	}

	// parse height
	if(!partitionValue.HasMember(JSON_HEIGHT_PROPERTY_NAME.c_str())) {
		spdlog::error("Partition is missing '{}' property.", JSON_HEIGHT_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & heightValue = partitionValue[JSON_HEIGHT_PROPERTY_NAME.c_str()];

	if(!heightValue.IsInt()) {
		spdlog::error("Partition has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_HEIGHT_PROPERTY_NAME, Utilities::typeToString(heightValue.GetType()));
		return nullptr;
	}

	int32_t height = heightValue.GetInt();

	// parse attributes
	if(!partitionValue.HasMember(JSON_ATTRIBUTES_PROPERTY_NAME.c_str())) {
		spdlog::error("Partition is missing '{}' property.", JSON_ATTRIBUTES_PROPERTY_NAME);
		return nullptr;
	}

	std::optional<Attributes> optionalAttributes(Attributes::parseFrom(partitionValue[JSON_ATTRIBUTES_PROPERTY_NAME.c_str()]));

	if(!optionalAttributes.has_value()) {
		return nullptr;
	}

	// parse textured item information
	std::optional<TexturedItem> optionalTexturedItem(TexturedItem::parseFrom(partitionValue));

	if(!optionalTexturedItem.has_value()) {
		return nullptr;
	}

	// parse slope
	if(!partitionValue.HasMember(JSON_SLOPE_PROPERTY_NAME.c_str())) {
		spdlog::error("Partition is missing '{}' property.", JSON_SLOPE_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & slopeValue = partitionValue[JSON_SLOPE_PROPERTY_NAME.c_str()];

	if(!slopeValue.IsInt()) {
		spdlog::error("Partition has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_SLOPE_PROPERTY_NAME, Utilities::typeToString(slopeValue.GetType()));
		return nullptr;
	}

	int32_t slope = slopeValue.GetInt();

	if(slope < std::numeric_limits<int16_t>::min() || slope > std::numeric_limits<int16_t>::max()) {
		spdlog::error("Invalid partition slope: {}, expected a value between {} and {}, inclusively.", slope, std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max());
		return nullptr;
	}

	// parse x panning
	if(!partitionValue.HasMember(JSON_X_PANNING_PROPERTY_NAME.c_str())) {
		spdlog::error("Partition is missing '{}' property.", JSON_X_PANNING_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & xPanningValue = partitionValue[JSON_X_PANNING_PROPERTY_NAME.c_str()];

	if(!xPanningValue.IsUint()) {
		spdlog::error("Partition has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_X_PANNING_PROPERTY_NAME, Utilities::typeToString(xPanningValue.GetType()));
		return nullptr;
	}

	uint32_t xPanning = xPanningValue.GetUint();

	if(xPanning > std::numeric_limits<uint8_t>::max()) {
		spdlog::error("Invalid partition x panning: {}, expected a value between 0 and {}, inclusively.", xPanning, std::numeric_limits<uint8_t>::max());
		return nullptr;
	}

	// parse y panning
	if(!partitionValue.HasMember(JSON_Y_PANNING_PROPERTY_NAME.c_str())) {
		spdlog::error("Partition is missing '{}' property.", JSON_Y_PANNING_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & yPanningValue = partitionValue[JSON_Y_PANNING_PROPERTY_NAME.c_str()];

	if(!yPanningValue.IsUint()) {
		spdlog::error("Partition has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_Y_PANNING_PROPERTY_NAME, Utilities::typeToString(yPanningValue.GetType()));
		return nullptr;
	}

	uint32_t yPanning = yPanningValue.GetUint();

	if(yPanning > std::numeric_limits<uint8_t>::max()) {
		spdlog::error("Invalid partition y panning: {}, expected a value between 0 and {}, inclusively.", yPanning, std::numeric_limits<uint8_t>::max());
		return nullptr;
	}

	return std::make_unique<Partition>(type, height, optionalAttributes.value(), optionalTexturedItem.value(), static_cast<int16_t>(slope), static_cast<uint8_t>(xPanning), static_cast<uint8_t>(yPanning));
}

bool Partition::operator == (const Partition & partition) const {
	return TexturedItem::operator == (partition) &&
		   m_type == partition.m_type &&
		   m_height == partition.m_height &&
		   m_attributes == partition.m_attributes &&
		   m_slope == partition.m_slope &&
		   m_xPanning == partition.m_xPanning &&
		   m_yPanning == partition.m_yPanning;
}

bool Partition::operator != (const Partition & partition) const {
	return !operator == (partition);
}

rapidjson::Value Partition::Attributes::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value attributesValue(rapidjson::kObjectType);

	addToJSONObject(attributesValue, allocator);

	return attributesValue;
}

bool Partition::Attributes::addToJSONObject(rapidjson::Value & value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	if(!value.IsObject()) {
		return false;
	}

	value.AddMember(rapidjson::StringRef(JSON_PARALLAXED_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(parallaxed), allocator);

	value.AddMember(rapidjson::StringRef(JSON_SLOPED_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(sloped), allocator);

	value.AddMember(rapidjson::StringRef(JSON_SWAP_TEXTURE_XY_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(swapTextureXY), allocator);

	value.AddMember(rapidjson::StringRef(JSON_DOUBLE_SMOOSHINESS_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(doubleSmooshiness), allocator);

	value.AddMember(rapidjson::StringRef(JSON_X_FLIPPED_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(xFlipped), allocator);

	value.AddMember(rapidjson::StringRef(JSON_Y_FLIPPED_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(yFlipped), allocator);

	value.AddMember(rapidjson::StringRef(JSON_TEXTURE_ALIGN_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(textureAlign), allocator);

	value.AddMember(rapidjson::StringRef(JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(reserved), allocator);

	return true;
}

Partition::Attributes Partition::Attributes::parseFrom(const rapidjson::Value & attributesValue, bool * error) {
	if(!attributesValue.IsObject()) {
		spdlog::error("Invalid partition attributes type: '{}', expected 'object'.", Utilities::typeToString(attributesValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse parallaxed attribute
	if(!attributesValue.HasMember(JSON_PARALLAXED_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Partition attribute '{}' is missing.", JSON_PARALLAXED_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & parallaxedValue = attributesValue[JSON_PARALLAXED_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!parallaxedValue.IsBool()) {
		spdlog::error("Partition has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_PARALLAXED_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(parallaxedValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool parallaxed = parallaxedValue.GetBool();

	// parse sloped attribute
	if(!attributesValue.HasMember(JSON_SLOPED_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Partition attribute '{}' is missing.", JSON_SLOPED_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & slopedValue = attributesValue[JSON_SLOPED_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!slopedValue.IsBool()) {
		spdlog::error("Partition has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_SLOPED_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(slopedValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool sloped = slopedValue.GetBool();

	// parse swap texture x/y attribute
	if(!attributesValue.HasMember(JSON_SWAP_TEXTURE_XY_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Partition attribute '{}' is missing.", JSON_SWAP_TEXTURE_XY_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & swapTextureXYValue = attributesValue[JSON_SWAP_TEXTURE_XY_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!swapTextureXYValue.IsBool()) {
		spdlog::error("Partition has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_SWAP_TEXTURE_XY_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(swapTextureXYValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool swapTextureXY = swapTextureXYValue.GetBool();

	// parse double smooshiness attribute
	if(!attributesValue.HasMember(JSON_DOUBLE_SMOOSHINESS_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Partition attribute '{}' is missing.", JSON_DOUBLE_SMOOSHINESS_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & doubleSmooshinessValue = attributesValue[JSON_DOUBLE_SMOOSHINESS_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!doubleSmooshinessValue.IsBool()) {
		spdlog::error("Partition has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_DOUBLE_SMOOSHINESS_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(doubleSmooshinessValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool doubleSmooshiness = doubleSmooshinessValue.GetBool();

	// parse x flipped attribute
	if(!attributesValue.HasMember(JSON_X_FLIPPED_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Partition attribute '{}' is missing.", JSON_X_FLIPPED_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & xFlippedValue = attributesValue[JSON_X_FLIPPED_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!xFlippedValue.IsBool()) {
		spdlog::error("Partition has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_X_FLIPPED_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(xFlippedValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool xFlipped = xFlippedValue.GetBool();

	// parse y flipped attribute
	if(!attributesValue.HasMember(JSON_Y_FLIPPED_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Partition attribute '{}' is missing.", JSON_Y_FLIPPED_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & yFlippedValue = attributesValue[JSON_Y_FLIPPED_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!yFlippedValue.IsBool()) {
		spdlog::error("Partition has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_Y_FLIPPED_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(yFlippedValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool yFlipped = yFlippedValue.GetBool();

	// parse texture align attribute
	if(!attributesValue.HasMember(JSON_TEXTURE_ALIGN_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Partition attribute '{}' is missing.", JSON_TEXTURE_ALIGN_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & textureAlignValue = attributesValue[JSON_TEXTURE_ALIGN_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!textureAlignValue.IsBool()) {
		spdlog::error("Partition has an invalid '{}' attribute type: '{}', expected 'boolean'.", JSON_TEXTURE_ALIGN_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(textureAlignValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	bool textureAlign = textureAlignValue.GetBool();

	// parse reserved attribute
	if(!attributesValue.HasMember(JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Partition attribute '{}' is missing.", JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & reservedValue = attributesValue[JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!reservedValue.IsUint()) {
		spdlog::error("Partition has an invalid '{}' attribute type: '{}', expected unsigned integer 'number'.", JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(reservedValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	uint32_t reserved = reservedValue.GetUint();

	if(reserved > 511) {
		spdlog::error("Partition has an invalid '{}' attribute value: {}, expected integer 'number' between 0 and {}, inclusively.", JSON_RESERVED_ATTRIBUTE_PROPERTY_NAME, reserved, 511);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	return Attributes({ parallaxed, sloped, swapTextureXY, doubleSmooshiness, xFlipped, yFlipped, textureAlign, static_cast<uint16_t>(reserved) });
}

std::optional<Partition::Attributes> Partition::Attributes::parseFrom(const rapidjson::Value & attributesValue) {
	bool error = false;

	Attributes value(parseFrom(attributesValue, &error));

	if(error) {
		return {};
	}

	return value;
}

bool Partition::Attributes::operator == (const Attributes & attributes) const {
	return rawValue == attributes.rawValue;
}

bool Partition::Attributes::operator != (const Attributes & attributes) const {
	return !operator == (attributes);
}
