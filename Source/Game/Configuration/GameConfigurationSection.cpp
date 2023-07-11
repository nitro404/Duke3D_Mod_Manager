#include "GameConfiguration.h"

#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

#include <sstream>

static size_t numberOfComments(const std::string & originalComments) {
	if(originalComments.empty()) {
		return 0;
	}

	size_t offset = 0;
	size_t commentCount = 0;

	while(offset < originalComments.size()) {
		Utilities::readLine(originalComments, offset);
		commentCount++;
	}

	return commentCount;
}

static std::string getComment(const std::string & originalComments, size_t index) {
	if(originalComments.empty()) {
		return {};
	}

	size_t offset = 0;
	size_t currentIndex = 0;
	std::string_view comment;

	while(offset < originalComments.size()) {
		comment = Utilities::readLine(originalComments, offset);

		if(currentIndex == index) {
			return std::string(comment);
		}

		currentIndex++;
	}

	return {};
}

static void addComment(std::string & originalComments, const std::string & newComment) {
	originalComments = originalComments + (originalComments.empty() ? "" : "\n") + newComment;
}

static bool insertComment(std::string & originalComments, const std::string & newComment, size_t index) {
	if(originalComments.empty()) {
		if(index == 0) {
			originalComments = newComment;
			return true;
		}

		return false;
	}

	size_t currentOffset = 0;
	size_t previousOffset = 0;
	size_t currentIndex = 0;

	while(currentOffset < originalComments.size()) {
		Utilities::readLine(originalComments, currentOffset);

		if(currentIndex == index) {
			std::stringstream commentStream;
			if(previousOffset != 0) {
				commentStream << std::string_view(originalComments.data(), previousOffset - 1);
			}

			if(commentStream.tellp() != 0) {
				commentStream << '\n';
			}

			commentStream << newComment << '\n';
			commentStream << std::string_view(originalComments.data() + previousOffset, originalComments.length() - previousOffset);

			originalComments = commentStream.str();

			return true;
		}

		previousOffset = currentOffset;
		currentIndex++;
	}

	if(currentIndex == index) {
		originalComments = originalComments + '\n' + newComment;
		return true;
	}

	return false;
}

static bool removeComment(std::string & originalComments, size_t index) {
	if(originalComments.empty()) {
		return false;
	}

	size_t currentOffset = 0;
	size_t previousOffset = 0;
	size_t currentIndex = 0;

	while(currentOffset < originalComments.size()) {
		Utilities::readLine(originalComments, currentOffset);

		if(currentIndex == index) {
			std::stringstream commentStream;

			if(previousOffset != 0) {
				commentStream << std::string_view(originalComments.data(), previousOffset - 1);
			}

			if(commentStream.tellp() != 0) {
				commentStream << '\n';
			}

			commentStream << std::string_view(originalComments.data() + currentOffset, originalComments.length() - currentOffset);

			originalComments = commentStream.str();

			return true;
		}

		previousOffset = currentOffset;
		currentIndex++;
	}

	return false;
}

GameConfiguration::Section::Section(const std::string & name, const std::string & precedingComments, const std::string & followingComments)
	: m_name(name)
	, m_precedingComments(precedingComments)
	, m_followingComments(followingComments)
	, m_parentGameConfiguration(nullptr) { }

GameConfiguration::Section::Section(Section && s) noexcept
	: m_name(std::move(s.m_name))
	, m_precedingComments(std::move(s.m_precedingComments))
	, m_followingComments(std::move(s.m_followingComments))
	, m_entries(std::move(s.m_entries))
	, m_parentGameConfiguration(nullptr) {
	updateParent();
}

GameConfiguration::Section::Section(const Section & s)
	: m_name(s.m_name)
	, m_precedingComments(s.m_precedingComments)
	, m_followingComments(s.m_followingComments)
	, m_parentGameConfiguration(nullptr) {
	clearEntries();

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = s.m_entries.cbegin(); i != s.m_entries.cend(); ++i) {
		m_entries.emplace_back(std::make_shared<Entry>(**i));
	}

	updateParent();
}

GameConfiguration::Section & GameConfiguration::Section::operator = (Section && s) noexcept {
	if(this != &s) {
		clearEntries();

		m_name = std::move(s.m_name);
		m_precedingComments = std::move(s.m_precedingComments);
		m_followingComments = std::move(s.m_followingComments);
		m_entries = std::move(s.m_entries);

		updateParent();
	}

	return *this;
}

