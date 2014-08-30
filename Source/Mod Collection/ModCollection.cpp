#include "Mod Collection/ModCollection.h"

ModCollection::ModCollection()
	: m_id(NULL)
	, m_name(NULL) {
	m_id = new char[1];
	m_id[0] = '\0';

	m_name = new char[1];
	m_name[0] = '\0';
}

ModCollection::ModCollection(const ModCollection & m)
	: m_id(NULL)
	, m_name(NULL) {
	m_id = Utilities::trimCopy(m.m_id);
	m_name = Utilities::trimCopy(m.m_name);

	for(int i=0;i<m.m_mods.size();i++) {
		m_mods.push_back(new Mod(*m.m_mods[i]));
	}
}

ModCollection & ModCollection::operator = (const ModCollection & m) {
	if(m_id != NULL) { delete [] m_id; }
	if(m_name != NULL) { delete [] m_name; }

	for(int i=0;i<m.m_mods.size();i++) {
		delete m_mods[i];
	}
	m_mods.clear();

	m_id = Utilities::trimCopy(m.m_id);
	m_name = Utilities::trimCopy(m.m_name);

	for(int i=0;i<m.m_mods.size();i++) {
		m_mods.push_back(new Mod(*m.m_mods[i]));
	}

	return *this;
}

ModCollection::~ModCollection() {
	if(m_id != NULL) { delete [] m_id; }
	if(m_name != NULL) { delete [] m_name; }

	for(int i=0;i<m_mods.size();i++) {
		delete m_mods[i];
	}
}

const char * ModCollection::getID() const {
	return m_id;
}

const char * ModCollection::getGameName() const {
	return m_name;
}

void ModCollection::setID(const char * id) {
	if(m_id != NULL) {
		delete [] m_id;
	}
	
	if(id == NULL) {
		m_id = new char[1];
		m_id[0] = '\0';
	}
	else {
		m_id = Utilities::trimCopy(id);
	}
}

void ModCollection::setID(const QString & id) {
	if(m_id != NULL) {
		delete [] m_id;
	}

	if(id.isEmpty()) {
		m_id = new char[1];
		m_id[0] = '\0';
	}
	else {
		QByteArray idBytes = id.toLocal8Bit();
		const char * idData = idBytes.data();
		m_id = Utilities::trimCopy(idData);
	}
}

