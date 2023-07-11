#include "DOSBoxConfiguration.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

static const std::string AUTOEXEC_SECTION_NAME("autoexec");

DOSBoxConfiguration::Section::Section(std::string_view name, DOSBoxConfiguration * parent)
	: CommentCollection()
	, m_name(name)
	, m_parent(parent) { }

DOSBoxConfiguration::Section::Section(Section && section) noexcept
	: CommentCollection(std::move(section))
	, m_name(std::move(section.m_name))
	, m_entries(std::move(section.m_entries))
	, m_orderedEntryNames(std::move(section.m_orderedEntryNames))
	, m_parent(nullptr) {
	updateParent();
}

DOSBoxConfiguration::Section::Section(const Section & section)
	: CommentCollection(section)
	, m_name(section.m_name)
	, m_orderedEntryNames(section.m_orderedEntryNames)
	, m_parent(nullptr) {
	clearEntries();

	for(const auto & entry : m_entries) {
		m_entries[entry.first] = std::make_shared<Entry>(*entry.second);
	}

	updateParent();
}

DOSBoxConfiguration::Section & DOSBoxConfiguration::Section::operator = (Section && section) noexcept {
	if(this != &section) {
		CommentCollection::operator = (std::move(section));

		m_name = std::move(section.m_name);
		m_entries = std::move(section.m_entries);
		m_orderedEntryNames = std::move(section.m_orderedEntryNames);

		updateParent();
	}

	return *this;
}

DOSBoxConfiguration::Section & DOSBoxConfiguration::Section::operator = (const Section & section) {
	CommentCollection::operator = (section);

	clearEntries();

	m_name = section.m_name;
	m_orderedEntryNames = section.m_orderedEntryNames;

	for(const auto & entry : m_entries) {
		m_entries[entry.first] = std::make_shared<Entry>(*entry.second);
	}

	updateParent();

	return *this;
}

DOSBoxConfiguration::Section::~Section() {
	for(const auto & entry : m_entries) {
		entry.second->m_parent = nullptr;
	}
}

const std::string & DOSBoxConfiguration::Section::getName() const {
	return m_name;
}

bool DOSBoxConfiguration::Section::setName(const std::string & newName) {
	if(m_parent == nullptr) {
		return false;
	}

	return m_parent->setSectionName(*this, newName);
}

bool DOSBoxConfiguration::Section::remove() {
	if(m_parent == nullptr) {
		return false;
	}

	return m_parent->removeSection(*this);
}

const DOSBoxConfiguration * DOSBoxConfiguration::Section::getParentConfiguration() const {
	return m_parent;
}

size_t DOSBoxConfiguration::Section::numberOfEntries() const {
	return m_entries.size();
}

bool DOSBoxConfiguration::Section::hasEntry(const Entry & entry) const {
	if(!entry.isValid(false)) {
		return false;
	}

	EntryMap::const_iterator entryIterator(m_entries.find(entry.m_name));

	if(entryIterator == m_entries.cend() || entryIterator->second.get() != &entry) {
		return false;
	}

	return true;
}

bool DOSBoxConfiguration::Section::hasEntryWithName(const std::string & entryName) const {
	return m_entries.find(entryName) != m_entries.end();
}

