#include "Tile.h"

#include <ByteBuffer.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

static const std::string JSON_X_OFFSET_ATTRIBUTE_PROPERTY_NAME("xOffset");
static const std::string JSON_Y_OFFSET_ATTRIBUTE_PROPERTY_NAME("yOffset");
static const std::string JSON_NUMBER_OF_ANIMATED_FRAMES_ATTRIBUTE_PROPERTY_NAME("numberOfAnimatedFrames");
static const std::string JSON_ANIMATION_TYPE_ATTRIBUTE_PROPERTY_NAME("animationType");
static const std::string JSON_ANIMATION_SPEED_ATTRIBUTE_PROPERTY_NAME("animationSpeed");
static const std::string JSON_EXTRA_ATTRIBUTE_PROPERTY_NAME("extra");

static const std::string JSON_NUMBER_PROPERTY_NAME("number");
static const std::string JSON_WIDTH_PROPERTY_NAME("width");
static const std::string JSON_HEIGHT_PROPERTY_NAME("height");
static const std::string JSON_ATTRIBUTES_PROPERTY_NAME("attributes");
static const std::string JSON_DATA_PROPERTY_NAME("data");

static constexpr bool BASE_64_ENCODE_DATA = false;

const std::map<uint32_t, uint8_t> Tile::SPECIAL_TILE_PALETTE_LOOKUP_TABLE_NUMBERS = {
	{ 2492, 3 }, // 3D Realms Logo
	{ 2493, 2 }, // Title Screen Background
	{ 2497, 2 }, // Duke Nukem
	{ 2498, 2 }, // 3D
	{ 3260, 4 }, // Episode 1 Ending Animation
	{ 3261, 4 }, 
	{ 3262, 4 },
	{ 3263, 4 },
	{ 3264, 4 },
	{ 3265, 4 },
	{ 3266, 4 },
	{ 3267, 4 },
	{ 3268, 4 }
};

Tile::Tile(Art * parent)
	: m_number(0)
	, m_width(0)
	, m_height(0)
	, m_attributes({ 0 })
	, m_parent(parent) { }

Tile::Tile(uint32_t number, Art * parent )
	: m_number(number)
	, m_width(0)
	, m_height(0)
	, m_attributes({ 0 })
	, m_parent(parent) { }

Tile::Tile(uint32_t number, uint16_t width, uint16_t height, const Attributes & attributes, std::unique_ptr<ByteBuffer> data, Art * parent)
	: m_number(number)
	, m_width(width)
	, m_height(height)
	, m_attributes(attributes)
	, m_data(std::shared_ptr<ByteBuffer>(data.release()))
	, m_parent(parent) { }

Tile::Tile(uint32_t number, uint16_t width, uint16_t height, const Attributes & attributes, ByteBuffer && data, Art * parent)
	: m_number(number)
	, m_width(width)
	, m_height(height)
	, m_attributes(attributes)
	, m_data(std::make_shared<ByteBuffer>(std::move(data)))
	, m_parent(parent) { }

Tile::Tile(Tile && tile) noexcept
	: m_number(tile.m_number)
	, m_width(tile.m_width)
	, m_height(tile.m_height)
	, m_attributes(tile.m_attributes)
	, m_data(std::move(tile.m_data))
	, m_parent(nullptr) { }

Tile::Tile(const Tile & tile)
	: m_number(tile.m_number)
	, m_width(tile.m_width)
	, m_height(tile.m_height)
	, m_attributes(tile.m_attributes)
	, m_data(tile.m_data != nullptr ? std::make_shared<ByteBuffer>(*tile.m_data) : nullptr)
	, m_parent(nullptr) { }

Tile & Tile::operator = (Tile && tile) noexcept {
	if(this != &tile) {
		m_number = tile.m_number;
		m_width = tile.m_width;
		m_height = tile.m_height;
		m_attributes = tile.m_attributes;
		m_data = std::move(tile.m_data);
	}

	return *this;
}

