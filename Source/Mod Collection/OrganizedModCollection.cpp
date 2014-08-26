#include "Mod Collection/OrganizedModCollection.h"

OrganizedModCollection::OrganizedModCollection(ModCollection * mods, FavouriteModCollection * favourites)
	: m_mods(mods)
	, m_favourites(favourites)
	, m_filterType(ModFilterTypes::defaultFilterType)
	, m_sortType(ModSortTypes::defaultSortType)
	, m_sortDirection(ModSortDirections::defaultSortDirection)
	, m_selectedTeam(NULL)
	, m_selectedAuthor(NULL) {
	if(m_mods != NULL) {
		m_mods->addListener(dynamic_cast<ModCollectionListener *>(this));

		updateTeamList();
		updateAuthorList();
	}

	if(m_favourites != NULL) {
		m_favourites->addListener(dynamic_cast<ModCollectionListener *>(this));
	}

	organizeModCollection();
}

OrganizedModCollection::OrganizedModCollection(const OrganizedModCollection & m)
	: m_mods(m.m_mods)
	, m_favourites(m.m_favourites)
	, m_filterType(m.m_filterType)
	, m_sortType(m.m_sortType)
	, m_sortDirection(m.m_sortDirection)
	, m_selectedTeam(NULL)
	, m_selectedAuthor(NULL) {
	if(m_mods != NULL) {
		m_mods->addListener(dynamic_cast<ModCollectionListener *>(this));

		updateTeamList();
		updateAuthorList();

		m_selectedTeam = m.m_selectedTeam == NULL ? NULL : getTeamInfo(m.m_selectedTeam->getName());
		m_selectedAuthor = m.m_selectedAuthor == NULL ? NULL : getAuthorInfo(m.m_selectedAuthor->getName());
	}

	if(m_favourites != NULL) {
		m_favourites->addListener(dynamic_cast<ModCollectionListener *>(this));
	}

	organizeModCollection();
}

OrganizedModCollection & OrganizedModCollection::operator = (const OrganizedModCollection & m) {
	bool organize = m_mods != m.m_mods ||
					m_favourites != m.m_favourites ||
					m_filterType != m.m_filterType ||
					m_sortType != m.m_sortType ||
					m_sortDirection != m.m_sortDirection;

	if(m_mods != NULL) {
		m_mods->removeListener(dynamic_cast<const ModCollectionListener *>(this));
	}

	if(m_favourites != NULL) {
		m_favourites->removeListener(dynamic_cast<const ModCollectionListener *>(this));
	}

	for(int i=0;i<m_teams.size();i++) {
		delete m_teams[i];
	}
	m_teams.clear();

	for(int i=0;i<m_authors.size();i++) {
		delete m_authors[i];
	}
	m_authors.clear();

	m_mods = m.m_mods;
	m_favourites = m.m_favourites;
	m_filterType = m.m_filterType;
	m_sortType = m.m_sortType;
	m_sortDirection = m.m_sortDirection;

	if(m_mods != NULL) {
		m_mods->addListener(dynamic_cast<ModCollectionListener *>(this));

		updateTeamList();
		updateAuthorList();

		m_selectedTeam = m.m_selectedTeam == NULL ? NULL : getTeamInfo(m.m_selectedTeam->getName());
		m_selectedAuthor = m.m_selectedAuthor == NULL ? NULL : getAuthorInfo(m.m_selectedAuthor->getName());
	}

	if(m_favourites != NULL) {
		m_favourites->addListener(dynamic_cast<ModCollectionListener *>(this));
	}

	if(organize) {
		organizeModCollection();
	}

	return *this;
}

OrganizedModCollection::~OrganizedModCollection() {
	for(int i=0;i<m_teams.size();i++) {
		delete m_teams[i];
	}

	for(int i=0;i<m_authors.size();i++) {
		delete m_authors[i];
	}
}

void OrganizedModCollection::init(ModCollection * mods, FavouriteModCollection * favourites) {
	setModCollection(mods);
	setFavouriteModCollection(favourites);
}

