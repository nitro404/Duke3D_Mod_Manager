#include "ModImage.h"

#include "Mod.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <array>
#include <string_view>

static const std::string XML_MOD_IMAGE_ELEMENT_NAME("image");
static const std::string XML_MOD_IMAGE_FILE_NAME_ATTRIBUTE_NAME("filename");
static const std::string XML_MOD_IMAGE_FILE_SIZE_ATTRIBUTE_NAME("filesize");
static const std::string XML_MOD_IMAGE_TYPE_ATTRIBUTE_NAME("type");
static const std::string XML_MOD_IMAGE_SUBFOLDER_ATTRIBUTE_NAME("subfolder");
static const std::string XML_MOD_IMAGE_CAPTION_ATTRIBUTE_NAME("caption");
static const std::string XML_MOD_IMAGE_WIDTH_ATTRIBUTE_NAME("width");
static const std::string XML_MOD_IMAGE_HEIGHT_ATTRIBUTE_NAME("height");
static const std::string XML_MOD_IMAGE_SHA1_ATTRIBUTE_NAME("sha1");
static const std::array<std::string_view, 8> XML_MOD_IMAGE_ATTRIBUTE_NAMES = {
	XML_MOD_IMAGE_FILE_NAME_ATTRIBUTE_NAME,
	XML_MOD_IMAGE_FILE_SIZE_ATTRIBUTE_NAME,
	XML_MOD_IMAGE_TYPE_ATTRIBUTE_NAME,
	XML_MOD_IMAGE_SUBFOLDER_ATTRIBUTE_NAME,
	XML_MOD_IMAGE_CAPTION_ATTRIBUTE_NAME,
	XML_MOD_IMAGE_WIDTH_ATTRIBUTE_NAME,
	XML_MOD_IMAGE_HEIGHT_ATTRIBUTE_NAME,
	XML_MOD_IMAGE_SHA1_ATTRIBUTE_NAME
};

static constexpr const char * JSON_MOD_IMAGE_FILE_NAME_PROPERTY_NAME = "fileName";
static constexpr const char * JSON_MOD_IMAGE_FILE_SIZE_PROPERTY_NAME = "fileSize";
static constexpr const char * JSON_MOD_IMAGE_TYPE_PROPERTY_NAME = "type";
static constexpr const char * JSON_MOD_IMAGE_SUBFOLDER_PROPERTY_NAME = "subfolder";
static constexpr const char * JSON_MOD_IMAGE_CAPTION_PROPERTY_NAME = "caption";
static constexpr const char * JSON_MOD_IMAGE_WIDTH_PROPERTY_NAME = "width";
static constexpr const char * JSON_MOD_IMAGE_HEIGHT_PROPERTY_NAME = "height";
static constexpr const char * JSON_MOD_IMAGE_SHA1_PROPERTY_NAME = "sha1";
static const std::array<std::string_view, 8> JSON_MOD_IMAGE_PROPERTY_NAMES = {
	JSON_MOD_IMAGE_FILE_NAME_PROPERTY_NAME,
	JSON_MOD_IMAGE_FILE_SIZE_PROPERTY_NAME,
	JSON_MOD_IMAGE_TYPE_PROPERTY_NAME,
	JSON_MOD_IMAGE_SUBFOLDER_PROPERTY_NAME,
	JSON_MOD_IMAGE_CAPTION_PROPERTY_NAME,
	JSON_MOD_IMAGE_WIDTH_PROPERTY_NAME,
	JSON_MOD_IMAGE_HEIGHT_PROPERTY_NAME,
	JSON_MOD_IMAGE_SHA1_PROPERTY_NAME
};

ModImage::ModImage(const std::string & fileName, uint64_t fileSize, uint16_t width, uint16_t height, const std::string & sha1)
	: m_fileName(Utilities::trimString(fileName))
	, m_fileSize(fileSize)
	, m_width(width)
	, m_height(height)
	, m_sha1(Utilities::trimString(sha1))
	, m_parentMod(nullptr) { }

