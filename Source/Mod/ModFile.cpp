#include "ModFile.h"

#include "ModGameVersion.h"
#include "Mod.h"
#include "ModVersion.h"
#include "ModVersionType.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <array>
#include <string_view>

static const std::string XML_MOD_FILE_ELEMENT_NAME("file");
static const std::string XML_MOD_FILE_FILE_NAME_ATTRIBUTE_NAME("name");
static const std::string XML_MOD_FILE_TYPE_ATTRIBUTE_NAME("type");
static const std::string XML_MOD_FILE_SHA1_ATTRIBUTE_NAME("sha1");
static const std::string XML_MOD_FILE_SHARED_ATTRIBUTE_NAME("shared");
static const std::array<std::string, 4> XML_MOD_FILE_ATTRIBUTE_NAMES = {
	XML_MOD_FILE_FILE_NAME_ATTRIBUTE_NAME,
	XML_MOD_FILE_TYPE_ATTRIBUTE_NAME,
	XML_MOD_FILE_SHA1_ATTRIBUTE_NAME,
	XML_MOD_FILE_SHARED_ATTRIBUTE_NAME
};

static constexpr const char * JSON_MOD_FILE_FILE_NAME_PROPERTY_NAME = "fileName";
static constexpr const char * JSON_MOD_FILE_TYPE_PROPERTY_NAME = "type";
static constexpr const char * JSON_MOD_FILE_SHA1_PROPERTY_NAME = "sha1";
static constexpr const char * JSON_MOD_FILE_SHARED_PROPERTY_NAME = "shared";
static const std::array<std::string_view, 4> JSON_MOD_FILE_PROPERTY_NAMES = {
	JSON_MOD_FILE_FILE_NAME_PROPERTY_NAME,
	JSON_MOD_FILE_TYPE_PROPERTY_NAME,
	JSON_MOD_FILE_SHA1_PROPERTY_NAME,
	JSON_MOD_FILE_SHARED_PROPERTY_NAME
};

ModFile::ModFile(const std::string & fileName, const std::string & type, const std::string & sha1)
	: m_fileName(Utilities::trimString(fileName))
	, m_type(Utilities::trimString(type))
	, m_sha1(Utilities::trimString(sha1))
	, m_parentModGameVersion(nullptr) { }

ModFile::ModFile(ModFile && f) noexcept
	: m_fileName(std::move(f.m_fileName))
	, m_type(std::move(f.m_type))
	, m_sha1(std::move(f.m_sha1))
	, m_shared(f.m_shared)
	, m_parentModGameVersion(nullptr) { }

ModFile::ModFile(const ModFile & f)
	: m_fileName(f.m_fileName)
	, m_type(f.m_type)
	, m_sha1(f.m_sha1)
	, m_shared(f.m_shared)
	, m_parentModGameVersion(nullptr) { }

ModFile & ModFile::operator = (ModFile && f) noexcept {
	if(this != &f) {
		m_fileName = std::move(f.m_fileName);
		m_type = std::move(f.m_type);
		m_sha1 = std::move(f.m_sha1);
		m_shared = f.m_shared;
	}

	return *this;
}

ModFile & ModFile::operator = (const ModFile & f) {
	m_fileName = f.m_fileName;
	m_type = f.m_type;
	m_sha1 = f.m_sha1;
	m_shared = f.m_shared;

	return *this;
}

ModFile::~ModFile() {
	m_parentModGameVersion = nullptr;
}

const std::string & ModFile::getFileName() const {
	return m_fileName;
}

std::string_view ModFile::getFileExtension() const {
	return Utilities::getFileExtension(m_fileName);
}

const std::string & ModFile::getType() const {
	return m_type;
}

const std::string & ModFile::getSHA1() const {
	return m_sha1;
}

bool ModFile::isShared() const {
	return m_shared.has_value() ? m_shared.value() : false;
}

std::optional<bool> ModFile::getShared() const {
	return m_shared;
}

