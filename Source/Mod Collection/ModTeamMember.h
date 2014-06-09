#ifndef MOD_TEAM_MEMBER_H
#define MOD_TEAM_MEMBER_H

#include <QString.h>
#include "Utilities/Utilities.h"

class ModTeamMember {
public:
	ModTeamMember(const char * name, const char * alias = NULL, const char * email = NULL);
	ModTeamMember(const QString & name, const QString & alias = QString(), const QString & email = QString());
	ModTeamMember(const ModTeamMember & m);
	ModTeamMember & operator = (const ModTeamMember & m);
	~ModTeamMember();

	const char * getName() const;
	const char * getAlias() const;
	const char * getEmail() const;

	void setName(const char * name);
	void setName(const QString & name);
	void setAlias(const char * alias);
	void setAlias(const QString & alias);
	void setEmail(const char * email);
	void setEmail(const QString & email);

	bool operator == (const ModTeamMember & m) const;
	bool operator != (const ModTeamMember & m) const;

private:
	char * m_name;
	char * m_alias;
	char * m_email;
};

#endif // MOD_TEAM_MEMBER_H
