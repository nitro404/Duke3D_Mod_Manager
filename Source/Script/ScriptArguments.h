#ifndef SCRIPT_ARGUMENTS_H
#define SCRIPT_ARGUMENTS_H

#include <QVector.h>
#include <QMap.h>
#include <QString.h>
#include <QStringList.h>
#include <QRegExp.h>

class ScriptArguments {
public:
	ScriptArguments();
	ScriptArguments(const ScriptArguments & s);
	ScriptArguments & operator = (const ScriptArguments & s);
	~ScriptArguments();

	int numberOfArguments() const;
	bool hasArgument(const char * name) const;
	bool hasArgument(const QString & name) const;
	QString getValue(const char * name) const;
	QString getValue(const QString & name) const;
	bool setArgument(const char * name, const char * value);
	bool setArgument(const char * name, const QString & value);
	bool setArgument(const QString & name, const char * value);
	bool setArgument(const QString & name, const QString & value);
	void removeArgument(const char * name);
	void removeArgument(const QString & name);
	void clear();

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

private:
	QMap<QString, QString> m_arguments;
};

#endif // SCRIPT_ARGUMENTS_H
