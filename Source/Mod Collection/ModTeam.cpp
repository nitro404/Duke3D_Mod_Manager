#include "ModTeam.h"

ModTeam::ModTeam(const char * name, const char * email)
	: m_name(NULL)
	, m_email(NULL) {
	if(name == NULL) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		m_name = Utilities::trimCopy(name);
	}

	if(email != NULL) {
		m_email = Utilities::trimCopy(email);
	}
}

ModTeam::ModTeam(const QString & name, const QString & email)
	: m_name(NULL)
	, m_email(NULL) {
	if(name.isEmpty()) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		QByteArray nameBytes = name.toLocal8Bit();
		const char * nameData = nameBytes.data();
		m_name = Utilities::trimCopy(nameData);
	}

	if(!email.isNull()) {
		QByteArray emailBytes = email.toLocal8Bit();
		const char * emailData = emailBytes.data();
		m_email = Utilities::trimCopy(emailData);
	}
}

ModTeam::ModTeam(const ModTeam & m)
	: m_name(NULL)
	, m_email(NULL) {
	m_name = Utilities::trimCopy(m.m_name);

	if(m.m_email != NULL) {
		m_email = Utilities::trimCopy(m.m_email);
	}

	for(int i=0;i<m.m_members.size();i++) {
		m_members.push_back(new ModTeamMember(*m.m_members[i]));
	}
}

ModTeam & ModTeam::operator = (const ModTeam & m) {
	delete [] m_name;

	if(m_email != NULL) {
		delete [] m_email;
		m_email = NULL;
	}

	for(int i=0;i<m_members.size();i++) {
		delete m_members[i];
	}
	m_members.clear();

	m_name = Utilities::trimCopy(m.m_name);

	if(m.m_email != NULL) {
		m_email = Utilities::trimCopy(m.m_email);
	}

	for(int i=0;i<m.m_members.size();i++) {
		m_members.push_back(new ModTeamMember(*m.m_members[i]));
	}

	return *this;
}

ModTeam::~ModTeam() {
	delete [] m_name;
	if(m_email != NULL) { delete [] m_email; }

	for(int i=0;i<m_members.size();i++) {
		delete m_members[i];
	}
}

const char * ModTeam::getName() const {
	return const_cast<const char *>(m_name);
}

const char * ModTeam::getEmail() const {
	return const_cast<const char *>(m_email);
}

void ModTeam::setName(const char * name) {
	delete [] m_name;
	
	if(name == NULL) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		m_name = Utilities::trimCopy(name);
	}
}

void ModTeam::setName(const QString & name) {
	delete [] m_name;

	if(name.isEmpty()) {
		m_name = new char[1];
		m_name[0] = '\0';
	}
	else {
		QByteArray nameBytes = name.toLocal8Bit();
		const char * nameData = nameBytes.data();
		m_name = Utilities::trimCopy(nameData);
	}
}

void ModTeam::setEmail(const char * email) {
	if(m_email != NULL) {
		delete [] m_email;
		m_email = NULL;
	}
	
	if(email != NULL) {
		m_email = Utilities::trimCopy(email);
	}
}

void ModTeam::setEmail(const QString & email) {
	if(m_email != NULL) {
		delete [] m_email;
		m_email = NULL;
	}

	if(!email.isNull()) {
		QByteArray emailBytes = email.toLocal8Bit();
		const char * emailData = emailBytes.data();
		m_email = Utilities::trimCopy(emailData);
	}
}

int ModTeam::numberOfMembers() const {
	return m_members.size();
}

bool ModTeam::hasMember(const ModTeamMember & member) const {
	for(int i=0;i<m_members.size();i++) {
		if(*m_members[i] == member) {
			return true;
		}
	}
	return false;
}

bool ModTeam::hasMember(const char * memberName) const {
	if(memberName == NULL || Utilities::stringLength(memberName) == 0) { return false; }

	for(int i=0;i<m_members.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_members[i]->getName(), memberName) == 0) {
			return true;
		}
	}
	return false;
}

