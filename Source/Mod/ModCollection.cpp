#include "ModCollection.h"

#include "Game/GameVersion.h"
#include "Game/GameVersionCollection.h"
#include "Mod.h"
#include "ModDependency.h"
#include "ModDownload.h"
#include "ModFile.h"
#include "ModGameVersion.h"
#include "ModTeam.h"
#include "ModTeamMember.h"
#include "ModVersion.h"
#include "ModVersionType.h"
#include "StandAloneMod.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/TinyXML2Utilities.h>
#include <Utilities/Utilities.h>

#include <fmt/core.h>
#include <magic_enum.hpp>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <filesystem>
#include <fstream>

static const std::string XML_MODS_ELEMENT_NAME("mods");

static const std::string XML_GAME_ID_ATTRIBUTE_NAME("game_id");
static const std::string XML_FILE_TYPE_ATTRIBUTE_NAME("file_type");
static const std::string XML_FILE_FORMAT_VERSION_ATTRIBUTE_NAME("file_format_version");

static constexpr const char * JSON_MODS_PROPERTY_NAME("mods");
static constexpr const char * JSON_GAME_ID_PROPERTY_NAME("gameID");
static constexpr const char * JSON_FILE_TYPE_PROPERTY_NAME("fileType");
static constexpr const char * JSON_FILE_FORMAT_VERSION_PROPERTY_NAME("fileFormatVersion");

const std::string ModCollection::GAME_ID("duke_nukem_3d");
const std::string ModCollection::FILE_TYPE("Mod List");
const std::string ModCollection::FILE_FORMAT_VERSION("1.0.0");

ModCollection::ModCollection() { }

ModCollection::ModCollection(ModCollection && m) noexcept
	: m_mods(std::move(m.m_mods)) { }

ModCollection::ModCollection(const ModCollection & m) {
	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m.m_mods.begin(); i != m.m_mods.end(); ++i) {
		m_mods.push_back(std::make_shared<Mod>(**i));
	}
}

ModCollection & ModCollection::operator = (ModCollection && m) noexcept {
	if(this != &m) {
		m_mods = std::move(m.m_mods);
	}

	updated(*this);

	return *this;
}

ModCollection & ModCollection::operator = (const ModCollection & m) {
	m_mods.clear();

	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m.m_mods.begin(); i != m.m_mods.end(); ++i) {
		m_mods.push_back(std::make_shared<Mod>(**i));
	}

	updated(*this);

	return *this;
}

ModCollection::~ModCollection() = default;

size_t ModCollection::numberOfMods() const {
	return m_mods.size();
}

bool ModCollection::hasMod(const Mod & mod) const {
	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_mods.begin(); i != m_mods.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getID(), mod.getID())) {
			return true;
		}
	}

	return false;
}

bool ModCollection::hasModWithID(const std::string & id) const {
	return indexOfModWithID(id) != std::numeric_limits<size_t>::max();
}

bool ModCollection::hasModWithName(const std::string & name) const {
	return indexOfModWithName(name) != std::numeric_limits<size_t>::max();
}

size_t ModCollection::indexOfMod(const Mod & mod) const {
	for(size_t i = 0; i < m_mods.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_mods[i]->getID(), mod.getID())) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t ModCollection::indexOfModWithID(const std::string & id) const {
	if(id.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_mods.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_mods[i]->getID(), id)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t ModCollection::indexOfModWithName(const std::string & name) const {
	if(name.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_mods.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_mods[i]->getName(), name)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<Mod> ModCollection::getMod(size_t index) const {
	if(index >= m_mods.size()) { return nullptr; }

	return m_mods[index];
}

std::shared_ptr<Mod> ModCollection::getModWithID(const std::string & id) const {
	if(id.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_mods.begin(); i != m_mods.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getID(), id)) {
			return *i;
		}
	}

	return nullptr;
}

std::shared_ptr<ModVersion> ModCollection::getModVersionWithModID(const std::string & id, const std::string & version) const {
	std::shared_ptr<Mod> mod(getModWithID(id));

	if(mod == nullptr) {
		return nullptr;
	}

	return mod->getVersion(version);
}

std::shared_ptr<ModVersionType> ModCollection::getModVersionTypeWithModID(const std::string & id, const std::string & version, const std::string & versionType) const {
	std::shared_ptr<ModVersion> modVersion(getModVersionWithModID(id, version));

	if(modVersion == nullptr) {
		return nullptr;
	}

	return modVersion->getType(versionType);
}

std::shared_ptr<ModGameVersion> ModCollection::getModGameVersionByIDWithModID(const std::string & id, const std::string & version, const std::string & versionType, const std::string & gameVersionID) const {
	std::shared_ptr<ModVersionType> modVersionType(getModVersionTypeWithModID(id, version, versionType));

	if(modVersionType == nullptr) {
		return nullptr;
	}

	return modVersionType->getGameVersionWithID(gameVersionID);
}

std::shared_ptr<Mod> ModCollection::getModWithName(const std::string & name) const {
	if(name.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_mods.begin(); i != m_mods.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			return *i;
		}
	}

	return nullptr;
}

