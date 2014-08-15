#include "Mod Manager/ArgumentParser.h"

ArgumentParser::ArgumentParser() {
	
}

ArgumentParser::ArgumentParser(int argc, char * argv[]) {
	parseArguments(argc, argv);
}

ArgumentParser::ArgumentParser(const ArgumentParser & a) {
	QList<QString> keys = a.m_arguments.keys();

	for(int i=0;i<keys.size();i++) {
		m_arguments[keys[i]] = a.m_arguments.value(keys[i]);
	}
}

ArgumentParser & ArgumentParser::operator = (const ArgumentParser & a) {
	m_arguments.clear();

	QList<QString> keys = a.m_arguments.keys();

	for(int i=0;i<keys.size();i++) {
		m_arguments[keys[i]] = a.m_arguments.value(keys[i]);
	}

	return *this;
}

ArgumentParser::~ArgumentParser() {
	
}

int ArgumentParser::numberOfArguments() const {
	return m_arguments.keys().size();
}

bool ArgumentParser::hasArgument(const char * name) const {
	if(name == NULL) { return false; }

	QString tempName(QString(name).trimmed());
	if(tempName.length() == 0) { return false; }

	return m_arguments.contains(tempName);
}

bool ArgumentParser::hasArgument(const QString & name) const {
	QString tempName(name.trimmed());
	if(tempName.length() == 0) { return false; }

	return m_arguments.contains(tempName);
}

QString ArgumentParser::getValue(const char * name) const {
	if(name == NULL) { return QString(); }

	QString tempName(QString(name).trimmed());
	if(tempName.length() == 0) { return QString(); }

	if(!m_arguments.contains(tempName)) { return QString(); }

	return m_arguments.value(tempName);
}

QString ArgumentParser::getValue(const QString & name) const {
	QString tempName(name.trimmed());
	if(tempName.length() == 0) { return QString(); }

	if(!m_arguments.contains(tempName)) { return QString(); }

	return m_arguments.value(tempName);
}

bool ArgumentParser::setArgument(const char * name, const char * value) {
	if(name == NULL || value == NULL) { return false; }

	QString tempName(QString(name).trimmed());
	if(tempName.length() == 0) { return false; }

	m_arguments[tempName] = QString(value);

	return true;
}

bool ArgumentParser::setArgument(const char * name, const QString & value) {
	QString tempName(QString(name).trimmed());
	if(tempName.length() == 0) { return false; }

	m_arguments[tempName] = value;

	return true;
}

bool ArgumentParser::setArgument(const QString & name, const char * value) {
	if(name.length() == 0 || value == NULL) { return false; }

	m_arguments[name] = QString(value);

	return true;
}

bool ArgumentParser::setArgument(const QString & name, const QString & value) {
	QString tempName(name.trimmed());
	if(tempName.length() == 0) { return false; }

	m_arguments[tempName] = value;

	return true;
}

void ArgumentParser::removeArgument(const char * name) {
	if(name == NULL) { return; }

	QString tempName(QString(name).trimmed());
	if(tempName.length() == 0) { return; }

	m_arguments.remove(tempName);
}

void ArgumentParser::removeArgument(const QString & name) {
	QString tempName(name.trimmed());
	if(tempName.length() == 0) { return; }

	m_arguments.remove(tempName);
}

void ArgumentParser::clear() {
	m_arguments.clear();
}

bool ArgumentParser::parseArguments(int argc, char * argv[]) {
	if(argc == 0) { return true; }
	if(argv == NULL) { return false; }

	QString arg;
	QString data;

	for(int i=1;i<argc;i++) {
		data = QString(argv[i]);

		if(arg.length() == 0) {
			if(data.startsWith("-")) {
				arg = data.mid(1, data.length() - 1);
			}
			else {
				printf("Malformed argument list.\n");
				return false;
			}
		}
		else {
			if(data.startsWith("-")) {
				m_arguments[arg] = QString();

				arg = data.mid(1, data.length() - 2);
			}
			else {
				m_arguments[arg] = data;
				
				arg.clear();
				data.clear();
			}
		}
	}

	return true;
}

void ArgumentParser::displayHelp() const {
	// TODO: add help display
}