Tile & Tile::operator = (const Tile & tile) {
	m_number = tile.m_number;
	m_width = tile.m_width;
	m_height = tile.m_height;
	m_attributes = tile.m_attributes;

	if(tile.m_data != nullptr) {
		m_data = std::make_shared<ByteBuffer>(*tile.m_data);
	}
	else {
		m_data.reset();
	}

	return *this;
}

Tile::~Tile() = default;

int8_t Tile::getXOffset() const {
	return m_attributes.xOffset;
}

void Tile::setXOffset(int8_t xOffset) {
	m_attributes.xOffset = xOffset;
}

int8_t Tile::getYOffset() const {
	return m_attributes.yOffset;
}

void Tile::setYOffset(int8_t yOffset) {
	m_attributes.yOffset = yOffset;
}

uint8_t Tile::getNumberOfAnimatedFrames() const {
	return m_attributes.numberOfAnimatedFrames;
}

bool Tile::setNumberOfAnimatedFrames(uint8_t numberOfAnimatedFrames) {
	if(numberOfAnimatedFrames > 63) {
		return false;
	}

	m_attributes.numberOfAnimatedFrames = numberOfAnimatedFrames;

	return true;
}

Tile::AnimationType Tile::getAnimationType() const {
	return m_attributes.animationType;
}

void Tile::setAnimationType(AnimationType animationType) {
	m_attributes.animationType = animationType;
}

uint8_t Tile::getAnimationSpeed() const {
	return m_attributes.animationSpeed;
}

bool Tile::setAnimationSpeed(uint8_t animationSpeed) {
	if(animationSpeed > 15) {
		return false;
	}

	m_attributes.animationSpeed = animationSpeed;

	return true;
}

uint8_t Tile::getExtra() const {
	return m_attributes.extra;
}

bool Tile::setExtra(uint8_t extra) {
	if(extra > 15) {
		return false;
	}

	m_attributes.extra = extra;

	return true;
}

Tile::Attributes & Tile::getAttributes() {
	return m_attributes;
}

const Tile::Attributes & Tile::getAttributes() const {
	return m_attributes;
}

void Tile::setAttributes(const Attributes & attributes) {
	m_attributes = attributes;
}

bool Tile::setAttributes(int8_t xOffset, int8_t yOffset, uint8_t numberOfAnimatedFrames, AnimationType animationType, uint8_t animationSpeed, uint8_t extra) {
	if(numberOfAnimatedFrames > 63 || animationSpeed > 15 || extra > 15) {
		return false;
	}

	m_attributes.xOffset = xOffset;
	m_attributes.yOffset = yOffset;
	m_attributes.numberOfAnimatedFrames = numberOfAnimatedFrames;
	m_attributes.animationType = animationType;
	m_attributes.animationSpeed = animationSpeed;
	m_attributes.extra = extra;

	return true;
}

uint32_t Tile::getNumber() const {
	return m_number;
}

void Tile::setNumber(uint32_t number) {
	m_number = number;
}

bool Tile::hasName() const {
	return NAMES.find(m_number) != NAMES.cend();
}

const std::string & Tile::getName() const {
	auto nameIterator = NAMES.find(m_number);

	if(nameIterator == NAMES.cend()) {
		return Utilities::emptyString;
	}

	return nameIterator->second;
}

bool Tile::isEmpty() const {
	return m_data == nullptr ||
		   m_data->isEmpty();
}

bool Tile::isNotEmpty() const {
	return m_data != nullptr &&
		   m_data->isNotEmpty();
}

uint16_t Tile::getWidth() const {
	return m_width;
}

void Tile::setWidth(uint16_t width) {
	m_width = width;
}

uint16_t Tile::getHeight() const {
	return m_height;
}

void Tile::setHeight(uint16_t height) {
	m_height = height;
}

size_t Tile::getSize() const {
	if(m_data == nullptr) {
		return 0;
	}

	return m_data->getSize();
}

