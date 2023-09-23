#include "ModDependency.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <array>
#include <sstream>
#include <string_view>
#include <vector>

static const std::string XML_MOD_DEPENDENCY_ELEMENT_NAME("dependency");
static const std::string XML_MOD_DEPENDENCY_MOD_ID_ATTRIBUTE_NAME("id");
static const std::string XML_MOD_DEPENDENCY_MOD_VERSION_ATTRIBUTE_NAME("version");
static const std::string XML_MOD_DEPENDENCY_MOD_VERSION_TYPE_ATTRIBUTE_NAME("version_type");
static const std::array<std::string_view, 3> XML_MOD_DEPENDENCY_ATTRIBUTE_NAMES = {
	XML_MOD_DEPENDENCY_MOD_ID_ATTRIBUTE_NAME,
	XML_MOD_DEPENDENCY_MOD_VERSION_ATTRIBUTE_NAME,
	XML_MOD_DEPENDENCY_MOD_VERSION_TYPE_ATTRIBUTE_NAME
};

static constexpr const char * JSON_ID_PROPERTY_NAME = "id";
static constexpr const char * JSON_VERSION_PROPERTY_NAME = "version";
static constexpr const char * JSON_VERSION_TYPE_PROPERTY_NAME = "versionType";
static const std::array<std::string_view, 3> JSON_PROPERTY_NAMES = {
	JSON_ID_PROPERTY_NAME,
	JSON_VERSION_PROPERTY_NAME,
	JSON_VERSION_TYPE_PROPERTY_NAME
};

ModDependency::ModDependency(const std::string & id, const std::string & version, const std::string & versionType)
	: m_id(Utilities::trimString(id))
	, m_version(Utilities::trimString(version))
	, m_versionType(Utilities::trimString(versionType)) { }

ModDependency::ModDependency(ModDependency && dependency) noexcept
	: m_id(std::move(dependency.m_id))
	, m_version(std::move(dependency.m_version))
	, m_versionType(std::move(dependency.m_versionType)) { }

ModDependency::ModDependency(const ModDependency & dependency)
	: m_id(dependency.m_id)
	, m_version(dependency.m_version)
	, m_versionType(dependency.m_versionType) { }

ModDependency & ModDependency::operator = (ModDependency && dependency) noexcept {
	if(this != &dependency) {
		m_id = std::move(dependency.m_id);
		m_version = std::move(dependency.m_version);
		m_versionType = std::move(dependency.m_versionType);
	}

	return *this;
}

ModDependency & ModDependency::operator = (const ModDependency & dependency) {
	m_id = dependency.m_id;
	m_version = dependency.m_version;
	m_versionType = dependency.m_versionType;

	return *this;
}

ModDependency::~ModDependency() = default;

const std::string & ModDependency::getID() const {
	return m_id;
}

bool ModDependency::hasVersion() const {
	return !m_version.empty();
}

const std::string & ModDependency::getVersion() const {
	return m_version;
}

bool ModDependency::hasVersionType() const {
	return !m_versionType.empty();
}

const std::string & ModDependency::getVersionType() const {
	return m_versionType;
}

bool ModDependency::setID(const std::string & id) {
	std::string formattedID(Utilities::trimString(id));

	if(formattedID.empty()) {
		return false;
	}

	m_id = formattedID;

	return true;
}

void ModDependency::setVersion(const std::string & version) {
	m_version = Utilities::trimString(version);
}

void ModDependency::clearVersion() {
	m_version = Utilities::emptyString;
}

void ModDependency::setVersionType(const std::string & versionType) {
	m_versionType = Utilities::trimString(versionType);
}

void ModDependency::clearVersionType() {
	m_version = Utilities::emptyString;
}

