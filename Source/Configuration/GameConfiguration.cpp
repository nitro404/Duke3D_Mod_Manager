#include "GameConfiguration.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <sstream>

const char GameConfiguration::COMMENT_CHARACTER = ';';
const char GameConfiguration::SECTION_NAME_START_CHARACTER = '[';
const char GameConfiguration::SECTION_NAME_END_CHARACTER = ']';
const char GameConfiguration::ASSIGNMENT_CHARACTER = '=';
const char GameConfiguration::EMPTY_VALUE_CHARACTER = '~';
const std::string GameConfiguration::SETUP_SECTION_NAME("Setup");
const std::string GameConfiguration::SETUP_VERSION_ENTRY_NAME("SetupVersion");
const std::string GameConfiguration::REGULAR_VERSION_SETUP_VERSION("1.3D");
const std::string GameConfiguration::ATOMIC_EDITION_SETUP_VERSION("1.4");
const std::string GameConfiguration::SCREEN_SETUP_SECTION_NAME("Screen Setup");
const std::string GameConfiguration::SCREEN_MODE_ENTRY_NAME("ScreenMode");
const std::string GameConfiguration::SCREEN_WIDTH_ENTRY_NAME("ScreenWidth");
const std::string GameConfiguration::SCREEN_HEIGHT_ENTRY_NAME("ScreenHeight");
const std::string GameConfiguration::SOUND_SETUP_SECTION_NAME("Sound Setup");
const std::string GameConfiguration::FX_DEVICE_ENTRY_NAME("FXDevice");
const std::string GameConfiguration::MUSIC_DEVICE_ENTRY_NAME("MusicDevice");
const std::string GameConfiguration::FX_VOLUME_ENTRY_NAME("FXVolume");
const std::string GameConfiguration::MUSIC_VOLUME_ENTRY_NAME("MusicVolume");
const std::string GameConfiguration::NUM_BITS_ENTRY_NAME("NumBits");
const std::string GameConfiguration::MIX_RATE_ENTRY_NAME("MixRate");
const std::string GameConfiguration::WEAPON_KEY_DEFINITION_ENTRY_NAME_PREFIX("Weapon_");
const std::string GameConfiguration::COMBAT_MACRO_ENTRY_NAME_PREFIX("CommbatMacro#");
const std::string GameConfiguration::PHONE_NAME_ENTRY_NAME_PREFIX("PhoneName#");
const std::string GameConfiguration::PHONE_NUMBER_ENTRY_NAME_PREFIX("PhoneNumber#");

const std::array<std::string, 10> GameConfiguration::DEFAULT_COMBAT_MACROS = {
	"An inspiration for birth control.",
	"You're gonna die for that!",
	"It hurts to be you.",
	"Lucky Son of a Bitch.",
	"Hmmm....Payback time.",
	"You bottom dwelling scum sucker.",
	"Damn, you're ugly.",
	"Ha ha ha...Wasted!",
	"You suck!",
	"AARRRGHHHHH!!!"
};

bool GameConfiguration::NameComparator::operator () (const std::string & nameA, const std::string & nameB) const {
	return std::lexicographical_compare(nameA.begin(), nameA.end(), nameB.begin(), nameB.end(), [](unsigned char a, unsigned char b) {
		return std::tolower(a) < std::tolower(b);
	});
}

GameConfiguration::GameConfiguration() { }

GameConfiguration::GameConfiguration(GameConfiguration && c) noexcept
	: m_filePath(std::move(c.m_filePath))
	, m_entries(std::move(c.m_entries))
	, m_sections(std::move(c.m_sections)) {
	updateParent();
}

GameConfiguration::GameConfiguration(const GameConfiguration & c)
	: m_filePath(c.m_filePath) {
	for(EntryMap::const_iterator i = c.m_entries.cbegin(); i != c.m_entries.cend(); ++i) {
		m_entries.emplace(i->first, std::make_shared<Entry>(*i->second));
	}

	for(SectionMap::const_iterator i = c.m_sections.cbegin(); i != c.m_sections.cend(); ++i) {
		m_sections.emplace(i->first, std::make_shared<Section>(*i->second));
	}

	updateParent();
}

