#include "ModVideo.h"

#include "Mod.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <fmt/core.h>
#include <tinyxml2.h>

#include <string_view>
#include <vector>

static const std::string XML_MOD_VIDEO_ELEMENT_NAME("video");
static const std::string XML_MOD_VIDEO_URL_ATTRIBUTE_NAME("url");
static const std::string XML_MOD_VIDEO_WIDTH_ATTRIBUTE_NAME("width");
static const std::string XML_MOD_VIDEO_HEIGHT_ATTRIBUTE_NAME("height");
static const std::vector<std::string> XML_MOD_VIDEO_ATTRIBUTE_NAMES = {
	XML_MOD_VIDEO_URL_ATTRIBUTE_NAME,
	XML_MOD_VIDEO_WIDTH_ATTRIBUTE_NAME,
	XML_MOD_VIDEO_HEIGHT_ATTRIBUTE_NAME
};

static constexpr const char * JSON_MOD_VIDEO_URL_PROPERTY_NAME = "url";
static constexpr const char * JSON_MOD_VIDEO_WIDTH_PROPERTY_NAME = "width";
static constexpr const char * JSON_MOD_VIDEO_HEIGHT_PROPERTY_NAME = "height";
static const std::vector<std::string_view> JSON_MOD_VIDEO_PROPERTY_NAMES = {
	JSON_MOD_VIDEO_URL_PROPERTY_NAME,
	JSON_MOD_VIDEO_WIDTH_PROPERTY_NAME,
	JSON_MOD_VIDEO_HEIGHT_PROPERTY_NAME
};

ModVideo::ModVideo(const std::string & url, uint16_t width, uint16_t height)
	: m_url(Utilities::trimString(url))
	, m_width(width)
	, m_height(height)
	, m_parentMod(nullptr) { }

ModVideo::ModVideo(ModVideo && v) noexcept
	: m_url(std::move(v.m_url))
	, m_width(v.m_width)
	, m_height(v.m_height)
	, m_parentMod(nullptr) { }

ModVideo::ModVideo(const ModVideo & v)
	: m_url(v.m_url)
	, m_width(v.m_width)
	, m_height(v.m_height)
	, m_parentMod(nullptr) { }

ModVideo & ModVideo::operator = (ModVideo && v) noexcept {
	if(this != &v) {
		m_url = std::move(v.m_url);
		m_width = v.m_width;
		m_height = v.m_height;
	}

	return *this;
}

ModVideo & ModVideo::operator = (const ModVideo & v) {
	m_url = v.m_url;
	m_width = v.m_width;
	m_height = v.m_height;

	return *this;
}

ModVideo::~ModVideo() {
	m_parentMod = nullptr;
}

const std::string & ModVideo::getURL() const {
	return m_url;
}

uint16_t ModVideo::getWidth() const {
	return m_width;
}

uint16_t ModVideo::getHeight() const {
	return m_height;
}

const Mod * ModVideo::getParentMod() const {
	return m_parentMod;
}

void ModVideo::setURL(const std::string & url) {
	m_url = Utilities::trimString(url);
}

void ModVideo::setWidth(uint16_t width) {
	m_width = width;
}

void ModVideo::setHeight(uint16_t height) {
	m_height = height;
}

void ModVideo::setParentMod(const Mod * mod) {
	m_parentMod = mod;
}

rapidjson::Value ModVideo::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value modVideoValue(rapidjson::kObjectType);

	rapidjson::Value urlValue(m_url.c_str(), allocator);
	modVideoValue.AddMember(rapidjson::StringRef(JSON_MOD_VIDEO_URL_PROPERTY_NAME), urlValue, allocator);

	modVideoValue.AddMember(rapidjson::StringRef(JSON_MOD_VIDEO_WIDTH_PROPERTY_NAME), rapidjson::Value(m_width), allocator);

	modVideoValue.AddMember(rapidjson::StringRef(JSON_MOD_VIDEO_HEIGHT_PROPERTY_NAME), rapidjson::Value(m_height), allocator);

	return modVideoValue;
}

tinyxml2::XMLElement * ModVideo::toXML(tinyxml2::XMLDocument * document) const {
	if(document == nullptr) {
		return nullptr;
	}

	tinyxml2::XMLElement * modVideoElement = document->NewElement(XML_MOD_VIDEO_ELEMENT_NAME.c_str());

	modVideoElement->SetAttribute(XML_MOD_VIDEO_URL_ATTRIBUTE_NAME.c_str(), m_url.c_str());
	modVideoElement->SetAttribute(XML_MOD_VIDEO_WIDTH_ATTRIBUTE_NAME.c_str(), m_width);
	modVideoElement->SetAttribute(XML_MOD_VIDEO_HEIGHT_ATTRIBUTE_NAME.c_str(), m_height);

	return modVideoElement;
}

