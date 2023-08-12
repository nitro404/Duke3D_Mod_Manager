#include "DOSBoxConfiguration.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

static const std::string AUTOEXEC_SECTION_NAME("autoexec");

DOSBoxConfiguration::Section::Section(std::string_view name, DOSBoxConfiguration * parent)
	: CommentCollection()
	, m_name(name)
	, m_modified(false)
	, m_parent(parent) {
	connectSignals();
}

DOSBoxConfiguration::Section::Section(std::string_view name, std::vector<std::unique_ptr<Entry>> entries, DOSBoxConfiguration * parent)
	: CommentCollection()
	, m_name(name)
	, m_modified(false)
	, m_parent(parent) {
	connectSignals();

	for(std::unique_ptr<Entry> & entry : entries) {
		addEntry(std::move(entry));
	}
}

DOSBoxConfiguration::Section::Section(std::string_view name, const std::vector<Entry> & entries, DOSBoxConfiguration * parent)
	: CommentCollection()
	, m_name(name)
	, m_modified(false)
	, m_parent(parent) {
	connectSignals();

	for(const Entry & entry : entries) {
		addEntry(entry);
	}
}

DOSBoxConfiguration::Section::Section(Section && section) noexcept
	: CommentCollection(std::move(section))
	, m_name(std::move(section.m_name))
	, m_entries(std::move(section.m_entries))
	, m_orderedEntryNames(std::move(section.m_orderedEntryNames))
	, m_modified(false)
	, m_parent(nullptr) {
	updateParent();
	connectSignals();
}

DOSBoxConfiguration::Section::Section(const Section & section)
	: CommentCollection(section)
	, m_name(section.m_name)
	, m_orderedEntryNames(section.m_orderedEntryNames)
	, m_modified(false)
	, m_parent(nullptr) {
	for(const auto & entry : section.m_entries) {
		m_entries[entry.first] = std::make_shared<Entry>(*entry.second);
	}

	updateParent();
	connectSignals();
}

DOSBoxConfiguration::Section & DOSBoxConfiguration::Section::operator = (Section && section) noexcept {
	if(this != &section) {
		disconnectSignals();

		CommentCollection::operator = (std::move(section));

		clearEntries();

		m_name = std::move(section.m_name);
		m_entries = std::move(section.m_entries);
		m_orderedEntryNames = std::move(section.m_orderedEntryNames);

		updateParent();
		connectSignals();

		for(size_t i = 0; i < m_entries.size(); i++) {
			sectionEntryAdded(*this, m_entries[m_orderedEntryNames[i]], i);
		}

		setModified(true);
	}

	return *this;
}

DOSBoxConfiguration::Section & DOSBoxConfiguration::Section::operator = (const Section & section) {
	disconnectSignals();

	CommentCollection::operator = (section);

	clearEntries();

	m_name = section.m_name;
	m_orderedEntryNames = section.m_orderedEntryNames;

	for(const auto & entry : section.m_entries) {
		m_entries[entry.first] = std::make_shared<Entry>(*entry.second);
	}

	updateParent();
	connectSignals();

	for(size_t i = 0; i < m_entries.size(); i++) {
		sectionEntryAdded(*this, m_entries[m_orderedEntryNames[i]], i);
	}

	setModified(true);

	return *this;
}

