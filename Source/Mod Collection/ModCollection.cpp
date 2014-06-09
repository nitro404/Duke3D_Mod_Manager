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
	if(name == NULL || Utilities::stringLength(name) == 0) { return false; }

	for(int i=0;i<m_mods.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_mods[i]->getName(), name) == 0) {
			return true;
		}
	}
	return false;
}

bool ModCollection::hasMod(const QString & name) const {
	if(name.isEmpty()) { return false; }
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
	if(name == NULL || Utilities::stringLength(name) == 0) { return -1; }

	for(int i=0;i<m_mods.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_mods[i]->getName(), name) == 0) {
			return i;
		}
	}
	return -1;
}

int ModCollection::indexOfMod(const QString & name) const {
	if(name.isEmpty()) { return -1; }
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
	if(name == NULL || Utilities::stringLength(name) == 0) { return NULL; }

	for(int i=0;i<m_mods.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_mods[i]->getName(), name) == 0) {
			return m_mods[i];
		}
	}
	return NULL;
}

const Mod * ModCollection::getMod(const QString & name) const {
	if(name.isEmpty()) { return NULL; }
	QByteArray nameBytes = name.toLocal8Bit();
	const char * nameData = nameBytes.data();

	for(int i=0;i<m_mods.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_mods[i]->getName(), nameData) == 0) {
			return m_mods[i];
		}
	}
	return NULL;
}

bool ModCollection::addMod(Mod * mod) {
	if(mod == NULL || Utilities::stringLength(mod->getName()) == 0 || hasMod(*mod)) {
		return false;
	}
	
	m_mods.push_back(mod);

	return true;
}

