#include "ModIdentifier.h"

#include "Manager/ModMatch.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/TimeUtilities.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <array>
#include <sstream>
#include <string_view>
#include <vector>

static constexpr const char * JSON_NAME_PROPERTY_NAME = "name";
static constexpr const char * JSON_VERSION_PROPERTY_NAME = "version";
static constexpr const char * JSON_VERSION_TYPE_PROPERTY_NAME = "versionType";
static constexpr const char * JSON_CREATED_TIMESTAMP_PROPERTY_NAME = "created";
static const std::array<std::string_view, 4> JSON_PROPERTY_NAMES = {
	JSON_NAME_PROPERTY_NAME,
	JSON_VERSION_PROPERTY_NAME,
	JSON_VERSION_TYPE_PROPERTY_NAME,
	JSON_CREATED_TIMESTAMP_PROPERTY_NAME
};

ModIdentifier::ModIdentifier(const std::string & name, const std::optional<std::string> & version, const std::optional<std::string> & versionType)
	: m_name(Utilities::trimString(name))
	, m_version(version.has_value() ? Utilities::trimString(version.value()) : std::optional<std::string>())
	, m_versionType(versionType.has_value() ? Utilities::trimString(versionType.value()) : std::optional<std::string>())
	, m_createdTimePoint(std::chrono::system_clock::now()) { }

ModIdentifier::ModIdentifier(const std::string & name, const std::optional<std::string> & version, const std::optional<std::string> & versionType, std::chrono::time_point<std::chrono::system_clock> createdTimePoint)
	: m_name(Utilities::trimString(name))
	, m_version(version.has_value() ? Utilities::trimString(version.value()) : std::optional<std::string>())
	, m_versionType(versionType.has_value() ? Utilities::trimString(versionType.value()) : std::optional<std::string>())
	, m_createdTimePoint(createdTimePoint) { }

ModIdentifier::ModIdentifier(const ModMatch & modMatch)
	: m_name(modMatch.getModName())
	, m_version(modMatch.getModVersionName())
	, m_versionType(modMatch.getModVersionTypeName())
	, m_createdTimePoint(std::chrono::system_clock::now()) { }

ModIdentifier::ModIdentifier(ModIdentifier && m) noexcept
	: m_name(std::move(m.m_name))
	, m_version(std::move(m.m_version))
	, m_versionType(std::move(m.m_versionType))
	, m_createdTimePoint(m.m_createdTimePoint) { }

ModIdentifier::ModIdentifier(const ModIdentifier & m)
	: m_name(m.m_name)
	, m_version(m.m_version)
	, m_versionType(m.m_versionType)
	, m_createdTimePoint(m.m_createdTimePoint) { }

ModIdentifier & ModIdentifier::operator = (ModIdentifier && m) noexcept {
	if(this != &m) {
		m_name = std::move(m.m_name);
		m_version = std::move(m.m_version);
		m_versionType = std::move(m.m_versionType);
		m_createdTimePoint = m.m_createdTimePoint;
	}

	return *this;
}

ModIdentifier & ModIdentifier::operator = (const ModIdentifier & m) {
	m_name = m.m_name;
	m_version = m.m_version;
	m_versionType = m.m_versionType;
	m_createdTimePoint = m.m_createdTimePoint;

	return *this;
}

ModIdentifier::~ModIdentifier() = default;

bool ModIdentifier::isModIdentifier() const {
	return !m_name.empty() &&
		   !m_version.has_value() &&
		   !m_versionType.has_value();
}

bool ModIdentifier::isModVersionIdentifier() const {
	return !m_name.empty() &&
		   m_version.has_value() &&
		   !m_versionType.has_value();
}

bool ModIdentifier::isModVersionTypeIdentifier() const {
	return !m_name.empty() &&
		   m_version.has_value() &&
		   m_versionType.has_value();
}

const std::string & ModIdentifier::getName() const {
	return m_name;
}

bool ModIdentifier::hasVersion() const {
	return m_version.has_value();
}

const std::optional<std::string> & ModIdentifier::getVersion() const {
	return m_version;
}

bool ModIdentifier::hasVersionType() const {
	return m_versionType.has_value();
}

const std::optional<std::string> & ModIdentifier::getVersionType() const {
	return m_versionType;
}

std::string ModIdentifier::getFullName() const {
	return fmt::format("{}{}{}", m_name, !m_version.has_value() || m_version.value().empty() ? "" : " " + m_version.value(), !m_versionType.has_value() || m_versionType.value().empty() ? "" : " " + m_versionType.value());
}

std::chrono::time_point<std::chrono::system_clock> ModIdentifier::getCreatedTimePoint() const {
	return m_createdTimePoint;
}

void ModIdentifier::setName(const std::string & name) {
	m_name = Utilities::trimString(name);
}

void ModIdentifier::setVersion(const std::string & version) {
	m_version = Utilities::trimString(version);
}

void ModIdentifier::clearVersion() {
	m_version.reset();
}

void ModIdentifier::setVersionType(const std::string & versionType) {
	m_versionType = Utilities::trimString(versionType);
}

void ModIdentifier::clearVersionType() {
	m_versionType.reset();
}

