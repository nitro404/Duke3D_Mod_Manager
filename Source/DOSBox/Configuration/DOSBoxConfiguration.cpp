#include "DOSBoxConfiguration.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

#include <filesystem>

DOSBoxConfiguration::DOSBoxConfiguration(const std::string & filePath)
	: CommentCollection()
	, m_style(Style::None)
	, m_newlineType(Utilities::newLine[0] == '\r' ? NewlineType::Windows : NewlineType::Unix)
	, m_filePath(filePath)
	, m_modified(false) {
	connectSignals();
}

DOSBoxConfiguration::DOSBoxConfiguration(std::vector<std::unique_ptr<Section>> sections, const std::string & filePath)
	: CommentCollection()
	, m_style(Style::None)
	, m_newlineType(Utilities::newLine[0] == '\r' ? NewlineType::Windows : NewlineType::Unix)
	, m_filePath(filePath)
	, m_modified(false) {
	connectSignals();

	for(std::unique_ptr<Section> & section : sections) {
		addSection(std::move(section));
	}
}

DOSBoxConfiguration::DOSBoxConfiguration(const std::vector<Section> & sections, const std::string & filePath)
	: CommentCollection()
	, m_style(Style::None)
	, m_newlineType(Utilities::newLine[0] == '\r' ? NewlineType::Windows : NewlineType::Unix)
	, m_filePath(filePath)
	, m_modified(false) {
	connectSignals();

	for(const Section & section : sections) {
		addSection(section);
	}
}

DOSBoxConfiguration::DOSBoxConfiguration(DOSBoxConfiguration && configuration) noexcept
	: CommentCollection(std::move(configuration))
	, m_style(configuration.m_style)
	, m_newlineType(configuration.m_newlineType)
	, m_sections(std::move(configuration.m_sections))
	, m_orderedSectionNames(std::move(configuration.m_orderedSectionNames))
	, m_filePath(std::move(configuration.m_filePath))
	, m_modified(false) {
	updateParent();
	connectSignals();
}

DOSBoxConfiguration::DOSBoxConfiguration(const DOSBoxConfiguration & configuration)
	: CommentCollection(configuration)
	, m_style(configuration.m_style)
	, m_newlineType(configuration.m_newlineType)
	, m_orderedSectionNames(configuration.m_orderedSectionNames)
	, m_filePath(configuration.m_filePath)
	, m_modified(false) {
	for(const auto & section : configuration.m_sections) {
		m_sections[section.first] = std::make_shared<Section>(*section.second);
	}

	updateParent();
	connectSignals();
}

DOSBoxConfiguration & DOSBoxConfiguration::operator = (DOSBoxConfiguration && configuration) noexcept {
	if(this != &configuration) {
		disconnectSignals();

		CommentCollection::operator = (std::move(configuration));

		clearSections();

		m_style = configuration.m_style;
		m_newlineType = configuration.m_newlineType;
		m_sections = std::move(configuration.m_sections);
		m_orderedSectionNames = std::move(configuration.m_orderedSectionNames);
		m_filePath = std::move(configuration.m_filePath);

		updateParent();
		connectSignals();

		for(size_t i = 0; i < m_sections.size(); i++) {
			configurationSectionAdded(*this, m_sections[m_orderedSectionNames[i]], i);
		}

		setModified(true);
	}

	return *this;
}

DOSBoxConfiguration & DOSBoxConfiguration::operator = (const DOSBoxConfiguration & configuration) {
	disconnectSignals();

	CommentCollection::operator = (configuration);

	clearSections();

	m_style = configuration.m_style;
	m_newlineType = configuration.m_newlineType;
	m_orderedSectionNames = configuration.m_orderedSectionNames;
	m_filePath = configuration.m_filePath;

	for(const auto & section : configuration.m_sections) {
		m_sections[section.first] = std::make_shared<Section>(*section.second);
	}

	updateParent();
	connectSignals();

	for(size_t i = 0; i < m_sections.size(); i++) {
		configurationSectionAdded(*this, m_sections[m_orderedSectionNames[i]], i);
	}

	setModified(true);

	return *this;
}

DOSBoxConfiguration::~DOSBoxConfiguration() {
	disconnectSignals();

	for(const auto & section : m_sections) {
		section.second->m_parent = nullptr;
	}
}

bool DOSBoxConfiguration::hasFilePath() const {
	return !m_filePath.empty();
}

const std::string & DOSBoxConfiguration::getFilePath() const {
	return m_filePath;
}

std::string_view DOSBoxConfiguration::getFileName() const {
	return Utilities::getFileName(m_filePath);
}

std::string_view DOSBoxConfiguration::getFileExtension() const {
	return Utilities::getFileExtension(m_filePath);
}

void DOSBoxConfiguration::setFilePath(const std::string & filePath) {
	if(Utilities::areStringsEqual(m_filePath, filePath)) {
		return;
	}

	m_filePath = filePath;
}

void DOSBoxConfiguration::clearFilePath() {
	setFilePath(Utilities::emptyString);
}

bool DOSBoxConfiguration::hasWhitespaceAfterEntryNames() const {
	return Any(m_style & Style::WhitespaceAfterEntryNames);
}

bool DOSBoxConfiguration::hasNewlineAfterSectionComments() const {
	return Any(m_style & Style::NewlineAfterSectionComments);
}

DOSBoxConfiguration::Style DOSBoxConfiguration::getStyle() const {
	return m_style;
}

bool DOSBoxConfiguration::hasStyle(Style style) const {
	return (m_style & style) == style;
}

void DOSBoxConfiguration::setStyle(Style style) {
	if(m_style == style) {
		return;
	}

	Style oldStyle = m_style;

	m_style = style;

	configurationStyleChanged(*this, m_style, oldStyle);

	setModified(true);
}

void DOSBoxConfiguration::addStyle(Style style) {
	if(hasStyle(style)) {
		return;
	}

	setStyle(m_style | style);
}

