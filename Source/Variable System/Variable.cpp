#include "Variable System/Variable.h"

const int Variable::NO_CATEGORY = -1;
char Variable::SEPARATORS[] = { ':', '=' };
const unsigned int Variable::NUMBER_OF_SEPARATORS = 2;

Variable::Variable(const char * id, const char * value, int category)
	: m_id(NULL) 
	, m_value(NULL)
	, m_category(category < 0 ? NO_CATEGORY : category) {
	setID(id);
	setValue(value);
}

#if USE_QT
Variable::Variable(const QString & id, const QString & value, int category)
	: m_id(NULL) 
	, m_value(NULL)
	, m_category(category < 0 ? NO_CATEGORY : category) {
	setID(id);
	setValue(value);
}
#endif // USE_QT

#if USE_STL
Variable::Variable(const std::string & id, const std::string & value, int category)
	: m_id(NULL) 
	, m_value(NULL)
	, m_category(category < 0 ? NO_CATEGORY : category) {
	setID(id);
	setValue(value);
}
#endif // USE_STL

Variable::Variable(const Variable & v)
	: m_id(NULL)
	, m_value(NULL)
	, m_category(v.m_category) {
	m_id = Utilities::copyString(v.m_id);
	m_value = Utilities::copyString(v.m_value);
}

Variable & Variable::operator = (const Variable & v) {
	delete [] m_id;
	delete [] m_value;

	m_id = Utilities::copyString(v.m_id);
	m_value = Utilities::copyString(v.m_value);

	m_category = v.m_category;

	return *this;
}

Variable::~Variable() {
	delete [] m_id;
	delete [] m_value;
}

char * Variable::getID() const {
	return m_id;
}

char * Variable::getValue() const {
	return m_value;
}

int Variable::getCategory() const {
	return m_category;
}

void Variable::setID(const char * id) {
	if(m_id != NULL) { delete [] m_id; }
	
	if(id == NULL) {
		m_id = new char[1];
		m_id[0] = '\0';
	}
	else {
		m_id = Utilities::trimCopyString(id);
	}
}

#if USE_QT
void Variable::setID(const QString & id) {
	if(m_id != NULL) { delete [] m_id; }
	
	if(id.isNull()) {
		m_id = new char[1];
		m_id[0] = '\0';
	}
	else {
		QByteArray idBytes = id.toLocal8Bit();
		m_id = Utilities::trimCopyString(idBytes.data());
	}
}
#endif // USE_QT

#if USE_STL
void Variable::setID(const std::string & id) {
	if(m_id != NULL) { delete [] m_id; }
	
	if(id.empty()) {
		m_id = new char[1];
		m_id[0] = '\0';
	}
	else {
		m_id = Utilities::trimCopyString(id.data());
	}
}
#endif // USE_STL

void Variable::setValue(const char * value) {
	if(m_value != NULL) { delete [] m_value; }
	
	if(value == NULL) {
		m_value = new char[1];
		m_value[0] = '\0';
	}
	else {
		m_value = Utilities::trimCopyString(value);
	}
}

#if USE_QT
void Variable::setValue(const QString & value) {
	if(m_value != NULL) { delete [] m_value; }
	
	if(value.isNull()) {
		m_value = new char[1];
		m_value[0] = '\0';
	}
	else {
		QByteArray valueBytes = value.toLocal8Bit();
		m_value = Utilities::trimCopyString(valueBytes.data());
	}
}
#endif // USE_QT

#if USE_STL
void Variable::setValue(const std::string & value) {
	if(m_value != NULL) { delete [] m_value; }
	
	if(value.empty()) {
		m_value = new char[1];
		m_value[0] = '\0';
	}
	else {
		m_value = Utilities::trimCopyString(value.data());
	}
}
#endif // USE_STL

void Variable::setValue(int value) {
	setValue(Utilities::toString(value));
}

void Variable::setValue(double value) {
	setValue(Utilities::toString(value));
}

void Variable::setValue(bool value) {
	setValue(value ? "true" : "false");
}

void Variable::setCategory(int category) {
	m_category = (category < 0) ? NO_CATEGORY : category;
}

void Variable::removeCategory() {
	m_category = NO_CATEGORY;
}

Variable * Variable::parseFrom(const char * data) {
#if USE_STL
	unsigned int i, j, formattedDataLength;
#else
	int i, j, formattedDataLength;
#endif // USE_STL

	if(data == NULL) { return NULL; }

	char * formattedData = Utilities::trimCopyString(data);
	
	if(Utilities::stringLength(formattedData) == 0) {
		delete [] formattedData;
		return NULL;
	}

	char * separator = NULL;
	formattedDataLength = Utilities::stringLength(formattedData);
	for(i=0;i<formattedDataLength;i++) {
		for(j=0;j<NUMBER_OF_SEPARATORS;j++) {
			if(formattedData[i] == SEPARATORS[j]) {
				separator = formattedData + (i * sizeof(char));
				break;
			}
		}
		if(separator != NULL) {
			break;
		}
	}

	if(separator == NULL) {
		delete [] formattedData;
		return NULL;
	}
	
	*separator = '\0';

	Variable * newVariable = new Variable(formattedData, separator + sizeof(char));

	delete [] formattedData;

	return newVariable;
}

#if USE_QT
Variable * Variable::parseFrom(const QString & data) {
	if(data.isEmpty()) { return NULL; }

	QString formattedData = data.trimmed();
	
	if(formattedData.isEmpty()) { return NULL; }

	int separatorIndex = -1;
	for(int i=0;i<formattedData.length();i++) {
		for(int j=0;j<NUMBER_OF_SEPARATORS;j++) {
			if(formattedData[i] == SEPARATORS[j]) {
				separatorIndex = i;
				break;
			}
		}
		if(separatorIndex != -1) {
			break;
		}
	}

	if(separatorIndex == -1) { return NULL; }

	return new Variable(Utilities::substring(formattedData, 0, separatorIndex), Utilities::substring(formattedData, separatorIndex + 1, formattedData.length()));
}
#endif // USE_QT

#if USE_STL
Variable * Variable::parseFrom(const std::string & data) {
	if(data.empty()) { return NULL; }

	std::string formattedData = Utilities::trimString(data);
	
	if(formattedData.empty()) { return NULL; }

	int separatorIndex = -1;
	for(unsigned int i=0;i<formattedData.length();i++) {
		for(unsigned int j=0;j<NUMBER_OF_SEPARATORS;j++) {
			if(formattedData[i] == SEPARATORS[j]) {
				separatorIndex = i;
				break;
			}
		}
		if(separatorIndex != -1) {
			break;
		}
	}

	if(separatorIndex == -1) { return NULL; }

	return new Variable(Utilities::substring(formattedData, 0, separatorIndex), Utilities::substring(formattedData, separatorIndex + 1, formattedData.length()));
}
#endif // USE_STL

bool Variable::operator == (const Variable & v) const {
	return Utilities::compareStringsIgnoreCase(m_id, v.m_id) == 0 &&
		   m_category == v.m_category;
}

bool Variable::operator != (const Variable & v) const {
	return !operator == (v);
}
