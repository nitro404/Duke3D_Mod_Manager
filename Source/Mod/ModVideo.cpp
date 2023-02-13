#include "ModVideo.h"

#include "Mod.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <array>
#include <string_view>

static const std::string XML_MOD_VIDEO_ELEMENT_NAME("video");
static const std::string XML_MOD_VIDEO_URL_ATTRIBUTE_NAME("url");
static const std::string XML_MOD_VIDEO_TITLE_ATTRIBUTE_NAME("title");
static const std::string XML_MOD_VIDEO_WIDTH_ATTRIBUTE_NAME("width");
static const std::string XML_MOD_VIDEO_HEIGHT_ATTRIBUTE_NAME("height");
static const std::array<std::string_view, 4> XML_MOD_VIDEO_ATTRIBUTE_NAMES = {
	XML_MOD_VIDEO_URL_ATTRIBUTE_NAME,
	XML_MOD_VIDEO_TITLE_ATTRIBUTE_NAME,
	XML_MOD_VIDEO_WIDTH_ATTRIBUTE_NAME,
	XML_MOD_VIDEO_HEIGHT_ATTRIBUTE_NAME
};

static constexpr const char * JSON_MOD_VIDEO_URL_PROPERTY_NAME = "url";
static constexpr const char * JSON_MOD_VIDEO_TITLE_PROPERTY_NAME = "title";
static constexpr const char * JSON_MOD_VIDEO_WIDTH_PROPERTY_NAME = "width";
static constexpr const char * JSON_MOD_VIDEO_HEIGHT_PROPERTY_NAME = "height";
static const std::array<std::string_view, 4> JSON_MOD_VIDEO_PROPERTY_NAMES = {
	JSON_MOD_VIDEO_URL_PROPERTY_NAME,
	JSON_MOD_VIDEO_TITLE_PROPERTY_NAME,
	JSON_MOD_VIDEO_WIDTH_PROPERTY_NAME,
	JSON_MOD_VIDEO_HEIGHT_PROPERTY_NAME
};

ModVideo::ModVideo(const std::string & url, const std::string & title, uint16_t width, uint16_t height)
	: m_url(Utilities::trimString(url))
	, m_title(Utilities::trimString(title))
	, m_width(width)
	, m_height(height)
	, m_parentMod(nullptr) { }

ModVideo::ModVideo(ModVideo && v) noexcept
	: m_url(std::move(v.m_url))
	, m_title(std::move(v.m_title))
	, m_width(v.m_width)
	, m_height(v.m_height)
	, m_parentMod(nullptr) { }

ModVideo::ModVideo(const ModVideo & v)
	: m_url(v.m_url)
	, m_title(v.m_title)
	, m_width(v.m_width)
	, m_height(v.m_height)
	, m_parentMod(nullptr) { }

ModVideo & ModVideo::operator = (ModVideo && v) noexcept {
	if(this != &v) {
		m_url = std::move(v.m_url);
		m_title = std::move(v.m_title);
		m_width = v.m_width;
		m_height = v.m_height;
	}

	return *this;
}