void DOSBoxConfiguration::removeStyle(Style style) {
	if(!hasStyle(style)) {
		return;
	}

	setStyle(m_style & ~style);
}

size_t DOSBoxConfiguration::numberOfSections() const {
	return m_sections.size();
}

size_t DOSBoxConfiguration::totalNumberOfEntries() const {
	size_t totalEntryCount = 0;

	for(const auto & section : m_sections) {
		totalEntryCount += section.second->numberOfEntries();
	}

	return totalEntryCount;
}

size_t DOSBoxConfiguration::totalNumberOfComments() const {
	size_t totalCommentCount = m_comments.size();

	for(const auto & section : m_sections) {
		totalCommentCount += section.second->numberOfComments();
	}

	return totalCommentCount;
}

DOSBoxConfiguration::NewlineType DOSBoxConfiguration::getNewlineType() const {
	return m_newlineType;
}

void DOSBoxConfiguration::setNewlineType(NewlineType newlineType) {
	if(m_newlineType == newlineType) {
		return;
	}

	NewlineType oldNewlineType = m_newlineType;

	m_newlineType = newlineType;

	configurationNewlineTypeChanged(*this, m_newlineType, oldNewlineType);

	setModified(true);
}

bool DOSBoxConfiguration::isEmpty() const {
	return m_sections.empty() &&
		   m_comments.empty();
}

bool DOSBoxConfiguration::isNotEmpty() const {
	return !m_sections.empty() ||
		   !m_comments.empty();
}

bool DOSBoxConfiguration::mergeWith(const DOSBoxConfiguration & configuration) {
	if(!isValid(true) || !configuration.isValid(true)) {
		return false;
	}

	CommentCollection::mergeWith(configuration);

	for(const auto & currentSection : configuration.m_sections) {
		std::shared_ptr<Section> existingSection(getSectionWithName(currentSection.second->getName()));

		if(existingSection != nullptr) {
			if(!existingSection->mergeWith(*currentSection.second)) {
				return false;
			}
		}
		else {
			if(!addSection(*currentSection.second)) {
				return false;
			}
		}
	}

	setModified(true);

	return true;
}

bool DOSBoxConfiguration::setConfiguration(const DOSBoxConfiguration & configuration) {
	if(!configuration.isValid()) {
		return false;
	}

	setComments(configuration.m_comments);

	for(const auto & currentSection : m_sections) {
		size_t existingSectionIndex = configuration.indexOfSectionWithName(currentSection.second->getName());

		if(existingSectionIndex == std::numeric_limits<size_t>::max()) {
			if(!removeSection(existingSectionIndex)) {
				return false;
			}
		}
	}

	for(const auto & currentSection : configuration.m_sections) {
		std::shared_ptr<Section> existingSection(getSectionWithName(currentSection.second->getName()));

		if(existingSection != nullptr) {
			if(!existingSection->setSection(*currentSection.second)) {
				return false;
			}
		}
		else {
			if(!addSection(*currentSection.second)) {
				return false;
			}
		}
	}

	m_orderedSectionNames = configuration.m_orderedSectionNames;

	setModified(true);

	return true;
}

void DOSBoxConfiguration::clear() {
	clearComments();
	clearSections();
}

bool DOSBoxConfiguration::isModified() const {
	return m_modified;
}

void DOSBoxConfiguration::setModified(bool value) {
	m_modified = value;

	if(!m_modified) {
		m_commentCollectionModified = false;

		for(auto & section : m_sections) {
			section.second->setModified(false);
		}
	}

	configurationModified(*this);
}

bool DOSBoxConfiguration::hasSection(const Section & section) const {
	if(!section.isValid(false)) {
		return false;
	}

	SectionMap::const_iterator sectionIterator(m_sections.find(section.m_name));

	if(sectionIterator == m_sections.cend() || sectionIterator->second.get() != &section) {
		return false;
	}

	return true;
}

bool DOSBoxConfiguration::hasSectionWithName(const std::string & sectionName) const {
	return m_sections.find(sectionName) != m_sections.cend();
}

