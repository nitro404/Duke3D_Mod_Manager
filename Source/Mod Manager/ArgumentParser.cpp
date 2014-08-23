#include "Mod Manager/ArgumentParser.h"

ArgumentParser::ArgumentParser() {
	
}

ArgumentParser::ArgumentParser(int argc, char * argv[]) {
	parseArguments(argc, argv);
}

ArgumentParser::ArgumentParser(const ArgumentParser & a) {
	m_case = a.m_case;
	m_arguments = a.m_arguments;
}

ArgumentParser & ArgumentParser::operator = (const ArgumentParser & a) {
	m_case = a.m_case;
	m_arguments = a.m_arguments;

	return *this;
}

ArgumentParser::~ArgumentParser() {
	
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