void OrganizedModCollection::uninit() {
	if(m_mods != NULL) {
		m_mods->removeListener(dynamic_cast<const ModCollectionListener *>(this));
	}

	if(m_favourites != NULL) {
		m_favourites->removeListener(dynamic_cast<ModCollectionListener *>(this));
	}

	m_mods = NULL;
	m_favourites = NULL;
	m_organizedMods.clear();
	m_selectedAuthor = NULL;
	m_selectedTeam = NULL;

	for(int i=0;i<m_teams.size();i++) {
		delete m_teams[i];
	}
	m_teams.clear();

	for(int i=0;i<m_authors.size();i++) {
		delete m_authors[i];
	}
	m_authors.clear();
}

ModCollection * OrganizedModCollection::getModCollection() const {
	return m_mods;
}

FavouriteModCollection * OrganizedModCollection::getFavouriteModCollection() const {
	return m_favourites;
}

void OrganizedModCollection::setModCollection(ModCollection * mods) {
	bool organize = m_mods != mods;

	if(m_mods != NULL) {
		m_mods->removeListener(dynamic_cast<const ModCollectionListener *>(this));
	}

	m_mods = mods;

	if(m_mods != NULL) {
		m_mods->addListener(dynamic_cast<ModCollectionListener *>(this));

		updateTeamList();
		updateAuthorList();
	}

	if(organize) {
		organizeModCollection();
	}
}

void OrganizedModCollection::setFavouriteModCollection(FavouriteModCollection * favourites) {
	bool organize = m_favourites != favourites &&
					m_filterType == ModFilterTypes::Favourites;

	if(m_favourites != NULL) {
		m_favourites->removeListener(dynamic_cast<const ModCollectionListener *>(this));
	}

	m_favourites = favourites;

	if(m_favourites != NULL) {
		m_favourites->addListener(dynamic_cast<ModCollectionListener *>(this));
	}

	if(organize) {
		organizeModCollection();
	}
}

ModFilterTypes::ModFilterType OrganizedModCollection::getFilterType() const {
	return m_filterType;
}

ModSortTypes::ModSortType OrganizedModCollection::getSortType() const {
	return m_sortType;
}

ModSortDirections::ModSortDirection OrganizedModCollection::getSortDirection() const {
	return m_sortDirection;
}
	
bool OrganizedModCollection::setFilterType(ModFilterTypes::ModFilterType filterType) {
	if(!ModFilterTypes::isValid(filterType)) {
		return false;
	}

	bool organize = m_filterType != filterType;
	m_filterType = filterType;

	if(organize) {
		organizeModCollection();
	}

	return true;
}

bool OrganizedModCollection::setSortType(ModSortTypes::ModSortType sortType) {
	if(!ModSortTypes::isValid(sortType)) {
		return false;
	}

	bool organize = m_sortType != sortType;
	m_sortType = sortType;

	if(organize) {
		sortModCollection();
	}

	return true;
}

bool OrganizedModCollection::setSortDirection(ModSortDirections::ModSortDirection sortDirection) {
	if(!ModSortDirections::isValid(sortDirection)) {
		return false;
	}

	bool organize = m_sortDirection != sortDirection;
	m_sortDirection = sortDirection;

	if(organize) {
		sortModCollection();
	}

	return true;
}

int OrganizedModCollection::numberOfMods() const {
	return m_organizedMods.size();
}

bool OrganizedModCollection::hasMod(const Mod & mod) const {
	for(int i=0;i<m_organizedMods.size();i++) {
		if(*m_organizedMods[i] == mod) {
			return true;
		}
	}
	return false;
}

bool OrganizedModCollection::hasMod(const char * name) const {
	if(name == NULL || Utilities::stringLength(name) == 0) { return false; }

	for(int i=0;i<m_organizedMods.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_organizedMods[i]->getName(), name) == 0) {
			return true;
		}
	}
	return false;
}

bool OrganizedModCollection::hasMod(const QString & name) const {
	if(name.isEmpty()) { return false; }
	QByteArray nameBytes = name.toLocal8Bit();
	const char * nameData = nameBytes.data();

	for(int i=0;i<m_organizedMods.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_organizedMods[i]->getName(), nameData) == 0) {
			return true;
		}
	}
	return false;
}

