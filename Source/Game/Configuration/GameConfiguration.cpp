#include "GameConfiguration.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <sstream>

const std::string GameConfiguration::DEFAULT_GAME_CONFIGURATION_FILE_NAME("DUKE3D.CFG");
const std::string GameConfiguration::SETUP_SECTION_NAME("Setup");
const std::string GameConfiguration::SETUP_VERSION_ENTRY_NAME("SetupVersion");
const std::string GameConfiguration::REGULAR_VERSION_SETUP_VERSION("1.3D");
const std::string GameConfiguration::ATOMIC_EDITION_SETUP_VERSION("1.4");
const std::string GameConfiguration::SCREEN_SETUP_SECTION_NAME("Screen Setup");
const std::string GameConfiguration::SCREEN_MODE_ENTRY_NAME("ScreenMode");
const std::string GameConfiguration::SCREEN_WIDTH_ENTRY_NAME("ScreenWidth");
const std::string GameConfiguration::SCREEN_HEIGHT_ENTRY_NAME("ScreenHeight");
const std::string GameConfiguration::SCREEN_SHADOWS_ENTRY_NAME("Shadows");
const std::string GameConfiguration::SCREEN_PASSWORD_ENTRY_NAME("Password");
const std::string GameConfiguration::SCREEN_DETAIL_ENTRY_NAME("Detail");
const std::string GameConfiguration::SCREEN_TILT_ENTRY_NAME("Tilt");
const std::string GameConfiguration::SCREEN_MESSAGES_ENTRY_NAME("Messages");
const std::string GameConfiguration::SCREEN_OUT_ENTRY_NAME("Out");
const std::string GameConfiguration::SCREEN_SIZE_ENTRY_NAME("ScreenSize");
const std::string GameConfiguration::SCREEN_GAMMA_ENTRY_NAME("ScreenGamma");
const std::string GameConfiguration::SOUND_SETUP_SECTION_NAME("Sound Setup");
const std::string GameConfiguration::SOUND_FX_DEVICE_ENTRY_NAME("FXDevice");
const std::string GameConfiguration::SOUND_MUSIC_DEVICE_ENTRY_NAME("MusicDevice");
const std::string GameConfiguration::SOUND_FX_VOLUME_ENTRY_NAME("FXVolume");
const std::string GameConfiguration::SOUND_MUSIC_VOLUME_ENTRY_NAME("MusicVolume");
const std::string GameConfiguration::SOUND_SOUND_TOGGLE_ENTRY_NAME("SoundToggle");
const std::string GameConfiguration::SOUND_VOICE_TOGGLE_ENTRY_NAME("VoiceToggle");
const std::string GameConfiguration::SOUND_AMBIENCE_TOGGLE_ENTRY_NAME("AmbienceToggle");
const std::string GameConfiguration::SOUND_MUSIC_TOGGLE_ENTRY_NAME("MusicToggle");
const std::string GameConfiguration::SOUND_NUM_BITS_ENTRY_NAME("NumBits");
const std::string GameConfiguration::SOUND_MIX_RATE_ENTRY_NAME("MixRate");
const std::string GameConfiguration::KEY_DEFINITIONS_SECTION_NAME("KeyDefinitions");
const std::string GameConfiguration::MISC_SECTION_NAME("Misc");
const std::string GameConfiguration::MISC_EXECUTIONS_ENTRY_NAME("Executions");
const std::string GameConfiguration::MISC_RUN_MODE_ENTRY_NAME("RunMode");
const std::string GameConfiguration::MISC_CROSSHAIRS_ENTRY_NAME("Crosshairs");
const std::string GameConfiguration::MOVE_FORWARD_ENTRY_NAME("Move_Forward");
const std::string GameConfiguration::MOVE_BACKWARD_ENTRY_NAME("Move_Backward");
const std::string GameConfiguration::TURN_LEFT_ENTRY_NAME("Turn_Left");
const std::string GameConfiguration::TURN_RIGHT_ENTRY_NAME("Turn_Right");
const std::string GameConfiguration::FIRE_ENTRY_NAME("Fire");
const std::string GameConfiguration::ALT_FIRE_ENTRY_NAME("Alt_Fire");
const std::string GameConfiguration::QUICK_KICK_ENTRY_NAME("Quick_Kick");
const std::string GameConfiguration::OPEN_ENTRY_NAME("Open");
const std::string GameConfiguration::RUN_ENTRY_NAME("Run");
const std::string GameConfiguration::AUTO_RUN_ENTRY_NAME("AutoRun");
const std::string GameConfiguration::JUMP_ENTRY_NAME("Jump");
const std::string GameConfiguration::CROUCH_ENTRY_NAME("Crouch");
const std::string GameConfiguration::TOGGLE_CROUCH_ENTRY_NAME("Toggle_Crouch");
const std::string GameConfiguration::INVENTORY_ENTRY_NAME("Inventory");
const std::string GameConfiguration::INVENTORY_LEFT_ENTRY_NAME("Inventory_Left");
const std::string GameConfiguration::INVENTORY_RIGHT_ENTRY_NAME("Inventory_Right");
const std::string GameConfiguration::MEDKIT_ENTRY_NAME("MedKit");
const std::string GameConfiguration::TURN_AROUND_ENTRY_NAME("TurnAround");
const std::string GameConfiguration::SEND_MESSAGE_ENTRY_NAME("SendMessage");
const std::string GameConfiguration::HOLO_DUKE_ENTRY_NAME("Holo_Duke");
const std::string GameConfiguration::JETPACK_ENTRY_NAME("Jetpack");
const std::string GameConfiguration::NIGHT_VISION_ENTRY_NAME("NightVision");
const std::string GameConfiguration::PREVIOUS_WEAPON_ENTRY_NAME("Previous_Weapon");
const std::string GameConfiguration::NEXT_WEAPON_ENTRY_NAME("Next_Weapon");
const std::string GameConfiguration::STRAFE_ENTRY_NAME("Strafe");
const std::string GameConfiguration::STRAFE_LEFT_ENTRY_NAME("Strafe_Left");
const std::string GameConfiguration::STRAFE_RIGHT_ENTRY_NAME("Strafe_Right");
const std::string GameConfiguration::LOOK_UP_ENTRY_NAME("Look_Up");
const std::string GameConfiguration::LOOK_DOWN_ENTRY_NAME("Look_Down");
const std::string GameConfiguration::LOOK_LEFT_ENTRY_NAME("Look_Left");
const std::string GameConfiguration::LOOK_RIGHT_ENTRY_NAME("Look_Right");
const std::string GameConfiguration::AIM_UP_ENTRY_NAME("Aim_Up");
const std::string GameConfiguration::AIM_DOWN_ENTRY_NAME("Aim_Down");
const std::string GameConfiguration::SHOW_OPPONENTS_WEAPON_ENTRY_NAME("Show_Opponents_Weapon");
const std::string GameConfiguration::MAP_FOLLOW_MODE_ENTRY_NAME("Map_Follow_Mode");
const std::string GameConfiguration::SEE_COOP_VIEW_ENTRY_NAME("See_Coop_View");
const std::string GameConfiguration::MOUSE_AIMING_ENTRY_NAME("Mouse_Aiming");
const std::string GameConfiguration::TOGGLE_CROSSHAIR_ENTRY_NAME("Toggle_Crosshair");
const std::string GameConfiguration::STEROIDS_ENTRY_NAME("Steroids");
const std::string GameConfiguration::MAP_ENTRY_NAME("Map");
const std::string GameConfiguration::SHRINK_SCREEN_ENTRY_NAME("Shrink_Screen");
const std::string GameConfiguration::ENLARGE_SCREEN_ENTRY_NAME("Enlarge_Screen");
const std::string GameConfiguration::CENTER_VIEW_ENTRY_NAME("Center_View");
const std::string GameConfiguration::HOLSTER_WEAPON_ENTRY_NAME("Holster_Weapon");
const std::string GameConfiguration::THIRD_PERSON_VIEW_ENTRY_NAME("Third_Person_View");
const std::string GameConfiguration::CONTROLS_SECTION_NAME("Controls");
const std::string GameConfiguration::CONTROLLER_TYPE_ENTRY_NAME("ControllerType");
const std::string GameConfiguration::CONTROLS_MOUSE_AIMING_ENTRY_NAME("MouseAiming");
const std::string GameConfiguration::GAME_MOUSE_AIMING_ENTRY_NAME("GameMouseAiming");
const std::string GameConfiguration::MOUSE_AIMING_FLIPPED_ENTRY_NAME("MouseAimingFlipped");
const std::string GameConfiguration::AIMING_FLAG_ENTRY_NAME("AimingFlag");