GameConfiguration & GameConfiguration::operator = (GameConfiguration && c) noexcept {
	if(this != &c) {
		clearSections();

		m_filePath = std::move(c.m_filePath);
		m_entries = std::move(c.m_entries);
		m_sections = std::move(c.m_sections);

		updateParent();
	}

	return *this;
}

GameConfiguration & GameConfiguration::operator = (const GameConfiguration & c) {
	clearSections();

	for(EntryMap::const_iterator i = c.m_entries.cbegin(); i != c.m_entries.cend(); ++i) {
		m_entries.emplace(i->first, std::make_shared<Entry>(*i->second));
	}

	for(SectionMap::const_iterator i = c.m_sections.cbegin(); i != c.m_sections.cend(); ++i) {
		m_sections.emplace(i->first, std::make_shared<Section>(*i->second));
	}

	updateParent();

	return *this;
}

GameConfiguration::~GameConfiguration() {
	for(EntryMap::const_iterator i = m_entries.cbegin(); i != m_entries.cend(); ++i) {
		i->second->m_parentGameConfiguration = nullptr;
	}

	for(SectionMap::const_iterator i = m_sections.cbegin(); i != m_sections.cend(); ++i) {
		i->second->m_parentGameConfiguration = nullptr;
	}
}

bool GameConfiguration::hasFilePath() const {
	return !m_filePath.empty();
}

const std::string & GameConfiguration::getFilePath() const {
	return m_filePath;
}

void GameConfiguration::setFilePath(const std::string & filePath) {
	m_filePath = filePath;
}

void GameConfiguration::clearFilePath() {
	m_filePath = "";
}

size_t GameConfiguration::numberOfEntries() const {
	return m_entries.size();
}

bool GameConfiguration::hasEntry(const Entry & entry) const {
	std::shared_ptr<Entry> sharedEntry(getEntry(entry));

	if(sharedEntry == nullptr || &entry != sharedEntry.get()) {
		return false;
	}

	return true;
}

bool GameConfiguration::hasEntryWithName(const std::string & entryName) const {
	return m_entries.find(entryName) != m_entries.cend();
}

std::shared_ptr<GameConfiguration::Entry> GameConfiguration::getEntry(const Entry & entry) const {
	std::shared_ptr<Entry> sharedEntry(getEntryWithName(entry.getName()));

	if(sharedEntry == nullptr || &entry != sharedEntry.get()) {
		return nullptr;
	}

	return sharedEntry;
}

std::shared_ptr<GameConfiguration::Entry> GameConfiguration::getEntryWithName(const std::string & entryName) const {
	EntryMap::const_iterator entryIterator(m_entries.find(entryName));

	if(entryIterator == m_entries.cend()) {
		return nullptr;
	}

	return entryIterator->second;
}

bool GameConfiguration::setEntryName(const std::string & oldEntryName, const std::string & newEntryName) {
	std::shared_ptr<Entry> sharedEntry(getEntry(oldEntryName));

	if(sharedEntry == nullptr) {
		return false;
	}

	return setEntryName(*sharedEntry, newEntryName);
}

bool GameConfiguration::setEntryName(Entry & entry, const std::string & newEntryName) {
	if(!Entry::isNameValid(newEntryName) || hasEntry(newEntryName)) {
		return false;
	}

	std::shared_ptr<Entry> sharedEntry(getEntry(entry));

	if(sharedEntry == nullptr || m_entries.erase(entry.getName()) == 0) {
		return false;
	}

	entry.m_name = newEntryName;

	m_entries.emplace(newEntryName, sharedEntry);

	return true;
}

bool GameConfiguration::addEntryToSection(std::shared_ptr<Entry> entry, const Section & section) {
	std::shared_ptr<Section> sharedSection(getSection(section));

	if(sharedSection == nullptr || hasEntryWithName(entry->m_name)) {
		return false;
	}

	if(!entry->isValid(false)) {
		return false;
	}

	entry->m_parentSection = sharedSection.get();
	entry->m_parentGameConfiguration = this;

	sharedSection->m_entries.emplace_back(entry);
	m_entries.emplace(entry->getName(), entry);

	return true;
}