const Mod * ModFile::getParentMod() const {
	const ModVersion * parentModVersion = getParentModVersion();

	if(parentModVersion == nullptr) {
		return nullptr;
	}

	return parentModVersion->getParentMod();
}

const ModVersion * ModFile::getParentModVersion() const {
	const ModVersionType * parentModVersionType = getParentModVersionType();

	if(parentModVersionType == nullptr) {
		return nullptr;
	}

	return parentModVersionType->getParentModVersion();
}

const ModVersionType * ModFile::getParentModVersionType() const {
	if(m_parentModGameVersion == nullptr) {
		return nullptr;
	}

	return m_parentModGameVersion->getParentModVersionType();
}

const ModGameVersion * ModFile::getParentModGameVersion() const {
	return m_parentModGameVersion;
}

void ModFile::setFileName(const std::string & fileName) {
	m_fileName = Utilities::trimString(fileName);
}

void ModFile::setType(const std::string & type) {
	m_type = Utilities::trimString(type);
}

void ModFile::setSHA1(const std::string & sha1) {
	m_sha1 = Utilities::trimString(sha1);
}

void ModFile::setShared(bool shared) {
	m_shared = shared;
}

void ModFile::clearShared() {
	m_shared.reset();
}

void ModFile::setParentModGameVersion(const ModGameVersion * modGameVersion) {
	m_parentModGameVersion = modGameVersion;
}

rapidjson::Value ModFile::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value modFileValue(rapidjson::kObjectType);

	rapidjson::Value fileNameValue(m_fileName.c_str(), allocator);
	modFileValue.AddMember(rapidjson::StringRef(JSON_MOD_FILE_FILE_NAME_PROPERTY_NAME), fileNameValue, allocator);

	rapidjson::Value fileTypeValue(m_type.c_str(), allocator);
	modFileValue.AddMember(rapidjson::StringRef(JSON_MOD_FILE_TYPE_PROPERTY_NAME), fileTypeValue, allocator);

	if(m_shared.has_value()) {
		modFileValue.AddMember(rapidjson::StringRef(JSON_MOD_FILE_SHARED_PROPERTY_NAME), rapidjson::Value(m_shared.value()), allocator);
	}

	if(!m_sha1.empty()) {
		rapidjson::Value sha1Value(m_sha1.c_str(), allocator);
		modFileValue.AddMember(rapidjson::StringRef(JSON_MOD_FILE_SHA1_PROPERTY_NAME), sha1Value, allocator);
	}

	return modFileValue;
}

tinyxml2::XMLElement * ModFile::toXML(tinyxml2::XMLDocument * document) const {
	if(document == nullptr) {
		return nullptr;
	}

	tinyxml2::XMLElement * modFileElement = document->NewElement(XML_MOD_FILE_ELEMENT_NAME.c_str());

	modFileElement->SetAttribute(XML_MOD_FILE_FILE_NAME_ATTRIBUTE_NAME.c_str(), m_fileName.c_str());
	modFileElement->SetAttribute(XML_MOD_FILE_TYPE_ATTRIBUTE_NAME.c_str(), m_type.c_str());

	if(m_shared.has_value()) {
		modFileElement->SetAttribute(XML_MOD_FILE_SHARED_ATTRIBUTE_NAME.c_str(), m_shared.value());
	}

	if(!m_sha1.empty()) {
		modFileElement->SetAttribute(XML_MOD_FILE_SHA1_ATTRIBUTE_NAME.c_str(), m_sha1.c_str());
	}

	return modFileElement;
}