rapidjson::Value ModIdentifier::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value modIdentifierValue(rapidjson::kObjectType);

	rapidjson::Value nameValue(m_name.c_str(), allocator);
	modIdentifierValue.AddMember(rapidjson::StringRef(JSON_NAME_PROPERTY_NAME), nameValue, allocator);

	if(m_version.has_value()) {
		rapidjson::Value versionValue(m_version.value().c_str(), allocator);
		modIdentifierValue.AddMember(rapidjson::StringRef(JSON_VERSION_PROPERTY_NAME), versionValue, allocator);
	}

	if(m_versionType.has_value()) {
		rapidjson::Value versionTypeValue(m_versionType.value().c_str(), allocator);
		modIdentifierValue.AddMember(rapidjson::StringRef(JSON_VERSION_TYPE_PROPERTY_NAME), versionTypeValue, allocator);
	}

	rapidjson::Value createdTimestampValue(Utilities::timePointToString(m_createdTimePoint, Utilities::TimeFormat::ISO8601).c_str(), allocator);
	modIdentifierValue.AddMember(rapidjson::StringRef(JSON_CREATED_TIMESTAMP_PROPERTY_NAME), createdTimestampValue, allocator);

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
			spdlog::warn("Mod identifier has unexpected property '{}'.", i->name.GetString());
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
	std::optional<std::string> optionalVersion;

	if(modIdentifierValue.HasMember(JSON_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & modIdentifierVersionValue = modIdentifierValue[JSON_VERSION_PROPERTY_NAME];

		if(!modIdentifierVersionValue.IsString()) {
			spdlog::error("Mod identifier has an invalid '{}' property type: '{}', expected 'string'.", JSON_VERSION_PROPERTY_NAME, Utilities::typeToString(modIdentifierVersionValue.GetType()));
			return nullptr;
		}

		optionalVersion = modIdentifierVersionValue.GetString();
	}

	// parse mod identifier version type
	std::optional<std::string> optionalVersionType;

	if(modIdentifierValue.HasMember(JSON_VERSION_TYPE_PROPERTY_NAME)) {
		const rapidjson::Value & modIdentifierVersionTypeValue = modIdentifierValue[JSON_VERSION_TYPE_PROPERTY_NAME];

		if(!modIdentifierVersionTypeValue.IsString()) {
			spdlog::error("Mod identifier has an invalid '{}' property type: '{}', expected 'string'.", JSON_VERSION_TYPE_PROPERTY_NAME, Utilities::typeToString(modIdentifierVersionTypeValue.GetType()));
			return nullptr;
		}

		optionalVersionType = modIdentifierVersionTypeValue.GetString();
	}

	// parse mod identifier created timestamp
	if(!modIdentifierValue.HasMember(JSON_CREATED_TIMESTAMP_PROPERTY_NAME)) {
		spdlog::error("Mod identifier is missing '{}' property.", JSON_CREATED_TIMESTAMP_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modIdentifierCreatedTimestampValue = modIdentifierValue[JSON_CREATED_TIMESTAMP_PROPERTY_NAME];

	if(!modIdentifierCreatedTimestampValue.IsString()) {
		spdlog::error("Mod identifier has an invalid '{}' property type: '{}', expected 'string'.", JSON_CREATED_TIMESTAMP_PROPERTY_NAME, Utilities::typeToString(modIdentifierCreatedTimestampValue.GetType()));
		return nullptr;
	}

	std::optional<std::chrono::time_point<std::chrono::system_clock>> downloadedTimePoint(Utilities::parseTimePointFromString(modIdentifierCreatedTimestampValue.GetString()));

	if(!downloadedTimePoint.has_value()) {
		spdlog::error("Mod identifier has an invalid '{}' timestamp value: '{}'.", JSON_CREATED_TIMESTAMP_PROPERTY_NAME, modIdentifierCreatedTimestampValue.GetString());
		return nullptr;
	}

	return std::unique_ptr<ModIdentifier>(new ModIdentifier(name, optionalVersion, optionalVersionType, downloadedTimePoint.value()));
}

std::string ModIdentifier::toString() const {
	std::stringstream modIdentifierStream;

	modIdentifierStream << m_name;

	if(m_version.has_value()) {
		if(!m_version.value().empty()) {
			modIdentifierStream << " " << m_version.value();
		}

		if(m_versionType.has_value()) {
			if(!m_versionType.value().empty()) {
				modIdentifierStream << " " << m_versionType.value();
			}
		}
	}

	return modIdentifierStream.str();
}

bool ModIdentifier::isValid() const {
	if(m_name.empty()) {
		return false;
	}

	if(!m_version.has_value() && m_versionType.has_value()) {
		return false;
	}

	return true;
}

bool ModIdentifier::isValid(const ModIdentifier * m) {
	return m != nullptr && m->isValid();
}

bool ModIdentifier::operator == (const ModIdentifier & m) const {
	if(!Utilities::areStringsEqualIgnoreCase(m_name, m.m_name) ||
	   m_version.has_value() != m.m_version.has_value() ||
	   m_versionType.has_value() != m.m_versionType.has_value()) {
		return false;
	}

	if(m_version.has_value() && m.m_version.has_value() && !Utilities::areStringsEqualIgnoreCase(m_version.value(), m.m_version.value())) {
		return false;
	}

	if(m_versionType.has_value() && m.m_versionType.has_value() && !Utilities::areStringsEqualIgnoreCase(m_versionType.value(), m.m_versionType.value())) {
		return false;
	}

	return true;
}

bool ModIdentifier::operator != (const ModIdentifier & m) const {
	return !operator == (m);
}
