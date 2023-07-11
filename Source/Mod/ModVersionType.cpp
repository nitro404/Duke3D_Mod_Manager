#include "ModVersionType.h"

#include "Game/GameVersion.h"
#include "Game/GameVersionCollection.h"
#include "Mod.h"
#include "ModVersion.h"
#include "ModGameVersion.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <array>
#include <sstream>
#include <string_view>
#include <vector>

static const std::string XML_MOD_VERSION_TYPE_ELEMENT_NAME("type");
static const std::string XML_MOD_GAME_VERSION_ELEMENT_NAME("game");
static const std::string XML_MOD_VERSION_TYPE_TYPE_ATTRIBUTE_NAME("id");
static const std::array<std::string_view, 1> XML_MOD_VERSION_TYPE_ATTRIBUTE_NAMES = {
	XML_MOD_VERSION_TYPE_TYPE_ATTRIBUTE_NAME,
};

static constexpr const char * JSON_MOD_VERSION_TYPE_TYPE_PROPERTY_NAME = "type";
static constexpr const char * JSON_MOD_VERSION_TYPE_GAME_VERSIONS_PROPERTY_NAME = "gameVersions";
static const std::array<std::string_view, 2> JSON_MOD_VERSION_TYPE_PROPERTY_NAMES = {
	JSON_MOD_VERSION_TYPE_TYPE_PROPERTY_NAME,
	JSON_MOD_VERSION_TYPE_GAME_VERSIONS_PROPERTY_NAME
};

ModVersionType::ModVersionType(const std::string & type)
	: m_type(Utilities::trimString(type))
	, m_hadXMLElement(false)
	, m_parentModVersion(nullptr) { }

ModVersionType::ModVersionType(ModVersionType && t) noexcept
	: m_type(std::move(t.m_type))
	, m_hadXMLElement(t.m_hadXMLElement)
	, m_gameVersions(std::move(t.m_gameVersions))
	, m_parentModVersion(nullptr) {
	updateParent();
}

ModVersionType::ModVersionType(const ModVersionType & t)
	: m_type(t.m_type)
	, m_hadXMLElement(t.m_hadXMLElement)
	, m_parentModVersion(nullptr) {
	for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = t.m_gameVersions.begin(); i != t.m_gameVersions.end(); ++i) {
		m_gameVersions.push_back(std::make_shared<ModGameVersion>(**i));
	}

	updateParent();
}

ModVersionType & ModVersionType::operator = (ModVersionType && t) noexcept {
	if(this != &t) {
		m_type = std::move(t.m_type);
		m_hadXMLElement = t.m_hadXMLElement;
		m_gameVersions = std::move(t.m_gameVersions);

		updateParent();
	}

	return *this;
}

ModVersionType & ModVersionType::operator = (const ModVersionType & t) {
	m_gameVersions.clear();

	m_type = t.m_type;
	m_hadXMLElement = t.m_hadXMLElement;

	for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = t.m_gameVersions.begin(); i != t.m_gameVersions.end(); ++i) {
		m_gameVersions.push_back(std::make_shared<ModGameVersion>(**i));
	}

	updateParent();

	return *this;
}

ModVersionType::~ModVersionType() {
	m_parentModVersion = nullptr;
}

bool ModVersionType::isDefault() const {
	return m_type.empty();
}

bool ModVersionType::isStandAlone() const {
	if(m_gameVersions.empty()) {
		return false;
	}

	return m_gameVersions[0]->isStandAlone();
}

const std::string & ModVersionType::getType() const {
	return m_type;
}

std::string ModVersionType::getFullName() const {
	const Mod * parentMod = getParentMod();

	if(!Mod::isValid(parentMod, true) || m_parentModVersion == nullptr) {
		return "";
	}

	std::stringstream fullModName;
	fullModName << parentMod->getName();

	if(!m_parentModVersion->getVersion().empty()) {
		fullModName << " " + m_parentModVersion->getVersion();
	}

	if(!m_type.empty()) {
		fullModName << " " + m_type;
	}

	return fullModName.str();
}