std::unique_ptr<ModFile> ModFile::parseFrom(const rapidjson::Value & modFileValue) {
	if(!modFileValue.IsObject()) {
		spdlog::error("Invalid mod file type: '{}', expected 'object'.", Utilities::typeToString(modFileValue.GetType()));
		return nullptr;
	}

	// check for unhandled mod file properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = modFileValue.MemberBegin(); i != modFileValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_MOD_FILE_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::error("Mod file has unexpected property '{}'.", i->name.GetString());
			return nullptr;
		}
	}

	// parse mod file name
	if(!modFileValue.HasMember(JSON_MOD_FILE_FILE_NAME_PROPERTY_NAME)) {
		spdlog::error("Mod file is missing '{}' property'.", JSON_MOD_FILE_FILE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modFileNameValue = modFileValue[JSON_MOD_FILE_FILE_NAME_PROPERTY_NAME];

	if(!modFileNameValue.IsString()) {
		spdlog::error("Mod file has an invalid '{}' property type: '{}', expected 'string'.", JSON_MOD_FILE_FILE_NAME_PROPERTY_NAME, Utilities::typeToString(modFileNameValue.GetType()));
		return nullptr;
	}

	std::string modFileName(Utilities::trimString(modFileNameValue.GetString()));

	if(modFileName.empty()) {
		spdlog::error("Mod file '{}' property cannot be empty.", JSON_MOD_FILE_FILE_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse mod file type
	if(!modFileValue.HasMember(JSON_MOD_FILE_TYPE_PROPERTY_NAME)) {
		spdlog::error("Mod file is missing '{}' property'.", JSON_MOD_FILE_TYPE_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modFileTypeValue = modFileValue[JSON_MOD_FILE_TYPE_PROPERTY_NAME];

	if(!modFileTypeValue.IsString()) {
		spdlog::error("Mod file has an invalid '{}' property type: '{}', expected 'string'.", JSON_MOD_FILE_TYPE_PROPERTY_NAME, Utilities::typeToString(modFileTypeValue.GetType()));
		return nullptr;
	}

	std::string modFileType(Utilities::trimString(modFileTypeValue.GetString()));

	if(modFileType.empty()) {
		spdlog::error("Mod file '{}' property cannot be empty.", JSON_MOD_FILE_TYPE_PROPERTY_NAME);
		return nullptr;
	}

	// parse the mod file sha1 property
	if(!modFileValue.HasMember(JSON_MOD_FILE_SHA1_PROPERTY_NAME)) {
		spdlog::error("Mod file is missing '{}' property'.", JSON_MOD_FILE_SHA1_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modFileSHA1Value = modFileValue[JSON_MOD_FILE_SHA1_PROPERTY_NAME];

	if(!modFileSHA1Value.IsString()) {
		spdlog::error("Mod file '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_FILE_SHA1_PROPERTY_NAME, Utilities::typeToString(modFileSHA1Value.GetType()));
		return nullptr;
	}

	std::string modFileSHA1(Utilities::trimString(modFileSHA1Value.GetString()));

	if(modFileSHA1.empty()) {
		spdlog::error("Mod file '{}' property cannot be empty.", JSON_MOD_FILE_SHA1_PROPERTY_NAME);
		return nullptr;
	}

	// initialize the mod file
	std::unique_ptr<ModFile> newModFile = std::make_unique<ModFile>(modFileName, modFileType, modFileSHA1);

	// parse the mod file shared property
	if(modFileValue.HasMember(JSON_MOD_FILE_SHARED_PROPERTY_NAME)) {
		const rapidjson::Value & modFileSharedValue = modFileValue[JSON_MOD_FILE_SHARED_PROPERTY_NAME];

		if(!modFileSharedValue.IsBool()) {
			spdlog::error("Mod file '{}' property has invalid type: '{}', expected 'boolean'.", JSON_MOD_FILE_SHARED_PROPERTY_NAME, Utilities::typeToString(modFileSharedValue.GetType()));
			return nullptr;
		}

		newModFile->setShared(modFileSharedValue.GetBool());
	}

	return newModFile;
}

std::unique_ptr<ModFile> ModFile::parseFrom(const tinyxml2::XMLElement * modFileElement) {
	if(modFileElement == nullptr) {
		return nullptr;
	}

	// verify element name
	if(modFileElement->Name() != XML_MOD_FILE_ELEMENT_NAME) {
		spdlog::error("Invalid mod file element name: '{}', expected '{}'.", modFileElement->Name(), XML_MOD_FILE_ELEMENT_NAME);
		return nullptr;
	}

	// check for unhandled mod file element attributes
	bool attributeHandled = false;
	const tinyxml2::XMLAttribute * modFileAttribute = modFileElement->FirstAttribute();

	while(true) {
		if(modFileAttribute == nullptr) {
			break;
		}

		attributeHandled = false;

		for(const std::string & attributeName : XML_MOD_FILE_ATTRIBUTE_NAMES) {
			if(modFileAttribute->Name() == attributeName) {
				attributeHandled = true;
				break;
			}
		}

		if(!attributeHandled) {
			spdlog::error("Element '{}' has unexpected attribute '{}'.", XML_MOD_FILE_ELEMENT_NAME, modFileAttribute->Name());
			return nullptr;
		}

		modFileAttribute = modFileAttribute->Next();
	}

	// check for unexpected mod file element child elements
	if(modFileElement->FirstChildElement() != nullptr) {
		spdlog::error("Element '{}' has an unexpected child element.", XML_MOD_FILE_ELEMENT_NAME);
		return nullptr;
	}

	// read the mod file attributes
	const char * modFileSharedData = modFileElement->Attribute(XML_MOD_FILE_SHARED_ATTRIBUTE_NAME.c_str());
	const char * modFileName = modFileElement->Attribute(XML_MOD_FILE_FILE_NAME_ATTRIBUTE_NAME.c_str());

	if(modFileName == nullptr || Utilities::stringLength(modFileName) == 0) {
		spdlog::error("Attribute '{}' is missing from '{}' element.", XML_MOD_FILE_FILE_NAME_ATTRIBUTE_NAME, XML_MOD_FILE_ELEMENT_NAME);
		return nullptr;
	}

	const char * modFileType = modFileElement->Attribute(XML_MOD_FILE_TYPE_ATTRIBUTE_NAME.c_str());

	if(modFileType == nullptr || Utilities::stringLength(modFileType) == 0) {
		spdlog::error("Attribute '{}' is missing from '{}' element.", XML_MOD_FILE_TYPE_ATTRIBUTE_NAME, XML_MOD_FILE_ELEMENT_NAME);
		return nullptr;
	}

	const char * modFileSHA1 = modFileElement->Attribute(XML_MOD_FILE_SHA1_ATTRIBUTE_NAME.c_str());

	if(modFileSHA1 == nullptr || Utilities::stringLength(modFileSHA1) == 0) {
		spdlog::error("Attribute '{}' is missing from '{}' element.", XML_MOD_FILE_SHA1_ATTRIBUTE_NAME, XML_MOD_FILE_ELEMENT_NAME);
		return nullptr;
	}

	// initialize the mod file
	std::unique_ptr<ModFile> newModFile = std::make_unique<ModFile>(modFileName, modFileType, modFileSHA1 == nullptr ? "" : modFileSHA1);

	bool error = false;

	if(modFileSharedData != nullptr) {
		bool shared = Utilities::parseBoolean(modFileSharedData, &error);

		if(error) {
			spdlog::error("Attribute '{}' in element '{}' has an invalid value: '{}', expected boolean.", XML_MOD_FILE_SHARED_ATTRIBUTE_NAME, XML_MOD_FILE_ELEMENT_NAME, modFileSharedData);
			return nullptr;
		}

		newModFile->setShared(shared);
	}


	return newModFile;
}

bool ModFile::isValid() const {
	return !m_fileName.empty() &&
		   !m_type.empty() &&
		   !m_sha1.empty();
}

bool ModFile::isValid(const ModFile * f) {
	return f != nullptr && f->isValid();
}

bool ModFile::operator == (const ModFile & f) const {
	return Utilities::areStringsEqualIgnoreCase(m_fileName, f.m_fileName) &&
		   Utilities::areStringsEqualIgnoreCase(m_type, f.m_type) &&
		   m_sha1 == f.m_sha1 &&
		   m_shared == f.m_shared;
}

bool ModFile::operator != (const ModFile & f) const {
	return !operator == (f);
}
