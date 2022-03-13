#ifndef _SCRIPT_ARGUMENTS_H_
#define _SCRIPT_ARGUMENTS_H_

#include "Arguments/ArgumentCollection.h"

#include <map>
#include <regex>
#include <string>

class ScriptArguments final : public ArgumentCollection {
public:
	ScriptArguments();
	ScriptArguments(ScriptArguments && arguments) noexcept;
	ScriptArguments(const ScriptArguments & arguments);
	ScriptArguments & operator = (ScriptArguments && arguments) noexcept;
	ScriptArguments & operator = (const ScriptArguments & arguments);
	virtual ~ScriptArguments();

	std::string applyConditionals(const std::string & command) const;
	std::string applyArguments(const std::string & command) const;

	bool operator == (const ScriptArguments & arguments) const;
	bool operator != (const ScriptArguments & arguments) const;

private:
	std::string applyConditionalsHelper(const std::string & command) const;

public:
	const static char condOpenChar;
	const static char condOptionChar;
	const static char condCloseChar;
	const static char argChar;
};

#endif // _SCRIPT_ARGUMENTS_H_