void ModCollection::setGameName(const char * name) {
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

void ModCollection::setGameName(const QString & name) {
	if(m_name != NULL) {
		delete [] m_name;
	}

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

	notifyCollectionChanged();

	return true;
}

bool ModCollection::removeMod(int index) {
	if(index < 0 || index >= m_mods.size()) { return false; }
	
	delete m_mods[index];
	m_mods.remove(index);

	notifyCollectionChanged();
	
	return true;
}

bool ModCollection::removeMod(const Mod & mod) {
	for(int i=0;i<m_mods.size();i++) {
		if(*m_mods[i] == mod) {
			delete m_mods[i];
			m_mods.remove(i);

			notifyCollectionChanged();
			
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

			notifyCollectionChanged();

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

			notifyCollectionChanged();

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

	notifyCollectionChanged();
}

bool ModCollection::load() {
	return loadFrom(SettingsManager::getInstance()->modListFileName);
}

bool ModCollection::loadFrom(const QString & fileName) {
	if(fileName.isEmpty()) { return false; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	return loadFrom(fileNameData);
}

bool ModCollection::loadFrom(const char * fileName) {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return false; }
	
	char * fileExtension = Utilities::getFileExtension(fileName);

	bool loaded = false;
	
	if(fileExtension == NULL) {
		return false;
	}
	else if(Utilities::compareStringsIgnoreCase(fileExtension, "ini") == 0) {
		loaded = loadFromINI(fileName);
	}
	else if(Utilities::compareStringsIgnoreCase(fileExtension, "xml") == 0) {
		loaded = loadFromXML(fileName);
	}

	delete [] fileExtension;

	return loaded;
}

bool ModCollection::loadFromINI(const QString & fileName) {
	if(fileName.isEmpty()) { return false; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	return loadFromINI(fileNameData);
}

bool ModCollection::loadFromINI(const char * fileName) {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return false; }

	QString modListPath(fileName);
	
	QFileInfo modListFile(modListPath);
	if(!modListFile.exists()) { return false; }
	
	QFile input(modListPath);
	if(!input.open(QIODevice::ReadOnly | QIODevice::Text)) { return false; }
	
	setID(NULL);
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

bool ModCollection::loadFromXML(const QString & fileName) {
	if(fileName.isEmpty()) { return false; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	return loadFromXML(fileNameData);
}

bool ModCollection::loadFromXML(const char * fileName) {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return false; }

	QString modListPath(fileName);
	
	QFileInfo modListFile(modListPath);
	if(!modListFile.exists()) { return false; }
	
	QFile file(modListPath);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) { return false; }
	
	setID(NULL);
	clearMods();

	QXmlStreamReader input(&file);

	QXmlStreamAttributes attributes;
	Mod * newMod = NULL;
	ModVersion * newModVersion = NULL;
	ModTeamMember * newTeamMember = NULL;
	ModVersionFile * newVersionFile = NULL;
	QString name, id;
	bool foundRootNode = false;

	// root level loop
	while(true) {
		if(input.atEnd() || input.hasError()) {
			break;
		}

		if(input.readNext() == QXmlStreamReader::StartDocument) {
			continue;
		}

		if(input.isStartElement()) {
			if(!foundRootNode) {
				if(QString::compare(input.name().toString(), "mods", Qt::CaseInsensitive) == 0) {
					foundRootNode = true;

					attributes = input.attributes();

					QString game, id, fileVersion;

					if(attributes.hasAttribute("game")) {
						setGameName(attributes.value("game").toString());
					}
					else {
						printf("Root mods node missing required game attribute.\n");
						break;
					}

					if(attributes.hasAttribute("id")) {
						setID(attributes.value("id").toString());
					}
					else {
						printf("Root mods node missing required id attribute.\n");
						break;
					}

					if(attributes.hasAttribute("mods_version")) {
						fileVersion = attributes.value("mods_version").toString();
						if(QString::compare(fileVersion, "1.0", Qt::CaseInsensitive) != 0) {
							printf("Mod list version %s is unsupported.\n", fileVersion.toLocal8Bit().data());
							break;
						}
					}
					else {
						printf("Root mods node missing required mods_version attribute.\n");
						break;
					}
				}
				
				continue;
			}

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

										ModTeam * newTeam = new ModTeam(teamName, teamEmail);
										newMod->setTeam(newTeam);

										if(attributes.hasAttribute("city")) {
											newTeam->setCity(attributes.value("city").toString());
										}

										if(attributes.hasAttribute("state")) {
											newTeam->setState(attributes.value("state").toString());
										}
										else if(attributes.hasAttribute("province")) {
											newTeam->setState(attributes.value("province").toString());
										}

										if(attributes.hasAttribute("country")) {
											newTeam->setCountry(attributes.value("country").toString());
										}

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

													newTeamMember = new ModTeamMember(memberName, memberAlias, memberEmail);
													if(!newMod->addTeamMember(newTeamMember)) {
														if(newMod->getTeam() == NULL) {
															printf("Attempted to add team member to mod with no team: \"%s\".\n", newMod->getName());
														}
														else {
															if(Utilities::stringLength(newTeamMember->getName()) == 0) {
																printf("Attempted to add team member with empty name to mod: \"%s\".\n", newMod->getName());
															}
															else {
																printf("Attempted to add duplicate team member \"%s\" to mod: \"%s\"\n", newTeamMember->getName(), newMod->getName());
															}
														}

														delete newTeamMember;

														break;
													}
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

													newVersionFile = new ModVersionFile(fileName, fileType);
													if(!newModVersion->addFile(newVersionFile)) {
														if(Utilities::stringLength(newVersionFile->getName()) == 0) {
															printf("Attempted to add version file with empty file name to mod: \"%s\".\n", newMod->getName());
														}
														else {
															printf("Attempted to add duplicate version file \"%s\" to mod: \"%s%s%s\"\n", newVersionFile->getName(), newMod->getName(), newModVersion->getVersion() == NULL ? "" : " ", newModVersion->getVersion() == NULL ? "" : newModVersion->getVersion());
														}

														delete newVersionFile;

														break;
													}
												}
											}
											else if(input.isEndElement()) {
												if(QString::compare(input.name().toString(), "version", Qt::CaseInsensitive) == 0) {
													if(!newMod->addVersion(newModVersion)) {
														if(!newModVersion->hasFileOfType("grp")) {
															printf("Attempted to add mod version %s%s%s without required group file to mod: \"%s\"\n", newModVersion->getVersion() == NULL ? "" : "\"", newModVersion->getVersion() == NULL ? "" : newModVersion->getVersion(), newModVersion->getVersion() == NULL ? "" : "\"", newMod->getName());
														}
														else {
															printf("Attempted to add duplicate mod version %s%s%s to mod: \"%s\"\n", newModVersion->getVersion() == NULL ? "" : "\"", newModVersion->getVersion() == NULL ? "" : newModVersion->getVersion(), newModVersion->getVersion() == NULL ? "" : "\"", newMod->getName());
														}

														delete newModVersion;
													}

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
						else if(QString::compare(input.name().toString(), "downloads", Qt::CaseInsensitive) == 0) {
							// root / mod / downloads level loop
							while(true) {
								if(input.atEnd() || input.hasError()) {
									break;
								}

								input.readNext();

								if(input.isStartElement()) {
									if(QString::compare(input.name().toString(), "download", Qt::CaseInsensitive) == 0) {
										attributes = input.attributes();

										QString downloadName, filePart, fileNumberOfParts, fileVersion, fileSubfolder, downloadType, fileDescription;
										ModDownload * newDownload = NULL;

										if(attributes.hasAttribute("filename")) {
											downloadName = attributes.value("filename").toString();
										}
										else {
											printf("Mod \"%s\" missing required filename attribute for a download node.\n", name.toLocal8Bit().data());
											break;
										}

										if(attributes.hasAttribute("type")) {
											downloadType = attributes.value("type").toString();
										}
										else {
											printf("Mod \"%s\" missing required type attribute for a download node.\n", name.toLocal8Bit().data());
											break;
										}

										newDownload = new ModDownload(downloadName, downloadType);
										if(!newMod->addDownload(newDownload)) {
											if(Utilities::stringLength(newDownload->getFileName()) == 0) {
												printf("Attempted to add download with empty file name to mod: \"%s\"\n", newMod->getName());
											}
											else if(Utilities::stringLength(newDownload->getType()) == 0) {
												printf("Attempted to add download with empty type to mod: \"%s\"\n", newMod->getName());
											}
											else {
												printf("Attempted to add duplicate download \"%s\" to mod: \"%s\"\n", newDownload->getFileName(), newMod->getName());
											}

											delete newDownload;

											break;
										}

										if(attributes.hasAttribute("part")) {
											newDownload->setPartNumber(attributes.value("part").toString());
										}

										if(attributes.hasAttribute("numparts")) {
											newDownload->setPartCount(attributes.value("numparts").toString());
										}

										if(attributes.hasAttribute("version")) {
											newDownload->setVersion(attributes.value("version").toString());
										}

										if(attributes.hasAttribute("special")) {
											newDownload->setVersion(attributes.value("special").toString());
										}

										if(attributes.hasAttribute("subfolder")) {
											newDownload->setSubfolder(attributes.value("subfolder").toString());
										}

										if(attributes.hasAttribute("description")) {
											newDownload->setDescription(attributes.value("description").toString());
										}
									}
								}
								else if(input.isEndElement()) {
									if(QString::compare(input.name().toString(), "downloads", Qt::CaseInsensitive) == 0) {
										break;
									}
								}
							}
						}
						else if(QString::compare(input.name().toString(), "screenshots", Qt::CaseInsensitive) == 0) {
							// root / mod / screenshots level loop
							while(true) {
								if(input.atEnd() || input.hasError()) {
									break;
								}

								input.readNext();

								if(input.isStartElement()) {
									if(QString::compare(input.name().toString(), "screenshot", Qt::CaseInsensitive) == 0) {
										attributes = input.attributes();

										QString fileName;
										ModScreenshot * newScreenshot = NULL;

										if(attributes.hasAttribute("filename")) {
											fileName = attributes.value("filename").toString();
										}
										else {
											printf("Mod \"%s\" missing required filename attribute for a download node.\n", name.toLocal8Bit().data());
											break;
										}

										newScreenshot = new ModScreenshot(fileName);
										if(!newMod->addScreenshot(newScreenshot)) {
											if(Utilities::stringLength(newScreenshot->getFileName()) == 0) {
												printf("Attempted to add screenshot with empty file name to mod: \"%s\"\n", newMod->getName());
											}
											else {
												printf("Attempted to add duplicate screenshot \"%s\" to mod: \"%s\"\n", newScreenshot->getFileName(), newMod->getName());
											}

											delete newScreenshot;

											break;
										}

										if(attributes.hasAttribute("thumbnail")) {
											newScreenshot->setThumbnail(attributes.value("thumbnail").toString());
										}

										if(attributes.hasAttribute("width")) {
											newScreenshot->setWidth(attributes.value("width").toString());
										}

										if(attributes.hasAttribute("height")) {
											newScreenshot->setHeight(attributes.value("height").toString());
										}
									}
								}
								else if(input.isEndElement()) {
									if(QString::compare(input.name().toString(), "screenshots", Qt::CaseInsensitive) == 0) {
										break;
									}
								}
							}
						}
					}
					else if(input.isEndElement()) {
						if(QString::compare(input.name().toString(), "mod", Qt::CaseInsensitive) == 0) {
							if(!addMod(newMod)) {
								printf("Failed to add mod to collection: \"%s\"\n", newMod->getName());

								delete newMod;
							}

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
		else if(input.isEndElement()) {
			if(QString::compare(input.name().toString(), "mods", Qt::CaseInsensitive) == 0) {
				break;
			}
		}
	}

	file.close();

	return true;
}

void ModCollection::notifyCollectionChanged() const {
	for(int i=0;i<numberOfListeners();i++) {
		getListener(i)->modCollectionUpdated();
	}
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