std::shared_ptr<ModVersionType> ModCollection::getModVersionTypeFromDependency(const ModDependency & modDependency) const {
	if(!modDependency.isValid()) {
		return nullptr;
	}

	return getModVersionTypeWithModID(modDependency.getID(), modDependency.getVersion(), modDependency.getVersionType());
}

std::shared_ptr<ModVersion> ModCollection::getStandAloneModVersion(const StandAloneMod & standAloneMod) const {
	if(!isValid(nullptr, true) || !standAloneMod.isValid()) {
		return nullptr;
	}

	for(const std::shared_ptr<Mod> & mod : m_mods) {
		if(Utilities::areStringsEqualIgnoreCase(mod->getID(), standAloneMod.getID())) {
			return mod->getVersion(standAloneMod.getVersion());
		}
	}

	return nullptr;
}

const std::vector<std::shared_ptr<Mod>> & ModCollection::getMods() const {
	return m_mods;
}

bool ModCollection::addMod(const Mod & mod) {
	if(!mod.isValid() || hasMod(mod)) {
		return false;
	}

	m_mods.push_back(std::make_shared<Mod>(mod));

	updated(*this);

	return true;
}

bool ModCollection::removeMod(size_t index) {
	if(index >= m_mods.size()) {
		return false;
	}

	m_mods.erase(m_mods.begin() + index);

	updated(*this);

	return true;
}

bool ModCollection::removeMod(const Mod & mod) {
	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_mods.begin(); i != m_mods.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getID(), mod.getID())) {
			m_mods.erase(i);

			updated(*this);

			return true;
		}
	}

	return false;
}

bool ModCollection::removeModWithID(const std::string & id) {
	if(id.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_mods.begin(); i != m_mods.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getID(), id)) {
			m_mods.erase(i);

			updated(*this);

			return true;
		}
	}

	return false;
}

bool ModCollection::removeModWithName(const std::string & name) {
	if(name.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_mods.begin(); i != m_mods.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			m_mods.erase(i);

			updated(*this);

			return true;
		}
	}

	return false;
}

void ModCollection::clearMods() {
	m_mods.clear();

	updated(*this);
}

bool ModCollection::copyHiddenPropertiesFrom(const ModCollection & modCollection) {
	if(!isValid(nullptr, true) || !modCollection.isValid(nullptr, true) || m_mods.size() != modCollection.m_mods.size()) {
		return false;
	}

	for(size_t i = 0; i < m_mods.size(); i++) {
		if(!m_mods[i]->copyHiddenPropertiesFrom(*modCollection.m_mods[i])) {
			return false;
		}
	}

	return true;
}

std::shared_ptr<ModVersionType> ModCollection::getModDependencyVersionType(const ModDependency & modDependency) const {
	if(!modDependency.isValid()) {
		return nullptr;
	}

	std::shared_ptr<Mod> mod(getModWithID(modDependency.getID()));

	if(mod == nullptr) {
		return nullptr;
	}

	std::shared_ptr<ModVersion> modVersion(mod->getVersion(modDependency.getVersion()));

	if(modVersion == nullptr) {
		return nullptr;
	}

	std::shared_ptr<ModVersionType> modVersionType(modVersion->getType(modDependency.getVersionType()));

	if(modVersionType == nullptr) {
		return nullptr;
	}

	return modVersionType;
}

std::vector<std::shared_ptr<ModVersionType>> ModCollection::getModDependencyVersionTypes(const std::vector<std::shared_ptr<ModDependency>> & modDependencies) const {
	std::vector<std::shared_ptr<ModVersionType>> modDependencyVersionTypes;
	std::shared_ptr<ModVersionType> modDependencyVersionType;

	for(const std::shared_ptr<ModDependency> & modDependency : modDependencies) {
		modDependencyVersionType = getModDependencyVersionType(*modDependency);

		if(modDependencyVersionType == nullptr) {
			spdlog::error("No mod dependency with ID: '{}', version: '{}', and version type: '{}' found.", modDependency->getID(), modDependency->getVersion(), modDependency->getVersionType());

			return {};
		}

		modDependencyVersionTypes.push_back(modDependencyVersionType);
	}

	return modDependencyVersionTypes;
}

std::vector<std::shared_ptr<ModVersionType>> ModCollection::getModDependencyVersionTypes(const ModVersionType & modVersionType) const {
	if(!modVersionType.isValid(true)) {
		return {};
	}

	return getModDependencyVersionTypes(modVersionType.getDependencies());
}

std::vector<std::shared_ptr<ModVersionType>> ModCollection::getModDependencyVersionTypes(const ModGameVersion & modGameVersion) const {
	if(!modGameVersion.isValid(true)) {
		return {};
	}

	return getModDependencyVersionTypes(modGameVersion.getParentModVersionType()->getDependencies());
}