rapidjson::Value ModDependency::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value modDependencyValue(rapidjson::kObjectType);

	rapidjson::Value idValue(m_id.c_str(), allocator);
	modDependencyValue.AddMember(rapidjson::StringRef(JSON_ID_PROPERTY_NAME), idValue, allocator);

	if(!m_version.empty()) {
		rapidjson::Value versionValue(m_version.c_str(), allocator);
		modDependencyValue.AddMember(rapidjson::StringRef(JSON_VERSION_PROPERTY_NAME), versionValue, allocator);
	}

	if(!m_versionType.empty()) {
		rapidjson::Value versionTypeValue(m_versionType.c_str(), allocator);
		modDependencyValue.AddMember(rapidjson::StringRef(JSON_VERSION_TYPE_PROPERTY_NAME), versionTypeValue, allocator);
	}

	return modDependencyValue;
}

tinyxml2::XMLElement * ModDependency::toXML(tinyxml2::XMLDocument * document) const {
	if(document == nullptr) {
		return nullptr;
	}

	tinyxml2::XMLElement * modDependencyElement = document->NewElement(XML_MOD_DEPENDENCY_ELEMENT_NAME.c_str());

	modDependencyElement->SetAttribute(XML_MOD_DEPENDENCY_MOD_ID_ATTRIBUTE_NAME.c_str(), m_id.c_str());

	if(!m_version.empty()) {
		modDependencyElement->SetAttribute(XML_MOD_DEPENDENCY_MOD_VERSION_ATTRIBUTE_NAME.c_str(), m_version.c_str());
	}

	if(!m_versionType.empty()) {
		modDependencyElement->SetAttribute(XML_MOD_DEPENDENCY_MOD_VERSION_TYPE_ATTRIBUTE_NAME.c_str(), m_versionType.c_str());
	}

	return modDependencyElement;
}

