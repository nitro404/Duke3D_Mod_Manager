#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H

#include <QMap.h>
#include <QList.h>
#include <QString.h>
#include "Utilities/Utilities.h"
#include "Argument Collection/ArgumentCollection.h"

class ArgumentParser : public ArgumentCollection {
public:
	ArgumentParser();
	ArgumentParser(int argc, char * argv[]);
	ArgumentParser(const ArgumentParser & a);
	ArgumentParser & operator = (const ArgumentParser & a);
	~ArgumentParser();

	bool parseArguments(int argc, char * argv[]);
};

#endif // ARGUMENT_PARSER_H