ModImage::ModImage(ModImage && i) noexcept
	: m_fileName(std::move(i.m_fileName))
	, m_fileSize(i.m_fileSize)
	, m_type(std::move(i.m_type))
	, m_subfolder(std::move(i.m_subfolder))
	, m_caption(std::move(i.m_caption))
	, m_width(i.m_width)
	, m_height(i.m_height)
	, m_sha1(std::move(i.m_sha1))
	, m_parentMod(nullptr) { }

ModImage::ModImage(const ModImage & i)
	: m_fileName(i.m_fileName)
	, m_fileSize(i.m_fileSize)
	, m_type(i.m_type)
	, m_subfolder(i.m_subfolder)
	, m_caption(i.m_caption)
	, m_width(i.m_width)
	, m_height(i.m_height)
	, m_sha1(i.m_sha1)
	, m_parentMod(nullptr) { }

ModImage & ModImage::operator = (ModImage && i) noexcept {
	if(this != &i) {
		m_fileName = std::move(i.m_fileName);
		m_fileSize = i.m_fileSize;
		m_type = std::move(i.m_type);
		m_subfolder = std::move(i.m_subfolder);
		m_caption = std::move(i.m_caption);
		m_width = i.m_width;
		m_height = i.m_height;
		m_sha1 = std::move(i.m_sha1);
	}

	return *this;
}

ModImage & ModImage::operator = (const ModImage & i) {
	m_fileName = i.m_fileName;
	m_fileSize = i.m_fileSize;
	m_type = i.m_type;
	m_subfolder = i.m_subfolder;
	m_caption = i.m_caption;
	m_width = i.m_width;
	m_height = i.m_height;
	m_sha1 = i.m_sha1;

	return *this;
}

ModImage::~ModImage() {
	m_parentMod = nullptr;
}

const std::string & ModImage::getFileName() const {
	return m_fileName;
}

uint64_t ModImage::getFileSize() const {
	return m_fileSize;
}

const std::string & ModImage::getType() const {
	return m_type;
}

const std::string & ModImage::getSubfolder() const {
	return m_subfolder;
}

const std::string & ModImage::getCaption() const {
	return m_caption;
}

uint16_t ModImage::getWidth() const {
	return m_width;
}

uint16_t ModImage::getHeight() const {
	return m_height;
}

const std::string & ModImage::getSHA1() const {
	return m_sha1;
}

const Mod * ModImage::getParentMod() const {
	return m_parentMod;
}

void ModImage::setFileName(const std::string & fileName) {
	m_fileName = Utilities::trimString(fileName);
}

void ModImage::setFileSize(uint64_t fileSize) {
	m_fileSize = fileSize;
}

void ModImage::setType(const std::string & type) {
	m_type = Utilities::trimString(type);
}

void ModImage::setSubfolder(const std::string & subfolder) {
	m_subfolder = Utilities::trimString(subfolder);
}

void ModImage::setCaption(const std::string & caption) {
	m_caption = Utilities::trimString(caption);
}

void ModImage::setWidth(uint16_t width) {
	m_width = width;
}

void ModImage::setHeight(uint16_t height) {
	m_height = height;
}

void ModImage::setSHA1(const std::string & sha1) {
	m_sha1 = Utilities::trimString(sha1);
}

void ModImage::setParentMod(const Mod * mod) {
	m_parentMod = mod;
}

