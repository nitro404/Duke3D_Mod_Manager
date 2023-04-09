#include "ModVersion.h"

#include "Game/GameVersion.h"
#include "Mod.h"
#include "ModGameVersion.h"
#include "ModVersionType.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <array>
#include <sstream>
#include <string_view>
#include <vector>

static const std::string XML_MOD_VERSION_ELEMENT_NAME("version");
static const std::string XML_MOD_VERSION_VERSION_ATTRIBUTE_NAME("id");
static const std::string XML_MOD_VERSION_RELEASE_DATE_ATTRIBUTE_NAME("release_date");
static const std::string XML_MOD_VERSION_REPAIRED_ATTRIBUTE_NAME("repaired");
static const std::array<std::string_view, 3> XML_MOD_VERSION_ATTRIBUTE_NAMES = {
	XML_MOD_VERSION_VERSION_ATTRIBUTE_NAME,
	XML_MOD_VERSION_RELEASE_DATE_ATTRIBUTE_NAME,
	XML_MOD_VERSION_REPAIRED_ATTRIBUTE_NAME
};
static const std::string XML_MOD_GAME_VERSION_ELEMENT_NAME("game");

static constexpr const char * JSON_MOD_VERSION_VERSION_PROPERTY_NAME = "version";
static constexpr const char * JSON_MOD_VERSION_RELEASE_DATE_PROPERTY_NAME = "releaseDate";
static constexpr const char * JSON_MOD_VERSION_REPAIRED_PROPERTY_NAME = "repaired";
static constexpr const char * JSON_MOD_VERSIONS_VERSION_TYPES_PROPERTY_NAME = "types";
static const std::array<std::string_view, 4> JSON_MOD_VERSION_PROPERTY_NAMES = {
	JSON_MOD_VERSION_VERSION_PROPERTY_NAME,
	JSON_MOD_VERSION_RELEASE_DATE_PROPERTY_NAME,
	JSON_MOD_VERSION_REPAIRED_PROPERTY_NAME,
	JSON_MOD_VERSIONS_VERSION_TYPES_PROPERTY_NAME
};

ModVersion::ModVersion(const std::string & version, std::optional<Date> releaseDate)
	: m_version(Utilities::trimString(version))
	, m_releaseDate(releaseDate)
	, m_parentMod(nullptr) { }

ModVersion::ModVersion(ModVersion && m) noexcept
	: m_version(std::move(m.m_version))
	, m_releaseDate(std::move(m.m_releaseDate))
	, m_repaired(m.m_repaired)
	, m_types(std::move(m.m_types))
	, m_parentMod(nullptr) {
	updateParent();
}

ModVersion::ModVersion(const ModVersion & m)
	: m_version(m.m_version)
	, m_releaseDate(m.m_releaseDate)
	, m_repaired(m.m_repaired)
	, m_parentMod(nullptr) {
	for(std::vector<std::shared_ptr<ModVersionType>>::const_iterator i = m.m_types.begin(); i != m.m_types.end(); ++i) {
		m_types.push_back(std::make_shared<ModVersionType>(**i));
	}

	updateParent();
}

ModVersion & ModVersion::operator = (ModVersion && m) noexcept {
	if(this != &m) {
		m_version = std::move(m.m_version);
		m_releaseDate = std::move(m.m_releaseDate);
		m_repaired = m.m_repaired;
		m_types = std::move(m.m_types);

		updateParent();
	}

	return *this;
}

ModVersion & ModVersion::operator = (const ModVersion & m) {
	m_types.clear();

	m_version = m.m_version;
	m_releaseDate = m.m_releaseDate;
	m_repaired = m.m_repaired;

	for(std::vector<std::shared_ptr<ModVersionType>>::const_iterator i = m.m_types.begin(); i != m.m_types.end(); ++i) {
		m_types.push_back(std::make_shared<ModVersionType>(**i));
	}

	updateParent();

	return *this;
}

