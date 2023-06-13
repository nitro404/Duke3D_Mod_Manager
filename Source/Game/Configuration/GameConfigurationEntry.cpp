#include "GameConfiguration.h"

#include <Utilities/StringUtilities.h>

#include <fmt/core.h>
#include <magic_enum.hpp>
#include <spdlog/spdlog.h>

#include <sstream>

static const std::string & getAssignmentString() {
	static std::string s_assignmentString;

	if(s_assignmentString.empty()) {
		s_assignmentString = fmt::format(" {} ", GameConfiguration::ASSIGNMENT_CHARACTER);
	}

	return s_assignmentString;
}

static std::vector<std::string> parseQuotedValues(const std::string & data) {
	size_t startIndex = 0;
	size_t endIndex = std::numeric_limits<size_t>::max();
	std::vector<std::string> values;

	while(true) {
		startIndex = data.find_first_of("\"", endIndex + 1);

		if(startIndex == std::string::npos) {
			break;
		}

		endIndex = data.find_first_of("\"", startIndex + 1);

		if(endIndex == std::string::npos) {
			break;
		}

		values.emplace_back(data.data() + startIndex + 1, endIndex - startIndex - 1);
	}

	return values;
}

GameConfiguration::Entry::Entry(const std::string & name)
	: m_name(name)
	, m_type(Type::Empty)
	, m_parentSection(nullptr)
	, m_parentGameConfiguration(nullptr) { }

GameConfiguration::Entry::Entry(Entry && e) noexcept
	: m_name(std::move(e.m_name))
	, m_type(e.m_type)
	, m_value(std::move(e.m_value))
	, m_parentSection(nullptr)
	, m_parentGameConfiguration(nullptr) { }

GameConfiguration::Entry::Entry(const Entry & e)
	: m_name(e.m_name)
	, m_type(e.m_type)
	, m_value(e.m_value)
	, m_parentSection(nullptr)
	, m_parentGameConfiguration(nullptr) { }

GameConfiguration::Entry & GameConfiguration::Entry::operator = (Entry && e) noexcept {
	if(this != &e) {
		m_name = std::move(e.m_name);
		m_type = e.m_type;
		m_value = std::move(e.m_value);
	}

	return *this;
}

GameConfiguration::Entry & GameConfiguration::Entry::operator = (const Entry & e) {
	m_name = e.m_name;
	m_type = e.m_type;
	m_value = e.m_value;

	return *this;
}

GameConfiguration::Entry::~Entry() {
	m_parentSection = nullptr;
	m_parentGameConfiguration = nullptr;
}

const std::string & GameConfiguration::Entry::getName() const {
	return m_name;
}

bool GameConfiguration::Entry::setName(const std::string & newName) {
	if(m_parentGameConfiguration == nullptr) {
		return false;
	}

	return m_parentGameConfiguration->setEntryName(*this, newName);
}

GameConfiguration::Entry::Type GameConfiguration::Entry::getType() const {
	return m_type;
}

bool GameConfiguration::Entry::isEmpty() const {
	return m_type == Type::Empty;
}

bool GameConfiguration::Entry::isInteger() const {
	return m_type == Type::Integer;
}

bool GameConfiguration::Entry::isHexadecimal() const {
	return m_type == Type::Hexadecimal;
}

bool GameConfiguration::Entry::isString() const {
	return m_type == Type::String;
}

bool GameConfiguration::Entry::isMultiString() const {
	return m_type == Type::MultiString;
}

int64_t GameConfiguration::Entry::getIntegerValue() const {
	if(!std::holds_alternative<int64_t>(m_value)) {
		return 0;
	}

	return std::get<int64_t>(m_value);
}

std::string GameConfiguration::Entry::getHexadecimalValue() const {
	if(!std::holds_alternative<int64_t>(m_value)) {
		return {};
	}

	std::stringstream hexadecimalValueStream;
	hexadecimalValueStream << "0x" << fmt::format("{:x}", std::get<int64_t>(m_value));

	return hexadecimalValueStream.str();
}

const std::string & GameConfiguration::Entry::getStringValue() const {
	if(!std::holds_alternative<std::string>(m_value)) {
		return Utilities::emptyString;
	}

	return std::get<std::string>(m_value);
}

