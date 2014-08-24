#include "Mod Manager/ModManager.h"

ModManager::ModManager()
	: m_initialized(false)
	, m_mode(ModManagerModes::defaultMode)
	, m_gameType(GameTypes::defaultGameType)
	, m_selectedMod(NULL)
	, m_selectedModVersionIndex(0) {
	
}

ModManager::~ModManager() {
	if(m_initialized) {
		SettingsManager::getInstance()->modManagerMode = m_mode;
		SettingsManager::getInstance()->gameType = m_gameType;
	}

	for(int i=0;i<m_favourites.size();i++) {
		delete m_favourites[i];
	}
}

bool ModManager::init(const ArgumentParser * args) {
	if(m_initialized) { return true; }

	m_mode = SettingsManager::getInstance()->modManagerMode;
	m_gameType = SettingsManager::getInstance()->gameType;

	if(!m_mods.load()) {
		printf("Failed to load mod list!\n");
		return false;
	}

	if(m_mods.numberOfMods() == 0) {
		printf("No mods loaded!\n");
		return false;
	}
	printf("%d mods loaded.\n", m_mods.numberOfMods());

	m_organizedMods.setModCollection(&m_mods);

	m_initialized = true;

	loadFavourites();

	checkForMissingExecutables();

	checkAllModsForMissingFiles();

	checkForUnlinkedModFiles();

	printf("\n");

	handleArguments(args);

	return true;
}

bool ModManager::uninit() {
	if(!m_initialized) { return false; }

	saveFavourites();

	m_organizedMods.setModCollection(NULL);
	m_selectedMod = NULL;
	m_favourites.clear();
	m_mods.clearMods();
	m_scriptArgs.clear();

	m_initialized = false;

	return true;
}

void ModManager::run() {
	if(!m_initialized) { return; }

	runMenu();
}

ModManagerModes::ModManagerMode ModManager::getMode() const {
	return m_mode;
}

const char * ModManager::getModeName() const {
	return ModManagerModes::toString(m_mode);
}

bool ModManager::setMode(const char * modeName) {
	if(modeName == NULL) { return false; }

	return setMode(ModManagerModes::parseFrom(modeName));
}

bool ModManager::setMode(const QString & modeName) {
	return setMode(ModManagerModes::parseFrom(modeName));
}

bool ModManager::setMode(int mode) {
	if(!ModManagerModes::isValid(mode)) { return false; }

	m_mode = static_cast<ModManagerModes::ModManagerMode>(mode);

	return true;
}

bool ModManager::setMode(ModManagerModes::ModManagerMode mode) {
	if(!ModManagerModes::isValid(mode)) { return false; }

	m_mode = mode;

	return true;
}

GameTypes::GameType ModManager::getGameType() const {
	return m_gameType;
}

const char * ModManager::getGameTypeName() const {
	return GameTypes::toString(m_mode);
}

bool ModManager::setGameType(const char * gameTypeName) {
	if(gameTypeName == NULL) { return false; }

	return setGameType(GameTypes::parseFrom(gameTypeName));
}

bool ModManager::setGameType(const QString & gameTypeName) {
	return setGameType(GameTypes::parseFrom(gameTypeName));
}

bool ModManager::setGameType(int gameType) {
	if(!GameTypes::isValid(gameType)) { return false; }

	m_gameType = static_cast<GameTypes::GameType>(gameType);

	return true;
}

bool ModManager::setGameType(GameTypes::GameType gameType) {
	if(!GameTypes::isValid(gameType)) { return false; }

	m_gameType = gameType;

	return true;
}

const char * ModManager::getServerIPAddress() const {
	return SettingsManager::getInstance()->serverIPAddress;
}

void ModManager::setServerIPAddress(const char * ipAddress) {
	if(SettingsManager::getInstance()->serverIPAddress != NULL) {
		delete [] SettingsManager::getInstance()->serverIPAddress;
	}
	
	if(ipAddress == NULL || Utilities::stringLength(ipAddress) == 0) {
		SettingsManager::getInstance()->serverIPAddress = new char[1];
		SettingsManager::getInstance()->serverIPAddress[0] = '\0';
	}
	else {
		SettingsManager::getInstance()->serverIPAddress = Utilities::trimCopy(ipAddress);
	}
}

void ModManager::setServerIPAddress(const QString & ipAddress) {
	if(SettingsManager::getInstance()->serverIPAddress != NULL) {
		delete [] SettingsManager::getInstance()->serverIPAddress;
	}

	if(ipAddress.length() == 0) {
		SettingsManager::getInstance()->serverIPAddress = new char[1];
		SettingsManager::getInstance()->serverIPAddress[0] = '\0';
	}
	else {
		QByteArray ipAddressBytes = ipAddress.toLocal8Bit();
		const char * ipAddressData = ipAddressBytes.data();

		SettingsManager::getInstance()->serverIPAddress = Utilities::trimCopy(ipAddressData);
	}
}

const Mod * ModManager::getSelectedMod() const {
	return m_selectedMod;
}

const char * ModManager::getSelectedModName() const {
	return m_selectedMod == NULL ? NULL : m_selectedMod->getName();
}

bool ModManager::setSelectedMod(const char * name) {
	const Mod * selectedMod = m_mods.getMod(name);

	if(selectedMod != NULL) {
		m_selectedMod = selectedMod;
		return true;
	}
	return false;
}

