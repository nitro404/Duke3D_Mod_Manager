#ifndef VARIABLE_H
#define VARIABLE_H

#include "Utilities/Utilities.h"
#if USE_QT
#include <QString.h>
#else
#include <iostream>
#endif // USE_QT

class Variable {
public:
	Variable(const char * id = NULL, const char * value = NULL, int category = NO_CATEGORY);
#if USE_QT
	Variable(const QString & id, const QString & value, int category = NO_CATEGORY);
#else
	Variable(const std::string & id, const std::string & value, int category = NO_CATEGORY);
#endif // USE_QT
	Variable(const Variable & v);
	Variable & operator = (const Variable & v);
	~Variable();

	char * getID() const;
	char * getValue() const;
	int getCategory() const;

	void setID(const char * id);
#if USE_QT
	void setID(const QString & id);
#else
	void setID(const std::string & id);
#endif // USE_QT
	void setValue(const char * value);
#if USE_QT
	void setValue(const QString & value);
#else
	void setValue(const std::string & value);
#endif // USE_QT
	void setValue(int value);
	void setValue(double value);
	void setValue(bool value);
	void setCategory(int category);

	void removeCategory();

	static Variable * parseFrom(const char * data);
#if USE_QT
	static Variable * parseFrom(const QString & data);
#else
	static Variable * parseFrom(const std::string & data);
#endif // USE_QT

	bool operator == (const Variable & v) const;
	bool operator != (const Variable & v) const;

private:
	char * m_id;
	char * m_value;
	int m_category;

public:
	static const int NO_CATEGORY;
	static char SEPARATORS[];
	static const unsigned int NUMBER_OF_SEPARATORS;
};

#endif // VARIABLE_H
