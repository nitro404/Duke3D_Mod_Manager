#include "Mod Collection/Mod.h"

Mod::Mod(const char * name, const char * type, const char * group, const char * con)
	: m_name(NULL)
	, m_type(ModTypes::Unknown)
	, m_group(NULL)
	, m_con(NULL) {
	init(name, ModTypes::parseFrom(type), con, group);
}

Mod::Mod(const char * name, int type, const char * group, const char * con)
	: m_name(NULL)
	, m_type(ModTypes::Unknown)
	, m_group(NULL)
	, m_con(NULL) {
	init(name, ModTypes::isValid(type) ? static_cast<ModTypes::ModType>(type) : ModTypes::Unknown, group, con);
}

Mod::Mod(const char * name, ModTypes::ModType type, const char * group, const char * con)
	: m_name(NULL)
	, m_type(ModTypes::Unknown)
	, m_group(NULL)
	, m_con(NULL) {
	init(name, type, group, con);
}

Mod::Mod(const Mod & m)
	: m_type(m.m_type) {
	if(m.m_name == NULL) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		m_name = Utilities::trimCopy(m.m_name);
	}
	
	if(m.m_group == NULL) {
		m_group = new char[1];
		m_group[0] = '\0';
	}
	else {
		m_group = Utilities::trimCopy(m.m_group);
	}

	if(m.m_con == NULL) {
		m_con = new char[1];
		m_con[0] = '\0';
	}
	else {
		m_con = Utilities::trimCopy(m.m_con);
	}
}

Mod & Mod::operator = (const Mod & m) {
	delete [] m_name;
	delete [] m_con;
	delete [] m_group;
	
	if(m.m_name == NULL) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		m_name = Utilities::trimCopy(m.m_name);
	}

	m_type = m.m_type;

	if(m.m_group == NULL) {
		m_group = new char[1];
		m_group[0] = '\0';
	}
	else {
		m_group = Utilities::trimCopy(m.m_group);
	}
	
	if(m.m_con == NULL) {
		m_con = new char[1];
		m_con[0] = '\0';
	}
	else {
		m_con = Utilities::trimCopy(m.m_con);
	}

	return *this;
}

Mod::~Mod() {
	delete [] m_name;
	delete [] m_group;
	delete [] m_con;
}

void Mod::init(const char * name, ModTypes::ModType type, const char * group, const char * con) {
	if(name == NULL) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		m_name = Utilities::trimCopy(name);
	}

	m_type = ModTypes::isValid(type) ? m_type : ModTypes::Unknown;
	
	if(group == NULL) {
		m_group = new char[1];
		m_group[0] = '\0';
	}
	else {
		m_group = Utilities::trimCopy(group);
	}

	if(con == NULL) {
		m_con = new char[1];
		m_con[0] = '\0';
	}
	else {
		m_con = Utilities::trimCopy(con);
	}
}

void Mod::setName(const char * name) {
	if(m_name != NULL) {
		delete [] m_name;
	}
	
	if(name == NULL) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		m_name = Utilities::trimCopy(name);
	}
}

void Mod::setType(ModTypes::ModType type) {
	if(!ModTypes::isValid(type)) { return; }
	
	m_type = type;
}

void Mod::setType(int type) {
	if(!ModTypes::isValid(type)) { return; }

	m_type = static_cast<ModTypes::ModType>(type);
}

void Mod::setType(const char * type) {
	m_type = ModTypes::parseFrom(type);
}

void Mod::setGroup(const char * group) {
	if(m_group != NULL) {
		delete [] m_group;
	}

	if(group == NULL) {
		m_group = new char[1];
		m_group[0] = '\0';
	}
	else {
		m_group = Utilities::trimCopy(group);
	}
}

void Mod::setCon(const char * con) {
	if(m_con != NULL) {
		delete [] m_con;
	}
	
	if(con == NULL) {
		m_con = new char[1];
		m_con[0] = '\0';
	}
	else {
		m_con = Utilities::trimCopy(con);
	}
}

const char * Mod::getName() const {
	return const_cast<const char *>(m_name);
}

ModTypes::ModType Mod::getType() const {
	return m_type;
}

const char * Mod::getTypeName() const {
	return ModTypes::toString(m_type);
}

const char * Mod::getGroup() const {
	return const_cast<const char *>(m_group);
}

const char * Mod::getCon() const {
	return const_cast<const char *>(m_con);
}

bool Mod::operator == (const Mod & m) const {
	return Utilities::compareStringsIgnoreCase(m_name, m.m_name) == 0;
}

bool Mod::operator != (const Mod & x) const {
	return !operator == (x);
}
