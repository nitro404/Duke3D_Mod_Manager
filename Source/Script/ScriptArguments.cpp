#include "ScriptArguments.h"

#include <Utilities/StringUtilities.h>

#include <fmt/core.h>

#include <sstream>
#include <vector>

const char ScriptArguments::condOpenChar = '<';
const char ScriptArguments::condOptionChar = '|';
const char ScriptArguments::condCloseChar = '>';
const char ScriptArguments::argChar = ':';

ScriptArguments::ScriptArguments()
	: ArgumentCollection(ArgumentCollection::ArgumentCase::UpperCase) { }

ScriptArguments::ScriptArguments(ScriptArguments && arguments) noexcept
	: ArgumentCollection(std::move(arguments)) { }

ScriptArguments::ScriptArguments(const ScriptArguments & arguments)
	: ArgumentCollection(arguments) { }

ScriptArguments & ScriptArguments::operator = (ScriptArguments && arguments) noexcept {
	ArgumentCollection::operator = (arguments);

	return *this;
}

ScriptArguments & ScriptArguments::operator = (const ScriptArguments & arguments) {
	ArgumentCollection::operator = (arguments);

	return *this;
}

ScriptArguments::~ScriptArguments() { }

std::string ScriptArguments::applyConditionals(const std::string & command) const {
	if(command.empty()) {
		return command;
	}

	// count the number of open / closing brackets
	int64_t depth = 0;
	int64_t brackets = 0;

	for(size_t i = 0; i < command.length(); i++) {
		if(command[i] == condOpenChar) {
			depth++;
			brackets++;
		}
		else if(command[i] == condCloseChar) {
			depth--;
			brackets++;
		}
	}

	// if there is a missing open / closing bracket, or none at all, stop parsing
	if(depth != 0 || brackets == 0) {
		return command;
	}

	return applyConditionalsHelper(command);
}

std::string ScriptArguments::applyConditionalsHelper(const std::string & command) const {
	if(command.empty()) {
		return command;
	}

	std::string newCommand;
	std::stringstream tempCommand;

	int64_t depth = 0;
	int64_t start = -1;
	int64_t opt = -1;
	int64_t end = -1;
	int64_t last = 0;

	for(size_t i = 0; i < command.length(); i++) {
		if(command[i] == condOpenChar) {
			depth++;

			if(start < 0) {
				start = i;
			}
			else {
				if(opt < 0) {
					return command;
				}
			}
		}
		else if(command[i] == condOptionChar) {
			if(start >= 0 && opt < 0) {
				opt = i;
			}
		}
		else if(command[i] == condCloseChar) {
			depth--;

			if(depth == 0) {
				end = i;
			}
		}

		if(depth < 0) {
			return command;
		}

		if(start >= 0 && end >= 0) {
			if(start >= end || opt < 0) {
				return command;
			}

			std::string variable(command.substr(start + 1, opt - start - 1));
			std::string text(command.substr(opt + 1, end - opt - 1));
			std::string leftover(command.substr(last, start - last));

			tempCommand << leftover;

			if(hasArgument(variable) && getFirstValue(variable).length() > 0) {
				tempCommand << text;
			}

			last = end + 1;
			start = -1;
			opt = -1;
			end = -1;
		}
	}

	std::string leftover(command.substr(last, command.length() - last));

	tempCommand << leftover;

	newCommand = tempCommand.str();

	int brackets = 0;

	for(size_t i = 0; i < newCommand.length(); i++) {
		if(newCommand[i] == condOpenChar) {
			brackets++;
		}
		else if(newCommand[i] == condCloseChar) {
			brackets++;
		}
	}

	if(brackets > 0) {
		return applyConditionalsHelper(newCommand);
	}

	return newCommand;
}

std::string ScriptArguments::applyArguments(const std::string & command) const {
	if(command.empty()) {
		return std::string();
	}

	static const std::regex       argumentRegExp(fmt::format("{0}[^{0}]+{0}", argChar));
	static const std::regex   argumentTrimRegExp(fmt::format("^{0}|{0}$", argChar));

	std::string commandWithConditionalsApplied(applyConditionals(command));

	std::vector<std::string> argumentParts(Utilities::regularExpressionStringSplit(commandWithConditionalsApplied, argumentRegExp));
	std::vector<std::string> arguments;
	std::string argumentSearchString = commandWithConditionalsApplied;
	std::smatch argumentMatch;

	while(std::regex_search(argumentSearchString, argumentMatch, argumentRegExp)) {
		for(std::sub_match submatch : argumentMatch) {
			std::string argument(submatch.str());
			arguments.push_back(argument.substr(1, argument.length() - 2));
		}

		argumentSearchString = argumentMatch.suffix().str();
	}

	bool argumentFirst = argumentParts.empty() || commandWithConditionalsApplied.find(argumentParts[0]) != 0;

	std::stringstream newCommand;
	size_t j = 0;

	for(size_t i = 0; i < argumentParts.size(); i++) {
		if(!argumentFirst) {
			newCommand << argumentParts[i];
		}

		if(j < arguments.size()) {
			if(hasArgument(arguments[j])) {
				newCommand << getFirstValue(arguments[j]);
			}
			else {
				newCommand << fmt::format("{0}{1}{0}", argChar, arguments[j]);
			}

			j++;
		}

		if(argumentFirst) {
			newCommand << argumentParts[i];
		}
	}

	return newCommand.str();
}

bool ScriptArguments::operator == (const ScriptArguments & arguments) const {
	return ArgumentCollection::operator == (static_cast<const ArgumentCollection &>(arguments));
}

bool ScriptArguments::operator != (const ScriptArguments & arguments) const {
	return !operator == (arguments);
}