int OrganizedModCollection::indexOfMod(const Mod & mod) const {
	for(int i=0;i<m_organizedMods.size();i++) {
		if(*m_organizedMods[i] == mod) {
			return i;
		}
	}
	return -1;
}

int OrganizedModCollection::indexOfMod(const char * name) const {
	if(name == NULL || Utilities::stringLength(name) == 0) { return -1; }

	for(int i=0;i<m_organizedMods.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_organizedMods[i]->getName(), name) == 0) {
			return i;
		}
	}
	return -1;
}

int OrganizedModCollection::indexOfMod(const QString & name) const {
	if(name.isEmpty()) { return -1; }
	QByteArray nameBytes = name.toLocal8Bit();
	const char * nameData = nameBytes.data();

	for(int i=0;i<m_organizedMods.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_organizedMods[i]->getName(), nameData) == 0) {
			return i;
		}
	}
	return -1;
}

const Mod * OrganizedModCollection::getMod(int index) const {
	if(index < 0 || index >= m_organizedMods.size()) { return NULL; }

	return m_organizedMods[index];
}

const Mod * OrganizedModCollection::getMod(const char * name) const {
	if(name == NULL || Utilities::stringLength(name) == 0) { return NULL; }

	for(int i=0;i<m_organizedMods.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_organizedMods[i]->getName(), name) == 0) {
			return m_organizedMods[i];
		}
	}
	return NULL;
}

const Mod * OrganizedModCollection::getMod(const QString & name) const {
	if(name.isEmpty()) { return NULL; }
	QByteArray nameBytes = name.toLocal8Bit();
	const char * nameData = nameBytes.data();

	for(int i=0;i<m_organizedMods.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_organizedMods[i]->getName(), nameData) == 0) {
			return m_organizedMods[i];
		}
	}
	return NULL;
}

int OrganizedModCollection::numberOfTeams() const {
	return m_teams.size();
}

bool OrganizedModCollection::hasTeamInfo(const char * name) const {
	if(name == NULL) { return false; }
	char * formattedName = Utilities::trimCopy(name);
	if(Utilities::stringLength(formattedName) == 0) {
		delete [] formattedName;
		return false;
	}

	for(int i=0;i<m_teams.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_teams[i]->getName(), formattedName) == 0) {
			delete [] formattedName;
			return true;
		}
	}
	delete [] formattedName;
	return false;
}

bool OrganizedModCollection::hasTeamInfo(const QString & name) const {
	if(name.isEmpty()) { return false; }
	QString formattedName = name.trimmed();
	if(formattedName.isEmpty()) { return false; }

	for(int i=0;i<m_teams.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_teams[i]->getName(), formattedName) == 0) {
			return true;
		}
	}
	return false;
}

const ModAuthorInformation * OrganizedModCollection::getTeamInfo(int index) const {
	if(index < 0 || index >= m_teams.size()) { return NULL; }

	return m_teams[index];
}

const ModAuthorInformation * OrganizedModCollection::getTeamInfo(const char * name) const {
	if(name == NULL) { return NULL; }
	char * formattedName = Utilities::trimCopy(name);
	if(Utilities::stringLength(formattedName) == 0) {
		delete [] formattedName;
		return NULL;
	}

	for(int i=0;i<m_teams.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_teams[i]->getName(), formattedName) == 0) {
			delete [] formattedName;
			return m_teams[i];
		}
	}
	delete [] formattedName;
	return NULL;
}

const ModAuthorInformation * OrganizedModCollection::getTeamInfo(const QString & name) const {
	if(name.isEmpty()) { return false; }
	QString formattedName = name.trimmed();
	if(formattedName.isEmpty()) { return false; }

	for(int i=0;i<m_teams.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_teams[i]->getName(), formattedName) == 0) {
			return m_teams[i];
		}
	}
	return false;
}

void OrganizedModCollection::incrementTeamModCount(int index) {
	if(index < 0 || index >= m_teams.size()) { return; }

	m_teams[index]->incrementModCount();
}