std::shared_ptr<ModGameVersion> ModCollection::getModDependencyGameVersion(const ModDependency & modDependency, const std::string & gameVersionID, const GameVersionCollection * gameVersions, bool allowCompatibleGameVersions) const {
	if(gameVersionID.empty()) {
		return nullptr;
	}

	std::shared_ptr<ModVersionType> modVersionType(getModDependencyVersionType(modDependency));

	if(modVersionType == nullptr) {
		return nullptr;
	}

	std::shared_ptr<ModGameVersion> modGameVersion(modVersionType->getGameVersionWithID(gameVersionID));

	if(modGameVersion != nullptr) {
		return modGameVersion;
	}

	if(!allowCompatibleGameVersions) {
		return nullptr;
	}

	if(!GameVersionCollection::isValid(gameVersions)) {
		spdlog::error("Failed to obtain compatible mod dependency mod game verison with invalid game version collection.");
		return nullptr;
	}

	std::shared_ptr<GameVersion> gameVersion(gameVersions->getGameVersionWithID(gameVersionID));

	if(gameVersion == nullptr) {
		spdlog::error("Failed to obtain compatible mod dependency mod game versions for non-existent game version with ID: '{}'.", gameVersionID);
		return nullptr;
	}

	std::vector<std::shared_ptr<GameVersion>> compatibleGameVersions(gameVersion->getCompatibleGameVersions(*gameVersions));
	std::shared_ptr<ModGameVersion> compatibleModGameVersion;

	for(std::vector<std::shared_ptr<GameVersion>>::const_reverse_iterator i = compatibleGameVersions.crbegin(); i != compatibleGameVersions.crend(); ++i) {
		compatibleModGameVersion = modVersionType->getGameVersionWithID((*i)->getID());

		if(compatibleModGameVersion == nullptr) {
			continue;
		}

		return compatibleModGameVersion;
	}

	return nullptr;
}

std::vector<std::shared_ptr<ModGameVersion>> ModCollection::getModDependencyGameVersions(const std::vector<std::shared_ptr<ModDependency>> & modDependencies, const std::string & gameVersionID, const GameVersionCollection * gameVersions, bool allowCompatibleGameVersions) const {
	std::vector<std::shared_ptr<ModGameVersion>> modDependencyGameVersions;
	std::shared_ptr<ModGameVersion> modDependencyGameVersion;

	for(const std::shared_ptr<ModDependency> & modDependency : modDependencies) {
		modDependencyGameVersion = getModDependencyGameVersion(*modDependency, gameVersionID, gameVersions, allowCompatibleGameVersions);

		if(modDependencyGameVersion == nullptr) {
			spdlog::error("No mod dependency with ID: '{}', version: '{}', and version type: '{}' found for game version with ID: '{}'.", modDependency->getID(), modDependency->getVersion(), modDependency->getVersionType(), gameVersionID);

			return {};
		}

		modDependencyGameVersions.push_back(modDependencyGameVersion);
	}

	return modDependencyGameVersions;
}

std::vector<std::shared_ptr<ModGameVersion>> ModCollection::getModDependencyGameVersions(const ModGameVersion & modGameVersion, const GameVersionCollection * gameVersions, bool allowCompatibleGameVersions) const {
	if(!modGameVersion.isValid(true)) {
		return {};
	}

	return getModDependencyGameVersions(modGameVersion.getParentModVersionType()->getDependencies(), modGameVersion.getGameVersionID(), gameVersions, allowCompatibleGameVersions);
}

std::shared_ptr<ModDownload> ModCollection::getModDependencyDownload(const ModDependency & modDependency, const std::string & gameVersionID, const GameVersionCollection * gameVersions, bool allowCompatibleGameVersions) const {
	std::shared_ptr<ModGameVersion> modDependencyModGameVersion(getModDependencyGameVersion(modDependency, gameVersionID, gameVersions, allowCompatibleGameVersions));

	if(modDependencyModGameVersion == nullptr) {
		return nullptr;
	}

	return modDependencyModGameVersion->getDownload();
}

std::vector<std::shared_ptr<ModDownload>> ModCollection::getModDependencyDownloads(const std::vector<std::shared_ptr<ModDependency>> & modDependencies, const std::string & gameVersionID, const GameVersionCollection * gameVersions, bool allowCompatibleGameVersions) const {
	std::vector<std::shared_ptr<ModDownload>> modDependencyDownloads;
	std::shared_ptr<ModDownload> modDependencyDownload;

	for(const std::shared_ptr<ModDependency> & modDependency : modDependencies) {
		modDependencyDownload = getModDependencyDownload(*modDependency, gameVersionID, gameVersions, allowCompatibleGameVersions);

		if(modDependencyDownload == nullptr) {
			spdlog::error("No mod dependency download with ID: '{}', version: '{}', and version type: '{}' found for game version with ID: '{}'.", modDependency->getID(), modDependency->getVersion(), modDependency->getVersionType(), gameVersionID);

			return {};
		}

		modDependencyDownloads.push_back(modDependencyDownload);
	}

	return modDependencyDownloads;
}

