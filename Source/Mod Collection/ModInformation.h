#ifndef MOD_INFORMATION_H
#define MOD_INFORMATION_H

#include <QString.h>
#include "Utilities/Utilities.h"

class ModInformation {
public:
	ModInformation(const char * name, const char * version);
	ModInformation(const QString & name, const QString & version);
	ModInformation(const ModInformation & m);
	ModInformation & operator = (const ModInformation & m);
	virtual ~ModInformation();

	const char * getName() const;
	const QString getFullName() const;
	const char * getVersion() const;

	void setName(const char * name);
	void setName(const QString & name);
	void setVersion(const char * version);
	void setVersion(const QString & version);

	bool operator == (const ModInformation & m) const;
	bool operator != (const ModInformation & m) const;

private:
	char * m_name;
	char * m_version;
};

#endif // MOD_INFORMATION_H