void OrganizedModCollection::incrementTeamModCount(const char * name) {
	if(name == NULL) { return; }
	char * formattedName = Utilities::trimCopy(name);
	if(Utilities::stringLength(formattedName) == 0) {
		delete [] formattedName;
		return;
	}

	for(int i=0;i<m_teams.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_teams[i]->getName(), formattedName) == 0) {
			delete [] formattedName;
			m_teams[i]->incrementModCount();
			return;
		}
	}

	m_teams.push_back(new ModAuthorInformation(formattedName));

	delete [] formattedName;
}

void OrganizedModCollection::incrementTeamModCount(const QString & name) {
	if(name.isEmpty()) { return; }
	QString formattedName = name.trimmed();
	if(formattedName.isEmpty()) { return; }

	for(int i=0;i<m_teams.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_teams[i]->getName(), formattedName) == 0) {
			m_teams[i]->incrementModCount();
			return;
		}
	}
	
	m_teams.push_back(new ModAuthorInformation(formattedName));
}

bool OrganizedModCollection::setSelectedTeam(int index) {
	if(index < 0 || index >= m_teams.size()) { return false; }

	m_selectedTeam = m_teams[index];

	return true;
}

bool OrganizedModCollection::setSelectedTeam(const char * name) {
	if(name == NULL) { return false; }
	char * formattedName = Utilities::trimCopy(name);
	if(Utilities::stringLength(formattedName) == 0) {
		delete [] formattedName;
		return false;
	}

	for(int i=0;i<m_teams.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_teams[i]->getName(), formattedName) == 0) {
			delete [] formattedName;
			m_selectedTeam = m_teams[i];
			return true;
		}
	}
	delete [] formattedName;
	return false;
}

bool OrganizedModCollection::setSelectedTeam(const QString & name) {
	if(name.isEmpty()) { return false; }
	QString formattedName = name.trimmed();
	if(formattedName.isEmpty()) { return false; }

	for(int i=0;i<m_teams.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_teams[i]->getName(), formattedName) == 0) {
			m_selectedTeam = m_teams[i];
			return true;
		}
	}
	return false;
}

bool OrganizedModCollection::setSelectedTeam(const ModAuthorInformation * teamInfo) {
	if(teamInfo == NULL) { return false; }

	for(int i=0;i<m_teams.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_teams[i]->getName(), teamInfo->getName()) == 0) {
			m_selectedTeam = m_teams[i];
			return true;
		}
	}
	return false;
}

void OrganizedModCollection::clearSelectedTeam() {
	m_selectedTeam = NULL;
}

int OrganizedModCollection::numberOfAuthors() const {
	return m_authors.size();
}

bool OrganizedModCollection::hasAuthorInfo(const char * name) const {
	if(name == NULL) { return false; }
	char * formattedName = Utilities::trimCopy(name);
	if(Utilities::stringLength(formattedName) == 0) {
		delete [] formattedName;
		return false;
	}

	for(int i=0;i<m_authors.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_authors[i]->getName(), formattedName) == 0) {
			delete [] formattedName;
			return true;
		}
	}
	delete [] formattedName;
	return false;
}

bool OrganizedModCollection::hasAuthorInfo(const QString & name) const {
	if(name.isEmpty()) { return false; }
	QString formattedName = name.trimmed();
	if(formattedName.isEmpty()) { return false; }

	for(int i=0;i<m_authors.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_authors[i]->getName(), formattedName) == 0) {
			return true;
		}
	}
	return false;
}

const ModAuthorInformation * OrganizedModCollection::getAuthorInfo(int index) const {
	if(index < 0 || index >= m_authors.size()) { return NULL; }

	return m_authors[index];
}

const ModAuthorInformation * OrganizedModCollection::getAuthorInfo(const char * name) const {
	if(name == NULL) { return NULL; }
	char * formattedName = Utilities::trimCopy(name);
	if(Utilities::stringLength(formattedName) == 0) {
		delete [] formattedName;
		return NULL;
	}

	for(int i=0;i<m_authors.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_authors[i]->getName(), formattedName) == 0) {
			delete [] formattedName;
			return m_authors[i];
		}
	}
	delete [] formattedName;
	return NULL;
}