std::shared_ptr<ByteBuffer> Tile::getData() const {
	return m_data;
}

void Tile::setData(std::unique_ptr<ByteBuffer> data) {
	if(data == nullptr) {
		clearData();
		return;
	}

	m_data = std::shared_ptr<ByteBuffer>(data.release());
}

void Tile::clearData() {
	m_data = nullptr;
}

void Tile::clear() {
	m_width = 0;
	m_height = 0;
	m_data.reset();
	m_attributes.rawValue = 0;
}

Art * Tile::getParent() {
	return m_parent;
}

const Art * Tile::getParent() const {
	return m_parent;
}

void Tile::setParent(Art * art) {
	m_parent = art;
}

void Tile::clearParent() {
	m_parent = nullptr;
}

rapidjson::Value Tile::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value tileValue(rapidjson::kObjectType);

	tileValue.AddMember(rapidjson::StringRef(JSON_NUMBER_PROPERTY_NAME.c_str()), rapidjson::Value(m_number), allocator);

	tileValue.AddMember(rapidjson::StringRef(JSON_WIDTH_PROPERTY_NAME.c_str()), rapidjson::Value(m_width), allocator);

	tileValue.AddMember(rapidjson::StringRef(JSON_HEIGHT_PROPERTY_NAME.c_str()), rapidjson::Value(m_height), allocator);

	rapidjson::Value attributesValue(m_attributes.toJSON(allocator));
	tileValue.AddMember(rapidjson::StringRef(JSON_ATTRIBUTES_PROPERTY_NAME.c_str()), attributesValue, allocator);

	if(m_data != nullptr && m_data->isNotEmpty()) {
		if(BASE_64_ENCODE_DATA) {
			std::string base64Data(m_data->toBase64());
			rapidjson::Value dataValue(base64Data.c_str(), allocator);
			tileValue.AddMember(rapidjson::StringRef(JSON_DATA_PROPERTY_NAME.c_str()), dataValue, allocator);
		}
		else {
			rapidjson::Value dataValue(rapidjson::kArrayType);

			for(size_t i = 0; i < m_data->getSize(); i++) {
				dataValue.PushBack(rapidjson::Value((*m_data)[i]), allocator);
			}

			tileValue.AddMember(rapidjson::StringRef(JSON_DATA_PROPERTY_NAME.c_str()), dataValue, allocator);
		}
	}

	return tileValue;
}

