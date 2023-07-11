#include "DOSBoxConfiguration.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <sstream>

DOSBoxConfiguration::Section::Entry::Entry(std::string_view name, std::string_view value, Section * parent)
	: m_name(name)
	, m_value(value)
	, m_parent(parent) { }

DOSBoxConfiguration::Section::Entry::Entry(Entry && entry) noexcept
	: m_name(std::move(entry.m_name))
	, m_value(std::move(entry.m_value))
	, m_parent(nullptr) { }

DOSBoxConfiguration::Section::Entry::Entry(const Entry & e)
	: m_name(e.m_name)
	, m_value(e.m_value)
	, m_parent(nullptr) { }

DOSBoxConfiguration::Section::Entry & DOSBoxConfiguration::Section::Entry::operator = (Entry && entry) noexcept {
	if(this != &entry) {
		m_name = std::move(entry.m_name);
		m_value = std::move(entry.m_value);
	}

	return *this;
}

DOSBoxConfiguration::Section::Entry & DOSBoxConfiguration::Section::Entry::operator = (const Entry & entry) {
	m_name = entry.m_name;
	m_value = entry.m_value;

	return *this;
}

DOSBoxConfiguration::Section::Entry::~Entry() = default;

const std::string & DOSBoxConfiguration::Section::Entry::getName() const {
	return m_name;
}

bool DOSBoxConfiguration::Section::Entry::setName(const std::string & newName) {
	if(m_parent == nullptr) {
		return false;
	}

	return m_parent->setEntryName(*this, newName);
}

bool DOSBoxConfiguration::Section::Entry::hasValue() const {
	return !m_value.empty();
}

const std::string & DOSBoxConfiguration::Section::Entry::getValue() const {
	return m_value;
}

void DOSBoxConfiguration::Section::Entry::setValue(std::string_view value) {
	m_value = value;
}

void DOSBoxConfiguration::Section::Entry::clearValue() {
	setValue(Utilities::emptyString);
}

bool DOSBoxConfiguration::Section::Entry::remove() {
	if(m_parent == nullptr) {
		return false;
	}

	return m_parent->removeEntry(*this);
}

const DOSBoxConfiguration::Section * DOSBoxConfiguration::Section::Entry::getParentSection() const {
	return m_parent;
}

const DOSBoxConfiguration * DOSBoxConfiguration::Section::Entry::getParentConfiguration() const {
	if(m_parent == nullptr) {
		return nullptr;
	}

	return m_parent->getParentConfiguration();
}

std::unique_ptr<DOSBoxConfiguration::Section::Entry> DOSBoxConfiguration::Section::Entry::parseFrom(std::string_view data, Style * style) {
	size_t assignmentCharacterIndex = data.find_first_of(ASSIGNMENT_CHARACTER);

	if(assignmentCharacterIndex == std::string::npos) {
		return nullptr;
	}

	std::string_view name(std::string_view(data).substr(0, assignmentCharacterIndex));
	std::string_view value(std::string_view(data).substr(assignmentCharacterIndex + 1));

	if(style != nullptr && !name.empty() && name[name.length() -1] == ' ') {
		*style |= Style::WhitespaceAfterEntryNames;
	}

	return std::make_unique<Entry>(Utilities::trimString(name), Utilities::trimString(value));
}

bool DOSBoxConfiguration::Section::Entry::writeTo(ByteBuffer & data, Style style, size_t maxLength, const std::string & newline) const {
	if(!isValid(false)) {
		return false;
	}

	const std::string & actualNewline = newline.empty() ? Utilities::newLine : newline;

	std::stringstream entryStream;
	bool whitespaceAfterEntryNames = Any(style & Style::WhitespaceAfterEntryNames);

	entryStream << m_name;

	if(whitespaceAfterEntryNames) {
		entryStream << std::string((m_name.length() >= maxLength ? 0 : maxLength - m_name.length()) + 1, ' ');
	}

	entryStream << ASSIGNMENT_CHARACTER;

	if(whitespaceAfterEntryNames) {
		entryStream << ' ';
	}

	entryStream << m_value;

	return data.writeLine(entryStream.str(), actualNewline);
}

bool DOSBoxConfiguration::Section::Entry::isValid(bool validateParents) const {
	if(!isNameValid(m_name)) {
		return false;
	}

	if(validateParents) {
		if(m_parent == nullptr ||
		   m_parent->getParentConfiguration() == nullptr) {
			return false;
		}
	}

	return true;
}

bool DOSBoxConfiguration::Section::Entry::isValid(const Entry * entry, bool validateParents) {
	return entry != nullptr &&
		   entry->isValid(validateParents);
}

bool DOSBoxConfiguration::Section::Entry::isNameValid(std::string_view entryName) {
	static const std::string INVALID_NAME_CHARACTERS(fmt::format("{}{}{}{}\r\n", COMMENT_CHARACTER, ASSIGNMENT_CHARACTER, Section::NAME_START_CHARACTER, Section::NAME_END_CHARACTER));

	return !entryName.empty() &&
		   entryName.find_first_of(INVALID_NAME_CHARACTERS) == std::string::npos;
}

std::string DOSBoxConfiguration::Section::Entry::toString() const {
	return fmt::format("{}{}{}", m_name, ASSIGNMENT_CHARACTER, m_value);
}

bool DOSBoxConfiguration::Section::Entry::operator == (const Entry & entry) const {
	return Utilities::areStringsEqual(m_name, entry.m_name) &&
		   Utilities::areStringsEqual(m_value, entry.m_value);
}

bool DOSBoxConfiguration::Section::Entry::operator != (const Entry & entry) const {
	return !operator == (entry);
}
