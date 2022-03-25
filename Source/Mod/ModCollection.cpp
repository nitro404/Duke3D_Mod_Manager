#include "ModCollection.h"

#include "Game/GameVersion.h"
#include "Game/GameVersionCollection.h"
#include "Mod.h"
#include "ModCollectionListener.h"
#include "ModDownload.h"
#include "ModFile.h"
#include "ModGameVersion.h"
#include "ModTeam.h"
#include "ModTeamMember.h"
#include "ModVersion.h"
#include "ModVersionType.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/Utilities.h>

#include <fmt/core.h>
#include <magic_enum.hpp>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <tinyxml2.h>

#include <filesystem>
#include <fstream>

static const std::string XML_MODS_ELEMENT_NAME("mods");

ModCollection::ModCollection()
	: ModCollectionBroadcaster() { }

ModCollection::ModCollection(ModCollection && m) noexcept
	: ModCollectionBroadcaster(std::move(m))
	, m_mods(std::move(m.m_mods)) { }

ModCollection::ModCollection(const ModCollection & m)
	: ModCollectionBroadcaster(m) {
	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m.m_mods.begin(); i != m.m_mods.end(); ++i) {
		m_mods.push_back(std::make_shared<Mod>(**i));
	}
}

ModCollection & ModCollection::operator = (ModCollection && m) noexcept {
	if(this != &m) {
		ModCollectionBroadcaster::operator = (m);

		m_mods = std::move(m.m_mods);
	}

	return *this;
}

ModCollection & ModCollection::operator = (const ModCollection & m) {
	ModCollectionBroadcaster::operator = (m);

	m_mods.clear();

	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m.m_mods.begin(); i != m.m_mods.end(); ++i) {
		m_mods.push_back(std::make_shared<Mod>(**i));
	}

	return *this;
}

ModCollection::~ModCollection() { }

size_t ModCollection::numberOfMods() const {
	return m_mods.size();
}

bool ModCollection::hasMod(const Mod & mod) const {
	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_mods.begin(); i != m_mods.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getID(), mod.getID()) == 0) {
			return true;
		}
	}

	return false;
}

bool ModCollection::hasMod(const std::string & id) const {
	if(id.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_mods.begin(); i != m_mods.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getID(), id) == 0) {
			return true;
		}
	}

	return false;
}

bool ModCollection::hasModWithName(const std::string & name) const {
	if(name.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_mods.begin(); i != m_mods.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getName(), name) == 0) {
			return true;
		}
	}

	return false;
}

size_t ModCollection::indexOfMod(const Mod & mod) const {
	for(size_t i = 0; i < m_mods.size(); i++) {
		if(Utilities::compareStringsIgnoreCase(m_mods[i]->getID(), mod.getID()) == 0) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t ModCollection::indexOfMod(const std::string & id) const {
	if(id.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_mods.size(); i++) {
		if(Utilities::compareStringsIgnoreCase(m_mods[i]->getID(), id) == 0) {
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
		if(Utilities::compareStringsIgnoreCase(m_mods[i]->getName(), name) == 0) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<Mod> ModCollection::getMod(size_t index) const {
	if(index >= m_mods.size()) { return nullptr; }

	return m_mods[index];
}

std::shared_ptr<Mod> ModCollection::getMod(const std::string & id) const {
	if(id.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_mods.begin(); i != m_mods.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getID(), id) == 0) {
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
		if(Utilities::compareStringsIgnoreCase((*i)->getName(), name) == 0) {
			return *i;
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

	notifyCollectionChanged();

	return true;
}

bool ModCollection::removeMod(size_t index) {
	if(index >= m_mods.size()) {
		return false;
	}

	m_mods.erase(m_mods.begin() + index);

	notifyCollectionChanged();

	return true;
}

bool ModCollection::removeMod(const Mod & mod) {
	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_mods.begin(); i != m_mods.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getID(), mod.getID()) == 0) {
			m_mods.erase(i);

			notifyCollectionChanged();

			return true;
		}
	}

	return false;
}

bool ModCollection::removeMod(const std::string & id) {
	if(id.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_mods.begin(); i != m_mods.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getID(), id) == 0) {
			m_mods.erase(i);

			notifyCollectionChanged();

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
		if(Utilities::compareStringsIgnoreCase((*i)->getName(), name) == 0) {
			m_mods.erase(i);

			notifyCollectionChanged();

			return true;
		}
	}

	return false;
}

void ModCollection::clearMods() {
	m_mods.clear();

	notifyCollectionChanged();
}

rapidjson::Document ModCollection::toJSON() const {
	rapidjson::Document modsValue(rapidjson::kArrayType);
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator = modsValue.GetAllocator();

	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_mods.begin(); i != m_mods.end(); ++i) {
		modsValue.PushBack((*i)->toJSON(allocator), allocator);
	}

	return modsValue;
}

tinyxml2::XMLElement * ModCollection::toXML(tinyxml2::XMLDocument * document) const {
	if(document == nullptr) {
		return nullptr;
	}

	tinyxml2::XMLElement * modsElement = document->NewElement(XML_MODS_ELEMENT_NAME.c_str());

	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_mods.begin(); i != m_mods.end(); ++i) {
		modsElement->InsertEndChild((*i)->toXML(document));
	}

	return modsElement;
}

