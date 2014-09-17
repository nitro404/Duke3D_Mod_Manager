#include "Launcher/ArgumentParser.h"

ArgumentParser::ArgumentParser()
	: ArgumentCollection() {
	
}

ArgumentParser::ArgumentParser(int argc, char * argv[])
	: ArgumentCollection() {
	parseArguments(argc, argv);
}

ArgumentParser::ArgumentParser(const ArgumentParser & a)
	: ArgumentCollection(a) {

}

ArgumentParser & ArgumentParser::operator = (const ArgumentParser & a) {
	ArgumentCollection::operator = (a);

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

				if(i == argc - 1) {
					m_arguments[arg] = QString();
				}
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

bool ArgumentParser::operator == (const ArgumentParser & a) const {
	return ArgumentCollection::operator == (dynamic_cast<const ArgumentCollection &>(a));
}

bool ArgumentParser::operator != (const ArgumentParser & a) const {
	return !operator == (a);
}
