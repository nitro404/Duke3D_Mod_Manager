#include "TexturedItem.h"

#include <string>

static const std::string JSON_TILE_NUMBER_PROPERTY_NAME("tileNumber");
static const std::string JSON_SHADE_PROPERTY_NAME("shade");
static const std::string JSON_PALETTE_LOOKUP_TABLE_NUMBER_PROPERTY_NAME("paletteLookupTableNumber");

#include <Utilities/RapidJSONUtilities.h>

#include <spdlog/spdlog.h>

TexturedItem::TexturedItem()
	: m_tileNumber(0)
	, m_shade(0)
	, m_paletteLookupTableNumber(0) { }

TexturedItem::TexturedItem(uint16_t tileNumber, int8_t shade, uint8_t paletteLookupTableNumber)
	: m_tileNumber(tileNumber)
	, m_shade(shade)
	, m_paletteLookupTableNumber(paletteLookupTableNumber) { }

TexturedItem::TexturedItem(const TexturedItem & texturedItem)
	: m_tileNumber(texturedItem.m_tileNumber)
	, m_shade(texturedItem.m_shade)
	, m_paletteLookupTableNumber(texturedItem.m_paletteLookupTableNumber) { }

TexturedItem & TexturedItem::operator = (const TexturedItem & texturedItem) {
	m_tileNumber = texturedItem.m_tileNumber;
	m_shade = texturedItem.m_shade;
	m_paletteLookupTableNumber = texturedItem.m_paletteLookupTableNumber;

	return *this;
}

TexturedItem::~TexturedItem() { }

uint16_t TexturedItem::getTileNumber() const {
	return m_tileNumber;
}

void TexturedItem::setTileNumber(uint16_t tileNumber) {
	m_tileNumber = tileNumber;
}

int8_t TexturedItem::getShade() const {
	return m_shade;
}

void TexturedItem::setShade(int8_t shade) {
	m_shade = shade;
}

uint8_t TexturedItem::getPaletteLookupTableNumber() const {
	return m_paletteLookupTableNumber;
}

void TexturedItem::setPaletteLookupTableNumber(uint8_t paletteLookupTableNumber) {
	m_paletteLookupTableNumber = paletteLookupTableNumber;
}

rapidjson::Value TexturedItem::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value texturedItemValue(rapidjson::kObjectType);

	addToJSONObject(texturedItemValue, allocator);

	return texturedItemValue;
}

bool TexturedItem::addToJSONObject(rapidjson::Value & value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	if(!value.IsObject()) {
		return false;
	}

	value.AddMember(rapidjson::StringRef(JSON_TILE_NUMBER_PROPERTY_NAME.c_str()), rapidjson::Value(m_tileNumber), allocator);

	value.AddMember(rapidjson::StringRef(JSON_SHADE_PROPERTY_NAME.c_str()), rapidjson::Value(m_shade), allocator);

	value.AddMember(rapidjson::StringRef(JSON_PALETTE_LOOKUP_TABLE_NUMBER_PROPERTY_NAME.c_str()), rapidjson::Value(m_paletteLookupTableNumber), allocator);

	return true;
}

TexturedItem TexturedItem::parseFrom(const rapidjson::Value & texturedItemValue, bool * error) {
	if(!texturedItemValue.IsObject()) {
		spdlog::error("Invalid textured item type: '{}', expected 'object'.", Utilities::typeToString(texturedItemValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse tile number
	if(!texturedItemValue.HasMember(JSON_TILE_NUMBER_PROPERTY_NAME.c_str())) {
		spdlog::error("Textured item is missing '{}' property.", JSON_TILE_NUMBER_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & tileNumberValue = texturedItemValue[JSON_TILE_NUMBER_PROPERTY_NAME.c_str()];

	if(!tileNumberValue.IsUint()) {
		spdlog::error("Textured item has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_TILE_NUMBER_PROPERTY_NAME, Utilities::typeToString(tileNumberValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	uint32_t tileNumber = tileNumberValue.GetUint();

	if(tileNumber > std::numeric_limits<uint16_t>::max()) {
		spdlog::error("Invalid textured item tile number: {}, expected a value between 0 and {}, inclusively.", tileNumber, std::numeric_limits<uint16_t>::max());

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse shade
	if(!texturedItemValue.HasMember(JSON_SHADE_PROPERTY_NAME.c_str())) {
		spdlog::error("Textured item is missing '{}' property.", JSON_SHADE_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & shadeValue = texturedItemValue[JSON_SHADE_PROPERTY_NAME.c_str()];

	if(!shadeValue.IsInt()) {
		spdlog::error("Textured item has an invalid '{}' property type: '{}', expected integer 'number'.", JSON_SHADE_PROPERTY_NAME, Utilities::typeToString(shadeValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	int32_t shade = shadeValue.GetInt();

	if(shade < std::numeric_limits<int8_t>::min() || shade > std::numeric_limits<int8_t>::max()) {
		spdlog::error("Invalid textured item shade: {}, expected a value between {} and {}, inclusively.", shade, std::numeric_limits<int8_t>::min(), std::numeric_limits<int8_t>::max());

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	// parse palette lookup table number
	if(!texturedItemValue.HasMember(JSON_PALETTE_LOOKUP_TABLE_NUMBER_PROPERTY_NAME.c_str())) {
		spdlog::error("Textured item is missing '{}' property.", JSON_PALETTE_LOOKUP_TABLE_NUMBER_PROPERTY_NAME);

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	const rapidjson::Value & paletteLookupTableNumberValue = texturedItemValue[JSON_PALETTE_LOOKUP_TABLE_NUMBER_PROPERTY_NAME.c_str()];

	if(!paletteLookupTableNumberValue.IsUint()) {
		spdlog::error("Textured item has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_PALETTE_LOOKUP_TABLE_NUMBER_PROPERTY_NAME, Utilities::typeToString(paletteLookupTableNumberValue.GetType()));

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	uint32_t paletteLookupTableNumber = paletteLookupTableNumberValue.GetUint();

	if(paletteLookupTableNumber > std::numeric_limits<uint8_t>::max()) {
		spdlog::error("Invalid textured item palette lookup table number: {}, expected a value between 0 and {}, inclusively.", paletteLookupTableNumber, std::numeric_limits<uint8_t>::max());

		if(error != nullptr) {
			*error = true;
		}

		return {};
	}

	return TexturedItem(static_cast<uint16_t>(tileNumber), static_cast<int8_t>(shade), static_cast<uint8_t>(paletteLookupTableNumber));
}

std::optional<TexturedItem> TexturedItem::parseFrom(const rapidjson::Value & texturedItemValue) {
	bool error = false;

	TexturedItem value(parseFrom(texturedItemValue, &error));

	if(error) {
		return {};
	}

	return value;
}

bool TexturedItem::operator == (const TexturedItem & texturedItem) const {
	return m_tileNumber == texturedItem.m_tileNumber &&
		   m_shade == texturedItem.m_shade &&
		   m_paletteLookupTableNumber == texturedItem.m_paletteLookupTableNumber;
}

bool TexturedItem::operator != (const TexturedItem & texturedItem) const {
	return !operator == (texturedItem);
}