const Mod * ModVersionType::getParentMod() const {
	if(m_parentModVersion == nullptr) {
		return nullptr;
	}

	return m_parentModVersion->getParentMod();
}

const ModVersion * ModVersionType::getParentModVersion() const {
	return m_parentModVersion;
}

void ModVersionType::setType(const std::string & type) {
	m_type = Utilities::trimString(type);
}

bool ModVersionType::copyHiddenPropertiesFrom(const ModVersionType & modVersionType) {
	if(!isValid(true) || !modVersionType.isValid(true) || !Utilities::areStringsEqualIgnoreCase(m_type, modVersionType.m_type) || m_gameVersions.size() != modVersionType.m_gameVersions.size()) {
		return false;
	}

	m_hadXMLElement = modVersionType.m_hadXMLElement;

	return true;
}

bool ModVersionType::hadXMLElement() const {
	return m_hadXMLElement;
}

void ModVersionType::setParentModVersion(const ModVersion * modVersion) {
	m_parentModVersion = modVersion;
}

size_t ModVersionType::numberOfGameVersions() const {
	return m_gameVersions.size();
}

bool ModVersionType::hasGameVersionWithID(const std::string & gameVersionID) const {
	if(gameVersionID.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getGameVersionID(), gameVersionID)) {
			return true;
		}
	}

	return false;
}

bool ModVersionType::hasGameVersion(const ModGameVersion & gameVersion) const {
	for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getGameVersionID(), gameVersion.getGameVersionID())) {
			return true;
		}
	}

	return false;
}

size_t ModVersionType::indexOfGameVersionWithID(const std::string & gameVersionID) const {
	if(gameVersionID.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_gameVersions.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_gameVersions[i]->getGameVersionID(), gameVersionID)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t ModVersionType::indexOfGameVersion(const ModGameVersion & gameVersion) const {
	for(size_t i = 0; i < m_gameVersions.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_gameVersions[i]->getGameVersionID(), gameVersion.getGameVersionID())) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<ModGameVersion> ModVersionType::getGameVersion(size_t index) const {
	if(index >= m_gameVersions.size()) {
		return nullptr;
	}

	return m_gameVersions[index];
}

std::shared_ptr<ModGameVersion> ModVersionType::getGameVersionWithID(const std::string & gameVersionID) const {
	if(gameVersionID.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getGameVersionID(), gameVersionID)) {
			return *i;
		}
	}

	return nullptr;
}

const std::vector<std::shared_ptr<ModGameVersion>> & ModVersionType::getGameVersions() const {
	return m_gameVersions;
}

bool ModVersionType::addGameVersion(const ModGameVersion & gameVersion) {
	if(!gameVersion.isValid() || hasGameVersion(gameVersion)) {
		return false;
	}

	std::shared_ptr<ModGameVersion> newModGameVersion = std::make_shared<ModGameVersion>(gameVersion);
	newModGameVersion->setParentModVersionType(this);

	m_gameVersions.push_back(newModGameVersion);

	return true;
}

bool ModVersionType::removeGameVersion(size_t index) {
	if(index >= m_gameVersions.size()) {
		return false;
	}

	m_gameVersions[index]->setParentModVersionType(nullptr);
	m_gameVersions.erase(m_gameVersions.begin() + index);

	return true;
}

bool ModVersionType::removeGameVersionWithID(const std::string & gameVersionID) {
	if(gameVersionID.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getGameVersionID(), gameVersionID)) {
			(*i)->setParentModVersionType(nullptr);
			m_gameVersions.erase(i);

			return true;
		}
	}

	return false;
}

bool ModVersionType::removeGameVersion(const ModGameVersion & gameVersion) {
	for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getGameVersionID(), gameVersion.getGameVersionID())) {
			(*i)->setParentModVersionType(nullptr);
			m_gameVersions.erase(i);

			return true;
		}
	}

	return false;
}

