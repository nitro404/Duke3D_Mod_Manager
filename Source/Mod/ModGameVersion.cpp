#include "ModGameVersion.h"

#include "Game/GameVersion.h"
#include "Mod.h"
#include "ModDownload.h"
#include "ModVersion.h"
#include "ModVersionType.h"

#include <Utilities/RapidJSONUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/TinyXML2Utilities.h>

#include <magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <array>
#include <sstream>
#include <string_view>
#include <vector>

static const std::string XML_MOD_ELEMENT_NAME("mod");
static const std::string XML_MOD_ID_ATTRIBUTE_NAME("id");
static const std::string XML_MOD_NAME_ATTRIBUTE_NAME("name");
static const std::string XML_MOD_VERSIONS_ELEMENT_NAME("versions");
static const std::string XML_MOD_VERSION_ELEMENT_NAME("version");
static const std::string XML_MOD_VERSION_TYPE_ELEMENT_NAME("type");
static const std::string XML_MOD_GAME_VERSION_ELEMENT_NAME("game");
static const std::string XML_MOD_GAME_VERSION_GAME_VERSION_ATTRIBUTE_NAME("type");
static const std::string XML_MOD_GAME_VERSION_CONVERTED_ATTRIBUTE_NAME("state");
static const std::string XML_MOD_GAME_VERSION_PROPERTIES_ELEMENT_NAME("properties");
static const std::string XML_MOD_GAME_VERSION_PROPERTY_BASE_NAME("base");
static const std::string XML_MOD_GAME_VERSION_PROPERTY_GAME_EXE_NAME_NAME("game_exe_name");
static const std::string XML_MOD_GAME_VERSION_PROPERTY_SETUP_EXE_NAME_NAME("setup_exe_name");
static const std::string XML_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_FILE_NAME_NAME("game_config_file_name");
static const std::string XML_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_DIRECTORY_PATH_NAME("game_config_directory_path");
static const std::string XML_MOD_GAME_VERSION_PROPERTY_SKILL_START_VALUE_NAME("skill_start_value");
static const std::string XML_MOD_GAME_VERSION_PROPERTY_LOCAL_WORKING_DIRECTORY_NAME("local_working_directory");
static const std::string XML_MOD_GAME_VERSION_PROPERTY_SCRIPT_FILES_READ_FROM_GROUP_NAME("script_files_read_from_group");
static const std::string XML_MOD_GAME_VERSION_PROPERTY_SUPPORTS_SUBDIRECTORIES_NAME("supports_subdirectories");
static const std::string XML_MOD_GAME_VERSION_PROPERTY_REQUIRES_GROUP_FILE_EXTRACTION_NAME("requires_group_file_extraction");
static const std::string XML_MOD_GAME_VERSION_ARGUMENTS_ELEMENT_NAME("arguments");
static const std::string XML_MOD_GAME_VERSION_ARGUMENT_ELEMENT_NAME("argument");
static const std::string XML_MOD_GAME_VERSION_ARGUMENT_NAME_PROPERTY_NAME("name");
static const std::string XML_MOD_GAME_VERSION_ARGUMENT_FLAG_PROPERTY_NAME("flag");
static const std::string XML_MOD_GAME_VERSION_ARGUMENT_CON_FILE_NAME("con-file");
static const std::string XML_MOD_GAME_VERSION_ARGUMENT_EXTRA_CON_FILE_NAME("extra-con-file");
static const std::string XML_MOD_GAME_VERSION_ARGUMENT_GROUP_FILE_NAME("group-file");
static const std::string XML_MOD_GAME_VERSION_ARGUMENT_DEF_FILE_NAME("def-file");
static const std::string XML_MOD_GAME_VERSION_ARGUMENT_EXTRA_DEF_FILE_NAME("extra-def-file");
static const std::string XML_MOD_GAME_VERSION_ARGUMENT_MAP_FILE_NAME("map-file");
static const std::string XML_MOD_GAME_VERSION_ARGUMENT_EPISODE_NAME("episode");
static const std::string XML_MOD_GAME_VERSION_ARGUMENT_LEVEL_NAME("level");
static const std::string XML_MOD_GAME_VERSION_ARGUMENT_SKILL_NAME("skill");
static const std::string XML_MOD_GAME_VERSION_ARGUMENT_RECORD_DEMO_NAME("record-demo");
static const std::string XML_MOD_GAME_VERSION_ARGUMENT_PLAY_DEMO_NAME("play-demo");
static const std::string XML_MOD_GAME_VERSION_ARGUMENT_RESPAWN_MODE_NAME("respawn-mode");
static const std::string XML_MOD_GAME_VERSION_ARGUMENT_WEAPON_SWITCH_ORDER_NAME("weapon-switch-order");
static const std::string XML_MOD_GAME_VERSION_ARGUMENT_DISABLE_MONSTERS_NAME("disable-monsters");
static const std::string XML_MOD_GAME_VERSION_ARGUMENT_DISABLE_SOUND_NAME("disable-sound");
static const std::string XML_MOD_GAME_VERSION_ARGUMENT_DISABLE_MUSIC_NAME("disable-music");
static const std::string XML_MOD_GAME_VERSION_FILES_ELEMENT_NAME("files");
static const std::string XML_MOD_GAME_VERSION_OPERATING_SYSTEMS_ELEMENT_NAME("operatingsystems");
static const std::string XML_MOD_GAME_VERSION_OPERATING_SYSTEM_ELEMENT_NAME("operatingsystem");
static const std::string XML_MOD_GAME_VERSION_OPERATING_SYSTEM_NAME_ATTRIBUTE_NAME("name");
static const std::array<std::string_view, 2> XML_MOD_GAME_VERSION_ATTRIBUTE_NAMES = {
	XML_MOD_GAME_VERSION_GAME_VERSION_ATTRIBUTE_NAME,
	XML_MOD_GAME_VERSION_CONVERTED_ATTRIBUTE_NAME
};

static const std::array<std::string_view, 11> XML_MOD_GAME_VERSION_PROPERTIES_ATTRIBUTE_NAMES = {
	XML_MOD_GAME_VERSION_PROPERTY_BASE_NAME,
	XML_MOD_GAME_VERSION_PROPERTY_GAME_EXE_NAME_NAME,
	XML_MOD_GAME_VERSION_PROPERTY_SETUP_EXE_NAME_NAME,
	XML_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_FILE_NAME_NAME,
	XML_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_DIRECTORY_PATH_NAME,
	XML_MOD_GAME_VERSION_PROPERTY_SKILL_START_VALUE_NAME,
	XML_MOD_GAME_VERSION_PROPERTY_LOCAL_WORKING_DIRECTORY_NAME,
	XML_MOD_GAME_VERSION_PROPERTY_SCRIPT_FILES_READ_FROM_GROUP_NAME,
	XML_MOD_GAME_VERSION_PROPERTY_SUPPORTS_SUBDIRECTORIES_NAME,
	XML_MOD_GAME_VERSION_PROPERTY_REQUIRES_GROUP_FILE_EXTRACTION_NAME
};

static const std::array<std::string_view, 16> XML_MOD_GAME_VERSION_ARGUMENT_NAMES = {
	XML_MOD_GAME_VERSION_ARGUMENT_CON_FILE_NAME,
	XML_MOD_GAME_VERSION_ARGUMENT_EXTRA_CON_FILE_NAME,
	XML_MOD_GAME_VERSION_ARGUMENT_GROUP_FILE_NAME,
	XML_MOD_GAME_VERSION_ARGUMENT_DEF_FILE_NAME,
	XML_MOD_GAME_VERSION_ARGUMENT_EXTRA_DEF_FILE_NAME,
	XML_MOD_GAME_VERSION_ARGUMENT_MAP_FILE_NAME,
	XML_MOD_GAME_VERSION_ARGUMENT_EPISODE_NAME,
	XML_MOD_GAME_VERSION_ARGUMENT_LEVEL_NAME,
	XML_MOD_GAME_VERSION_ARGUMENT_SKILL_NAME,
	XML_MOD_GAME_VERSION_ARGUMENT_RECORD_DEMO_NAME,
	XML_MOD_GAME_VERSION_ARGUMENT_PLAY_DEMO_NAME,
	XML_MOD_GAME_VERSION_ARGUMENT_RESPAWN_MODE_NAME,
	XML_MOD_GAME_VERSION_ARGUMENT_WEAPON_SWITCH_ORDER_NAME,
	XML_MOD_GAME_VERSION_ARGUMENT_DISABLE_MONSTERS_NAME,
	XML_MOD_GAME_VERSION_ARGUMENT_DISABLE_SOUND_NAME,
	XML_MOD_GAME_VERSION_ARGUMENT_DISABLE_MUSIC_NAME
};

static const std::string JSON_MOD_ID_PROPERTY_NAME("id");
static const std::string JSON_MOD_NAME_PROPERTY_NAME("name");
static const std::string JSON_MOD_GAME_VERSION_GAME_VERSION_PROPERTY_NAME("gameVersion");
static const std::string JSON_MOD_GAME_VERSION_CONVERTED_PROPERTY_NAME("converted");
static const std::string JSON_MOD_GAME_VERSION_FILES_PROPERTY_NAME("files");
static const std::string JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME("properties");
static const std::string JSON_MOD_GAME_VERSION_PROPERTY_BASE_PROPERTY_NAME("base");
static const std::string JSON_MOD_GAME_VERSION_PROPERTY_GAME_EXECUTABLE_NAME_PROPERTY_NAME("gameExecutableName");
static const std::string JSON_MOD_GAME_VERSION_PROPERTY_SETUP_EXECUTABLE_NAME_PROPERTY_NAME("setupExecutableName");
static const std::string JSON_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_FILE_NAME_PROPERTY_NAME("gameConfigurationFileName");
static const std::string JSON_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_DIRECTORY_PATH_PROPERTY_NAME("gameConfigurationDirectoryPath");
static const std::string JSON_MOD_GAME_VERSION_PROPERTY_SKILL_START_VALUE_PROPERTY_NAME("skillStartValue");
static const std::string JSON_MOD_GAME_VERSION_PROPERTY_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME("localWorkingDirectory");
static const std::string JSON_MOD_GAME_VERSION_PROPERTY_SCRIPT_FILES_READ_FROM_GROUP_PROPERTY_NAME("scriptFilesReadFromGroup");
static const std::string JSON_MOD_GAME_VERSION_PROPERTY_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME("supportsSubdirectories");
static const std::string JSON_MOD_GAME_VERSION_PROPERTY_REQUIRES_GROUP_FILE_EXTRACTION_PROPERTY_NAME("requiresGroupFileExtraction");
static const std::string JSON_MOD_GAME_VERSION_ARGUMENTS_PROPERTY_NAME("arguments");
static const std::string JSON_MOD_GAME_VERSION_ARGUMENT_CON_FILE_PROPERTY_NAME("conFile");
static const std::string JSON_MOD_GAME_VERSION_ARGUMENT_EXTRA_CON_FILE_PROPERTY_NAME("extraConFile");
static const std::string JSON_MOD_GAME_VERSION_ARGUMENT_GROUP_FILE_PROPERTY_NAME("groupFile");
static const std::string JSON_MOD_GAME_VERSION_ARGUMENT_DEF_FILE_PROPERTY_NAME("defFile");
static const std::string JSON_MOD_GAME_VERSION_ARGUMENT_EXTRA_DEF_FILE_PROPERTY_NAME("extraDefFile");
static const std::string JSON_MOD_GAME_VERSION_ARGUMENT_MAP_FILE_PROPERTY_NAME("mapFile");
static const std::string JSON_MOD_GAME_VERSION_ARGUMENT_EPISODE_PROPERTY_NAME("episode");
static const std::string JSON_MOD_GAME_VERSION_ARGUMENT_LEVEL_PROPERTY_NAME("level");
static const std::string JSON_MOD_GAME_VERSION_ARGUMENT_SKILL_PROPERTY_NAME("skill");
static const std::string JSON_MOD_GAME_VERSION_ARGUMENT_RECORD_DEMO_PROPERTY_NAME("recordDemo");
static const std::string JSON_MOD_GAME_VERSION_ARGUMENT_PLAY_DEMO_PROPERTY_NAME("playDemo");
static const std::string JSON_MOD_GAME_VERSION_ARGUMENT_RESPAWN_MODE_PROPERTY_NAME("respawnMode");
static const std::string JSON_MOD_GAME_VERSION_ARGUMENT_WEAPON_SWITCH_ORDER_PROPERTY_NAME("weaponSwitchOrder");
static const std::string JSON_MOD_GAME_VERSION_ARGUMENT_DISABLE_MONSTERS_PROPERTY_NAME("disableMonsters");
static const std::string JSON_MOD_GAME_VERSION_ARGUMENT_DISABLE_SOUND_PROPERTY_NAME("disableSound");
static const std::string JSON_MOD_GAME_VERSION_ARGUMENT_DISABLE_MUSIC_PROPERTY_NAME("disableMusic");
static const std::string JSON_MOD_GAME_VERSION_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME("supportedOperatingSystems");
static const std::array<std::string_view, 6> JSON_MOD_GAME_VERSION_ATTRIBUTE_NAMES = {
	JSON_MOD_GAME_VERSION_GAME_VERSION_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_CONVERTED_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_ARGUMENTS_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_FILES_PROPERTY_NAME
};