GameConfiguration::Section & GameConfiguration::Section::operator = (const Section & s) {
	clearEntries();

	m_name = s.m_name;
	m_precedingComments = s.m_precedingComments;
	m_followingComments = s.m_followingComments;

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = s.m_entries.cbegin(); i != s.m_entries.cend(); ++i) {
		m_entries.emplace_back(std::make_shared<Entry>(**i));
	}

	updateParent();

	return *this;
}

GameConfiguration::Section::~Section() {
	for(std::shared_ptr<Entry> & entry : m_entries) {
		entry->m_parentSection = nullptr;
	}
}

const std::string & GameConfiguration::Section::getName() const {
	return m_name;
}

bool GameConfiguration::Section::setName(const std::string & newName) {
	if(m_parentGameConfiguration == nullptr) {
		return false;
	}

	return m_parentGameConfiguration->setSectionName(*this, newName);
}

size_t GameConfiguration::Section::numberOfPrecedingComments() const {
	return numberOfComments(m_precedingComments);
}

std::string GameConfiguration::Section::getPrecedingComment(size_t index) const {
	return getComment(m_precedingComments, index);
}

const std::string & GameConfiguration::Section::getPrecedingComments() const {
	return m_precedingComments;
}

void GameConfiguration::Section::setPrecedingComments(const std::string & precedingComments) {
	m_precedingComments = precedingComments;
}

void GameConfiguration::Section::addPrecedingComment(const std::string & newComment) {
	addComment(m_precedingComments, newComment);
}

bool GameConfiguration::Section::insertPrecedingComment(const std::string & newComment, size_t index) {
	return insertComment(m_precedingComments, newComment, index);
}

bool GameConfiguration::Section::removePrecedingComment(size_t index) {
	return removeComment(m_precedingComments, index);
}

void GameConfiguration::Section::clearPrecedingComments() {
	m_precedingComments = "";
}

size_t GameConfiguration::Section::numberOfFollowingComments() const {
	return numberOfComments(m_followingComments);
}

std::string GameConfiguration::Section::getFollowingComment(size_t index) const {
	return getComment(m_followingComments, index);
}

const std::string & GameConfiguration::Section::getFollowingComments() const {
	return m_followingComments;
}

void GameConfiguration::Section::setFollowingComments(const std::string & followingComments) {
	m_followingComments = followingComments;
}

void GameConfiguration::Section::addFollowingComment(const std::string & newComment) {
	addComment(m_followingComments, newComment);
}

bool GameConfiguration::Section::insertFollowingComment(const std::string & newComment, size_t index) {
	return insertComment(m_followingComments, newComment, index);
}

bool GameConfiguration::Section::removeFollowingComment(size_t index) {
	return removeComment(m_followingComments, index);
}

void GameConfiguration::Section::clearFollowingComments() {
	m_followingComments = "";
}

size_t GameConfiguration::Section::numberOfEntries() const {
	return m_entries.size();
}

bool GameConfiguration::Section::hasEntry(const Entry & entry) const {
	return std::find_if(std::begin(m_entries), std::end(m_entries), [&entry](const std::shared_ptr<Entry> & currentEntry) {
		return &entry == currentEntry.get();
	}) != std::end(m_entries);
}

bool GameConfiguration::Section::hasEntryWithName(const std::string & entryName) const {
	return std::find_if(std::begin(m_entries), std::end(m_entries), [&entryName](const std::shared_ptr<Entry> & currentEntry) {
		return Utilities::areStringsEqualIgnoreCase(entryName, currentEntry->getName());
	}) != std::end(m_entries);
}

size_t GameConfiguration::Section::indexOfEntry(const Entry & entry) const {
	auto entryIterator = std::find_if(std::begin(m_entries), std::end(m_entries), [entry](const std::shared_ptr<Entry> & currentEntry) {
		return &entry == currentEntry.get();
	});

	if(entryIterator == std::end(m_entries)) {
		return std::numeric_limits<size_t>::max();
	}

	return entryIterator - std::begin(m_entries);
}

size_t GameConfiguration::Section::indexOfEntryWithName(const std::string & entryName) const {
	auto entryIterator = std::find_if(std::begin(m_entries), std::end(m_entries), [entryName](const std::shared_ptr<Entry> & currentEntry) {
		return Utilities::areStringsEqualIgnoreCase(entryName, currentEntry->getName());
	});

	if(entryIterator == std::end(m_entries)) {
		return std::numeric_limits<size_t>::max();
	}

	return entryIterator - std::begin(m_entries);
}

