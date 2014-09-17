#ifndef ARGUMENT_CASE_H
#define ARGUMENT_CASE_H

#include "Utilities/Utilities.h"

namespace ArgumentCases {
	enum ArgumentCase {
		Invalid = -1,
		OriginalCase,
		UpperCase,
		LowerCase,
		NumberOfCases
	};
	
	extern const char * caseStrings[];
	extern const ArgumentCase defaultCase;
	
	bool isValid(ArgumentCase caseType);
	bool isValid(int caseType);
	const char * toString(ArgumentCase caseType);
	const char * toString(int caseType);
	ArgumentCase parseFrom(const char * data);
	ArgumentCase parseFrom(const QString & data);
}

#endif // ARGUMENT_CASE_H