static const std::array<std::string_view, 10> JSON_MOD_GAME_VERSION_PROPERTY_NAMES = {
	JSON_MOD_GAME_VERSION_PROPERTY_BASE_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_PROPERTY_GAME_EXECUTABLE_NAME_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_PROPERTY_SETUP_EXECUTABLE_NAME_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_FILE_NAME_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_DIRECTORY_PATH_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_PROPERTY_SKILL_START_VALUE_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_PROPERTY_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_PROPERTY_SCRIPT_FILES_READ_FROM_GROUP_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_PROPERTY_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_PROPERTY_REQUIRES_GROUP_FILE_EXTRACTION_PROPERTY_NAME
};

static const std::array<std::string_view, 16> JSON_MOD_GAME_VERSION_ARGUMENT_NAMES = {
	JSON_MOD_GAME_VERSION_ARGUMENT_CON_FILE_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_ARGUMENT_EXTRA_CON_FILE_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_ARGUMENT_GROUP_FILE_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_ARGUMENT_DEF_FILE_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_ARGUMENT_EXTRA_DEF_FILE_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_ARGUMENT_MAP_FILE_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_ARGUMENT_EPISODE_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_ARGUMENT_LEVEL_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_ARGUMENT_SKILL_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_ARGUMENT_RECORD_DEMO_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_ARGUMENT_PLAY_DEMO_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_ARGUMENT_RESPAWN_MODE_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_ARGUMENT_WEAPON_SWITCH_ORDER_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_ARGUMENT_DISABLE_MONSTERS_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_ARGUMENT_DISABLE_SOUND_PROPERTY_NAME,
	JSON_MOD_GAME_VERSION_ARGUMENT_DISABLE_MUSIC_PROPERTY_NAME
};

ModGameVersion::ModGameVersion(const std::string & gameVersionID, bool converted)
	: m_gameVersionID(Utilities::trimString(gameVersionID))
	, m_converted(converted)
	, m_parentModVersionType(nullptr) { }

ModGameVersion::ModGameVersion(ModGameVersion && m) noexcept
	: m_gameVersionID(std::move(m.m_gameVersionID))
	, m_converted(m.m_converted)
	, m_files(std::move(m.m_files))
	, m_parentModVersionType(nullptr) {
	updateParent();
}

ModGameVersion::ModGameVersion(const ModGameVersion & m)
	: m_gameVersionID(m.m_gameVersionID)
	, m_converted(m.m_converted)
	, m_parentModVersionType(nullptr) {
	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m.m_files.begin(); i != m.m_files.end(); ++i) {
		m_files.emplace_back(std::make_shared<ModFile>(**i));
	}

	updateParent();
}

ModGameVersion & ModGameVersion::operator = (ModGameVersion && m) noexcept {
	if(this != &m) {
		m_gameVersionID = std::move(m.m_gameVersionID);
		m_converted = m.m_converted;
		m_files = std::move(m.m_files);

		updateParent();
	}

	return *this;
}

ModGameVersion & ModGameVersion::operator = (const ModGameVersion & m) {
	m_files.clear();

	m_gameVersionID = m.m_gameVersionID;
	m_converted = m.m_converted;

	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m.m_files.begin(); i != m.m_files.end(); ++i) {
		m_files.emplace_back(std::make_shared<ModFile>(**i));
	}

	updateParent();

	return *this;
}

ModGameVersion::~ModGameVersion() {
	m_parentModVersionType = nullptr;
}

const std::string & ModGameVersion::getGameVersionID() const {
	return m_gameVersionID;
}

std::shared_ptr<GameVersion> ModGameVersion::getStandAloneGameVersion() const {
	return m_standAloneGameVersion;
}

std::string ModGameVersion::getFullName(bool includeGameVersionID = true) const {
	const Mod * parentMod = getParentMod();
	const ModVersion * parentModVersion = getParentModVersion();

	if(!Mod::isValid(parentMod, true) || parentModVersion == nullptr || m_parentModVersionType == nullptr) {
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

	if(includeGameVersionID && m_standAloneGameVersion == nullptr) {
		fullModName << " (" << m_gameVersionID << ")";
	}

	return fullModName.str();
}

bool ModGameVersion::isStandAlone() const {
	return m_standAloneGameVersion != nullptr;
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

void ModGameVersion::setGameVersionID(const std::string & gameVersionID) {
	m_gameVersionID = Utilities::trimString(gameVersionID);
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
		if(Utilities::areStringsEqualIgnoreCase((*i)->getType(), fileType)) {
			fileCount++;
		}
	}

	return fileCount;
}

bool ModGameVersion::hasFile(const ModFile & file) const {
	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), file.getFileName())) {
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
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), fileName)) {
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
		if(Utilities::areStringsEqualIgnoreCase((*i)->getType(), fileType)) {
			return true;
		}
	}

	return false;
}

size_t ModGameVersion::indexOfFile(const ModFile & file) const {
	for(size_t i = 0; i < m_files.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_files[i]->getFileName(), file.getFileName())) {
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
		if(Utilities::areStringsEqualIgnoreCase(m_files[i]->getFileName(), fileName)) {
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
		if(Utilities::areStringsEqualIgnoreCase(m_files[i]->getType(), fileType)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t ModGameVersion::indexOfLastFileOfType(const std::string & fileType) const {
	if(fileType.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(std::vector<std::shared_ptr<ModFile>>::const_reverse_iterator i = m_files.crbegin(); i != m_files.crend(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getType(), fileType)) {
			return m_files.crend() - i - 1;
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
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), fileName)) {
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
		if(Utilities::areStringsEqualIgnoreCase((*i)->getType(), fileType)) {
			return *i;
		}
	}

	return nullptr;
}

std::shared_ptr<ModFile> ModGameVersion::getLastFileOfType(const std::string & fileType) const {
	if(fileType.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<ModFile>>::const_reverse_iterator i = m_files.crbegin(); i != m_files.crend(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getType(), fileType)) {
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
		if(Utilities::areStringsEqualIgnoreCase((*i)->getType(), fileType)) {
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
		if(Utilities::areStringsEqualIgnoreCase((*i)->getType(), fileType)) {
			return (*i)->getFileName();
		}
	}

	return {};
}

std::optional<std::string> ModGameVersion::getLastFileNameOfType(const std::string & fileType) const {
	if(fileType.empty()) {
		return {};
	}

	for(std::vector<std::shared_ptr<ModFile>>::const_reverse_iterator i = m_files.crbegin(); i != m_files.crend(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getType(), fileType)) {
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
		if(Utilities::areStringsEqualIgnoreCase((*i)->getType(), fileType)) {
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

	std::shared_ptr<ModFile> newModFile(std::make_shared<ModFile>(file));
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
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), file.getFileName())) {
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
		if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), fileName)) {
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

	std::vector<std::shared_ptr<ModFile>> modFilesToRemove;
	size_t numberOfFilesRemoved = 0;

	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getType(), fileType)) {
			modFilesToRemove.push_back(*i);
		}
	}

	for(const std::shared_ptr<ModFile> & modFile : modFilesToRemove) {
		modFile->setParentModGameVersion(nullptr);

		std::vector<std::shared_ptr<ModFile>>::const_iterator modFileIterator(std::find(m_files.cbegin(), m_files.cend(), modFile));

		if(modFileIterator == m_files.cend()) {
			continue;
		}

		m_files.erase(modFileIterator);
		numberOfFilesRemoved++;
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

	rapidjson::Value gameVersionValue(m_gameVersionID.c_str(), allocator);
	modGameVersionValue.AddMember(rapidjson::StringRef(JSON_MOD_GAME_VERSION_GAME_VERSION_PROPERTY_NAME.c_str()), gameVersionValue, allocator);

	modGameVersionValue.AddMember(rapidjson::StringRef(JSON_MOD_GAME_VERSION_CONVERTED_PROPERTY_NAME.c_str()), rapidjson::Value(m_converted), allocator);

	if(isStandAlone()) {
		rapidjson::Value propertiesValue(rapidjson::kObjectType);

		std::vector<std::pair<std::string, std::optional<std::string>>> stringProperties({
			{ JSON_MOD_GAME_VERSION_PROPERTY_BASE_PROPERTY_NAME,                               m_standAloneGameVersion->getBase() },
			{ JSON_MOD_GAME_VERSION_PROPERTY_GAME_EXECUTABLE_NAME_PROPERTY_NAME,               m_standAloneGameVersion->getGameExecutableName() },
			{ JSON_MOD_GAME_VERSION_PROPERTY_SETUP_EXECUTABLE_NAME_PROPERTY_NAME,              m_standAloneGameVersion->getSetupExecutableName() },
			{ JSON_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_FILE_NAME_PROPERTY_NAME,       m_standAloneGameVersion->getGameConfigurationFileName() },
			{ JSON_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_DIRECTORY_PATH_PROPERTY_NAME,  m_standAloneGameVersion->getGameConfigurationDirectoryPath() }
		});

		for(const std::pair<std::string, std::optional<std::string>> & stringProperty : stringProperties) {
			if(!stringProperty.second.has_value()) {
				continue;
			}

			rapidjson::Value propertyName(stringProperty.first.c_str(), allocator);
			rapidjson::Value propertyValue(stringProperty.second.value().c_str(), allocator);
			propertiesValue.AddMember(propertyName, propertyValue, allocator);
		}

		propertiesValue.AddMember(rapidjson::StringRef(JSON_MOD_GAME_VERSION_PROPERTY_SKILL_START_VALUE_PROPERTY_NAME.c_str()), rapidjson::Value(m_standAloneGameVersion->getSkillStartValue()), allocator);

		std::vector<std::pair<std::string, std::optional<bool>>> booleanProperties({
			{ JSON_MOD_GAME_VERSION_PROPERTY_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME,          m_standAloneGameVersion->hasLocalWorkingDirectory() },
			{ JSON_MOD_GAME_VERSION_PROPERTY_SCRIPT_FILES_READ_FROM_GROUP_PROPERTY_NAME,     m_standAloneGameVersion->getScriptFilesReadFromGroup() },
			{ JSON_MOD_GAME_VERSION_PROPERTY_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME,          m_standAloneGameVersion->doesSupportSubdirectories() },
			{ JSON_MOD_GAME_VERSION_PROPERTY_REQUIRES_GROUP_FILE_EXTRACTION_PROPERTY_NAME,   m_standAloneGameVersion->getRequiresGroupFileExtraction() }
		});

		for(const std::pair<std::string, std::optional<bool>> & booleanProperty : booleanProperties) {
			if(!booleanProperty.second.has_value()) {
				continue;
			}

			rapidjson::Value propertyName(booleanProperty.first.c_str(), allocator);
			propertiesValue.AddMember(propertyName, rapidjson::Value(booleanProperty.second.value()), allocator);
		}

		modGameVersionValue.AddMember(rapidjson::StringRef(JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME.c_str()), propertiesValue, allocator);

		rapidjson::Value argumentsValue(rapidjson::kObjectType);

		std::vector<std::pair<std::string, std::optional<std::string>>> arguments({
			{ JSON_MOD_GAME_VERSION_ARGUMENT_CON_FILE_PROPERTY_NAME,            m_standAloneGameVersion->getConFileArgumentFlag() },
			{ JSON_MOD_GAME_VERSION_ARGUMENT_EXTRA_CON_FILE_PROPERTY_NAME,      m_standAloneGameVersion->getExtraConFileArgumentFlag() },
			{ JSON_MOD_GAME_VERSION_ARGUMENT_GROUP_FILE_PROPERTY_NAME,          m_standAloneGameVersion->getGroupFileArgumentFlag() },
			{ JSON_MOD_GAME_VERSION_ARGUMENT_DEF_FILE_PROPERTY_NAME,            m_standAloneGameVersion->getDefFileArgumentFlag() },
			{ JSON_MOD_GAME_VERSION_ARGUMENT_EXTRA_DEF_FILE_PROPERTY_NAME,      m_standAloneGameVersion->getExtraDefFileArgumentFlag() },
			{ JSON_MOD_GAME_VERSION_ARGUMENT_MAP_FILE_PROPERTY_NAME,            m_standAloneGameVersion->getMapFileArgumentFlag() },
			{ JSON_MOD_GAME_VERSION_ARGUMENT_EPISODE_PROPERTY_NAME,             m_standAloneGameVersion->getEpisodeArgumentFlag() },
			{ JSON_MOD_GAME_VERSION_ARGUMENT_LEVEL_PROPERTY_NAME,               m_standAloneGameVersion->getLevelArgumentFlag() },
			{ JSON_MOD_GAME_VERSION_ARGUMENT_SKILL_PROPERTY_NAME,               m_standAloneGameVersion->getSkillArgumentFlag() },
			{ JSON_MOD_GAME_VERSION_ARGUMENT_RECORD_DEMO_PROPERTY_NAME,         m_standAloneGameVersion->getRecordDemoArgumentFlag() },
			{ JSON_MOD_GAME_VERSION_ARGUMENT_PLAY_DEMO_PROPERTY_NAME,           m_standAloneGameVersion->getPlayDemoArgumentFlag() },
			{ JSON_MOD_GAME_VERSION_ARGUMENT_RESPAWN_MODE_PROPERTY_NAME,        m_standAloneGameVersion->getRespawnModeArgumentFlag() },
			{ JSON_MOD_GAME_VERSION_ARGUMENT_WEAPON_SWITCH_ORDER_PROPERTY_NAME, m_standAloneGameVersion->getWeaponSwitchOrderArgumentFlag() },
			{ JSON_MOD_GAME_VERSION_ARGUMENT_DISABLE_MONSTERS_PROPERTY_NAME,    m_standAloneGameVersion->getDisableMonstersArgumentFlag() },
			{ JSON_MOD_GAME_VERSION_ARGUMENT_DISABLE_SOUND_PROPERTY_NAME,       m_standAloneGameVersion->getDisableSoundArgumentFlag() },
			{ JSON_MOD_GAME_VERSION_ARGUMENT_DISABLE_MUSIC_PROPERTY_NAME,       m_standAloneGameVersion->getDisableMusicArgumentFlag() }
		});

		for(const std::pair<std::string, std::optional<std::string>> & argument : arguments) {
			if(!argument.second.has_value()) {
				continue;
			}

			rapidjson::Value argumentName(argument.first.c_str(), allocator);
			rapidjson::Value argumentFlag(argument.second.value().c_str(), allocator);
			argumentsValue.AddMember(argumentName, argumentFlag, allocator);
		}

		modGameVersionValue.AddMember(rapidjson::StringRef(JSON_MOD_GAME_VERSION_ARGUMENTS_PROPERTY_NAME.c_str()), argumentsValue, allocator);

		rapidjson::Value operatingSystemsValue(rapidjson::kArrayType);
		operatingSystemsValue.Reserve(m_standAloneGameVersion->numberOfSupportedOperatingSystems(), allocator);

		for(size_t i = 0; i < m_standAloneGameVersion->numberOfSupportedOperatingSystems(); i++) {
			rapidjson::Value operatingSystemNameValue(m_standAloneGameVersion->getSupportedOperatingSystemName(i).c_str(), allocator);
			operatingSystemsValue.PushBack(operatingSystemNameValue, allocator);
		}

		modGameVersionValue.AddMember(rapidjson::StringRef(JSON_MOD_GAME_VERSION_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME.c_str()), operatingSystemsValue, allocator);
	}

	rapidjson::Value filesValue(rapidjson::kArrayType);
	filesValue.Reserve(m_files.size(), allocator);

	for(const std::shared_ptr<ModFile> & file : m_files) {
		filesValue.PushBack(file->toJSON(allocator), allocator);
	}

	modGameVersionValue.AddMember(rapidjson::StringRef(JSON_MOD_GAME_VERSION_FILES_PROPERTY_NAME.c_str()), filesValue, allocator);

	return modGameVersionValue;
}

