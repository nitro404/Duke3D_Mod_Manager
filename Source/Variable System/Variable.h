#ifndef VARIABLE_H
#define VARIABLE_H

#include "Utilities/Utilities.h"

class Variable {
public:
	Variable(const char * id = "", const char * value = "", int category = NO_CATEGORY);
	Variable(const Variable & v);
	Variable & operator = (const Variable & v);
	~Variable();

	char * getID() const;
	char * getValue() const;
	int getCategory() const;

	void setID(const char * id);
	void setValue(const char * value);
	void setValue(int value);
	void setValue(double value);
	void setValue(bool value);
	void setCategory(int category);

	void removeCategory();

	static Variable * parseFrom(const char * data);

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
