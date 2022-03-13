#include "Script.h"

#include <Utilities/FileUtilities.h>
#include <Utilities/StringUtilities.h>
#include <Utilities/Utilities.h>

#include <fmt/core.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <regex>
#include <sstream>

Script::Script() = default;

Script::Script(Script && s) noexcept
	: m_commands(std::move(s.m_commands)) { }

Script::Script(const Script & s) {
	for(std::vector<std::string>::const_iterator i = s.m_commands.begin(); i != s.m_commands.end(); ++i) {
		m_commands.push_back(*i);
	}
}

Script & Script::operator = (Script && s) noexcept {
	if(this != &s) {
		m_commands = std::move(s.m_commands);
	}

	return *this;
}

Script & Script::operator = (const Script & s) {
	m_commands.clear();

	for(std::vector<std::string>::const_iterator i = s.m_commands.begin(); i != s.m_commands.end(); ++i) {
		m_commands.push_back(*i);
	}

	return *this;
}

Script::~Script() = default;

size_t Script::numberOfCommands() const {
	return m_commands.size();
}

const std::string * Script::getCommand(size_t lineNumber) const {
	if(lineNumber >= m_commands.size()) {
		return nullptr;
	}

	return &m_commands[lineNumber];
}

bool Script::addCommand(const std::string & command) {
	if(command.empty()) {
		return false;
	}

	m_commands.push_back(command);

	return true;
}

bool Script::setCommand(size_t lineNumber, const std::string & command) {
	if(lineNumber >= m_commands.size() || command.empty() == 0) {
		return false;
	}

	m_commands[lineNumber] = command;

	return true;
}

bool Script::removeCommand(size_t lineNumber) {
	if(lineNumber >= m_commands.size()) {
		return false;
	}

	m_commands.erase(m_commands.begin() + lineNumber);

	return true;
}

void Script::clear() {
	m_commands.clear();
}

bool Script::readFrom(const std::string & scriptPath) {
	if(scriptPath.empty()) {
		return false;
	}

	if(!std::filesystem::is_regular_file(std::filesystem::path(scriptPath))) {
		return false;
	}

	std::ifstream fileStream(scriptPath);

	if(!fileStream.is_open()) {
		return false;
	}

	m_commands.clear();

	std::string line;

	while(std::getline(fileStream, line)) {
		line = Utilities::trimString(line);

		if(line.empty()) {
			continue;
		}

		m_commands.push_back(line);
	}

	fileStream.close();

	return true;
}

std::string Script::generateDOSBoxCommand(const ScriptArguments & arguments, const std::string & dosboxPath, const  std::string & dosboxArguments) const {
	static const std::regex unescapedQuotesRegExp("(?:^\"|([^\\\\])\")");

	if(dosboxPath.empty()) {
		return std::string();
	}

	std::stringstream command;

	command << fmt::format("CALL \"{}\" {} ", dosboxPath, dosboxArguments);

	std::string line;
	std::string formattedLine;

	for(size_t i = 0; i < m_commands.size(); i++) {
		line = arguments.applyArguments(m_commands[i]);

		formattedLine.clear();
		std::regex_replace(std::back_inserter(formattedLine), line.begin(), line.end(), unescapedQuotesRegExp, "$1\\\"");

		if(!formattedLine.empty()) {
			if(command.tellp() != 0) {
				command << " ";
			}

			command << fmt::format("-c \"{}\"", formattedLine);
		}
	}

	return Utilities::trimString(command.str());
}

bool Script::operator == (const Script & s) const {
	if(m_commands.size() != s.m_commands.size()) {
		return false;
	}

	for(size_t i = 0; i < m_commands.size(); i++) {
		if(m_commands[i] != s.m_commands[i]) {
			return false;
		}
	}

	return true;
}

bool Script::operator != (const Script & s) const {
	return !operator == (s);
}
