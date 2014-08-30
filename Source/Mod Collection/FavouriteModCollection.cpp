#include "Mod Collection/FavouriteModCollection.h"

FavouriteModCollection::FavouriteModCollection(ModCollection * mods)
	: m_mods(NULL)
	, m_initialized(false) {
	
}

FavouriteModCollection::FavouriteModCollection(const FavouriteModCollection & m)
	: m_mods(m.m_mods)
	, m_initialized(m.m_initialized) {
	for(int i=0;i<m.m_favourites.size();i++) {
		m_favourites.push_back(new ModInformation(*m.m_favourites[i]));
	}
}

FavouriteModCollection & FavouriteModCollection::operator = (const FavouriteModCollection & m) {
	clearFavourites();

	m_mods = m.m_mods;

	for(int i=0;i<m.m_favourites.size();i++) {
		m_favourites.push_back(new ModInformation(*m.m_favourites[i]));
	}

	m_initialized = m.m_initialized;

	return *this;
}

FavouriteModCollection::~FavouriteModCollection() {
	for(int i=0;i<m_favourites.size();i++) {
		delete m_favourites[i];
	}
}

bool FavouriteModCollection::init(ModCollection * mods, bool loadFavouriteMods) {
	if(m_initialized) { return true; }
	if(mods == NULL) { return false; }

	m_mods = mods;

	if(loadFavouriteMods) {
		m_initialized = true;

		loadFavourites();

		checkForMissingFavouriteMods();
	}

	return true;
}

void FavouriteModCollection::uninit(bool saveFavouriteMods) {
	if(saveFavouriteMods) {
		saveFavourites();
	}

	m_initialized = false;
	m_mods = NULL;

	clearFavourites();
}

int FavouriteModCollection::numberOfFavourites() {
	return m_favourites.size();
}

bool FavouriteModCollection::hasFavourite(const ModInformation & favourite) const {
	for(int i=0;i<m_favourites.size();i++) {
		if(*m_favourites[i] == favourite) {
			return true;
		}
	}
	return false;
}

bool FavouriteModCollection::hasFavourite(const char * name) const {
	if(name == NULL || Utilities::stringLength(name) == 0) { return false; }

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), name) == 0) {
			return true;
		}
	}
	return false;
}

bool FavouriteModCollection::hasFavourite(const QString & name) const {
	if(name.isEmpty()) { return false; }
	QByteArray nameBytes = name.toLocal8Bit();

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), nameBytes.data()) == 0) {
			return true;
		}
	}
	return false;
}

bool FavouriteModCollection::hasFavourite(const char * name, const char * version) const {
	if(name == NULL || Utilities::stringLength(name) == 0) { return false; }

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), name) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_favourites[i]->getVersion(), version) == 0) {
			return true;
		}
	}
	return false;
}

bool FavouriteModCollection::hasFavourite(const QString & name, const QString & version) const {
	if(name.isEmpty()) { return false; }

	QByteArray nameBytes = name.toLocal8Bit();
	QByteArray versionBytes = version.toLocal8Bit();

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), nameBytes.data()) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_favourites[i]->getVersion(), version.isNull() ? NULL : versionBytes.data()) == 0) {
			return true;
		}
	}
	return false;
}

int FavouriteModCollection::indexOfFavourite(const ModInformation & favourite) const {
	for(int i=0;i<m_favourites.size();i++) {
		if(*m_favourites[i] == favourite) {
			return i;
		}
	}
	return -1;
}

int FavouriteModCollection::indexOfFavourite(const char * name) const {
	if(name == NULL || Utilities::stringLength(name) == 0) { return -1; }

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), name) == 0) {
			return i;
		}
	}
	return -1;
}

int FavouriteModCollection::indexOfFavourite(const QString & name) const {
	if(name.isEmpty()) { return -1; }
	QByteArray nameBytes = name.toLocal8Bit();

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), nameBytes.data()) == 0) {
			return i;
		}
	}
	return -1;
}