bool GameConfiguration::addEntryToSectionWithName(std::shared_ptr<Entry> entry, const std::string & sectionName) {
	std::shared_ptr<Section> sharedSection(getSectionWithName(sectionName));

	if(sharedSection == nullptr) {
		return false;
	}

	return addEntryToSection(entry, *sharedSection);
}

bool GameConfiguration::removeEntry(const Entry & entry) {
	std::shared_ptr<Entry> sharedEntry(getEntry(entry));

	if(sharedEntry == nullptr || sharedEntry->m_parentSection == nullptr) {
		return false;
	}

	std::shared_ptr<Section> sharedSection(getSection(*sharedEntry->m_parentSection));

	if(sharedSection == nullptr) {
		return false;
	}

	size_t entryIndexInSection = sharedSection->indexOfEntry(*sharedEntry);

	if(entryIndexInSection == std::numeric_limits<size_t>::max()) {
		spdlog::warn("Entry '{}' not found in '{}' section.", sharedEntry->getName(), sharedSection->getName());
	}
	else {
		sharedSection->m_entries.erase(sharedSection->m_entries.begin() + entryIndexInSection);
	}

	sharedEntry->m_parentSection = nullptr;
	sharedEntry->m_parentGameConfiguration = nullptr;

	return m_entries.erase(sharedEntry->getName()) != 0;
}

bool GameConfiguration::removeEntryWithName(const std::string & entryName) {
	std::shared_ptr<Entry> sharedEntry(getEntryWithName(entryName));

	if(sharedEntry == nullptr) {
		return false;
	}

	return removeEntry(*sharedEntry);
}

size_t GameConfiguration::removeEntries(const std::vector<std::shared_ptr<Entry>> & entries) {
	if(entries.empty()) {
		return true;
	}

	size_t entriesRemoved = 0;

	for(const std::shared_ptr<Entry> & entry : entries) {
		if(removeEntry(*entry)) {
			entriesRemoved++;
		}
	}

	return entriesRemoved;
}

void GameConfiguration::clearEntries() {
	for(EntryMap::const_iterator i = m_entries.cbegin(); i != m_entries.cend(); ++i) {
		i->second->m_parentSection = nullptr;
		i->second->m_parentGameConfiguration = nullptr;
	}

	for(SectionMap::const_iterator i = m_sections.cbegin(); i != m_sections.cend(); ++i) {
		i->second->m_entries.clear();
	}

	m_entries.clear();
}

size_t GameConfiguration::numberOfSections() const {
	return m_sections.size();
}

bool GameConfiguration::hasSection(const Section & section) const {
	std::shared_ptr<Section> sharedSection(getSectionWithName(section.getName()));

	if(sharedSection == nullptr || &section != sharedSection.get()) {
		return false;
	}

	return true;
}

bool GameConfiguration::hasSectionWithName(const std::string & sectionName) const {
	return m_sections.find(sectionName) != m_sections.cend();
}

std::shared_ptr<GameConfiguration::Section> GameConfiguration::getSection(const Section & section) const {
	std::shared_ptr<Section> sharedSection(getSectionWithName(section.getName()));

	if(sharedSection == nullptr || &section != sharedSection.get()) {
		return nullptr;
	}

	return sharedSection;
}

std::shared_ptr<GameConfiguration::Section> GameConfiguration::getSectionWithName(const std::string & sectionName) const {
	SectionMap::const_iterator sectionIterator(m_sections.find(sectionName));

	if(sectionIterator == m_sections.cend()) {
		return nullptr;
	}

	return sectionIterator->second;
}

bool GameConfiguration::setSectionName(const std::string & oldSectionName, const std::string & newSectionName) {
	std::shared_ptr<Section> sharedSection(getSection(oldSectionName));

	if(sharedSection == nullptr) {
		return false;
	}

	return setSectionName(*sharedSection, newSectionName);
}

bool GameConfiguration::setSectionName(Section & section, const std::string & newSectionName) {
	if(!Section::isNameValid(newSectionName) || hasSection(newSectionName)) {
		return false;
	}

	std::shared_ptr<Section> sharedSection(getSection(section));

	if(sharedSection == nullptr || m_sections.erase(section.getName()) == 0) {
		return false;
	}

	section.m_name = newSectionName;

	m_sections.emplace(newSectionName, sharedSection);

	return true;
}