bool ModTeam::hasMember(const QString & memberName) const {
	if(memberName.isEmpty()) { return false; }
	QByteArray memberNameBytes = memberName.toLocal8Bit();
	const char * memberNameData = memberNameBytes.data();

	for(int i=0;i<m_members.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_members[i]->getName(), memberNameData) == 0) {
			return true;
		}
	}
	return false;
}

int ModTeam::indexOfMember(const ModTeamMember & member) const {
	for(int i=0;i<m_members.size();i++) {
		if(*m_members[i] == member) {
			return i;
		}
	}
	return -1;
}

int ModTeam::indexOfMember(const char * memberName) const {
	if(memberName == NULL || Utilities::stringLength(memberName) == 0) { return -1; }

	for(int i=0;i<m_members.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_members[i]->getName(), memberName) == 0) {
			return i;
		}
	}
	return -1;
}

int ModTeam::indexOfMember(const QString & memberName) const {
	if(memberName.isEmpty()) { return -1; }
	QByteArray memberNameBytes = memberName.toLocal8Bit();
	const char * memberNameData = memberNameBytes.data();

	for(int i=0;i<m_members.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_members[i]->getName(), memberNameData) == 0) {
			return i;
		}
	}
	return -1;
}

const ModTeamMember * ModTeam::getMember(int index) const {
	if(index < 0 || index >= m_members.size()) { return NULL; }

	return m_members[index];
}

const ModTeamMember * ModTeam::getMember(const char * memberName) const {
	if(memberName == NULL || Utilities::stringLength(memberName) == 0) { return NULL; }

	for(int i=0;i<m_members.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_members[i]->getName(), memberName) == 0) {
			return m_members[i];
		}
	}
	return NULL;
}

const ModTeamMember * ModTeam::getMember(const QString & memberName) const {
	if(memberName.isEmpty()) { return NULL; }
	QByteArray memberNameBytes = memberName.toLocal8Bit();
	const char * memberNameData = memberNameBytes.data();

	for(int i=0;i<m_members.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_members[i]->getName(), memberNameData) == 0) {
			return m_members[i];
		}
	}
	return NULL;
}

bool ModTeam::addMember(ModTeamMember * member) {
	if(member == NULL || Utilities::stringLength(member->getName()) == 0 || hasMember(*member)) {
		return false;
	}
	
	m_members.push_back(member);

	return true;
}

bool ModTeam::removeMember(int index) {
	if(index < 0 || index >= m_members.size()) { return false; }
	
	delete m_members[index];
	m_members.remove(index);
	
	return true;
}

bool ModTeam::removeMember(const ModTeamMember & member) {
	for(int i=0;i<m_members.size();i++) {
		if(*m_members[i] == member) {
			delete m_members[i];
			m_members.remove(i);
			
			return true;
		}
	}
	return false;
}

bool ModTeam::removeMember(const char * memberName) {
	if(memberName == NULL || Utilities::stringLength(memberName) == 0) { return false; }

	for(int i=0;i<m_members.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_members[i]->getName(), memberName) == 0) {
			delete m_members[i];
			m_members.remove(i);

			return true;
		}
	}
	return false;
}

bool ModTeam::removeMember(const QString & memberName) {
	if(memberName.isEmpty()) { return false; }
	QByteArray memberNameBytes = memberName.toLocal8Bit();
	const char * memberNameData = memberNameBytes.data();

	for(int i=0;i<m_members.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_members[i]->getName(), memberNameData) == 0) {
			delete m_members[i];
			m_members.remove(i);

			return true;
		}
	}
	return false;
}

void ModTeam::clearMembers() {
	for(int i=0;i<m_members.size();i++) {
		delete m_members[i];
	}
	m_members.clear();
}

bool ModTeam::operator == (const ModTeam & m) const {
	return Utilities::compareStringsIgnoreCase(m_name, m.m_name) == 0;
}

bool ModTeam::operator != (const ModTeam & m) const {
	return !operator == (m);
}