int FavouriteModCollection::indexOfFavourite(const char * name, const char * version) const {
	if(name == NULL || Utilities::stringLength(name) == 0) { return -1; }

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), name) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_favourites[i]->getVersion(), version) == 0) {
			return i;
		}
	}
	return -1;
}

int FavouriteModCollection::indexOfFavourite(const QString & name, const QString & version) const {
	if(name.isEmpty()) { return -1; }

	QByteArray nameBytes = name.toLocal8Bit();
	QByteArray versionBytes = version.toLocal8Bit();

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), nameBytes.data()) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_favourites[i]->getVersion(), version.isNull() ? NULL : versionBytes.data()) == 0) {
			return i;
		}
	}
	return -1;
}

const ModInformation * FavouriteModCollection::getFavourite(int index) const {
	if(index < 0 || index >= m_favourites.size()) { return NULL; }

	return m_favourites[index];
}

const ModInformation * FavouriteModCollection::getFavourite(const char * name) const {
	if(name == NULL || Utilities::stringLength(name) == 0) { return NULL; }

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), name) == 0) {
			return m_favourites[i];
		}
	}
	return NULL;
}

const ModInformation * FavouriteModCollection::getFavourite(const QString & name) const {
	if(name.isEmpty()) { return NULL; }

	QByteArray nameBytes = name.toLocal8Bit();

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), nameBytes.data()) == 0) {
			return m_favourites[i];
		}
	}
	return NULL;
}

const ModInformation * FavouriteModCollection::getFavourite(const char * name, const char * version) const {
	if(name == NULL || Utilities::stringLength(name) == 0) { return NULL; }

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), name) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_favourites[i]->getVersion(), version) == 0) {
			return m_favourites[i];
		}
	}
	return NULL;
}

const ModInformation * FavouriteModCollection::getFavourite(const QString & name, const QString & version) const {
	if(name.isEmpty()) { return NULL; }

	QByteArray nameBytes = name.toLocal8Bit();
	QByteArray versionBytes = version.toLocal8Bit();

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), nameBytes.data()) == 0,
		   Utilities::compareStringsIgnoreCase(m_favourites[i]->getVersion(), version.isNull() ? NULL : versionBytes.data()) == 0) {
			return m_favourites[i];
		}
	}
	return NULL;
}

bool FavouriteModCollection::addFavourite(ModInformation * favourite) {
	if(favourite == NULL || Utilities::stringLength(favourite->getName()) == 0 || hasFavourite(*favourite)) {
		return false;
	}
	
	m_favourites.push_back(favourite);

	return true;
}

bool FavouriteModCollection::removeFavourite(int index) {
	if(index < 0 || index >= m_favourites.size()) { return false; }
	
	delete m_favourites[index];
	m_favourites.remove(index);
	
	return true;
}

bool FavouriteModCollection::removeFavourite(const ModInformation & favourite) {
	for(int i=0;i<m_favourites.size();i++) {
		if(*m_favourites[i] == favourite) {
			delete m_favourites[i];
			m_favourites.remove(i);
			
			return true;
		}
	}
	return false;
}

bool FavouriteModCollection::removeFavourite(const char * name) {
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

bool FavouriteModCollection::removeFavourite(const QString & name) {
	if(name.isEmpty()) { return false; }

	QByteArray nameBytes = name.toLocal8Bit();

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), nameBytes.data()) == 0) {
			delete m_favourites[i];
			m_favourites.remove(i);

			return true;
		}
	}
	return false;
}

bool FavouriteModCollection::removeFavourite(const char * name, const char * version) {
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

bool FavouriteModCollection::removeFavourite(const QString & name, const QString & version) {
	if(name.isEmpty()) { return false; }

	QByteArray nameBytes = name.toLocal8Bit();
	QByteArray versionBytes = version.toLocal8Bit();

	for(int i=0;i<m_favourites.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_favourites[i]->getName(), nameBytes.data()) == 0 &&
		   Utilities::compareStringsIgnoreCase(m_favourites[i]->getVersion(), version.isNull() ? NULL : versionBytes.data()) == 0) {
			delete m_favourites[i];
			m_favourites.remove(i);

			return true;
		}
	}
	return false;
}

