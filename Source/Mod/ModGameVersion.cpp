#include "ModGameVersion.h"

#include "Game/GameVersion.h"
#include "Mod.h"
#include "ModDownload.h"
#include "ModFile.h"
#include "ModVersion.h"
#include "ModVersionType.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>

#include <fmt/core.h>
#include <tinyxml2.h>

#include <array>
#include <sstream>
#include <string_view>
#include <vector>

static const std::string XML_MOD_GAME_VERSION_ELEMENT_NAME("game");
static const std::string XML_MOD_GAME_VERSION_GAME_VERSION_ATTRIBUTE_NAME("type");
static const std::string XML_MOD_GAME_VERSION_CONVERTED_ATTRIBUTE_NAME("state");
static const std::array<std::string_view, 2> XML_MOD_GAME_VERSION_ATTRIBUTE_NAMES = {
	XML_MOD_GAME_VERSION_GAME_VERSION_ATTRIBUTE_NAME,
	XML_MOD_GAME_VERSION_CONVERTED_ATTRIBUTE_NAME
};

static constexpr const char * JSON_MOD_GAME_VERSION_GAME_VERSION_PROPERTY_NAME = "gameVersion";
static constexpr const char * JSON_MOD_GAME_VERSION_CONVERTED_PROPERTY_NAME = "converted";
static constexpr const char * JSON_MOD_GAME_VERSION_FILES_PROPERTY_NAME = "files";
static const std::array<std::string_view, 3> JSON_MOD_GAME_VERSION_PROPERTY_NAMES = {
	JSON_MOD_GAME_VERSION_GAME_VERSION_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_CONVERTED_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_FILES_PROPERTY_NAME
};

ModGameVersion::ModGameVersion(const std::string & gameVersion, bool converted)
	: m_gameVersion(Utilities::trimString(gameVersion))
	, m_converted(converted)
	, m_parentModVersionType(nullptr) { }

ModGameVersion::ModGameVersion(ModGameVersion && m) noexcept
	: m_gameVersion(std::move(m.m_gameVersion))
	, m_converted(m.m_converted)
	, m_files(std::move(m.m_files))
	, m_parentModVersionType(nullptr) {
	updateParent();
}

ModGameVersion::ModGameVersion(const ModGameVersion & m)
	: m_gameVersion(m.m_gameVersion)
	, m_converted(m.m_converted)
	, m_parentModVersionType(nullptr) {
	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m.m_files.begin(); i != m.m_files.end(); ++i) {
		m_files.push_back(std::make_shared<ModFile>(**i));
	}

	updateParent();
}

ModGameVersion & ModGameVersion::operator = (ModGameVersion && m) noexcept {
	if(this != &m) {
		m_gameVersion = std::move(m.m_gameVersion);
		m_converted = m.m_converted;
		m_files = std::move(m.m_files);

		updateParent();
	}

	return *this;
}

ModGameVersion & ModGameVersion::operator = (const ModGameVersion & m) {
	m_files.clear();

	m_gameVersion = m.m_gameVersion;
	m_converted = m.m_converted;

	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m.m_files.begin(); i != m.m_files.end(); ++i) {
		m_files.push_back(std::make_shared<ModFile>(**i));
	}

	updateParent();

	return *this;
}

ModGameVersion::~ModGameVersion() {
	m_parentModVersionType = nullptr;
}

const std::string & ModGameVersion::getGameVersion() const {
	return m_gameVersion;
}

std::string ModGameVersion::getFullName() const {
	const Mod * parentMod = getParentMod();
	const ModVersion * parentModVersion = getParentModVersion();

	if(!Mod::isValid(parentMod) || parentModVersion == nullptr || m_parentModVersionType == nullptr) {
		return "";
	}

	std::stringstream fullModName;
	fullModName << parentMod->getName();

	if(!parentModVersion->getVersion().empty()) {
		fullModName << " " + parentModVersion->getVersion();
	}

	if(!m_parentModVersionType->getType().empty()) {
		fullModName << " " + m_parentModVersionType->getType();
	}

	fullModName << " (" << m_gameVersion << ")";

	return fullModName.str();
}

bool ModGameVersion::isEDuke32() const {
	return Utilities::compareStringsIgnoreCase(m_gameVersion, GameVersion::EDUKE32.getName()) == 0;
}

bool ModGameVersion::isConverted() const {
	return m_converted;
}

const Mod * ModGameVersion::getParentMod() const {
	const ModVersion * parentModVersion = getParentModVersion();

	if(parentModVersion == nullptr) {
		return nullptr;
	}

	return parentModVersion->getParentMod();
}