ModVideo & ModVideo::operator = (const ModVideo & v) {
	m_url = v.m_url;
	m_title = v.m_title;
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

const std::string & ModVideo::getTitle() const {
	return m_title;
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

void ModVideo::setTitle(const std::string & title) {
	m_title = Utilities::trimString(title);
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

	rapidjson::Value titleValue(m_title.c_str(), allocator);
	modVideoValue.AddMember(rapidjson::StringRef(JSON_MOD_VIDEO_TITLE_PROPERTY_NAME), titleValue, allocator);

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
	modVideoElement->SetAttribute(XML_MOD_VIDEO_TITLE_ATTRIBUTE_NAME.c_str(), m_title.c_str());
	modVideoElement->SetAttribute(XML_MOD_VIDEO_WIDTH_ATTRIBUTE_NAME.c_str(), m_width);
	modVideoElement->SetAttribute(XML_MOD_VIDEO_HEIGHT_ATTRIBUTE_NAME.c_str(), m_height);

	return modVideoElement;
}

std::unique_ptr<ModVideo> ModVideo::parseFrom(const rapidjson::Value & modVideoValue) {
	if(!modVideoValue.IsObject()) {
		spdlog::error("Invalid mod video type: '{}', expected 'object'.", Utilities::typeToString(modVideoValue.GetType()));
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
			spdlog::warn("Mod video has unexpected property '{}'.", i->name.GetString());
		}
	}

	// parse mod video url
	if(!modVideoValue.HasMember(JSON_MOD_VIDEO_URL_PROPERTY_NAME)) {
		spdlog::error("Mod video is missing '{}' property'.", JSON_MOD_VIDEO_URL_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modVideoURLValue = modVideoValue[JSON_MOD_VIDEO_URL_PROPERTY_NAME];

	if(!modVideoURLValue.IsString()) {
		spdlog::error("Mod video has an invalid '{}' property type: '{}', expected 'string'.", JSON_MOD_VIDEO_URL_PROPERTY_NAME, Utilities::typeToString(modVideoURLValue.GetType()));
		return nullptr;
	}

	std::string modVideoURL(Utilities::trimString(modVideoURLValue.GetString()));

	if(modVideoURL.empty()) {
		spdlog::error("Mod video '{}' property cannot be empty.", JSON_MOD_VIDEO_URL_PROPERTY_NAME);
		return nullptr;
	}

	// parse mod video title
	std::string modVideoTitle;

	if(modVideoValue.HasMember(JSON_MOD_VIDEO_TITLE_PROPERTY_NAME)) {
		const rapidjson::Value & modVideoTitleValue = modVideoValue[JSON_MOD_VIDEO_TITLE_PROPERTY_NAME];

		if(!modVideoTitleValue.IsString()) {
			spdlog::error("Mod video has an invalid '{}' property type: '{}', expected 'string'.", JSON_MOD_VIDEO_TITLE_PROPERTY_NAME, Utilities::typeToString(modVideoTitleValue.GetType()));
			return nullptr;
		}

		modVideoTitle = Utilities::trimString(modVideoTitleValue.GetString());

		if(modVideoTitle.empty()) {
			spdlog::error("Mod video '{}' property cannot be empty.", JSON_MOD_VIDEO_TITLE_PROPERTY_NAME);
			return nullptr;
		}
	}

	// parse mod video width
	uint32_t modVideoWidth = 0;

	if(modVideoValue.HasMember(JSON_MOD_VIDEO_WIDTH_PROPERTY_NAME)) {
		const rapidjson::Value & modVideoWidthValue = modVideoValue[JSON_MOD_VIDEO_WIDTH_PROPERTY_NAME];

		if(!modVideoWidthValue.IsUint()) {
			spdlog::error("Mod video has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_MOD_VIDEO_WIDTH_PROPERTY_NAME, Utilities::typeToString(modVideoWidthValue.GetType()));
			return nullptr;
		}

		modVideoWidth = modVideoWidthValue.GetUint();

		if(modVideoWidth > std::numeric_limits<uint16_t>::max()) {
			spdlog::error("Mod video '{}' property value has an invalid value: '{}', expected unsigned integer 'number' between 1 and {} inclusively.", JSON_MOD_VIDEO_WIDTH_PROPERTY_NAME, modVideoWidth, std::numeric_limits<uint8_t>::max());
			return nullptr;
		}
	}

	// parse mod video height
	uint32_t modVideoHeight = 0;

	if(modVideoValue.HasMember(JSON_MOD_VIDEO_HEIGHT_PROPERTY_NAME)) {
		const rapidjson::Value & modVideoHeightValue = modVideoValue[JSON_MOD_VIDEO_HEIGHT_PROPERTY_NAME];

		if(!modVideoHeightValue.IsUint()) {
			spdlog::error("Mod video has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_MOD_VIDEO_HEIGHT_PROPERTY_NAME, Utilities::typeToString(modVideoHeightValue.GetType()));
			return nullptr;
		}

		modVideoHeight = modVideoHeightValue.GetUint();

		if(modVideoHeight > std::numeric_limits<uint16_t>::max()) {
			spdlog::error("Mod video '{}' property value has an invalid value: '{}', expected unsigned integer 'number' between 1 and {} inclusively.", JSON_MOD_VIDEO_HEIGHT_PROPERTY_NAME, modVideoHeight, std::numeric_limits<uint8_t>::max());
			return nullptr;
		}
	}

	// initialize the mod video
	return std::make_unique<ModVideo>(modVideoURL, modVideoTitle, static_cast<uint16_t>(modVideoWidth), static_cast<uint16_t>(modVideoHeight));
}

std::unique_ptr<ModVideo> ModVideo::parseFrom(const tinyxml2::XMLElement * modVideoElement) {
	if(modVideoElement == nullptr) {
		return nullptr;
	}

	// verify element name
	if(modVideoElement->Name() != XML_MOD_VIDEO_ELEMENT_NAME) {
		spdlog::error("Invalid mod video element name: '{}', expected '{}'.", modVideoElement->Name(), XML_MOD_VIDEO_ELEMENT_NAME);
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

		for(const std::string_view & attributeName : XML_MOD_VIDEO_ATTRIBUTE_NAMES) {
			if(modVideoAttribute->Name() == attributeName) {
				attributeHandled = true;
				break;
			}
		}

		if(!attributeHandled) {
			spdlog::warn("Element '{}' has unexpected attribute '{}'.", XML_MOD_VIDEO_ELEMENT_NAME, modVideoAttribute->Name());
		}

		modVideoAttribute = modVideoAttribute->Next();
	}

	// check for unexpected mod video element child elements
	if(modVideoElement->FirstChildElement() != nullptr) {
		spdlog::warn("Element '{}' has an unexpected child element.", XML_MOD_VIDEO_ELEMENT_NAME);
	}

	// read the mod video attributes
	const char * modVideoURL = modVideoElement->Attribute(XML_MOD_VIDEO_URL_ATTRIBUTE_NAME.c_str());

	if(modVideoURL == nullptr || Utilities::stringLength(modVideoURL) == 0) {
		spdlog::error("Attribute '{}' is missing from '{}' element.", XML_MOD_VIDEO_URL_ATTRIBUTE_NAME, XML_MOD_VIDEO_ELEMENT_NAME);
		return nullptr;
	}

	const char * modVideoTitle = modVideoElement->Attribute(XML_MOD_VIDEO_TITLE_ATTRIBUTE_NAME.c_str());

	bool error = false;
	uint32_t modVideoWidth = 0;
	uint32_t modVideoHeight = 0;
	const char * modVideoWidthData = modVideoElement->Attribute(XML_MOD_VIDEO_WIDTH_ATTRIBUTE_NAME.c_str());
	const char * modVideoHeightData = modVideoElement->Attribute(XML_MOD_VIDEO_HEIGHT_ATTRIBUTE_NAME.c_str());

	if(modVideoWidthData != nullptr && Utilities::stringLength(modVideoWidthData) != 0) {
		modVideoWidth = Utilities::parseUnsignedInteger(modVideoWidthData, &error);

		if(error || modVideoWidth < 1 || modVideoWidth > std::numeric_limits<uint16_t>::max()) {
			spdlog::error("Attribute '{}' in element '{}' has an invalid value: '{}', expected integer number between 1 and {} inclusively.", XML_MOD_VIDEO_WIDTH_ATTRIBUTE_NAME, XML_MOD_VIDEO_ELEMENT_NAME, modVideoWidthData, std::numeric_limits<uint16_t>::max());
			return nullptr;
		}
	}

	if(modVideoHeightData != nullptr && Utilities::stringLength(modVideoHeightData) != 0) {
		modVideoHeight = Utilities::parseUnsignedInteger(modVideoHeightData, &error);

		if(error || modVideoHeight < 1 || modVideoHeight > std::numeric_limits<uint16_t>::max()) {
			spdlog::error("Attribute '{}' in element '{}' has an invalid value: '{}', expected integer number between 1 and {} inclusively.", XML_MOD_VIDEO_HEIGHT_ATTRIBUTE_NAME, XML_MOD_VIDEO_ELEMENT_NAME, modVideoHeightData, std::numeric_limits<uint16_t>::max());
			return nullptr;
		}
	}

	// initialize the mod video
	std::unique_ptr<ModVideo> newModVideo = std::make_unique<ModVideo>(modVideoURL, modVideoTitle != nullptr ? modVideoTitle : Utilities::emptyString, static_cast<uint16_t>(modVideoWidth), static_cast<uint16_t>(modVideoHeight));

	return newModVideo;
}

bool ModVideo::isValid() const {
	return !m_url.empty();
}

bool ModVideo::isValid(const ModVideo * v) {
	return v != nullptr && v->isValid();
}

bool ModVideo::operator == (const ModVideo & v) const {
	return m_width == v.m_width &&
		   m_height == v.m_height &&
		   Utilities::areStringsEqualIgnoreCase(m_url, v.m_url) &&
		   Utilities::areStringsEqualIgnoreCase(m_title, v.m_title);
}

bool ModVideo::operator != (const ModVideo & v) const {
	return !operator == (v);
}