ModVersion::~ModVersion() {
	m_parentMod = nullptr;
}

bool ModVersion::isDefault() const {
	return m_version.empty();
}

bool ModVersion::isStandAlone() const {
	for(const std::shared_ptr<ModVersionType> & type : m_types) {
		if(type->isStandAlone()) {
			return true;
		}
	}

	return false;
}

const std::string & ModVersion::getVersion() const {
	return m_version;
}

std::string ModVersion::getFullName(size_t versionTypeIndex) const {
	if(!Mod::isValid(m_parentMod, true)) {
		return "";
	}

	std::stringstream fullModName;
	fullModName << m_parentMod->getName();

	if(!m_version.empty()) {
		fullModName << " " + m_version;
	}

	const std::shared_ptr<ModVersionType> type = getType(versionTypeIndex);

	if(type == nullptr) {
		return fullModName.str();
	}

	if(!type->getType().empty()) {
		fullModName << " " + type->getType();
	}

	return fullModName.str();
}

const std::optional<Date> & ModVersion::getReleaseDate() const {
	return m_releaseDate;
}

std::string ModVersion::getReleaseDateAsString() const {
	return m_releaseDate.has_value() ? m_releaseDate.value().toString() : std::string();
}

bool ModVersion::isRepaired() const {
	return m_repaired.has_value() ? m_repaired.value() : false;
}

std::optional<bool> ModVersion::getRepaired() const {
	return m_repaired;
}

const Mod * ModVersion::getParentMod() const {
	return m_parentMod;
}

void ModVersion::setVersion(const std::string & version) {
	m_version = Utilities::trimString(version);
}

bool ModVersion::setReleaseDate(const Date & releaseDate) {
	if(!releaseDate.isValid()) {
		return false;
	}

	m_releaseDate = releaseDate;

	return true;
}

void ModVersion::clearReleaseDate() {
	m_releaseDate.reset();
}

void ModVersion::setRepaired(bool repaired) {
	m_repaired = repaired;
}

void ModVersion::clearRepaired() {
	m_repaired.reset();
}

void ModVersion::setParentMod(const Mod * mod) {
	m_parentMod = mod;
}

size_t ModVersion::numberOfTypes() const {
	return m_types.size();
}

bool ModVersion::hasType(const std::string & type) const {
	for(std::vector<std::shared_ptr<ModVersionType>>::const_iterator i = m_types.begin(); i != m_types.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getType(), type)) {
			return true;
		}
	}

	return false;
}

bool ModVersion::hasType(const ModVersionType & type) const {
	for(std::vector<std::shared_ptr<ModVersionType>>::const_iterator i = m_types.begin(); i != m_types.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getType(), type.getType())) {
			return true;
		}
	}

	return false;
}

size_t ModVersion::indexOfType(const std::string & type) const {
	for(size_t i = 0; i < m_types.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_types[i]->getType(), type)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t ModVersion::indexOfType(const ModVersionType & type) const {
	for(size_t i = 0; i < m_types.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_types[i]->getType(), type.getType())) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<ModVersionType> ModVersion::getType(size_t index) const {
	if(index >= m_types.size()) {
		return nullptr;
	}

	return m_types[index];
}

std::shared_ptr<ModVersionType> ModVersion::getType(const std::string & type) const {
	for(std::vector<std::shared_ptr<ModVersionType>>::const_iterator i = m_types.begin(); i != m_types.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getType(), type)) {
			return *i;
		}
	}

	return nullptr;
}

const std::vector<std::shared_ptr<ModVersionType>> & ModVersion::getTypes() const {
	return m_types;
}

std::vector<std::string> ModVersion::getTypeDisplayNames(const std::string & emptySubstitution) const {
	std::vector<std::string> typeDisplayNames;

	for(const std::shared_ptr<ModVersionType> & type : m_types) {
		if(type->getType().empty()) {
			typeDisplayNames.push_back(emptySubstitution);
		}
		else {
			typeDisplayNames.push_back(type->getType());
		}
	}

	return typeDisplayNames;
}