std::shared_ptr<GameConfiguration::Entry> GameConfiguration::Section::getEntry(size_t index) const {
	if(index >= m_entries.size()) {
		return nullptr;
	}

	return m_entries[index];
}

std::shared_ptr<GameConfiguration::Entry> GameConfiguration::Section::getEntryWithName(const std::string & entryName) const {
	auto entryIterator = std::find_if(std::begin(m_entries), std::end(m_entries), [entryName](const std::shared_ptr<Entry> & currentEntry) {
		return Utilities::areStringsEqualIgnoreCase(entryName, currentEntry->getName());
	});

	if(entryIterator == std::end(m_entries)) {
		return nullptr;
	}

	return *entryIterator;
}

bool GameConfiguration::Section::addEntry(std::shared_ptr<Entry> entry) {
	if(m_parentGameConfiguration == nullptr) {
		return false;
	}

	return m_parentGameConfiguration->addEntryToSection(entry, *this);
}

bool GameConfiguration::Section::addEmptyEntry(const std::string & entryName) {
	if(!Entry::isNameValid(entryName)) {
		return false;
	}

	std::shared_ptr<Entry> entry(std::make_shared<Entry>(entryName));
	entry->setEmpty();

	if(!entry->isValid(false)) {
		return false;
	}

	return addEntry(entry);
}

bool GameConfiguration::Section::addIntegerEntry(const std::string & entryName, uint64_t value) {
	if(!Entry::isNameValid(entryName)) {
		return false;
	}

	std::shared_ptr<Entry> entry(std::make_shared<Entry>(entryName));
	entry->setIntegerValue(value);

	if(!entry->isValid(false)) {
		return false;
	}

	return addEntry(entry);
}

bool GameConfiguration::Section::addHexadecimalEntryUsingDecimal(const std::string & entryName, uint64_t value) {
	if(!Entry::isNameValid(entryName)) {
		return false;
	}

	std::shared_ptr<Entry> entry(std::make_shared<Entry>(entryName));
	entry->setHexadecimalValueFromDecimal(value);

	if(!entry->isValid(false)) {
		return false;
	}

	return addEntry(entry);
}

bool GameConfiguration::Section::addStringEntry(const std::string & entryName, const std::string & value) {
	if(!Entry::isNameValid(entryName)) {
		return false;
	}

	std::shared_ptr<Entry> entry(std::make_shared<Entry>(entryName));
	entry->setStringValue(value);

	if(!entry->isValid(false)) {
		return false;
	}

	return addEntry(entry);
}

bool GameConfiguration::Section::addMultiStringEntry(const std::string & entryName, const std::string & valueA, const std::string & valueB) {
	if(!Entry::isNameValid(entryName)) {
		return false;
	}

	std::shared_ptr<Entry> entry(std::make_shared<Entry>(entryName));
	entry->setMultiStringValue(valueA, valueB);

	if(!entry->isValid(false)) {
		return false;
	}

	return addEntry(entry);
}

bool GameConfiguration::Section::addMultiStringEntry(const std::string & entryName, const std::vector<std::string> & values) {
	if(!Entry::isNameValid(entryName)) {
		return false;
	}

	std::shared_ptr<Entry> entry(std::make_shared<Entry>(entryName));
	entry->setMultiStringValue(values);

	if(!entry->isValid(false)) {
		return false;
	}

	return addEntry(entry);
}

std::shared_ptr<GameConfiguration::Entry> GameConfiguration::Section::getOrCreateEntry(const std::string & entryName, bool createEntry, bool & entryExists) {
	if(!Entry::isNameValid(entryName)) {
		return nullptr;
	}

	std::shared_ptr<Entry> entry(getEntryWithName(entryName));
	bool doesEntryAlreadyExist = entry != nullptr;

	if(!doesEntryAlreadyExist) {
		if(!createEntry) {
			return nullptr;
		}

		entry = std::make_shared<Entry>(entryName);
	}

	if(!entry->isValid(false)) {
		return nullptr;
	}

	entryExists = doesEntryAlreadyExist;

	return entry;
}

bool GameConfiguration::Section::setEntryEmptyValue(const std::string & entryName, bool create) {
	bool entryExists = false;
	std::shared_ptr<Entry> entry(getOrCreateEntry(entryName, create, entryExists));

	entry->setEmpty();

	if(!entryExists) {
		return addEntry(entry);
	}

	return true;
}