bool GameConfiguration::addSection(std::shared_ptr<Section> section) {
	if(!Section::isValid(section.get(), false) || hasSectionWithName(section->m_name)) {
		return false;
	}

	for(const std::shared_ptr<Entry> & entry : section->m_entries) {
		if(hasEntryWithName(entry->m_name)) {
			return false;
		}
	}

	section->m_parentGameConfiguration = this;

	for(std::shared_ptr<Entry> & entry : section->m_entries) {
		entry->m_parentSection = section.get();
		entry->m_parentGameConfiguration = this;

		m_entries.emplace(entry->getName(), entry);
	}

	m_sections.emplace(section->getName(), section);

	return true;
}

bool GameConfiguration::removeSection(const Section & section) {
	std::shared_ptr<Section> sharedSection(getSection(section));

	if(sharedSection == nullptr) {
		return false;
	}

	for(std::vector<std::shared_ptr<Entry>>::const_iterator sectionEntryiterator = sharedSection->m_entries.cbegin(); sectionEntryiterator != sharedSection->m_entries.cend(); ++sectionEntryiterator) {
		(*sectionEntryiterator)->m_parentGameConfiguration = nullptr;

		if(m_entries.erase((*sectionEntryiterator)->getName()) == 0) {
			spdlog::warn("Failed to remove section '{}' entry '{}' from game configuration.", sharedSection->getName(), (*sectionEntryiterator)->getName());
		}
	}

	sharedSection->m_parentGameConfiguration = nullptr;

	return m_sections.erase(sharedSection->getName()) != 0;
}

bool GameConfiguration::removeSectionWithName(const std::string & sectionName) {
	std::shared_ptr<Section> sharedSection(getSectionWithName(sectionName));

	if(sharedSection == nullptr) {
		return false;
	}

	return removeSection(*sharedSection);
}

void GameConfiguration::clearSections() {
	for(EntryMap::const_iterator i = m_entries.cbegin(); i != m_entries.cend(); ++i) {
		i->second->m_parentGameConfiguration = nullptr;
	}

	for(SectionMap::const_iterator i = m_sections.cbegin(); i != m_sections.cend(); ++i) {
		i->second->m_parentGameConfiguration = nullptr;
	}

	m_entries.clear();
	m_sections.clear();
}

std::string GameConfiguration::toString() const {
	std::stringstream gameConfigurationStream;

	for(SectionMap::const_iterator i = m_sections.cbegin(); i != m_sections.cend(); ++i) {
		if(gameConfigurationStream.tellp() != 0) {
			gameConfigurationStream << std::endl;
		}

		gameConfigurationStream << i->second->toString();
	}

	return gameConfigurationStream.str();
}

std::unique_ptr<GameConfiguration> GameConfiguration::parseFrom(const std::string & data) {
	std::unique_ptr<GameConfiguration> gameConfiguration(std::make_unique<GameConfiguration>());

	if(data.empty()) {
		return std::move(gameConfiguration);
	}

	size_t offset = 0;
	std::shared_ptr<Section> section;

	while(offset <= data.length()) {
		section = std::shared_ptr<Section>(Section::parseFrom(data, offset).release());

		if(section == nullptr) {
			if(offset < data.length()) {
				spdlog::warn("Possible unexpected end of configuration file!");
			}

			break;
		}

		section->m_parentGameConfiguration = gameConfiguration.get();

		for(std::vector<std::shared_ptr<Entry>>::const_iterator i = section->m_entries.cbegin(); i != section->m_entries.cend(); ++i) {
			if(gameConfiguration->hasEntryWithName((*i)->getName())) {
				spdlog::error("Duplicate entry with name '{}'.", (*i)->getName());
				return nullptr;
			}

			(*i)->m_parentGameConfiguration = gameConfiguration.get();
			gameConfiguration->m_entries.emplace((*i)->getName(), *i);
		}

		if(gameConfiguration->hasSectionWithName(section->getName())) {
			spdlog::error("Duplicate section with name '{}'.", section->getName());
			return nullptr;
		}

		gameConfiguration->m_sections.emplace(section->getName(), section);
	}

	if(!gameConfiguration->isValid()) {
		spdlog::warn("Game configuration validation check failed after parsing.");
	}

	return std::move(gameConfiguration);
}

