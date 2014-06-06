#ifndef SCRIPT_H
#define SCRIPT_H

#include <QString.h>
#include <QVector.h>
#include "Script/ScriptArguments.h"
#include "Settings Manager/SettingsManager.h"

class Script {
public:
	Script();
	Script(const Script & s);
	Script & operator = (const Script & s);
	~Script();

	int numberOfCommands() const;
	const QString * getCommand(int lineNumber) const;
	bool addCommand(const char * command);
	bool addCommand(const QString & command);
	bool setCommand(int lineNumber, const char * command);
	bool setCommand(int lineNumber, const QString & command);
	bool removeCommand(int lineNumber);
	void clear();

	bool readFrom(const char * fileName);
	bool readFrom(const QString & fileName);

	QString generateWindowsCommand(const ScriptArguments & arguments, int lineNumber) const;
	QString generateDOSBoxCommand(const ScriptArguments & arguments) const;

	bool operator == (const Script & s) const;
	bool operator != (const Script & s) const;

private:
	QVector<QString> m_commands;
};

#endif // SCRIPT_H