bool ModVersion::addType(const ModVersionType & type) {
	if(!type.isValid() || hasType(type)) {
		return false;
	}

	std::shared_ptr<ModVersionType> newModVersionType = std::make_shared<ModVersionType>(type);
	newModVersionType->setParentModVersion(this);

	m_types.push_back(newModVersionType);

	return true;
}

bool ModVersion::removeType(size_t index) {
	if(index >= m_types.size()) {
		return false;
	}

	m_types[index]->setParentModVersion(nullptr);
	m_types.erase(m_types.begin() + index);

	return true;
}

bool ModVersion::removeType(const std::string & type) {
	for(std::vector<std::shared_ptr<ModVersionType>>::const_iterator i = m_types.begin(); i != m_types.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getType(), type)) {
			(*i)->setParentModVersion(nullptr);
			m_types.erase(i);

			return true;
		}
	}

	return false;
}

bool ModVersion::removeType(const ModVersionType & type) {
	for(std::vector<std::shared_ptr<ModVersionType>>::const_iterator i = m_types.begin(); i != m_types.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getType(), type.getType())) {
			(*i)->setParentModVersion(nullptr);
			m_types.erase(i);

			return true;
		}
	}

	return false;
}

void ModVersion::clearTypes() {
	m_types.clear();
}

void ModVersion::updateParent() {
	for(std::vector<std::shared_ptr<ModVersionType>>::const_iterator i = m_types.begin(); i != m_types.end(); ++i) {
		(*i)->setParentModVersion(this);
	}
}

rapidjson::Value ModVersion::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value modVersionValue(rapidjson::kObjectType);

	if(!m_version.empty()) {
		rapidjson::Value versionValue(m_version.c_str(), allocator);
		modVersionValue.AddMember(rapidjson::StringRef(JSON_MOD_VERSION_VERSION_PROPERTY_NAME), versionValue, allocator);
	}

	if(m_releaseDate.has_value()) {
		rapidjson::Value releaseDateValue(m_releaseDate.value().toString().c_str(), allocator);
		modVersionValue.AddMember(rapidjson::StringRef(JSON_MOD_VERSION_RELEASE_DATE_PROPERTY_NAME), releaseDateValue, allocator);
	}

	if(m_repaired.has_value()) {
		modVersionValue.AddMember(rapidjson::StringRef(JSON_MOD_VERSION_REPAIRED_PROPERTY_NAME), rapidjson::Value(m_repaired.value()), allocator);
	}

	rapidjson::Value versionsTypesValue(rapidjson::kArrayType);

	for(std::vector<std::shared_ptr<ModVersionType>>::const_iterator i = m_types.begin(); i != m_types.end(); ++i) {
		versionsTypesValue.PushBack((*i)->toJSON(allocator), allocator);
	}

	modVersionValue.AddMember(rapidjson::StringRef(JSON_MOD_VERSIONS_VERSION_TYPES_PROPERTY_NAME), versionsTypesValue, allocator);

	return modVersionValue;
}