size_t GameConfiguration::Entry::numberOfMultiStringValues() const {
	if(!std::holds_alternative<std::vector<std::string>>(m_value)) {
		return 0;
	}

	return std::get<std::vector<std::string>>(m_value).size();
}

bool GameConfiguration::Entry::hasMultiStringValue(const std::string & value) const {
	return indexOfMultiStringValue(value) != std::numeric_limits<size_t>::max();
}

size_t GameConfiguration::Entry::indexOfMultiStringValue(const std::string & value) const {
	if(!std::holds_alternative<std::vector<std::string>>(m_value)) {
		return false;
	}

	const std::vector<std::string> & multiStringValue = std::get<std::vector<std::string>>(m_value);

	for(const std::string & stringValue : multiStringValue) {
		if(Utilities::areStringsEqual(stringValue, value)) {
			return true;
		}
	}

	return false;
}

const std::string & GameConfiguration::Entry::getMultiStringValue(size_t index) const {
	if(!std::holds_alternative<std::vector<std::string>>(m_value)) {
		return Utilities::emptyString;
	}

	const std::vector<std::string> & multiStringValue = std::get<std::vector<std::string>>(m_value);

	if(index >= multiStringValue.size()) {
		return Utilities::emptyString;
	}

	return multiStringValue[index];
}

const std::vector<std::string> & GameConfiguration::Entry::getMultiStringValue() const {
	static const std::vector<std::string> s_emptyStringVector;

	if(!std::holds_alternative<std::vector<std::string>>(m_value)) {
		return s_emptyStringVector;
	}

	return std::get<std::vector<std::string>>(m_value);
}

void GameConfiguration::Entry::setEmpty() {
	m_type = Type::Empty;
}

void GameConfiguration::Entry::setIntegerValue(int64_t value) {
	m_type = Type::Integer;
	m_value = value;
}

void GameConfiguration::Entry::setHexadecimalValueFromDecimal(int64_t value) {
	m_type = Type::Hexadecimal;
	m_value = value;
}

void GameConfiguration::Entry::setStringValue(const std::string & value) {
	m_type = Type::String;
	m_value = value;
}

bool GameConfiguration::Entry::setMultiStringValue(const std::string & value, size_t index, bool resizeValue) {
	if(m_type != Type::MultiString) {
		m_type = Type::MultiString;
		m_value = std::vector<std::string>();
	}

	std::vector<std::string> & multiStringValue = std::get<std::vector<std::string>>(m_value);

	if(index >= multiStringValue.size()) {
		if(!resizeValue) {
			return false;
		}
		else {
			if(index == std::numeric_limits<size_t>::max()) {
				return false;
			}

			size_t numberOfValuesToInsert = multiStringValue.size() - index + 1;

			for(size_t i = 0; i < numberOfValuesToInsert; i++) {
				multiStringValue.push_back("");
			}
		}
	}

	multiStringValue[index] = value;

	return true;
}

void GameConfiguration::Entry::setMultiStringValue(const std::string & valueA, const std::string & valueB) {
	m_type = Type::MultiString;
	m_value = std::vector<std::string>({ valueA, valueB });
}

void GameConfiguration::Entry::setMultiStringValue(const std::vector<std::string> & value) {
	m_type = Type::MultiString;
	m_value = value;
}

void GameConfiguration::Entry::clearValue() {
	switch(m_type) {
		case Type::Empty: {
			break;
		}

		case Type::Integer:
		case Type::Hexadecimal: {
			m_value = static_cast<int64_t>(0);
			break;
		}

		case Type::String: {
			m_value = Utilities::emptyString;
			break;
		}

		case Type::MultiString: {
			std::vector<std::string> & multiStringValue = std::get<std::vector<std::string>>(m_value);

			for(std::string & stringValue : multiStringValue) {
				stringValue = "";
			}

			break;
		}
	}
}

bool GameConfiguration::Entry::remove() {
	if(m_parentSection == nullptr) {
		return false;
	}

	return m_parentSection->removeEntry(*this);
}

const GameConfiguration::Section * GameConfiguration::Entry::getParentSection() const {
	return m_parentSection;
}

const GameConfiguration * GameConfiguration::Entry::getParentGameConfiguration() const {
	return m_parentGameConfiguration;
}