const ModVersion * ModGameVersion::getParentModVersion() const {
	if(m_parentModVersionType == nullptr) {
		return nullptr;
	}

	return m_parentModVersionType->getParentModVersion();
}

const ModVersionType * ModGameVersion::getParentModVersionType() const {
	return m_parentModVersionType;
}

void ModGameVersion::setGameVersion(const std::string & gameVersion) {
	m_gameVersion = Utilities::trimString(gameVersion);
}

void ModGameVersion::setConverted(bool converted) {
	m_converted = converted;
}

void ModGameVersion::setParentModVersionType(const ModVersionType * modVersionType) {
	m_parentModVersionType = modVersionType;
}

size_t ModGameVersion::numberOfFiles() const {
	return m_files.size();
}

size_t ModGameVersion::numberOfFilesOfType(const std::string & fileType) {
	if(fileType.empty()) {
		return 0;
	}

	size_t fileCount = 0;

	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getType(), fileType) == 0) {
			fileCount++;
		}
	}

	return fileCount;
}

bool ModGameVersion::hasFile(const ModFile & file) const {
	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getFileName(), file.getFileName()) == 0) {
			return true;
		}
	}

	return false;
}

bool ModGameVersion::hasFile(const std::string & fileName) const {
	if(fileName.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getFileName(), fileName) == 0) {
			return true;
		}
	}

	return false;
}

bool ModGameVersion::hasFileOfType(const std::string & fileType) const {
	if(fileType.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getType(), fileType) == 0) {
			return true;
		}
	}

	return false;
}

size_t ModGameVersion::indexOfFile(const ModFile & file) const {
	for(size_t i = 0; i < m_files.size(); i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getFileName(), file.getFileName()) == 0) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t ModGameVersion::indexOfFile(const std::string & fileName) const {
	if(fileName.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_files.size(); i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getFileName(), fileName) == 0) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t ModGameVersion::indexOfFirstFileOfType(const std::string & fileType) const {
	if(fileType.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_files.size(); i++) {
		if(Utilities::compareStringsIgnoreCase(m_files[i]->getType(), fileType) == 0) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<ModFile> ModGameVersion::getFile(size_t index) const {
	if(index >= m_files.size()) {
		return nullptr;
	}

	return m_files[index];
}

std::shared_ptr<ModFile> ModGameVersion::getFile(const std::string & fileName) const {
	if(fileName.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getFileName(), fileName) == 0) {
			return *i;
		}
	}

	return nullptr;
}

std::shared_ptr<ModFile> ModGameVersion::getFirstFileOfType(const std::string & fileType) const {
	if(fileType.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getType(), fileType) == 0) {
			return *i;
		}
	}

	return nullptr;
}

std::vector<std::shared_ptr<ModFile>> ModGameVersion::getFilesOfType(const std::string & fileType) const {
	std::vector<std::shared_ptr<ModFile>> files;

	if(fileType.empty()) {
		return files;
	}

	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getType(), fileType) == 0) {
			files.push_back(*i);
		}
	}

	return files;
}

std::optional<std::string> ModGameVersion::getFirstFileNameOfType(const std::string & fileType) const {
	if(fileType.empty()) {
		return {};
	}

	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getType(), fileType) == 0) {
			return (*i)->getFileName();
		}
	}

	return {};
}

std::vector<std::string> ModGameVersion::getFileNamesOfType(const std::string & fileType) const {
	std::vector<std::string> fileNames;

	if(fileType.empty()) {
		return fileNames;
	}

	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getType(), fileType) == 0) {
			fileNames.emplace_back((*i)->getFileName());
		}
	}

	return fileNames;
}

const std::vector<std::shared_ptr<ModFile>> & ModGameVersion::getFiles() const {
	return m_files;
}

std::shared_ptr<ModDownload> ModGameVersion::getDownload() const {
	const Mod * mod = getParentMod();

	if(mod == nullptr) {
		return nullptr;
	}

	return mod->getDownloadForGameVersion(this);
}

bool ModGameVersion::addFile(const ModFile & file) {
	if(!file.isValid() || hasFile(file)) {
		return false;
	}

	std::shared_ptr<ModFile> newModFile = std::make_shared<ModFile>(file);
	newModFile->setParentModGameVersion(this);

	m_files.push_back(newModFile);

	return true;
}

bool ModGameVersion::removeFile(size_t index) {
	if(index >= m_files.size()) {
		return false;
	}

	m_files[index]->setParentModGameVersion(nullptr);
	m_files.erase(m_files.begin() + index);

	return true;
}

