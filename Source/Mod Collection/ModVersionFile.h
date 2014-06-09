#ifndef MOD_VERSION_FILE_H
#define MOD_VERSION_FILE_H

#include <QString.h>
#include "Utilities/Utilities.h"

class ModVersionFile {
public:
	ModVersionFile(const char * name, const char * type = NULL);
	ModVersionFile(const QString & name, const QString & type = QString());
	ModVersionFile(const ModVersionFile & f);
	ModVersionFile & operator = (const ModVersionFile & f);
	~ModVersionFile();

	const char * getName() const;
	const char * getType() const;

	void setName(const char * name);
	void setName(const QString & name);
	void setType(const char * type);
	void setType(const QString & type);

	bool operator == (const ModVersionFile & f) const;
	bool operator != (const ModVersionFile & f) const;

private:
	char * m_name;
	char * m_type;
};

#endif // MOD_VERSION_FILE_H