bool ModManager::setSelectedMod(const QString & name) {
	const Mod * selectedMod = m_mods.getMod(name);

	if(selectedMod != NULL) {
		m_selectedMod = selectedMod;
		return true;
	}
	return false;
}

void ModManager::selectRandomMod() {
// TODO: account for category filter
	if(!m_initialized) { return; }

	m_selectedMod = m_mods.getMod(Utilities::randomInteger(0, m_mods.numberOfMods() - 1));
}

int ModManager::searchForAndSelectMod(const char * query) {
// TODO: account for category filter
	if(!m_initialized || query == NULL || Utilities::stringLength(query) == 0) { return -1; }

	return searchForAndSelectMod(QString(query));
}

int ModManager::searchForAndSelectMod(const QString & query) {
// TODO: account for category filter
	if(!m_initialized) { return -1; }

	QString data = query.trimmed().toLower();
	if(data.length() == 0) { return -1; }

	QString modName;
	const Mod * matchingMod = NULL;
	int numberOfMatches = 0;

	for(int i=0;i<m_mods.numberOfMods();i++) {
		modName = QString(m_mods.getMod(i)->getName()).toLower();

		if(QString::compare(modName, data, Qt::CaseSensitive) == 0) {
			matchingMod = m_mods.getMod(i);
			numberOfMatches = 1;
			break;
		}

		if(modName.contains(data)) {
			matchingMod = m_mods.getMod(i);
			numberOfMatches++;
		}
	}

	if(numberOfMatches) {
		m_selectedMod = matchingMod;
	}

	return numberOfMatches;
}

void ModManager::clearSelectedMod() {
	m_selectedMod = NULL;
}

int ModManager::numberOfFavourites() {
	return m_favourites.size();
}

bool ModManager::hasFavourite(const ModInformation & favourite) const {
	for(int i=0;i<m_favourites.size();i++) {
		if(*m_favourites[i] == favourite) {
			return true;
		}
	}
	return false;
}

bool ModManager::hasFavourite(const char * name) const {
	if(name == NULL || Utilities::stringLength(name) == 0) { return false; }

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), name) == 0) {
			return true;
		}
	}
	return false;
}

bool ModManager::hasFavourite(const QString & name) const {
	if(name.isEmpty()) { return false; }
	QByteArray nameBytes = name.toLocal8Bit();
	const char * nameData = nameBytes.data();

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), nameData) == 0) {
			return true;
		}
	}
	return false;
}

bool ModManager::hasFavourite(const char * name, const char * version) const {
	if(name == NULL || Utilities::stringLength(name) == 0) { return false; }

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), name) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_favourites[i]->getVersion(), version) == 0) {
			return true;
		}
	}
	return false;
}

bool ModManager::hasFavourite(const QString & name, const QString & version) const {
	if(name.isEmpty()) { return false; }
	QByteArray nameBytes = name.toLocal8Bit();
	const char * nameData = nameBytes.data();

	QByteArray versionBytes = version.toLocal8Bit();
	const char * versionData = version.isNull() ? NULL : versionBytes.data();

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), nameData) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_favourites[i]->getVersion(), versionData) == 0) {
			return true;
		}
	}
	return false;
}

int ModManager::indexOfFavourite(const ModInformation & favourite) const {
	for(int i=0;i<m_favourites.size();i++) {
		if(*m_favourites[i] == favourite) {
			return i;
		}
	}
	return -1;
}

int ModManager::indexOfFavourite(const char * name) const {
	if(name == NULL || Utilities::stringLength(name) == 0) { return -1; }

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), name) == 0) {
			return i;
		}
	}
	return -1;
}

int ModManager::indexOfFavourite(const QString & name) const {
	if(name.isEmpty()) { return -1; }
	QByteArray nameBytes = name.toLocal8Bit();
	const char * nameData = nameBytes.data();

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), nameData) == 0) {
			return i;
		}
	}
	return -1;
}

int ModManager::indexOfFavourite(const char * name, const char * version) const {
	if(name == NULL || Utilities::stringLength(name) == 0) { return -1; }

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), name) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_favourites[i]->getVersion(), version) == 0) {
			return i;
		}
	}
	return -1;
}

int ModManager::indexOfFavourite(const QString & name, const QString & version) const {
	if(name.isEmpty()) { return -1; }
	QByteArray nameBytes = name.toLocal8Bit();
	const char * nameData = nameBytes.data();

	QByteArray versionBytes = version.toLocal8Bit();
	const char * versionData = version.isNull() ? NULL : versionBytes.data();

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), nameData) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_favourites[i]->getVersion(), versionData) == 0) {
			return i;
		}
	}
	return -1;
}

const ModInformation * ModManager::getFavourite(int index) const {
	if(index < 0 || index >= m_favourites.size()) { return NULL; }

	return m_favourites[index];
}

const ModInformation * ModManager::getFavourite(const char * name) const {
	if(name == NULL || Utilities::stringLength(name) == 0) { return NULL; }

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), name) == 0) {
			return m_favourites[i];
		}
	}
	return NULL;
}

const ModInformation * ModManager::getFavourite(const QString & name) const {
	if(name.isEmpty()) { return NULL; }
	QByteArray nameBytes = name.toLocal8Bit();
	const char * nameData = nameBytes.data();

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), nameData) == 0) {
			return m_favourites[i];
		}
	}
	return NULL;
}