std::vector<std::shared_ptr<ModDownload>> ModCollection::getModDependencyDownloads(const ModGameVersion & modGameVersion, const GameVersionCollection * gameVersions, bool allowCompatibleGameVersions) const {
	if(!modGameVersion.isValid(true)) {
		return {};
	}

	return getModDependencyDownloads(modGameVersion.getParentModVersionType()->getDependencies(), modGameVersion.getGameVersionID(), gameVersions, allowCompatibleGameVersions);
}

std::vector<std::shared_ptr<ModFile>> ModCollection::getModDependencyGroupFiles(const ModDependency & modDependency, const GameVersion & gameVersion, const GameVersionCollection * gameVersions, bool allowCompatibleGameVersions, bool recursive) const {
	std::shared_ptr<ModGameVersion> modDependencyModGameVersion(getModDependencyGameVersion(modDependency, gameVersion.getID(), gameVersions, allowCompatibleGameVersions));

	if(modDependencyModGameVersion == nullptr) {
		return {};
	}

	std::vector<std::shared_ptr<ModFile>> groupModFiles;

	if(recursive) {
		std::shared_ptr<ModGameVersion> modDependencyModGameVersion(getModDependencyGameVersion(modDependency, gameVersion.getID(), gameVersions, allowCompatibleGameVersions));

		if(modDependencyModGameVersion == nullptr) {
			return {};
		}

		std::vector<std::shared_ptr<ModFile>> currentModDependencyGroupModFiles(getModDependencyGroupFiles(modDependencyModGameVersion->getParentModVersionType()->getDependencies(), gameVersion, gameVersions, allowCompatibleGameVersions, true));

		for(const std::shared_ptr<ModFile> & currentModDependencyGroupModFile : currentModDependencyGroupModFiles) {
			groupModFiles.push_back(currentModDependencyGroupModFile);
		}
	}

	for(const std::shared_ptr<ModFile> & modFile : modDependencyModGameVersion->getFiles()) {
		if(modFile->hasFileExtension("grp")) {
			groupModFiles.push_back(modFile);
		}
		else if(gameVersion.areZipArchiveGroupsSupported() && modFile->hasFileExtension("zip")) {
			groupModFiles.push_back(modFile);
		}
	}

	return groupModFiles;
}

std::vector<std::shared_ptr<ModFile>> ModCollection::getModDependencyGroupFiles(const std::vector<std::shared_ptr<ModDependency>> & modDependencies, const GameVersion & gameVersion, const GameVersionCollection * gameVersions, bool allowCompatibleGameVersions, bool recursive) const {
	std::vector<std::shared_ptr<ModFile>> allModDependencyGroupModFiles;
	std::vector<std::shared_ptr<ModFile>> currentModDependencyGroupModFiles;

	if(recursive) {
		for(const std::shared_ptr<ModDependency> & modDependency : modDependencies) {
			std::shared_ptr<ModGameVersion> modDependencyModGameVersion(getModDependencyGameVersion(*modDependency, gameVersion.getID(), gameVersions, allowCompatibleGameVersions));

			if(modDependencyModGameVersion == nullptr) {
				return {};
			}

			currentModDependencyGroupModFiles = getModDependencyGroupFiles(modDependencyModGameVersion->getParentModVersionType()->getDependencies(), gameVersion, gameVersions, allowCompatibleGameVersions, true);

			for(const std::shared_ptr<ModFile> & currentModDependencyGroupModFile : currentModDependencyGroupModFiles) {
				allModDependencyGroupModFiles.push_back(currentModDependencyGroupModFile);
			}
		}
	}

	for(const std::shared_ptr<ModDependency> & modDependency : modDependencies) {
		currentModDependencyGroupModFiles = getModDependencyGroupFiles(*modDependency, gameVersion, gameVersions, allowCompatibleGameVersions, false);

		for(const std::shared_ptr<ModFile> & currentModDependencyGroupModFile : currentModDependencyGroupModFiles) {
			allModDependencyGroupModFiles.push_back(currentModDependencyGroupModFile);
		}
	}

	return allModDependencyGroupModFiles;
}

std::vector<std::shared_ptr<ModFile>> ModCollection::getModDependencyGroupFiles(const ModGameVersion & modGameVersion, const GameVersion & gameVersion, const GameVersionCollection * gameVersions, bool allowCompatibleGameVersions, bool recursive) const {
	if(!modGameVersion.isValid(true) || !gameVersion.isValid() || !Utilities::areStringsEqualIgnoreCase(modGameVersion.getGameVersionID(), gameVersion.getID())) {
		return {};
	}

	return getModDependencyGroupFiles(modGameVersion.getParentModVersionType()->getDependencies(), gameVersion, gameVersions, allowCompatibleGameVersions, recursive);
}