bool ModGameVersion::removeFile(const ModFile & file) {
	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getFileName(), file.getFileName()) == 0) {
			(*i)->setParentModGameVersion(nullptr);
			m_files.erase(i);

			return true;
		}
	}

	return false;
}

bool ModGameVersion::removeFile(const std::string & fileName) {
	if(fileName.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getFileName(), fileName) == 0) {
			(*i)->setParentModGameVersion(nullptr);
			m_files.erase(i);

			return true;
		}
	}

	return false;
}

size_t ModGameVersion::removeFilesOfType(const std::string & fileType) {
	if(fileType.empty()) {
		return 0;
	}

	size_t numberOfFilesRemoved = 0;

	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::compareStringsIgnoreCase((*i)->getType(), fileType) == 0) {
			(*i)->setParentModGameVersion(nullptr);
			m_files.erase(i);

			numberOfFilesRemoved++;
		}
	}

	return numberOfFilesRemoved;
}

void ModGameVersion::clearFiles() {
	m_files.clear();
}

void ModGameVersion::updateParent() {
	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		(*i)->setParentModGameVersion(this);
	}
}

rapidjson::Value ModGameVersion::toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> & allocator) const {
	rapidjson::Value modGameVersionValue(rapidjson::kObjectType);

	rapidjson::Value gameVersionValue(m_gameVersion.c_str(), allocator);
	modGameVersionValue.AddMember(rapidjson::StringRef(JSON_MOD_GAME_VERSION_GAME_VERSION_PROPERTY_NAME), gameVersionValue, allocator);

	modGameVersionValue.AddMember(rapidjson::StringRef(JSON_MOD_GAME_VERSION_CONVERTED_PROPERTY_NAME), rapidjson::Value(m_converted), allocator);

	rapidjson::Value filesValue(rapidjson::kArrayType);

	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		filesValue.PushBack((*i)->toJSON(allocator), allocator);
	}

	modGameVersionValue.AddMember(rapidjson::StringRef(JSON_MOD_GAME_VERSION_FILES_PROPERTY_NAME), filesValue, allocator);

	return modGameVersionValue;
}

tinyxml2::XMLElement * ModGameVersion::toXML(tinyxml2::XMLDocument * document) const {
	if(document == nullptr) {
		return nullptr;
	}

	tinyxml2::XMLElement * modGameVersionElement = document->NewElement(XML_MOD_GAME_VERSION_ELEMENT_NAME.c_str());

	modGameVersionElement->SetAttribute(XML_MOD_GAME_VERSION_GAME_VERSION_ATTRIBUTE_NAME.c_str(), m_gameVersion.c_str());
	modGameVersionElement->SetAttribute(XML_MOD_GAME_VERSION_CONVERTED_ATTRIBUTE_NAME.c_str(), m_converted ? "Converted" : "Native");

	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		modGameVersionElement->InsertEndChild((*i)->toXML(document));
	}

	return modGameVersionElement;
}