void ModVersionType::clearGameVersions() {
	m_gameVersions.clear();
}

void ModVersionType::updateParent() {
	for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		(*i)->setParentModVersionType(this);
	}
}

rapidjson::Value ModVersionType::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value modVersionTypeValue(rapidjson::kObjectType);

	if(!m_type.empty()) {
		rapidjson::Value typeValue(m_type.c_str(), allocator);
		modVersionTypeValue.AddMember(rapidjson::StringRef(JSON_MOD_VERSION_TYPE_TYPE_PROPERTY_NAME), typeValue, allocator);
	}

	rapidjson::Value gameVersionsValue(rapidjson::kArrayType);
	gameVersionsValue.Reserve(m_gameVersions.size(), allocator);

	for(const std::shared_ptr<ModGameVersion> & gameversion : m_gameVersions) {
		gameVersionsValue.PushBack(gameversion->toJSON(allocator), allocator);
	}

	modVersionTypeValue.AddMember(rapidjson::StringRef(JSON_MOD_VERSION_TYPE_GAME_VERSIONS_PROPERTY_NAME), gameVersionsValue, allocator);

	return modVersionTypeValue;
}

tinyxml2::XMLElement * ModVersionType::toXML(tinyxml2::XMLDocument * document) const {
	if(document == nullptr) {
		return nullptr;
	}

	tinyxml2::XMLElement * modVersionTypeElement = document->NewElement(XML_MOD_VERSION_TYPE_ELEMENT_NAME.c_str());

	if(!m_type.empty()) {
		modVersionTypeElement->SetAttribute(XML_MOD_VERSION_TYPE_TYPE_ATTRIBUTE_NAME.c_str(), m_type.c_str());
	}

	for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		modVersionTypeElement->InsertEndChild((*i)->toXML(document));
	}

	return modVersionTypeElement;
}