rapidjson::Document ModCollection::toJSON() const {
	rapidjson::Document modsDocument(rapidjson::kObjectType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = modsDocument.GetAllocator();

	rapidjson::Value gameIDValue(GAME_ID.c_str(), allocator);
	modsDocument.AddMember(rapidjson::StringRef(JSON_GAME_ID_PROPERTY_NAME), gameIDValue, allocator);

	rapidjson::Value fileTypeVersionValue(FILE_TYPE.c_str(), allocator);
	modsDocument.AddMember(rapidjson::StringRef(JSON_FILE_TYPE_PROPERTY_NAME), fileTypeVersionValue, allocator);

	rapidjson::Value fileFormatVersionValue(FILE_FORMAT_VERSION.c_str(), allocator);
	modsDocument.AddMember(rapidjson::StringRef(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME), fileFormatVersionValue, allocator);

	rapidjson::Value modsValue(rapidjson::kArrayType);
	modsValue.Reserve(m_mods.size(), allocator);

	for(const std::shared_ptr<Mod> & mod : m_mods) {
		modsValue.PushBack(mod->toJSON(allocator), allocator);
	}

	modsDocument.AddMember(rapidjson::StringRef(JSON_MODS_PROPERTY_NAME), modsValue, allocator);

	return modsDocument;
}

tinyxml2::XMLElement * ModCollection::toXML(tinyxml2::XMLDocument * document) const {
	if(document == nullptr) {
		return nullptr;
	}

	tinyxml2::XMLElement * modsElement = document->NewElement(XML_MODS_ELEMENT_NAME.c_str());

	modsElement->SetAttribute(XML_GAME_ID_ATTRIBUTE_NAME.c_str(), GAME_ID.c_str());
	modsElement->SetAttribute(XML_FILE_TYPE_ATTRIBUTE_NAME.c_str(), FILE_TYPE.c_str());
	modsElement->SetAttribute(XML_FILE_FORMAT_VERSION_ATTRIBUTE_NAME.c_str(), FILE_FORMAT_VERSION.c_str());

	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_mods.begin(); i != m_mods.end(); ++i) {
		modsElement->InsertEndChild((*i)->toXML(document));
	}

	return modsElement;
}