DOSBoxConfiguration::Section::~Section() {
	disconnectSignals();

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

bool DOSBoxConfiguration::Section::isEmpty() const {
	return m_entries.empty() &&
		   m_comments.empty();
}

bool DOSBoxConfiguration::Section::isNotEmpty() const {
	return !m_entries.empty() ||
		   !m_comments.empty();
}

bool DOSBoxConfiguration::Section::mergeWith(const Section & section) {
	if(!isValid(true) || !section.isValid(true)) {
		return false;
	}

	CommentCollection::mergeWith(section);

	for(const auto & currentEntry : section.m_entries) {
		std::shared_ptr<Entry> existingEntry(getEntryWithName(currentEntry.second->getName()));

		if(existingEntry != nullptr) {
			existingEntry->setValue(currentEntry.second->getValue());
		}
		else {
			if(!addEntry(*currentEntry.second)) {
				return false;
			}
		}
	}

	setModified(true);

	return true;
}

bool DOSBoxConfiguration::Section::setSection(const Section & section) {
	if(!section.isValid()) {
		return false;
	}

	setComments(section.m_comments);

	for(const auto & currentEntry : m_entries) {
		size_t existingEntryIndex = section.indexOfEntryWithName(currentEntry.second->getName());

		if(existingEntryIndex == std::numeric_limits<size_t>::max()) {
			if(!removeEntry(existingEntryIndex)) {
				return false;
			}
		}
	}

	for(const auto & currentEntry : section.m_entries) {
		std::shared_ptr<Entry> existingEntry(getEntryWithName(currentEntry.second->getName()));

		if(existingEntry != nullptr) {
			existingEntry->setValue(currentEntry.second->getValue());
		}
		else {
			if(!addEntry(std::make_unique<Entry>(*currentEntry.second))) {
				return false;
			}
		}
	}

	m_orderedEntryNames = section.m_orderedEntryNames;

	setModified(true);

	return true;
}

void DOSBoxConfiguration::Section::clear() {
	clearComments();
	clearEntries();
}

bool DOSBoxConfiguration::Section::isModified() const {
	return m_modified;
}

void DOSBoxConfiguration::Section::setModified(bool value) {
	m_modified = value;

	if(!m_modified) {
		m_commentCollectionModified = false;

		for(auto & entry : m_entries) {
			entry.second->setModified(false);
		}
	}

	sectionModified(*this);
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

const DOSBoxConfiguration::Section::EntryMap & DOSBoxConfiguration::Section::getUnorderedEntries() const {
	return m_entries;
}

std::vector<std::shared_ptr<DOSBoxConfiguration::Section::Entry>> DOSBoxConfiguration::Section::getOrderedEntries() const {
	std::vector<std::shared_ptr<Entry>> orderedEntries;

	for(const std::string & entryName : m_orderedEntryNames) {
		EntryMap::const_iterator entryIterator(m_entries.find(entryName));

		if(entryIterator == m_entries.cend()) {
			continue;
		}

		orderedEntries.push_back(entryIterator->second);
	}

	return orderedEntries;
}

const std::vector<std::string> & DOSBoxConfiguration::Section::getOrderedEntryNames() const {
	return m_orderedEntryNames;
}

bool DOSBoxConfiguration::Section::setEntryName(size_t entryIndex, const std::string & newEntryName) {
	if(entryIndex >= m_entries.size() || !Entry::isNameValid(newEntryName)) {
		return false;
	}

	EntryMap::const_iterator entryIterator(m_entries.find(m_orderedEntryNames[entryIndex]));

	if(entryIterator == m_entries.cend()) {
		return false;
	}

	std::shared_ptr<Entry> sharedEntry(entryIterator->second);

	if(Utilities::areStringsEqual(sharedEntry->getName(), newEntryName)) {
		return true;
	}

	if(hasEntryWithName(newEntryName) || m_entries.erase(sharedEntry->m_name) == 0) {
		return false;
	}

	std::string oldEntryName(sharedEntry->m_name);

	sharedEntry->m_name = newEntryName;
	m_orderedEntryNames[entryIndex] = newEntryName;
	m_entries.emplace(sharedEntry->m_name, sharedEntry);

	sharedEntry->entryNameChanged(*sharedEntry, oldEntryName);

	sharedEntry->setModified(true);

	return true;
}

bool DOSBoxConfiguration::Section::setEntryName(const std::string & oldEntryName, const std::string & newEntryName) {
	return setEntryName(indexOfEntryWithName(oldEntryName), newEntryName);
}

bool DOSBoxConfiguration::Section::setEntryName(Entry & entry, const std::string & newEntryName) {
	return setEntryName(indexOfEntry(entry), newEntryName);
}

bool DOSBoxConfiguration::Section::setEntryValue(size_t entryIndex, const std::string & newEntryValue) {
	if(entryIndex >= m_entries.size()) {
		return false;
	}

	EntryMap::const_iterator entryIterator(m_entries.find(m_orderedEntryNames[entryIndex]));

	if(entryIterator == m_entries.cend()) {
		return false;
	}

	entryIterator->second->setValue(newEntryValue);

	return true;
}

bool DOSBoxConfiguration::Section::setEntryValue(const std::string & entryName, const std::string & newEntryValue) {
	std::shared_ptr<Entry> internalEntry(getEntryWithName(entryName));

	if(internalEntry == nullptr) {
		return false;
	}

	internalEntry->setValue(newEntryValue);

	return true;
}

bool DOSBoxConfiguration::Section::setEntryValue(Entry & entry, const std::string & newEntryValue) {
	std::shared_ptr<Entry> internalEntry(getEntryWithName(entry.getName()));

	if(internalEntry.get() != &entry) {
		return false;
	}

	internalEntry->setValue(newEntryValue);

	return true;
}

bool DOSBoxConfiguration::Section::addEntry(std::unique_ptr<Entry> newEntry) {
	if(!Entry::isValid(newEntry.get(), false) || hasEntryWithName(newEntry->m_name)) {
		return false;
	}

	m_orderedEntryNames.push_back(newEntry->m_name);
	newEntry->m_parent = this;
	const std::string & entryName = newEntry->m_name;
	m_entryConnections.emplace_back(connectEntrySignals(*newEntry));
	m_entries.emplace(entryName, std::move(newEntry));

	sectionEntryAdded(*this, m_entries[m_orderedEntryNames.back()], m_entries.size() - 1);

	setModified(true);

	return true;
}

bool DOSBoxConfiguration::Section::addEntry(const Entry & newEntry) {
	if(!newEntry.isValid(false) || hasEntryWithName(newEntry.m_name)) {
		return false;
	}

	return addEntry(std::make_unique<Entry>(newEntry));
}

bool DOSBoxConfiguration::Section::addEntry(const std::string & newEntryName, const std::string & newEntryValue) {
	if(!Entry::isNameValid(newEntryName) || hasEntryWithName(newEntryName)) {
		return false;
	}

	return addEntry(std::make_unique<Entry>(newEntryName, newEntryValue));
}

bool DOSBoxConfiguration::Section::replaceEntry(size_t entryIndex, std::unique_ptr<Entry> newEntry) {
	if(!Entry::isValid(newEntry.get(), false) || entryIndex >= m_entries.size()) {
		return false;
	}

	size_t existingEntryIndex = indexOfEntryWithName(newEntry->m_name);

	if(existingEntryIndex != entryIndex && existingEntryIndex != std::numeric_limits<size_t>::max()) {
		return false;
	}

	std::shared_ptr<Entry> oldEntry(m_entries[newEntry->m_name]);

	std::shared_ptr<Entry> sharedNewEntry(std::move(newEntry));
	m_orderedEntryNames[entryIndex] = sharedNewEntry->m_name;
	sharedNewEntry->m_parent = this;
	m_entryConnections[entryIndex] = connectEntrySignals(*sharedNewEntry);
	m_entries[sharedNewEntry->m_name] = std::move(sharedNewEntry);

	sectionEntryReplaced(*this, sharedNewEntry, entryIndex, oldEntry);

	setModified(true);

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
	m_entryConnections.insert(m_entryConnections.begin() + entryIndex, connectEntrySignals(*sharedNewEntry));
	m_entries[sharedNewEntry->m_name] = std::move(sharedNewEntry);

	sectionEntryInserted(*this, sharedNewEntry, entryIndex);

	setModified(true);

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
	m_entryConnections[entryIndex].disconnect();
	m_entryConnections.erase(m_entryConnections.begin() + entryIndex);

	sectionEntryRemoved(*this, sharedEntry, entryIndex);

	setModified(true);

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

	for(SignalConnectionGroup & connection : m_entryConnections) {
		connection.disconnect();
	}

	m_entryConnections.clear();

	sectionEntriesCleared(*this);

	setModified(true);
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

void DOSBoxConfiguration::Section::connectSignals() {
	m_commentCollectionConnections = SignalConnectionGroup(
		commentCollectionModified.connect(std::bind(&DOSBoxConfiguration::Section::onCommentCollectionModified, this, std::placeholders::_1)),
		commentAdded.connect(std::bind(&DOSBoxConfiguration::Section::onCommentAdded, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
		commentReplaced.connect(std::bind(&DOSBoxConfiguration::Section::onCommentReplaced, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)),
		commentInserted.connect(std::bind(&DOSBoxConfiguration::Section::onCommentInserted, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
		commentRemoved.connect(std::bind(&DOSBoxConfiguration::Section::onCommentRemoved, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
		commentsCleared.connect(std::bind(&DOSBoxConfiguration::Section::onCommentsCleared, this, std::placeholders::_1))
	);

	for(const std::string & entryName : m_orderedEntryNames) {
		EntryMap::const_iterator entryIterator(m_entries.find(entryName));

		if(entryIterator == m_entries.cend()) {
			continue;
		}

		m_entryConnections.emplace_back(connectEntrySignals(*entryIterator->second));
	}
}

SignalConnectionGroup DOSBoxConfiguration::Section::connectEntrySignals(Entry & entry) {
	return SignalConnectionGroup(
		entry.entryModified.connect(std::bind(&DOSBoxConfiguration::Section::onEntryModified, this, std::placeholders::_1)),
		entry.entryNameChanged.connect(std::bind(&DOSBoxConfiguration::Section::onEntryNameChanged, this, std::placeholders::_1, std::placeholders::_2)),
		entry.entryValueChanged.connect(std::bind(&DOSBoxConfiguration::Section::onEntryValueChanged, this, std::placeholders::_1, std::placeholders::_2))
	);
}

void DOSBoxConfiguration::Section::disconnectSignals() {
	m_commentCollectionConnections.disconnect();

	for(SignalConnectionGroup & connection : m_entryConnections) {
		connection.disconnect();
	}

	m_entryConnections.clear();
}

void DOSBoxConfiguration::Section::onCommentCollectionModified(CommentCollection & commentCollection) {
	if(this != &commentCollection) {
		return;
	}

	if(commentCollection.isCommentCollectionModified()) {
		setModified(true);
	}
}

void DOSBoxConfiguration::Section::onCommentAdded(CommentCollection & commentCollection, std::string newComment, size_t commentIndex) {
	if(this != &commentCollection) {
		return;
	}

	sectionCommentAdded(*this, newComment, commentIndex);
}

void DOSBoxConfiguration::Section::onCommentReplaced(CommentCollection & commentCollection, std::string newComment, size_t commentIndex, std::string oldComment) {
	if(this != &commentCollection) {
		return;
	}

	sectionCommentReplaced(*this, newComment, commentIndex, oldComment);
}

void DOSBoxConfiguration::Section::onCommentInserted(CommentCollection & commentCollection, std::string newComment, size_t commentIndex) {
	if(this != &commentCollection) {
		return;
	}

	sectionCommentInserted(*this, newComment, commentIndex);
}

void DOSBoxConfiguration::Section::onCommentRemoved(CommentCollection & commentCollection, std::string comment, size_t commentIndex) {
	if(this != &commentCollection) {
		return;
	}

	sectionCommentRemoved(*this, comment, commentIndex);
}

void DOSBoxConfiguration::Section::onCommentsCleared(CommentCollection & commentCollection) {
	if(this != &commentCollection) {
		return;
	}

	sectionCommentsCleared(*this);
}

void DOSBoxConfiguration::Section::onEntryModified(Entry & entry) {
	if(entry.isModified()) {
		setModified(true);
	}
}

void DOSBoxConfiguration::Section::onEntryNameChanged(Entry & entry, std::string oldEntryName) {
	size_t entryIndex = indexOfEntry(entry);

	if(entryIndex == std::numeric_limits<size_t>::max()) {
		return;
	}

	sectionEntryNameChanged(*this, m_entries[m_orderedEntryNames[entryIndex]], entryIndex, oldEntryName);
}

void DOSBoxConfiguration::Section::onEntryValueChanged(Entry & entry, std::string oldEntryValue) {
	size_t entryIndex = indexOfEntry(entry);

	if(entryIndex == std::numeric_limits<size_t>::max()) {
		return;
	}

	sectionEntryValueChanged(*this, m_entries[m_orderedEntryNames[entryIndex]], entryIndex, oldEntryValue);
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