bool ModCollection::removeMod(int index) {
	if(index < 0 || index >= m_mods.size()) { return false; }
	
	delete m_mods[index];
	m_mods.remove(index);
	
	return true;
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

bool ModCollection::removeMod(const char * name) {
	if(name == NULL || Utilities::stringLength(name) == 0) { return false; }

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
	if(name.isEmpty()) { return false; }
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

void ModCollection::clearMods() {
	for(int i=0;i<m_mods.size();i++) {
		delete m_mods[i];
	}
	m_mods.clear();
}

bool ModCollection::load() {
	return loadFrom(SettingsManager::getInstance()->modListFileName);
}

bool ModCollection::loadFrom(const char * fileName) {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return false; }
	
	char * fileExtension = Utilities::getFileExtension(fileName);
	
	if(fileExtension == NULL) {
		return false;
	}
	else if(Utilities::compareStringsIgnoreCase(fileExtension, "ini") == 0) {
		return loadFromINI(fileName);
	}
	else if(Utilities::compareStringsIgnoreCase(fileExtension, "xml") == 0) {
		return loadFromXML(fileName);
	}
	return false;
}

bool ModCollection::loadFromINI(const char * fileName) {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return false; }

	QString modListPath(fileName);
	
	QFileInfo modListFile(modListPath);
	if(!modListFile.exists()) { return false; }
	
	QFile input(modListPath);
	if(!input.open(QIODevice::ReadOnly | QIODevice::Text)) { return false; }
	
	clearMods();
	
	QString line;
	QByteArray bytes;
	QString temp;
	QString name, id, type, group, con;
	bool addModToCollection = false;
	bool skipReadLine = false;
	
	while(true) {
		if(addModToCollection) {
			Mod * newMod = new Mod(name, QString());
			newMod->setType(type);
			
			ModVersion * newVersion = new ModVersion();
			newVersion->addFile(new ModVersionFile(con, QString("con")));
			newVersion->addFile(new ModVersionFile(group, QString("grp")));
			newMod->addVersion(newVersion);

			addMod(newMod);
			
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

bool ModCollection::loadFromXML(const char * fileName) {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return false; }

	QString modListPath(fileName);
	
	QFileInfo modListFile(modListPath);
	if(!modListFile.exists()) { return false; }
	
	QFile file(modListPath);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) { return false; }
	
	clearMods();

	QXmlStreamReader input(&file);

	QXmlStreamAttributes attributes;
	Mod * newMod = NULL;
	ModVersion * newModVersion = NULL;
	QString name, id;

	// root level loop
	while(true) {
		if(input.atEnd() || input.hasError()) {
			break;
		}

		if(input.readNext() == QXmlStreamReader::StartDocument) {
			continue;
		}

		if(input.isStartElement()) {
			if(QString::compare(input.name().toString(), "mod", Qt::CaseInsensitive) == 0) {
				attributes = input.attributes();

				if(attributes.hasAttribute("name")) {
					name = attributes.value("name").toString();
				}
				else {
					printf("Found mod entry missing required name attribute.\n");
					break;
				}

				if(attributes.hasAttribute("id")) {
					id = attributes.value("id").toString();
				}
				else {
					printf("Mod \"%s\" missing required id attribute.\n", name.toLocal8Bit().data());
					break;
				}

				newMod = new Mod(name, id);

				// root / mod level loop
				while(true) {
					if(input.atEnd() || input.hasError()) {
						break;
					}

					input.readNext();

					if(input.isStartElement()) {
						if(QString::compare(input.name().toString(), "information", Qt::CaseInsensitive) == 0) {
							attributes = input.attributes();

							if(attributes.hasAttribute("type")) {
								newMod->setType(attributes.value("type").toString());
							}
							else {
								printf("Mod \"%s\" missing required type attribute in information node.\n", name.toLocal8Bit().data());
								break;
							}

							if(attributes.hasAttribute("game")) {
								newMod->setGameVersion(attributes.value("game").toString());
							}
							else {
								printf("Mod \"%s\" missing required game version attribute in information node.\n", name.toLocal8Bit().data());
								break;
							}

							if(attributes.hasAttribute("version")) {
								newMod->setLatestVersion(attributes.value("version").toString());
							}

							if(attributes.hasAttribute("release_date")) {
								newMod->setReleaseDate(attributes.value("release_date").toString());
							}
							else {
								printf("Mod \"%s\" missing required release_date attribute in information node.\n", name.toLocal8Bit().data());
								break;
							}

							if(attributes.hasAttribute("website")) {
								newMod->setWebsite(attributes.value("website").toString());
							}

							// root / mod / information level loop
							while(true) {
								if(input.atEnd() || input.hasError()) {
									break;
								}

								input.readNext();

								if(input.isStartElement()) {
									if(QString::compare(input.name().toString(), "team", Qt::CaseInsensitive) == 0) {
										attributes = input.attributes();

										QString teamName, teamEmail;

										if(attributes.hasAttribute("name")) {
											teamName = attributes.value("name").toString();
										}

										if(attributes.hasAttribute("email")) {
											teamEmail = attributes.value("email").toString();
										}

										newMod->setTeam(new ModTeam(teamName, teamEmail));

										// root / mod / information / team level loop
										while(true) {
											if(input.atEnd() || input.hasError()) {
												break;
											}

											input.readNext();

											if(input.isStartElement()) {
												if(QString::compare(input.name().toString(), "member", Qt::CaseInsensitive) == 0) {
													attributes = input.attributes();

													QString memberName, memberAlias, memberEmail;

													if(attributes.hasAttribute("name")) {
														memberName = attributes.value("name").toString();
													}
													else {
														memberName = QString("Unknown");
													}

													if(attributes.hasAttribute("alias")) {
														memberAlias = attributes.value("alias").toString();
													}

													if(attributes.hasAttribute("email")) {
														memberEmail = attributes.value("email").toString();
													}

													newMod->addTeamMember(new ModTeamMember(memberName, memberAlias, memberEmail));
												}
											}
											else if(input.isEndElement()) {
												if(QString::compare(input.name().toString(), "team", Qt::CaseInsensitive) == 0) {
													break;
												}
											}
										}
									}
								}
								else if(input.isEndElement()) {
									if(QString::compare(input.name().toString(), "information", Qt::CaseInsensitive) == 0) {
										break;
									}
								}
							}
						}
						else if(QString::compare(input.name().toString(), "files", Qt::CaseInsensitive) == 0) {
							// root / mod / files level loop
							while(true) {
								if(input.atEnd() || input.hasError()) {
									break;
								}

								input.readNext();

								if(input.isStartElement()) {
									if(QString::compare(input.name().toString(), "version", Qt::CaseInsensitive) == 0) {
										attributes = input.attributes();
										
										newModVersion = new ModVersion(attributes.hasAttribute("id") ? attributes.value("id").toString() : NULL);

										// root / mod / files / version level loop
										while(true) {
											if(input.atEnd() || input.hasError()) {
												break;
											}

											input.readNext();

											if(input.isStartElement()) {
												if(QString::compare(input.name().toString(), "file", Qt::CaseInsensitive) == 0) {
													attributes = input.attributes();
													
													QString fileName, fileType;

													if(attributes.hasAttribute("name")) {
														fileName = attributes.value("name").toString();
													}
													else {
														printf("Mod \"%s\" missing required name attribute for a version / file node.\n", name.toLocal8Bit().data());
														break;
													}

													if(attributes.hasAttribute("type")) {
														fileType = attributes.value("type").toString();
													}
													else {
														QString fileExtension = Utilities::getFileExtension(fileName);

														if(QString::compare(fileExtension, "con", Qt::CaseInsensitive) == 0) {
															fileType = "con";
														}
														else if(QString::compare(fileExtension, "grp", Qt::CaseInsensitive) == 0) {
															fileType = "grp";
														}
														else {
															printf("Unhandled or unknown file type for version / file node with unspecified type attribute for mod \"%s\".\n", name.toLocal8Bit().data());
															break;
														}
													}

													newModVersion->addFile(new ModVersionFile(fileName, fileType));
												}
											}
											else if(input.isEndElement()) {
												if(QString::compare(input.name().toString(), "version", Qt::CaseInsensitive) == 0) {
													newMod->addVersion(newModVersion);

													newModVersion = NULL;

													break;
												}
											}
										}
									}
								}
								else if(input.isEndElement()) {
									if(QString::compare(input.name().toString(), "files", Qt::CaseInsensitive) == 0) {
										break;
									}
								}
							}
						}
					}
					else if(input.isEndElement()) {
						if(QString::compare(input.name().toString(), "mod", Qt::CaseInsensitive) == 0) {
							addMod(newMod);

							newModVersion = NULL;
							newMod = NULL;

							name.clear();
							id.clear();

							break;
						}
					}
				}
			}
		}
	}

	file.close();

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