const ModAuthorInformation * OrganizedModCollection::getAuthorInfo(const QString & name) const {
	if(name.isEmpty()) { return false; }
	QString formattedName = name.trimmed();
	if(formattedName.isEmpty()) { return false; }

	for(int i=0;i<m_authors.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_authors[i]->getName(), formattedName) == 0) {
			return m_authors[i];
		}
	}
	return false;
}

void OrganizedModCollection::incrementAuthorModCount(int index) {
	if(index < 0 || index >= m_authors.size()) { return; }

	m_authors[index]->incrementModCount();
}

void OrganizedModCollection::incrementAuthorModCount(const char * name) {
	if(name == NULL) { return; }
	char * formattedName = Utilities::trimCopy(name);
	if(Utilities::stringLength(formattedName) == 0) {
		delete [] formattedName;
		return;
	}

	for(int i=0;i<m_authors.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_authors[i]->getName(), formattedName) == 0) {
			delete [] formattedName;
			m_authors[i]->incrementModCount();
			return;
		}
	}

	m_authors.push_back(new ModAuthorInformation(formattedName));

	delete [] formattedName;
}

void OrganizedModCollection::incrementAuthorModCount(const QString & name) {
	if(name.isEmpty()) { return; }
	QString formattedName = name.trimmed();
	if(formattedName.isEmpty()) { return; }

	for(int i=0;i<m_authors.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_authors[i]->getName(), formattedName) == 0) {
			m_authors[i]->incrementModCount();
			return;
		}
	}
	
	m_authors.push_back(new ModAuthorInformation(formattedName));
}

bool OrganizedModCollection::setSelectedAuthor(int index) {
	if(index < 0 || index >= m_authors.size()) { return false; }

	m_selectedAuthor = m_authors[index];

	return true;
}

bool OrganizedModCollection::setSelectedAuthor(const char * name) {
	if(name == NULL) { return false; }
	char * formattedName = Utilities::trimCopy(name);
	if(Utilities::stringLength(formattedName) == 0) {
		delete [] formattedName;
		return false;
	}

	for(int i=0;i<m_authors.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_authors[i]->getName(), formattedName) == 0) {
			delete [] formattedName;
			m_selectedAuthor = m_authors[i];
			return true;
		}
	}
	delete [] formattedName;
	return false;
}

bool OrganizedModCollection::setSelectedAuthor(const QString & name) {
	if(name.isEmpty()) { return false; }
	QString formattedName = name.trimmed();
	if(formattedName.isEmpty()) { return false; }

	for(int i=0;i<m_authors.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_authors[i]->getName(), formattedName) == 0) {
			m_selectedAuthor = m_authors[i];
			return true;
		}
	}
	return false;
}

bool OrganizedModCollection::setSelectedAuthor(const ModAuthorInformation * authorInfo) {
	if(authorInfo == NULL) { return false; }

	for(int i=0;i<m_authors.size();i++) {
		if(Utilities::compareStringsIgnoreCase(m_authors[i]->getName(), authorInfo->getName()) == 0) {
			m_selectedAuthor = m_authors[i];
			return true;
		}
	}
	return false;
}

void OrganizedModCollection::clearSelectedAuthor() {
	m_selectedAuthor = NULL;
}

void OrganizedModCollection::modCollectionUpdated() {
	updateTeamList();
	updateAuthorList();

	organizeModCollection();
}

void OrganizedModCollection::favouriteModCollectionUpdated() {
	if(m_filterType == ModFilterTypes::Favourites) {
		organizeModCollection();
	}
}

void OrganizedModCollection::organizeModCollection() {
	if(m_mods == NULL) { return; }

	filterModCollection();

	sortModCollection();
}