void FavouriteModCollection::clearFavourites() {
	for(int i=0;i<m_favourites.size();i++) {
		delete m_favourites[i];
	}
	m_favourites.clear();
}

bool FavouriteModCollection::loadFavourites() {
	return loadFavourites(SettingsManager::getInstance()->favouritesListFileName);
}

bool FavouriteModCollection::loadFavourites(const QString & fileName) {
	if(fileName.isEmpty()) { return false; }

	QByteArray fileNameBytes = fileName.toLocal8Bit();

	return loadFavourites(fileNameBytes.data());
}

bool FavouriteModCollection::loadFavourites(const char * fileName) {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return false; }
	
	char * fileExtension = Utilities::getFileExtension(fileName);

	bool loaded = false;
	
	if(fileExtension == NULL) {
		return false;
	}
	else if(Utilities::compareStringsIgnoreCase(fileExtension, "txt") == 0 || Utilities::compareStringsIgnoreCase(fileExtension, "ini") == 0) {
		loaded = loadFavouritesList(fileName);
	}
	else if(Utilities::compareStringsIgnoreCase(fileExtension, "xml") == 0) {
		loaded = loadFavouritesXML(fileName);
	}

	delete [] fileExtension;

	return loaded;
}

bool FavouriteModCollection::loadFavouritesList(const QString & fileName) {
	if(fileName.isEmpty()) { return false; }

	QByteArray fileNameBytes = fileName.toLocal8Bit();;

	return loadFavouritesList(fileNameBytes.data());
}

bool FavouriteModCollection::loadFavouritesList(const char * fileName) {
	if(!m_initialized || m_mods == NULL) { return false; }

	QString favouritesListPath(fileName);
	
	QFileInfo favouritesListFileInfo(favouritesListPath);
	if(!favouritesListFileInfo.exists()) { return false; }
	
	QFile input(favouritesListPath);
	if(!input.open(QIODevice::ReadOnly | QIODevice::Text)) { return false; }
	
	clearFavourites();

	QString line;
	ModInformation * newFavourite = NULL;

	while(true) {
		if(input.atEnd()) { break; }
		
		line = input.readLine().trimmed();

		if(line.length() == 0) { continue; }

		const Mod * mod = m_mods->getMod(line);

		if(mod != NULL) {
			newFavourite = new ModInformation(mod->getName(), mod->getLatestVersion());
			if(!addFavourite(newFavourite)) {
				printf("Attempted to add duplicate favourite mod: %s\n", newFavourite->getFullName());

				delete newFavourite;
			}
		}
		else {
			QByteArray lineBytes = line.toLocal8Bit();

			printf("Unknown or missing mod in favourites list: %s\n", lineBytes.data());
		}
	}

	if(m_favourites.size() > 0) {
		printf("%d favourite mod%s loaded.\n", m_favourites.size(), m_favourites.size() == 1 ? "" : "s");
	}

	input.close();

	return true;
}

bool FavouriteModCollection::loadFavouritesXML(const QString & fileName) {
	if(fileName.isEmpty()) { return false; }

	QByteArray fileNameBytes = fileName.toLocal8Bit();

	return loadFavouritesXML(fileNameBytes.data());
}