std::unique_ptr<ModVersionType> ModVersionType::parseFrom(const rapidjson::Value & modVersionTypeValue, const rapidjson::Value & modValue, bool skipFileInfoValidation) {
	if(!modVersionTypeValue.IsObject()) {
		spdlog::error("Invalid mod version type: '{}', expected 'object'.", Utilities::typeToString(modVersionTypeValue.GetType()));
		return nullptr;
	}

	// check for unhandled mod version type properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = modVersionTypeValue.MemberBegin(); i != modVersionTypeValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_MOD_VERSION_TYPE_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::warn("Mod version type has unexpected property '{}'.", i->name.GetString());
		}
	}

	// initialize the mod version type
	std::unique_ptr<ModVersionType> newModVersionType = std::make_unique<ModVersionType>();

	// parse the mod version type type property
	if(modVersionTypeValue.HasMember(JSON_MOD_VERSION_TYPE_TYPE_PROPERTY_NAME)) {
		const rapidjson::Value & modVersionTypeTypeValue = modVersionTypeValue[JSON_MOD_VERSION_TYPE_TYPE_PROPERTY_NAME];

		if(!modVersionTypeTypeValue.IsString()) {
			spdlog::error("Mod version type '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_VERSION_TYPE_TYPE_PROPERTY_NAME, Utilities::typeToString(modVersionTypeTypeValue.GetType()));
			return nullptr;
		}

		newModVersionType->setType(modVersionTypeTypeValue.GetString());
	}

	// parse the mod version game types property
	if(!modVersionTypeValue.HasMember(JSON_MOD_VERSION_TYPE_GAME_VERSIONS_PROPERTY_NAME)) {
		spdlog::error("Mod version type is missing '{}' property.", JSON_MOD_VERSION_TYPE_GAME_VERSIONS_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modGameVersionsValue = modVersionTypeValue[JSON_MOD_VERSION_TYPE_GAME_VERSIONS_PROPERTY_NAME];

	if(!modGameVersionsValue.IsArray()) {
		spdlog::error("Mod game version '{}' property has invalid type: '{}', expected 'array'.", JSON_MOD_VERSION_TYPE_GAME_VERSIONS_PROPERTY_NAME, Utilities::typeToString(modGameVersionsValue.GetType()));
		return nullptr;
	}

	std::shared_ptr<ModGameVersion> newModGameVersion;

	for(rapidjson::Value::ConstValueIterator i = modGameVersionsValue.Begin(); i != modGameVersionsValue.End(); ++i) {
		newModGameVersion = ModGameVersion::parseFrom(*i, modValue, skipFileInfoValidation);

		if(!ModGameVersion::isValid(newModGameVersion.get(), skipFileInfoValidation)) {
			spdlog::error("Failed to parse mod game version #{}.", newModVersionType->m_gameVersions.size() + 1);
			return nullptr;
		}

		newModGameVersion->setParentModVersionType(newModVersionType.get());

		if(newModVersionType->hasGameVersion(*newModGameVersion)) {
			spdlog::error("Encountered duplicate game mod version #{}.", newModVersionType->m_gameVersions.size() + 1);
			return nullptr;
		}

		newModVersionType->m_gameVersions.push_back(newModGameVersion);
	}

	return newModVersionType;
}

std::unique_ptr<ModVersionType> ModVersionType::parseFrom(const tinyxml2::XMLElement * modVersionTypeElement, bool skipFileInfoValidation) {
	if(modVersionTypeElement == nullptr) {
		return nullptr;
	}

	const tinyxml2::XMLElement * modGameVersionElement = nullptr;
	std::unique_ptr<ModVersionType> newModVersionType;
	bool hadXMLElement = false;

	// parse non-default mod version type
	if(modVersionTypeElement->Name() == XML_MOD_VERSION_TYPE_ELEMENT_NAME) {
		hadXMLElement = true;

		// check for unhandled mod version type element attributes
		bool attributeHandled = false;
		const tinyxml2::XMLAttribute * modVersionTypeAttribute = modVersionTypeElement->FirstAttribute();

		while(true) {
			if(modVersionTypeAttribute == nullptr) {
				break;
			}

			attributeHandled = false;

			for(const std::string_view & attributeName : XML_MOD_VERSION_TYPE_ATTRIBUTE_NAMES) {
				if(modVersionTypeAttribute->Name() == attributeName) {
					attributeHandled = true;
					break;
				}
			}

			if(!attributeHandled) {
				spdlog::warn("Element '{}' has unexpected attribute '{}'.", XML_MOD_VERSION_TYPE_ELEMENT_NAME, modVersionTypeAttribute->Name());
			}

			modVersionTypeAttribute = modVersionTypeAttribute->Next();
		}

		// read the mod version type attributes
		const char * modVersionTypeType = modVersionTypeElement->Attribute(XML_MOD_VERSION_TYPE_TYPE_ATTRIBUTE_NAME.c_str());

		// initialize the mod version type
		newModVersionType = std::make_unique<ModVersionType>(modVersionTypeType == nullptr ? "" : modVersionTypeType);
		newModVersionType->m_hadXMLElement = hadXMLElement;

		// use the first child element of the mod version type element as the game version element
		modGameVersionElement = modVersionTypeElement->FirstChildElement();
	}
	else if(modVersionTypeElement->Name() == XML_MOD_GAME_VERSION_ELEMENT_NAME) {
		// initialize a default mod version type
		newModVersionType = std::make_unique<ModVersionType>();

		// use the current mod version type element as the game version element
		modGameVersionElement = modVersionTypeElement;
	}
	else {
		spdlog::error("Invalid mod version type / game version element name: '{}', expected '{}' or '{}'.", modVersionTypeElement->Name(), XML_MOD_VERSION_TYPE_ELEMENT_NAME, XML_MOD_GAME_VERSION_ELEMENT_NAME);
		return nullptr;
	}

	// iterate over all of the mod game version elements
	if(modGameVersionElement == nullptr) {
		spdlog::error("Element '{}' has no children.", XML_MOD_VERSION_TYPE_ELEMENT_NAME);
		return nullptr;
	}

	std::shared_ptr<ModGameVersion> newModGameVersion;

	while(true) {
		if(modGameVersionElement == nullptr) {
			break;
		}

		newModGameVersion = ModGameVersion::parseFrom(modGameVersionElement, skipFileInfoValidation);

		if(!ModGameVersion::isValid(newModGameVersion.get(), skipFileInfoValidation)) {
			spdlog::error("Failed to parse mod game version #{}.", newModVersionType->m_gameVersions.size() + 1);
			return nullptr;
		}

		newModGameVersion->setParentModVersionType(newModVersionType.get());

		if(newModVersionType->hasGameVersion(*newModGameVersion)) {
			spdlog::error("Encountered duplicate mod game version #{}.", newModVersionType->m_gameVersions.size() + 1);
			return nullptr;
		}

		newModVersionType->m_gameVersions.push_back(newModGameVersion);

		modGameVersionElement = modGameVersionElement->NextSiblingElement();
	}

	return newModVersionType;
}

bool ModVersionType::isGameVersionSupported(const GameVersion & gameVersion) const {
	if(!gameVersion.isValid()) {
		return false;
	}

	for(size_t i = 0; i < m_gameVersions.size(); i++) {
		if(m_gameVersions[i]->isGameVersionSupported(gameVersion)) {
			return true;
		}
	}

	return false;
}

bool ModVersionType::isGameVersionCompatible(const GameVersion & gameVersion) const {
	if(!gameVersion.isValid()) {
		return false;
	}

	for(size_t i = 0; i < m_gameVersions.size(); i++) {
		if(m_gameVersions[i]->isGameVersionCompatible(gameVersion)) {
			return true;
		}
	}

	return false;
}

std::vector<std::shared_ptr<ModGameVersion>> ModVersionType::getCompatibleModGameVersions(const GameVersion & gameVersion) const {
	if(!gameVersion.isValid()) {
		return {};
	}

	std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions;

	for(const std::shared_ptr<ModGameVersion> & modGameVersion : m_gameVersions) {
		if(modGameVersion->isGameVersionCompatible(gameVersion)) {
			compatibleModGameVersions.push_back(modGameVersion);
		}
	}

	return compatibleModGameVersions;
}

std::vector<std::string> ModVersionType::getModGameVersionIdentifiers() const {
	std::vector<std::string> gameVersionIdentifiers;
	gameVersionIdentifiers.reserve(m_gameVersions.size());

	for(const std::shared_ptr<ModGameVersion> & modGameVersion : m_gameVersions) {
		gameVersionIdentifiers.push_back(modGameVersion->getGameVersionID());
	}

	return gameVersionIdentifiers;
}

std::vector<std::string> ModVersionType::getModGameVersionLongNames(const GameVersionCollection & gameVersions) const {
	std::vector<std::string> gameVersionLongNames;
	gameVersionLongNames.reserve(m_gameVersions.size());

	for(const std::shared_ptr<ModGameVersion> & modGameVersion : m_gameVersions) {
		if(modGameVersion->isStandAlone()) {
			gameVersionLongNames.push_back(modGameVersion->getGameVersionID());
		}
		else {
			gameVersionLongNames.push_back(gameVersions.getLongNameOfGameVersionWithID(modGameVersion->getGameVersionID()));
		}
	}

	return gameVersionLongNames;
}

std::vector<std::string> ModVersionType::getModGameVersionShortNames(const GameVersionCollection & gameVersions) const {
	std::vector<std::string> gameVersionShortNames;
	gameVersionShortNames.reserve(m_gameVersions.size());

	for(const std::shared_ptr<ModGameVersion> & modGameVersion : m_gameVersions) {
		if(modGameVersion->isStandAlone()) {
			gameVersionShortNames.push_back(modGameVersion->getGameVersionID());
		}
		else {
			gameVersionShortNames.push_back(gameVersions.getShortNameOfGameVersionWithID(modGameVersion->getGameVersionID()));
		}
	}

	return gameVersionShortNames;
}

std::vector<std::string> ModVersionType::getCompatibleModGameVersionIdentifiers(const GameVersion & gameVersion) const {
	std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(getCompatibleModGameVersions(gameVersion));
	std::vector<std::string> compatibleModGameVersionIdentifiers;
	compatibleModGameVersionIdentifiers.reserve(compatibleModGameVersions.size());

	for(const std::shared_ptr<ModGameVersion> & modGameVersion : compatibleModGameVersions) {
		compatibleModGameVersionIdentifiers.push_back(modGameVersion->getGameVersionID());
	}

	return compatibleModGameVersionIdentifiers;
}

std::vector<std::string> ModVersionType::getCompatibleModGameVersionLongNames(const GameVersion & gameVersion, const GameVersionCollection & gameVersions) const {
	std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(getCompatibleModGameVersions(gameVersion));
	std::vector<std::string> compatibleModGameVersionLongNames;
	compatibleModGameVersionLongNames.reserve(compatibleModGameVersions.size());

	for(const std::shared_ptr<ModGameVersion> & modGameVersion : compatibleModGameVersions) {
		compatibleModGameVersionLongNames.push_back(gameVersions.getLongNameOfGameVersionWithID(modGameVersion->getGameVersionID()));
	}

	return compatibleModGameVersionLongNames;
}

std::vector<std::string> ModVersionType::getCompatibleModGameVersionShortNames(const GameVersion & gameVersion, const GameVersionCollection & gameVersions) const {
	std::vector<std::shared_ptr<ModGameVersion>> compatibleModGameVersions(getCompatibleModGameVersions(gameVersion));
	std::vector<std::string> compatibleModGameVersionShortNames;
	compatibleModGameVersionShortNames.reserve(compatibleModGameVersions.size());

	for(const std::shared_ptr<ModGameVersion> & modGameVersion : compatibleModGameVersions) {
		compatibleModGameVersionShortNames.push_back(gameVersions.getShortNameOfGameVersionWithID(modGameVersion->getGameVersionID()));
	}

	return compatibleModGameVersionShortNames;
}

bool ModVersionType::isValid(bool skipFileInfoValidation) const {
	if(m_gameVersions.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator i = m_gameVersions.begin(); i != m_gameVersions.end(); ++i) {
		if(!(*i)->isValid(skipFileInfoValidation)) {
			return false;
		}

		if((*i)->getParentModVersionType() != this) {
			return false;
		}

		for(std::vector<std::shared_ptr<ModGameVersion>>::const_iterator j = i + 1; j != m_gameVersions.end(); ++j) {
			if(Utilities::areStringsEqualIgnoreCase((*i)->getGameVersionID(), (*j)->getGameVersionID())) {
				return false;
			}
		}
	}

	if(isStandAlone() && m_gameVersions.size() != 1) {
		return false;
	}

	return true;
}

bool ModVersionType::isValid(const ModVersionType * t, bool skipFileInfoValidation) {
	return t != nullptr && t->isValid(skipFileInfoValidation);
}

bool ModVersionType::operator == (const ModVersionType & t) const {
	if(!Utilities::areStringsEqualIgnoreCase(m_type, t.m_type)) {
		return false;
	}

	if(m_gameVersions.size() != t.m_gameVersions.size()) {
		return false;
	}

	for(size_t i = 0; i < m_gameVersions.size(); i++) {
		if(!Utilities::areStringsEqualIgnoreCase(m_gameVersions[i]->getGameVersionID(), t.m_gameVersions[i]->getGameVersionID())) {
			return false;
		}
	}

	return true;
}

bool ModVersionType::operator != (const ModVersionType & t) const {
	return !operator == (t);
}
