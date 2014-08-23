#ifndef SCRIPT_ARGUMENTS_H
#define SCRIPT_ARGUMENTS_H

#include <QMap.h>
#include <QString.h>
#include <QStringList.h>
#include <QRegExp.h>
#include "Argument Collection/ArgumentCollection.h"

class ScriptArguments : public ArgumentCollection {
public:
	ScriptArguments();
	ScriptArguments(const ScriptArguments & s);
	ScriptArguments & operator = (const ScriptArguments & s);
	~ScriptArguments();

	QString applyConditionals(const QString & command) const;
	QString applyArguments(const QString & command) const;
	
	bool operator == (const ScriptArguments & s) const;
	bool operator != (const ScriptArguments & s) const;

private:
	QString applyConditionalsHelper(const QString & command) const;

public:
	const static char condOpenChar;
	const static char condOptionChar;
	const static char condCloseChar;
	const static char argChar;
};

#endif // SCRIPT_ARGUMENTS_H