tinyxml2::XMLElement * ModGameVersion::toXML(tinyxml2::XMLDocument * document) const {
	if(document == nullptr) {
		return nullptr;
	}

	tinyxml2::XMLElement * modGameVersionElement = document->NewElement(XML_MOD_GAME_VERSION_ELEMENT_NAME.c_str());

	modGameVersionElement->SetAttribute(XML_MOD_GAME_VERSION_GAME_VERSION_ATTRIBUTE_NAME.c_str(), m_gameVersionID.c_str());
	modGameVersionElement->SetAttribute(XML_MOD_GAME_VERSION_CONVERTED_ATTRIBUTE_NAME.c_str(), m_converted ? "Converted" : "Native");

	tinyxml2::XMLElement * fileContainerElement = nullptr;

	if(isStandAlone()) {
		tinyxml2::XMLElement * propertiesElement = document->NewElement(XML_MOD_GAME_VERSION_PROPERTIES_ELEMENT_NAME.c_str());

		std::vector<std::pair<std::string, std::optional<std::string>>> stringProperties({
			{ XML_MOD_GAME_VERSION_PROPERTY_BASE_NAME,                              m_standAloneGameVersion->getBase() },
			{ XML_MOD_GAME_VERSION_PROPERTY_GAME_EXE_NAME_NAME,                     m_standAloneGameVersion->getGameExecutableName() },
			{ XML_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_FILE_NAME_NAME,      m_standAloneGameVersion->getGameConfigurationFileName() },
			{ XML_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_DIRECTORY_PATH_NAME, m_standAloneGameVersion->getGameConfigurationDirectoryPath() },
			{ XML_MOD_GAME_VERSION_PROPERTY_SKILL_START_VALUE_NAME,                 std::to_string(m_standAloneGameVersion->getSkillStartValue()) }
		});

		for(const std::pair<std::string, std::optional<std::string>> & stringProperty : stringProperties) {
			if(!stringProperty.second.has_value()) {
				continue;
			}

			propertiesElement->SetAttribute(stringProperty.first.c_str(), stringProperty.second.value().c_str());
		}

		std::vector<std::pair<std::string, std::optional<bool>>> booleanProperties({
			{ XML_MOD_GAME_VERSION_PROPERTY_LOCAL_WORKING_DIRECTORY_NAME,        m_standAloneGameVersion->hasLocalWorkingDirectory() },
			{ XML_MOD_GAME_VERSION_PROPERTY_SCRIPT_FILES_READ_FROM_GROUP_NAME,   m_standAloneGameVersion->getScriptFilesReadFromGroup() },
			{ XML_MOD_GAME_VERSION_PROPERTY_SUPPORTS_SUBDIRECTORIES_NAME,        m_standAloneGameVersion->doesSupportSubdirectories() },
			{ XML_MOD_GAME_VERSION_PROPERTY_REQUIRES_GROUP_FILE_EXTRACTION_NAME, m_standAloneGameVersion->getRequiresGroupFileExtraction() }
		});

		for(const std::pair<std::string, std::optional<bool>> & booleanProperty : booleanProperties) {
			if(!booleanProperty.second.has_value()) {
				continue;
			}

			propertiesElement->SetAttribute(booleanProperty.first.c_str(), booleanProperty.second.value());
		}

		modGameVersionElement->InsertEndChild(propertiesElement);

		tinyxml2::XMLElement * argumentsElement = document->NewElement(XML_MOD_GAME_VERSION_ARGUMENTS_ELEMENT_NAME.c_str());

		std::vector<std::pair<std::string, std::optional<std::string>>> arguments({
			{ XML_MOD_GAME_VERSION_ARGUMENT_CON_FILE_NAME,            m_standAloneGameVersion->getConFileArgumentFlag() },
			{ XML_MOD_GAME_VERSION_ARGUMENT_EXTRA_CON_FILE_NAME,      m_standAloneGameVersion->getExtraConFileArgumentFlag() },
			{ XML_MOD_GAME_VERSION_ARGUMENT_GROUP_FILE_NAME,          m_standAloneGameVersion->getGroupFileArgumentFlag() },
			{ XML_MOD_GAME_VERSION_ARGUMENT_DEF_FILE_NAME,            m_standAloneGameVersion->getDefFileArgumentFlag() },
			{ XML_MOD_GAME_VERSION_ARGUMENT_EXTRA_DEF_FILE_NAME,      m_standAloneGameVersion->getExtraDefFileArgumentFlag() },
			{ XML_MOD_GAME_VERSION_ARGUMENT_MAP_FILE_NAME,            m_standAloneGameVersion->getMapFileArgumentFlag() },
			{ XML_MOD_GAME_VERSION_ARGUMENT_EPISODE_NAME,             m_standAloneGameVersion->getEpisodeArgumentFlag() },
			{ XML_MOD_GAME_VERSION_ARGUMENT_LEVEL_NAME,               m_standAloneGameVersion->getLevelArgumentFlag() },
			{ XML_MOD_GAME_VERSION_ARGUMENT_SKILL_NAME,               m_standAloneGameVersion->getSkillArgumentFlag() },
			{ XML_MOD_GAME_VERSION_ARGUMENT_RECORD_DEMO_NAME,         m_standAloneGameVersion->getRecordDemoArgumentFlag() },
			{ XML_MOD_GAME_VERSION_ARGUMENT_PLAY_DEMO_NAME,           m_standAloneGameVersion->getPlayDemoArgumentFlag() },
			{ XML_MOD_GAME_VERSION_ARGUMENT_RESPAWN_MODE_NAME,        m_standAloneGameVersion->getRespawnModeArgumentFlag() },
			{ XML_MOD_GAME_VERSION_ARGUMENT_WEAPON_SWITCH_ORDER_NAME, m_standAloneGameVersion->getWeaponSwitchOrderArgumentFlag() },
			{ XML_MOD_GAME_VERSION_ARGUMENT_DISABLE_MONSTERS_NAME,    m_standAloneGameVersion->getDisableMonstersArgumentFlag() },
			{ XML_MOD_GAME_VERSION_ARGUMENT_DISABLE_SOUND_NAME,       m_standAloneGameVersion->getDisableSoundArgumentFlag() },
			{ XML_MOD_GAME_VERSION_ARGUMENT_DISABLE_MUSIC_NAME,       m_standAloneGameVersion->getDisableMusicArgumentFlag() }
		});

		for(const std::pair<std::string, std::optional<std::string>> & argument : arguments) {
			if(!argument.second.has_value()) {
				continue;
			}

			tinyxml2::XMLElement * argumentElement = document->NewElement(XML_MOD_GAME_VERSION_ARGUMENT_ELEMENT_NAME.c_str());

			argumentElement->SetAttribute(XML_MOD_GAME_VERSION_ARGUMENT_NAME_PROPERTY_NAME.c_str(), argument.first.c_str());
			argumentElement->SetAttribute(XML_MOD_GAME_VERSION_ARGUMENT_FLAG_PROPERTY_NAME.c_str(), argument.second.value().c_str());

			argumentsElement->InsertEndChild(argumentElement);
		}

		modGameVersionElement->InsertEndChild(argumentsElement);

		tinyxml2::XMLElement * operatingSystemsElement = document->NewElement(XML_MOD_GAME_VERSION_OPERATING_SYSTEMS_ELEMENT_NAME.c_str());

		for(size_t i = 0; i < m_standAloneGameVersion->numberOfSupportedOperatingSystems(); i++) {
			tinyxml2::XMLElement * operatingSystemElement = document->NewElement(XML_MOD_GAME_VERSION_OPERATING_SYSTEM_ELEMENT_NAME.c_str());

			operatingSystemElement->SetAttribute(XML_MOD_GAME_VERSION_OPERATING_SYSTEM_NAME_ATTRIBUTE_NAME.c_str(), m_standAloneGameVersion->getSupportedOperatingSystemName(i).c_str());

			operatingSystemsElement->InsertEndChild(operatingSystemElement);
		}

		modGameVersionElement->InsertEndChild(operatingSystemsElement);

		fileContainerElement = document->NewElement(XML_MOD_GAME_VERSION_FILES_ELEMENT_NAME.c_str());
		modGameVersionElement->InsertEndChild(fileContainerElement);
	}
	else {
		fileContainerElement = modGameVersionElement;
	}

	for(const std::shared_ptr<ModFile> file : m_files) {
		fileContainerElement->InsertEndChild(file->toXML(document));
	}

	return modGameVersionElement;
}

