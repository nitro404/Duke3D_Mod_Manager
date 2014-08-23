#include "Script/ScriptArguments.h"

const char ScriptArguments::condOpenChar = '<';
const char ScriptArguments::condOptionChar = '|';
const char ScriptArguments::condCloseChar = '>';
const char ScriptArguments::argChar = ':';

ScriptArguments::ScriptArguments() {
	m_case = ArgumentCases::UpperCase;
}

ScriptArguments::ScriptArguments(const ScriptArguments & s) {
	m_case = s.m_case;
	m_arguments = s.m_arguments;
}

ScriptArguments & ScriptArguments::operator = (const ScriptArguments & s) {
	m_case = s.m_case;
	m_arguments = s.m_arguments;

	return *this;
}

ScriptArguments::~ScriptArguments() {
	
}

QString ScriptArguments::applyConditionals(const QString & command) const {
	if(command.length() == 0) { return command; }

	// count the number of open / closing brackets
	int depth = 0;
	int brackets = 0;
	for(int i=0;i<command.length();i++) {
		if(command[i] == condOpenChar) {
			depth++;
			brackets++;
		}
		else if(command[i] == condCloseChar) {
			depth--;
			brackets++;
		}
	}

	// if there is a missing open / closing bracket, or none at all, stop parsing
	if(depth != 0 || brackets == 0) {
		return command;
	}

	return applyConditionalsHelper(command);
}

QString ScriptArguments::applyConditionalsHelper(const QString & command) const {
	if(command.length() == 0) { return command; }
	
	QString tempCommand;

	int depth = 0;
	int start = -1;
	int opt = -1;
	int end = -1;
	int last = 0;
	for(int i=0;i<command.length();i++) {
		if(command[i] == condOpenChar) {
			depth++;
			if(start < 0) {
				start = i;
			}
			else {
				if(opt < 0) {
					return command;
				}
			}
		}
		else if(command[i] == condOptionChar) {
			if(start >= 0 && opt < 0) {
				opt = i;
			}
		}
		else if(command[i] == condCloseChar) {
			depth--;
			if(depth == 0) {
				end = i;
			}
		}

		if(depth < 0) { return command; }

		if(start >= 0 && end >= 0) {
			if(start >= end || opt < 0) { return command; }

			QString variable = command.mid(start + 1, opt - start - 1);
			QString text = command.mid(opt + 1, end - opt - 1);
			QString leftover = command.mid(last, start - last);

			tempCommand.append(leftover);
			
			if(hasArgument(variable) && getValue(variable).length() > 0) {
				tempCommand.append(text);
			}

			last = end + 1;
			start = -1;
			opt = -1;
			end = -1;
		}
	}

	QString leftover = command.mid(last, command.length() - last);

	tempCommand.append(leftover);

	int brackets = 0;
	for(int i=0;i<tempCommand.length();i++) {
		if(tempCommand[i] == condOpenChar) {
			brackets++;
		}
		else if(tempCommand[i] == condCloseChar) {
			brackets++;
		}
	}

	if(brackets > 0) {
		return applyConditionalsHelper(tempCommand);
	}
	
	return tempCommand;
}

QString ScriptArguments::applyArguments(const QString & command) const {
	if(command.length() == 0) { return QString(); }
	
	QString newCommand, tempCommand;

	tempCommand = applyConditionals(command);

	QRegExp argumentRegExp(QString("%1[^%1]+%1").arg(argChar));

	QStringList argumentParts = tempCommand.split(argumentRegExp, QString::KeepEmptyParts);
	
	QRegExp argumentTrimRegExp(QString("^%1|%1$").arg(argChar));
	QStringList arguments;
	int position = 0;
	while((position = argumentRegExp.indexIn(tempCommand, position)) != -1)  {
		arguments << argumentRegExp.cap(0).replace(argumentTrimRegExp, "");
		position += argumentRegExp.matchedLength();
	}

	bool argumentFirst = !(argumentParts.size() > 0 && tempCommand.startsWith(argumentParts[0]), Qt::CaseSensitive);

	int j = 0;
	for(int i=0;i<argumentParts.size();i++) {
		if(!argumentFirst) {
			newCommand.append(argumentParts[i]);
		}

		if(j < arguments.size()) {
			if(hasArgument(arguments[j])) {
				newCommand.append(getValue(arguments[j]));
			}
			else {
				newCommand.append(QString("%1%2%1").arg(argChar).arg(arguments[j]));
			}

			j++;
		}

		if(argumentFirst) {
			newCommand.append(argumentParts[i]);
		}
	}
	
	return newCommand;
}

bool ScriptArguments::operator == (const ScriptArguments & s) const {
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

bool ScriptArguments::operator != (const ScriptArguments & s) const {
	return !operator == (s);
}
