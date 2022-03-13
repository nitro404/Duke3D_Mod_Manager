#include "ArgumentCollection.h"

#include <Utilities/StringUtilities.h>

#include <sstream>

const ArgumentCollection::ArgumentCase ArgumentCollection::DEFAULT_CASE = ArgumentCollection::ArgumentCase::OriginalCase;

ArgumentCollection::ArgumentCollection(ArgumentCollection::ArgumentCase caseType)
	: m_case(caseType) { }

ArgumentCollection::ArgumentCollection(const ArgumentCollection & argumentCollection) {
	m_case = argumentCollection.m_case;

	for(std::multimap<std::string, std::string>::const_iterator i = argumentCollection.m_arguments.begin(); i != argumentCollection.m_arguments.end(); ++i) {
		m_arguments.insert(std::pair<std::string, std::string>(i->first, i->second));
	}
}

ArgumentCollection::ArgumentCollection(ArgumentCollection && argumentCollection) noexcept
	: m_case(argumentCollection.m_case)
	, m_arguments(std::move(argumentCollection.m_arguments)) { }

ArgumentCollection & ArgumentCollection::operator = (ArgumentCollection && argumentCollection) noexcept {
	if (this != &argumentCollection) {
		m_case = argumentCollection.m_case;
		m_arguments = std::move(argumentCollection.m_arguments);
	}

	return *this;
}

ArgumentCollection & ArgumentCollection::operator = (const ArgumentCollection & argumentCollection) {
	m_arguments.clear();

	m_case = argumentCollection.m_case;

	for(std::multimap<std::string, std::string>::const_iterator i = argumentCollection.m_arguments.begin(); i != argumentCollection.m_arguments.end(); ++i) {
		m_arguments.insert(std::pair<std::string, std::string>(i->first, i->second));
	}

	return *this;
}

ArgumentCollection::~ArgumentCollection() { }

ArgumentCollection::ArgumentCase ArgumentCollection::getCase() const {
	return m_case;
}

void ArgumentCollection::setCase(ArgumentCollection::ArgumentCase caseType) {
	m_case = caseType;
}

size_t ArgumentCollection::numberOfArguments() const {
	return m_arguments.size();
}

size_t ArgumentCollection::numberOfArguments(const std::string & name) const {
	std::string formattedName(formatArgument(name));

	if(formattedName.empty()) {
		return 0;
	}

	return m_arguments.count(formattedName);
}

bool ArgumentCollection::hasArgument(const std::string & name) const {
	std::string formattedName(formatArgument(name));

	if(formattedName.empty()) {
		return false;
	}

	return m_arguments.find(formattedName) != m_arguments.end();
}

std::string ArgumentCollection::getFirstValue(const std::string & name) const {
	std::string formattedName(formatArgument(name));

	if(formattedName.empty()) {
		return std::string();
	}

	std::multimap<std::string, std::string>::const_iterator argument = m_arguments.find(formattedName);

	if(argument == m_arguments.end()) {
		return std::string();
	}

	return argument->second;
}

std::vector<std::string> ArgumentCollection::getValues(const std::string & name) const {
	std::string formattedName(formatArgument(name));

	if(formattedName.empty()) {
		return {};
	}

	std::vector<std::string> values;
	std::pair<std::multimap<std::string, std::string>::const_iterator, std::multimap<std::string, std::string>::const_iterator> nameRange = m_arguments.equal_range(formattedName);

	for(std::multimap<std::string, std::string>::const_iterator i = nameRange.first; i != nameRange.second; ++i) {
		values.emplace_back(i->second);
	}

	return values;
}

bool ArgumentCollection::addArgument(const std::string & name, const std::string & value) {
	std::string formattedName(formatArgument(name));

	if(formattedName.empty()) {
		return false;
	}

	m_arguments.insert(std::pair<std::string, std::string>(formattedName, value));

	return true;
}

void ArgumentCollection::removeArgument(const std::string & name) {
	std::string formattedName(formatArgument(name));

	if(formattedName.empty()) {
		return;
	}

	m_arguments.erase(formattedName);
}

void ArgumentCollection::clear() {
	m_arguments.clear();
}

std::string ArgumentCollection::formatArgument(const std::string & data) const {
	if(data.empty()) {
		return data;
	}

	std::string formattedData(Utilities::trimString(data));

	if(m_case == ArgumentCollection::ArgumentCase::UpperCase) {
		return Utilities::toUpperCase(formattedData);
	}
	else if(m_case == ArgumentCollection::ArgumentCase::LowerCase) {
		return Utilities::toLowerCase(formattedData);
	}

	return formattedData;
}

std::string ArgumentCollection::toString() const {
	std::stringstream args;

	for(std::multimap<std::string, std::string>::const_iterator i = m_arguments.begin(); i != m_arguments.end(); ++i) {
		if(i != m_arguments.begin()) {
			args << " ";
		}

		args << (i->first.length() == 1 ? "-" : "--") << i->first;

		if(!i->second.empty()) {
			args << " " << i->second;
		}
	}

	return args.str();
}

bool ArgumentCollection::operator == (const ArgumentCollection & argumentCollection) const {
	if(m_arguments.size() != argumentCollection.m_arguments.size()) {
		return false;
	}

	for(std::multimap<std::string, std::string>::const_iterator i = m_arguments.begin(); i != m_arguments.end(); ++i) {
		std::multimap<std::string, std::string>::const_iterator argument = argumentCollection.m_arguments.find(i->first);

		if(argument == m_arguments.end()) {
			return false;
		}

		std::string key(i->first);

		if(i->second != argument->second) {
			return false;
		}
	}
	return true;
}

bool ArgumentCollection::operator != (const ArgumentCollection & s) const {
	return !operator == (s);
}