const ModInformation * ModManager::getFavourite(const char * name, const char * version) const {
	if(name == NULL || Utilities::stringLength(name) == 0) { return NULL; }

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), name) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_favourites[i]->getVersion(), version) == 0) {
			return m_favourites[i];
		}
	}
	return NULL;
}

const ModInformation * ModManager::getFavourite(const QString & name, const QString & version) const {
	if(name.isEmpty()) { return NULL; }
	QByteArray nameBytes = name.toLocal8Bit();
	const char * nameData = nameBytes.data();

	QByteArray versionBytes = version.toLocal8Bit();
	const char * versionData = version.isNull() ? NULL : versionBytes.data();

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), nameData) == 0,
		   Utilities::compareStringsIgnoreCase(m_favourites[i]->getVersion(), versionData) == 0) {
			return m_favourites[i];
		}
	}
	return NULL;
}

bool ModManager::addFavourite(ModInformation * favourite) {
	if(favourite == NULL || Utilities::stringLength(favourite->getName()) == 0 || hasFavourite(*favourite)) {
		return false;
	}
	
	m_favourites.push_back(favourite);

	return true;
}

bool ModManager::removeFavourite(int index) {
	if(index < 0 || index >= m_favourites.size()) { return false; }
	
	delete m_favourites[index];
	m_favourites.remove(index);
	
	return true;
}

bool ModManager::removeFavourite(const ModInformation & favourite) {
	for(int i=0;i<m_favourites.size();i++) {
		if(*m_favourites[i] == favourite) {
			delete m_favourites[i];
			m_favourites.remove(i);
			
			return true;
		}
	}
	return false;
}

bool ModManager::removeFavourite(const char * name) {
	if(name == NULL || Utilities::stringLength(name) == 0) { return false; }

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), name) == 0) {
			delete m_favourites[i];
			m_favourites.remove(i);

			return true;
		}
	}
	return false;
}

bool ModManager::removeFavourite(const QString & name) {
	if(name.isEmpty()) { return false; }
	QByteArray nameBytes = name.toLocal8Bit();
	const char * nameData = nameBytes.data();

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), nameData) == 0) {
			delete m_favourites[i];
			m_favourites.remove(i);

			return true;
		}
	}
	return false;
}

bool ModManager::removeFavourite(const char * name, const char * version) {
	if(name == NULL || Utilities::stringLength(name) == 0) { return false; }

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), name) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_favourites[i]->getVersion(), version) == 0) {
			delete m_favourites[i];
			m_favourites.remove(i);

			return true;
		}
	}
	return false;
}

bool ModManager::removeFavourite(const QString & name, const QString & version) {
	if(name.isEmpty()) { return false; }
	QByteArray nameBytes = name.toLocal8Bit();
	const char * nameData = nameBytes.data();

	QByteArray versionBytes = version.toLocal8Bit();
	const char * versionData = version.isNull() ? NULL : versionBytes.data();

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), nameData) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_favourites[i]->getVersion(), versionData) == 0) {
			delete m_favourites[i];
			m_favourites.remove(i);

			return true;
		}
	}
	return false;
}

void ModManager::clearFavourites() {
	for(int i=0;i<m_favourites.size();i++) {
		delete m_favourites[i];
	}
	m_favourites.clear();
}

bool ModManager::loadFavourites() {
	return loadFavourites(SettingsManager::getInstance()->favouritesListFileName);
}

bool ModManager::loadFavourites(const QString & fileName) {
	if(fileName.isEmpty()) { return false; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	return loadFavourites(fileNameData);
}

bool ModManager::loadFavourites(const char * fileName) {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return false; }
	
	char * fileExtension = Utilities::getFileExtension(fileName);
	
	if(fileExtension == NULL) {
		return false;
	}
	else if(Utilities::compareStringsIgnoreCase(fileExtension, "txt") == 0 || Utilities::compareStringsIgnoreCase(fileExtension, "ini") == 0) {
		return loadFavouritesList(fileName);
	}
	else if(Utilities::compareStringsIgnoreCase(fileExtension, "xml") == 0) {
		return loadFavouritesXML(fileName);
	}
	return false;
}