tinyxml2::XMLElement * ModVersion::toXML(tinyxml2::XMLDocument * document) const {
	static constexpr bool forceAddSingleEmptyVersionTypes = false;

	if(document == nullptr) {
		return nullptr;
	}

	tinyxml2::XMLElement * modVersionElement = document->NewElement(XML_MOD_VERSION_ELEMENT_NAME.c_str());

	if(!m_version.empty()) {
		modVersionElement->SetAttribute(XML_MOD_VERSION_VERSION_ATTRIBUTE_NAME.c_str(), m_version.c_str());
	}

	if(m_releaseDate.has_value()) {
		modVersionElement->SetAttribute(XML_MOD_VERSION_RELEASE_DATE_ATTRIBUTE_NAME.c_str(), m_releaseDate.value().toString().c_str());
	}

	if(m_repaired.has_value()) {
		modVersionElement->SetAttribute(XML_MOD_VERSION_REPAIRED_ATTRIBUTE_NAME.c_str(), m_repaired.value());
	}

	if(!forceAddSingleEmptyVersionTypes &&
	   m_types.size() == 1 &&
	   m_types[0]->isDefault() &&
	   !m_types[0]->hadXMLElement()) {
		const std::shared_ptr<ModVersionType> modVersionType = m_types[0];

		for(size_t i = 0; i < modVersionType->numberOfGameVersions(); i++) {
			modVersionElement->InsertEndChild(modVersionType->getGameVersion(i)->toXML(document));
		}
	}
	else {
		for(std::vector<std::shared_ptr<ModVersionType>>::const_iterator i = m_types.begin(); i != m_types.end(); ++i) {
			modVersionElement->InsertEndChild((*i)->toXML(document));
		}
	}

	return modVersionElement;
}