std::unique_ptr<GameConfiguration> GameConfiguration::loadFrom(const std::string & filePath) {
	if(filePath.empty() || !std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		return nullptr;
	}

	std::ifstream fileStream(filePath, std::ios::binary | std::ios::ate);

	if(!fileStream.is_open()) {
		return nullptr;
	}

	size_t fileSize = fileStream.tellg();

	std::string data(fileSize, '\0');

	fileStream.seekg(0, std::ios::beg);
	fileStream.read(data.data(), fileSize);
	fileStream.close();

	std::unique_ptr<GameConfiguration> gameConfiguration(parseFrom(data));

	if(gameConfiguration == nullptr) {
		return nullptr;
	}

	gameConfiguration->m_filePath = filePath;

	return std::move(gameConfiguration);
}

bool GameConfiguration::save(bool overwrite, bool createParentDirectories) const {
	if(m_filePath.empty()) {
		return false;
	}

	return saveTo(m_filePath, overwrite, createParentDirectories);
}

bool GameConfiguration::saveTo(const std::string & filePath, bool overwrite, bool createParentDirectories) const {
	if(!overwrite && std::filesystem::exists(std::filesystem::path(filePath))) {
		return false;
	}

	if(createParentDirectories) {
		std::filesystem::path destinationFileBasePath(Utilities::getFilePath(filePath));

		if(!destinationFileBasePath.empty() && !std::filesystem::exists(std::filesystem::path(destinationFileBasePath))) {
			std::error_code errorCode;
			std::filesystem::create_directories(destinationFileBasePath, errorCode);

			if(errorCode) {
				spdlog::error("Failed to create file destination directory structure for path '{}': {}", destinationFileBasePath.string(), errorCode.message());
				return false;
			}
		}

	}

	std::ofstream fileStream(filePath, std::ios::binary);

	if(!fileStream.is_open()) {
		return false;
	}

	std::string data(toString());

	fileStream.write(data.data(), data.size());

	fileStream.close();

	return true;
}

bool GameConfiguration::isValid(bool validateParents) const {
	for(EntryMap::const_iterator i = m_entries.cbegin(); i != m_entries.cend(); ++i) {
		if(!Entry::isValid(i->second.get(), validateParents) ||
		   !Entry::isNameValid(i->first) ||
		   !Utilities::areStringsEqual(i->first, i->second->getName())) {
			return false;
		}

		if(validateParents &&
		   (i->second->m_parentGameConfiguration != this ||
		    getSection(*i->second->m_parentSection) == nullptr)) {
			return false;
		}
	}

	for(SectionMap::const_iterator i = m_sections.cbegin(); i != m_sections.cend(); ++i) {
		if(!Section::isValid(i->second.get(), validateParents) ||
		   !Section::isNameValid(i->first) ||
		   !Utilities::areStringsEqual(i->first, i->second->getName())) {
			return false;
		}

		if(validateParents && i->second->m_parentGameConfiguration != this) {
			return false;
		}

		for(const std::shared_ptr<Entry> & sectionEntry : i->second->m_entries) {
			if(!hasEntry(*sectionEntry)) {
				return false;
			}
		}
	}

	return true;
}

bool GameConfiguration::isValid(const GameConfiguration * gameConfiguration, bool validateParents) {
	return gameConfiguration != nullptr &&
		   gameConfiguration->isValid(validateParents);
}

void GameConfiguration::updateParent() {
	for(EntryMap::const_iterator i = m_entries.cbegin(); i != m_entries.cend(); ++i) {
		i->second->m_parentGameConfiguration = this;
	}

	for(SectionMap::const_iterator i = m_sections.cbegin(); i != m_sections.cend(); ++i) {
		i->second->m_parentGameConfiguration = this;
		i->second->updateParent();
	}
}

bool GameConfiguration::operator == (const GameConfiguration & c) const {
	return m_entries == c.m_entries &&
		   m_sections == c.m_sections;
}

bool GameConfiguration::operator != (const GameConfiguration & c) const {
	return !operator == (c);
}