std::unique_ptr<ModCollection> ModCollection::parseFrom(const rapidjson::Value & modCollectionValue) {
	if(!modCollectionValue.IsArray()) {
		fmt::print("Invalid mod collection type: '{}', expected 'array'.\n", Utilities::typeToString(modCollectionValue.GetType()));
		return nullptr;
	}

	std::unique_ptr<ModCollection> newModCollection(std::make_unique<ModCollection>());

	if(modCollectionValue.Empty()) {
		return newModCollection;
	}

	std::unique_ptr<Mod> newMod;

	for(rapidjson::Value::ConstValueIterator i = modCollectionValue.Begin(); i != modCollectionValue.End(); ++i) {
		newMod = Mod::parseFrom(*i);

		if(!Mod::isValid(newMod.get())) {
			fmt::print("Failed to parse mod #{}{}!\n", newModCollection->m_mods.size() + 1, newModCollection->numberOfMods() == 0 ? "" : fmt::format(" (after mod with ID '{}')", newModCollection->getMod(newModCollection->numberOfMods() - 1)->getID()));
			return nullptr;
		}

		if(newModCollection->hasMod(*newMod.get())) {
			fmt::print("Encountered duplicate mod #{}{}.\n", newModCollection->m_mods.size() + 1, newModCollection->numberOfMods() == 0 ? "" : fmt::format(" (after mod with ID '{}')", newModCollection->getMod(newModCollection->numberOfMods() - 1)->getID()));
			return nullptr;
		}

		newModCollection->m_mods.push_back(std::shared_ptr<Mod>(newMod.release()));
	}

	newModCollection->notifyCollectionChanged();

	return newModCollection;
}

std::unique_ptr<ModCollection> ModCollection::parseFrom(const tinyxml2::XMLElement * modsElement) {
	// verify the mods element
	if(modsElement == nullptr) {
		fmt::print("Missing '{}' element!\n", XML_MODS_ELEMENT_NAME);
		return false;
	}

	if(modsElement->Name() != XML_MODS_ELEMENT_NAME) {
		fmt::print("Invalid element name '{}', expected '{}'!\n", modsElement->Name(), XML_MODS_ELEMENT_NAME);
		return false;
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

		newMod = Mod::parseFrom(modElement);

		if(!Mod::isValid(newMod.get())) {
			fmt::print("Failed to parse mod #{}{}!\n", modCollection->m_mods.size() + 1, modCollection->numberOfMods() == 0 ? "" : fmt::format(" (after mod with ID '{}')", modCollection->getMod(modCollection->numberOfMods() - 1)->getID()));
			return false;
		}

		if(modCollection->hasMod(*newMod.get())) {
			fmt::print("Encountered duplicate mod #{}{}.\n", modCollection->m_mods.size() + 1, modCollection->numberOfMods() == 0 ? "" : fmt::format(" (after mod with ID '{}')", modCollection->getMod(modCollection->numberOfMods() - 1)->getID()));
			return nullptr;
		}

		modCollection->m_mods.push_back(std::shared_ptr<Mod>(newMod.release()));

		modElement = modElement->NextSiblingElement();
	}

	modCollection->notifyCollectionChanged();

	return modCollection;
}

