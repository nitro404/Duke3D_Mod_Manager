#ifndef MOD_TEAM_H
#define MOD_TEAM_H

#include <QString.h>
#include <QVector.h>
#include "Utilities/Utilities.h"
#include "Mod Collection/ModTeamMember.h"

class ModTeam {
public:
	ModTeam(const char * name, const char * email = NULL);
	ModTeam(const QString & name, const QString & email = QString());
	ModTeam(const ModTeam & m);
	ModTeam & operator = (const ModTeam & m);
	~ModTeam();

	const char * getName() const;
	const char * getEmail() const;
	const char * getCity() const;
	const char * getState() const;
	const char * getCountry() const;

	void setName(const char * name);
	void setName(const QString & name);
	void setEmail(const char * email);
	void setEmail(const QString & email);
	void setCity(const char * city);
	void setCity(const QString & city);
	void setState(const char * state);
	void setState(const QString & state);
	void setCountry(const char * country);
	void setCountry(const QString & country);

	int numberOfMembers() const;
	bool hasMember(const ModTeamMember & member) const;
	bool hasMember(const char * memberName) const;
	bool hasMember(const QString & memberName) const;
	int indexOfMember(const ModTeamMember & member) const;
	int indexOfMember(const char * memberName) const;
	int indexOfMember(const QString & memberName) const;
	const ModTeamMember * getMember(int index) const;
	const ModTeamMember * getMember(const char * memberName) const;
	const ModTeamMember * getMember(const QString & memberName) const;
	bool addMember(ModTeamMember * member);
	bool removeMember(int index);
	bool removeMember(const ModTeamMember & member);
	bool removeMember(const char * memberName);
	bool removeMember(const QString & memberName);
	void clearMembers();

	bool operator == (const ModTeam & m) const;
	bool operator != (const ModTeam & m) const;

private:
	char * m_name;
	char * m_email;
	char * m_city;
	char * m_state;
	char * m_country;
	QVector<ModTeamMember *> m_members;
};

#endif // MOD_TEAM_H