size_t DOSBoxConfiguration::Section::indexOfEntry(const Entry & entry) const {
	if(!entry.isValid(true)) {
		return std::numeric_limits<size_t>::max();
	}

	std::vector<std::string>::const_iterator orderedEntryNameIterator(std::find_if(m_orderedEntryNames.cbegin(), m_orderedEntryNames.cend(), [&entry](const std::string & entryName) {
		return Utilities::areStringsEqualIgnoreCase(entry.m_name, entryName);
	}));

	if(orderedEntryNameIterator == m_orderedEntryNames.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	size_t entryIndex = orderedEntryNameIterator - m_orderedEntryNames.cbegin();

	EntryMap::const_iterator entryIterator(m_entries.find(m_orderedEntryNames[entryIndex]));

	if(entryIterator == m_entries.cend() || &entry != entryIterator->second.get()) {
		return std::numeric_limits<size_t>::max();
	}

	return entryIndex;
}

size_t DOSBoxConfiguration::Section::indexOfEntryWithName(const std::string & entryName) const {
	if(!Entry::isNameValid(entryName)) {
		return std::numeric_limits<size_t>::max();
	}

	std::vector<std::string>::const_iterator orderedEntryNameIterator(std::find(m_orderedEntryNames.cbegin(), m_orderedEntryNames.cend(), entryName));

	if(orderedEntryNameIterator == m_orderedEntryNames.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return orderedEntryNameIterator - m_orderedEntryNames.cbegin();
}

std::shared_ptr<DOSBoxConfiguration::Section::Entry> DOSBoxConfiguration::Section::getEntry(size_t entryIndex) const {
	if(entryIndex >= m_entries.size()) {
		return nullptr;
	}

	EntryMap::const_iterator entryIterator(m_entries.find(m_orderedEntryNames[entryIndex]));

	if(entryIterator == m_entries.cend()) {
		return nullptr;
	}

	return entryIterator->second;
}

std::shared_ptr<DOSBoxConfiguration::Section::Entry> DOSBoxConfiguration::Section::getEntry(const Entry & entry) const {
	EntryMap::const_iterator entryIterator(m_entries.find(entry.m_name));

	if(entryIterator == m_entries.cend() || &entry != entryIterator->second.get()) {
		return nullptr;
	}

	return entryIterator->second;
}

std::shared_ptr<DOSBoxConfiguration::Section::Entry> DOSBoxConfiguration::Section::getEntryWithName(const std::string & entryName) const {
	if(!Entry::isNameValid(entryName)) {
		return nullptr;
	}

	EntryMap::const_iterator entryIterator(m_entries.find(entryName));

	if(entryIterator == m_entries.cend()) {
		return nullptr;
	}

	return entryIterator->second;
}

bool DOSBoxConfiguration::Section::setEntryName(size_t entryIndex, const std::string & newEntryName) {
	if(entryIndex >= m_entries.size() || !Entry::isNameValid(newEntryName) || hasEntryWithName(newEntryName)) {
		return false;
	}

	EntryMap::const_iterator entryIterator(m_entries.find(m_orderedEntryNames[entryIndex]));

	if(entryIterator == m_entries.cend()) {
		return false;
	}

	std::shared_ptr<Entry> sharedEntry(entryIterator->second);

	if(m_entries.erase(sharedEntry->m_name) == 0) {
		return false;
	}

	sharedEntry->m_name = newEntryName;
	m_orderedEntryNames[entryIndex] = newEntryName;
	m_entries.emplace(sharedEntry->m_name, sharedEntry);

	return true;
}

bool DOSBoxConfiguration::Section::setEntryName(const std::string & oldEntryName, const std::string & newEntryName) {
	return setEntryName(indexOfEntryWithName(oldEntryName), newEntryName);
}

bool DOSBoxConfiguration::Section::setEntryName(Entry & entry, const std::string & newEntryName) {
	return setEntryName(indexOfEntry(entry), newEntryName);
}

bool DOSBoxConfiguration::Section::addEntry(std::unique_ptr<Entry> newEntry) {
	if(!Entry::isValid(newEntry.get(), false) || hasEntryWithName(newEntry->m_name)) {
		return false;
	}

	m_orderedEntryNames.push_back(newEntry->m_name);
	newEntry->m_parent = this;
	const std::string & entryName = newEntry->m_name;
	m_entries.emplace(entryName, std::move(newEntry));

	return true;
}

bool DOSBoxConfiguration::Section::addEntry(const Entry & newEntry) {
	if(!newEntry.isValid(false) || hasEntryWithName(newEntry.m_name)) {
		return false;
	}

	return addEntry(std::make_unique<Entry>(newEntry));
}

bool DOSBoxConfiguration::Section::replaceEntry(size_t entryIndex, std::unique_ptr<Entry> newEntry) {
	if(!Entry::isValid(newEntry.get(), false) || entryIndex >= m_entries.size()) {
		return false;
	}

	size_t existingEntryIndex = indexOfEntryWithName(newEntry->m_name);

	if(existingEntryIndex != entryIndex && existingEntryIndex != std::numeric_limits<size_t>::max()) {
		return false;
	}

	std::shared_ptr<Entry> sharedNewEntry(std::move(newEntry));
	m_orderedEntryNames[entryIndex] = sharedNewEntry->m_name;
	sharedNewEntry->m_parent = this;
	m_entries[sharedNewEntry->m_name] = sharedNewEntry;

	return true;
}

bool DOSBoxConfiguration::Section::replaceEntry(const Entry & oldEntry, std::unique_ptr<Entry> newEntry) {
	return replaceEntry(indexOfEntry(oldEntry), std::move(newEntry));
}

bool DOSBoxConfiguration::Section::replaceEntryWithName(const std::string & oldEntryName, std::unique_ptr<Entry> newEntry) {
	return replaceEntry(indexOfEntryWithName(oldEntryName), std::move(newEntry));
}

bool DOSBoxConfiguration::Section::replaceEntry(size_t entryIndex, const Entry & newEntry) {
	if(!newEntry.isValid(false) || entryIndex >= m_entries.size()) {
		return false;
	}

	return replaceEntry(entryIndex, std::make_unique<Entry>(newEntry));
}

bool DOSBoxConfiguration::Section::replaceEntry(const Entry & oldEntry, const Entry & newEntry) {
	if(!newEntry.isValid(false)) {
		return false;
	}

	return replaceEntry(oldEntry, std::make_unique<Entry>(newEntry));
}

bool DOSBoxConfiguration::Section::replaceEntryWithName(const std::string & oldEntryName, const Entry & newEntry) {
	if(!newEntry.isValid(false)) {
		return false;
	}

	return replaceEntryWithName(oldEntryName, std::make_unique<Entry>(newEntry));
}

bool DOSBoxConfiguration::Section::insertEntry(size_t entryIndex, std::unique_ptr<Entry> newEntry) {
	if(!Entry::isValid(newEntry.get(), false) || entryIndex >= m_entries.size()) {
		return false;
	}

	size_t existingEntryIndex = indexOfEntryWithName(newEntry->m_name);

	if(existingEntryIndex != entryIndex && existingEntryIndex != std::numeric_limits<size_t>::max()) {
		return false;
	}

	std::shared_ptr<Entry> sharedNewEntry(std::move(newEntry));
	m_orderedEntryNames.insert(m_orderedEntryNames.begin() + entryIndex, sharedNewEntry->m_name);
	sharedNewEntry->m_parent = this;
	m_entries[sharedNewEntry->m_name] = sharedNewEntry;

	return true;
}

bool DOSBoxConfiguration::Section::insertEntry(size_t entryIndex, const Entry & newEntry) {
	if(!newEntry.isValid(false)) {
		return false;
	}

	return insertEntry(entryIndex, std::make_unique<Entry>(newEntry));
}

bool DOSBoxConfiguration::Section::removeEntry(size_t entryIndex) {
	if(entryIndex >= m_entries.size()) {
		return false;
	}

	EntryMap::const_iterator entryIterator(m_entries.find(m_orderedEntryNames[entryIndex]));

	if(entryIterator == m_entries.cend()) {
		return nullptr;
	}

	std::shared_ptr<Entry> sharedEntry(entryIterator->second);

	sharedEntry->m_parent = nullptr;
	bool removed = m_entries.erase(sharedEntry->m_name) != 0;
	m_orderedEntryNames.erase(m_orderedEntryNames.begin() + entryIndex);

	return removed;
}

bool DOSBoxConfiguration::Section::removeEntry(const Entry & entry) {
	return removeEntry(indexOfEntry(entry));
}

bool DOSBoxConfiguration::Section::removeEntryWithName(const std::string & entryName) {
	return removeEntry(indexOfEntryWithName(entryName));
}

void DOSBoxConfiguration::Section::clearEntries() {
	for(auto & entry : m_entries) {
		entry.second->m_parent = nullptr;
	}

	m_entries.clear();
	m_orderedEntryNames.clear();
}

std::unique_ptr<DOSBoxConfiguration::Section> DOSBoxConfiguration::Section::readFrom(const ByteBuffer & data, Style * style) {
	bool error = false;
	bool wasNewlineAfterComments = false;
	size_t previousReadOffset = 0;
	std::string line;
	std::unique_ptr<Section> section;

	while(data.hasMoreLines()) {
		previousReadOffset = data.getReadOffset();
		line = data.readLine(&error);

		if(error) {
			return nullptr;
		}

		if(line.empty()) {
			wasNewlineAfterComments = true;

			continue;
		}

		if(line[0] == Section::NAME_START_CHARACTER && line[line.length() - 1] == Section::NAME_END_CHARACTER) {
			if(section != nullptr) {
				data.setReadOffset(previousReadOffset);

				return section;
			}

			section = std::make_unique<Section>(std::string_view(line).substr(1, line.length() - 2));
		}
		else {
			if(section == nullptr) {
				spdlog::error("Malformed DOSBox configuration section data, expected comment or section name on line: '{}'.", line);
				return nullptr;
			}

			if(line[0] == COMMENT_CHARACTER) {
				section->addComment(std::string_view(line).substr(line.length() >= 2 && (line[1] == ' ' || line[1] == '\t') ? 2 : 1));
			}
			else {
				if(style != nullptr && section != nullptr && wasNewlineAfterComments && section->m_entries.empty()) {
					*style |= Style::NewlineAfterSectionComments;
				}

				if(!section->addEntry(Entry::parseFrom(line, style))) {
					spdlog::error("Malformed DOSBox configuration section '{}' entry on line: '{}'.", section->m_name, line);
					return nullptr;
				}
			}
		}
	}

	return section;
}

bool DOSBoxConfiguration::Section::writeTo(ByteBuffer & data, Style style, const std::string & newline) const {
	if(!isValid(false)) {
		return false;
	}

	const std::string & actualNewline = newline.empty() ? Utilities::newLine : newline;

	if(!data.writeLine(fmt::format("{}{}{}", Section::NAME_START_CHARACTER, m_name, Section::NAME_END_CHARACTER), actualNewline)) {
		return false;
	}

	for(const std::string & comment : m_comments) {
		if(!data.writeLine(fmt::format("{}{}{}", COMMENT_CHARACTER, comment.empty() ? "" : " ", comment), actualNewline)) {
			return false;
		}
	}

	if(!m_comments.empty() && Any(style & Style::NewlineAfterSectionComments)) {
		if(!data.writeLine(Utilities::emptyString, actualNewline)) {
			return false;
		}
	}

	size_t maxEntryNameLength = 0;

	for(const std::string & entryName : m_orderedEntryNames) {
		if(entryName.length() > maxEntryNameLength) {
			maxEntryNameLength = entryName.length();
		}
	}

	for(const std::string & entryName : m_orderedEntryNames) {
		EntryMap::const_iterator entryIterator(m_entries.find(entryName));

		if(entryIterator == m_entries.cend()) {
			return false;
		}

		if(!entryIterator->second->writeTo(data, style, maxEntryNameLength, actualNewline)) {
			return false;
		}
	}

	if(m_entries.empty() && Utilities::areStringsEqual(m_name, AUTOEXEC_SECTION_NAME)) {
		if(!data.writeLine(Utilities::emptyString, actualNewline)) {
			return false;
		}
	}

	return true;
}

bool DOSBoxConfiguration::Section::isValid(bool validateParents) const {
	if(!isNameValid(m_name) ||
	   m_entries.size() != m_orderedEntryNames.size()) {
		return false;
	}

	if(validateParents) {
		if(m_parent == nullptr) {
			return false;
		}
	}

	for(const auto & entry : m_entries) {
		if(!Utilities::areStringsEqual(entry.first, entry.second->m_name) ||
		   !Entry::isValid(entry.second.get(), validateParents)) {
			return false;
		}

		if(validateParents) {
			if(entry.second->m_parent != this) {
				return false;
			}
		}
	}

	return true;
}

bool DOSBoxConfiguration::Section::isValid(const Section * section, bool validateParents) {
	return section != nullptr &&
		   section->isValid(validateParents);
}

bool DOSBoxConfiguration::Section::isNameValid(std::string_view sectionName) {
	static const std::string INVALID_NAME_CHARACTERS(fmt::format("{}{}{}\r\n", COMMENT_CHARACTER, Section::NAME_START_CHARACTER, Section::NAME_END_CHARACTER));

	return !sectionName.empty() &&
		   sectionName.find_first_of(INVALID_NAME_CHARACTERS) == std::string::npos;
}

void DOSBoxConfiguration::Section::updateParent() {
	for(auto & entry : m_entries) {
		entry.second->m_parent = this;
	}
}

bool DOSBoxConfiguration::Section::operator == (const Section & section) const {
	if(CommentCollection::operator != (section) ||
	   !Utilities::areStringsEqualIgnoreCase(m_name, section.m_name) ||
	   m_entries.size() != section.m_entries.size()) {
		return false;
	}

	for(const std::string & entryName : m_orderedEntryNames) {
		EntryMap::const_iterator entryIteratorA(m_entries.find(entryName));

		if(entryIteratorA == m_entries.cend()) {
			return false;
		}

		EntryMap::const_iterator entryIteratorB(section.m_entries.find(entryName));

		if(entryIteratorB == section.m_entries.cend()) {
			return false;
		}

		if(*entryIteratorA->second != *entryIteratorB->second) {
			return false;
		}
	}

	return true;
}

bool DOSBoxConfiguration::Section::operator != (const Section & section) const {
	return !operator == (section);
}
