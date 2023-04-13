#include "ModCollection.h"

#include "Game/GameVersion.h"
#include "Game/GameVersionCollection.h"
#include "Mod.h"
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
static const std::string XML_FILE_FORMAT_VERSION_ATTRIBUTE_NAME("file_format_version");

static constexpr const char * JSON_MODS_PROPERTY_NAME("mods");
static constexpr const char * JSON_GAME_ID_PROPERTY_NAME("gameID");
static constexpr const char * JSON_FILE_FORMAT_VERSION_PROPERTY_NAME("fileFormatVersion");

const std::string ModCollection::GAME_ID("duke_nukem_3d");
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

std::shared_ptr<ModVersion> ModCollection::getStandAloneModVersion(const StandAloneMod & standAloneMod) const {
	if(!isValid(true) || !standAloneMod.isValid()) {
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
	if(!isValid(true) || !modCollection.isValid(true) || m_mods.size() != modCollection.m_mods.size()) {
		return false;
	}

	for(size_t i = 0; i < m_mods.size(); i++) {
		if(!m_mods[i]->copyHiddenPropertiesFrom(*modCollection.m_mods[i])) {
			return false;
		}
	}

	return true;
}

rapidjson::Document ModCollection::toJSON() const {
	rapidjson::Document modsDocument(rapidjson::kObjectType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = modsDocument.GetAllocator();

	rapidjson::Value gameIDValue(GAME_ID.c_str(), allocator);
	modsDocument.AddMember(rapidjson::StringRef(JSON_GAME_ID_PROPERTY_NAME), gameIDValue, allocator);

	rapidjson::Value fileFormatVersionValue(FILE_FORMAT_VERSION.c_str(), allocator);
	modsDocument.AddMember(rapidjson::StringRef(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME), fileFormatVersionValue, allocator);

	rapidjson::Value modsValue(rapidjson::kArrayType);

	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_mods.begin(); i != m_mods.end(); ++i) {
		modsValue.PushBack((*i)->toJSON(allocator), allocator);
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

	// verify file format version
	if(modCollectionValue.HasMember(JSON_FILE_FORMAT_VERSION_PROPERTY_NAME)) {
		const rapidjson::Value & fileFormatVersionValue = modCollectionValue[JSON_FILE_FORMAT_VERSION_PROPERTY_NAME];

		if(!fileFormatVersionValue.IsString()) {
			spdlog::error("Invalid mod collection file format version type: '{}', expected: 'string'.", Utilities::typeToString(fileFormatVersionValue.GetType()));
			return false;
		}

		std::optional<std::uint8_t> optionalVersionComparison(Utilities::compareVersions(fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION));

		if(!optionalVersionComparison.has_value()) {
			spdlog::error("Invalid mod collection file format version: '{}'.", fileFormatVersionValue.GetString());
			return false;
		}

		if(*optionalVersionComparison != 0) {
			spdlog::error("Unsupported mod collection file format version: '{}', only version '{}' is supported.", fileFormatVersionValue.GetString(), FILE_FORMAT_VERSION);
			return false;
		}
	}
	else {
		spdlog::warn("Mod collection JSON data is missing file format version, and may fail to load correctly!");
	}

	// verify game identifier
	if(!modCollectionValue.HasMember(JSON_GAME_ID_PROPERTY_NAME)) {
		spdlog::error("Mod collection is missing '{}' property'.", JSON_GAME_ID_PROPERTY_NAME);
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
		spdlog::error("Mod collection is missing '{}' property'.", JSON_MODS_PROPERTY_NAME);
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

		if(newModCollection->hasMod(*newMod.get())) {
			spdlog::error("Encountered duplicate mod #{}{}.", newModCollection->m_mods.size() + 1, newModCollection->numberOfMods() == 0 ? "" : fmt::format(" (after mod with ID '{}')", newModCollection->getMod(newModCollection->numberOfMods() - 1)->getID()));
			return nullptr;
		}

		newModCollection->m_mods.push_back(std::shared_ptr<Mod>(newMod.release()));
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

	// verify file format version
	const char * fileFormatVersion = modsElement->Attribute(XML_FILE_FORMAT_VERSION_ATTRIBUTE_NAME.c_str());

	if(fileFormatVersion != nullptr && Utilities::stringLength(fileFormatVersion) != 0) {
		std::optional<std::uint8_t> optionalVersionComparison(Utilities::compareVersions(fileFormatVersion, FILE_FORMAT_VERSION));

		if(!optionalVersionComparison.has_value()) {
			spdlog::error("Invalid mod collection file format version: '{}'.", fileFormatVersion);
			return false;
		}

		if(*optionalVersionComparison != 0) {
			spdlog::error("Unsupported mod collection file format version: '{}', only version '{}' is supported.", fileFormatVersion, FILE_FORMAT_VERSION);
			return false;
		}
	}
	else {
		spdlog::warn("Mod collection XML data element '{}' is missing file format version attribute '{}', and may fail to load correctly!", XML_MODS_ELEMENT_NAME, XML_FILE_FORMAT_VERSION_ATTRIBUTE_NAME);
	}

	// verify game identifier
	const char * gameID = modsElement->Attribute(XML_GAME_ID_ATTRIBUTE_NAME.c_str());

	if(gameID == nullptr || Utilities::stringLength(gameID) == 0) {
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

		if(modCollection->hasMod(*newMod.get())) {
			spdlog::error("Encountered duplicate mod #{}{}.", modCollection->m_mods.size() + 1, modCollection->numberOfMods() == 0 ? "" : fmt::format(" (after mod with ID '{}')", modCollection->getMod(modCollection->numberOfMods() - 1)->getID()));
			return nullptr;
		}

		modCollection->m_mods.push_back(std::shared_ptr<Mod>(newMod.release()));

		modElement = modElement->NextSiblingElement();
	}

	return modCollection;
}

bool ModCollection::loadFrom(const std::string & filePath, bool skipFileInfoValidation) {
	if(filePath.empty()) {
		return false;
	}

	std::string fileExtension(Utilities::getFileExtension(filePath));

	if(fileExtension.empty()) {
		return false;
	}
	else if(Utilities::areStringsEqualIgnoreCase(fileExtension, "xml")) {
		return loadFromXML(filePath, skipFileInfoValidation);
	}
	else if(Utilities::areStringsEqualIgnoreCase(fileExtension, "json")) {
		return loadFromJSON(filePath, skipFileInfoValidation);
	}

	return false;
}

bool ModCollection::loadFromXML(const std::string & filePath, bool skipFileInfoValidation) {
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

	if(!ModCollection::isValid(modCollection.get(), skipFileInfoValidation)) {
		spdlog::error("Failed to parse mod collection from XML file '{}'.", filePath);
		return false;
	}

	m_mods = std::move(modCollection->m_mods);

	updated(*this);

	return true;
}

bool ModCollection::loadFromJSON(const std::string & filePath, bool skipFileInfoValidation) {
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

	if(!ModCollection::isValid(modCollection.get(), skipFileInfoValidation)) {
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

		if(mod->hasAlias() && !hasModWithID(mod->getAlias())) {
			if(verbose) {
				spdlog::warn("Mod '{}' alias with ID '{}' does not exist in the current collection.", mod->getName(), mod->getAlias());
			}

			return false;
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

bool ModCollection::isValid(bool skipFileInfoValidation) const {
	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_mods.begin(); i != m_mods.end(); ++i) {
		if(!(*i)->isValid(skipFileInfoValidation)) {
			return false;
		}
	}

	return true;
}

bool ModCollection::isValid(const ModCollection * m, bool skipFileInfoValidation) {
	return m != nullptr && m->isValid(skipFileInfoValidation);
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