bool GameConfiguration::Section::setEntryIntegerValue(const std::string & entryName, uint64_t value, bool createEntry) {
	bool entryExists = false;
	std::shared_ptr<Entry> entry(getOrCreateEntry(entryName, createEntry, entryExists));

	entry->setIntegerValue(value);

	if(!entryExists) {
		return addEntry(entry);
	}

	return true;
}

bool GameConfiguration::Section::setEntryHexadecimalValueUsingDecimal(const std::string & entryName, uint64_t value, bool createEntry) {
	bool entryExists = false;
	std::shared_ptr<Entry> entry(getOrCreateEntry(entryName, createEntry, entryExists));

	entry->setHexadecimalValueFromDecimal(value);

	if(!entryExists) {
		return addEntry(entry);
	}

	return true;
}

bool GameConfiguration::Section::setEntryStringValue(const std::string & entryName, const std::string & value, bool createEntry) {
	bool entryExists = false;
	std::shared_ptr<Entry> entry(getOrCreateEntry(entryName, createEntry, entryExists));

	entry->setStringValue(value);

	if(!entryExists) {
		return addEntry(entry);
	}

	return true;
}

bool GameConfiguration::Section::setEntryMultiStringValue(const std::string & entryName, const std::string & value, size_t index, bool resizeValue, bool createEntry) {
	bool entryExists = false;
	std::shared_ptr<Entry> entry(getOrCreateEntry(entryName, createEntry, entryExists));

	if(!entry->setMultiStringValue(value, index, resizeValue)) {
		return false;
	}

	if(!entryExists) {
		return addEntry(entry);
	}

	return true;
}

bool GameConfiguration::Section::setEntryMultiStringValue(const std::string & entryName, const std::string & valueA, const std::string & valueB, bool createEntry) {
	bool entryExists = false;
	std::shared_ptr<Entry> entry(getOrCreateEntry(entryName, createEntry, entryExists));

	entry->setMultiStringValue(valueA, valueB);

	if(!entryExists) {
		return addEntry(entry);
	}

	return true;
}

bool GameConfiguration::Section::setEntryMultiStringValue(const std::string & entryName, const std::vector<std::string> & values, bool createEntry) {
	bool entryExists = false;
	std::shared_ptr<Entry> entry(getOrCreateEntry(entryName, createEntry, entryExists));

	entry->setMultiStringValue(values);

	if(!entryExists) {
		return addEntry(entry);
	}

	return true;
}

bool GameConfiguration::Section::removeEntry(size_t index) {
	if(m_parentGameConfiguration == nullptr || index >= m_entries.size()) {
		return false;
	}

	return m_parentGameConfiguration->removeEntry(*m_entries[index]);
}

bool GameConfiguration::Section::removeEntry(const Entry & entry) {
	return removeEntry(indexOfEntry(entry));
}

bool GameConfiguration::Section::removeEntryWithName(const std::string & entryName) {
	return removeEntry(indexOfEntryWithName(entryName));
}

bool GameConfiguration::Section::clearEntries() {
	if(m_parentGameConfiguration == nullptr) {
		return false;
	}

	return m_parentGameConfiguration->removeEntries(m_entries);
}

bool GameConfiguration::Section::remove() {
	if(m_parentGameConfiguration == nullptr) {
		return false;
	}

	return m_parentGameConfiguration->removeSection(*this);
}

const GameConfiguration * GameConfiguration::Section::getParentGameConfiguration() const {
	return m_parentGameConfiguration;
}

std::string GameConfiguration::Section::toString() const {
	std::stringstream sectionData;

	std::string precedingComments(formatComments(m_precedingComments));

	sectionData << precedingComments;

	if(!precedingComments.empty()) {
		sectionData << std::endl;
	}

	sectionData << GameConfiguration::Section::NAME_START_CHARACTER << m_name << GameConfiguration::Section::NAME_END_CHARACTER;

	std::string followingComments(formatComments(m_followingComments));

	if(!followingComments.empty()) {
		sectionData << std::endl;
	}

	sectionData << followingComments;

	for(std::vector<std::shared_ptr<Entry>>::const_iterator i = m_entries.cbegin(); i != m_entries.cend(); ++i) {
		sectionData << std::endl << (*i)->toString();
	}

	return sectionData.str();
}