rapidjson::Value ModImage::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value modImageValue(rapidjson::kObjectType);

	rapidjson::Value fileNameValue(m_fileName.c_str(), allocator);
	modImageValue.AddMember(rapidjson::StringRef(JSON_MOD_IMAGE_FILE_NAME_PROPERTY_NAME), fileNameValue, allocator);

	modImageValue.AddMember(rapidjson::StringRef(JSON_MOD_IMAGE_FILE_SIZE_PROPERTY_NAME), rapidjson::Value(m_fileSize), allocator);

	if(!m_type.empty()) {
		rapidjson::Value typeValue(m_type.c_str(), allocator);
		modImageValue.AddMember(rapidjson::StringRef(JSON_MOD_IMAGE_TYPE_PROPERTY_NAME), typeValue, allocator);
	}

	if(!m_subfolder.empty()) {
		rapidjson::Value subfolderValue(m_subfolder.c_str(), allocator);
		modImageValue.AddMember(rapidjson::StringRef(JSON_MOD_IMAGE_SUBFOLDER_PROPERTY_NAME), subfolderValue, allocator);
	}

	modImageValue.AddMember(rapidjson::StringRef(JSON_MOD_IMAGE_WIDTH_PROPERTY_NAME), rapidjson::Value(m_width), allocator);

	modImageValue.AddMember(rapidjson::StringRef(JSON_MOD_IMAGE_HEIGHT_PROPERTY_NAME), rapidjson::Value(m_height), allocator);

	if(!m_caption.empty()) {
		rapidjson::Value captionValue(m_caption.c_str(), allocator);
		modImageValue.AddMember(rapidjson::StringRef(JSON_MOD_IMAGE_CAPTION_PROPERTY_NAME), captionValue, allocator);
	}

	if(!m_sha1.empty()) {
		rapidjson::Value sha1Value(m_sha1.c_str(), allocator);
		modImageValue.AddMember(rapidjson::StringRef(JSON_MOD_IMAGE_SHA1_PROPERTY_NAME), sha1Value, allocator);
	}

	return modImageValue;
}

tinyxml2::XMLElement * ModImage::toXML(tinyxml2::XMLDocument * document) const {
	return toXML(document, XML_MOD_IMAGE_ELEMENT_NAME);
}

tinyxml2::XMLElement * ModImage::toXML(tinyxml2::XMLDocument * document, const std::string & name) const {
	if(document == nullptr) {
		return nullptr;
	}

	tinyxml2::XMLElement * modImageElement = document->NewElement(name.c_str());

	modImageElement->SetAttribute(XML_MOD_IMAGE_FILE_NAME_ATTRIBUTE_NAME.c_str(), m_fileName.c_str());

	if(m_fileSize != 0) {
		modImageElement->SetAttribute(XML_MOD_IMAGE_FILE_SIZE_ATTRIBUTE_NAME.c_str(), m_fileSize);
	}

	if(!m_type.empty()) {
		modImageElement->SetAttribute(XML_MOD_IMAGE_TYPE_ATTRIBUTE_NAME.c_str(), m_type.c_str());
	}

	if(!m_subfolder.empty()) {
		modImageElement->SetAttribute(XML_MOD_IMAGE_SUBFOLDER_ATTRIBUTE_NAME.c_str(), m_subfolder.c_str());
	}

	modImageElement->SetAttribute(XML_MOD_IMAGE_WIDTH_ATTRIBUTE_NAME.c_str(), m_width);
	modImageElement->SetAttribute(XML_MOD_IMAGE_HEIGHT_ATTRIBUTE_NAME.c_str(), m_height);

	if(!m_caption.empty()) {
		modImageElement->SetAttribute(XML_MOD_IMAGE_CAPTION_ATTRIBUTE_NAME.c_str(), m_caption.c_str());
	}

	if(!m_sha1.empty()) {
		modImageElement->SetAttribute(XML_MOD_IMAGE_SHA1_ATTRIBUTE_NAME.c_str(), m_sha1.c_str());
	}

	return modImageElement;
}