bool FavouriteModCollection::loadFavouritesXML(const char * fileName) {
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
	ModInformation * newFavourite = NULL;
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
						if(m_mods != NULL && !game.isEmpty() && QString::compare(game, m_mods->getGameName(), Qt::CaseInsensitive) != 0) {
							printf("Attempted to load a favourite mod list for game \"%s\", while mod list is for game \"%s\".\n", game.toLocal8Bit().data(), m_mods->getGameName());
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
				newFavourite = new ModInformation(name, version);
				if(!addFavourite(newFavourite)) {
					printf("Attempted to add duplicate favourite mod: %s\n", newFavourite->getFullName());

					delete newFavourite;
				}
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

bool FavouriteModCollection::saveFavourites() {
	return saveFavourites(SettingsManager::getInstance()->favouritesListFileName);
}

bool FavouriteModCollection::saveFavourites(const QString & fileName) {
	if(fileName.isEmpty()) { return false; }

	QByteArray fileNameBytes = fileName.toLocal8Bit();

	return saveFavourites(fileNameBytes.data());
}

bool FavouriteModCollection::saveFavourites(const char * fileName) {
	if(fileName == NULL || Utilities::stringLength(fileName) == 0) { return false; }
	
	char * fileExtension = Utilities::getFileExtension(fileName);

	bool loaded = false;
	
	if(fileExtension == NULL) {
		return false;
	}
	else if(Utilities::compareStringsIgnoreCase(fileExtension, "txt") == 0 || Utilities::compareStringsIgnoreCase(fileExtension, "ini") == 0) {
		loaded = saveFavouritesList(fileName);
	}
	else if(Utilities::compareStringsIgnoreCase(fileExtension, "xml") == 0) {
		loaded = saveFavouritesXML(fileName);
	}

	delete [] fileExtension;

	return loaded;
}

bool FavouriteModCollection::saveFavouritesList(const QString & fileName) {
	if(fileName.isEmpty()) { return false; }

	QByteArray fileNameBytes = fileName.toLocal8Bit();

	return saveFavouritesList(fileNameBytes.data());
}

bool FavouriteModCollection::saveFavouritesList(const char * fileName) {
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

bool FavouriteModCollection::saveFavouritesXML(const QString & fileName) {
	if(fileName.isEmpty()) { return false; }

	QByteArray fileNameBytes = fileName.toLocal8Bit();

	return saveFavouritesList(fileNameBytes.data());
}

bool FavouriteModCollection::saveFavouritesXML(const char * fileName) {
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

bool FavouriteModCollection::operator == (const FavouriteModCollection & m) const {
	if(!m_initialized && !m.m_initialized) {
		return true;
	}
	else if((m_initialized && !m.m_initialized) ||
			(!m_initialized && m.m_initialized)) {
		return false;
	}

	if(m_favourites.size() != m.m_favourites.size()) {
		return false;
	}

	if(*m_mods != *m.m_mods) {
		return false;
	}

	bool foundFavourite;
	for(int i=0;i<m_favourites.size();i++) {
		foundFavourite = false;

		for(int j=0;j<m.m_favourites.size();j++) {
			if(*m_favourites[i] == *m.m_favourites[j]) {
				foundFavourite = true;
			}
		}

		if(!foundFavourite) {
			return false;
		}
	}
	return true;
}

int FavouriteModCollection::checkForMissingFavouriteMods() const {
	if(!m_initialized || m_favourites.size() == 0) { return 0; }

	int numberOfMissingFavouriteMods = 0;

	for(int i=0;i<m_favourites.size();i++) {
		if(!m_mods->hasMod(m_favourites[i]->getName())) {
			numberOfMissingFavouriteMods++;

			printf("Missing favourite mod %d: %s\n", numberOfMissingFavouriteMods, m_favourites[i]->getName());
		}
	}

	if(numberOfMissingFavouriteMods > 0) {
		printf("Missing %d favourite mod%s.\n", numberOfMissingFavouriteMods, numberOfMissingFavouriteMods == 1 ? "" : "s");
	}

	return numberOfMissingFavouriteMods;
}

bool FavouriteModCollection::operator != (const FavouriteModCollection & m) const {
	return !operator == (m);
}

void FavouriteModCollection::notifyFavouriteModsChanged() const {
	for(int i=0;i<numberOfListeners();i++) {
		getListener(i)->favouriteModCollectionUpdated();
	}
}
