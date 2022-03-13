#ifndef _SCRIPT_H_
#define _SCRIPT_H_

#include "ScriptArguments.h"

#include <string>
#include <vector>

class Script final {
public:
	Script();
	Script(Script && s) noexcept;
	Script(const Script & s);
	Script & operator = (Script && s) noexcept;
	Script & operator = (const Script & s);
	~Script();

	size_t numberOfCommands() const;
	const std::string * getCommand(size_t lineNumber) const;
	bool addCommand(const std::string & command);
	bool setCommand(size_t lineNumber, const std::string & command);
	bool removeCommand(size_t lineNumber);
	void clear();

	bool readFrom(const std::string & scriptPath);

	std::string generateDOSBoxCommand(const ScriptArguments & arguments, const std::string & dosboxPath, const std::string & dosboxArguments) const;

	bool operator == (const Script & s) const;
	bool operator != (const Script & s) const;

private:
	std::vector<std::string> m_commands;
};

#endif // _SCRIPT_H_