std::string GameConfiguration::Entry::toString() const {
	const std::string & assignmentString(getAssignmentString());

	std::stringstream valueStream;

	switch(m_type) {
		case Type::Empty: {
			valueStream << EMPTY_VALUE_CHARACTER;
			break;
		}

		case Type::Integer: {
			valueStream << std::get<int64_t>(m_value);
			break;
		}

		case Type::Hexadecimal: {
			valueStream << "0x" << fmt::format("{:x}", std::get<int64_t>(m_value));
			break;
		}

		case Type::String: {
			valueStream << '"' << std::get<std::string>(m_value) << '"';
			break;
		}

		case Type::MultiString: {
			const std::vector<std::string> & multiStringValue = std::get<std::vector<std::string>>(m_value);

			for(const std::string & stringValue : multiStringValue) {
				if(valueStream.tellp() != 0) {
					valueStream << ' ';
				}

				valueStream << '"' << stringValue << '"';
			}

			break;
		}
	}

	return m_name + assignmentString + valueStream.str();
}

std::unique_ptr<GameConfiguration::Entry> GameConfiguration::Entry::parseFrom(std::string_view data) {
	const std::string & assignmentString(getAssignmentString());

	if(data.empty()) {
		return nullptr;
	}

	size_t assignmentStringIndex = data.find(assignmentString);

	if(assignmentStringIndex == std::string::npos) {
		return nullptr;
	}

	size_t valueIndex = assignmentStringIndex + assignmentString.size();

	std::string_view name(data.data(), assignmentStringIndex);

	if(name.empty()) {
		spdlog::warn("Entry is missing a name.");
		return nullptr;
	}

	std::string value(data.data() + valueIndex, data.length() - valueIndex);

	std::unique_ptr<Entry> entry(new Entry(std::string(name)));

	if(value.empty()) {
		spdlog::warn("'{}' entry is missing a value.", name);
		return nullptr;
	}

	if(value[0] == EMPTY_VALUE_CHARACTER) {
		entry->m_type = Type::Empty;
	}
	else if(value[0] == '0' && value.length() >= 2 && value[1] == 'x') {
		entry->m_type = Type::Hexadecimal;
		entry->m_value = static_cast<int64_t>(std::stoul(value, nullptr, 16));
	}
	else if(value[0] == '"' && value.length() >= 2 && value[value.length() - 1] == '"') {
		std::string_view stringValue(value.data() + 1, value.length() - 2);

		if(stringValue.find_first_of("\"") == std::string::npos) {
			entry->m_type = Type::String;
			entry->m_value = std::string(stringValue);
		}
		else {
			entry->m_type = Type::MultiString;
			entry->m_value = parseQuotedValues(value);
		}
	}
	else {
		std::optional<int64_t> optionalLongValue(Utilities::parseLong(value));

		if(!optionalLongValue.has_value()) {
			spdlog::warn("'{}' entry has an invalid value: '{}'.", name, value);
			return nullptr;
		}

		entry->m_type = Type::Integer;
		entry->m_value = optionalLongValue.value();
	}

	return std::move(entry);
}

bool GameConfiguration::Entry::isValid(bool validateParents) const {
	if(!isNameValid(m_name)) {
		return false;
	}

	if(validateParents &&
	   (m_parentSection == nullptr ||
	    m_parentGameConfiguration == nullptr)) {
		return false;
	}

	switch(m_type) {
		case Type::Empty: {
			break;
		}

		case Type::Integer:
		case Type::Hexadecimal: {
			if(!std::holds_alternative<int64_t>(m_value)) {
				return false;
			}

			break;
		}

		case Type::String: {
			if(!std::holds_alternative<std::string>(m_value)) {
				return false;
			}

			break;
		}

		case Type::MultiString: {
			if(!std::holds_alternative<std::vector<std::string>>(m_value)) {
				return false;
			}

			break;
		}
	}

	return true;
}

bool GameConfiguration::Entry::isValid(const Entry * entry, bool validateParents) {
	return entry != nullptr &&
		   entry->isValid(validateParents);
}

bool GameConfiguration::Entry::isNameValid(const std::string & entryName) {
	return !entryName.empty() &&
		   entryName.find_first_of(";[]=\t ") == std::string::npos;
}

bool GameConfiguration::Entry::operator == (const Entry & e) const {
	return Utilities::areStringsEqualIgnoreCase(m_name, e.m_name) &&
		   m_value == e.m_value;
}

bool GameConfiguration::Entry::operator != (const Entry & e) const {
	return !operator == (e);
}
