#include "Argument Collection/ArgumentCollection.h"

const ArgumentCases::ArgumentCase ArgumentCollection::defaultCase = ArgumentCases::defaultCase;

ArgumentCollection::ArgumentCollection(ArgumentCases::ArgumentCase caseType) : m_case(defaultCase) {
	setCase(caseType);
}

ArgumentCollection::ArgumentCollection(const ArgumentCollection & s) {
	m_case = s.m_case;

	QMapIterator<QString, QString> i(s.m_arguments);
	while(i.hasNext()) {
		i.next();
		m_arguments[i.key()] = i.value();
	}
}

ArgumentCollection & ArgumentCollection::operator = (const ArgumentCollection & s) {
	m_case = s.m_case;

	m_arguments.clear();

	QMapIterator<QString, QString> i(s.m_arguments);
	while(i.hasNext()) {
		i.next();
		m_arguments[i.key()] = i.value();
	}

	return *this;
}

ArgumentCollection::~ArgumentCollection() {
	
}

ArgumentCases::ArgumentCase ArgumentCollection::getCase() const {
	return m_case;
}

bool ArgumentCollection::setCase(ArgumentCases::ArgumentCase caseType) {
	if(!ArgumentCases::isValid(caseType) || m_arguments.size() > 0) {
		return false;
	}

	m_case = caseType;

	return true;
}

int ArgumentCollection::numberOfArguments() const {
	return m_arguments.keys().size();
}

bool ArgumentCollection::hasArgument(const char * name) const {
	if(name == NULL) { return false; }

	QString formattedName = formatArgument(name);
	if(formattedName.isEmpty()) { return false; }

	return m_arguments.contains(formattedName);
}

bool ArgumentCollection::hasArgument(const QString & name) const {
	QString formattedName = formatArgument(name);
	if(formattedName.isEmpty()) { return false; }

	return m_arguments.contains(formattedName);
}

QString ArgumentCollection::getValue(int index) const {
	if(index < 0) { return QString(); }

	QList<QString> keys = m_arguments.keys();

	if(index >= keys.size()) { return QString(); }

	return m_arguments.value(keys[index]);
}

QString ArgumentCollection::getValue(const char * name) const {
	if(name == NULL) { return QString(); }

	QString formattedName = formatArgument(name);
	if(formattedName.isEmpty()) { return QString(); }

	if(!m_arguments.contains(formattedName)) { return QString(); }

	return m_arguments.value(formattedName);
}

QString ArgumentCollection::getValue(const QString & name) const {
	QString formattedName = formatArgument(name);
	if(formattedName.isEmpty()) { return QString(); }

	if(!m_arguments.contains(formattedName)) { return QString(); }

	return m_arguments.value(formattedName);
}

bool ArgumentCollection::setArgument(const char * name, const char * value) {
	if(name == NULL || value == NULL) { return false; }

	QString formattedName = formatArgument(name);
	if(formattedName.isEmpty()) { return false; }

	m_arguments[formattedName] = QString(value);

	return true;
}

bool ArgumentCollection::setArgument(const char * name, const QString & value) {
	QString formattedName = formatArgument(name);
	if(formattedName.isEmpty()) { return false; }

	m_arguments[formattedName] = value;

	return true;
}

bool ArgumentCollection::setArgument(const QString & name, const char * value) {
	if(name.length() == 0 || value == NULL) { return false; }

	QString formattedName = formatArgument(name);
	if(formattedName.isEmpty()) { return false; }

	m_arguments[formattedName] = QString(value);

	return true;
}

bool ArgumentCollection::setArgument(const QString & name, const QString & value) {
	QString formattedName = formatArgument(name);
	if(formattedName.isEmpty()) { return false; }

	m_arguments[formattedName] = value;

	return true;
}

void ArgumentCollection::removeArgument(const char * name) {
	if(name == NULL) { return; }

	QString formattedName = formatArgument(name);
	if(formattedName.isEmpty()) { return; }

	m_arguments.remove(formattedName);
}

void ArgumentCollection::removeArgument(const QString & name) {
	QString formattedName = formatArgument(name);
	if(formattedName.isEmpty()) { return; }

	m_arguments.remove(formattedName);
}

void ArgumentCollection::clear() {
	m_arguments.clear();
}

bool ArgumentCollection::operator == (const ArgumentCollection & s) const {
	if(m_arguments.size() != s.m_arguments.size()) { return false; }
	
	QList<QString> keys = m_arguments.keys();

	for(int i=0;i<keys.size();i++) {
		if(!s.m_arguments.contains(keys[i])) {
			return false;
		}
		
		if(QString::compare(m_arguments.value(keys[i]), s.m_arguments.value(keys[i]), Qt::CaseSensitive) != 0) {
			return false;
		}
	}
	return true;
}

bool ArgumentCollection::operator != (const ArgumentCollection & s) const {
	return !operator == (s);
}

QString ArgumentCollection::formatArgument(const char * data) const {
	if(data == NULL) {
		return QString();
	}

	return formatArgument(QString(data));
}

QString ArgumentCollection::formatArgument(const QString & data) const {
	if(data.isEmpty()) {
		return data;
	}

	QString formattedData = data.trimmed();

	if(m_case == ArgumentCases::UpperCase) {
		return formattedData.toUpper();
	}
	else if(m_case == ArgumentCases::LowerCase) {
		return formattedData.toLower();
	}

	return formattedData;
}
