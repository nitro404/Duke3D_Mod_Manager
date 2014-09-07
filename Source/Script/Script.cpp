#include "Script/Script.h"

Script::Script() {
	
}

Script::Script(const Script & s) {
	for(int i=0;i<s.m_commands.size();i++) {
		m_commands.push_back(s.m_commands[i]);
	}
}

Script & Script::operator = (const Script & s) {
	m_commands.clear();

	for(int i=0;i<s.m_commands.size();i++) {
		m_commands.push_back(s.m_commands[i]);
	}

	return *this;
}

Script::~Script() {
	
}

int Script::numberOfCommands() const {
	return m_commands.size();
}

const QString * Script::getCommand(int lineNumber) const {
	if(lineNumber < 0 || lineNumber >= m_commands.size()) { return NULL; }

	return &m_commands[lineNumber];
}

bool Script::addCommand(const char * command) {
	if(command == NULL || Utilities::stringLength(command) == 0) { return false; }

	m_commands.push_back(QString(command));

	return true;
}

bool Script::addCommand(const QString & command) {
	if(command.length() == 0) { return false; }

	m_commands.push_back(command);

	return true;
}

bool Script::setCommand(int lineNumber, const char * command) {
	if(lineNumber < 0 || lineNumber >= m_commands.size() || command == NULL || Utilities::stringLength(command) == 0) { return false; }

	m_commands.replace(lineNumber, QString(command));

	return true;
}

bool Script::setCommand(int lineNumber, const QString & command) {
	if(lineNumber < 0 || lineNumber >= m_commands.size() || command.length() == 0) { return false; }

	m_commands.replace(lineNumber, command);

	return true;
}

bool Script::removeCommand(int lineNumber) {
	if(lineNumber < 0 || lineNumber >= m_commands.size()) { return false; }

	m_commands.remove(lineNumber);

	return true;
}

void Script::clear() {
	m_commands.clear();
}

bool Script::readFrom(const char * fileName) {
	if(fileName == NULL) { return false; }

	return readFrom(QString(fileName));
}

bool Script::readFrom(const QString & fileName) {
	if(fileName.length() == 0) { return false; }

	QString scriptPath = QString("%1/%2").arg(SettingsManager::getInstance()->dataDirectoryName).arg(fileName);
	
	QFileInfo scriptFile(scriptPath);
	if(!scriptFile.exists()) { return false; }
	
	QFile input(scriptPath);
	if(!input.open(QIODevice::ReadOnly | QIODevice::Text)) { return false; }

	m_commands.clear();

	QString line;
	while(true) {
		if(input.atEnd()) { break; }
		
		line = input.readLine().trimmed();

		if(line.length() == 0) { continue; }

		m_commands.push_back(line);
	}

	input.close();

	return true;
}

QString Script::generateWindowsCommand(const ScriptArguments & arguments, int lineNumber) const {
	if(lineNumber < 0 || lineNumber >= m_commands.size()) { return QString(); }

	return arguments.applyArguments(m_commands[lineNumber]).trimmed();
}

QString Script::generateDOSBoxCommand(const ScriptArguments & arguments) const {
	QString command, line;

	command.append("CALL \"");
	command.append(Utilities::generateFullPath(SettingsManager::getInstance()->DOSBoxPath, SettingsManager::getInstance()->DOSBoxFileName));
	command.append(QString("\" %1 ").arg(SettingsManager::getInstance()->DOSBoxArgs));

	for(int i=0;i<m_commands.size();i++) {
		line = arguments.applyArguments(m_commands[i]);

		if(line.length() > 0) {
			command.append(QString("-c \"%1\"").arg(line));

			if(i < m_commands.size() - 1) {
				command.append(" ");
			}
		}
	}

	return command.trimmed();
}

bool Script::operator == (const Script & s) const {
	if(m_commands.size() != s.m_commands.size()) { return false; }

	for(int i=0;i<m_commands.size();i++) {
		if(QString::compare(m_commands[i], s.m_commands[i], Qt::CaseSensitive) != 0) {
			return false;
		}
	}
	return true;
}

bool Script::operator != (const Script & s) const {
	return !operator == (s);
}