size_t DOSBoxConfiguration::indexOfSection(const Section & section) const {
	if(!section.isValid(true)) {
		return std::numeric_limits<size_t>::max();
	}

	std::vector<std::string>::const_iterator orderedSectionNameIterator(std::find_if(m_orderedSectionNames.cbegin(), m_orderedSectionNames.cend(), [&section](const std::string & sectionName) {
		return Utilities::areStringsEqualIgnoreCase(section.m_name, sectionName);
	}));

	if(orderedSectionNameIterator == m_orderedSectionNames.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	size_t sectionIndex = orderedSectionNameIterator - m_orderedSectionNames.cbegin();

	SectionMap::const_iterator sectionIterator(m_sections.find(m_orderedSectionNames[sectionIndex]));

	if(sectionIterator == m_sections.cend() || &section != sectionIterator->second.get()) {
		return std::numeric_limits<size_t>::max();
	}

	return sectionIndex;
}

size_t DOSBoxConfiguration::indexOfSectionWithName(const std::string & sectionName) const {
	if(!Section::isNameValid(sectionName)) {
		return std::numeric_limits<size_t>::max();
	}

	std::vector<std::string>::const_iterator orderedSectionNameIterator(std::find(m_orderedSectionNames.cbegin(), m_orderedSectionNames.cend(), sectionName));

	if(orderedSectionNameIterator == m_orderedSectionNames.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return orderedSectionNameIterator - m_orderedSectionNames.cbegin();
}

std::shared_ptr<DOSBoxConfiguration::Section> DOSBoxConfiguration::getSection(size_t sectionIndex) const {
	if(sectionIndex >= m_orderedSectionNames.size()) {
		return nullptr;
	}

	return getSectionWithName(m_orderedSectionNames[sectionIndex]);
}

std::shared_ptr<DOSBoxConfiguration::Section> DOSBoxConfiguration::getSection(const Section & section) const {
	if(!section.isValid(true)) {
		return nullptr;
	}

	std::shared_ptr<Section> sharedSection(getSectionWithName(section.getName()));

	if(sharedSection == nullptr || &section != sharedSection.get()) {
		return nullptr;
	}

	return sharedSection;
}

const DOSBoxConfiguration::SectionMap & DOSBoxConfiguration::getUnorderedSections() const {
	return m_sections;
}

std::vector<std::shared_ptr<DOSBoxConfiguration::Section>> DOSBoxConfiguration::getOrderedSections() const {
	if(!isValid(true)) {
		return {};
	}

	std::vector<std::shared_ptr<Section>> orderedSections;

	for(const std::string & sectionName : m_orderedSectionNames) {
		SectionMap::const_iterator sectionIterator(m_sections.find(sectionName));

		if(sectionIterator == m_sections.cend()) {
			continue;
		}

		orderedSections.push_back(sectionIterator->second);
	}

	return orderedSections;
}

const std::vector<std::string> & DOSBoxConfiguration::getOrderedSectionNames() const {
	return m_orderedSectionNames;
}

std::shared_ptr<DOSBoxConfiguration::Section> DOSBoxConfiguration::getSectionWithName(const std::string & sectionName) const {
	if(!Section::isNameValid(sectionName)) {
		return nullptr;
	}

	SectionMap::const_iterator sectionIterator(m_sections.find(sectionName));

	if(sectionIterator == m_sections.cend()) {
		return nullptr;
	}

	return sectionIterator->second;
}

bool DOSBoxConfiguration::setSectionName(size_t sectionIndex, const std::string & newSectionName) {
	if(sectionIndex >= m_orderedSectionNames.size() || !Section::isNameValid(newSectionName)) {
		return false;
	}

	SectionMap::const_iterator sectionIterator(m_sections.find(m_orderedSectionNames[sectionIndex]));

	if(sectionIterator == m_sections.cend()) {
		return false;
	}

	std::shared_ptr<Section> sharedSection(m_sections[m_orderedSectionNames[sectionIndex]]);

	if(Utilities::areStringsEqual(sharedSection->getName(), newSectionName)) {
		return true;
	}

	if(hasSectionWithName(newSectionName) || m_sections.erase(sharedSection->m_name) == 0) {
		return false;
	}

	std::string oldSectionName(sharedSection->m_name);

	sharedSection->m_name = newSectionName;
	m_orderedSectionNames[sectionIndex] = newSectionName;
	m_sections.emplace(newSectionName, sharedSection);

	sharedSection->sectionNameChanged(*sharedSection, oldSectionName);

	sharedSection->setModified(true);

	return true;
}

bool DOSBoxConfiguration::setSectionName(const std::string & oldSectionName, const std::string & newSectionName) {
	return setSectionName(indexOfSectionWithName(oldSectionName), newSectionName);
}

bool DOSBoxConfiguration::setSectionName(Section & section, const std::string & newSectionName) {
	return setSectionName(indexOfSection(section), newSectionName);
}

bool DOSBoxConfiguration::addSection(std::unique_ptr<Section> newSection) {
	if(!Section::isValid(newSection.get(), false) || hasSectionWithName(newSection->m_name)) {
		return false;
	}

	newSection->m_parent = this;
	m_orderedSectionNames.push_back(newSection->getName());
	const std::string & sectionName = newSection->m_name;
	m_sectionConnections.emplace_back(connectSectionSignals(*newSection));
	m_sections.emplace(sectionName, std::move(newSection));

	configurationSectionAdded(*this, m_sections[m_orderedSectionNames.back()], m_sections.size() - 1);

	setModified(true);

	return true;
}

bool DOSBoxConfiguration::addSection(const Section & newSection) {
	if(!newSection.isValid(false) || hasSectionWithName(newSection.m_name)) {
		return false;
	}

	return addSection(std::make_unique<Section>(newSection));
}

bool DOSBoxConfiguration::addSection(const std::string & newSectionName) {
	if(!Section::isNameValid(newSectionName) || hasSectionWithName(newSectionName)) {
		return false;
	}

	return addSection(std::make_unique<Section>(newSectionName));
}

bool DOSBoxConfiguration::addEntryToSection(std::unique_ptr<Section::Entry> newEntry, size_t sectionIndex) {
	std::shared_ptr<Section> section(getSection(sectionIndex));

	if(section == nullptr) {
		return false;
	}

	return section->addEntry(std::move(newEntry));
}

bool DOSBoxConfiguration::addEntryToSection(const Section::Entry & newEntry, size_t sectionIndex) {
	std::shared_ptr<Section> section(getSection(sectionIndex));

	if(section == nullptr) {
		return false;
	}

	return section->addEntry(newEntry);
}

bool DOSBoxConfiguration::addEntryToSection(const std::string & newEntryName, const std::string & newEntryValue, size_t sectionIndex) {
	std::shared_ptr<Section> section(getSection(sectionIndex));

	if(section == nullptr) {
		return false;
	}

	return section->addEntry(newEntryName, newEntryValue);
}

bool DOSBoxConfiguration::addEntryToSectionWithName(std::unique_ptr<Section::Entry> newEntry, const std::string & sectionName) {
	std::shared_ptr<Section> section(getSectionWithName(sectionName));

	if(section == nullptr) {
		return false;
	}

	return section->addEntry(std::move(newEntry));
}

bool DOSBoxConfiguration::addEntryToSectionWithName(const Section::Entry & newEntry, const std::string & sectionName) {
	std::shared_ptr<Section> section(getSectionWithName(sectionName));

	if(section == nullptr) {
		return false;
	}

	return section->addEntry(newEntry);
}

bool DOSBoxConfiguration::addEntryToSectionWithName(const std::string & newEntryName, const std::string & newEntryValue, const std::string & sectionName) {
	std::shared_ptr<Section> section(getSectionWithName(sectionName));

	if(section == nullptr) {
		return false;
	}

	return section->addEntry(newEntryName, newEntryValue);
}

bool DOSBoxConfiguration::replaceSection(size_t sectionIndex, std::unique_ptr<Section> newSection) {
	if(!Section::isValid(newSection.get(), false) || sectionIndex >= m_sections.size()) {
		return false;
	}

	size_t existingSectionIndex = indexOfSectionWithName(newSection->m_name);

	if(existingSectionIndex != sectionIndex && existingSectionIndex != std::numeric_limits<size_t>::max()) {
		return false;
	}

	std::shared_ptr<Section> oldSection(m_sections[newSection->m_name]);

	std::shared_ptr<Section> sharedNewSection(std::move(newSection));
	m_orderedSectionNames[sectionIndex] = sharedNewSection->m_name;
	sharedNewSection->m_parent = this;
	m_sectionConnections[sectionIndex] = connectSectionSignals(*sharedNewSection);
	m_sections[sharedNewSection->m_name] = std::move(sharedNewSection);

	configurationSectionReplaced(*this, sharedNewSection, sectionIndex, oldSection);

	setModified(true);

	return true;
}

bool DOSBoxConfiguration::replaceSection(const Section & oldSection, std::unique_ptr<Section> newSection) {
	return replaceSection(indexOfSection(oldSection), std::move(newSection));
}

bool DOSBoxConfiguration::replaceSection(const std::string & oldSectionName, std::unique_ptr<Section> newSection) {
	return replaceSection(indexOfSectionWithName(oldSectionName), std::move(newSection));
}

bool DOSBoxConfiguration::replaceSection(size_t sectionIndex, const Section & newSection) {
	if(!newSection.isValid(false) || sectionIndex >= m_sections.size()) {
		return false;
	}

	return replaceSection(sectionIndex, std::make_unique<Section>(newSection));
}

bool DOSBoxConfiguration::replaceSection(const Section & oldSection, const Section & newSection) {
	if(!newSection.isValid(false)) {
		return false;
	}

	return replaceSection(oldSection, std::make_unique<Section>(newSection));
}

bool DOSBoxConfiguration::replaceSection(const std::string & oldSectionName, const Section & newSection) {
	if(!newSection.isValid(false)) {
		return false;
	}

	return replaceSection(oldSectionName, std::make_unique<Section>(newSection));
}

bool DOSBoxConfiguration::replaceEntryInSection(size_t entryIndex, std::unique_ptr<Section::Entry> newEntry, size_t sectionIndex) {
	std::shared_ptr<Section> section(getSection(sectionIndex));

	if(section == nullptr) {
		return false;
	}

	return section->replaceEntry(entryIndex, std::move(newEntry));
}

bool DOSBoxConfiguration::replaceEntryInSection(const Section::Entry & oldEntry, std::unique_ptr<Section::Entry> newEntry, size_t sectionIndex) {
	std::shared_ptr<Section> section(getSection(sectionIndex));

	if(section == nullptr) {
		return false;
	}

	return section->replaceEntry(oldEntry, std::move(newEntry));
}

bool DOSBoxConfiguration::replaceEntryInSection(const std::string & oldEntryName, std::unique_ptr<Section::Entry> newEntry, size_t sectionIndex) {
	std::shared_ptr<Section> section(getSection(sectionIndex));

	if(section == nullptr) {
		return false;
	}

	return section->replaceEntryWithName(oldEntryName, std::move(newEntry));
}

bool DOSBoxConfiguration::replaceEntryInSection(size_t entryIndex, const Section::Entry & newEntry, size_t sectionIndex) {
	std::shared_ptr<Section> section(getSection(sectionIndex));

	if(section == nullptr) {
		return false;
	}

	return section->replaceEntry(entryIndex, newEntry);
}

bool DOSBoxConfiguration::replaceEntryInSection(const Section::Entry & oldEntry, const Section::Entry & newEntry, size_t sectionIndex) {
	std::shared_ptr<Section> section(getSection(sectionIndex));

	if(section == nullptr) {
		return false;
	}

	return section->replaceEntry(oldEntry, newEntry);
}

bool DOSBoxConfiguration::replaceEntryInSection(const std::string & oldEntryName, const Section::Entry & newEntry, size_t sectionIndex) {
	std::shared_ptr<Section> section(getSection(sectionIndex));

	if(section == nullptr) {
		return false;
	}

	return section->replaceEntryWithName(oldEntryName, newEntry);
}

bool DOSBoxConfiguration::replaceEntryInSectionWithName(size_t entryIndex, std::unique_ptr<Section::Entry> newEntry, const std::string & sectionName) {
	std::shared_ptr<Section> section(getSectionWithName(sectionName));

	if(section == nullptr) {
		return false;
	}

	return section->replaceEntry(entryIndex, std::move(newEntry));
}

bool DOSBoxConfiguration::replaceEntryInSectionWithName(const Section::Entry & oldEntry, std::unique_ptr<Section::Entry> newEntry, const std::string & sectionName) {
	std::shared_ptr<Section> section(getSectionWithName(sectionName));

	if(section == nullptr) {
		return false;
	}

	return section->replaceEntry(oldEntry, std::move(newEntry));
}

bool DOSBoxConfiguration::replaceEntryInSectionWithName(const std::string & oldEntryName, std::unique_ptr<Section::Entry> newEntry, const std::string & sectionName) {
	std::shared_ptr<Section> section(getSectionWithName(sectionName));

	if(section == nullptr) {
		return false;
	}

	return section->replaceEntryWithName(oldEntryName, std::move(newEntry));
}

bool DOSBoxConfiguration::replaceEntryInSectionWithName(size_t entryIndex, const Section::Entry & newEntry, const std::string & sectionName) {
	std::shared_ptr<Section> section(getSectionWithName(sectionName));

	if(section == nullptr) {
		return false;
	}

	return section->replaceEntry(entryIndex, newEntry);
}

bool DOSBoxConfiguration::replaceEntryInSectionWithName(const Section::Entry & oldEntry, const Section::Entry & newEntry, const std::string & sectionName) {
	std::shared_ptr<Section> section(getSectionWithName(sectionName));

	if(section == nullptr) {
		return false;
	}

	return section->replaceEntry(oldEntry, newEntry);
}

bool DOSBoxConfiguration::replaceEntryInSectionWithName(const std::string & oldEntryName, const Section::Entry & newEntry, const std::string & sectionName) {
	std::shared_ptr<Section> section(getSectionWithName(sectionName));

	if(section == nullptr) {
		return false;
	}

	return section->replaceEntryWithName(oldEntryName, newEntry);
}

bool DOSBoxConfiguration::insertSection(size_t sectionIndex, std::unique_ptr<Section> newSection) {
	if(!Section::isValid(newSection.get(), false) || sectionIndex >= m_sections.size()) {
		return false;
	}

	size_t existingSectionIndex = indexOfSectionWithName(newSection->m_name);

	if(existingSectionIndex != sectionIndex && existingSectionIndex != std::numeric_limits<size_t>::max()) {
		return false;
	}

	std::shared_ptr<Section> sharedNewSection(std::move(newSection));
	m_orderedSectionNames.insert(m_orderedSectionNames.begin() + sectionIndex, sharedNewSection->m_name);
	sharedNewSection->m_parent = this;
	m_sectionConnections.insert(m_sectionConnections.begin() + sectionIndex, connectSectionSignals(*sharedNewSection));
	m_sections[sharedNewSection->m_name] = std::move(sharedNewSection);

	configurationSectionInserted(*this, sharedNewSection, sectionIndex);

	setModified(true);

	return true;
}

bool DOSBoxConfiguration::insertEntryInSection(size_t entryIndex, std::unique_ptr<Section::Entry> newEntry, size_t sectionIndex) {
	std::shared_ptr<Section> section(getSection(sectionIndex));

	if(section == nullptr) {
		return false;
	}

	return section->insertEntry(entryIndex, std::move(newEntry));
}

bool DOSBoxConfiguration::insertEntryInSection(size_t entryIndex, const Section::Entry & newEntry, size_t sectionIndex) {
	std::shared_ptr<Section> section(getSection(sectionIndex));

	if(section == nullptr) {
		return false;
	}

	return section->insertEntry(entryIndex, newEntry);
}

bool DOSBoxConfiguration::insertEntryInSectionWithName(size_t entryIndex, std::unique_ptr<Section::Entry> newEntry, const std::string & sectionName) {
	std::shared_ptr<Section> section(getSectionWithName(sectionName));

	if(section == nullptr) {
		return false;
	}

	return section->insertEntry(entryIndex, std::move(newEntry));
}

bool DOSBoxConfiguration::insertEntryInSectionWithName(size_t entryIndex, const Section::Entry & newEntry, const std::string & sectionName) {
	std::shared_ptr<Section> section(getSectionWithName(sectionName));

	if(section == nullptr) {
		return false;
	}

	return section->insertEntry(entryIndex, newEntry);
}

bool DOSBoxConfiguration::removeSection(size_t sectionIndex) {
	if(sectionIndex >= m_orderedSectionNames.size()) {
		return false;
	}

	return removeSectionWithName(m_orderedSectionNames[sectionIndex]);
}

bool DOSBoxConfiguration::removeSection(const Section & section) {
	size_t sectionIndex = indexOfSection(section);

	if(sectionIndex == std::numeric_limits<size_t>::max()) {
		return false;
	}

	std::shared_ptr<Section> sharedSection(m_sections[m_orderedSectionNames[sectionIndex]]);

	sharedSection->m_parent = nullptr;
	m_orderedSectionNames.erase(m_orderedSectionNames.begin() + sectionIndex);
	m_sectionConnections[sectionIndex].disconnect();
	m_sectionConnections.erase(m_sectionConnections.begin() + sectionIndex);

	if(m_sections.erase(sharedSection->getName()) == 0) {
		return false;
	}

	configurationSectionRemoved(*this, sharedSection, sectionIndex);

	setModified(true);

	return true;
}

bool DOSBoxConfiguration::removeSectionWithName(const std::string & sectionName) {
	std::shared_ptr<Section> sharedSection(getSectionWithName(sectionName));

	if(sharedSection == nullptr) {
		return false;
	}

	return removeSection(*sharedSection);
}

bool DOSBoxConfiguration::removeEntryFromSection(size_t entryIndex, size_t sectionIndex) {
	std::shared_ptr<Section> section(getSection(sectionIndex));

	if(section == nullptr) {
		return false;
	}

	return section->removeEntry(entryIndex);
}

bool DOSBoxConfiguration::removeEntryFromSection(const Section::Entry & entry, size_t sectionIndex) {
	std::shared_ptr<Section> section(getSection(sectionIndex));

	if(section == nullptr) {
		return false;
	}

	return section->removeEntry(entry);
}

bool DOSBoxConfiguration::removeEntryFromSectionWithName(size_t entryIndex, const std::string & sectionName) {
	std::shared_ptr<Section> section(getSectionWithName(sectionName));

	if(section == nullptr) {
		return false;
	}

	return section->removeEntry(entryIndex);
}

bool DOSBoxConfiguration::removeEntryFromSectionWithName(const Section::Entry & entry, const std::string & sectionName) {
	std::shared_ptr<Section> section(getSectionWithName(sectionName));

	if(section == nullptr) {
		return false;
	}

	return section->removeEntry(entry);
}

void DOSBoxConfiguration::clearSections() {
	if(m_sections.empty()) {
		return;
	}

	for(SectionMap::const_iterator i = m_sections.cbegin(); i != m_sections.cend(); ++i) {
		i->second->m_parent = nullptr;
	}

	m_sections.clear();
	m_orderedSectionNames.clear();

	for(SignalConnectionGroup & connection : m_sectionConnections) {
		connection.disconnect();
	}

	m_sectionConnections.clear();

	configurationSectionsCleared(*this);

	setModified(true);
}

std::unique_ptr<DOSBoxConfiguration> DOSBoxConfiguration::readFrom(const ByteBuffer & data) {
	bool error = false;
	Style sectionStyle = Style::None;
	size_t previousReadOffset = 0;
	size_t newlineOffset = 0;
	uint8_t currentByte = 0;
	std::string line;
	std::unique_ptr<DOSBoxConfiguration> configuration(std::make_unique<DOSBoxConfiguration>());

	while(newlineOffset < data.getSize()) {
		currentByte = data.getUnsignedByte(newlineOffset++, &error);

		if(error) {
			break;
		}

		if(currentByte == '\r') {
			spdlog::trace("Detected Windows style newlines in DOSBox configuration data.");

			configuration->setNewlineType(NewlineType::Windows);

			break;
		}
		else if(currentByte == '\n') {
			spdlog::trace("Detected Unix style newlines in DOSBox configuration data.");

			configuration->setNewlineType(NewlineType::Unix);

			break;
		}
	}

	while(data.hasMoreLines()) {
		previousReadOffset = data.getReadOffset();
		line = data.readLine(&error);

		if(error) {
			return nullptr;
		}

		if(line.empty()) {
			continue;
		}

		if(line[0] == COMMENT_CHARACTER) {
			configuration->addComment(std::string_view(line).substr(line.length() >= 2 && (line[1] == ' ' || line[1] == '\t') ? 2 : 1));
		}
		else {
			data.setReadOffset(previousReadOffset);

			if(!configuration->addSection(Section::readFrom(data, &sectionStyle))) {
				spdlog::error("Failed to parse section #{} when parsing DOSBox configuration data.", configuration->numberOfSections());
				return nullptr;
			}

			if(Any(sectionStyle & Style::WhitespaceAfterEntryNames)) {
				configuration->addStyle(Style::WhitespaceAfterEntryNames);
			}

			if(Any(sectionStyle & Style::NewlineAfterSectionComments)) {
				configuration->addStyle(Style::NewlineAfterSectionComments);
			}
		}
	}

	configuration->setModified(false);

	if(!configuration->isValid(true)) {
		spdlog::error("Parsed DOSBox configuration is not valid.");
		return nullptr;
	}

	return configuration;
}

bool DOSBoxConfiguration::writeTo(ByteBuffer & data, std::optional<Style> styleOverride, std::optional<NewlineType> newlineTypeOverride) const {
	if(!isValid(false)) {
		return false;
	}

	Style style = styleOverride.has_value() ? styleOverride.value() : m_style;

	NewlineType newlineType = newlineTypeOverride.has_value() ? newlineTypeOverride.value() : m_newlineType;

	std::string newline;

	switch(newlineType) {
		case NewlineType::Unix: {
			newline = "\n";
			break;
		}

		case NewlineType::Windows: {
			newline = "\r\n";
			break;
		}
	}

	if(!m_comments.empty()) {
		for(const std::string & comment : m_comments) {
			if(!data.writeLine(fmt::format("{}{}{}", COMMENT_CHARACTER, comment.empty() ? "" : " ", comment), newline)) {
				return false;
			}
		}

		if(!m_sections.empty() &&
		   !data.writeLine(Utilities::emptyString, newline)) {
			return false;
		}
	}

	for(std::vector<std::string>::const_iterator sectionNameIterator = m_orderedSectionNames.cbegin(); sectionNameIterator != m_orderedSectionNames.cend(); ++sectionNameIterator) {
		if(sectionNameIterator != m_orderedSectionNames.cbegin() &&
		   !data.writeLine(Utilities::emptyString, newline)) {
			return false;
		}

		SectionMap::const_iterator sectionIterator(m_sections.find(*sectionNameIterator));

		if(sectionIterator == m_sections.cend() ||
		   !sectionIterator->second->writeTo(data, style, newline)) {
			return false;
		}
	}

	return true;
}

std::unique_ptr<DOSBoxConfiguration> DOSBoxConfiguration::loadFrom(const std::string & filePath) {
	std::unique_ptr<ByteBuffer> configurationData(ByteBuffer::readFrom(filePath));

	if(configurationData == nullptr) {
		spdlog::error("Failed to read DOSBox configuration data from file: '{}'.", filePath);
		return nullptr;
	}

	std::unique_ptr<DOSBoxConfiguration> configuration(readFrom(*configurationData));

	if(configuration == nullptr) {
		spdlog::error("Failed to parse DOSBox configuration data from file: '{}'.", filePath);
		return nullptr;
	}

	configuration->setFilePath(filePath);

	return configuration;
}

bool DOSBoxConfiguration::save(bool overwrite, bool createParentDirectories) {
	if(m_filePath.empty()) {
		spdlog::error("DOSBox configuration has no file path.");
		return false;
	}

	return saveTo(m_filePath, overwrite);
}

bool DOSBoxConfiguration::saveTo(const std::string & filePath, bool overwrite, bool createParentDirectories) {
	if(!overwrite && std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		spdlog::warn("File '{}' already exists, use overwrite to force write.", filePath);
		return false;
	}

	ByteBuffer byteBuffer;

	if(!writeTo(byteBuffer) || !byteBuffer.writeTo(filePath, overwrite)) {
		return false;
	}

	setModified(false);

	return true;
}

void DOSBoxConfiguration::updateParent(bool recursive) {
	for(auto & section : m_sections) {
		section.second->m_parent = this;
		section.second->updateParent();
	}
}

bool DOSBoxConfiguration::isValid(bool validateParents) const {
	for(const auto & section : m_sections) {
		if(!Utilities::areStringsEqual(section.first, section.second->m_name) ||
		   !Section::isValid(section.second.get(), validateParents)) {
			return false;
		}
	}

	return true;
}

bool DOSBoxConfiguration::isValid(const DOSBoxConfiguration * configuration, bool validateParents) {
	return configuration != nullptr &&
		   configuration->isValid(validateParents);
}

void DOSBoxConfiguration::connectSignals() {
	m_commentCollectionConnections = SignalConnectionGroup(
		commentCollectionModified.connect(std::bind(&DOSBoxConfiguration::onCommentCollectionModified, this, std::placeholders::_1)),
		commentAdded.connect(std::bind(&DOSBoxConfiguration::onCommentAdded, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
		commentReplaced.connect(std::bind(&DOSBoxConfiguration::onCommentReplaced, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)),
		commentInserted.connect(std::bind(&DOSBoxConfiguration::onCommentInserted, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
		commentRemoved.connect(std::bind(&DOSBoxConfiguration::onCommentRemoved, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
		commentsCleared.connect(std::bind(&DOSBoxConfiguration::onCommentsCleared, this, std::placeholders::_1))
	);

	for(const std::string & sectionName : m_orderedSectionNames) {
		SectionMap::const_iterator sectionIterator(m_sections.find(sectionName));

		if(sectionIterator == m_sections.cend()) {
			continue;
		}

		m_sectionConnections.emplace_back(connectSectionSignals(*sectionIterator->second));
	}
}

SignalConnectionGroup DOSBoxConfiguration::connectSectionSignals(Section & section) {
	return SignalConnectionGroup(
		section.sectionModified.connect(std::bind(&DOSBoxConfiguration::onSectionModified, this, std::placeholders::_1)),
		section.sectionNameChanged.connect(std::bind(&DOSBoxConfiguration::onSectionNameChanged, this, std::placeholders::_1, std::placeholders::_2)),
		section.sectionCommentAdded.connect(std::bind(&DOSBoxConfiguration::onSectionCommentAdded, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
		section.sectionCommentReplaced.connect(std::bind(&DOSBoxConfiguration::onSectionCommentReplaced, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)),
		section.sectionCommentInserted.connect(std::bind(&DOSBoxConfiguration::onSectionCommentInserted, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
		section.sectionCommentRemoved.connect(std::bind(&DOSBoxConfiguration::onSectionCommentRemoved, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
		section.sectionCommentsCleared.connect(std::bind(&DOSBoxConfiguration::onSectionCommentsCleared, this, std::placeholders::_1)),
		section.sectionEntryNameChanged.connect(std::bind(&DOSBoxConfiguration::onSectionEntryNameChanged, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)),
		section.sectionEntryValueChanged.connect(std::bind(&DOSBoxConfiguration::onSectionEntryValueChanged, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)),
		section.sectionEntryAdded.connect(std::bind(&DOSBoxConfiguration::onSectionEntryAdded, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
		section.sectionEntryReplaced.connect(std::bind(&DOSBoxConfiguration::onSectionEntryReplaced, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)),
		section.sectionEntryInserted.connect(std::bind(&DOSBoxConfiguration::onSectionEntryInserted, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
		section.sectionEntryRemoved.connect(std::bind(&DOSBoxConfiguration::onSectionEntryRemoved, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
		section.sectionEntriesCleared.connect(std::bind(&DOSBoxConfiguration::onSectionEntriesCleared, this, std::placeholders::_1))
	);
}

void DOSBoxConfiguration::disconnectSignals() {
	m_commentCollectionConnections.disconnect();

	for(SignalConnectionGroup & connection : m_sectionConnections) {
		connection.disconnect();
	}

	m_sectionConnections.clear();
}

void DOSBoxConfiguration::onCommentCollectionModified(CommentCollection & commentCollection) {
	if(this != &commentCollection) {
		return;
	}

	if(commentCollection.isCommentCollectionModified()) {
		setModified(true);
	}
}

void DOSBoxConfiguration::onCommentAdded(CommentCollection & commentCollection, std::string newComment, size_t commentIndex) {
	if(this != &commentCollection) {
		return;
	}

	configurationCommentAdded(*this, newComment, commentIndex);
}

void DOSBoxConfiguration::onCommentReplaced(CommentCollection & commentCollection, std::string newComment, size_t commentIndex, std::string oldComment) {
	if(this != &commentCollection) {
		return;
	}

	configurationCommentReplaced(*this, newComment, commentIndex, oldComment);
}

void DOSBoxConfiguration::onCommentInserted(CommentCollection & commentCollection, std::string newComment, size_t commentIndex) {
	if(this != &commentCollection) {
		return;
	}

	configurationCommentInserted(*this, newComment, commentIndex);
}

void DOSBoxConfiguration::onCommentRemoved(CommentCollection & commentCollection, std::string comment, size_t commentIndex) {
	if(this != &commentCollection) {
		return;
	}

	configurationCommentRemoved(*this, comment, commentIndex);
}

void DOSBoxConfiguration::onCommentsCleared(CommentCollection & commentCollection) {
	if(this != &commentCollection) {
		return;
	}

	configurationCommentsCleared(*this);
}

void DOSBoxConfiguration::onSectionModified(Section & section) {
	if(section.isModified()) {
		setModified(true);
	}
}

void DOSBoxConfiguration::onSectionNameChanged(Section & section, std::string oldSectionName) {
	size_t sectionIndex = indexOfSection(section);

	if(sectionIndex == std::numeric_limits<size_t>::max()) {
		return;
	}

	configurationSectionNameChanged(*this, m_sections[m_orderedSectionNames[sectionIndex]], sectionIndex, oldSectionName);
}

void DOSBoxConfiguration::onSectionCommentAdded(Section & section, std::string newComment, size_t commentIndex) {
	size_t sectionIndex = indexOfSection(section);

	if(sectionIndex == std::numeric_limits<size_t>::max()) {
		return;
	}

	configurationSectionCommentAdded(*this, m_sections[m_orderedSectionNames[sectionIndex]], sectionIndex, newComment, commentIndex);
}

void DOSBoxConfiguration::onSectionCommentReplaced(Section & section, std::string newComment, size_t commentIndex, std::string oldComment) {
	size_t sectionIndex = indexOfSection(section);

	if(sectionIndex == std::numeric_limits<size_t>::max()) {
		return;
	}

	configurationSectionCommentReplaced(*this, m_sections[m_orderedSectionNames[sectionIndex]], sectionIndex, newComment, commentIndex, oldComment);
}

void DOSBoxConfiguration::onSectionCommentInserted(Section & section, std::string newComment, size_t commentIndex) {
	size_t sectionIndex = indexOfSection(section);

	if(sectionIndex == std::numeric_limits<size_t>::max()) {
		return;
	}

	configurationSectionCommentInserted(*this, m_sections[m_orderedSectionNames[sectionIndex]], sectionIndex, newComment, commentIndex);
}

void DOSBoxConfiguration::onSectionCommentRemoved(Section & section, std::string comment, size_t commentIndex) {
	size_t sectionIndex = indexOfSection(section);

	if(sectionIndex == std::numeric_limits<size_t>::max()) {
		return;
	}

	configurationSectionCommentRemoved(*this, m_sections[m_orderedSectionNames[sectionIndex]], sectionIndex, comment, commentIndex);
}

void DOSBoxConfiguration::onSectionCommentsCleared(Section & section) {
	size_t sectionIndex = indexOfSection(section);

	if(sectionIndex == std::numeric_limits<size_t>::max()) {
		return;
	}

	configurationSectionCommentsCleared(*this, m_sections[m_orderedSectionNames[sectionIndex]], sectionIndex);
}

void DOSBoxConfiguration::onSectionEntryNameChanged(Section & section, std::shared_ptr<Section::Entry> entry, size_t entryIndex, std::string oldEntryName) {
	size_t sectionIndex = indexOfSection(section);

	if(sectionIndex == std::numeric_limits<size_t>::max()) {
		return;
	}

	configurationSectionEntryNameChanged(*this, m_sections[m_orderedSectionNames[sectionIndex]], sectionIndex, entry, entryIndex, oldEntryName);
}

void DOSBoxConfiguration::onSectionEntryValueChanged(Section & section, std::shared_ptr<Section::Entry> entry, size_t entryIndex, std::string oldEntryValue) {
	size_t sectionIndex = indexOfSection(section);

	if(sectionIndex == std::numeric_limits<size_t>::max()) {
		return;
	}

	configurationSectionEntryValueChanged(*this, m_sections[m_orderedSectionNames[sectionIndex]], sectionIndex, entry, entryIndex, oldEntryValue);
}

void DOSBoxConfiguration::onSectionEntryAdded(Section & section, std::shared_ptr<Section::Entry> newEntry, size_t entryIndex) {
	size_t sectionIndex = indexOfSection(section);

	if(sectionIndex == std::numeric_limits<size_t>::max()) {
		return;
	}

	configurationSectionEntryAdded(*this, m_sections[m_orderedSectionNames[sectionIndex]], sectionIndex, newEntry, entryIndex);
}

void DOSBoxConfiguration::onSectionEntryReplaced(Section & section, std::shared_ptr<Section::Entry> newEntry, size_t entryIndex, std::shared_ptr<Section::Entry> oldEntry) {
	size_t sectionIndex = indexOfSection(section);

	if(sectionIndex == std::numeric_limits<size_t>::max()) {
		return;
	}

	configurationSectionEntryReplaced(*this, m_sections[m_orderedSectionNames[sectionIndex]], sectionIndex, newEntry, entryIndex, oldEntry);
}

void DOSBoxConfiguration::onSectionEntryInserted(Section & section, std::shared_ptr<Section::Entry> newEntry, size_t entryIndex) {
	size_t sectionIndex = indexOfSection(section);

	if(sectionIndex == std::numeric_limits<size_t>::max()) {
		return;
	}

	configurationSectionEntryInserted(*this, m_sections[m_orderedSectionNames[sectionIndex]], sectionIndex, newEntry, entryIndex);
}

void DOSBoxConfiguration::onSectionEntryRemoved(Section & section, std::shared_ptr<Section::Entry> entry, size_t entryIndex) {
	size_t sectionIndex = indexOfSection(section);

	if(sectionIndex == std::numeric_limits<size_t>::max()) {
		return;
	}

	configurationSectionEntryRemoved(*this, m_sections[m_orderedSectionNames[sectionIndex]], sectionIndex, entry, entryIndex);
}

void DOSBoxConfiguration::onSectionEntriesCleared(Section & section) {
	size_t sectionIndex = indexOfSection(section);

	if(sectionIndex == std::numeric_limits<size_t>::max()) {
		return;
	}

	configurationSectionEntriesCleared(*this, m_sections[m_orderedSectionNames[sectionIndex]], sectionIndex);
}

bool DOSBoxConfiguration::operator == (const DOSBoxConfiguration & configuration) const {
	if(CommentCollection::operator != (configuration) ||
	   m_sections.size() != configuration.m_sections.size()) {
		return false;
	}

	for(const std::string & sectionName : m_orderedSectionNames) {
		SectionMap::const_iterator sectionIteratorA(m_sections.find(sectionName));

		if(sectionIteratorA == m_sections.cend()) {
			return false;
		}

		SectionMap::const_iterator sectionIteratorB(configuration.m_sections.find(sectionName));

		if(sectionIteratorB == configuration.m_sections.cend()) {
			return false;
		}

		if(*sectionIteratorA->second != *sectionIteratorB->second) {
			return false;
		}
	}

	return true;
}

bool DOSBoxConfiguration::operator != (const DOSBoxConfiguration & configuration) const {
	return !operator == (configuration);
}

bool DOSBoxConfiguration::NameComparator::operator () (const std::string & nameA, const std::string & nameB) const {
	return std::lexicographical_compare(nameA.begin(), nameA.end(), nameB.begin(), nameB.end(), [](unsigned char a, unsigned char b) {
		return std::tolower(a) < std::tolower(b);
	});
}