std::unique_ptr<ModGameVersion> ModGameVersion::parseFrom(const rapidjson::Value & modGameVersionValue) {
	if(!modGameVersionValue.IsObject()) {
		fmt::print("Invalid mod game version type: '{}', expected 'object'.\n", Utilities::typeToString(modGameVersionValue.GetType()));
		return nullptr;
	}

	// check for unhandled mod game version properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = modGameVersionValue.MemberBegin(); i != modGameVersionValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view propertyName : JSON_MOD_GAME_VERSION_PROPERTY_NAMES) {
			if(i->name.GetString() == propertyName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			fmt::print("Mod game version has unexpected property '{}'.\n", i->name.GetString());
			return nullptr;
		}
	}

	// parse the mod game version game version property
	if(!modGameVersionValue.HasMember(JSON_MOD_GAME_VERSION_GAME_VERSION_PROPERTY_NAME)) {
		fmt::print("Mod game version is missing '{}' property'.\n", JSON_MOD_GAME_VERSION_GAME_VERSION_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modGameVersionGameVersionValue = modGameVersionValue[JSON_MOD_GAME_VERSION_GAME_VERSION_PROPERTY_NAME];

	if(!modGameVersionGameVersionValue.IsString()) {
		fmt::print("Mod game version '{}' property has invalid type: '{}', expected 'string'.\n", JSON_MOD_GAME_VERSION_GAME_VERSION_PROPERTY_NAME, Utilities::typeToString(modGameVersionGameVersionValue.GetType()));
		return nullptr;
	}

	std::string modGameVersionGameVersion(modGameVersionGameVersionValue.GetString());

	// parse the mod game version converted property
	if(!modGameVersionValue.HasMember(JSON_MOD_GAME_VERSION_CONVERTED_PROPERTY_NAME)) {
		fmt::print("Mod game version is missing '{}' property'.\n", JSON_MOD_GAME_VERSION_CONVERTED_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modVersionConvertedValue = modGameVersionValue[JSON_MOD_GAME_VERSION_CONVERTED_PROPERTY_NAME];

	if(!modVersionConvertedValue.IsBool()) {
		fmt::print("Mod game version '{}' property has invalid type: '{}', expected 'boolean'.\n", JSON_MOD_GAME_VERSION_CONVERTED_PROPERTY_NAME, Utilities::typeToString(modVersionConvertedValue.GetType()));
		return nullptr;
	}

	bool converted = modVersionConvertedValue.GetBool();

	// initialize the mod game version
	std::unique_ptr<ModGameVersion> newModGameVersion = std::make_unique<ModGameVersion>(modGameVersionGameVersion, converted);

	// parse the mod game version files property
	if(!modGameVersionValue.HasMember(JSON_MOD_GAME_VERSION_FILES_PROPERTY_NAME)) {
		fmt::print("Mod game version is missing '{}' property'.\n", JSON_MOD_GAME_VERSION_FILES_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modFilesValue = modGameVersionValue[JSON_MOD_GAME_VERSION_FILES_PROPERTY_NAME];

	if(!modFilesValue.IsArray()) {
		fmt::print("Mod game version '{}' property has invalid type: '{}', expected 'array'.\n", JSON_MOD_GAME_VERSION_FILES_PROPERTY_NAME, Utilities::typeToString(modFilesValue.GetType()));
		return nullptr;
	}

	std::shared_ptr<ModFile> newModFile;

	for(rapidjson::Value::ConstValueIterator i = modFilesValue.Begin(); i != modFilesValue.End(); ++i) {
		newModFile = std::shared_ptr<ModFile>(std::move(ModFile::parseFrom(*i)).release());

		if(!ModFile::isValid(newModFile.get())) {
			fmt::print("Failed to parse mod file #{}.\n", newModGameVersion->m_files.size() + 1);
			return nullptr;
		}

		newModFile->setParentModGameVersion(newModGameVersion.get());

		if(newModGameVersion->hasFile(*newModFile.get())) {
			fmt::print("Encountered duplicate mod file #{}.\n", newModGameVersion->m_files.size() + 1);
			return nullptr;
		}

		newModGameVersion->m_files.push_back(newModFile);
	}

	return newModGameVersion;
}

std::unique_ptr<ModGameVersion> ModGameVersion::parseFrom(const tinyxml2::XMLElement * modGameVersionElement) {
	if(modGameVersionElement == nullptr) {
		return nullptr;
	}

	// verify element name
	if(modGameVersionElement->Name() != XML_MOD_GAME_VERSION_ELEMENT_NAME) {
		fmt::print("Invalid mod game version element name: '{}', expected '{}'.\n", modGameVersionElement->Name(), XML_MOD_GAME_VERSION_ELEMENT_NAME);
		return nullptr;
	}

	// check for unhandled mod game version element attributes
	bool attributeHandled = false;
	const tinyxml2::XMLAttribute * modGameVersionAttribute = modGameVersionElement->FirstAttribute();

	while(true) {
		if(modGameVersionAttribute == nullptr) {
			break;
		}

		attributeHandled = false;

		for(const std::string_view & attributeName : XML_MOD_GAME_VERSION_ATTRIBUTE_NAMES) {
			if(modGameVersionAttribute->Name() == attributeName) {
				attributeHandled = true;
				break;
			}
		}

		if(!attributeHandled) {
			fmt::print("Element '{}' has unexpected attribute '{}'.\n", XML_MOD_GAME_VERSION_ELEMENT_NAME, modGameVersionAttribute->Name());
			return nullptr;
		}

		modGameVersionAttribute = modGameVersionAttribute->Next();
	}

	// read the mod game version attributes
	const char * modGameVersionGameVersion = modGameVersionElement->Attribute(XML_MOD_GAME_VERSION_GAME_VERSION_ATTRIBUTE_NAME.c_str());

	if(modGameVersionGameVersion == nullptr || Utilities::stringLength(modGameVersionGameVersion) == 0) {
		fmt::print("Attribute '{}' is missing from '{}' element.\n", XML_MOD_GAME_VERSION_GAME_VERSION_ATTRIBUTE_NAME, XML_MOD_GAME_VERSION_ELEMENT_NAME);
		return nullptr;
	}

	const char * modGameVersionConvertedData = modGameVersionElement->Attribute(XML_MOD_GAME_VERSION_CONVERTED_ATTRIBUTE_NAME.c_str());

	if(modGameVersionConvertedData == nullptr || Utilities::stringLength(modGameVersionConvertedData) == 0) {
		fmt::print("Attribute '{}' is missing from '{}' element.\n", XML_MOD_GAME_VERSION_CONVERTED_ATTRIBUTE_NAME, XML_MOD_GAME_VERSION_ELEMENT_NAME);
		return nullptr;
	}

	bool modGameVersionConverted = false;

	if(Utilities::compareStringsIgnoreCase(modGameVersionConvertedData, "Converted") == 0) {
		modGameVersionConverted = true;
	}
	else if(Utilities::compareStringsIgnoreCase(modGameVersionConvertedData, "Native") == 0) {
		modGameVersionConverted = false;
	}
	else {
		fmt::print("Attribute '{}' in element '{}' has an invalid value: '{}', expected 'Native' or 'Converted'.\n", XML_MOD_GAME_VERSION_CONVERTED_ATTRIBUTE_NAME, XML_MOD_GAME_VERSION_ELEMENT_NAME, modGameVersionConvertedData);
		return nullptr;
	}

	// initialize the mod game version
	std::unique_ptr<ModGameVersion> newModGameVersion = std::make_unique<ModGameVersion>(modGameVersionGameVersion, modGameVersionConverted);

	// iterate over all of the mod file elements
	const tinyxml2::XMLElement * modFileElement = modGameVersionElement->FirstChildElement();

	if(modFileElement == nullptr) {
		fmt::print("Element '{}' has no children.\n", XML_MOD_GAME_VERSION_ELEMENT_NAME);
		return nullptr;
	}

	std::shared_ptr<ModFile> newModFile;

	while(true) {
		if(modFileElement == nullptr) {
			break;
		}

		newModFile = std::shared_ptr<ModFile>(std::move(ModFile::parseFrom(modFileElement)).release());

		if(!ModFile::isValid(newModFile.get())) {
			fmt::print("Failed to parse mod file #{}.\n", newModGameVersion->m_files.size() + 1);
			return nullptr;
		}

		newModFile->setParentModGameVersion(newModGameVersion.get());

		if(newModGameVersion->hasFile(*newModFile.get())) {
			fmt::print("Encountered duplicate mod file #{}.\n", newModGameVersion->m_files.size() + 1);
			return nullptr;
		}

		newModGameVersion->m_files.push_back(newModFile);

		modFileElement = modFileElement->NextSiblingElement();
	}

	return newModGameVersion;
}

bool ModGameVersion::isGameVersionSupported(const GameVersion & gameVersion) const {
	return gameVersion.isValid() &&
		   Utilities::compareStringsIgnoreCase(m_gameVersion, gameVersion.getName()) == 0;
}

bool ModGameVersion::isGameVersionCompatible(const GameVersion & gameVersion) const {
	return gameVersion.isValid() &&
		   isGameVersionSupported(gameVersion) ||
		   gameVersion.hasCompatibleGameVersion(m_gameVersion);
}

bool ModGameVersion::isValid() const {
	if(m_gameVersion.empty() ||
	   m_files.empty()) {
		return false;
	}

	if(isEDuke32()) {
		if(!hasFileOfType("grp") && !hasFileOfType("zip")) {
			return false;
		}
	}
	else {
		if(!hasFileOfType("grp")) {
			return false;
		}
	}

	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(!(*i)->isValid()) {
			return false;
		}

		if((*i)->getParentModGameVersion() != this) {
			return false;
		}

		for(std::vector<std::shared_ptr<ModFile>>::const_iterator j = i + 1; j != m_files.end(); ++j) {
			if(Utilities::compareStringsIgnoreCase((*i)->getFileName(), (*j)->getFileName()) == 0) {
				return false;
			}
		}
	}

	return true;
}

bool ModGameVersion::isValid(const ModGameVersion * m) {
	return m != nullptr && m->isValid();
}

bool ModGameVersion::operator == (const ModGameVersion & m) const {
	if(m_files.size() != m.m_files.size() ||
	   Utilities::compareStringsIgnoreCase(m_gameVersion, m.m_gameVersion) != 0 ||
	   m_converted != m.m_converted) {
		return false;
	}

	for(size_t i = 0; i < m_files.size(); i++) {
		if(m_files[i] != m.m_files[i]) {
			return false;
		}
	}

	return true;
}

bool ModGameVersion::operator != (const ModGameVersion & m) const {
	return !operator == (m);
}