std::unique_ptr<ModImage> ModImage::parseFrom(const rapidjson::Value & modImageValue, bool skipFileInfoValidation) {
	if(!modImageValue.IsObject()) {
		spdlog::error("Invalid mod image type: '{}', expected 'object'.", Utilities::typeToString(modImageValue.GetType()));
		return nullptr;
	}

	// check for unhandled mod image properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = modImageValue.MemberBegin(); i != modImageValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_MOD_IMAGE_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::warn("Mod image has unexpected property '{}'.", i->name.GetString());
		}
	}

	// parse mod image file name
	if(!modImageValue.HasMember(JSON_MOD_IMAGE_FILE_NAME_PROPERTY_NAME)) {
		spdlog::error("Mod image is missing '{}' property.", JSON_MOD_IMAGE_FILE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modImageFileNameValue = modImageValue[JSON_MOD_IMAGE_FILE_NAME_PROPERTY_NAME];

	if(!modImageFileNameValue.IsString()) {
		spdlog::error("Mod image has an invalid '{}' property type: '{}', expected 'string'.", JSON_MOD_IMAGE_FILE_NAME_PROPERTY_NAME, Utilities::typeToString(modImageFileNameValue.GetType()));
		return nullptr;
	}

	std::string modImageFileName(Utilities::trimString(modImageFileNameValue.GetString()));

	if(modImageFileName.empty()) {
		spdlog::error("Mod image '{}' property cannot be empty.", JSON_MOD_IMAGE_FILE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse mod image file size
	uint64_t modImageFileSize = 0;

	if(modImageValue.HasMember(JSON_MOD_IMAGE_FILE_SIZE_PROPERTY_NAME)) {
		const rapidjson::Value & modImageFileSizeValue = modImageValue[JSON_MOD_IMAGE_FILE_SIZE_PROPERTY_NAME];

		if(!modImageFileSizeValue.IsUint64()) {
			spdlog::error("Mod image has an invalid '{}' property type: '{}', expected unsigned long 'number'.", JSON_MOD_IMAGE_FILE_SIZE_PROPERTY_NAME, Utilities::typeToString(modImageFileSizeValue.GetType()));
			return nullptr;
		}

		modImageFileSize = modImageFileSizeValue.GetUint64();

		if(modImageFileSize == 0) {
			spdlog::error("Mod image has an invalid '{}' property value, expected positive integer value.", JSON_MOD_IMAGE_FILE_SIZE_PROPERTY_NAME);
			return nullptr;
		}
	}
	else {
		if(!skipFileInfoValidation) {
			spdlog::error("Mod image is missing '{}' property.", JSON_MOD_IMAGE_FILE_SIZE_PROPERTY_NAME);
			return nullptr;
		}
	}

	// parse mod image width
	if(!modImageValue.HasMember(JSON_MOD_IMAGE_WIDTH_PROPERTY_NAME)) {
		spdlog::error("Mod image is missing '{}' property.", JSON_MOD_IMAGE_WIDTH_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modImageWidthValue = modImageValue[JSON_MOD_IMAGE_WIDTH_PROPERTY_NAME];

	if(!modImageWidthValue.IsUint()) {
		spdlog::error("Mod image has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_MOD_IMAGE_WIDTH_PROPERTY_NAME, Utilities::typeToString(modImageWidthValue.GetType()));
		return nullptr;
	}

	uint32_t modImageWidth = modImageWidthValue.GetUint();

	if(modImageWidth > std::numeric_limits<uint16_t>::max()) {
		spdlog::error("Mod image '{}' property value has an invalid value: '{}', expected unsigned integer 'number' between 1 and {} inclusively.", JSON_MOD_IMAGE_WIDTH_PROPERTY_NAME, modImageWidth, std::numeric_limits<uint8_t>::max());
		return nullptr;
	}

	// parse mod image height
	if(!modImageValue.HasMember(JSON_MOD_IMAGE_HEIGHT_PROPERTY_NAME)) {
		spdlog::error("Mod image is missing '{}' property.", JSON_MOD_IMAGE_HEIGHT_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modImageHeightValue = modImageValue[JSON_MOD_IMAGE_HEIGHT_PROPERTY_NAME];

	if(!modImageHeightValue.IsUint()) {
		spdlog::error("Mod image has an invalid '{}' property type: '{}', expected unsigned integer 'number'.", JSON_MOD_IMAGE_HEIGHT_PROPERTY_NAME, Utilities::typeToString(modImageHeightValue.GetType()));
		return nullptr;
	}

	uint32_t modImageHeight = modImageHeightValue.GetUint();

	if(modImageHeight > std::numeric_limits<uint16_t>::max()) {
		spdlog::error("Mod image '{}' property value has an invalid value: '{}', expected unsigned integer 'number' between 1 and {} inclusively.", JSON_MOD_IMAGE_HEIGHT_PROPERTY_NAME, modImageHeight, std::numeric_limits<uint8_t>::max());
		return nullptr;
	}

	// parse the mod image sha1 property
	std::string modImageSHA1;

	if(modImageValue.HasMember(JSON_MOD_IMAGE_SHA1_PROPERTY_NAME)) {
		const rapidjson::Value & modImageSHA1Value = modImageValue[JSON_MOD_IMAGE_SHA1_PROPERTY_NAME];

		if(!modImageSHA1Value.IsString()) {
			spdlog::error("Mod image '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_IMAGE_SHA1_PROPERTY_NAME, Utilities::typeToString(modImageSHA1Value.GetType()));
			return nullptr;
		}

		modImageSHA1 = Utilities::trimString(modImageSHA1Value.GetString());

		if(modImageSHA1.empty()) {
			spdlog::error("Mod image '{}' property cannot be empty.", JSON_MOD_IMAGE_SHA1_PROPERTY_NAME);
			return nullptr;
		}
	}
	else {
		if(!skipFileInfoValidation) {
			spdlog::error("Mod image is missing '{}' property.", JSON_MOD_IMAGE_SHA1_PROPERTY_NAME);
			return nullptr;
		}
	}

	// initialize the mod image
	std::unique_ptr<ModImage> newModImage = std::make_unique<ModImage>(modImageFileName, modImageFileSize, static_cast<uint16_t>(modImageWidth), static_cast<uint16_t>(modImageHeight), modImageSHA1);

	// parse the mod image type property
	if(modImageValue.HasMember(JSON_MOD_IMAGE_TYPE_PROPERTY_NAME)) {
		const rapidjson::Value & modImageTypeValue = modImageValue[JSON_MOD_IMAGE_TYPE_PROPERTY_NAME];

		if(!modImageTypeValue.IsString()) {
			spdlog::error("Mod image '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_IMAGE_TYPE_PROPERTY_NAME, Utilities::typeToString(modImageTypeValue.GetType()));
			return nullptr;
		}

		newModImage->setType(modImageTypeValue.GetString());
	}

	// parse the mod image subfolder property
	if(modImageValue.HasMember(JSON_MOD_IMAGE_SUBFOLDER_PROPERTY_NAME)) {
		const rapidjson::Value & modImageSubfolderValue = modImageValue[JSON_MOD_IMAGE_SUBFOLDER_PROPERTY_NAME];

		if(!modImageSubfolderValue.IsString()) {
			spdlog::error("Mod image '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_IMAGE_SUBFOLDER_PROPERTY_NAME, Utilities::typeToString(modImageSubfolderValue.GetType()));
			return nullptr;
		}

		newModImage->setSubfolder(modImageSubfolderValue.GetString());
	}

	// parse the mod image caption property
	if(modImageValue.HasMember(JSON_MOD_IMAGE_CAPTION_PROPERTY_NAME)) {
		const rapidjson::Value & modImageCaptionValue = modImageValue[JSON_MOD_IMAGE_CAPTION_PROPERTY_NAME];

		if(!modImageCaptionValue.IsString()) {
			spdlog::error("Mod image '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_IMAGE_CAPTION_PROPERTY_NAME, Utilities::typeToString(modImageCaptionValue.GetType()));
			return nullptr;
		}

		newModImage->setCaption(modImageCaptionValue.GetString());
	}

	return newModImage;
}

std::unique_ptr<ModImage> ModImage::parseFrom(const tinyxml2::XMLElement * modImageElement, bool skipFileInfoValidation) {
	return parseFrom(modImageElement, XML_MOD_IMAGE_ELEMENT_NAME, skipFileInfoValidation);
}

std::unique_ptr<ModImage> ModImage::parseFrom(const tinyxml2::XMLElement * modImageElement, const std::string & name, bool skipFileInfoValidation) {
	if(modImageElement == nullptr) {
		return nullptr;
	}

	// verify element name
	if(modImageElement->Name() != name) {
		spdlog::error("Invalid mod image element name: '{}', expected '{}'.", modImageElement->Name(), name);
		return nullptr;
	}

	// check for unhandled mod image element attributes
	bool attributeHandled = false;
	const tinyxml2::XMLAttribute * modImageAttribute = modImageElement->FirstAttribute();

	while(true) {
		if(modImageAttribute == nullptr) {
			break;
		}

		attributeHandled = false;

		for(const std::string_view & attributeName : XML_MOD_IMAGE_ATTRIBUTE_NAMES) {
			if(modImageAttribute->Name() == attributeName) {
				attributeHandled = true;
				break;
			}
		}

		if(!attributeHandled) {
			spdlog::warn("Element '{}' has unexpected attribute '{}'.", name, modImageAttribute->Name());
		}

		modImageAttribute = modImageAttribute->Next();
	}

	// check for unexpected mod image element child elements
	if(modImageElement->FirstChildElement() != nullptr) {
		spdlog::warn("Element '{}' has an unexpected child element.", name);
	}

	// read the mod image attributes
	const char * modImageFileName = modImageElement->Attribute(XML_MOD_IMAGE_FILE_NAME_ATTRIBUTE_NAME.c_str());

	const char * modImageFileSizeData = modImageElement->Attribute(XML_MOD_IMAGE_FILE_SIZE_ATTRIBUTE_NAME.c_str());

	if(modImageFileName == nullptr || Utilities::stringLength(modImageFileName) == 0) {
		spdlog::error("Attribute '{}' is missing from '{}' element.", XML_MOD_IMAGE_FILE_NAME_ATTRIBUTE_NAME, name);
		return nullptr;
	}

	const char * modImageWidthData = modImageElement->Attribute(XML_MOD_IMAGE_WIDTH_ATTRIBUTE_NAME.c_str());

	if(modImageWidthData == nullptr || Utilities::stringLength(modImageWidthData) == 0) {
		spdlog::error("Attribute '{}' is missing from '{}' element.", XML_MOD_IMAGE_WIDTH_ATTRIBUTE_NAME, name);
		return nullptr;
	}

	const char * modImageHeightData = modImageElement->Attribute(XML_MOD_IMAGE_HEIGHT_ATTRIBUTE_NAME.c_str());

	if(modImageHeightData == nullptr || Utilities::stringLength(modImageHeightData) == 0) {
		spdlog::error("Attribute '{}' is missing from '{}' element.", XML_MOD_IMAGE_HEIGHT_ATTRIBUTE_NAME, name);
		return nullptr;
	}

	bool error = false;

	uint64_t modImageFileSize = 0;

	if(Utilities::stringLength(modImageFileSizeData) != 0) {
		modImageFileSize = Utilities::parseUnsignedLong(modImageFileSizeData, &error);

		if(error || modImageFileSize == 0) {
			spdlog::error("Attribute '{}' in element '{}' has an invalid value: '{}', expected positive integer number.", XML_MOD_IMAGE_FILE_SIZE_ATTRIBUTE_NAME, name, modImageFileSizeData);
			return nullptr;
		}
	}
	else {
		if(!skipFileInfoValidation) {
			spdlog::error("Attribute '{}' is missing from '{}' element.", XML_MOD_IMAGE_FILE_SIZE_ATTRIBUTE_NAME, name);
		}
	}

	uint32_t modImageWidth = Utilities::parseUnsignedInteger(modImageWidthData, &error);

	if(error || modImageWidth < 1 || modImageWidth > std::numeric_limits<uint16_t>::max()) {
		spdlog::error("Attribute '{}' in element '{}' has an invalid value: '{}', expected integer number between 1 and {} inclusively.", XML_MOD_IMAGE_WIDTH_ATTRIBUTE_NAME, name, modImageWidthData, std::numeric_limits<uint16_t>::max());
		return nullptr;
	}

	uint32_t modImageHeight = Utilities::parseUnsignedInteger(modImageHeightData, &error);

	if(error || modImageHeight < 1 || modImageHeight > std::numeric_limits<uint16_t>::max()) {
		spdlog::error("Attribute '{}' in element '{}' has an invalid value: '{}', expected integer number between 1 and {} inclusively.", XML_MOD_IMAGE_HEIGHT_ATTRIBUTE_NAME, name, modImageHeightData, std::numeric_limits<uint16_t>::max());
		return nullptr;
	}

	const char * modImageSHA1 = modImageElement->Attribute(XML_MOD_IMAGE_SHA1_ATTRIBUTE_NAME.c_str());

	if(!skipFileInfoValidation && Utilities::stringLength(modImageSHA1) == 0) {
		spdlog::error("Attribute '{}' is missing from '{}' element.", XML_MOD_IMAGE_SHA1_ATTRIBUTE_NAME, XML_MOD_IMAGE_ELEMENT_NAME);
		return nullptr;
	}

	const char * modImageType = modImageElement->Attribute(XML_MOD_IMAGE_TYPE_ATTRIBUTE_NAME.c_str());
	const char * modImageSubfolder = modImageElement->Attribute(XML_MOD_IMAGE_SUBFOLDER_ATTRIBUTE_NAME.c_str());
	const char * modImageCaption = modImageElement->Attribute(XML_MOD_IMAGE_CAPTION_ATTRIBUTE_NAME.c_str());

	// initialize the mod image
	std::unique_ptr<ModImage> newModImage = std::make_unique<ModImage>(modImageFileName, modImageFileSize, static_cast<uint16_t>(modImageWidth), static_cast<uint16_t>(modImageHeight), modImageSHA1 == nullptr ? "" : modImageSHA1);

	if(modImageType != nullptr) {
		newModImage->setType(modImageType);
	}

	if(modImageSubfolder != nullptr) {
		newModImage->setSubfolder(modImageSubfolder);
	}

	if(modImageCaption != nullptr) {
		newModImage->setCaption(modImageCaption);
	}

	return newModImage;
}

bool ModImage::isValid(bool skipFileInfoValidation) const {
	if(!skipFileInfoValidation) {
		if(m_fileSize == 0 ||
		   m_sha1.empty()) {
			return false;
		}
	}

	return !m_fileName.empty() &&
		   m_width != 0 &&
		   m_height != 0;
}

bool ModImage::isValid(const ModImage * i, bool skipFileInfoValidation) {
	return i != nullptr && i->isValid(skipFileInfoValidation);
}

bool ModImage::operator == (const ModImage & i) const {
	return m_fileSize == i.m_fileSize &&
		   m_width == i.m_width &&
		   m_height == i.m_height &&
		   Utilities::areStringsEqualIgnoreCase(m_fileName, i.m_fileName) &&
		   Utilities::areStringsEqualIgnoreCase(m_type, i.m_type) &&
		   Utilities::areStringsEqualIgnoreCase(m_subfolder, i.m_subfolder) &&
		   Utilities::areStringsEqualIgnoreCase(m_caption, i.m_caption) &&
		   m_sha1 == i.m_sha1;
}

bool ModImage::operator != (const ModImage & i) const {
	return !operator == (i);
}
