#include "Mod Collection/ModCollection.h"

ModCollection::ModCollection() {
	
}

ModCollection::ModCollection(const ModCollection & m) {
	for(int i=0;i<m.m_mods.size();i++) {
		m_mods.push_back(new Mod(*m.m_mods[i]));
	}
}

ModCollection & ModCollection::operator = (const ModCollection & m) {
	for(int i=0;i<m.m_mods.size();i++) {
		delete m_mods[i];
	}
	m_mods.clear();

	for(int i=0;i<m.m_mods.size();i++) {
		m_mods.push_back(new Mod(*m.m_mods[i]));
	}

	return *this;
}

ModCollection::~ModCollection() {
	for(int i=0;i<m_mods.size();i++) {
		delete m_mods[i];
	}
}

int ModCollection::numberOfMods() const {
	return m_mods.size();
}

bool ModCollection::hasMod(const Mod & mod) const {
	for(int i=0;i<m_mods.size();i++) {
		if(*m_mods[i] == mod) {
			return true;
		}
	}

	return false;
}

bool ModCollection::hasMod(const char * name) const {
	if(name == NULL) { return false; }

	for(int i=0;i<m_mods.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_mods[i]->getName(), name) == 0) {
			return true;
		}
	}

	return false;
}

bool ModCollection::hasMod(const QString & name) const {
	QByteArray nameBytes = name.toLocal8Bit();
	const char * nameData = nameBytes.data();

	for(int i=0;i<m_mods.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_mods[i]->getName(), nameData) == 0) {
			return true;
		}
	}

	return false;
}

int ModCollection::indexOfMod(const Mod & mod) const {
	for(int i=0;i<m_mods.size();i++) {
		if(*m_mods[i] == mod) {
			return i;
		}
	}

	return -1;
}

int ModCollection::indexOfMod(const char * name) const {
	if(name == NULL) { return -1; }

	for(int i=0;i<m_mods.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_mods[i]->getName(), name) == 0) {
			return i;
		}
	}

	return -1;
}

int ModCollection::indexOfMod(const QString & name) const {
	QByteArray nameBytes = name.toLocal8Bit();
	const char * nameData = nameBytes.data();

	for(int i=0;i<m_mods.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_mods[i]->getName(), nameData) == 0) {
			return i;
		}
	}

	return -1;
}

const Mod * ModCollection::getMod(int index) const {
	if(index < 0 || index >= m_mods.size()) { return NULL; }

	return m_mods[index];
}

const Mod * ModCollection::getMod(const char * name) const {
	if(name == NULL) { return NULL; }

	for(int i=0;i<m_mods.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_mods[i]->getName(), name) == 0) {
			return m_mods[i];
		}
	}

	return NULL;
}

const Mod * ModCollection::getMod(const QString & name) const {
	QByteArray nameBytes = name.toLocal8Bit();
	const char * nameData = nameBytes.data();

	for(int i=0;i<m_mods.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_mods[i]->getName(), nameData) == 0) {
			return m_mods[i];
		}
	}

	return NULL;
}

bool ModCollection::addMod(const QString & name, const QString & type, const QString & group, const QString & con) {
	return addMod(name.toLocal8Bit().data(), ModTypes::parseFrom(type), group.toLocal8Bit().data(), con.toLocal8Bit().data());
}

bool ModCollection::addMod(const char * name, const char * type, const char * group, const char * con) {
	return addMod(name, ModTypes::parseFrom(type), group, con);
}

bool ModCollection::addMod(const QString & name, int type, const QString & group, const QString & con) {
	if(!ModTypes::isValid(type)) { return false; }

	return addMod(name.toLocal8Bit().data(), static_cast<ModTypes::ModType>(type), group.toLocal8Bit().data(), con.toLocal8Bit().data());
}

bool ModCollection::addMod(const char * name, int type, const char * group, const char * con) {
	if(!ModTypes::isValid(type)) { return false; }

	return addMod(name, static_cast<ModTypes::ModType>(type), group, con);
}

bool ModCollection::addMod(const QString & name, ModTypes::ModType type, const QString & group, const QString & con) {
	return addMod(name.toLocal8Bit().data(), type, group.toLocal8Bit().data(), con.toLocal8Bit().data());
}

bool ModCollection::addMod(const char * name, ModTypes::ModType type, const char * group, const char * con) {
	if(name == NULL || Utilities::stringLength(name) == 0 || hasMod(name) || !isValid(type) || ((group == NULL || Utilities::stringLength(group) == 0) && (con == NULL || Utilities::stringLength(con) == 0))) {
		return false;
	}
	
	m_mods.push_back(new Mod(name, type, group, con));

	return true;
}

bool ModCollection::addMod(const Mod & mod) {
	return addMod(mod.getName(), mod.getType(), mod.getGroup(), mod.getCon());
}

