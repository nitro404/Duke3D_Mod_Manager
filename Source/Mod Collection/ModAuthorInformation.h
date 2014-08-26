#ifndef MOD_AUTHOR_INFORMATION_H
#define MOD_AUTHOR_INFORMATION_H

#include <QString.h>
#include "Utilities/Utilities.h"

class ModAuthorInformation {
public:
	ModAuthorInformation(const char * name = NULL);
	ModAuthorInformation(const QString & name);
	ModAuthorInformation(const ModAuthorInformation & m);
	ModAuthorInformation & operator = (const ModAuthorInformation & m);
	~ModAuthorInformation();
	
	const char * getName() const;
	void setName(const char * name);
	void setName(const QString & name);

	int getModCount() const;
	int incrementModCount();
	void setModCount(int numberOfMods);
	void resetModCount();
	
	bool operator == (const ModAuthorInformation & m) const;
	bool operator != (const ModAuthorInformation & m) const;

private:
	char * m_name;
	int m_numberOfMods;
};

#endif // MOD_AUTHOR_INFORMATION_H
