#ifndef MOD_H
#define MOD_H

#include "Utilities/Utilities.h"
#include "Mod Collection/ModType.h"

class Mod {
public:
	Mod(const char * name, const char * type = NULL, const char * group = NULL, const char * con = NULL);
	Mod(const char * name, int type = static_cast<int>(ModTypes::Unknown), const char * group = NULL, const char * con = NULL);
	Mod(const char * name, ModTypes::ModType type = ModTypes::Unknown, const char * group = NULL, const char * con = NULL);
	Mod(const Mod & m);
	Mod & operator = (const Mod & m);
	~Mod(void);

private:
	void Mod::init(const char * name, ModTypes::ModType type, const char * group, const char * con);

public:
	void setName(const char * name);
	void setType(ModTypes::ModType type);
	void setType(int type);
	void setType(const char * type);
	void setGroup(const char * group);
	void setCon(const char * con);

	const char * getName() const;
	ModTypes::ModType getType() const;
	const char * getTypeName() const;
	const char * getGroup() const;
	const char * getCon() const;

	bool operator == (const Mod & m) const;
	bool operator != (const Mod & m) const;

private:
	char * m_name;
	ModTypes::ModType m_type;
	char * m_group;
	char * m_con;
};

#endif // MOD_H