bool ModCollection::removeMod(int index) {
	if(index < 0 || index >= m_mods.size()) { return false; }
	
	delete m_mods[index];
	m_mods.remove(index);
	
	return true;
}

bool ModCollection::removeMod(const char * name) {
	if(name == NULL) { return false; }

	for(int i=0;i<m_mods.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_mods[i]->getName(), name) == 0) {
			delete m_mods[i];
			m_mods.remove(i);

			return true;
		}
	}

	return false;
}

bool ModCollection::removeMod(const QString & name) {
	QByteArray nameBytes = name.toLocal8Bit();
	const char * nameData = nameBytes.data();

	for(int i=0;i<m_mods.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_mods[i]->getName(), nameData) == 0) {
			delete m_mods[i];
			m_mods.remove(i);

			return true;
		}
	}

	return false;
}

bool ModCollection::removeMod(const Mod & mod) {
	for(int i=0;i<m_mods.size();i++) {
		if(*m_mods[i] == mod) {
			delete m_mods[i];
			m_mods.remove(i);
			
			return true;
		}
	}

	return false;
}

void ModCollection::clear() {
	for(int i=0;i<m_mods.size();i++) {
		delete m_mods[i];
	}

	m_mods.clear();
}

bool ModCollection::load() {
	return loadFrom(SettingsManager::getInstance()->modListFileName);
}

bool ModCollection::loadFrom(const char * fileName) {
	QString modListPath(SettingsManager::getInstance()->modListFileName);
	
	QFileInfo modListFile(modListPath);
	if(!modListFile.exists()) { return false; }
	
	QFile input(modListPath);
	if(!input.open(QIODevice::ReadOnly | QIODevice::Text)) { return false; }
	
	clear();
	
	QString line;
	QByteArray bytes;
	QString temp;
	QString name, type, group, con;
	bool addModToCollection = false;
	bool skipReadLine = false;
	
	while(true) {
		if(addModToCollection) {
			QByteArray nameBytes = name.toLocal8Bit();
			const char * nameData = nameBytes.data();
			
			QByteArray typeBytes = type.toLocal8Bit();
			const char * typeData = typeBytes.data();
			
			QByteArray groupBytes = group.toLocal8Bit();
			const char * groupData = groupBytes.data();
			
			QByteArray conBytes = con.toLocal8Bit();
			const char * conData = conBytes.data();
			
			addMod(nameData, typeData, groupData, conData);
			
			name.clear();
			type.clear();
			group.clear();
			con.clear();
			
			addModToCollection = false;
		}

		if(input.atEnd()) { break; }
		
		if(!skipReadLine) {
			line = input.readLine().trimmed();
		}
		else {
			skipReadLine = false;
		}

		if(line.length() == 0) { continue; }

		int separatorIndex = line.indexOf(':');

		if(separatorIndex < 0) { continue; }

		temp = line.mid(0, separatorIndex).trimmed();
		
		if(QString::compare(temp, "Name", Qt::CaseInsensitive) == 0) {
			if(name.length() > 0) {
				if(type.length() > 0 && (group.length() > 0 || con.length() > 0)) {
					addModToCollection = true;
					skipReadLine = true;
				}
				else {
					input.close();
					return false;
				}
			}
			
			if(!skipReadLine) {
				name = line.mid(separatorIndex + 1, line.length() - separatorIndex).trimmed();
			}
		}
		else if(QString::compare(temp, "Type", Qt::CaseInsensitive) == 0) {
			if(type.length() > 0) {
				if(name.length() > 0 && (group.length() > 0 || con.length() > 0)) {
					addModToCollection = true;
					skipReadLine = true;
				}
				else {
					input.close();
					return false;
				}
			}

			if(!skipReadLine) {
				type = line.mid(separatorIndex + 1, line.length() - separatorIndex).trimmed();
			}
		}
		else if(QString::compare(temp, "Group", Qt::CaseInsensitive) == 0) {
			if(group.length() > 0) {
				input.close();
				return false;
			}
			
			group = line.mid(separatorIndex + 1, line.length() - separatorIndex).trimmed();

			if(name.length() > 0 && type.length() > 0 && con.length() > 0) {
				addModToCollection = true;
			}
		}
		else if(QString::compare(temp, "Con", Qt::CaseInsensitive) == 0) {
			if(con.length() > 0) {
				input.close();
				return false;
			}
			
			con = line.mid(separatorIndex + 1, line.length() - separatorIndex).trimmed();

			if(name.length() > 0 && type.length() > 0 && group.length() > 0) {
				addModToCollection = true;
			}
		}
	}

	input.close();

	return true;
}

bool ModCollection::operator == (const ModCollection & m) const {
	if(m_mods.size() != m.m_mods.size()) { return false; }
	
	for(int i=0;i<m.m_mods.size();i++) {
		if(!hasMod(*m.m_mods[i])) {
			return false;
		}
	}
	return true;
}

bool ModCollection::operator != (const ModCollection & m) const {
	return !operator == (m);
}