void OrganizedModCollection::filterModCollection() {
	if(m_mods == NULL) { return; }

	m_organizedMods.clear();

	if(m_filterType == ModFilterTypes::Favourites &&
	   (m_favourites == NULL || m_favourites->numberOfFavourites() == 0)) {
		return;
	}

	if(m_filterType == ModFilterTypes::None) {
		for(int i=0;i<m_mods->numberOfMods();i++) {
			m_organizedMods.push_back(m_mods->getMod(i));
		}
	}
	else if(m_filterType == ModFilterTypes::Favourites) {
		for(int i=0;i<m_mods->numberOfMods();i++) {
			if(m_favourites->hasFavourite(m_mods->getMod(i)->getName())) {
				m_organizedMods.push_back(m_mods->getMod(i));
			}
		}
	}
	else if(m_filterType == ModFilterTypes::Teams) {
		if(m_selectedTeam != NULL) {
			for(int i=0;i<m_mods->numberOfMods();i++) {
				if(Utilities::compareStrings(m_mods->getMod(i)->getTeam()->getName(), m_selectedTeam->getName()) == 0) {
					m_organizedMods.push_back(m_mods->getMod(i));
				}
			}
		}
	}
	else if(m_filterType == ModFilterTypes::Authors) {
		const ModTeam * team = NULL;
		if(m_selectedAuthor != NULL) {
			for(int i=0;i<m_mods->numberOfMods();i++) {
				team = m_mods->getMod(i)->getTeam();
				if(team != NULL && team->numberOfMembers() > 0) {
					for(int j=0;j<team->numberOfMembers();j++) {
						if(Utilities::compareStrings(team->getMember(j)->getName(), m_selectedAuthor->getName()) == 0) {
							m_organizedMods.push_back(m_mods->getMod(i));
						}
					}
				}
			}
		}
	}
}

void OrganizedModCollection::sortModCollection() {
	if(m_mods == NULL || m_sortType == ModSortTypes::Unsorted) { return; }

	if(m_filterType == ModFilterTypes::None || m_filterType == ModFilterTypes::Favourites) {
		m_organizedMods = mergeSortMods(m_organizedMods);
	}
	else if(m_filterType == ModFilterTypes::Teams) {
		m_teams = mergeSortAuthors(m_teams);
	}
	else if(m_filterType == ModFilterTypes::Authors) {
		m_authors = mergeSortAuthors(m_authors);
	}
}

QVector<const Mod *> OrganizedModCollection::mergeSortMods(QVector<const Mod *> mods) {
	if(mods.size() <= 1) {
		return mods;
	}

	QVector<const Mod *> left;
	QVector<const Mod *> right;

	int mid = mods.size() / 2;

	for(int i=0;i<mid;i++) {
		left.push_back(mods[i]);
	}

	for(int i=mid;i<mods.size();i++) {
		right.push_back(mods[i]);
	}

	left = mergeSortMods(left);
	right = mergeSortMods(right);

	return mergeMods(left, right);
}

QVector<const Mod *> OrganizedModCollection::mergeMods(QVector<const Mod *> left, QVector<const Mod *> right) {
	QVector<const Mod *> result;

	bool pushLeft = true;

	while(left.size() > 0 && right.size() > 0) {
		if(m_sortType == ModSortTypes::Name) {
			if(m_sortDirection == ModSortDirections::Ascending) {
				pushLeft = Utilities::compareStringsIgnoreCase(left[0]->getName(), right[0]->getName()) <= 0;
			}
			else {
				pushLeft = Utilities::compareStringsIgnoreCase(left[0]->getName(), right[0]->getName()) > 0;
			}
		}
		if(m_sortType == ModSortTypes::ReleaseDate) {
			if(m_sortDirection == ModSortDirections::Ascending) {
				if(left[0]->getReleaseDate().isNull()) { pushLeft = false; }
				else if(right[0]->getReleaseDate().isNull()) { pushLeft = true; }
				else { pushLeft = left[0]->getReleaseDate() <= right[0]->getReleaseDate(); }
			}
			else {
				if(left[0]->getReleaseDate().isNull()) { pushLeft = false; }
				else if(right[0]->getReleaseDate().isNull()) { pushLeft = true; }
				else { pushLeft = left[0]->getReleaseDate() > right[0]->getReleaseDate(); }
			}
		}
// TODO: sort by rating
		if(m_sortType == ModSortTypes::Rating) {
			if(m_sortDirection == ModSortDirections::Ascending) {

			}
			else {

			}
		}

		if(pushLeft) {
			result.push_back(left[0]);
			left.remove(0);
		}
		else {
			result.push_back(right[0]);
			right.remove(0);
		}
	}

	for(int i=0;i<left.size();i++) {
		result.push_back(left[i]);
	}

	for(int i=0;i<right.size();i++) {
		result.push_back(right[i]);
	}

	return result;
}

