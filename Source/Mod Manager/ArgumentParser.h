#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H

#include <QMap.h>
#include <QList.h>
#include <QString.h>
#include "Utilities/Utilities.h"

class ArgumentParser {
public:
	ArgumentParser();
	ArgumentParser(int argc, char * argv[]);
	ArgumentParser(const ArgumentParser & a);
	ArgumentParser & operator = (const ArgumentParser & a);
	~ArgumentParser();

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

	bool parseArguments(int argc, char * argv[]);
	void displayHelp() const;

private:
	QMap<QString, QString> m_arguments;
};

#endif // ARGUMENT_PARSER_H
