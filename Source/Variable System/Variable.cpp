#include "Variable System/Variable.h"

const int Variable::NO_CATEGORY = -1;
char Variable::SEPARATORS[] = { ':', '=' };
const unsigned int Variable::NUMBER_OF_SEPARATORS = 2;

Variable::Variable(const char * id, const char * value, int category) {
	if(id == NULL) {
		m_id = new char[1];
		m_id[0] = '\0';
	}
	else {
		m_id = Utilities::trimCopyString(id);
	}

	if(value == NULL) {
		m_value = new char[1];
		m_value[0] = '\0';
	}
	else {
		m_value = Utilities::trimCopyString(value);
	}

	m_category = (category < -1) ? NO_CATEGORY : category;
}

Variable::Variable(const Variable & v) {
	m_id = new char[Utilities::stringLength(v.m_id) + 1];
	Utilities::copyString(m_id, Utilities::stringLength(v.m_id) + 1, v.m_id);

	m_value = new char[Utilities::stringLength(v.m_value) + 1];
	Utilities::copyString(m_value, Utilities::stringLength(v.m_value) + 1, v.m_value);
	
	m_category = v.m_category;
}

Variable & Variable::operator = (const Variable & v) {
	delete [] m_id;
	delete [] m_value;

	m_id = new char[Utilities::stringLength(v.m_id) + 1];
	Utilities::copyString(m_id, Utilities::stringLength(v.m_id) + 1, v.m_id);

	m_value = new char[Utilities::stringLength(v.m_value) + 1];
	Utilities::copyString(m_value, Utilities::stringLength(v.m_value) + 1, v.m_value);

	m_category = v.m_category;

	return *this;
}

Variable::~Variable() {
	delete [] m_id;
	delete [] m_value;
}

char * Variable::getID() const { return m_id; }

char * Variable::getValue() const { return m_value; }

int Variable::getCategory() const { return m_category; }

void Variable::setID(const char * id) {
	delete [] m_id;
	
	if(id == NULL) {
		m_id = new char[1];
		m_id[0] = '\0';
	}
	else {
		m_id = Utilities::trimCopyString(id);
	}
}

void Variable::setValue(const char * value) {
	delete [] m_value;
	
	if(value == NULL) {
		m_value = new char[1];
		m_value[0] = '\0';
	}
	else {
		m_value = Utilities::trimCopyString(value);
	}
}

void Variable::setValue(int value) {
	static char buffer[32];
	sprintf_s(buffer, 32, "%d", value);
	setValue(buffer);
}

void Variable::setValue(double value) {
	static char buffer[32];
	sprintf_s(buffer, 32, "%f", value);
	setValue(buffer);
}

void Variable::setValue(bool value) {
	setValue(value ? "true" : "false");
}

void Variable::setCategory(int category) {
	m_category = (category < -1) ? NO_CATEGORY : category;
}

void Variable::removeCategory() {
	m_category = NO_CATEGORY;
}

Variable * Variable::parseFrom(const char * data) {
	if(data == NULL) { return NULL; }

	char * temp = Utilities::trimCopyString(data);
	
	if(Utilities::stringLength(temp) == 0) {
		delete [] temp;
		return NULL;
	}

	char * separator = NULL;
	for(unsigned int i=0;i<Utilities::stringLength(temp);i++) {
		for(unsigned int j=0;j<NUMBER_OF_SEPARATORS;j++) {
			if(temp[i] == SEPARATORS[j]) {
				separator = temp + (i * sizeof(char));
				break;
			}
		}
		if(separator != NULL) {
			break;
		}
	}

	if(separator == NULL) {
		delete [] temp;
		return NULL;
	}
	
	*separator = '\0';

	Variable * newVariable = new Variable(temp, separator + sizeof(char));

	delete [] temp;

	return newVariable;
}

bool Variable::operator == (const Variable & v) const {
	return Utilities::compareStringsIgnoreCase(m_id, v.m_id) == 0 &&
		   m_category == v.m_category;
}

bool Variable::operator != (const Variable & v) const {
	return !operator == (v);
}

