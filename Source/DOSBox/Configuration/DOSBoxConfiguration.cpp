#include "DOSBoxConfiguration.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>

#include <spdlog/spdlog.h>

#include <filesystem>

DOSBoxConfiguration::DOSBoxConfiguration(const std::string & filePath)
	: CommentCollection()
	, m_style(Style::None)
	, m_newlineType(Utilities::newLine[0] == '\r' ? NewlineType::Windows : NewlineType::Unix)
	, m_filePath(filePath) { }

DOSBoxConfiguration::DOSBoxConfiguration(DOSBoxConfiguration && configuration) noexcept
	: CommentCollection(std::move(configuration))
	, m_style(configuration.m_style)
	, m_newlineType(configuration.m_newlineType)
	, m_sections(std::move(configuration.m_sections))
	, m_orderedSectionNames(std::move(configuration.m_orderedSectionNames))
	, m_filePath(std::move(configuration.m_filePath)) {
	updateParent();
}

DOSBoxConfiguration::DOSBoxConfiguration(const DOSBoxConfiguration & configuration)
	: CommentCollection(configuration)
	, m_style(configuration.m_style)
	, m_newlineType(configuration.m_newlineType)
	, m_orderedSectionNames(configuration.m_orderedSectionNames)
	, m_filePath(configuration.m_filePath) {
	clearSections();

	for(const auto & section : configuration.m_sections) {
		m_sections[section.first] = std::make_shared<Section>(*section.second);
	}

	updateParent();
}

DOSBoxConfiguration & DOSBoxConfiguration::operator = (DOSBoxConfiguration && configuration) noexcept {
	if(this != &configuration) {
		CommentCollection::operator = (std::move(configuration));

		m_style = configuration.m_style;
		m_newlineType = configuration.m_newlineType;
		m_sections = std::move(configuration.m_sections);
		m_orderedSectionNames = std::move(configuration.m_orderedSectionNames);
		m_filePath = std::move(configuration.m_filePath);

		updateParent();
	}

	return *this;
}

DOSBoxConfiguration & DOSBoxConfiguration::operator = (const DOSBoxConfiguration & configuration) {
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

	return *this;
}

DOSBoxConfiguration::~DOSBoxConfiguration() {
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
	m_filePath = filePath;
}

void DOSBoxConfiguration::clearFilePath() {
	setFilePath(Utilities::emptyString);
}

bool DOSBoxConfiguration::hasWhitespaceAfterEntryNames() {
	return Any(m_style & Style::WhitespaceAfterEntryNames);
}

bool DOSBoxConfiguration::hasNewlineAfterSectionComments() {
	return Any(m_style & Style::NewlineAfterSectionComments);
}

DOSBoxConfiguration::Style DOSBoxConfiguration::getStyle() const {
	return m_style;
}

bool DOSBoxConfiguration::hasStyle(Style style) const {
	return (m_style & style) == style;
}

void DOSBoxConfiguration::setStyle(Style style) {
	m_style = style;
}

void DOSBoxConfiguration::addStyle(Style style) {
	m_style |= style;
}

void DOSBoxConfiguration::removeStyle(Style style) {
	m_style &= ~style;
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
	m_newlineType = newlineType;
}

bool DOSBoxConfiguration::isEmpty() const {
	return m_sections.empty();
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
		std::shared_ptr<Section> existingSection(getSectionWithName(currentSection.second->m_name));

		if(existingSection != nullptr) {
			if(!existingSection->mergeWith(*currentSection.second)) {
				return false;
			}
		}
		else {
			if(!addSection(std::make_unique<Section>(*currentSection.second))) {
				return false;
			}
		}
	}

	return true;
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
	if(sectionIndex >= m_orderedSectionNames.size() || !Section::isNameValid(newSectionName) || hasSectionWithName(newSectionName)) {
		return false;
	}

	SectionMap::const_iterator sectionIterator(m_sections.find(m_orderedSectionNames[sectionIndex]));

	if(sectionIterator == m_sections.cend()) {
		return false;
	}

	std::shared_ptr<Section> sharedSection(m_sections[m_orderedSectionNames[sectionIndex]]);

	if(m_sections.erase(sharedSection->m_name) == 0) {
		return false;
	}

	sharedSection->m_name = newSectionName;
	m_orderedSectionNames[sectionIndex] = newSectionName;
	m_sections.emplace(newSectionName, sharedSection);

	return true;
}

bool DOSBoxConfiguration::setSectionName(const std::string & oldSectionName, const std::string & newSectionName) {
	return setSectionName(indexOfSectionWithName(oldSectionName), newSectionName);
}

bool DOSBoxConfiguration::setSectionName(Section & section, const std::string & newSectionName) {
	return setSectionName(indexOfSection(section), newSectionName);
}

bool DOSBoxConfiguration::addSection(std::unique_ptr<Section> section) {
	if(!Section::isValid(section.get(), false) || hasSectionWithName(section->m_name)) {
		return false;
	}

	section->m_parent = this;
	m_orderedSectionNames.push_back(section->getName());
	const std::string & sectionName = section->m_name;
	m_sections.emplace(sectionName, std::move(section));

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

	std::shared_ptr<Section> sharedNewSection(std::move(newSection));
	m_orderedSectionNames[sectionIndex] = sharedNewSection->m_name;
	sharedNewSection->m_parent = this;
	m_sections[sharedNewSection->m_name] = sharedNewSection;

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
	m_sections[sharedNewSection->m_name] = sharedNewSection;

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

	return m_sections.erase(sharedSection->getName()) != 0;
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
	for(SectionMap::const_iterator i = m_sections.cbegin(); i != m_sections.cend(); ++i) {
		i->second->m_parent = nullptr;
	}

	m_sections.clear();
	m_orderedSectionNames.clear();
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

			configuration->setNewlineType(NewlineType::Windows);\

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

bool DOSBoxConfiguration::save(bool overwrite, bool createParentDirectories) const {
	if(m_filePath.empty()) {
		spdlog::error("DOSBox configuration has no file path.");
		return false;
	}

	return saveTo(m_filePath, overwrite);
}

bool DOSBoxConfiguration::saveTo(const std::string & filePath, bool overwrite, bool createParentDirectories) const {
	if(!overwrite && std::filesystem::is_regular_file(std::filesystem::path(filePath))) {
		spdlog::warn("File '{}' already exists, use overwrite to force write.", filePath);
		return false;
	}

	ByteBuffer byteBuffer;

	if(!writeTo(byteBuffer) || !byteBuffer.writeTo(filePath, overwrite)) {
		return false;
	}

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
