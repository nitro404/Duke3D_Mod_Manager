#include "ModIdentifier.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <array>
#include <string_view>
#include <vector>

static constexpr const char * JSON_NAME_PROPERTY_NAME = "name";
static constexpr const char * JSON_VERSION_PROPERTY_NAME = "version";
static constexpr const char * JSON_VERSION_TYPE_PROPERTY_NAME = "versionType";
static const std::array<std::string_view, 3> JSON_PROPERTY_NAMES = {
	JSON_NAME_PROPERTY_NAME,
	JSON_VERSION_PROPERTY_NAME,
	JSON_VERSION_TYPE_PROPERTY_NAME
};

ModIdentifier::ModIdentifier(const std::string & name, const std::string & version, const std::string & versionType)
	: m_name(Utilities::trimString(name))
	, m_version(Utilities::trimString(version))
	, m_versionType(Utilities::trimString(versionType)) { }

ModIdentifier::ModIdentifier(ModIdentifier && m) noexcept
	: m_name(std::move(m.m_name))
	, m_version(std::move(m.m_version))
	, m_versionType(std::move(m.m_versionType)) { }

ModIdentifier::ModIdentifier(const ModIdentifier & m)
	: m_name(m.m_name)
	, m_version(m.m_version)
	, m_versionType(m.m_versionType) { }

ModIdentifier & ModIdentifier::operator = (ModIdentifier && m) noexcept {
	if(this != &m) {
		m_name = std::move(m.m_name);
		m_version = std::move(m.m_version);
		m_versionType = std::move(m.m_versionType);
	}

	return *this;
}

ModIdentifier & ModIdentifier::operator = (const ModIdentifier & m) {
	m_name = m.m_name;
	m_version = m.m_version;
	m_versionType = m.m_versionType;

	return *this;
}

ModIdentifier::~ModIdentifier() = default;

const std::string & ModIdentifier::getName() const {
	return m_name;
}

const std::string & ModIdentifier::getVersion() const {
	return m_version;
}

const std::string & ModIdentifier::getVersionType() const {
	return m_versionType;
}

std::string ModIdentifier::getFullName() const {
	return fmt::format("{}{}{}", m_name, m_version.empty() ? "" : " " + m_version, m_versionType.empty() ? "" : " " + m_versionType);
}

void ModIdentifier::setName(const std::string & name) {
	m_name = Utilities::trimString(name);
}

void ModIdentifier::setVersion(const std::string & version) {
	m_version = Utilities::trimString(version);
}

void ModIdentifier::setVersionType(const std::string & versionType) {
	m_versionType = Utilities::trimString(versionType);
}

rapidjson::Value ModIdentifier::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value modIdentifierValue(rapidjson::kObjectType);

	rapidjson::Value nameValue(m_name.c_str(), allocator);
	modIdentifierValue.AddMember(rapidjson::StringRef(JSON_NAME_PROPERTY_NAME), nameValue, allocator);
	rapidjson::Value versionValue(m_version.c_str(), allocator);
	modIdentifierValue.AddMember(rapidjson::StringRef(JSON_VERSION_PROPERTY_NAME), versionValue, allocator);
	rapidjson::Value versionTypeValue(m_versionType.c_str(), allocator);
	modIdentifierValue.AddMember(rapidjson::StringRef(JSON_VERSION_TYPE_PROPERTY_NAME), versionTypeValue, allocator);

	return modIdentifierValue;
}

std::unique_ptr<ModIdentifier> ModIdentifier::parseFrom(const rapidjson::Value & modIdentifierValue) {
	if(!modIdentifierValue.IsObject()) {
		spdlog::error("Invalid mod identifier type: '{}', expected 'object'.", Utilities::typeToString(modIdentifierValue.GetType()));
		return nullptr;
	}

	// check for unhandled mod identifier properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = modIdentifierValue.MemberBegin(); i != modIdentifierValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::error("Mod identifier has unexpected property '{}'.", i->name.GetString());
			return nullptr;
		}
	}

	// parse mod identifier name
	if(!modIdentifierValue.HasMember(JSON_NAME_PROPERTY_NAME)) {
		spdlog::error("Mod identifier is missing '{}' property.", JSON_NAME_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modIdentifierNameValue = modIdentifierValue[JSON_NAME_PROPERTY_NAME];

	if(!modIdentifierNameValue.IsString()) {
		spdlog::error("Mod identifier has an invalid '{}' property type: '{}', expected 'string'.", JSON_NAME_PROPERTY_NAME, Utilities::typeToString(modIdentifierNameValue.GetType()));
		return nullptr;
	}

	std::string name(Utilities::trimString(modIdentifierNameValue.GetString()));

	if(name.empty()) {
		spdlog::error("Mod identifier '{}' property cannot be empty.", JSON_NAME_PROPERTY_NAME);
		return nullptr;
	}

	// parse mod identifier version
	std::string version;

	if(modIdentifierValue.HasMember(JSON_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & modIdentifierVersionValue = modIdentifierValue[JSON_VERSION_PROPERTY_NAME];

		if(!modIdentifierVersionValue.IsString()) {
			spdlog::error("Mod identifier has an invalid '{}' property type: '{}', expected 'string'.", JSON_VERSION_PROPERTY_NAME, Utilities::typeToString(modIdentifierVersionValue.GetType()));
			return nullptr;
		}

		version = modIdentifierVersionValue.GetString();
	}

	// parse mod identifier version type
	std::string versionType;

	if(modIdentifierValue.HasMember(JSON_VERSION_TYPE_PROPERTY_NAME)) {
		const rapidjson::Value & modIdentifierVersionTypeValue = modIdentifierValue[JSON_VERSION_TYPE_PROPERTY_NAME];

		if(!modIdentifierVersionTypeValue.IsString()) {
			spdlog::error("Mod identifier has an invalid '{}' property type: '{}', expected 'string'.", JSON_VERSION_TYPE_PROPERTY_NAME, Utilities::typeToString(modIdentifierVersionTypeValue.GetType()));
			return nullptr;
		}

		versionType = modIdentifierVersionTypeValue.GetString();
	}

	return std::make_unique<ModIdentifier>(name, version, versionType);
}

bool ModIdentifier::isValid() const {
	return !m_name.empty();
}

bool ModIdentifier::isValid(const ModIdentifier * m) {
	return m != nullptr && m->isValid();
}

bool ModIdentifier::operator == (const ModIdentifier & m) const {
	return Utilities::areStringsEqualIgnoreCase(m_name, m.m_name) &&
		   Utilities::areStringsEqualIgnoreCase(m_version, m.m_version) &&
		   Utilities::areStringsEqualIgnoreCase(m_versionType, m.m_versionType);
}

bool ModIdentifier::operator != (const ModIdentifier & m) const {
	return !operator == (m);
}