bool ModManager::loadFavouritesList(const QString & fileName) {
	if(fileName.isEmpty()) { return false; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	return loadFavouritesList(fileNameData);
}

bool ModManager::loadFavouritesList(const char * fileName) {
	if(!m_initialized) { return false; }

	QString favouritesListPath(fileName);
	
	QFileInfo favouritesListFileInfo(favouritesListPath);
	if(!favouritesListFileInfo.exists()) { return false; }
	
	QFile input(favouritesListPath);
	if(!input.open(QIODevice::ReadOnly | QIODevice::Text)) { return false; }

	clearFavourites();

	QString line;

	while(true) {
		if(input.atEnd()) { break; }
		
		line = input.readLine().trimmed();

		if(line.length() == 0) { continue; }

		const Mod * mod = m_mods.getMod(line);

		if(mod != NULL) {
			addFavourite(new ModInformation(mod->getName(), mod->getLatestVersion()));
		}
		else {
			QByteArray lineBytes = line.toLocal8Bit();
			const char * lineData = lineBytes.data();

			printf("Unknown or missing mod in favourites list: %s\n", lineData);
		}
	}

	if(m_favourites.size() > 0) {
		printf("%d favourite mod%s loaded.\n", m_favourites.size(), m_favourites.size() == 1 ? "" : "s");
	}

	input.close();

	return true;
}

bool ModManager::loadFavouritesXML(const QString & fileName) {
	if(fileName.isEmpty()) { return false; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	return loadFavouritesXML(fileNameData);
}

bool ModManager::loadFavouritesXML(const char * fileName) {
	if(!m_initialized) { return false; }

	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return false; }

	QString favouritesListPath(fileName);
	
	QFileInfo favouritesListFileInfo(favouritesListPath);
	if(!favouritesListFileInfo.exists()) { return false; }
	
	QFile favouritesListFile(favouritesListPath);
	if(!favouritesListFile.open(QIODevice::ReadOnly | QIODevice::Text)) { return false; }
	
	clearFavourites();

	QXmlStreamReader input(&favouritesListFile);

	QXmlStreamAttributes attributes;
	ModInformation * newModInfo = NULL;
	QString name, version;
	bool foundRootNode = false;

	while(true) {
		if(input.atEnd() || input.hasError()) {
			break;
		}

		if(input.readNext() == QXmlStreamReader::StartDocument) {
			continue;
		}

		if(input.isStartElement()) {
			if(!foundRootNode) {
				if(QString::compare(input.name().toString(), "favourites", Qt::CaseInsensitive) == 0) {
					foundRootNode = true;

					attributes = input.attributes();

					QString game, fileVersion;

					if(attributes.hasAttribute("game")) {
						game = attributes.value("game").toString();
						if(QString::compare(game, "Duke Nukem 3D", Qt::CaseInsensitive) != 0) {
							printf("Specified mod list is for an unsupported game: \"%s\".\n", game.toLocal8Bit().data());
							break;
						}
					}
					else {
						printf("Root mods node missing required game attribute.\n");
						break;
					}

					if(attributes.hasAttribute("favourites_version")) {
						fileVersion = attributes.value("favourites_version").toString();
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
					printf("Found favourite mod entry missing required name attribute.\n");
					break;
				}

				if(attributes.hasAttribute("version")) {
					version = attributes.value("version").toString();
				}
			}

			if(!name.isEmpty()) {
				addFavourite(new ModInformation(name, version));
			}

			name.clear();
			version.clear();
		}
		else if(input.isEndElement()) {
			if(QString::compare(input.name().toString(), "favourites", Qt::CaseInsensitive) == 0) {
				break;
			}
		}
	}

	favouritesListFile.close();

	return true;
}

bool ModManager::saveFavourites() {
	return saveFavourites(SettingsManager::getInstance()->favouritesListFileName);
}

bool ModManager::saveFavourites(const QString & fileName) {
	if(fileName.isEmpty()) { return false; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	return saveFavourites(fileNameData);
}

bool ModManager::saveFavourites(const char * fileName) {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return false; }
	
	char * fileExtension = Utilities::getFileExtension(fileName);
	
	if(fileExtension == NULL) {
		return false;
	}
	else if(Utilities::compareStringsIgnoreCase(fileExtension, "txt") == 0 || Utilities::compareStringsIgnoreCase(fileExtension, "ini") == 0) {
		return saveFavouritesList(fileName);
	}
	else if(Utilities::compareStringsIgnoreCase(fileExtension, "xml") == 0) {
		return saveFavouritesXML(fileName);
	}
	return false;
}

bool ModManager::saveFavouritesList(const QString & fileName) {
	if(fileName.isEmpty()) { return false; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	return saveFavouritesList(fileNameData);
}

bool ModManager::saveFavouritesList(const char * fileName) {
	if(!m_initialized) { return false; }

	if(m_favourites.size() == 0) { return true; }

	QString favouritesListPath(fileName);
	
	QFile output(favouritesListPath);
	if(!output.open(QIODevice::WriteOnly)) { return false; }
	
	for(int i=0;i<m_favourites.size();i++) {
		output.write(m_favourites[i]->getName(), Utilities::stringLength(m_favourites[i]->getName()));
		output.write(Utilities::newLine, Utilities::stringLength(Utilities::newLine));
	}
	
	output.close();
	
	return true;
}

bool ModManager::saveFavouritesXML(const QString & fileName) {
	if(fileName.isEmpty()) { return false; }
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	return saveFavouritesList(fileNameData);
}

bool ModManager::saveFavouritesXML(const char * fileName) {
	if(!m_initialized) { return false; }

	if(m_favourites.size() == 0) { return true; }

	QString favouritesListPath(fileName);
	
	QFile favouritesListFile(favouritesListPath);
	if(!favouritesListFile.open(QIODevice::WriteOnly)) { return false; }

	QXmlStreamWriter output(&favouritesListFile);

	output.setAutoFormatting(true);

	output.writeStartDocument();

	output.writeStartElement("favourites");

	output.writeAttribute("game", "Duke Nukem 3D");
	output.writeAttribute("id", "duke_nukem_3d");
	output.writeAttribute("favourites_version", "1.0");

	for(int i=0;i<m_favourites.size();i++) {
		output.writeStartElement("mod");

		output.writeAttribute("name", m_favourites[i]->getName());
		if(m_favourites[i]->getVersion() != NULL) {
			output.writeAttribute("version", m_favourites[i]->getVersion());
		}

		output.writeEndElement();
	}

	output.writeEndElement();

	output.writeEndDocument();

	favouritesListFile.close();

	return true;
}

void ModManager::runGameTypePrompt() {
	// TODO: finish runGameTypePrompt
}

void ModManager::runModePrompt() {
	// TODO: finish runModePrompt
}

void ModManager::runIPAddressPrompt() {
	bool valid = false;
	QTextStream in(stdin);
	QString input, data;
	do {
		printf("Enter host IP Address:\n");
		printf("> ");

		input = in.readLine();
		data = input.trimmed();

		valid = true;

		if(data.length() == 0) { valid = false; }

		for(int i=0;i<data.length();i++) {
			if(data[i] == ' ' || data[i] == '\t') {
				valid = false;
			}
		}

		if(valid) {
			QByteArray dataBytes = data.toLocal8Bit();
			const char * rawData = dataBytes.data();

			if(SettingsManager::getInstance()->serverIPAddress != NULL) { delete [] SettingsManager::getInstance()->serverIPAddress; }
			SettingsManager::getInstance()->serverIPAddress = new char[Utilities::stringLength(rawData) + 1];
			Utilities::copyString(SettingsManager::getInstance()->serverIPAddress, Utilities::stringLength(rawData) + 1, rawData);

			printf("\nHost IP Address changed to: %s\n\n", rawData);

			Utilities::pause();

			break;
		}
		else {
			printf("Invalid IP Address!\n\n");
			Utilities::pause();
		}
	} while(true);
}

void ModManager::runMenu() {
	if(!m_initialized) { return; }

	int selectedIndex = -1;
	bool valid = false;
	QTextStream in(stdin);
	QString input, data;

	while(true) {
		for(int i=0;i<m_mods.numberOfMods();i++) {
			printf("%d: %s\n", (i + 1), m_mods.getMod(i)->getName());
		}
		printf("> ");

		input = in.readLine();
		data = input.trimmed().toLower();

		if(QRegExp("^(s(earch)?)$").exactMatch(data)) {
			printf("\n");
			runSearchPrompt();
			Utilities::pause();
			break;
		}
		else if(QRegExp("^(r(andom)?)$").exactMatch(data)) {
// TODO: add confirmation loop
			selectRandomMod();
			printf("\nRandomly selected mod: %s\n\n", m_selectedMod->getName());
			Utilities::pause();
			break;
		}
		else if(QRegExp("^(ip?|c(onnect)?)$").exactMatch(data)) {
			runIPAddressPrompt();
		}
		else if(QRegExp("^(g(ame)?|t(type)?)$").exactMatch(data)) {
			runGameTypePrompt();
		}
		else if(QRegExp("^(m(ode)?)$").exactMatch(data)) {
			runModePrompt();
		}
		else if(QRegExp("^(\\?|h(elp)?|commands)$").exactMatch(data)) {
			printf("\nCommand information:\n");
			printf("- query / q / search / s: Open a prompt to search for a mod.\n");
			printf("-             random / r: Randomly select a mod from the list.\n");
			printf("-   connect / c / ip / i: Run input prompt for host IP Address.\n");
			printf("-    type / t / game / g: Run input prompt for game type.\n");
			printf("-               mode / m: Run input prompt for mode.\n");
			printf("-           ? / help / h: Display command information.\n");
			printf("-        exit / quit / x: Close the program.\n\n");
			Utilities::pause();
			continue;
		}
		else if(QRegExp("^(x|exit|quit)$").exactMatch(data)) {
			printf("\nGoodbye!\n");
			return;
		}

		selectedIndex = input.toInt(&valid, 10);

		valid = valid && selectedIndex >= 1 && selectedIndex <= m_mods.numberOfMods();

		if(valid) {
			m_selectedMod = m_mods.getMod(selectedIndex - 1);
			break;
		}
		else {
			printf("Invalid selection!\n\n");
			Utilities::pause();
		}
	}

	printf("\n");

	runSelectedMod();
}

void ModManager::runSearchPrompt() {
	if(!m_initialized) { return; }

	QTextStream in(stdin);
	QString input, data;

	while(true) {
		printf("Enter search query:\n");
		printf("> ");

		input = in.readLine();
		data = input.trimmed();

		int numberOfMatches = searchForAndSelectMod(data);

		if(numberOfMatches == -1) {
			printf("\nInvalid or empty search query.\n\n");
		}
		else if(numberOfMatches == 0) {
			printf("\nNo matches found.\n\n");
		}
		else if(numberOfMatches == 1) {
			printf("\nSelected mod: %s\n\n", m_selectedMod->getName());
			break;
		}
		else {
			printf("\n%d matches found, please refine your search query.\n\n", numberOfMatches);
		}
	}
}

bool ModManager::runModVersionSelectionPrompt() {
	if(!m_initialized || m_selectedMod == NULL || m_selectedMod->numberOfVersions() == 0) {
		return false;
	}
	
	m_selectedModVersionIndex = 0;

	if(m_selectedMod->numberOfVersions() == 1) {
		return true;
	}

	int selectedModVersion = -1;
	bool valid = false;
	QTextStream in(stdin);
	QString input, data;

	while(true) {
		printf("Please select a version of the mod to run:\n");
		for(int i=0;i<m_selectedMod->numberOfVersions();i++) {
			printf("%d: %s\n", (i + 1), m_selectedMod->getFullName(i).toLocal8Bit().data());
		}
		printf("> ");

		input = in.readLine();
		data = input.trimmed().toLower();

		if(QRegExp("^(a(bort)?|c(ancel)?)$").exactMatch(data)) {
			return false;
		}

		selectedModVersion = input.toInt(&valid, 10);

		valid = valid && selectedModVersion >= 1 && selectedModVersion <= m_selectedMod->numberOfVersions();

		if(valid) {
			m_selectedModVersionIndex = selectedModVersion - 1;
			return true;
		}
		else {
			printf("Invalid selection! To cancel, type cancel or abort.\n\n");
			Utilities::pause();
		}
	}

	return false;
}

void ModManager::runSelectedMod(const ArgumentParser * args) {
	if(!m_initialized || m_selectedMod == NULL) { return; }

	if(checkModForMissingFiles(*m_selectedMod) > 0) {
		printf("Mod is missing files, aborting execution.\n");
		return;
	}

	if(m_selectedMod->numberOfVersions() == 0) {
		printf("Mod has no versions specified, aborting execution.\n");
		return;
	}

	m_selectedModVersionIndex = 0;
	if(m_selectedMod->numberOfVersions() > 1) {
		if(!runModVersionSelectionPrompt()) {
			return;
		}

		printf("\n");
	}

	Script script;

	if(!updateScriptArgs()) {
		return;
	}

	bool customMod = false;
	if(args != NULL) {
		if((args->hasArgument("g") && args->getValue("g").length() > 0) || (args->hasArgument("x") && args->getValue("x").length() > 0)) {
			m_scriptArgs.setArgument("GROUP", args->getValue("g"));
			m_scriptArgs.setArgument("CON", args->getValue("x"));

			customMod = true;
		}
	}

	if(!customMod) {
		printf("Running mod \"%s\" in %s mode.\n\n", m_selectedMod->getFullName(m_selectedModVersionIndex).toLocal8Bit().data(), GameTypes::toString(m_gameType));
	}

	Utilities::renameFiles("DMO", "DMO_");

	if(m_mode == ModManagerModes::DOSBox) {
		bool scriptLoaded = false;
		const char * scriptFileName;

		if(m_gameType == GameTypes::Game) {
			scriptLoaded = script.readFrom(SettingsManager::getInstance()->DOSBoxGameScriptFileName);

			scriptFileName = SettingsManager::getInstance()->DOSBoxGameScriptFileName;
		}
		else if(m_gameType == GameTypes::Setup) {
			scriptLoaded = script.readFrom(SettingsManager::getInstance()->DOSBoxSetupScriptFileName);

			scriptFileName = SettingsManager::getInstance()->DOSBoxSetupScriptFileName;
		}
		else if(m_gameType == GameTypes::Client) {
			scriptLoaded = script.readFrom(SettingsManager::getInstance()->DOSBoxClientScriptFileName);

			scriptFileName = SettingsManager::getInstance()->DOSBoxClientScriptFileName;
		}
		else if(m_gameType == GameTypes::Server) {
			scriptLoaded = script.readFrom(SettingsManager::getInstance()->DOSBoxServerScriptFileName);

			scriptFileName = SettingsManager::getInstance()->DOSBoxServerScriptFileName;
		}
		else {	
			return;
		}

		if(!scriptLoaded) {
			printf("Failed to load DOSBox script file: %s\n", scriptFileName);
			return;
		}

		QString DOSBoxCommand = script.generateDOSBoxCommand(m_scriptArgs);
		QByteArray DOSBoxCommandBytes = DOSBoxCommand.toLocal8Bit();
		const char * DOSBoxCommandData = DOSBoxCommandBytes.data();

#if _DEBUG
		printf("%s\n", DOSBoxCommandData);
#endif // _DEBUG

		system(DOSBoxCommandData);
	}
	else if(m_mode == ModManagerModes::Windows) {
		bool scriptLoaded = false;
		const char * scriptFileName;

		if(m_gameType == GameTypes::Game) {
			scriptLoaded = script.readFrom(SettingsManager::getInstance()->windowsGameScriptFileName);

			scriptFileName = SettingsManager::getInstance()->windowsGameScriptFileName;
		}
		else if(m_gameType == GameTypes::Setup || m_gameType == GameTypes::Client || m_gameType == GameTypes::Server) {
			scriptLoaded = script.readFrom(SettingsManager::getInstance()->windowsSetupScriptFileName);

			scriptFileName = SettingsManager::getInstance()->windowsSetupScriptFileName;
		}
		else {
			return;
		}

		if(!scriptLoaded) {
			printf("Failed to load Windows script file: %s\n", scriptFileName);
			return;
		}

		for(int i=0;i<script.numberOfCommands();i++) {
			QString windowsCommand = script.generateWindowsCommand(m_scriptArgs, i);
			QByteArray windowsCommandBytes = windowsCommand.toLocal8Bit();
			const char * windowsCommandData = windowsCommandBytes.data();

#if _DEBUG
			printf("%s\n", windowsCommandData);
#endif // _DEBUG

			system(windowsCommandData);
		}
	}

	Utilities::deleteFiles("DMO");
	
	Utilities::renameFiles("DMO_", "DMO");
}

bool ModManager::updateScriptArgs() {
	if(!m_initialized || m_selectedMod == NULL || m_selectedMod->numberOfVersions() == 0 || m_selectedModVersionIndex < 0 || m_selectedModVersionIndex >= m_selectedMod->numberOfVersions()) { return false; }

	m_scriptArgs.setArgument("KEXTRACT", SettingsManager::getInstance()->kextractFileName);
	m_scriptArgs.setArgument("DUKE3D", SettingsManager::getInstance()->gameFileName);
	m_scriptArgs.setArgument("SETUP", SettingsManager::getInstance()->setupFileName);
	m_scriptArgs.setArgument("MODDIR", SettingsManager::getInstance()->modsDirectoryName);
	if(m_selectedMod != NULL) {
		m_scriptArgs.setArgument("CON", m_selectedMod->getVersion(m_selectedModVersionIndex)->getFileNameByType("con"));
		m_scriptArgs.setArgument("GROUP", m_selectedMod->getVersion(m_selectedModVersionIndex)->getFileNameByType("grp"));
	}
	m_scriptArgs.setArgument("IP", SettingsManager::getInstance()->serverIPAddress);

	return true;
}

bool ModManager::handleArguments(const ArgumentParser * args) {
	if(args == NULL) { return false; }

	if(args->hasArgument("m")) {
		ModManagerModes::ModManagerMode newMode = ModManagerModes::parseFrom(args->getValue("m"));
			
		if(isValid(newMode)) {
			m_mode = newMode;
		}
		else {
			printf("Invalid mode argument, please specify one of the following: %s / %s\n", ModManagerModes::toString(ModManagerModes::DOSBox), ModManagerModes::toString(ModManagerModes::Windows));
			return false;
		}
	}

	if(args->hasArgument("t")) {
		GameTypes::GameType newGameType = GameTypes::parseFrom(args->getValue("t"));

		if(isValid(newGameType)) {
			m_gameType = newGameType;

			printf("Setting game type to: %s\n", GameTypes::toString(m_gameType));

			if(m_gameType == GameTypes::Client) {
				QString ipAddress;

				if(args->hasArgument("ip")) {
					ipAddress = args->getValue("ip").trimmed();

					bool valid = true;
					for(int i=0;i<ipAddress.length();i++) {
						if(ipAddress[i] == ' ' || ipAddress[i] == '\t') {
							valid = false;
						}
					}

					if(ipAddress.length() == 0) { valid = false; }

					QByteArray ipAddressBytes = ipAddress.toLocal8Bit();
					const char * ipAddressData = ipAddressBytes.data();

					if(valid) {
						if(SettingsManager::getInstance()->serverIPAddress != NULL) { delete [] SettingsManager::getInstance()->serverIPAddress; }
						SettingsManager::getInstance()->serverIPAddress = new char[Utilities::stringLength(ipAddressData) + 1];
						Utilities::copyString(SettingsManager::getInstance()->serverIPAddress, Utilities::stringLength(ipAddressData) + 1, ipAddressData);
					}
					else {
						printf("\nInvalid IP Address entered in arguments: %s\n\n", ipAddressData);

						runIPAddressPrompt();
					}
				}
			}
		}
		else {
			printf("Invalid game type argument, please specify one of the following: %s / %s / %s / %s\n", GameTypes::toString(GameTypes::Game), GameTypes::toString(GameTypes::Setup), GameTypes::toString(GameTypes::Client), GameTypes::toString(GameTypes::Server));
			return false;
		}
	}

// TODO: add better output
	if(args->hasArgument("q")) {
		if(args->hasArgument("r") || args->hasArgument("g") || args->hasArgument("x")) {
			printf("Redundant arguments specified, please specify either q OR r OR (x AND/OR g).\n");
			return false;
		}

		int numberOfMatches = searchForAndSelectMod(args->getValue("q"));

		if(numberOfMatches == -1) {
			printf("Invalid or empty search query argument.\n");
			return false;
		}
		else if(numberOfMatches == 0) {
			printf("No matches found for specified argument.\n\n");
			return false;
		}
		else if(numberOfMatches == 1) {
			printf("Selected mod from argument: %s\n", m_selectedMod->getName());

			runSelectedMod();

			return true;
		}
		else {
			printf("%d matches found, please refine your argument search query.\n", numberOfMatches);
			return false;
		}
	}
	else if(args->hasArgument("r")) {
		if(args->hasArgument("g") || args->hasArgument("x")) {
			printf("Redundant arguments specified, please specify either q OR r OR (x AND/OR g).\n");
			return false;
		}

		selectRandomMod();
		runSelectedMod();

		return true;
	}
	else if(args->hasArgument("g") || args->hasArgument("x")) {
		runSelectedMod(args);
		return true;
	}

	return false;
}

int ModManager::checkForUnlinkedModFiles() const {
	if(!m_initialized) { return false; }

	QMap<QString, int> linkedModFiles;

	QDir dir(SettingsManager::getInstance()->modsDirectoryName);
	QFileInfoList files = dir.entryInfoList();
	for(int i=0;i<files.size();i++) {
		if(files[i].isFile()) {
			linkedModFiles[files[i].fileName().toUpper()] = 0;
		}
	}

	int numberOfMultipleLinkedModFiles = 0;
	const ModVersion * modVersion = NULL;
	const char * fileName = NULL;
	QString key;
	for(int i=0;i<m_mods.numberOfMods();i++) {
		for(int j=0;j<m_mods.getMod(i)->numberOfVersions();j++) {

			fileName = m_mods.getMod(i)->getVersion(j)->getFileNameByType("grp");
			if(fileName != NULL) {
				key = QString(fileName).toUpper();
				if(linkedModFiles[key] == 1) {
					numberOfMultipleLinkedModFiles++;

					QByteArray keyBytes = key.toLocal8Bit();
					const char * keyData = keyBytes.data();

					printf("Mod file linked multiple times: \"%s\"\n", keyData);
				}
				linkedModFiles[key] = 1;
			}

			fileName = m_mods.getMod(i)->getVersion(j)->getFileNameByType("con");
			if(fileName != NULL) {
				key = QString(fileName).toUpper();
				if(linkedModFiles[key] == 1) {
					numberOfMultipleLinkedModFiles++;

					QByteArray keyBytes = key.toLocal8Bit();
					const char * keyData = keyBytes.data();

					printf("Mod file linked multiple times: \"%s\"\n", keyData);
				}
				linkedModFiles[key] = 1;
			}
		}
	}

	int numberOfUnlinkedModFiles = 0;
	QList<QString> keys = linkedModFiles.keys();
	for(int i=0;i<keys.size();i++) {
		if(linkedModFiles[keys[i]] == 0) {
			numberOfUnlinkedModFiles++;
			
			QByteArray fileNameBytes = keys[i].toLocal8Bit();
			const char * fileNameData = fileNameBytes.data();
			
			printf("Unlinked file %d: \"%s\"\n", numberOfUnlinkedModFiles, fileNameData);
		}
	}
	if(numberOfUnlinkedModFiles > 0) {
		printf("Found %d unlinked mod file%s in mods directory.\n", numberOfUnlinkedModFiles, numberOfUnlinkedModFiles == 1 ? "" : "s");
	}
	if(numberOfMultipleLinkedModFiles > 0) {
		printf("Found %d multiple linked mod file%s in mods directory.\n", numberOfMultipleLinkedModFiles, numberOfMultipleLinkedModFiles == 1 ? "" : "s");
	}

	return numberOfUnlinkedModFiles;
}

int ModManager::checkModForMissingFiles(const char * modName, int versionIndex) const {
	if(!m_initialized) { return 0; }

	if(modName == NULL || Utilities::stringLength(modName) == 0) { return 0; }
	
	const Mod * mod = m_mods.getMod(modName);
	if(mod == NULL) { return 0; }

	return checkModForMissingFiles(*mod, versionIndex);
}

int ModManager::checkModForMissingFiles(const QString & modName, int versionIndex) const {
	if(!m_initialized) { return 0; }

	const Mod * mod = m_mods.getMod(modName);
	if(mod == NULL) { return 0; }

	return checkModForMissingFiles(*mod, versionIndex);
}

int ModManager::checkModForMissingFiles(const Mod & mod, int versionIndex) const {
	if(!m_initialized || mod.numberOfVersions() == 0 || versionIndex >= mod.numberOfVersions()) { return 0; }

	int numberOfMissingFiles = 0;
	const char * fileName = NULL;

	for(int i=(versionIndex < 0 ? 0 : versionIndex);i<(versionIndex < 0 ? mod.numberOfVersions() : versionIndex + 1);i++) {

		fileName = mod.getVersion(i)->getFileNameByType("con");

		if(fileName != NULL) {
			QFileInfo con(QString("%1/%2").arg(SettingsManager::getInstance()->modsDirectoryName).arg(fileName));

			if(!con.isFile() || !con.exists()) {
				printf("Missing mod con file: %s\n", fileName);
				
				numberOfMissingFiles++;
			}
		}

		fileName = mod.getVersion(i)->getFileNameByType("grp");

		if(fileName != NULL) {
			QFileInfo group(QString("%1/%2").arg(SettingsManager::getInstance()->modsDirectoryName).arg(fileName));

			if(!group.isFile() || !group.exists()) {
				printf("Missing mod group file: %s\n", fileName);
				
				numberOfMissingFiles++;
			}
		}
	}

	return numberOfMissingFiles;
}

int ModManager::checkAllModsForMissingFiles() const {
	if(!m_initialized) { return 0; }
	
	int numberOfMissingModFiles = 0;

	for(int i=0;i<m_mods.numberOfMods();i++) {
		numberOfMissingModFiles += checkModForMissingFiles(*m_mods.getMod(i));
	}

	if(numberOfMissingModFiles > 0) {
		printf("Found %d missing mod file%s in mods directory.\n", numberOfMissingModFiles, numberOfMissingModFiles == 1 ? "" : "s");
	}

	return numberOfMissingModFiles;
}

int ModManager::checkForMissingExecutables() {
	int numberOfMissingExecutables = 0;

	QFileInfo DOSBoxExe(QString(SettingsManager::getInstance()->DOSBoxPath));
	QFileInfo gameExe(QString(SettingsManager::getInstance()->gameFileName));
	QFileInfo setupExe(QString(SettingsManager::getInstance()->setupFileName));
	QFileInfo kextractExe(QString(SettingsManager::getInstance()->kextractFileName));

	if(!DOSBoxExe.isFile() || !DOSBoxExe.exists()) {
		numberOfMissingExecutables++;
		printf("Missing DOSBox executable: %s\n", SettingsManager::getInstance()->DOSBoxPath);
	}

	if(!gameExe.isFile() || !gameExe.exists()) {
		numberOfMissingExecutables++;
		printf("Missing game executable: %s\n", SettingsManager::getInstance()->gameFileName);
	}

	if(!setupExe.isFile() || !setupExe.exists()) {
		numberOfMissingExecutables++;
		printf("Missing setup executable: %s\n", SettingsManager::getInstance()->setupFileName);
	}

	if(!kextractExe.isFile() || !kextractExe.exists()) {
		numberOfMissingExecutables++;
		printf("Missing kextract executable: %s\n", SettingsManager::getInstance()->kextractFileName);
	}

	return numberOfMissingExecutables++;
}