std::unique_ptr<ModCollection> ModCollection::parseFrom(const rapidjson::Value & modCollectionValue, bool skipFileInfoValidation) {
	if(!modCollectionValue.IsObject()) {
		spdlog::error("Invalid mod collection type: '{}', expected 'object'.", Utilities::typeToString(modCollectionValue.GetType()));
		return nullptr;
	}

	// verify the file type
	if(modCollectionValue.HasMember(JSON_FILE_TYPE_PROPERTY_NAME)) {
		const rapidjson::Value & fileTypeValue = modCollectionValue[JSON_FILE_TYPE_PROPERTY_NAME];

		if(!fileTypeValue.IsString()) {
			spdlog::error("Invalid mod collection file type type: '{}', expected: 'string'.", Utilities::typeToString(fileTypeValue.GetType()));
			return nullptr;
		}

		if(!Utilities::areStringsEqualIgnoreCase(fileTypeValue.GetString(), FILE_TYPE)) {
			spdlog::error("Incorrect mod collection file type: '{}', expected: '{}'.", fileTypeValue.GetString(), FILE_TYPE);
			return nullptr;
		}
	}
	else {
		spdlog::warn("Mod collection JSON data is missing file type, and may fail to load correctly!");
	}

	// verify file format version
	if(modCollectionValue.HasMember(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & fileFormatVersionValue = modCollectionValue[JSON_FILE_FORMAT_VERSION_PROPERTY_NAME];

		if(!fileFormatVersionValue.IsString()) {
			spdlog::error("Invalid mod collection file format version type: '{}', expected: 'string'.", Utilities::typeToString(fileFormatVersionValue.GetType()));
			return nullptr;
		}

		std::optional<std::uint8_t> optionalVersionComparison(Utilities::compareVersions(fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION));

		if(!optionalVersionComparison.has_value()) {
			spdlog::error("Invalid mod collection file format version: '{}'.", fileFormatVersionValue.GetString());
			return nullptr;
		}

		if(*optionalVersionComparison != 0) {
			spdlog::error("Unsupported mod collection file format version: '{}', only version '{}' is supported.", fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION);
			return nullptr;
		}
	}
	else {
		spdlog::warn("Mod collection JSON data is missing file format version, and may fail to load correctly!");
	}

	// verify game identifier
	if(!modCollectionValue.HasMember(JSON_GAME_ID_PROPERTY_NAME)) {
		spdlog::error("Mod collection is missing '{}' property.", JSON_GAME_ID_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & gameIDValue = modCollectionValue[JSON_GAME_ID_PROPERTY_NAME];

	if(!gameIDValue.IsString()) {
		spdlog::error("Mod collection has an invalid '{}' property type: '{}', expected 'string'.", JSON_GAME_ID_PROPERTY_NAME, Utilities::typeToString(gameIDValue.GetType()));
		return nullptr;
	}

	std::string gameID(Utilities::trimString(gameIDValue.GetString()));

	if(!Utilities::areStringsEqual(gameID, GAME_ID)) {
		spdlog::error("Unsupported mod collection game identifier '{}', expected '{}'.", gameID, GAME_ID);
		return nullptr;
	}

	// parse mods list
	if(!modCollectionValue.HasMember(JSON_MODS_PROPERTY_NAME)) {
		spdlog::error("Mod collection is missing '{}' property.", JSON_MODS_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modsValue = modCollectionValue[JSON_MODS_PROPERTY_NAME];

	if(!modsValue.IsArray()) {
		spdlog::error("Invalid mod collection '{}' type: '{}', expected 'array'.", JSON_MODS_PROPERTY_NAME, Utilities::typeToString(modsValue.GetType()));
		return nullptr;
	}

	std::unique_ptr<ModCollection> newModCollection(std::make_unique<ModCollection>());

	if(modsValue.Empty()) {
		return newModCollection;
	}

	std::unique_ptr<Mod> newMod;

	for(rapidjson::Value::ConstValueIterator i = modsValue.Begin(); i != modsValue.End(); ++i) {
		newMod = Mod::parseFrom(*i, skipFileInfoValidation);

		if(!Mod::isValid(newMod.get(), skipFileInfoValidation)) {
			spdlog::error("Failed to parse mod #{}{}!", newModCollection->m_mods.size() + 1, newModCollection->numberOfMods() == 0 ? "" : fmt::format(" (after mod with ID '{}')", newModCollection->getMod(newModCollection->numberOfMods() - 1)->getID()));
			return nullptr;
		}

		if(newModCollection->hasMod(*newMod)) {
			spdlog::error("Encountered duplicate mod #{}{}.", newModCollection->m_mods.size() + 1, newModCollection->numberOfMods() == 0 ? "" : fmt::format(" (after mod with ID '{}')", newModCollection->getMod(newModCollection->numberOfMods() - 1)->getID()));
			return nullptr;
		}

		newModCollection->m_mods.emplace_back(std::move(newMod));
	}

	return newModCollection;
}

std::unique_ptr<ModCollection> ModCollection::parseFrom(const tinyxml2::XMLElement * modsElement, bool skipFileInfoValidation) {
	// verify the mods element
	if(modsElement == nullptr) {
		spdlog::error("Missing '{}' element!", XML_MODS_ELEMENT_NAME);
		return nullptr;
	}

	if(modsElement->Name() != XML_MODS_ELEMENT_NAME) {
		spdlog::error("Invalid element name '{}', expected '{}'!", modsElement->Name(), XML_MODS_ELEMENT_NAME);
		return nullptr;
	}

	// verify the file type
	const char * fileType = modsElement->Attribute(XML_FILE_TYPE_ATTRIBUTE_NAME.c_str());

	if(Utilities::stringLength(fileType) != 0) {
		if(!Utilities::areStringsEqualIgnoreCase(fileType, FILE_TYPE)) {
			spdlog::error("Incorrect XML mod collection file type: '{}', expected: '{}'.", fileType, FILE_TYPE);
			return nullptr;
		}
	}
	else {
		spdlog::warn("Mod collection XML data element '{}' is missing file type attribute '{}', and may fail to load correctly!", XML_MODS_ELEMENT_NAME, XML_FILE_TYPE_ATTRIBUTE_NAME);
	}

	// verify file format version
	const char * fileFormatVersion = modsElement->Attribute(XML_FILE_FORMAT_VERSION_ATTRIBUTE_NAME.c_str());

	if(Utilities::stringLength(fileFormatVersion) != 0) {
		std::optional<std::uint8_t> optionalVersionComparison(Utilities::compareVersions(fileFormatVersion, FILE_FORMAT_VERSION));

		if(!optionalVersionComparison.has_value()) {
			spdlog::error("Invalid mod collection file format version: '{}'.", fileFormatVersion);
			return nullptr;
		}

		if(*optionalVersionComparison != 0) {
			spdlog::error("Unsupported mod collection file format version: '{}', only version '{}' is supported.", fileFormatVersion, FILE_FORMAT_VERSION);
			return nullptr;
		}
	}
	else {
		spdlog::warn("Mod collection XML data element '{}' is missing file format version attribute '{}', and may fail to load correctly!", XML_MODS_ELEMENT_NAME, XML_FILE_FORMAT_VERSION_ATTRIBUTE_NAME);
	}

	// verify game identifier
	const char * gameID = modsElement->Attribute(XML_GAME_ID_ATTRIBUTE_NAME.c_str());

	if(Utilities::stringLength(gameID) == 0) {
		spdlog::error("Attribute '{}' is missing from '{}' element.", XML_GAME_ID_ATTRIBUTE_NAME, XML_MODS_ELEMENT_NAME);
		return nullptr;
	}

	if(!Utilities::areStringsEqual(gameID, GAME_ID)) {
		spdlog::error("Unsupported mod collection game identifier '{}', expected '{}'.", gameID, GAME_ID);
		return nullptr;
	}

	// find the first mod element within the mods element
	const tinyxml2::XMLElement * modElement = modsElement->FirstChildElement();

	std::unique_ptr<ModCollection> modCollection(std::make_unique<ModCollection>());
	std::unique_ptr<Mod> newMod;

	// iterate over all of the mod elements
	while(true) {
		if(modElement == nullptr) {
			break;
		}

		newMod = Mod::parseFrom(modElement, skipFileInfoValidation);

		if(!Mod::isValid(newMod.get(), skipFileInfoValidation)) {
			spdlog::error("Failed to parse mod #{}{}!", modCollection->m_mods.size() + 1, modCollection->numberOfMods() == 0 ? "" : fmt::format(" (after mod with ID '{}')", modCollection->getMod(modCollection->numberOfMods() - 1)->getID()));
			return nullptr;
		}

		if(modCollection->hasMod(*newMod)) {
			spdlog::error("Encountered duplicate mod #{}{}.", modCollection->m_mods.size() + 1, modCollection->numberOfMods() == 0 ? "" : fmt::format(" (after mod with ID '{}')", modCollection->getMod(modCollection->numberOfMods() - 1)->getID()));
			return nullptr;
		}

		modCollection->m_mods.emplace_back(std::move(newMod));

		modElement = modElement->NextSiblingElement();
	}

	return modCollection;
}

bool ModCollection::loadFrom(const std::string & filePath, const GameVersionCollection * gameVersions, bool skipFileInfoValidation) {
	if(filePath.empty()) {
		return false;
	}

	std::string fileExtension(Utilities::getFileExtension(filePath));

	if(fileExtension.empty()) {
		return false;
	}
	else if(Utilities::areStringsEqualIgnoreCase(fileExtension, "xml")) {
		return loadFromXML(filePath, gameVersions, skipFileInfoValidation);
	}
	else if(Utilities::areStringsEqualIgnoreCase(fileExtension, "json")) {
		return loadFromJSON(filePath, gameVersions, skipFileInfoValidation);
	}

	return false;
}

bool ModCollection::loadFromXML(const std::string & filePath, const GameVersionCollection * gameVersions, bool skipFileInfoValidation) {
	if(filePath.empty() || !std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		return false;
	}

	tinyxml2::XMLDocument modCollectionDocument;
	tinyxml2::XMLError result = modCollectionDocument.LoadFile(filePath.c_str());

	if(result != tinyxml2::XML_SUCCESS) {
		spdlog::error("Failed to load mod collection from XML file '{}' with error code: '{}', and message: '{}'.", filePath, magic_enum::enum_name(result), modCollectionDocument.ErrorStr());
		return false;
	}

	m_mods.clear();

	std::unique_ptr<ModCollection> modCollection(parseFrom(modCollectionDocument.RootElement(), skipFileInfoValidation));

	if(!ModCollection::isValid(modCollection.get(), gameVersions, skipFileInfoValidation)) {
		spdlog::error("Failed to parse mod collection from XML file '{}'.", filePath);
		return false;
	}

	m_mods = std::move(modCollection->m_mods);

	updated(*this);

	return true;
}

bool ModCollection::loadFromJSON(const std::string & filePath, const GameVersionCollection * gameVersions, bool skipFileInfoValidation) {
	if(filePath.empty() || !std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		return false;
	}

	std::ifstream fileStream(filePath);

	if(!fileStream.is_open()) {
		return false;
	}

	rapidjson::Document modsValue;
	rapidjson::IStreamWrapper fileStreamWrapper(fileStream);
	if(modsValue.ParseStream(fileStreamWrapper).HasParseError()) {
		return false;
	}

	fileStream.close();

	std::unique_ptr<ModCollection> modCollection(parseFrom(modsValue, skipFileInfoValidation));

	if(!ModCollection::isValid(modCollection.get(), gameVersions, skipFileInfoValidation)) {
		spdlog::error("Failed to parse mod collection from JSON file '{}'.", filePath);
		return false;
	}

	m_mods = std::move(modCollection->m_mods);

	updated(*this);

	return true;
}

bool ModCollection::saveTo(const std::string & filePath, bool overwrite) const {
	if(filePath.empty()) {
		return false;
	}

	std::string fileExtension(Utilities::getFileExtension(filePath));

	if(fileExtension.empty()) {
		return false;
	}
	else if(Utilities::areStringsEqualIgnoreCase(fileExtension, "json")) {
		return saveToJSON(filePath, overwrite);
	}
	else if(Utilities::areStringsEqualIgnoreCase(fileExtension, "xml")) {
		return saveToXML(filePath, overwrite);
	}

	return false;
}

bool ModCollection::saveToXML(const std::string & filePath, bool overwrite) const {
	if(!overwrite && std::filesystem::exists(std::filesystem::path(filePath))) {
		spdlog::warn("File '{}' already exists, use overwrite to force write.", filePath);
		return false;
	}

	tinyxml2::XMLDocument modCollectionDocument;

	modCollectionDocument.InsertFirstChild(modCollectionDocument.NewDeclaration());

	modCollectionDocument.InsertEndChild(toXML(&modCollectionDocument));

	if(!Utilities::saveXMLDocumentToFile(&modCollectionDocument, filePath, overwrite)) {
		spdlog::error("Failed to save mod collection to XML file '{}'.", filePath);
		return false;
	}

	return true;
}

bool ModCollection::saveToJSON(const std::string & filePath, bool overwrite) const {
	if(!overwrite && std::filesystem::exists(std::filesystem::path(filePath))) {
		spdlog::warn("File '{}' already exists, use overwrite to force write.", filePath);
		return false;
	}

	std::ofstream fileStream(filePath);

	if(!fileStream.is_open()) {
		return false;
	}

	rapidjson::Document mods(toJSON());

	rapidjson::OStreamWrapper fileStreamWrapper(fileStream);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> fileStreamWriter(fileStreamWrapper);
	fileStreamWriter.SetIndent('\t', 1);
	mods.Accept(fileStreamWriter);
	fileStream.close();

	return true;
}

bool ModCollection::checkGameVersions(const GameVersionCollection & gameVersions, bool verbose) const {
	std::shared_ptr<Mod> mod;
	std::shared_ptr<ModVersion> modVersion;
	std::shared_ptr<ModVersionType> modVersionType;
	std::shared_ptr<ModGameVersion> modGameVersion;
	std::shared_ptr<ModDownload> modDownload;

	for(size_t i = 0; i < m_mods.size(); i++) {
		mod = m_mods[i];

		for(size_t j = 0; j < mod->numberOfVersions(); j++) {
			modVersion = mod->getVersion(j);

			for(size_t k = 0; k < modVersion->numberOfTypes(); k++) {
				modVersionType = modVersion->getType(k);

				for(size_t l = 0; l < modVersionType->numberOfGameVersions(); l++) {
					modGameVersion = modVersionType->getGameVersion(l);

					if(!modGameVersion->isStandAlone() && !gameVersions.hasGameVersionWithID(modGameVersion->getGameVersionID())) {
						if(verbose) {
							spdlog::warn("Mod '{}' contains unknown game version ID: '{}'.", mod->getFullName(j, k), modGameVersion->getGameVersionID());
						}

						return false;
					}
				}
			}
		}

		for(size_t j = 0; j < mod->numberOfDownloads(); j++) {
			modDownload = mod->getDownload(j);

			if(!modDownload->getGameVersionID().empty() &&
			   !modDownload->isStandAlone() &&
			   !Utilities::areStringsEqualIgnoreCase(modDownload->getGameVersionID(), GameVersion::ALL_VERSIONS) &&
			   !gameVersions.hasGameVersionWithID(modDownload->getGameVersionID())) {
				if(verbose) {
					spdlog::warn("Mod '{}' has download '{}' with unknown game version ID: '{}'.", mod->getID(), modDownload->getFileName(), modDownload->getGameVersionID());
				}

				return false;
			}
		}
	}

	return true;
}

bool ModCollection::isValid(const GameVersionCollection * gameVersions, bool skipFileInfoValidation) const {
	std::shared_ptr<ModVersion> modVersion;
	std::shared_ptr<ModVersionType> modVersionType;
	std::shared_ptr<ModGameVersion> modGameVersion;
	std::shared_ptr<ModDependency> modDepdency;

	for(const std::shared_ptr<Mod> & mod : m_mods) {
		if(!mod->isValid(skipFileInfoValidation)) {
			return false;
		}

		if(mod->hasAlias() && !hasModWithID(mod->getAlias())) {
			return false;
		}

		for(size_t i = 0; i < mod->numberOfRelatedMods(); i++) {
			if(!hasModWithID(mod->getRelatedMod(i))) {
				return false;
			}
		}

		for(size_t i = 0; i < mod->numberOfSimilarMods(); i++) {
			if(!hasModWithID(mod->getSimilarMod(i))) {
				return false;
			}
		}

		if(gameVersions != nullptr) {
			for(size_t i = 0; i < mod->numberOfVersions(); i++) {
				modVersion = mod->getVersion(i);

				for(size_t j = 0; j < modVersion->numberOfTypes(); j++) {
					modVersionType = modVersion->getType(j);

					for(size_t k = 0; k < modVersionType->numberOfDependencies(); k++) {
						modDepdency = modVersionType->getDependency(k);

						for(size_t l = 0; l < modVersionType->numberOfGameVersions(); l++) {
							modGameVersion = modVersionType->getGameVersion(l);

							if(getModDependencyGameVersion(*modDepdency, modGameVersion->getGameVersionID(), gameVersions, true) == nullptr) {
								spdlog::error("'{}' mod is missing dependency with ID: '{}', version: '{}', and version type: '{}' for game version with ID: '{}'.", modVersionType->getFullName(), modDepdency->getID(), modDepdency->getVersion(), modDepdency->getVersionType(), modGameVersion->getGameVersionID());
								return false;
							}
						}
					}
				}
			}
		}
	}

	return true;
}

bool ModCollection::isValid(const ModCollection * m, const GameVersionCollection * gameVersions, bool skipFileInfoValidation) {
	return m != nullptr && m->isValid(gameVersions, skipFileInfoValidation);
}

bool ModCollection::operator == (const ModCollection & m) const {
	if(m_mods.size() != m.m_mods.size()) {
		return false;
	}

	for(size_t i = 0; i < m.m_mods.size(); i++) {
		if(*m_mods[i] != *m.m_mods[i]) {
			return false;
		}
	}

	return true;
}

bool ModCollection::operator != (const ModCollection & m) const {
	return !operator == (m);
}
