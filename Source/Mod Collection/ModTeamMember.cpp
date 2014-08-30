#include "ModTeamMember.h"

ModTeamMember::ModTeamMember(const char * name, const char * alias, const char * email)
	: m_name(NULL)
	, m_alias(NULL)
	, m_email(NULL) {
	if(name == NULL) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		m_name = Utilities::trimCopyString(name);
	}

	if(alias != NULL) {
		m_alias = Utilities::trimCopyString(alias);
	}

	if(email != NULL) {
		m_email = Utilities::trimCopyString(email);
	}
}

ModTeamMember::ModTeamMember(const QString & name, const QString & alias, const QString & email)
	: m_name(NULL)
	, m_alias(NULL)
	, m_email(NULL) {
	if(name.isEmpty()) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		QByteArray nameBytes = name.toLocal8Bit();
		m_name = Utilities::trimCopyString(nameBytes.data());
	}

	if(!alias.isNull()) {
		QByteArray aliasBytes = alias.toLocal8Bit();
		m_alias = Utilities::trimCopyString(aliasBytes.data());
	}

	if(!email.isNull()) {
		QByteArray emailBytes = email.toLocal8Bit();
		m_email = Utilities::trimCopyString(emailBytes.data());
	}
}

ModTeamMember::ModTeamMember(const ModTeamMember & m)
	: m_name(NULL)
	, m_alias(NULL)
	, m_email(NULL) {
	m_name = Utilities::trimCopyString(m.m_name);

	if(m.m_alias != NULL) {
		m_alias = Utilities::trimCopyString(m.m_alias);
	}

	if(m.m_email != NULL) {
		m_email = Utilities::trimCopyString(m.m_email);
	}
}

ModTeamMember & ModTeamMember::operator = (const ModTeamMember & m) {
	delete [] m_name;

	if(m_alias != NULL) {
		delete [] m_alias;
		m_alias = NULL;
	}

	if(m_email != NULL) {
		delete [] m_email;
		m_email = NULL;
	}

	m_name = Utilities::trimCopyString(m.m_name);

	if(m.m_alias != NULL) {
		m_alias = Utilities::trimCopyString(m.m_alias);
	}

	if(m.m_email != NULL) {
		m_email = Utilities::trimCopyString(m.m_email);
	}

	return *this;
}

ModTeamMember::~ModTeamMember() {
	delete [] m_name;
	if(m_alias != NULL) { delete [] m_alias; }
	if(m_email != NULL) { delete [] m_email; }
}

const char * ModTeamMember::getName() const {
	return m_name;
}

const char * ModTeamMember::getAlias() const {
	return m_alias;
}

const char * ModTeamMember::getEmail() const {
	return m_email;
}

void ModTeamMember::setName(const char * name) {
	delete [] m_name;
	
	if(name == NULL) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		m_name = Utilities::trimCopyString(name);
	}
}

void ModTeamMember::setName(const QString & name) {
	delete [] m_name;

	if(name.isEmpty()) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		QByteArray nameBytes = name.toLocal8Bit();
		m_name = Utilities::trimCopyString(nameBytes.data());
	}
}

void ModTeamMember::setAlias(const char * alias) {
	if(m_alias != NULL) {
		delete [] m_alias;
		m_alias = NULL;
	}
	
	if(alias != NULL) {
		m_alias = Utilities::trimCopyString(alias);
	}
}

void ModTeamMember::setAlias(const QString & alias) {
	if(m_alias != NULL) {
		delete [] m_alias;
		m_alias = NULL;
	}

	if(!alias.isNull()) {
		QByteArray aliasBytes = alias.toLocal8Bit();
		m_alias = Utilities::trimCopyString(aliasBytes.data());
	}
}

void ModTeamMember::setEmail(const char * email) {
	if(m_email != NULL) {
		delete [] m_email;
		m_email = NULL;
	}
	
	if(email != NULL) {
		m_email = Utilities::trimCopyString(email);
	}
}

void ModTeamMember::setEmail(const QString & email) {
	if(m_email != NULL) {
		delete [] m_email;
		m_email = NULL;
	}

	if(!email.isNull()) {
		QByteArray emailBytes = email.toLocal8Bit();
		m_email = Utilities::trimCopyString(emailBytes.data());
	}
}

bool ModTeamMember::operator == (const ModTeamMember & m) const {
	return Utilities::compareStringsIgnoreCase(m_name, m.m_name) == 0;
}

bool ModTeamMember::operator != (const ModTeamMember & m) const {
	return !operator == (m);
}