const std::string GameConfiguration::DEFAULT_PLAYER_NAME("DUKE");
const Dimension GameConfiguration::DEFAULT_RESOLUTION(320, 200);
static constexpr uint8_t AUDIO_DEVICE_UNSET = 13;
const uint8_t GameConfiguration::SOUND_DEVICE_UNSET = AUDIO_DEVICE_UNSET;
const uint8_t GameConfiguration::MUSIC_DEVICE_UNSET = AUDIO_DEVICE_UNSET;
const uint32_t GameConfiguration::DEFAULT_ANALOG_SCALE = std::numeric_limits<uint16_t>::max() + 1;

const std::array<std::string, 10> GameConfiguration::DEFAULT_COMBAT_MACROS({
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
});

bool GameConfiguration::NameComparator::operator () (const std::string & nameA, const std::string & nameB) const {
	return std::lexicographical_compare(nameA.begin(), nameA.end(), nameB.begin(), nameB.end(), [](unsigned char a, unsigned char b) {
		return std::tolower(a) < std::tolower(b);
	});
}

GameConfiguration::GameConfiguration()
	: m_style(Style::None)
	, m_newlineType(Utilities::newLine[0] == '\r' ? NewlineType::Windows : NewlineType::Unix) { }

GameConfiguration::GameConfiguration(GameConfiguration && c) noexcept
	: m_style(c.m_style)
	, m_newlineType(c.m_newlineType)
	, m_filePath(std::move(c.m_filePath))
	, m_entries(std::move(c.m_entries))
	, m_sections(std::move(c.m_sections))
	, m_orderedSectionNames(std::move(c.m_orderedSectionNames)) {
	updateParent();
}