std::unique_ptr<ModGameVersion> ModGameVersion::parseFrom(const rapidjson::Value & modGameVersionValue, const rapidjson::Value & modValue, bool skipFileInfoValidation) {
	if(!modGameVersionValue.IsObject()) {
		spdlog::error("Invalid mod game version type: '{}', expected 'object'.", Utilities::typeToString(modGameVersionValue.GetType()));
		return nullptr;
	}

	// check for unhandled mod game version properties
	bool propertyHandled = false;

	for(rapidjson::Value::ConstMemberIterator i = modGameVersionValue.MemberBegin(); i != modGameVersionValue.MemberEnd(); ++i) {
		propertyHandled = false;

		for(const std::string_view attributeName : JSON_MOD_GAME_VERSION_ATTRIBUTE_NAMES) {
			if(i->name.GetString() == attributeName) {
				propertyHandled = true;
				break;
			}
		}

		if(!propertyHandled) {
			spdlog::warn("Mod game version has unexpected property '{}'.", i->name.GetString());
		}
	}

	// parse the mod game version game version property
	if(!modGameVersionValue.HasMember(JSON_MOD_GAME_VERSION_GAME_VERSION_PROPERTY_NAME.c_str())) {
		spdlog::error("Mod game version is missing '{}' property.", JSON_MOD_GAME_VERSION_GAME_VERSION_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modGameVersionGameVersionValue = modGameVersionValue[JSON_MOD_GAME_VERSION_GAME_VERSION_PROPERTY_NAME.c_str()];

	if(!modGameVersionGameVersionValue.IsString()) {
		spdlog::error("Mod game version '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_GAME_VERSION_GAME_VERSION_PROPERTY_NAME, Utilities::typeToString(modGameVersionGameVersionValue.GetType()));
		return nullptr;
	}

	std::string modGameVersionGameVersion(modGameVersionGameVersionValue.GetString());

	bool standAlone = Utilities::areStringsEqualIgnoreCase(modGameVersionGameVersion, GameVersion::STANDALONE);

	// parse the mod game version converted property
	if(!modGameVersionValue.HasMember(JSON_MOD_GAME_VERSION_CONVERTED_PROPERTY_NAME.c_str())) {
		spdlog::error("Mod game version is missing '{}' property.", JSON_MOD_GAME_VERSION_CONVERTED_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modVersionConvertedValue = modGameVersionValue[JSON_MOD_GAME_VERSION_CONVERTED_PROPERTY_NAME.c_str()];

	if(!modVersionConvertedValue.IsBool()) {
		spdlog::error("Mod game version '{}' property has invalid type: '{}', expected 'boolean'.", JSON_MOD_GAME_VERSION_CONVERTED_PROPERTY_NAME, Utilities::typeToString(modVersionConvertedValue.GetType()));
		return nullptr;
	}

	bool converted = modVersionConvertedValue.GetBool();

	// initialize the mod game version
	std::unique_ptr<ModGameVersion> newModGameVersion = std::make_unique<ModGameVersion>(modGameVersionGameVersion, converted);

	if(standAlone) {
		std::shared_ptr<GameVersion> standAloneGameVersion(std::make_unique<GameVersion>());
		standAloneGameVersion->setStandAlone(true);
		newModGameVersion->m_standAloneGameVersion = standAloneGameVersion;
		std::map<std::string, std::string> arguments;

		if(!modGameVersionValue.HasMember(JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME.c_str())) {
			spdlog::error("Stand-alone mod game version is missing '{}' property.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME);
			return nullptr;
		}

		const rapidjson::Value & propertiesValue = modGameVersionValue[JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME.c_str()];

		if(!propertiesValue.IsObject()) {
			spdlog::error("Stand-alone mod game version '{}' property has invalid type: '{}', expected 'object'.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, Utilities::typeToString(propertiesValue.GetType()));
			return nullptr;
		}

		for(rapidjson::Value::ConstMemberIterator i = propertiesValue.MemberBegin(); i != propertiesValue.MemberEnd(); ++i) {
			auto acceptedPropertyNameIterator = std::find(JSON_MOD_GAME_VERSION_PROPERTY_NAMES.cbegin(), JSON_MOD_GAME_VERSION_PROPERTY_NAMES.cend(), i->name.GetString());

			if(acceptedPropertyNameIterator == JSON_MOD_GAME_VERSION_PROPERTY_NAMES.cend()) {
				spdlog::warn("Stand-alone mod game version '{}' property has unexpected property: '{}'.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, i->name.GetString());
			}
		}

		// read base property
		if(!propertiesValue.HasMember(JSON_MOD_GAME_VERSION_PROPERTY_BASE_PROPERTY_NAME.c_str())) {
			spdlog::error("Stand-alone mod game version '{}' is missing '{}' property.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, JSON_MOD_GAME_VERSION_PROPERTY_BASE_PROPERTY_NAME);
			return nullptr;
		}

		const rapidjson::Value & baseValue = propertiesValue[JSON_MOD_GAME_VERSION_PROPERTY_BASE_PROPERTY_NAME.c_str()];

		if(!baseValue.IsString()) {
			spdlog::error("Stand-alone mod game version '{}' '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, JSON_MOD_GAME_VERSION_PROPERTY_BASE_PROPERTY_NAME, Utilities::typeToString(baseValue.GetType()));
			return nullptr;
		}

		std::string base(baseValue.GetString());

		if(base.empty()) {
			spdlog::error("Stand-alone mod game version '{}' has empty '{}' property value.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, JSON_MOD_GAME_VERSION_PROPERTY_BASE_PROPERTY_NAME);
			return nullptr;
		}

		standAloneGameVersion->setBase(base);

		// read game executable name property
		if(!propertiesValue.HasMember(JSON_MOD_GAME_VERSION_PROPERTY_GAME_EXECUTABLE_NAME_PROPERTY_NAME.c_str())) {
			spdlog::error("Stand-alone mod game version '{}' is missing '{}' property.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, JSON_MOD_GAME_VERSION_PROPERTY_GAME_EXECUTABLE_NAME_PROPERTY_NAME);
			return nullptr;
		}

		const rapidjson::Value & gameExecutableNameValue = propertiesValue[JSON_MOD_GAME_VERSION_PROPERTY_GAME_EXECUTABLE_NAME_PROPERTY_NAME.c_str()];

		if(!gameExecutableNameValue.IsString()) {
			spdlog::error("Stand-alone mod game version '{}' '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, JSON_MOD_GAME_VERSION_PROPERTY_GAME_EXECUTABLE_NAME_PROPERTY_NAME, Utilities::typeToString(gameExecutableNameValue.GetType()));
			return nullptr;
		}

		std::string gameExecutableName(gameExecutableNameValue.GetString());

		if(gameExecutableName.empty()) {
			spdlog::error("Stand-alone mod game version '{}' has empty '{}' property value.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, JSON_MOD_GAME_VERSION_PROPERTY_GAME_EXECUTABLE_NAME_PROPERTY_NAME);
			return nullptr;
		}

		standAloneGameVersion->setGameExecutableName(gameExecutableName);

		// read setup executable name property
		if(propertiesValue.HasMember(JSON_MOD_GAME_VERSION_PROPERTY_SETUP_EXECUTABLE_NAME_PROPERTY_NAME.c_str())) {
			const rapidjson::Value & setupExecutableNameValue = propertiesValue[JSON_MOD_GAME_VERSION_PROPERTY_SETUP_EXECUTABLE_NAME_PROPERTY_NAME.c_str()];

			if(!setupExecutableNameValue.IsString()) {
				spdlog::error("Stand-alone mod game version '{}' '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, JSON_MOD_GAME_VERSION_PROPERTY_SETUP_EXECUTABLE_NAME_PROPERTY_NAME, Utilities::typeToString(setupExecutableNameValue.GetType()));
				return nullptr;
			}

			std::string setupExecutableName(setupExecutableNameValue.GetString());

			if(setupExecutableName.empty()) {
				spdlog::error("Stand-alone mod game version '{}' has empty '{}' property value.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, JSON_MOD_GAME_VERSION_PROPERTY_SETUP_EXECUTABLE_NAME_PROPERTY_NAME);
				return nullptr;
			}

			standAloneGameVersion->setSetupExecutableName(setupExecutableName);
		}

		// read game configuration file name property
		if(!propertiesValue.HasMember(JSON_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_FILE_NAME_PROPERTY_NAME.c_str())) {
			spdlog::error("Stand-alone mod game version '{}' is missing '{}' property.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, JSON_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_FILE_NAME_PROPERTY_NAME);
			return nullptr;
		}

		const rapidjson::Value & gameConfigurationFileNameValue = propertiesValue[JSON_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_FILE_NAME_PROPERTY_NAME.c_str()];

		if(!gameConfigurationFileNameValue.IsString()) {
			spdlog::error("Stand-alone mod game version '{}' '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, JSON_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_FILE_NAME_PROPERTY_NAME, Utilities::typeToString(gameConfigurationFileNameValue.GetType()));
			return nullptr;
		}

		std::string gameConfigurationFileName(gameConfigurationFileNameValue.GetString());

		if(gameConfigurationFileName.empty()) {
			spdlog::error("Stand-alone mod game version '{}' has empty '{}' property value.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, JSON_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_FILE_NAME_PROPERTY_NAME);
			return nullptr;
		}

		standAloneGameVersion->setGameConfigurationFileName(gameConfigurationFileName);

		// read game configuration directory path property
		if(!propertiesValue.HasMember(JSON_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_DIRECTORY_PATH_PROPERTY_NAME.c_str())) {
			spdlog::error("Stand-alone mod game version '{}' is missing '{}' property.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, JSON_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_DIRECTORY_PATH_PROPERTY_NAME);
			return nullptr;
		}

		const rapidjson::Value & gameConfigurationDirectoryPathValue = propertiesValue[JSON_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_DIRECTORY_PATH_PROPERTY_NAME.c_str()];

		if(!gameConfigurationDirectoryPathValue.IsString()) {
			spdlog::error("Stand-alone mod game version '{}' '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, JSON_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_DIRECTORY_PATH_PROPERTY_NAME, Utilities::typeToString(gameConfigurationDirectoryPathValue.GetType()));
			return nullptr;
		}

		standAloneGameVersion->setGameConfigurationDirectoryPath(gameConfigurationDirectoryPathValue.GetString());

		// read skill start value property
		if(propertiesValue.HasMember(JSON_MOD_GAME_VERSION_PROPERTY_SKILL_START_VALUE_PROPERTY_NAME.c_str())) {
			const rapidjson::Value & skillStartValueValue = propertiesValue[JSON_MOD_GAME_VERSION_PROPERTY_SKILL_START_VALUE_PROPERTY_NAME.c_str()];

			if(!skillStartValueValue.IsUint()) {
				spdlog::error("Stand-alone mod game version '{}' '{}' property has invalid type: '{}', expected unsigned integer 'number'.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, JSON_MOD_GAME_VERSION_PROPERTY_SKILL_START_VALUE_PROPERTY_NAME, Utilities::typeToString(skillStartValueValue.GetType()));
				return nullptr;
			}

			uint32_t skillStartValue = skillStartValueValue.GetUint();

			if(skillStartValue > std::numeric_limits<uint8_t>::max()) {
				spdlog::error("Stand-alone mod game version '{}' '{}' property value {} exceeds the maximum possible value of {}.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, JSON_MOD_GAME_VERSION_PROPERTY_SKILL_START_VALUE_PROPERTY_NAME, skillStartValue, std::numeric_limits<uint8_t>::max());
				return nullptr;
			}

			standAloneGameVersion->setSkillStartValue(static_cast<uint8_t>(skillStartValue));
		}

		// read local working directory property
		if(propertiesValue.HasMember(JSON_MOD_GAME_VERSION_PROPERTY_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME.c_str())) {
			const rapidjson::Value & localWorkingDirectoryValue = propertiesValue[JSON_MOD_GAME_VERSION_PROPERTY_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME.c_str()];

			if(!localWorkingDirectoryValue.IsBool()) {
				spdlog::error("Stand-alone mod game version '{}' '{}' property has invalid type: '{}', expected 'boolean'.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, JSON_MOD_GAME_VERSION_PROPERTY_LOCAL_WORKING_DIRECTORY_PROPERTY_NAME, Utilities::typeToString(localWorkingDirectoryValue.GetType()));
				return nullptr;
			}

			standAloneGameVersion->setLocalWorkingDirectory(localWorkingDirectoryValue.GetBool());
		}

		// read script files read from group property
		if(propertiesValue.HasMember(JSON_MOD_GAME_VERSION_PROPERTY_SCRIPT_FILES_READ_FROM_GROUP_PROPERTY_NAME.c_str())) {
			const rapidjson::Value & scriptFilesReadFromGroupValue = propertiesValue[JSON_MOD_GAME_VERSION_PROPERTY_SCRIPT_FILES_READ_FROM_GROUP_PROPERTY_NAME.c_str()];

			if(!scriptFilesReadFromGroupValue.IsBool()) {
				spdlog::error("Stand-alone mod game version '{}' '{}' property has invalid type: '{}', expected 'boolean'.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, JSON_MOD_GAME_VERSION_PROPERTY_SCRIPT_FILES_READ_FROM_GROUP_PROPERTY_NAME, Utilities::typeToString(scriptFilesReadFromGroupValue.GetType()));
				return nullptr;
			}

			standAloneGameVersion->setScriptFilesReadFromGroup(scriptFilesReadFromGroupValue.GetBool());
		}

		// read supports subdirectories property
		if(propertiesValue.HasMember(JSON_MOD_GAME_VERSION_PROPERTY_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME.c_str())) {
			const rapidjson::Value & supportsSubdirectoriesValue = propertiesValue[JSON_MOD_GAME_VERSION_PROPERTY_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME.c_str()];

			if(!supportsSubdirectoriesValue.IsBool()) {
				spdlog::error("Stand-alone mod game version '{}' '{}' property has invalid type: '{}', expected 'boolean'.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, JSON_MOD_GAME_VERSION_PROPERTY_SUPPORTS_SUBDIRECTORIES_PROPERTY_NAME, Utilities::typeToString(supportsSubdirectoriesValue.GetType()));
				return nullptr;
			}

			standAloneGameVersion->setSupportsSubdirectories(supportsSubdirectoriesValue.GetBool());
		}

		// read requires group file extraction property
		if(propertiesValue.HasMember(JSON_MOD_GAME_VERSION_PROPERTY_REQUIRES_GROUP_FILE_EXTRACTION_PROPERTY_NAME.c_str())) {
			const rapidjson::Value & requiresGroupFileExtractionValue = propertiesValue[JSON_MOD_GAME_VERSION_PROPERTY_REQUIRES_GROUP_FILE_EXTRACTION_PROPERTY_NAME.c_str()];

			if(!requiresGroupFileExtractionValue.IsBool()) {
				spdlog::error("Stand-alone mod game version '{}' '{}' property has invalid type: '{}', expected 'boolean'.", JSON_MOD_GAME_VERSION_PROPERTIES_PROPERTY_NAME, JSON_MOD_GAME_VERSION_PROPERTY_REQUIRES_GROUP_FILE_EXTRACTION_PROPERTY_NAME, Utilities::typeToString(requiresGroupFileExtractionValue.GetType()));
				return nullptr;
			}

			standAloneGameVersion->setRequiresGroupFileExtraction(requiresGroupFileExtractionValue.GetBool());
		}

		// read arguments
		if(modGameVersionValue.HasMember(JSON_MOD_GAME_VERSION_ARGUMENTS_PROPERTY_NAME.c_str())) {
			const rapidjson::Value & argumentsValue = modGameVersionValue[JSON_MOD_GAME_VERSION_ARGUMENTS_PROPERTY_NAME.c_str()];

			if(!argumentsValue.IsObject()) {
				spdlog::error("Stand-alone mod game version '{}' property has invalid type: '{}', expected 'object'.", JSON_MOD_GAME_VERSION_ARGUMENTS_PROPERTY_NAME, Utilities::typeToString(argumentsValue.GetType()));
				return nullptr;
			}

			for(rapidjson::Value::ConstMemberIterator i = argumentsValue.MemberBegin(); i != argumentsValue.MemberEnd(); ++i) {
				if(!i->value.IsString()) {
					spdlog::error("Stand-alone mod game version '{}' '{}' property has invalid type: '{}', expected 'string'.", JSON_MOD_GAME_VERSION_ARGUMENTS_PROPERTY_NAME, i->name.GetString(), Utilities::typeToString(i->value.GetType()));
					return nullptr;
				}

				auto acceptedArgumentNameIterator = std::find(JSON_MOD_GAME_VERSION_ARGUMENT_NAMES.cbegin(), JSON_MOD_GAME_VERSION_ARGUMENT_NAMES.cend(), i->name.GetString());

				if(acceptedArgumentNameIterator == JSON_MOD_GAME_VERSION_ARGUMENT_NAMES.cend()) {
					spdlog::warn("Stand-alone mod game version '{}' property has unexpected property: '{}'.", JSON_MOD_GAME_VERSION_ARGUMENTS_PROPERTY_NAME, i->name.GetString());
				}

				arguments[i->name.GetString()] = i->value.GetString();
			}
		}

		// read operating systems
		if(!modGameVersionValue.HasMember(JSON_MOD_GAME_VERSION_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME.c_str())) {
			spdlog::error("Stand-alone mod game version is missing '{}' property.", JSON_MOD_GAME_VERSION_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME);
			return nullptr;
		}

		const rapidjson::Value & supportedOperatingSystemsValue = modGameVersionValue[JSON_MOD_GAME_VERSION_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME.c_str()];

		if(!supportedOperatingSystemsValue.IsArray()) {
			spdlog::error("Stand-alone mod game version '{}' property has invalid type: '{}', expected 'array'.", JSON_MOD_GAME_VERSION_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME, Utilities::typeToString(supportedOperatingSystemsValue.GetType()));
			return nullptr;
		}

		for(rapidjson::Value::ConstValueIterator i = supportedOperatingSystemsValue.Begin(); i != supportedOperatingSystemsValue.End(); ++i) {
			const rapidjson::Value & supportedOperatingSystemValue = *i;

			if(!supportedOperatingSystemValue.IsString()) {
				spdlog::error("Stand-alone mod game version '{}' property entry #{} has invalid operating system name type: '{}', expected 'string'.", JSON_MOD_GAME_VERSION_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME, i - supportedOperatingSystemsValue.Begin() + 1, Utilities::typeToString(supportedOperatingSystemValue.GetType()));
				return nullptr;
			}

			std::optional<GameVersion::OperatingSystem> optionalOperatingSystem(magic_enum::enum_cast<GameVersion::OperatingSystem>(supportedOperatingSystemValue.GetString()));

			if(!optionalOperatingSystem.has_value()) {
				spdlog::error("Stand-alone mod game version '{}' property entry #{} has invalid operating system name value: '{}'.", JSON_MOD_GAME_VERSION_SUPPORTED_OPERATING_SYSTEMS_PROPERTY_NAME, i - supportedOperatingSystemsValue.Begin() + 1, supportedOperatingSystemValue.GetString());
				return nullptr;
			}

			standAloneGameVersion->addSupportedOperatingSystem(optionalOperatingSystem.value());
		}

		// parse mod id
		if(!modValue.HasMember(JSON_MOD_ID_PROPERTY_NAME.c_str())) {
			spdlog::error("Mod is missing '{}' property.", JSON_MOD_ID_PROPERTY_NAME);
			return nullptr;
		}

		const rapidjson::Value & modIDValue = modValue[JSON_MOD_ID_PROPERTY_NAME.c_str()];

		if(!modIDValue.IsString()) {
			spdlog::error("Mod has an invalid '{}' property type: '{}', expected 'string'.", JSON_MOD_ID_PROPERTY_NAME, Utilities::typeToString(modIDValue.GetType()));
			return nullptr;
		}

		std::string modID(Utilities::trimString(modIDValue.GetString()));

		if(modID.empty()) {
			spdlog::error("Mod '{}' property cannot be empty.", JSON_MOD_ID_PROPERTY_NAME);
			return nullptr;
		}

		// parse mod name
		if(!modValue.HasMember(JSON_MOD_NAME_PROPERTY_NAME.c_str())) {
			spdlog::error("Mod '{}' is missing '{}' property.", modID, JSON_MOD_NAME_PROPERTY_NAME);
			return nullptr;
		}

		const rapidjson::Value & modNameValue = modValue[JSON_MOD_NAME_PROPERTY_NAME.c_str()];

		if(!modNameValue.IsString()) {
			spdlog::error("Mod '{}' has an invalid '{}' property type: '{}', expected 'string'.", modID, JSON_MOD_NAME_PROPERTY_NAME, Utilities::typeToString(modNameValue.GetType()));
			return nullptr;
		}

		std::string modName(Utilities::trimString(modNameValue.GetString()));

		if(modName.empty()) {
			spdlog::error("Mod '{}' '{}' property cannot be empty.", modID, JSON_MOD_NAME_PROPERTY_NAME);
			return nullptr;
		}

		standAloneGameVersion->setID(modID);
		standAloneGameVersion->setLongName(modName);
		standAloneGameVersion->setShortName(modName);

		if(!arguments.empty()) {
			if(arguments.find(JSON_MOD_GAME_VERSION_ARGUMENT_CON_FILE_PROPERTY_NAME) != arguments.cend()) {
				standAloneGameVersion->setConFileArgumentFlag(arguments[JSON_MOD_GAME_VERSION_ARGUMENT_CON_FILE_PROPERTY_NAME]);
			}

			if(arguments.find(JSON_MOD_GAME_VERSION_ARGUMENT_EXTRA_CON_FILE_PROPERTY_NAME) != arguments.cend()) {
				standAloneGameVersion->setExtraConFileArgumentFlag(arguments[JSON_MOD_GAME_VERSION_ARGUMENT_EXTRA_CON_FILE_PROPERTY_NAME]);
			}

			if(arguments.find(JSON_MOD_GAME_VERSION_ARGUMENT_GROUP_FILE_PROPERTY_NAME) != arguments.cend()) {
				standAloneGameVersion->setGroupFileArgumentFlag(arguments[JSON_MOD_GAME_VERSION_ARGUMENT_GROUP_FILE_PROPERTY_NAME]);
			}

			if(arguments.find(JSON_MOD_GAME_VERSION_ARGUMENT_DEF_FILE_PROPERTY_NAME) != arguments.cend()) {
				standAloneGameVersion->setDefFileArgumentFlag(arguments[JSON_MOD_GAME_VERSION_ARGUMENT_DEF_FILE_PROPERTY_NAME]);
			}

			if(arguments.find(JSON_MOD_GAME_VERSION_ARGUMENT_EXTRA_DEF_FILE_PROPERTY_NAME) != arguments.cend()) {
				standAloneGameVersion->setExtraDefFileArgumentFlag(arguments[JSON_MOD_GAME_VERSION_ARGUMENT_EXTRA_DEF_FILE_PROPERTY_NAME]);
			}

			if(arguments.find(JSON_MOD_GAME_VERSION_ARGUMENT_MAP_FILE_PROPERTY_NAME) != arguments.cend()) {
				standAloneGameVersion->setMapFileArgumentFlag(arguments[JSON_MOD_GAME_VERSION_ARGUMENT_MAP_FILE_PROPERTY_NAME]);
			}

			if(arguments.find(JSON_MOD_GAME_VERSION_ARGUMENT_EPISODE_PROPERTY_NAME) != arguments.cend()) {
				standAloneGameVersion->setEpisodeArgumentFlag(arguments[JSON_MOD_GAME_VERSION_ARGUMENT_EPISODE_PROPERTY_NAME]);
			}

			if(arguments.find(JSON_MOD_GAME_VERSION_ARGUMENT_LEVEL_PROPERTY_NAME) != arguments.cend()) {
				standAloneGameVersion->setLevelArgumentFlag(arguments[JSON_MOD_GAME_VERSION_ARGUMENT_LEVEL_PROPERTY_NAME]);
			}

			if(arguments.find(JSON_MOD_GAME_VERSION_ARGUMENT_SKILL_PROPERTY_NAME) != arguments.cend()) {
				standAloneGameVersion->setSkillArgumentFlag(arguments[JSON_MOD_GAME_VERSION_ARGUMENT_SKILL_PROPERTY_NAME]);
			}

			if(arguments.find(JSON_MOD_GAME_VERSION_ARGUMENT_RECORD_DEMO_PROPERTY_NAME) != arguments.cend()) {
				standAloneGameVersion->setRecordDemoArgumentFlag(arguments[JSON_MOD_GAME_VERSION_ARGUMENT_RECORD_DEMO_PROPERTY_NAME]);
			}

			if(arguments.find(JSON_MOD_GAME_VERSION_ARGUMENT_PLAY_DEMO_PROPERTY_NAME) != arguments.cend()) {
				standAloneGameVersion->setPlayDemoArgumentFlag(arguments[JSON_MOD_GAME_VERSION_ARGUMENT_PLAY_DEMO_PROPERTY_NAME]);
			}

			if(arguments.find(JSON_MOD_GAME_VERSION_ARGUMENT_RESPAWN_MODE_PROPERTY_NAME) != arguments.cend()) {
				standAloneGameVersion->setRespawnModeArgumentFlag(arguments[JSON_MOD_GAME_VERSION_ARGUMENT_RESPAWN_MODE_PROPERTY_NAME]);
			}

			if(arguments.find(JSON_MOD_GAME_VERSION_ARGUMENT_WEAPON_SWITCH_ORDER_PROPERTY_NAME) != arguments.cend()) {
				standAloneGameVersion->setWeaponSwitchOrderArgumentFlag(arguments[JSON_MOD_GAME_VERSION_ARGUMENT_WEAPON_SWITCH_ORDER_PROPERTY_NAME]);
			}

			if(arguments.find(JSON_MOD_GAME_VERSION_ARGUMENT_DISABLE_MONSTERS_PROPERTY_NAME) != arguments.cend()) {
				standAloneGameVersion->setDisableMonstersArgumentFlag(arguments[JSON_MOD_GAME_VERSION_ARGUMENT_DISABLE_MONSTERS_PROPERTY_NAME]);
			}

			if(arguments.find(JSON_MOD_GAME_VERSION_ARGUMENT_DISABLE_SOUND_PROPERTY_NAME) != arguments.cend()) {
				standAloneGameVersion->setDisableSoundArgumentFlag(arguments[JSON_MOD_GAME_VERSION_ARGUMENT_DISABLE_SOUND_PROPERTY_NAME]);
			}

			if(arguments.find(JSON_MOD_GAME_VERSION_ARGUMENT_DISABLE_MUSIC_PROPERTY_NAME) != arguments.cend()) {
				standAloneGameVersion->setDisableMusicArgumentFlag(arguments[JSON_MOD_GAME_VERSION_ARGUMENT_DISABLE_MUSIC_PROPERTY_NAME]);
			}
		}

		if(!standAloneGameVersion->isValid()) {
			spdlog::error("Mod with ID '{}' has an invalid stand-alone game version.", standAloneGameVersion->getID());
			return nullptr;
		}
	}

	// parse the mod game version files property
	if(!modGameVersionValue.HasMember(JSON_MOD_GAME_VERSION_FILES_PROPERTY_NAME.c_str())) {
		spdlog::error("Mod game version is missing '{}' property.", JSON_MOD_GAME_VERSION_FILES_PROPERTY_NAME);
		return nullptr;
	}

	const rapidjson::Value & modFilesValue = modGameVersionValue[JSON_MOD_GAME_VERSION_FILES_PROPERTY_NAME.c_str()];

	if(!modFilesValue.IsArray()) {
		spdlog::error("Mod game version '{}' property has invalid type: '{}', expected 'array'.", JSON_MOD_GAME_VERSION_FILES_PROPERTY_NAME, Utilities::typeToString(modFilesValue.GetType()));
		return nullptr;
	}

	std::shared_ptr<ModFile> newModFile;

	for(rapidjson::Value::ConstValueIterator i = modFilesValue.Begin(); i != modFilesValue.End(); ++i) {
		newModFile = ModFile::parseFrom(*i, skipFileInfoValidation);

		if(!ModFile::isValid(newModFile.get(), skipFileInfoValidation)) {
			spdlog::error("Failed to parse mod file #{}.", newModGameVersion->m_files.size() + 1);
			return nullptr;
		}

		newModFile->setParentModGameVersion(newModGameVersion.get());

		if(newModGameVersion->hasFile(*newModFile)) {
			spdlog::error("Encountered duplicate mod file #{}.", newModGameVersion->m_files.size() + 1);
			return nullptr;
		}

		newModGameVersion->m_files.push_back(newModFile);
	}

	return newModGameVersion;
}

std::unique_ptr<ModGameVersion> ModGameVersion::parseFrom(const tinyxml2::XMLElement * modGameVersionElement, bool skipFileInfoValidation) {
	if(modGameVersionElement == nullptr) {
		return nullptr;
	}

	// verify element name
	if(modGameVersionElement->Name() != XML_MOD_GAME_VERSION_ELEMENT_NAME) {
		spdlog::error("Invalid mod game version element name: '{}', expected '{}'.", modGameVersionElement->Name(), XML_MOD_GAME_VERSION_ELEMENT_NAME);
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
			spdlog::warn("Element '{}' has unexpected attribute '{}'.", XML_MOD_GAME_VERSION_ELEMENT_NAME, modGameVersionAttribute->Name());
		}

		modGameVersionAttribute = modGameVersionAttribute->Next();
	}

	// read the mod game version attributes
	const char * modGameVersionGameVersion = modGameVersionElement->Attribute(XML_MOD_GAME_VERSION_GAME_VERSION_ATTRIBUTE_NAME.c_str());

	if(modGameVersionGameVersion == nullptr || Utilities::stringLength(modGameVersionGameVersion) == 0) {
		spdlog::error("Attribute '{}' is missing from '{}' element.", XML_MOD_GAME_VERSION_GAME_VERSION_ATTRIBUTE_NAME, XML_MOD_GAME_VERSION_ELEMENT_NAME);
		return nullptr;
	}

	bool standAlone = Utilities::areStringsEqualIgnoreCase(modGameVersionGameVersion, GameVersion::STANDALONE);

	const char * modGameVersionConvertedData = modGameVersionElement->Attribute(XML_MOD_GAME_VERSION_CONVERTED_ATTRIBUTE_NAME.c_str());

	if(modGameVersionConvertedData == nullptr || Utilities::stringLength(modGameVersionConvertedData) == 0) {
		spdlog::error("Attribute '{}' is missing from '{}' element.", XML_MOD_GAME_VERSION_CONVERTED_ATTRIBUTE_NAME, XML_MOD_GAME_VERSION_ELEMENT_NAME);
		return nullptr;
	}

	bool modGameVersionConverted = false;

	if(Utilities::areStringsEqualIgnoreCase(modGameVersionConvertedData, "Converted")) {
		modGameVersionConverted = true;
	}
	else if(Utilities::areStringsEqualIgnoreCase(modGameVersionConvertedData, "Native")) {
		modGameVersionConverted = false;
	}
	else {
		spdlog::error("Attribute '{}' in element '{}' has an invalid value: '{}', expected 'Native' or 'Converted'.", XML_MOD_GAME_VERSION_CONVERTED_ATTRIBUTE_NAME, XML_MOD_GAME_VERSION_ELEMENT_NAME, modGameVersionConvertedData);
		return nullptr;
	}

	// initialize the mod game version
	std::unique_ptr<ModGameVersion> newModGameVersion = std::make_unique<ModGameVersion>(modGameVersionGameVersion, modGameVersionConverted);

	const tinyxml2::XMLElement * filesContainerElement = nullptr;

	if(standAlone) {
		std::shared_ptr<GameVersion> standAloneGameVersion(std::make_unique<GameVersion>());
		standAloneGameVersion->setStandAlone(true);
		newModGameVersion->m_standAloneGameVersion = standAloneGameVersion;
		bool foundPropertiesElement = false;
		std::map<std::string, std::string> properties;
		std::map<std::string, std::string> arguments;
		const tinyxml2::XMLElement * currentModGameVersionChildElement = modGameVersionElement->FirstChildElement();
		const tinyxml2::XMLElement * modGameVersionParentElement = Utilities::getParentXMLElement(modGameVersionElement);
		const tinyxml2::XMLElement * parentModVersionElement = nullptr;

		if(Utilities::areStringsEqual(modGameVersionParentElement->Name(), XML_MOD_VERSION_TYPE_ELEMENT_NAME)) {
			parentModVersionElement = Utilities::getParentXMLElement(modGameVersionParentElement);
		}
		else if(Utilities::areStringsEqual(modGameVersionParentElement->Name(), "version")) {
			parentModVersionElement = modGameVersionParentElement;
		}
		else {
			spdlog::error("Invalid element '{}' parent name: '{}', expected '{}' or '{}'.", XML_MOD_GAME_VERSION_ELEMENT_NAME, modGameVersionParentElement->Name(), XML_MOD_VERSION_TYPE_ELEMENT_NAME, XML_MOD_VERSION_ELEMENT_NAME);
			return nullptr;
		}

		if(parentModVersionElement == nullptr || !Utilities::areStringsEqual(parentModVersionElement->Name(), XML_MOD_VERSION_ELEMENT_NAME)) {
			spdlog::error("Invalid element '{}' parent mod version element name: '{}', expected '{}'.", XML_MOD_GAME_VERSION_ELEMENT_NAME, parentModVersionElement->Name(), XML_MOD_VERSION_ELEMENT_NAME);
			return nullptr;
		}

		const tinyxml2::XMLElement * parentVersionsContainerElement = Utilities::getParentXMLElement(parentModVersionElement);

		if(parentVersionsContainerElement == nullptr || !Utilities::areStringsEqual(parentVersionsContainerElement->Name(), XML_MOD_VERSIONS_ELEMENT_NAME)) {
			spdlog::error("Invalid element '{}' parent mod versions container element name: '{}', expected '{}'.", XML_MOD_GAME_VERSION_ELEMENT_NAME, parentVersionsContainerElement->Name(), XML_MOD_VERSIONS_ELEMENT_NAME);
			return nullptr;
		}

		const tinyxml2::XMLElement * parentModElement = Utilities::getParentXMLElement(parentVersionsContainerElement);

		if(parentModElement == nullptr || !Utilities::areStringsEqual(parentModElement->Name(), XML_MOD_ELEMENT_NAME)) {
			spdlog::error("Invalid element '{}' parent mod element name: '{}', expected '{}'.", XML_MOD_GAME_VERSION_ELEMENT_NAME, parentModElement->Name(), XML_MOD_ELEMENT_NAME);
			return nullptr;
		}

		// read the mod id attribute value
		const char * modIDRaw = parentModElement->Attribute(XML_MOD_ID_ATTRIBUTE_NAME.c_str());

		if(Utilities::stringLength(modIDRaw) == 0) {
			spdlog::error("Attribute '{}' is missing from '{}' parent mod element.", XML_MOD_ID_ATTRIBUTE_NAME, XML_MOD_GAME_VERSION_ELEMENT_NAME, XML_MOD_ELEMENT_NAME);
			return nullptr;
		}

		// read the mod name attribute value
		const char * modNameRaw = parentModElement->Attribute(XML_MOD_NAME_ATTRIBUTE_NAME.c_str());

		if(Utilities::stringLength(modNameRaw) == 0) {
			spdlog::error("Attribute '{}' is missing from '{}' parent mod element.", XML_MOD_NAME_ATTRIBUTE_NAME, XML_MOD_GAME_VERSION_ELEMENT_NAME, XML_MOD_ELEMENT_NAME);
			return nullptr;
		}

		while(currentModGameVersionChildElement != nullptr) {
			if(Utilities::areStringsEqual(currentModGameVersionChildElement->Name(), XML_MOD_GAME_VERSION_PROPERTIES_ELEMENT_NAME)) {
				foundPropertiesElement = true;

				const tinyxml2::XMLAttribute * currentPropertiesAttribute = currentModGameVersionChildElement->FirstAttribute();

				if(currentPropertiesAttribute == nullptr) {
					spdlog::warn("Element '{}' '{}' has no  attributes.", XML_MOD_GAME_VERSION_ELEMENT_NAME, XML_MOD_GAME_VERSION_PROPERTIES_ELEMENT_NAME);
					return nullptr;
				}

				size_t propertyNumber = 1;

				while(currentPropertiesAttribute != nullptr) {
					auto acceptedPropertyNameIterator = std::find(XML_MOD_GAME_VERSION_PROPERTIES_ATTRIBUTE_NAMES.cbegin(), XML_MOD_GAME_VERSION_PROPERTIES_ATTRIBUTE_NAMES.cend(), currentPropertiesAttribute->Name());

					if(acceptedPropertyNameIterator == XML_MOD_GAME_VERSION_PROPERTIES_ATTRIBUTE_NAMES.cend()) {
						spdlog::warn("Element '{}' '{}' attribute #{} has unexpected name: '{}'.", XML_MOD_GAME_VERSION_ELEMENT_NAME, XML_MOD_GAME_VERSION_PROPERTIES_ELEMENT_NAME, propertyNumber, currentPropertiesAttribute->Name());
					}

					properties[currentPropertiesAttribute->Name()] = currentPropertiesAttribute->Value();

					currentPropertiesAttribute = currentPropertiesAttribute->Next();
					propertyNumber++;
				}
			}
			else if(Utilities::areStringsEqual(currentModGameVersionChildElement->Name(), XML_MOD_GAME_VERSION_ARGUMENTS_ELEMENT_NAME)) {
				const tinyxml2::XMLElement * currentArgumentElement = currentModGameVersionChildElement->FirstChildElement();
				size_t argumentNumber = 1;

				if(currentArgumentElement == nullptr) {
					spdlog::warn("Element '{}' has no '{}' child elements.", XML_MOD_GAME_VERSION_ELEMENT_NAME, XML_MOD_GAME_VERSION_ARGUMENTS_ELEMENT_NAME);
					return nullptr;
				}

				while(currentArgumentElement != nullptr) {
					if(!Utilities::areStringsEqual(currentArgumentElement->Name(), XML_MOD_GAME_VERSION_ARGUMENT_ELEMENT_NAME)) {
						spdlog::error("Element '{}' '{}' child element #{} has invalid name: '{}', expected: '{}'.", XML_MOD_GAME_VERSION_ELEMENT_NAME, XML_MOD_GAME_VERSION_ARGUMENTS_ELEMENT_NAME, argumentNumber, currentArgumentElement->Name(), XML_MOD_GAME_VERSION_ARGUMENT_ELEMENT_NAME);
						return nullptr;
					}

					const char * argumentNameRaw = currentArgumentElement->Attribute(XML_MOD_GAME_VERSION_ARGUMENT_NAME_PROPERTY_NAME.c_str());
					const char * argumentFlagRaw = currentArgumentElement->Attribute(XML_MOD_GAME_VERSION_ARGUMENT_FLAG_PROPERTY_NAME.c_str());

					if(Utilities::stringLength(argumentNameRaw) == 0) {
						spdlog::error("Element '{}' '{}' child element #{} has empty '{}' attribute.", XML_MOD_GAME_VERSION_ELEMENT_NAME, XML_MOD_GAME_VERSION_ARGUMENTS_ELEMENT_NAME, argumentNumber, XML_MOD_GAME_VERSION_ARGUMENT_NAME_PROPERTY_NAME);
						return nullptr;
					}

					if(Utilities::stringLength(argumentFlagRaw) == 0) {
						spdlog::error("Element '{}' '{}' child element #{} has empty '{}' attribute.", XML_MOD_GAME_VERSION_ELEMENT_NAME, XML_MOD_GAME_VERSION_ARGUMENTS_ELEMENT_NAME, argumentNumber, XML_MOD_GAME_VERSION_ARGUMENT_FLAG_PROPERTY_NAME);
						return nullptr;
					}

					auto acceptedArgumentNameIterator = std::find(XML_MOD_GAME_VERSION_ARGUMENT_NAMES.cbegin(), XML_MOD_GAME_VERSION_ARGUMENT_NAMES.cend(), argumentNameRaw);

					if(acceptedArgumentNameIterator == XML_MOD_GAME_VERSION_ARGUMENT_NAMES.cend()) {
						spdlog::warn("Element '{}' '{}' child element #{} has unexpected '{}' attribute value: '{}'.", XML_MOD_GAME_VERSION_ELEMENT_NAME, XML_MOD_GAME_VERSION_ARGUMENTS_ELEMENT_NAME, argumentNumber, XML_MOD_GAME_VERSION_ARGUMENT_NAME_PROPERTY_NAME, argumentNameRaw);
					}

					arguments[argumentNameRaw] = argumentFlagRaw;

					currentArgumentElement = currentArgumentElement->NextSiblingElement();
					argumentNumber++;
				}
			}
			else if(Utilities::areStringsEqual(currentModGameVersionChildElement->Name(), XML_MOD_GAME_VERSION_OPERATING_SYSTEMS_ELEMENT_NAME)) {
				const tinyxml2::XMLElement * currentOperatingSystemElement = currentModGameVersionChildElement->FirstChildElement();
				size_t operatingSystemNumber = 1;

				if(currentOperatingSystemElement == nullptr) {
					spdlog::warn("Element '{}' has no '{}' child elements.", XML_MOD_GAME_VERSION_ELEMENT_NAME, XML_MOD_GAME_VERSION_OPERATING_SYSTEMS_ELEMENT_NAME);
					return nullptr;
				}

				while(currentOperatingSystemElement != nullptr) {
					if(!Utilities::areStringsEqual(currentOperatingSystemElement->Name(), XML_MOD_GAME_VERSION_OPERATING_SYSTEM_ELEMENT_NAME)) {
						spdlog::error("Element '{}' '{}' child element #{} has invalid name: '{}', expected: '{}'.", XML_MOD_GAME_VERSION_ELEMENT_NAME, XML_MOD_GAME_VERSION_OPERATING_SYSTEMS_ELEMENT_NAME, operatingSystemNumber, currentOperatingSystemElement->Name(), XML_MOD_GAME_VERSION_OPERATING_SYSTEM_ELEMENT_NAME);
						return nullptr;
					}

					const char * operatingSystemNameRaw = currentOperatingSystemElement->Attribute(XML_MOD_GAME_VERSION_OPERATING_SYSTEM_NAME_ATTRIBUTE_NAME.c_str());

					if(Utilities::stringLength(operatingSystemNameRaw) == 0) {
						spdlog::error("Element '{}' '{}' child element #{} has empty '{}' attribute.", XML_MOD_GAME_VERSION_ELEMENT_NAME, XML_MOD_GAME_VERSION_OPERATING_SYSTEMS_ELEMENT_NAME, operatingSystemNumber, XML_MOD_GAME_VERSION_OPERATING_SYSTEM_NAME_ATTRIBUTE_NAME);
						return nullptr;
					}

					std::optional<GameVersion::OperatingSystem> optionalOperatingSystem(magic_enum::enum_cast<GameVersion::OperatingSystem>(operatingSystemNameRaw));

					if(!optionalOperatingSystem.has_value()) {
						spdlog::error("Element '{}' '{}' child element #{} has invalid operating system name value: '{}'.", XML_MOD_GAME_VERSION_ELEMENT_NAME, XML_MOD_GAME_VERSION_OPERATING_SYSTEMS_ELEMENT_NAME, operatingSystemNumber, operatingSystemNameRaw);
						return nullptr;
					}

					standAloneGameVersion->addSupportedOperatingSystem(optionalOperatingSystem.value());

					currentOperatingSystemElement = currentOperatingSystemElement->NextSiblingElement();
					operatingSystemNumber++;
				}
			}
			else if(Utilities::areStringsEqual(currentModGameVersionChildElement->Name(), XML_MOD_GAME_VERSION_FILES_ELEMENT_NAME)) {
				filesContainerElement = currentModGameVersionChildElement;
			}
			else {
				spdlog::warn("Element '{}' has unexpected '{}' child element.", XML_MOD_GAME_VERSION_ELEMENT_NAME, currentModGameVersionChildElement->Name());
			}

			currentModGameVersionChildElement = currentModGameVersionChildElement->NextSiblingElement();
		}

		if(!foundPropertiesElement) {
			spdlog::error("Element '{}' is missing required '{}' child element.", XML_MOD_GAME_VERSION_ELEMENT_NAME, XML_MOD_GAME_VERSION_PROPERTIES_ELEMENT_NAME);
			return nullptr;
		}

		if(filesContainerElement == nullptr) {
			spdlog::error("Element '{}' is missing required '{}' child element.", XML_MOD_GAME_VERSION_ELEMENT_NAME, XML_MOD_GAME_VERSION_FILES_ELEMENT_NAME);
			return nullptr;
		}

		standAloneGameVersion->setID(modIDRaw);
		standAloneGameVersion->setLongName(modNameRaw);
		standAloneGameVersion->setShortName(modNameRaw);

		if(!properties.empty()) {
			if(properties.find(XML_MOD_GAME_VERSION_PROPERTY_BASE_NAME) != properties.cend()) {
				standAloneGameVersion->setBase(properties[XML_MOD_GAME_VERSION_PROPERTY_BASE_NAME]);
			}

			if(properties.find(XML_MOD_GAME_VERSION_PROPERTY_GAME_EXE_NAME_NAME) != properties.cend()) {
				standAloneGameVersion->setGameExecutableName(properties[XML_MOD_GAME_VERSION_PROPERTY_GAME_EXE_NAME_NAME]);
			}

			if(properties.find(XML_MOD_GAME_VERSION_PROPERTY_SETUP_EXE_NAME_NAME) != properties.cend()) {
				standAloneGameVersion->setSetupExecutableName(properties[XML_MOD_GAME_VERSION_PROPERTY_SETUP_EXE_NAME_NAME]);
			}

			if(properties.find(XML_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_FILE_NAME_NAME) != properties.cend()) {
				standAloneGameVersion->setGameConfigurationFileName(properties[XML_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_FILE_NAME_NAME]);
			}

			if(properties.find(XML_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_DIRECTORY_PATH_NAME) != properties.cend()) {
				standAloneGameVersion->setGameConfigurationDirectoryPath(properties[XML_MOD_GAME_VERSION_PROPERTY_GAME_CONFIGURATION_DIRECTORY_PATH_NAME]);
			}

			if(properties.find(XML_MOD_GAME_VERSION_PROPERTY_SKILL_START_VALUE_NAME) != properties.cend()) {
				standAloneGameVersion->setSkillStartValue(Utilities::parseUnsignedByte(properties[XML_MOD_GAME_VERSION_PROPERTY_SKILL_START_VALUE_NAME]).value_or(GameVersion::DEFAULT_SKILL_START_VALUE));
			}

			if(properties.find(XML_MOD_GAME_VERSION_PROPERTY_LOCAL_WORKING_DIRECTORY_NAME) != properties.cend()) {
				standAloneGameVersion->setLocalWorkingDirectory(Utilities::parseBoolean(properties[XML_MOD_GAME_VERSION_PROPERTY_LOCAL_WORKING_DIRECTORY_NAME]).value_or(GameVersion::DEFAULT_LOCAL_WORKING_DIRECTORY));
			}

			if(properties.find(XML_MOD_GAME_VERSION_PROPERTY_SCRIPT_FILES_READ_FROM_GROUP_NAME) != properties.cend()) {
				std::optional<bool> optionalScriptFilesReadFromGroup(Utilities::parseBoolean(properties[XML_MOD_GAME_VERSION_PROPERTY_SCRIPT_FILES_READ_FROM_GROUP_NAME]));

				if(optionalScriptFilesReadFromGroup.has_value()) {
					standAloneGameVersion->setScriptFilesReadFromGroup(optionalScriptFilesReadFromGroup.value());
				}
			}

			if(properties.find(XML_MOD_GAME_VERSION_PROPERTY_SUPPORTS_SUBDIRECTORIES_NAME) != properties.cend()) {
				standAloneGameVersion->setSupportsSubdirectories(Utilities::parseBoolean(properties[XML_MOD_GAME_VERSION_PROPERTY_SUPPORTS_SUBDIRECTORIES_NAME]).value_or(GameVersion::DEFAULT_SUPPORTS_SUBDIRECTORIES));
			}

			if(properties.find(XML_MOD_GAME_VERSION_PROPERTY_REQUIRES_GROUP_FILE_EXTRACTION_NAME) != properties.cend()) {
				std::optional<bool> optionalRequiresGroupFileExtraction(Utilities::parseBoolean(properties[XML_MOD_GAME_VERSION_PROPERTY_REQUIRES_GROUP_FILE_EXTRACTION_NAME]));

				if(optionalRequiresGroupFileExtraction.has_value()) {
					standAloneGameVersion->setRequiresGroupFileExtraction(optionalRequiresGroupFileExtraction.value());
				}
			}
		}

		if(!arguments.empty()) {
			if(arguments.find(XML_MOD_GAME_VERSION_ARGUMENT_CON_FILE_NAME) != arguments.cend()) {
				standAloneGameVersion->setConFileArgumentFlag(arguments[XML_MOD_GAME_VERSION_ARGUMENT_CON_FILE_NAME]);
			}

			if(arguments.find(XML_MOD_GAME_VERSION_ARGUMENT_EXTRA_CON_FILE_NAME) != arguments.cend()) {
				standAloneGameVersion->setExtraConFileArgumentFlag(arguments[XML_MOD_GAME_VERSION_ARGUMENT_EXTRA_CON_FILE_NAME]);
			}

			if(arguments.find(XML_MOD_GAME_VERSION_ARGUMENT_GROUP_FILE_NAME) != arguments.cend()) {
				standAloneGameVersion->setGroupFileArgumentFlag(arguments[XML_MOD_GAME_VERSION_ARGUMENT_GROUP_FILE_NAME]);
			}

			if(arguments.find(XML_MOD_GAME_VERSION_ARGUMENT_DEF_FILE_NAME) != arguments.cend()) {
				standAloneGameVersion->setDefFileArgumentFlag(arguments[XML_MOD_GAME_VERSION_ARGUMENT_DEF_FILE_NAME]);
			}

			if(arguments.find(XML_MOD_GAME_VERSION_ARGUMENT_EXTRA_DEF_FILE_NAME) != arguments.cend()) {
				standAloneGameVersion->setExtraDefFileArgumentFlag(arguments[XML_MOD_GAME_VERSION_ARGUMENT_EXTRA_DEF_FILE_NAME]);
			}

			if(arguments.find(XML_MOD_GAME_VERSION_ARGUMENT_MAP_FILE_NAME) != arguments.cend()) {
				standAloneGameVersion->setMapFileArgumentFlag(arguments[XML_MOD_GAME_VERSION_ARGUMENT_MAP_FILE_NAME]);
			}

			if(arguments.find(XML_MOD_GAME_VERSION_ARGUMENT_EPISODE_NAME) != arguments.cend()) {
				standAloneGameVersion->setEpisodeArgumentFlag(arguments[XML_MOD_GAME_VERSION_ARGUMENT_EPISODE_NAME]);
			}

			if(arguments.find(XML_MOD_GAME_VERSION_ARGUMENT_LEVEL_NAME) != arguments.cend()) {
				standAloneGameVersion->setLevelArgumentFlag(arguments[XML_MOD_GAME_VERSION_ARGUMENT_LEVEL_NAME]);
			}

			if(arguments.find(XML_MOD_GAME_VERSION_ARGUMENT_SKILL_NAME) != arguments.cend()) {
				standAloneGameVersion->setSkillArgumentFlag(arguments[XML_MOD_GAME_VERSION_ARGUMENT_SKILL_NAME]);
			}

			if(arguments.find(XML_MOD_GAME_VERSION_ARGUMENT_RECORD_DEMO_NAME) != arguments.cend()) {
				standAloneGameVersion->setRecordDemoArgumentFlag(arguments[XML_MOD_GAME_VERSION_ARGUMENT_RECORD_DEMO_NAME]);
			}

			if(arguments.find(XML_MOD_GAME_VERSION_ARGUMENT_PLAY_DEMO_NAME) != arguments.cend()) {
				standAloneGameVersion->setPlayDemoArgumentFlag(arguments[XML_MOD_GAME_VERSION_ARGUMENT_PLAY_DEMO_NAME]);
			}

			if(arguments.find(XML_MOD_GAME_VERSION_ARGUMENT_RESPAWN_MODE_NAME) != arguments.cend()) {
				standAloneGameVersion->setRespawnModeArgumentFlag(arguments[XML_MOD_GAME_VERSION_ARGUMENT_RESPAWN_MODE_NAME]);
			}

			if(arguments.find(XML_MOD_GAME_VERSION_ARGUMENT_WEAPON_SWITCH_ORDER_NAME) != arguments.cend()) {
				standAloneGameVersion->setWeaponSwitchOrderArgumentFlag(arguments[XML_MOD_GAME_VERSION_ARGUMENT_WEAPON_SWITCH_ORDER_NAME]);
			}

			if(arguments.find(XML_MOD_GAME_VERSION_ARGUMENT_DISABLE_MONSTERS_NAME) != arguments.cend()) {
				standAloneGameVersion->setDisableMonstersArgumentFlag(arguments[XML_MOD_GAME_VERSION_ARGUMENT_DISABLE_MONSTERS_NAME]);
			}

			if(arguments.find(XML_MOD_GAME_VERSION_ARGUMENT_DISABLE_SOUND_NAME) != arguments.cend()) {
				standAloneGameVersion->setDisableSoundArgumentFlag(arguments[XML_MOD_GAME_VERSION_ARGUMENT_DISABLE_SOUND_NAME]);
			}

			if(arguments.find(XML_MOD_GAME_VERSION_ARGUMENT_DISABLE_MUSIC_NAME) != arguments.cend()) {
				standAloneGameVersion->setDisableMusicArgumentFlag(arguments[XML_MOD_GAME_VERSION_ARGUMENT_DISABLE_MUSIC_NAME]);
			}
		}

		if(!standAloneGameVersion->isValid()) {
			spdlog::error("Mod with ID '{}' has an invalid stand-alone game version.", modIDRaw);
			return nullptr;
		}
	}
	else {
		filesContainerElement = modGameVersionElement;
	}

	// iterate over all of the mod file elements
	const tinyxml2::XMLElement * modFileElement = filesContainerElement->FirstChildElement();

	if(modFileElement == nullptr) {
		spdlog::error("Element '{}' has no children.", XML_MOD_GAME_VERSION_ELEMENT_NAME);
		return nullptr;
	}

	std::shared_ptr<ModFile> newModFile;

	while(true) {
		if(modFileElement == nullptr) {
			break;
		}

		newModFile = ModFile::parseFrom(modFileElement, skipFileInfoValidation);

		if(!ModFile::isValid(newModFile.get(), skipFileInfoValidation)) {
			spdlog::error("Failed to parse mod file #{}.", newModGameVersion->m_files.size() + 1);
			return nullptr;
		}

		newModFile->setParentModGameVersion(newModGameVersion.get());

		if(newModGameVersion->hasFile(*newModFile)) {
			spdlog::error("Encountered duplicate mod file #{}.", newModGameVersion->m_files.size() + 1);
			return nullptr;
		}

		newModGameVersion->m_files.push_back(newModFile);

		modFileElement = modFileElement->NextSiblingElement();
	}

	return newModGameVersion;
}

bool ModGameVersion::isGameVersionSupported(const GameVersion & gameVersion) const {
	if(isStandAlone()) {
		return false;
	}

	return gameVersion.isValid() &&
		   Utilities::areStringsEqualIgnoreCase(m_gameVersionID, gameVersion.getID());
}

bool ModGameVersion::isGameVersionCompatible(const GameVersion & gameVersion) const {
	if(isStandAlone()) {
		return false;
	}

	return gameVersion.isValid() &&
		   isGameVersionSupported(gameVersion) ||
		   gameVersion.hasCompatibleGameVersionWithID(m_gameVersionID);
}

bool ModGameVersion::isValid(bool skipFileInfoValidation) const {
	if(m_gameVersionID.empty() ||
	   m_files.empty()) {
		return false;
	}

	if(!isStandAlone() && !hasFileOfType("grp") && !hasFileOfType("zip")) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModFile>>::const_iterator i = m_files.begin(); i != m_files.end(); ++i) {
		if(!(*i)->isValid(skipFileInfoValidation)) {
			return false;
		}

		if((*i)->getParentModGameVersion() != this) {
			return false;
		}

		for(std::vector<std::shared_ptr<ModFile>>::const_iterator j = i + 1; j != m_files.end(); ++j) {
			if(Utilities::areStringsEqualIgnoreCase((*i)->getFileName(), (*j)->getFileName())) {
				return false;
			}
		}
	}

	return true;
}

bool ModGameVersion::isValid(const ModGameVersion * m, bool skipFileInfoValidation) {
	return m != nullptr && m->isValid(skipFileInfoValidation);
}

bool ModGameVersion::operator == (const ModGameVersion & m) const {
	if(m_files.size() != m.m_files.size() ||
	   !Utilities::areStringsEqualIgnoreCase(m_gameVersionID, m.m_gameVersionID) ||
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