QVector<ModAuthorInformation *> OrganizedModCollection::mergeSortAuthors(QVector<ModAuthorInformation *> authors) {
	if(authors.size() <= 1) {
		return authors;
	}

	QVector<ModAuthorInformation *> left;
	QVector<ModAuthorInformation *> right;

	int mid = authors.size() / 2;

	for(int i=0;i<mid;i++) {
		left.push_back(authors[i]);
	}

	for(int i=mid;i<authors.size();i++) {
		right.push_back(authors[i]);
	}

	left = mergeSortAuthors(left);
	right = mergeSortAuthors(right);

	return mergeAuthors(left, right);
}

QVector<ModAuthorInformation *> OrganizedModCollection::mergeAuthors(QVector<ModAuthorInformation *> left, QVector<ModAuthorInformation *> right) {
	QVector<ModAuthorInformation *> result;

	bool pushLeft = true;

	while(left.size() > 0 && right.size() > 0) {
		if(m_sortType == ModSortTypes::Name) {
			if(m_sortDirection == ModSortDirections::Ascending) {
				pushLeft = Utilities::compareStringsIgnoreCase(left[0]->getName(), right[0]->getName()) <= 0;
			}
			else {
				pushLeft = Utilities::compareStringsIgnoreCase(left[0]->getName(), right[0]->getName()) > 0;
			}
		}
		else if(m_sortType == ModSortTypes::NumberOfMods) {
			if(m_sortDirection == ModSortDirections::Ascending) {
				pushLeft = left[0]->getModCount() > right[0]->getModCount();
			}
			else {
				pushLeft = left[0]->getModCount() <= right[0]->getModCount();
			}
		}

		if(pushLeft) {
			result.push_back(left[0]);
			left.remove(0);
		}
		else {
			result.push_back(right[0]);
			right.remove(0);
		}
	}

	for(int i=0;i<left.size();i++) {
		result.push_back(left[i]);
	}

	for(int i=0;i<right.size();i++) {
		result.push_back(right[i]);
	}

	return result;
}

void OrganizedModCollection::updateTeamList() {
	if(m_mods == NULL) { return; }

	for(int i=0;i<m_teams.size();i++) {
		delete m_teams[i];
	}
	m_teams.clear();

	const ModTeam * team = NULL;
	for(int i=0;i<m_mods->numberOfMods();i++) {
		team = m_mods->getMod(i)->getTeam();
		if(team != NULL) {
			incrementTeamModCount(team->getName());
		}
	}
}

void OrganizedModCollection::updateAuthorList() {
	if(m_mods == NULL) { return; }

	for(int i=0;i<m_authors.size();i++) {
		delete m_authors[i];
	}
	m_authors.clear();

	const ModTeam * team = NULL;
	for(int i=0;i<m_mods->numberOfMods();i++) {
		team = m_mods->getMod(i)->getTeam();
		if(team != NULL && team->getName() != NULL) {
			for(int j=0;j<team->numberOfMembers();j++) {
				incrementAuthorModCount(team->getMember(j)->getName());
			}
		}
	}
}

bool OrganizedModCollection::operator == (const OrganizedModCollection & m) const {
	if(m_mods == NULL && m.m_mods == NULL) {
		return true;
	}
	else if((m_mods == NULL && m.m_mods != NULL) || 
			(m_mods != NULL && m.m_mods == NULL)) {
		return false;
	}

	return *m_mods == *m.m_mods;
}

bool OrganizedModCollection::operator != (const OrganizedModCollection & m) const {
	return !operator == (m);
}