bool ModCollection::loadFrom(const std::string & filePath) {
	if(filePath.empty()) {
		return false;
	}

	std::string fileExtension(Utilities::getFileExtension(filePath));

	if(fileExtension.empty()) {
		return false;
	}
	else if(Utilities::compareStringsIgnoreCase(fileExtension, "xml") == 0) {
		return loadFromXML(filePath);
	}
	else if(Utilities::compareStringsIgnoreCase(fileExtension, "json") == 0) {
		return loadFromJSON(filePath);
	}

	return false;
}

bool ModCollection::loadFromXML(const std::string & filePath) {
	if(filePath.empty() || !std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		return false;
	}

	tinyxml2::XMLDocument modCollectionDocument;
	tinyxml2::XMLError result = modCollectionDocument.LoadFile(filePath.c_str());

	if(result != tinyxml2::XML_SUCCESS) {
		fmt::print("Failed to load mod collection from XML file '{}' with error code: '{}'.\n", filePath, magic_enum::enum_name(result));
		return false;
	}

	m_mods.clear();

	std::unique_ptr<ModCollection> modCollection(parseFrom(modCollectionDocument.RootElement()));

	if(!ModCollection::isValid(modCollection.get())) {
		fmt::print("Failed to parse mod collection from XML file '{}'.\n", filePath);
		return false;
	}

	m_mods = std::move(modCollection->m_mods);

	notifyCollectionChanged();

	return true;
}

bool ModCollection::loadFromJSON(const std::string & filePath) {
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

	std::unique_ptr<ModCollection> modCollection(parseFrom(modsValue));

	if(!ModCollection::isValid(modCollection.get())) {
		fmt::print("Failed to parse mod collection from JSON file '{}'.\n", filePath);
		return false;
	}

	m_mods = std::move(modCollection->m_mods);

	notifyCollectionChanged();

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
	else if(Utilities::compareStringsIgnoreCase(fileExtension, "json") == 0) {
		return saveToJSON(filePath, overwrite);
	}
	else if(Utilities::compareStringsIgnoreCase(fileExtension, "xml") == 0) {
		return saveToXML(filePath, overwrite);
	}

	return false;
}

bool ModCollection::saveToXML(const std::string & filePath, bool overwrite) const {
	if (!overwrite && std::filesystem::exists(std::filesystem::path(filePath))) {
		fmt::print("File '{}' already exists, use overwrite to force write.\n", filePath);
		return false;
	}

	tinyxml2::XMLDocument modCollectionDocument;

	modCollectionDocument.InsertFirstChild(modCollectionDocument.NewDeclaration());

	modCollectionDocument.InsertEndChild(toXML(&modCollectionDocument));

	tinyxml2::XMLError result = modCollectionDocument.SaveFile(filePath.c_str());

	if(result != tinyxml2::XML_SUCCESS) {
		fmt::print("Failed to save mod collection to XML file '{}' with error code: '{}'.\n", filePath, magic_enum::enum_name(result));
		return false;
	}

	return true;
}

bool ModCollection::saveToJSON(const std::string & filePath, bool overwrite) const {
	if (!overwrite && std::filesystem::exists(std::filesystem::path(filePath))) {
		fmt::print("File '{}' already exists, use overwrite to force write.\n", filePath);
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

void ModCollection::notifyCollectionChanged() const {
	for(size_t i = 0; i < numberOfListeners(); i++) {
		getListener(i)->modCollectionUpdated();
	}
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

					if(!gameVersions.hasGameVersion(modGameVersion->getGameVersion())) {
						if(verbose) {
							fmt::print("Mod '{}' contains unknown game version: '{}'.\n", mod->getFullName(j, k), modGameVersion->getGameVersion());
						}

						return false;
					}
				}
			}
		}

		for(size_t j = 0; j < mod->numberOfDownloads(); j++) {
			modDownload = mod->getDownload(j);

			if(!modDownload->getGameVersion().empty() &&
			   modDownload->getGameVersion() != GameVersion::ALL_VERSIONS &&
			   !gameVersions.hasGameVersion(modDownload->getGameVersion())) {
				if(verbose) {
					fmt::print("Mod '{}' has download '{}' with unknown game version: '{}'.\n", mod->getID(), modDownload->getFileName(), modDownload->getGameVersion());
				}

				return false;
			}
		}
	}

	return true;
}

bool ModCollection::isValid() const {
	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_mods.begin(); i != m_mods.end(); ++i) {
		if(!(*i)->isValid()) {
			return false;
		}
	}

	return true;
}

bool ModCollection::isValid(const ModCollection * m) {
	return m != nullptr && m->isValid();
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