GameConfiguration::GameConfiguration(const GameConfiguration & c)
	: m_style(c.m_style)
	, m_newlineType(c.m_newlineType)
	, m_filePath(c.m_filePath)
	, m_orderedSectionNames(c.m_orderedSectionNames) {
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

		m_style = c.m_style;
		m_newlineType = c.m_newlineType;
		m_filePath = std::move(c.m_filePath);
		m_entries = std::move(c.m_entries);
		m_sections = std::move(c.m_sections);
		m_orderedSectionNames = std::move(c.m_orderedSectionNames);

		updateParent();
	}

	return *this;
}

GameConfiguration & GameConfiguration::operator = (const GameConfiguration & c) {
	clearSections();

	m_style = c.m_style;
	m_newlineType = c.m_newlineType;
	m_orderedSectionNames = c.m_orderedSectionNames;

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

bool GameConfiguration::hasNewlineAfterSections() const {
	return Any(m_style & Style::NewlineAfterSections);
}

GameConfiguration::Style GameConfiguration::getStyle() const {
	return m_style;
}

bool GameConfiguration::hasStyle(Style style) const {
	return (m_style & style) == style;
}

void GameConfiguration::setStyle(Style style) {
	if(m_style == style) {
		return;
	}

	m_style = style;
}

void GameConfiguration::addStyle(Style style) {
	if(hasStyle(style)) {
		return;
	}

	setStyle(m_style | style);
}

void GameConfiguration::removeStyle(Style style) {
	if(!hasStyle(style)) {
		return;
	}

	setStyle(m_style & ~style);
}

GameConfiguration::NewlineType GameConfiguration::getNewlineType() const {
	return m_newlineType;
}

void GameConfiguration::setNewlineType(NewlineType newlineType) {
	if(m_newlineType == newlineType) {
		return;
	}

	m_newlineType = newlineType;
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

size_t GameConfiguration::indexOfSection(const Section & section) const {
	std::vector<std::string>::const_iterator orderedSectionNameIterator = std::find_if(m_orderedSectionNames.cbegin(), m_orderedSectionNames.cend(), [&section](const std::string & sectionName) {
		return Utilities::areStringsEqualIgnoreCase(section.m_name, sectionName);
	});

	if(orderedSectionNameIterator == m_orderedSectionNames.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	size_t sectionIndex = orderedSectionNameIterator - m_orderedSectionNames.cbegin();

	std::shared_ptr<Section> sharedSection(getSection(section));

	if(sharedSection == nullptr) {
		return std::numeric_limits<size_t>::max();
	}

	return sectionIndex;
}

size_t GameConfiguration::indexOfSectionWithName(const std::string & sectionName) const {
	std::shared_ptr<Section> sharedSection(getSectionWithName(sectionName));

	if(sharedSection == nullptr) {
		return std::numeric_limits<size_t>::max();
	}

	return indexOfSection(*sharedSection);
}

std::shared_ptr<GameConfiguration::Section> GameConfiguration::getSection(size_t index) const {
	if(index >= m_orderedSectionNames.size()) {
		return nullptr;
	}

	return getSectionWithName(m_orderedSectionNames[index]);
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

bool GameConfiguration::setSectionName(size_t index, const std::string & newSectionName) {
	if(index >= m_orderedSectionNames.size()) {
		return false;
	}

	return setSectionName(m_orderedSectionNames[index], newSectionName);
}

bool GameConfiguration::setSectionName(const std::string & oldSectionName, const std::string & newSectionName) {
	std::shared_ptr<Section> sharedSection(getSectionWithName(oldSectionName));

	if(sharedSection == nullptr) {
		return false;
	}

	return setSectionName(*sharedSection, newSectionName);
}

bool GameConfiguration::setSectionName(Section & section, const std::string & newSectionName) {
	if(!Section::isNameValid(newSectionName) || hasSectionWithName(newSectionName)) {
		return false;
	}

	size_t sectionIndex = indexOfSection(section);

	if(sectionIndex == std::numeric_limits<size_t>::max() || m_sections.erase(section.m_name) == 0) {
		return false;
	}

	std::shared_ptr<Section> sharedSection(m_sections[m_orderedSectionNames[sectionIndex]]);

	m_orderedSectionNames[sectionIndex] = newSectionName;
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
	m_orderedSectionNames.push_back(section->getName());

	return true;
}

bool GameConfiguration::removeSection(size_t index) {
	if(index >= m_orderedSectionNames.size()) {
		return false;
	}

	return removeSectionWithName(m_orderedSectionNames[index]);
}

bool GameConfiguration::removeSection(const Section & section) {
	size_t sectionIndex = indexOfSection(section);

	if(sectionIndex == std::numeric_limits<size_t>::max()) {
		return false;
	}

	std::shared_ptr<Section> sharedSection(m_sections[m_orderedSectionNames[sectionIndex]]);

	for(std::vector<std::shared_ptr<Entry>>::const_iterator sectionEntryiterator = sharedSection->m_entries.cbegin(); sectionEntryiterator != sharedSection->m_entries.cend(); ++sectionEntryiterator) {
		(*sectionEntryiterator)->m_parentGameConfiguration = nullptr;

		if(m_entries.erase((*sectionEntryiterator)->getName()) == 0) {
			spdlog::warn("Failed to remove section '{}' entry '{}' from game configuration.", sharedSection->getName(), (*sectionEntryiterator)->getName());
		}
	}

	sharedSection->m_parentGameConfiguration = nullptr;
	m_orderedSectionNames.erase(m_orderedSectionNames.begin() + sectionIndex);

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
	m_orderedSectionNames.clear();
}

std::string GameConfiguration::toString() const {
	std::shared_ptr<Section> currentSection;
	std::stringstream gameConfigurationStream;
	std::string newline;

	switch(m_newlineType) {
		case NewlineType::Unix: {
			newline = "\n";
			break;
		}

		case NewlineType::Windows: {
			newline = "\r\n";
			break;
		}
	}

	for(const std::string & sectionName : m_orderedSectionNames) {
		currentSection = m_sections.find(sectionName)->second;

		if(gameConfigurationStream.tellp() != 0) {
			gameConfigurationStream << newline;
		}

		gameConfigurationStream << currentSection->toString(newline);

		if(hasNewlineAfterSections()) {
			gameConfigurationStream << newline;
		}
	}

	return gameConfigurationStream.str();
}

std::unique_ptr<GameConfiguration> GameConfiguration::parseFrom(const std::string & data) {
	std::unique_ptr<GameConfiguration> gameConfiguration(std::make_unique<GameConfiguration>());

	if(data.empty()) {
		return std::move(gameConfiguration);
	}

	char currentCharacter = '\0';
	size_t newlineOffset = 0;

	while(newlineOffset < data.size()) {
		currentCharacter = data[newlineOffset++];

		if(currentCharacter == '\r') {
			spdlog::trace("Detected Windows style newlines in game configuration data.");

			gameConfiguration->setNewlineType(NewlineType::Windows);

			break;
		}
		else if(currentCharacter == '\n') {
			spdlog::trace("Detected Unix style newlines in game configuration data.");

			gameConfiguration->setNewlineType(NewlineType::Unix);

			break;
		}
	}

	size_t offset = 0;
	std::string line;
	std::shared_ptr<Section> section;

	while(offset <= data.length()) {
		section = Section::parseFrom(data, offset);

		if(section == nullptr) {
			if(offset < data.length()) {
				spdlog::warn("Possible unexpected end of configuration file!");
			}

			break;
		}

		if(offset < data.length()) {
			size_t temporaryOffset = offset;
			line = Utilities::readLine(data, temporaryOffset);

			if(line.empty() && !gameConfiguration->hasStyle(Style::NewlineAfterSections)) {
				spdlog::trace("Detected newlines after sections in game configuration data.");

				gameConfiguration->addStyle(Style::NewlineAfterSections);
			}
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
		gameConfiguration->m_orderedSectionNames.push_back(section->getName());
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
		std::error_code errorCode;
		Utilities::createDirectoryStructureForFilePath(filePath, errorCode);

		if(errorCode) {
			spdlog::error("Failed to create file destination directory structure for file path '{}': {}", filePath, errorCode.message());
			return false;
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
		   !Utilities::areStringsEqual(i->first, i->second->getName()) ||
		   m_sections.size() != m_orderedSectionNames.size()) {
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

	for(const std::string & sectionName : m_orderedSectionNames) {
		if(m_sections.find(sectionName) == m_sections.cend()) {
			return false;
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