std::unique_ptr<ModVersion> ModVersion::parseFrom(const rapidjson::Value & modVersionValue, const rapidjson::Value & modValue, bool skipFileInfoValidation) {
	if(!modVersionValue.IsObject()) {
		spdlog::error("Invalid mod version type: '{}', expected 'object'.", Utilities::typeToString(modVersionValue.GetType()));
		return nullptr;
	}

	// check for unhandled mod version properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = modVersionValue.MemberBegin(); i != modVersionValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_MOD_VERSION_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::warn("Mod version has unexpected property '{}'.", i->name.GetString());
		}
	}

	// initialize the mod version
	std::unique_ptr<ModVersion> newModVersion = std::make_unique<ModVersion>();

	// parse the mod version version property
	if(modVersionValue.HasMember(JSON_MOD_VERSION_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & modVersionVersionValue = modVersionValue[JSON_MOD_VERSION_VERSION_PROPERTY_NAME];

		if(!modVersionVersionValue.IsString()) {
			spdlog::error("Mod version '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_VERSION_VERSION_PROPERTY_NAME, Utilities::typeToString(modVersionVersionValue.GetType()));
			return nullptr;
		}

		newModVersion->setVersion(modVersionVersionValue.GetString());
	}

	// parse the mod version release date property
	if(modVersionValue.HasMember(JSON_MOD_VERSION_RELEASE_DATE_PROPERTY_NAME)) {
		const rapidjson::Value & modVersionReleaseDateValue = modVersionValue[JSON_MOD_VERSION_RELEASE_DATE_PROPERTY_NAME];

		if(!modVersionReleaseDateValue.IsString()) {
			spdlog::error("Mod version '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_VERSION_RELEASE_DATE_PROPERTY_NAME, Utilities::typeToString(modVersionReleaseDateValue.GetType()));
			return nullptr;
		}

		std::optional<Date> releaseDate = Date::parseFrom(modVersionReleaseDateValue.GetString());

		if(releaseDate.has_value()) {
			newModVersion->setReleaseDate(releaseDate.value());
		}
		else {
			spdlog::error("Mod version '{}' property has invalid value: '{}'.", JSON_MOD_VERSION_RELEASE_DATE_PROPERTY_NAME, Utilities::valueToString(modVersionReleaseDateValue));
			return nullptr;
		}
	}

	// parse the mod version repaired property
	if(modVersionValue.HasMember(JSON_MOD_VERSION_REPAIRED_PROPERTY_NAME)) {
		const rapidjson::Value & modVersionRepairedValue = modVersionValue[JSON_MOD_VERSION_REPAIRED_PROPERTY_NAME];

		if(!modVersionRepairedValue.IsBool()) {
			spdlog::error("Mod version '{}' property has invalid type: '{}', expected 'boolean'.", JSON_MOD_VERSION_REPAIRED_PROPERTY_NAME, Utilities::typeToString(modVersionRepairedValue.GetType()));
			return nullptr;
		}

		newModVersion->setRepaired(modVersionRepairedValue.GetBool());
	}

	// parse the mod version types property
	if(!modVersionValue.HasMember(JSON_MOD_VERSIONS_VERSION_TYPES_PROPERTY_NAME)) {
		spdlog::error("Mod version is missing '{}' property'.", JSON_MOD_VERSIONS_VERSION_TYPES_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modVersionTypesValue = modVersionValue[JSON_MOD_VERSIONS_VERSION_TYPES_PROPERTY_NAME];

	if(!modVersionTypesValue.IsArray()) {
		spdlog::error("Mod version '{}' property has invalid type: '{}', expected 'array'.", JSON_MOD_VERSIONS_VERSION_TYPES_PROPERTY_NAME, Utilities::typeToString(modVersionTypesValue.GetType()));
		return nullptr;
	}

	std::shared_ptr<ModVersionType> newModVersionType;

	for(rapidjson::Value::ConstValueIterator i = modVersionTypesValue.Begin(); i != modVersionTypesValue.End(); ++i) {
		newModVersionType = std::shared_ptr<ModVersionType>(ModVersionType::parseFrom(*i, modValue, skipFileInfoValidation).release());

		if(!ModVersionType::isValid(newModVersionType.get(), skipFileInfoValidation)) {
			spdlog::error("Failed to parse mod version type #{}.", newModVersion->m_types.size() + 1);
			return nullptr;
		}

		newModVersionType->setParentModVersion(newModVersion.get());

		if(newModVersion->hasType(*newModVersionType.get())) {
			spdlog::error("Encountered duplicate mod version type #{}.", newModVersion->m_types.size() + 1);
			return nullptr;
		}

		newModVersion->m_types.push_back(newModVersionType);
	}

	return newModVersion;
}

std::unique_ptr<ModVersion> ModVersion::parseFrom(const tinyxml2::XMLElement * modVersionElement, bool skipFileInfoValidation) {
	if(modVersionElement == nullptr) {
		return nullptr;
	}

	// verify element name
	if(modVersionElement->Name() != XML_MOD_VERSION_ELEMENT_NAME) {
		spdlog::error("Invalid mod version element name: '{}', expected '{}'.", modVersionElement->Name(), XML_MOD_VERSION_ELEMENT_NAME);
		return nullptr;
	}

	// check for unhandled mod version element attributes
	bool attributeHandled = false;
	const tinyxml2::XMLAttribute * modVersionAttribute = modVersionElement->FirstAttribute();

	while(true) {
		if(modVersionAttribute == nullptr) {
			break;
		}

		attributeHandled = false;

		for(const std::string_view & attributeName : XML_MOD_VERSION_ATTRIBUTE_NAMES) {
			if(modVersionAttribute->Name() == attributeName) {
				attributeHandled = true;
				break;
			}
		}

		if(!attributeHandled) {
			spdlog::warn("Element '{}' has unexpected attribute '{}'.", XML_MOD_VERSION_ELEMENT_NAME, modVersionAttribute->Name());
		}

		modVersionAttribute = modVersionAttribute->Next();
	}

	// read the mod version attributes
	const char * modVersionVersion = modVersionElement->Attribute(XML_MOD_VERSION_VERSION_ATTRIBUTE_NAME.c_str());
	const char * modVersionReleaseDate = modVersionElement->Attribute(XML_MOD_VERSION_RELEASE_DATE_ATTRIBUTE_NAME.c_str());
	const char * modVersionRepairedData = modVersionElement->Attribute(XML_MOD_VERSION_REPAIRED_ATTRIBUTE_NAME.c_str());

	std::optional<Date> modVersionReleaseDateOptional;

	if(modVersionReleaseDate != nullptr) {
		modVersionReleaseDateOptional = Date::parseFrom(modVersionReleaseDate);
	}

	// initialize the mod version
	std::unique_ptr<ModVersion> newModVersion = std::make_unique<ModVersion>(modVersionVersion == nullptr ? "" : modVersionVersion, modVersionReleaseDateOptional);

	bool error = false;

	if(modVersionRepairedData != nullptr) {
		bool repaired = Utilities::parseBoolean(modVersionRepairedData, &error);

		if(error) {
			spdlog::error("Attribute '{}' in element '{}' has an invalid value: '{}', expected boolean.", XML_MOD_VERSION_REPAIRED_ATTRIBUTE_NAME, XML_MOD_VERSION_ELEMENT_NAME, modVersionRepairedData);
			return nullptr;
		}

		newModVersion->setRepaired(repaired);
	}

	// iterate over all of the mod version type elements
	const tinyxml2::XMLElement * modVersionTypeElement = modVersionElement->FirstChildElement();

	if(modVersionTypeElement == nullptr) {
		spdlog::error("Element '{}' has no children.", XML_MOD_VERSION_ELEMENT_NAME);
		return nullptr;
	}

	std::shared_ptr<ModVersionType> newModVersionType;

	while(true) {
		if(modVersionTypeElement == nullptr) {
			break;
		}

		newModVersionType = std::shared_ptr<ModVersionType>(ModVersionType::parseFrom(modVersionTypeElement, skipFileInfoValidation).release());

		if(!ModVersionType::isValid(newModVersionType.get(), skipFileInfoValidation)) {
			spdlog::error("Failed to parse mod version type #{}.", newModVersion->m_types.size() + 1);
			return nullptr;
		}

		newModVersionType->setParentModVersion(newModVersion.get());

		if(newModVersion->hasType(*newModVersionType.get())) {
			spdlog::error("Encountered duplicate mod version type #{}.", newModVersion->m_types.size() + 1);
			return nullptr;
		}

		newModVersion->m_types.push_back(newModVersionType);

		// special handling for mod versions with no explicit type
		if(modVersionTypeElement->Name() == XML_MOD_GAME_VERSION_ELEMENT_NAME) {
			break;
		}

		modVersionTypeElement = modVersionTypeElement->NextSiblingElement();
	}

	return newModVersion;
}

bool ModVersion::isGameVersionSupported(const GameVersion & gameVersion) const {
	if(!gameVersion.isValid()) {
		return false;
	}

	for(size_t i = 0; i < m_types.size(); i++) {
		if(m_types[i]->isGameVersionSupported(gameVersion)) {
			return true;
		}
	}

	return false;
}

bool ModVersion::isGameVersionCompatible(const GameVersion & gameVersion) const {
	if(!gameVersion.isValid()) {
		return false;
	}

	for(size_t i = 0; i < m_types.size(); i++) {
		if(m_types[i]->isGameVersionCompatible(gameVersion)) {
			return true;
		}
	}

	return false;
}

bool ModVersion::isValid(bool skipFileInfoValidation) const {
	if(m_types.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModVersionType>>::const_iterator i = m_types.begin(); i != m_types.end(); ++i) {
		if(!(*i)->isValid(skipFileInfoValidation)) {
			return false;
		}

		if((*i)->getParentModVersion() != this) {
			return false;
		}

		for(std::vector<std::shared_ptr<ModVersionType>>::const_iterator j = i + 1; j != m_types.end(); ++j) {
			if(Utilities::areStringsEqualIgnoreCase((*i)->getType(), (*j)->getType())) {
				return false;
			}
		}
	}

	return true;
}

bool ModVersion::isValid(const ModVersion * m, bool skipFileInfoValidation) {
	return m != nullptr && m->isValid(skipFileInfoValidation);
}

bool ModVersion::operator == (const ModVersion & m) const {
	return m_repaired == m.m_repaired;
		   Utilities::areStringsEqualIgnoreCase(m_version, m.m_version) &&
		   m_releaseDate != m.m_releaseDate;
}

bool ModVersion::operator != (const ModVersion & m) const {
	return !operator == (m);
}
