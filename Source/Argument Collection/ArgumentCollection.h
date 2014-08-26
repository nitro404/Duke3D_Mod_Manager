#ifndef ARGUMENT_COLLECTION_H
#define ARGUMENT_COLLECTION_H

#include <QMap.h>
#include <QString.h>
#include <QStringList.h>
#include "Utilities/Utilities.h"
#include "Argument Collection/ArgumentCase.h"

class ArgumentCollection {
public:
	ArgumentCollection(ArgumentCases::ArgumentCase caseType = defaultCase);
	ArgumentCollection(const ArgumentCollection & s);
	ArgumentCollection & operator = (const ArgumentCollection & s);
	virtual ~ArgumentCollection();

	ArgumentCases::ArgumentCase getCase() const;
	bool setCase(ArgumentCases::ArgumentCase caseType);

	int numberOfArguments() const;
	bool hasArgument(const char * name) const;
	bool hasArgument(const QString & name) const;
	QString getValue(int index) const;
	QString getValue(const char * name) const;
	QString getValue(const QString & name) const;
	bool setArgument(const char * name, const char * value);
	bool setArgument(const char * name, const QString & value);
	bool setArgument(const QString & name, const char * value);
	bool setArgument(const QString & name, const QString & value);
	void removeArgument(const char * name);
	void removeArgument(const QString & name);
	void clear();
	
	bool operator == (const ArgumentCollection & s) const;
	bool operator != (const ArgumentCollection & s) const;

private:
	QString formatArgument(const char * data) const;
	QString formatArgument(const QString & data) const;

public:
	const static ArgumentCases::ArgumentCase defaultCase;

protected:
	ArgumentCases::ArgumentCase m_case;
	QMap<QString, QString> m_arguments;
};

#endif // ARGUMENT_COLLECTION_H