std::unique_ptr<ModDependency> ModDependency::parseFrom(const rapidjson::Value & modDependencyValue) {
	if(!modDependencyValue.IsObject()) {
		spdlog::error("Invalid mod dependency type: '{}', expected 'object'.", Utilities::typeToString(modDependencyValue.GetType()));
		return nullptr;
	}

	// check for unhandled mod dependency properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = modDependencyValue.MemberBegin(); i != modDependencyValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::warn("Mod dependency has unexpected property '{}'.", i->name.GetString());
		}
	}

	// parse mod dependency id
	if(!modDependencyValue.HasMember(JSON_ID_PROPERTY_NAME)) {
		spdlog::error("Mod dependency is missing '{}' property.", JSON_ID_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modDependencyIDValue = modDependencyValue[JSON_ID_PROPERTY_NAME];

	if(!modDependencyIDValue.IsString()) {
		spdlog::error("Mod dependency has an invalid '{}' property type: '{}', expected 'string'.", JSON_ID_PROPERTY_NAME, Utilities::typeToString(modDependencyIDValue.GetType()));
		return nullptr;
	}

	std::string id(Utilities::trimString(modDependencyIDValue.GetString()));

	if(id.empty()) {
		spdlog::error("Mod dependency '{}' property cannot be empty.", JSON_ID_PROPERTY_NAME);
		return nullptr;
	}

	// parse mod dependency version
	std::string version;

	if(modDependencyValue.HasMember(JSON_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & modDependencyVersionValue = modDependencyValue[JSON_VERSION_PROPERTY_NAME];

		if(!modDependencyVersionValue.IsString()) {
			spdlog::error("Mod dependency has an invalid '{}' property type: '{}', expected 'string'.", JSON_VERSION_PROPERTY_NAME, Utilities::typeToString(modDependencyVersionValue.GetType()));
			return nullptr;
		}

		version = modDependencyVersionValue.GetString();
	}

	// parse mod dependency version type
	std::string versionType;

	if(modDependencyValue.HasMember(JSON_VERSION_TYPE_PROPERTY_NAME)) {
		const rapidjson::Value & modDependencyVersionTypeValue = modDependencyValue[JSON_VERSION_TYPE_PROPERTY_NAME];

		if(!modDependencyVersionTypeValue.IsString()) {
			spdlog::error("Mod dependency has an invalid '{}' property type: '{}', expected 'string'.", JSON_VERSION_TYPE_PROPERTY_NAME, Utilities::typeToString(modDependencyVersionTypeValue.GetType()));
			return nullptr;
		}

		versionType = modDependencyVersionTypeValue.GetString();
	}

	return std::unique_ptr<ModDependency>(std::make_unique<ModDependency>(id, version, versionType));
}

std::unique_ptr<ModDependency> ModDependency::parseFrom(const tinyxml2::XMLElement * modDependencyElement) {
	if(modDependencyElement == nullptr) {
		return nullptr;
	}

	// verify element name
	if(!Utilities::areStringsEqual(modDependencyElement->Name(), XML_MOD_DEPENDENCY_ELEMENT_NAME)) {
		spdlog::error("Invalid mod dependency element name: '{}', expected '{}'.", modDependencyElement->Name(), XML_MOD_DEPENDENCY_ELEMENT_NAME);
		return nullptr;
	}

	// check for unhandled mod dependency element attributes
	bool attributeHandled = false;
	const tinyxml2::XMLAttribute * modDependencyAttribute = modDependencyElement->FirstAttribute();

	while(true) {
		if(modDependencyAttribute == nullptr) {
			break;
		}

		attributeHandled = false;

		for(const std::string_view & attributeName : XML_MOD_DEPENDENCY_ATTRIBUTE_NAMES) {
			if(modDependencyAttribute->Name() == attributeName) {
				attributeHandled = true;
				break;
			}
		}

		if(!attributeHandled) {
			spdlog::warn("Element '{}' has unexpected attribute '{}'.", XML_MOD_DEPENDENCY_ELEMENT_NAME, modDependencyAttribute->Name());
		}

		modDependencyAttribute = modDependencyAttribute->Next();
	}

	// check for unexpected mod dependency element child elements
	if(modDependencyElement->FirstChildElement() != nullptr) {
		spdlog::warn("Element '{}' has an unexpected child element.", XML_MOD_DEPENDENCY_ELEMENT_NAME);
	}

	// read the mod dependency attributes
	const char * modDependencyID = modDependencyElement->Attribute(XML_MOD_DEPENDENCY_MOD_ID_ATTRIBUTE_NAME.c_str());

	if(Utilities::stringLength(modDependencyID) == 0) {
		spdlog::error("Attribute '{}' is missing from '{}' element.", XML_MOD_DEPENDENCY_MOD_ID_ATTRIBUTE_NAME, XML_MOD_DEPENDENCY_ELEMENT_NAME);
		return nullptr;
	}

	const char * modDependencyVersion = modDependencyElement->Attribute(XML_MOD_DEPENDENCY_MOD_VERSION_ATTRIBUTE_NAME.c_str());
	const char * modDependencyVersionType = modDependencyElement->Attribute(XML_MOD_DEPENDENCY_MOD_VERSION_TYPE_ATTRIBUTE_NAME.c_str());

	return std::make_unique<ModDependency>(modDependencyID, modDependencyVersion != nullptr ? modDependencyVersion : "", modDependencyVersionType != nullptr ? modDependencyVersionType : "");
}

std::string ModDependency::toString() const {
	std::stringstream modDependencyStream;

	modDependencyStream << m_id;

	if(!m_version.empty()) {
		modDependencyStream << " " << m_version;
	}

	if(!m_versionType.empty()) {
		modDependencyStream << " " << m_versionType;
	}

	return modDependencyStream.str();
}

bool ModDependency::isValid() const {
	return !m_id.empty();
}

bool ModDependency::isValid(const ModDependency * dependency) {
	return dependency != nullptr && dependency->isValid();
}

bool ModDependency::operator == (const ModDependency & dependency) const {
	return Utilities::areStringsEqualIgnoreCase(m_id, dependency.m_id) &&
		   Utilities::areStringsEqualIgnoreCase(m_version, dependency.m_version) &&
		   Utilities::areStringsEqualIgnoreCase(m_versionType, dependency.m_versionType);

}

bool ModDependency::operator != (const ModDependency & dependency) const {
	return !operator == (dependency);
}