std::unique_ptr<Tile> Tile::parseFrom(const rapidjson::Value & tileValue) {
	if(!tileValue.IsObject()) {
		spdlog::error("Invalid tile type: '{}', expected 'object'.", Utilities::typeToString(tileValue.GetType()));
		return nullptr;
	}

	// parse number
	if(!tileValue.HasMember(JSON_NUMBER_PROPERTY_NAME.c_str())) {
		spdlog::error("Tile is missing '{}' property.", JSON_NUMBER_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & numberValue = tileValue[JSON_NUMBER_PROPERTY_NAME.c_str()];

	if(!numberValue.IsUint()) {
		spdlog::error("Tile has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_NUMBER_PROPERTY_NAME, Utilities::typeToString(numberValue.GetType()));
		return nullptr;
	}

	uint32_t number = numberValue.GetUint();

	// parse width
	if(!tileValue.HasMember(JSON_WIDTH_PROPERTY_NAME.c_str())) {
		spdlog::error("Tile #{} is missing '{}' property.", number, JSON_WIDTH_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & widthValue = tileValue[JSON_WIDTH_PROPERTY_NAME.c_str()];

	if(!widthValue.IsUint()) {
		spdlog::error("Tile #{} has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", number, JSON_WIDTH_PROPERTY_NAME, Utilities::typeToString(widthValue.GetType()));
		return nullptr;
	}

	uint32_t width = widthValue.GetUint();

	if(width > std::numeric_limits<uint16_t>::max()) {
		spdlog::error("Invalid tile #{} width: {}, expected a value between 0 and {}, inclusively.", number, width, std::numeric_limits<uint16_t>::max());
		return nullptr;
	}

	// parse height
	if(!tileValue.HasMember(JSON_HEIGHT_PROPERTY_NAME.c_str())) {
		spdlog::error("Tile #{} is missing '{}' property.", number, JSON_HEIGHT_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & heightValue = tileValue[JSON_HEIGHT_PROPERTY_NAME.c_str()];

	if(!heightValue.IsUint()) {
		spdlog::error("Tile #{} has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", number, JSON_HEIGHT_PROPERTY_NAME, Utilities::typeToString(heightValue.GetType()));
		return nullptr;
	}

	uint32_t height = heightValue.GetUint();

	if(height > std::numeric_limits<uint16_t>::max()) {
		spdlog::error("Invalid tile #{} height: {}, expected a value between 0 and {}, inclusively.", number, height, std::numeric_limits<uint16_t>::max());
		return nullptr;
	}

	// parse attributes
	if(!tileValue.HasMember(JSON_ATTRIBUTES_PROPERTY_NAME.c_str())) {
		spdlog::error("Tile #{} is missing '{}' property.", number, JSON_ATTRIBUTES_PROPERTY_NAME);
		return nullptr;
	}

	std::optional<Attributes> optionalAttributes(Attributes::parseFrom(tileValue[JSON_ATTRIBUTES_PROPERTY_NAME.c_str()]));

	if(!optionalAttributes.has_value()) {
		spdlog::error("Tile #{} has invalid attributes.", number, JSON_ATTRIBUTES_PROPERTY_NAME);
		return nullptr;
	}

	// parse base 64 encoded data
	if(!tileValue.HasMember(JSON_DATA_PROPERTY_NAME.c_str())) {
		return std::make_unique<Tile>(number, static_cast<uint16_t>(width), static_cast<uint16_t>(height), optionalAttributes.value());
	}

	const rapidjson::Value & dataValue = tileValue[JSON_DATA_PROPERTY_NAME.c_str()];

	if(dataValue.IsArray()) {
		std::unique_ptr<ByteBuffer> data(std::make_unique<ByteBuffer>());
		data->reserve(dataValue.Size());

		for(rapidjson::Value::ConstValueIterator i = dataValue.Begin(); i != dataValue.End(); ++i) {
			const rapidjson::Value & dataByteValue = *i;

			if(!dataByteValue.IsUint()) {
				spdlog::error("Tile #{} '{}' property byte #{} has an invalid type: '{}', expected unsigned byte 'number'.", number, JSON_DATA_PROPERTY_NAME, data->getSize(), Utilities::typeToString(dataByteValue.GetType()));
				return nullptr;
			}

			uint32_t dataByte = dataByteValue.GetUint();

			if(dataByte > std::numeric_limits<uint8_t>::max()) {
				spdlog::error("Tile #{} '{}' property byte #{} has an invalid value: {}, expected unsigned byte 'number' between 0 and {}, inclusively.", number, JSON_DATA_PROPERTY_NAME, data->getSize(), dataByte, std::numeric_limits<uint8_t>::max());
				return nullptr;
			}

			data->writeByte(static_cast<uint8_t>(dataByte));
		}

		return std::make_unique<Tile>(number, static_cast<uint16_t>(width), static_cast<uint16_t>(height), optionalAttributes.value(), std::move(data));
	}
	else if(dataValue.IsString()) {
		std::optional<ByteBuffer> optionalData(ByteBuffer::fromBase64(dataValue.GetString()));

		if(!optionalData.has_value()) {
			spdlog::error("Tile #{} has invalid base 64 encoded data.", number);
			return nullptr;
		}

		return std::make_unique<Tile>(number, static_cast<uint16_t>(width), static_cast<uint16_t>(height), optionalAttributes.value(), std::move(optionalData.value()));
	}
	else {
		spdlog::error("Tile #{} has an invalid '{}' property type: '{}', expected 'array' or 'string'.", number, JSON_DATA_PROPERTY_NAME, Utilities::typeToString(dataValue.GetType()));
		return nullptr;
	}
}

bool Tile::operator == (const Tile & tile) const {
	return m_number == tile.m_number &&
		   m_width == tile.m_width &&
		   m_height == tile.m_height &&
		   m_attributes == tile.m_attributes &&
		   (m_data == nullptr && tile.m_data == nullptr || (m_data != nullptr && tile.m_data != nullptr && *m_data == *tile.m_data));
}

bool Tile::operator != (const Tile & tile) const {
	return !operator == (tile);
}

rapidjson::Value Tile::Attributes::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value attributesValue(rapidjson::kObjectType);

	addToJSONObject(attributesValue, allocator);

	return attributesValue;
}

bool Tile::Attributes::addToJSONObject(rapidjson::Value & value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	if(!value.IsObject()) {
		return false;
	}

	value.AddMember(rapidjson::StringRef(JSON_X_OFFSET_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(xOffset), allocator);

	value.AddMember(rapidjson::StringRef(JSON_Y_OFFSET_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(yOffset), allocator);

	value.AddMember(rapidjson::StringRef(JSON_NUMBER_OF_ANIMATED_FRAMES_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(numberOfAnimatedFrames), allocator);

	rapidjson::Value animationTypeValue(std::string(magic_enum::enum_name(animationType)).c_str(), allocator);
	value.AddMember(rapidjson::StringRef(JSON_ANIMATION_TYPE_ATTRIBUTE_PROPERTY_NAME.c_str()), animationTypeValue, allocator);

	value.AddMember(rapidjson::StringRef(JSON_ANIMATION_SPEED_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(animationSpeed), allocator);

	value.AddMember(rapidjson::StringRef(JSON_EXTRA_ATTRIBUTE_PROPERTY_NAME.c_str()), rapidjson::Value(extra), allocator);

	return true;
}

Tile::Attributes Tile::Attributes::parseFrom(const rapidjson::Value & attributesValue, bool * error) {
	if(!attributesValue.IsObject()) {
		spdlog::error("Invalid tile attributes type: '{}', expected 'object'.", Utilities::typeToString(attributesValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse x offset attribute
	if(!attributesValue.HasMember(JSON_X_OFFSET_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Tile attributes is missing '{}' property.", JSON_X_OFFSET_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & xOffsetValue = attributesValue[JSON_X_OFFSET_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!xOffsetValue.IsInt()) {
		spdlog::error("Tile attributes has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_X_OFFSET_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(xOffsetValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	int32_t xOffset = xOffsetValue.GetInt();

	if(xOffset < std::numeric_limits<int8_t>::min() || xOffset > std::numeric_limits<int8_t>::max()) {
		spdlog::error("Invalid tile x offset attribute: {}, expected a value between {} and {}, inclusively.", xOffset, std::numeric_limits<int8_t>::min(), std::numeric_limits<int8_t>::max());

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse y offset attribute
	if(!attributesValue.HasMember(JSON_Y_OFFSET_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Tile attributes is missing '{}' property.", JSON_Y_OFFSET_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & yOffsetValue = attributesValue[JSON_Y_OFFSET_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!yOffsetValue.IsInt()) {
		spdlog::error("Tile attributes has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_Y_OFFSET_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(yOffsetValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	int32_t yOffset = yOffsetValue.GetInt();

	if(yOffset < std::numeric_limits<int8_t>::min() || yOffset > std::numeric_limits<int8_t>::max()) {
		spdlog::error("Invalid tile y offset attribute: {}, expected a value between {} and {}, inclusively.", yOffset, std::numeric_limits<int8_t>::min(), std::numeric_limits<int8_t>::max());

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse number of animated frames attribute
	if(!attributesValue.HasMember(JSON_NUMBER_OF_ANIMATED_FRAMES_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Tile attributes is missing '{}' property.", JSON_NUMBER_OF_ANIMATED_FRAMES_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & numberOfAnimatedFramesValue = attributesValue[JSON_NUMBER_OF_ANIMATED_FRAMES_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!numberOfAnimatedFramesValue.IsUint()) {
		spdlog::error("Tile attributes has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_NUMBER_OF_ANIMATED_FRAMES_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(numberOfAnimatedFramesValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	uint32_t numberOfAnimatedFrames = numberOfAnimatedFramesValue.GetUint();

	if(numberOfAnimatedFrames > 63) {
		spdlog::error("Invalid tile number of animated frames attribute: {}, expected a value between 0 and 63, inclusively.", numberOfAnimatedFrames);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse animation type attribute
	std::optional<AnimationType> optionalAnimationType;

	if(!attributesValue.HasMember(JSON_ANIMATION_TYPE_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Tile attribute '{}' is missing.", JSON_ANIMATION_TYPE_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & animationTypeValue = attributesValue[JSON_ANIMATION_TYPE_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(animationTypeValue.IsUint()) {
		optionalAnimationType = magic_enum::enum_cast<AnimationType>(animationTypeValue.GetUint());
	}
	else if(animationTypeValue.IsString()) {
		optionalAnimationType = magic_enum::enum_cast<AnimationType>(animationTypeValue.GetString());
	}
	else {
		spdlog::error("Tile has an invalid '{}' attribute type: '{}', expected 'string' or unsigned integer 'number'.", JSON_ANIMATION_TYPE_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(animationTypeValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	if(!optionalAnimationType.has_value()) {
		spdlog::error("Tile has an invalid '{}' attribute value: {}.", JSON_ANIMATION_TYPE_ATTRIBUTE_PROPERTY_NAME, Utilities::valueToString(animationTypeValue));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse animation speed attribute
	if(!attributesValue.HasMember(JSON_ANIMATION_SPEED_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Tile attributes is missing '{}' property.", JSON_ANIMATION_SPEED_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & animationSpeedValue = attributesValue[JSON_ANIMATION_SPEED_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!animationSpeedValue.IsUint()) {
		spdlog::error("Tile attributes has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_ANIMATION_SPEED_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(animationSpeedValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	uint32_t animationSpeed = animationSpeedValue.GetUint();

	if(animationSpeed > 15) {
		spdlog::error("Invalid tile animation speed attribute: {}, expected a value between 0 and 15, inclusively.", animationSpeed);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse extra attribute
	if(!attributesValue.HasMember(JSON_EXTRA_ATTRIBUTE_PROPERTY_NAME.c_str())) {
		spdlog::error("Tile attributes is missing '{}' property.", JSON_EXTRA_ATTRIBUTE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & extraValue = attributesValue[JSON_EXTRA_ATTRIBUTE_PROPERTY_NAME.c_str()];

	if(!extraValue.IsUint()) {
		spdlog::error("Tile attributes has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_EXTRA_ATTRIBUTE_PROPERTY_NAME, Utilities::typeToString(extraValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	uint32_t extra = extraValue.GetUint();

	if(extra > 15) {
		spdlog::error("Invalid tile extra attribute: {}, expected a value between 0 and 15, inclusively.", extra);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	return Attributes({ static_cast<uint8_t>(numberOfAnimatedFrames), optionalAnimationType.value(), static_cast<int8_t>(xOffset), static_cast<int8_t>(yOffset), static_cast<uint8_t>(animationSpeed), static_cast<uint8_t>(extra) });
}

std::optional<Tile::Attributes> Tile::Attributes::parseFrom(const rapidjson::Value & attributesValue) {
	bool error = false;

	Attributes value(parseFrom(attributesValue, &error));

	if(error) {
		return {};
	}

	return value;
}

bool Tile::Attributes::operator == (const Attributes & attributes) const {
	return rawValue == attributes.rawValue;
}

bool Tile::Attributes::operator != (const Attributes & attributes) const {
	return !operator == (attributes);
}