std::unique_ptr<GameConfiguration::Section> GameConfiguration::Section::parseFrom(const std::string & data, size_t & offset) {
	std::string_view line;

	size_t lastEntryOffset = offset;
	std::string sectionName;
	std::stringstream precedingCommentsStream;
	std::stringstream followingCommentsStream;
	std::shared_ptr<Entry> entry;
	std::vector<std::shared_ptr<Entry>> entries;

	while(offset < data.length()) {
		line = Utilities::readLine(data, offset);

		if(line.empty()) {
			continue;
		}

		if(line[0] == Section::NAME_START_CHARACTER) {
			if(!sectionName.empty()) {
				break;
			}

			size_t sectionNameEndCharacter = line.find_last_not_of(Section::NAME_END_CHARACTER);

			if(sectionNameEndCharacter == std::string::npos) {
				spdlog::warn("Invalid section name identifier, missing '{}' end character.", Section::NAME_END_CHARACTER);
				continue;
			}

			sectionName = std::string(line.data() + 1, line.length() - 2);
		}
		else if(line[0] == COMMENT_CHARACTER) {
			if(!entries.empty()) {
				continue;
			}

			std::string_view comment(line.data() + 1, line.length() - 1);

			std::stringstream * commentsStream = sectionName.empty() ? &precedingCommentsStream : &followingCommentsStream;

			if(commentsStream->tellp() != 0) {
				*commentsStream << '\n';
			}

			*commentsStream << comment;
		}
		else {
			entry = std::shared_ptr<Entry>(Entry::parseFrom(line).release());

			if(entry == nullptr) {
				spdlog::warn("Failed to parse configuration entry from line: '{}' at offset {}.", line, offset - line.length());
			}
			else {
				entries.emplace_back(entry);
			}
		}

		lastEntryOffset = offset;
	}

	if(sectionName.empty()) {
		return nullptr;
	}

	// shift offset back to where the last entry was read so that preceding comments are not truncated
	offset = lastEntryOffset;

	std::unique_ptr<Section> section(new Section(sectionName, precedingCommentsStream.str(), followingCommentsStream.str()));

	for(const std::shared_ptr<Entry> & entry : entries) {
		entry->m_parentSection = section.get();
	}

	section->m_entries = std::move(entries);

	return section;
}

bool GameConfiguration::Section::isValid(bool validateParents) const {
	if(!isNameValid(m_name)) {
		return false;
	}

	if(validateParents && m_parentGameConfiguration == nullptr) {
		return false;
	}

	for(const std::shared_ptr<Entry> & entry : m_entries) {
		if(!Entry::isValid(entry.get(), validateParents)) {
			return false;
		}

		if(validateParents &&
		   (entry->m_parentSection != this ||
		    entry->m_parentGameConfiguration != m_parentGameConfiguration)) {
			return false;
		}
	}

	return true;
}

bool GameConfiguration::Section::isValid(const Section * section, bool validateParents) {
	return section != nullptr &&
		   section->isValid(validateParents);
}

bool GameConfiguration::Section::isNameValid(const std::string & sectionName) {
	static const std::string INVALID_NAME_CHARACTERS(fmt::format("{}{}{}\r\n", Section::COMMENT_CHARACTER, Section::NAME_START_CHARACTER, Section::NAME_END_CHARACTER));

	return !sectionName.empty() &&
		   sectionName.find_first_of(INVALID_NAME_CHARACTERS) == std::string::npos;
}

std::string GameConfiguration::Section::formatComments(const std::string & unformattedComments) {
	std::stringstream formattedCommentsStream;

	std::string_view unformattedComment;
	size_t offset = 0;

	while(offset < unformattedComments.length()) {
		unformattedComment = Utilities::readLine(unformattedComments, offset);

		if(formattedCommentsStream.tellp() != 0) {
			formattedCommentsStream << '\n';
		}

		formattedCommentsStream << GameConfiguration::Section::COMMENT_CHARACTER << unformattedComment;
	}

	return formattedCommentsStream.str();
}

void GameConfiguration::Section::updateParent() {
	for(std::shared_ptr<Entry> & entry : m_entries) {
		entry->m_parentSection = this;
	}
}

bool GameConfiguration::Section::operator == (const Section & s) const {
	return Utilities::areStringsEqualIgnoreCase(m_name, s.m_name) &&
		   Utilities::areStringsEqual(m_precedingComments, s.m_precedingComments) &&
		   Utilities::areStringsEqual(m_followingComments, s.m_followingComments) &&
		   m_entries == s.m_entries;
}

bool GameConfiguration::Section::operator != (const Section & s) const {
	return !operator == (s);
}