std::unique_ptr<ModVideo> ModVideo::parseFrom(const rapidjson::Value & modVideoValue) {
	if(!modVideoValue.IsObject()) {
		fmt::print("Invalid mod video type: '{}', expected 'object'.\n", Utilities::typeToString(modVideoValue.GetType()));
		return nullptr;
	}

	// check for unhandled mod video properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = modVideoValue.MemberBegin(); i != modVideoValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_MOD_VIDEO_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			fmt::print("Mod video has unexpected property '{}'.\n", i->name.GetString());
			return nullptr;
		}
	}

	// parse mod video url
	if(!modVideoValue.HasMember(JSON_MOD_VIDEO_URL_PROPERTY_NAME)) {
		fmt::print("Mod video is missing '{}' property'.\n", JSON_MOD_VIDEO_URL_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modVideoURLValue = modVideoValue[JSON_MOD_VIDEO_URL_PROPERTY_NAME];

	if(!modVideoURLValue.IsString()) {
		fmt::print("Mod video has an invalid '{}' property type: '{}', expected 'string'.\n", JSON_MOD_VIDEO_URL_PROPERTY_NAME, Utilities::typeToString(modVideoURLValue.GetType()));
		return nullptr;
	}

	std::string modVideoURL(Utilities::trimString(modVideoURLValue.GetString()));

	if(modVideoURL.empty()) {
		fmt::print("Mod video '{}' property cannot be empty.\n", JSON_MOD_VIDEO_URL_PROPERTY_NAME);
		return nullptr;
	}

	// parse mod video width
	if(!modVideoValue.HasMember(JSON_MOD_VIDEO_WIDTH_PROPERTY_NAME)) {
		fmt::print("Mod video is missing '{}' property'.\n", JSON_MOD_VIDEO_WIDTH_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modVideoWidthValue = modVideoValue[JSON_MOD_VIDEO_WIDTH_PROPERTY_NAME];

	if(!modVideoWidthValue.IsUint()) {
		fmt::print("Mod video has an invalid '{}' property type: '{}', expected unsigned integer 'number'.\n", JSON_MOD_VIDEO_WIDTH_PROPERTY_NAME, Utilities::typeToString(modVideoWidthValue.GetType()));
		return nullptr;
	}

	uint32_t modVideoWidth = modVideoWidthValue.GetUint();

	if(modVideoWidth > std::numeric_limits<uint16_t>::max()) {
		fmt::print("Mod video '{}' property value has an invalid value: '{}', expected unsigned integer 'number' between 1 and {} inclusively.\n", JSON_MOD_VIDEO_WIDTH_PROPERTY_NAME, modVideoWidth, std::numeric_limits<uint8_t>::max());
		return nullptr;
	}

	// parse mod video height
	if(!modVideoValue.HasMember(JSON_MOD_VIDEO_HEIGHT_PROPERTY_NAME)) {
		fmt::print("Mod video is missing '{}' property'.\n", JSON_MOD_VIDEO_HEIGHT_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modVideoHeightValue = modVideoValue[JSON_MOD_VIDEO_HEIGHT_PROPERTY_NAME];

	if(!modVideoHeightValue.IsUint()) {
		fmt::print("Mod video has an invalid '{}' property type: '{}', expected unsigned integer 'number'.\n", JSON_MOD_VIDEO_HEIGHT_PROPERTY_NAME, Utilities::typeToString(modVideoHeightValue.GetType()));
		return nullptr;
	}

	uint32_t modVideoHeight = modVideoHeightValue.GetUint();

	if(modVideoHeight > std::numeric_limits<uint16_t>::max()) {
		fmt::print("Mod video '{}' property value has an invalid value: '{}', expected unsigned integer 'number' between 1 and {} inclusively.\n", JSON_MOD_VIDEO_HEIGHT_PROPERTY_NAME, modVideoHeight, std::numeric_limits<uint8_t>::max());
		return nullptr;
	}

	// initialize the mod video
	return std::make_unique<ModVideo>(modVideoURL, static_cast<uint16_t>(modVideoWidth), static_cast<uint16_t>(modVideoHeight));
}

std::unique_ptr<ModVideo> ModVideo::parseFrom(const tinyxml2::XMLElement * modVideoElement) {
	if(modVideoElement == nullptr) {
		return nullptr;
	}

	// verify element name
	if(modVideoElement->Name() != XML_MOD_VIDEO_ELEMENT_NAME) {
		fmt::print("Invalid mod video element name: '{}', expected '{}'.\n", modVideoElement->Name(), XML_MOD_VIDEO_ELEMENT_NAME);
		return nullptr;
	}

	// check for unhandled mod video element attributes
	bool attributeHandled = false;
	const tinyxml2::XMLAttribute * modVideoAttribute = modVideoElement->FirstAttribute();

	while(true) {
		if(modVideoAttribute == nullptr) {
			break;
		}

		attributeHandled = false;

		for(const std::string & attributeName : XML_MOD_VIDEO_ATTRIBUTE_NAMES) {
			if(modVideoAttribute->Name() == attributeName) {
				attributeHandled = true;
				break;
			}
		}

		if(!attributeHandled) {
			fmt::print("Element '{}' has unexpected attribute '{}'.\n", XML_MOD_VIDEO_ELEMENT_NAME, modVideoAttribute->Name());
			return nullptr;
		}

		modVideoAttribute = modVideoAttribute->Next();
	}

	// check for unexpected mod video element child elements
	if(modVideoElement->FirstChildElement() != nullptr) {
		fmt::print("Element '{}' has an unexpected child element.\n", XML_MOD_VIDEO_ELEMENT_NAME);
		return nullptr;
	}

	// read the mod video attributes
	const char * modVideoURL = modVideoElement->Attribute(XML_MOD_VIDEO_URL_ATTRIBUTE_NAME.c_str());

	if(modVideoURL == nullptr || Utilities::stringLength(modVideoURL) == 0) {
		fmt::print("Attribute '{}' is missing from '{}' element.\n", XML_MOD_VIDEO_URL_ATTRIBUTE_NAME, XML_MOD_VIDEO_ELEMENT_NAME);
		return nullptr;
	}

	const char * modVideoWidthData = modVideoElement->Attribute(XML_MOD_VIDEO_WIDTH_ATTRIBUTE_NAME.c_str());

	if(modVideoWidthData == nullptr || Utilities::stringLength(modVideoWidthData) == 0) {
		fmt::print("Attribute '{}' is missing from '{}' element.\n", XML_MOD_VIDEO_WIDTH_ATTRIBUTE_NAME, XML_MOD_VIDEO_ELEMENT_NAME);
		return nullptr;
	}

	const char * modVideoHeightData = modVideoElement->Attribute(XML_MOD_VIDEO_HEIGHT_ATTRIBUTE_NAME.c_str());

	if(modVideoHeightData == nullptr || Utilities::stringLength(modVideoHeightData) == 0) {
		fmt::print("Attribute '{}' is missing from '{}' element.\n", XML_MOD_VIDEO_HEIGHT_ATTRIBUTE_NAME, XML_MOD_VIDEO_ELEMENT_NAME);
		return nullptr;
	}

	bool error = false;

	uint32_t modVideoWidth = Utilities::parseUnsignedInteger(modVideoWidthData, &error);

	if(error || modVideoWidth < 1 || modVideoWidth > std::numeric_limits<uint16_t>::max()) {
		fmt::print("Attribute '{}' in element '{}' has an invalid value: '{}', expected integer number between 1 and {} inclusively.\n", XML_MOD_VIDEO_WIDTH_ATTRIBUTE_NAME, XML_MOD_VIDEO_ELEMENT_NAME, modVideoWidthData, std::numeric_limits<uint16_t>::max());
		return nullptr;
	}

	uint32_t modVideoHeight = Utilities::parseUnsignedInteger(modVideoHeightData, &error);

	if(error || modVideoHeight < 1 || modVideoHeight > std::numeric_limits<uint16_t>::max()) {
		fmt::print("Attribute '{}' in element '{}' has an invalid value: '{}', expected integer number between 1 and {} inclusively.\n", XML_MOD_VIDEO_HEIGHT_ATTRIBUTE_NAME, XML_MOD_VIDEO_ELEMENT_NAME, modVideoHeightData, std::numeric_limits<uint16_t>::max());
		return nullptr;
	}

	// initialize the mod video
	std::unique_ptr<ModVideo> newModVideo = std::make_unique<ModVideo>(modVideoURL, static_cast<uint16_t>(modVideoWidth), static_cast<uint16_t>(modVideoHeight));

	return newModVideo;
}

bool ModVideo::isValid() const {
	return !m_url.empty() &&
		   m_width != 0 &&
		   m_height != 0;
}

bool ModVideo::isValid(const ModVideo * v) {
	return v != nullptr && v->isValid();
}

bool ModVideo::operator == (const ModVideo & v) const {
	return m_width == v.m_width &&
		   m_height == v.m_height &&
		   Utilities::compareStringsIgnoreCase(m_url, v.m_url);
}

bool ModVideo::operator != (const ModVideo & v) const {
	return !operator == (v);
}
