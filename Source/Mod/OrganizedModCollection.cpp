#include "OrganizedModCollection.h"

#include "FavouriteModCollection.h"
#include "Game/GameVersion.h"
#include "Game/GameVersionCollection.h"
#include "Mod.h"
#include "ModAuthorInformation.h"
#include "ModCollection.h"
#include "ModTeam.h"
#include "ModTeamMember.h"

#include <Utilities/Utilities.h>
#include <Utilities/StringUtilities.h>

#include <sstream>

const OrganizedModCollection::FilterType OrganizedModCollection::DEFAULT_FILTER_TYPE = FilterType::None;
const OrganizedModCollection::SortType OrganizedModCollection::DEFAULT_SORT_TYPE = SortType::Name;
const OrganizedModCollection::SortDirection OrganizedModCollection::DEFAULT_SORT_DIRECTION = SortDirection::Ascending;

OrganizedModCollection::OrganizedModCollection(std::shared_ptr<ModCollection> mods, std::shared_ptr<FavouriteModCollection> favourites, std::shared_ptr<GameVersionCollection> gameVersions)
	: ModCollectionListener()
	, m_filterType(OrganizedModCollection::DEFAULT_FILTER_TYPE)
	, m_sortType(OrganizedModCollection::DEFAULT_SORT_TYPE)
	, m_sortDirection(OrganizedModCollection::DEFAULT_SORT_DIRECTION)
	, m_mods(mods)
	, m_favourites(favourites)
	, m_gameVersions(gameVersions) {
	if(m_mods != nullptr) {
		m_mods->addListener(*this);

		updateGameVersionList();
		updateTeamList();
		updateAuthorList();
	}

	if(m_favourites != nullptr) {
		m_favourites->addListener(*this);
	}

	if(m_gameVersions != nullptr) {
		m_gameVersions->addListener(*this);
	}

	organize();
}

OrganizedModCollection::OrganizedModCollection(OrganizedModCollection && m) noexcept
	: ModCollectionListener(m)
	, m_filterType(m.m_filterType)
	, m_sortType(m.m_sortType)
	, m_sortDirection(m.m_sortDirection)
	, m_mods(m.m_mods == nullptr ? nullptr : std::move(m.m_mods))
	, m_favourites(m.m_favourites == nullptr ? nullptr : std::move(m.m_favourites))
	, m_gameVersions(m.m_gameVersions == nullptr ? nullptr : std::move(m.m_gameVersions))
	, m_organizedMods(std::move(m.m_organizedMods))
	, m_organizedGameVersions(std::move(m.m_organizedGameVersions))
	, m_teams(std::move(m.m_teams))
	, m_authors(std::move(m.m_authors))
	, m_selectedTeam(m.m_selectedTeam == nullptr ? nullptr : std::move(m.m_selectedTeam))
	, m_selectedAuthor(m.m_selectedAuthor == nullptr ? nullptr : std::move(m.m_selectedAuthor)) {
	if(m_mods != nullptr) {
		m_mods->addListener(*this);
	}

	if(m_favourites != nullptr) {
		m_favourites->addListener(*this);
	}

	if(m_gameVersions != nullptr) {
		m_gameVersions->addListener(*this);
	}
}

OrganizedModCollection::OrganizedModCollection(const OrganizedModCollection & m)
	: ModCollectionListener(m)
	, m_filterType(m.m_filterType)
	, m_sortType(m.m_sortType)
	, m_sortDirection(m.m_sortDirection)
	, m_mods(m.m_mods)
	, m_favourites(m.m_favourites)
	, m_gameVersions(m.m_gameVersions) {
	if(m_mods != nullptr) {
		m_mods->addListener(*this);

		updateGameVersionList();
		updateTeamList();
		updateAuthorList();

		m_selectedTeam = m.m_selectedTeam == nullptr ? nullptr : getTeamInfo(m.m_selectedTeam->getName());
		m_selectedAuthor = m.m_selectedAuthor == nullptr ? nullptr : getAuthorInfo(m.m_selectedAuthor->getName());
	}

	if(m_favourites != nullptr) {
		m_favourites->addListener(*this);
	}

	if(m_gameVersions != nullptr) {
		m_gameVersions->addListener(*this);
	}

	organize();
}

OrganizedModCollection & OrganizedModCollection::operator = (OrganizedModCollection && m) noexcept {
	if(this != &m) {
		ModCollectionListener::operator = (m);

		if(m_mods != nullptr) {
			m_mods->removeListener(*this);
		}

		if(m_favourites != nullptr) {
			m_favourites->removeListener(*this);
		}

		if(m_gameVersions != nullptr) {
			m_gameVersions->removeListener(*this);
		}

		m_filterType = m.m_filterType;
		m_sortType = m.m_sortType;
		m_sortDirection = m.m_sortDirection;
		m_mods = m.m_mods == nullptr ? nullptr : std::move(m.m_mods);
		m_favourites = m.m_favourites == nullptr ? nullptr : std::move(m.m_favourites);
		m_gameVersions = m.m_gameVersions == nullptr ? nullptr : std::move(m.m_gameVersions);
		m_teams = std::move(m.m_teams);
		m_authors = std::move(m.m_authors);
		m_selectedTeam = m.m_selectedTeam == nullptr ? nullptr : std::move(m.m_selectedTeam);
		m_selectedAuthor = m.m_selectedAuthor == nullptr ? nullptr : std::move(m.m_selectedAuthor);

		if(m_mods != nullptr) {
			m_mods->addListener(*this);
		}

		if(m_favourites != nullptr) {
			m_favourites->addListener(*this);
		}

		if(m_gameVersions != nullptr) {
			m_gameVersions->addListener(*this);
		}
	}

	return *this;
}

OrganizedModCollection & OrganizedModCollection::operator = (const OrganizedModCollection & m) {
	ModCollectionListener::operator = (m);

	if(m_mods != nullptr) {
		m_mods->removeListener(*this);
	}

	if(m_favourites != nullptr) {
		m_favourites->removeListener(*this);
	}

	if(m_gameVersions != nullptr) {
		m_gameVersions->removeListener(*this);
	}

	m_teams.clear();
	m_authors.clear();

	m_filterType = m.m_filterType;
	m_sortType = m.m_sortType;
	m_sortDirection = m.m_sortDirection;
	m_mods = m.m_mods;
	m_favourites = m.m_favourites;
	m_gameVersions = m.m_gameVersions;
	m_selectedTeam.reset();
	m_selectedAuthor.reset();

	if(m_mods != nullptr) {
		m_mods->addListener(*this);

		updateGameVersionList();
		updateTeamList();
		updateAuthorList();

		if(m.m_selectedTeam != nullptr) {
			m_selectedTeam = getTeamInfo(m.m_selectedTeam->getName());
		}

		if(m.m_selectedAuthor != nullptr) {
			m_selectedAuthor = getAuthorInfo(m.m_selectedAuthor->getName());
		}
	}

	if(m_favourites != nullptr) {
		m_favourites->addListener(*this);
	}

	if(m_gameVersions != nullptr) {
		m_gameVersions->addListener(*this);
	}

	organize();

	return *this;
}

OrganizedModCollection::~OrganizedModCollection() {
	if(m_mods != nullptr) {
		m_mods->removeListener(*this);
	}

	if(m_favourites != nullptr) {
		m_favourites->removeListener(*this);
	}

	if(m_gameVersions != nullptr) {
		m_gameVersions->removeListener(*this);
	}
}

std::shared_ptr<ModCollection> OrganizedModCollection::getModCollection() const {
	return m_mods;
}

const std::vector<std::shared_ptr<Mod>> & OrganizedModCollection::getOrganizedMods() const {
	return m_organizedMods;
}

const std::vector<std::shared_ptr<GameVersion>> & OrganizedModCollection::getOrganizedGameVersions() const {
	return m_organizedGameVersions;
}

std::shared_ptr<FavouriteModCollection> OrganizedModCollection::getFavouriteModCollection() const {
	return m_favourites;
}

std::shared_ptr<GameVersionCollection> OrganizedModCollection::getGameVersionCollection() const {
	return m_gameVersions;
}

std::vector<std::string> OrganizedModCollection::getOrganizedItemDisplayNames(bool prependItemNumber) const {
	std::vector<std::string> organizedItemDisplayNames;

	if(shouldDisplayMods()) {
		organizedItemDisplayNames.reserve(m_organizedMods.size());

		for(size_t i = 0; i < m_organizedMods.size(); i++) {
			std::stringstream modTextStream;

			if(prependItemNumber) {
				modTextStream << i + 1 << ": ";
			}

			modTextStream << m_organizedMods[i]->getName();

			if(m_sortType == OrganizedModCollection::SortType::InitialReleaseDate || m_sortType == OrganizedModCollection::SortType::LatestReleaseDate) {
				std::string releaseDate(m_sortType == OrganizedModCollection::SortType::InitialReleaseDate ? m_organizedMods[i]->getInitialReleaseDateAsString() : m_organizedMods[i]->getLatestReleaseDateAsString());

				if(!releaseDate.empty()) {
					modTextStream << " (" << releaseDate << ")";
				}
			}
			else if(m_sortType == OrganizedModCollection::SortType::NumberOfVersions) {
				modTextStream << " (" << std::to_string(m_organizedMods[i]->numberOfVersions()) << ")";
			}

			organizedItemDisplayNames.push_back(modTextStream.str());
		}
	}
	else if(shouldDisplayGameVersions()) {
		organizedItemDisplayNames.reserve(m_organizedGameVersions.size());

		for(size_t i = 0; i < m_organizedGameVersions.size(); i++) {
			std::stringstream gameVersionTextStream;

			if(prependItemNumber) {
				gameVersionTextStream << std::to_string(i + 1) << ": ";
			}

			gameVersionTextStream << m_organizedGameVersions[i]->getName();

			if(m_sortType == OrganizedModCollection::SortType::NumberOfSupportedMods ||
			   (m_filterType == OrganizedModCollection::FilterType::SupportedGameVersions && m_sortType != OrganizedModCollection::SortType::NumberOfCompatibleMods)) {
				gameVersionTextStream << " (" << std::to_string(getSupportedModCountForGameVersion(m_organizedGameVersions[i]->getName())) << ")";
			}
			else if(m_sortType == OrganizedModCollection::SortType::NumberOfCompatibleMods || m_filterType == OrganizedModCollection::FilterType::CompatibleGameVersions) {
				gameVersionTextStream << " (" << std::to_string(getCompatibleModCountForGameVersion(m_organizedGameVersions[i]->getName())) << ")";
			}

			organizedItemDisplayNames.push_back(gameVersionTextStream.str());
		}
	}
	else if(shouldDisplayTeams()) {
		organizedItemDisplayNames.reserve(m_teams.size());

		for(size_t i = 0; i < m_teams.size(); i++) {
			std::stringstream teamTextStream;

			if(prependItemNumber) {
				teamTextStream << std::to_string(i + 1) << ": ";
			}

			teamTextStream << m_teams[i]->getName() << " (" << std::to_string(m_teams[i]->getModCount()) << ")";

			organizedItemDisplayNames.push_back(teamTextStream.str());
		}
	}
	else if(shouldDisplayAuthors()) {
		organizedItemDisplayNames.reserve(m_authors.size());

		for(size_t i = 0; i < m_authors.size(); i++) {
			std::stringstream authorTextStream;

			if(prependItemNumber) {
				authorTextStream << std::to_string(i + 1) << ": ";
			}

			authorTextStream << m_authors[i]->getName() << " (" << std::to_string(m_authors[i]->getModCount()) << ")";

			organizedItemDisplayNames.push_back(authorTextStream.str());
		}
	}

	return organizedItemDisplayNames;
}

OrganizedModCollection::FilterType OrganizedModCollection::getFilterType() const {
	return m_filterType;
}

OrganizedModCollection::SortType OrganizedModCollection::getSortType() const {
	return m_sortType;
}

OrganizedModCollection::SortDirection OrganizedModCollection::getSortDirection() const {
	return m_sortDirection;
}

void OrganizedModCollection::setModCollection(std::shared_ptr<ModCollection> mods) {
	bool shouldOrganize = m_mods != mods;

	if(m_mods != nullptr) {
		m_mods->removeListener(*this);
	}

	m_mods = mods;

	if(m_mods != nullptr) {
		m_mods->addListener(*this);

		updateGameVersionList();
		updateTeamList();
		updateAuthorList();
	}

	if(shouldOrganize) {
		organize();
	}
	else {
		notifyOrganizedModCollectionChanged();
	}
}

void OrganizedModCollection::setFavouriteModCollection(std::shared_ptr<FavouriteModCollection> favourites) {
	bool shouldOrganize = m_favourites != favourites &&
						  m_filterType == FilterType::Favourites;

	if(m_favourites != nullptr) {
		m_favourites->removeListener(*this);
	}

	m_favourites = favourites;

	if(m_favourites != nullptr) {
		m_favourites->addListener(*this);
	}

	if(shouldOrganize) {
		organize();
	}
	else {
		notifyOrganizedModCollectionChanged();
	}
}

void OrganizedModCollection::setGameVersionCollection(std::shared_ptr<GameVersionCollection> gameVersions) {
	bool shouldOrganize = m_gameVersions != gameVersions &&
						  (m_filterType == FilterType::SupportedGameVersions || m_filterType == FilterType::CompatibleGameVersions);

	if(m_gameVersions != nullptr) {
		m_gameVersions->removeListener(*this);
	}

	m_gameVersions = gameVersions;

	if(m_gameVersions != nullptr) {
		m_gameVersions->addListener(*this);
	}

	if(shouldOrganize) {
		organize();
	}
	else {
		notifyOrganizedModGameVersionCollectionChanged();
	}
}

bool OrganizedModCollection::setFilterType(FilterType filterType) {
	bool shouldOrganize = m_filterType != filterType;

	if(m_filterType != filterType) {
		m_filterType = filterType;

		notifyFilterTypeChanged();
	}

	if(filterType != FilterType::Teams && m_selectedTeam != nullptr) {
		m_selectedTeam = nullptr;

		notifySelectedTeamChanged();
	}

	if(filterType != FilterType::Authors && m_selectedAuthor != nullptr) {
		m_selectedAuthor = nullptr;

		notifySelectedAuthorChanged();
	}

	if(!areCurrentSortOptionsValidInCurrentContext()) {
		m_sortType = OrganizedModCollection::DEFAULT_SORT_TYPE;
		m_sortDirection = OrganizedModCollection::DEFAULT_SORT_DIRECTION;

		notifySortOptionsChanged();

		shouldOrganize = true;
	}

	if(shouldOrganize) {
		organize();
	}

	return true;
}

bool OrganizedModCollection::setSortType(SortType sortType) {
	if(!OrganizedModCollection::areSortOptionsValidInCurrentContext(sortType, m_filterType)) {
		return false;
	}

	if(m_sortType != sortType) {
		m_sortType = sortType;

		notifySortOptionsChanged();

		if(m_sortType == SortType::Unsorted) {
			updateAuthorList();
			updateTeamList();
			updateGameVersionList();
			applyFilter();
		}

		sort();
	}

	return true;
}

bool OrganizedModCollection::setSortDirection(SortDirection sortDirection) {
	if(m_sortDirection != sortDirection) {
		m_sortDirection = sortDirection;

		notifySortOptionsChanged();

		sort();
	}

	return true;
}

bool OrganizedModCollection::setSortOptions(SortType sortType, SortDirection sortDirection) {
	if(!OrganizedModCollection::areSortOptionsValidInCurrentContext(sortType, m_filterType)) {
		return false;
	}

	bool sortTypeChanged = m_sortType != sortType;

	if(m_sortType != sortType || m_sortDirection != sortDirection) {
		m_sortType = sortType;
		m_sortDirection = sortDirection;

		notifySortOptionsChanged();

		if(sortTypeChanged && m_sortType == SortType::Unsorted) {
			updateAuthorList();
			updateTeamList();
			updateGameVersionList();
			applyFilter();
		}

		sort();
	}

	return true;
}

bool OrganizedModCollection::shouldDisplayMods() const {
	return  m_filterType == FilterType::None ||
		    m_filterType == FilterType::Favourites ||
		   (m_filterType == FilterType::Teams && m_selectedTeam != nullptr) ||
		   (m_filterType == FilterType::Authors && m_selectedAuthor != nullptr) ||
		   ((m_filterType == FilterType::SupportedGameVersions || m_filterType == FilterType::CompatibleGameVersions) && m_selectedGameVersion != nullptr);
}

bool OrganizedModCollection::shouldDisplayGameVersions() const {
	return (m_filterType == FilterType::SupportedGameVersions || m_filterType == FilterType::CompatibleGameVersions) &&
		   m_selectedGameVersion == nullptr;
}

bool OrganizedModCollection::shouldDisplayTeams() const {
	return m_filterType == FilterType::Teams &&
		   m_selectedTeam == nullptr;
}

bool OrganizedModCollection::shouldDisplayAuthors() const {
	return m_filterType == FilterType::Authors &&
		   m_selectedAuthor == nullptr;
}

size_t OrganizedModCollection::indexOfSelectedItem() const {
	if(shouldDisplayMods()) {
		if(m_selectedMod == nullptr) {
			return std::numeric_limits<size_t>::max();
		}

		return indexOfMod(*m_selectedMod);
	}
	else if(shouldDisplayGameVersions()) {
		if(m_selectedGameVersion == nullptr) {
			return std::numeric_limits<size_t>::max();
		}

		return indexOfGameVersion(*m_selectedGameVersion);
	}
	else if(shouldDisplayTeams()) {
		if(m_selectedTeam == nullptr) {
			return std::numeric_limits<size_t>::max();
		}

		return indexOfTeamInfo(*m_selectedTeam);
	}
	else if(shouldDisplayAuthors()) {
		if(m_selectedAuthor == nullptr) {
			return std::numeric_limits<size_t>::max();
		}

		return indexOfAuthorInfo(*m_selectedAuthor);
	}

	return std::numeric_limits<size_t>::max();
}

bool OrganizedModCollection::selectItem(size_t index) {
	if(shouldDisplayMods()) {
		return setSelectedMod(index);
	}
	else if(shouldDisplayGameVersions()) {
		return setSelectedGameVersion(index);
	}
	else if(shouldDisplayTeams()) {
		return setSelectedTeam(index);
	}
	else if(shouldDisplayAuthors()) {
		return setSelectedAuthor(index);
	}

	return false;
}

bool OrganizedModCollection::selectRandomItem() {
	if(shouldDisplayMods()) {
		return selectRandomMod();
	}
	else if(shouldDisplayGameVersions()) {
		return selectRandomGameVersion();
	}
	else if(shouldDisplayTeams()) {
		return selectRandomTeam();
	}
	else if(shouldDisplayAuthors()) {
		return selectRandomAuthor();
	}

	return false;
}

void OrganizedModCollection::clearSelectedItems() {
	if(m_selectedMod != nullptr) {
		m_selectedMod = nullptr;

		notifySelectedModChanged();
	}

	if(m_selectedGameVersion != nullptr) {
		m_selectedGameVersion = nullptr;

		notifySelectedGameVersionChanged();
	}

	if(m_selectedAuthor != nullptr) {
		m_selectedAuthor = nullptr;

		notifySelectedAuthorChanged();
	}

	if(m_selectedTeam != nullptr) {
		m_selectedTeam = nullptr;

		notifySelectedTeamChanged();
	}

	if(!areCurrentSortOptionsValidInCurrentContext()) {
		m_sortType = OrganizedModCollection::DEFAULT_SORT_TYPE;
		m_sortDirection = OrganizedModCollection::DEFAULT_SORT_DIRECTION;

		notifySortOptionsChanged();

		organize();
	}
}

size_t OrganizedModCollection::numberOfMods() const {
	return m_organizedMods.size();
}

bool OrganizedModCollection::hasMod(const Mod & mod) const {
	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_organizedMods.begin(); i != m_organizedMods.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getID(), mod.getID())) {
			return true;
		}
	}

	return false;
}

bool OrganizedModCollection::hasMod(const std::string & id) const {
	if(id.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_organizedMods.begin(); i != m_organizedMods.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getID(), id)) {
			return true;
		}
	}

	return false;
}

bool OrganizedModCollection::hasModWithName(const std::string & name) const {
	if(name.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_organizedMods.begin(); i != m_organizedMods.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			return true;
		}
	}

	return false;
}

size_t OrganizedModCollection::indexOfMod(const Mod & mod) const {
	for(size_t i = 0; i < m_organizedMods.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_organizedMods[i]->getID(), mod.getID())) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t OrganizedModCollection::indexOfMod(const std::string & id) const {
	if(id.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_organizedMods.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_organizedMods[i]->getID(), id)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t OrganizedModCollection::indexOfModWithName(const std::string & name) const {
	if(name.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_organizedMods.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_organizedMods[i]->getName(), name)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<Mod> OrganizedModCollection::getMod(size_t index) const {
	if(index >= m_organizedMods.size()) {
		return nullptr;
	}

	return m_organizedMods[index];
}

std::shared_ptr<Mod> OrganizedModCollection::getMod(const std::string & id) const {
	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_organizedMods.begin(); i != m_organizedMods.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getID(), id)) {
			return *i;
		}
	}

	return nullptr;
}

std::shared_ptr<Mod> OrganizedModCollection::getModWithName(const std::string & name) const {
	for(std::vector<std::shared_ptr<Mod>>::const_iterator i = m_organizedMods.begin(); i != m_organizedMods.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			return *i;
		}
	}

	return nullptr;
}

std::shared_ptr<Mod> OrganizedModCollection::getSelectedMod() const {
	return m_selectedMod;
}

bool OrganizedModCollection::setSelectedMod(size_t index) {
	if(index >= m_organizedMods.size()) {
		return false;
	}

	if(m_selectedMod != m_organizedMods[index]) {
		m_selectedMod = m_organizedMods[index];

		notifySelectedModChanged();
	}

	return true;
}

bool OrganizedModCollection::setSelectedMod(const std::string & name) {
	if(name.empty()) {
		clearSelectedMod();

		return true;
	}

	return setSelectedMod(indexOfModWithName(name));
}

bool OrganizedModCollection::setSelectedMod(const Mod * mod) {
	if(mod == nullptr) {
		clearSelectedMod();

		return true;
	}

	return setSelectedMod(mod->getName());
}

bool OrganizedModCollection::selectRandomMod() {
	if(m_organizedMods.empty()) {
		return false;
	}

	setSelectedMod(Utilities::randomInteger(0, m_organizedMods.size() - 1));

	return true;
}

void OrganizedModCollection::clearSelectedMod() {
	if(m_selectedMod != nullptr) {
		m_selectedMod = nullptr;

		notifySelectedModChanged();
	}
}

size_t OrganizedModCollection::numberOfGameVersions() const {
	return m_organizedGameVersions.size();
}

bool OrganizedModCollection::hasGameVersion(const GameVersion & gameVersion) const {
	return indexOfGameVersion(gameVersion.getName()) != std::numeric_limits<size_t>::max();
}

bool OrganizedModCollection::hasGameVersion(const std::string & gameVersion) const {
	return indexOfGameVersion(gameVersion) != std::numeric_limits<size_t>::max();
}

size_t OrganizedModCollection::indexOfGameVersion(const GameVersion & gameVersion) const {
	return indexOfGameVersion(gameVersion.getName());
}

size_t OrganizedModCollection::indexOfGameVersion(const std::string & gameVersion) const {
	if(gameVersion.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_organizedGameVersions.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_organizedGameVersions[i]->getName(), gameVersion)) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<GameVersion> OrganizedModCollection::getGameVersion(size_t index) const {
	if(index >= m_organizedGameVersions.size()) {
		return nullptr;
	}

	return m_organizedGameVersions[index];
}

std::shared_ptr<GameVersion> OrganizedModCollection::getGameVersion(const std::string & gameVersion) const {
	size_t gameVersionIndex = indexOfGameVersion(gameVersion);

	return gameVersionIndex == std::numeric_limits<size_t>::max() ? nullptr : m_organizedGameVersions[gameVersionIndex];
}

bool OrganizedModCollection::hasSelectedGameVersion() const {
	return m_selectedGameVersion != nullptr;
}

std::shared_ptr<GameVersion> OrganizedModCollection::getSelectedGameVersion() const {
	return m_selectedGameVersion;
}

bool OrganizedModCollection::setSelectedGameVersion(size_t index) {
	if(index >= m_organizedGameVersions.size()) {
		return false;
	}

	bool shouldOrganize = false;

	if(m_selectedGameVersion != m_organizedGameVersions[index]) {
		m_selectedGameVersion = m_organizedGameVersions[index];

		notifySelectedGameVersionChanged();

		shouldOrganize = true;
	}

	if(!areCurrentSortOptionsValidInCurrentContext()) {
		m_sortType = OrganizedModCollection::DEFAULT_SORT_TYPE;
		m_sortDirection = OrganizedModCollection::DEFAULT_SORT_DIRECTION;

		notifySortOptionsChanged();

		shouldOrganize = true;
	}

	if(shouldOrganize) {
		organize();
	}

	return true;
}

bool OrganizedModCollection::setSelectedGameVersion(const std::string & gameVersion) {
	if(gameVersion.empty()) {
		clearSelectedGameVersion();

		return true;
	}

	return setSelectedGameVersion(indexOfGameVersion(gameVersion));
}

bool OrganizedModCollection::setSelectedGameVersion(const GameVersion * gameVersion) {
	if(gameVersion == nullptr) {
		clearSelectedGameVersion();

		return true;
	}

	return setSelectedGameVersion(gameVersion->getName());
}

bool OrganizedModCollection::selectRandomGameVersion() {
	if(m_gameVersions->numberOfGameVersions() == 0) {
		return false;
	}

	return setSelectedGameVersion(Utilities::randomInteger(0, m_gameVersions->numberOfGameVersions() - 1));
}

void OrganizedModCollection::clearSelectedGameVersion() {
	bool shouldOrganize = false;

	if(m_selectedGameVersion != nullptr) {
		m_selectedGameVersion = nullptr;

		notifySelectedGameVersionChanged();

		shouldOrganize = true;
	}

	if(!areCurrentSortOptionsValidInCurrentContext()) {
		m_sortType = OrganizedModCollection::DEFAULT_SORT_TYPE;
		m_sortDirection = OrganizedModCollection::DEFAULT_SORT_DIRECTION;

		notifySortOptionsChanged();

		shouldOrganize = true;
	}

	if(shouldOrganize) {
		organize();
	}
}

size_t OrganizedModCollection::getSupportedModCountForGameVersion(const GameVersion & gameVersion) const {
	return getSupportedModCountForGameVersion(gameVersion.getName());
}

size_t OrganizedModCollection::getSupportedModCountForGameVersion(const std::string & gameVersion) const {
	std::map<std::string, size_t>::const_iterator gameVersionSupportedModCount(m_gameVersionSupportedModCountMap.find(gameVersion));

	return gameVersionSupportedModCount == m_gameVersionSupportedModCountMap.end() ? 0 : gameVersionSupportedModCount->second;
}

size_t OrganizedModCollection::getCompatibleModCountForGameVersion(const GameVersion & gameVersion) const {
	return getCompatibleModCountForGameVersion(gameVersion.getName());
}

size_t OrganizedModCollection::getCompatibleModCountForGameVersion(const std::string & gameVersion) const {
	std::map<std::string, size_t>::const_iterator gameVersionCompatibleModCount(m_gameVersionCompatibleModCountMap.find(gameVersion));

	return gameVersionCompatibleModCount == m_gameVersionCompatibleModCountMap.end() ? 0 : gameVersionCompatibleModCount->second;
}

size_t OrganizedModCollection::numberOfTeams() const {
	return m_teams.size();
}

bool OrganizedModCollection::hasTeamInfo(const std::string & name) const {
	if(name.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModAuthorInformation>>::const_iterator i = m_teams.begin(); i != m_teams.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			return true;
		}
	}

	return false;
}

size_t OrganizedModCollection::indexOfTeamInfo(const ModAuthorInformation & teamInfo) const {
	return indexOfTeamInfo(teamInfo.getName());
}

size_t OrganizedModCollection::indexOfTeamInfo(const std::string & name) const {
	for(std::vector<std::shared_ptr<ModAuthorInformation>>::const_iterator i = m_teams.begin(); i != m_teams.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			return i - m_teams.begin();
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<ModAuthorInformation> OrganizedModCollection::getTeamInfo(size_t index) const {
	if(index >= m_teams.size()) {
		return nullptr;
	}

	return m_teams[index];
}

std::shared_ptr<ModAuthorInformation> OrganizedModCollection::getTeamInfo(const std::string & name) const {
	if(name.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<ModAuthorInformation>>::const_iterator i = m_teams.begin(); i != m_teams.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			return *i;
		}
	}

	return nullptr;
}

void OrganizedModCollection::incrementTeamModCount(size_t index) {
	if(index >= m_teams.size()) {
		return;
	}

	m_teams[index]->incrementModCount();
}

void OrganizedModCollection::incrementTeamModCount(const std::string & name) {
	if(name.empty()) {
		return;
	}

	for(std::vector<std::shared_ptr<ModAuthorInformation>>::const_iterator i = m_teams.begin(); i != m_teams.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			(*i)->incrementModCount();
			return;
		}
	}

	m_teams.push_back(std::make_shared<ModAuthorInformation>(name));
}

bool OrganizedModCollection::hasSelectedTeam() const {
	return m_selectedTeam != nullptr;
}

std::shared_ptr<ModAuthorInformation> OrganizedModCollection::getSelectedTeam() const {
	return m_selectedTeam;
}

bool OrganizedModCollection::setSelectedTeam(size_t index) {
	if(index >= m_teams.size()) {
		return false;
	}

	bool shouldOrganize = false;

	if(m_selectedTeam != m_teams[index]) {
		m_selectedTeam = m_teams[index];

		notifySelectedTeamChanged();

		shouldOrganize = true;
	}

	if(m_filterType != FilterType::Teams) {
		m_filterType = FilterType::Teams;

		notifyFilterTypeChanged();

		shouldOrganize = true;
	}

	if(!areCurrentSortOptionsValidInCurrentContext()) {
		m_sortType = OrganizedModCollection::DEFAULT_SORT_TYPE;
		m_sortDirection = OrganizedModCollection::DEFAULT_SORT_DIRECTION;

		notifySortOptionsChanged();

		shouldOrganize = true;
	}

	if(shouldOrganize) {
		organize();
	}

	return true;
}

bool OrganizedModCollection::setSelectedTeam(const std::string & name) {
	if(name.empty()) {
		clearSelectedTeam();

		return true;
	}

	return setSelectedTeam(indexOfTeamInfo(name));
}

bool OrganizedModCollection::setSelectedTeam(const ModAuthorInformation * teamInfo) {
	if(teamInfo == nullptr) {
		clearSelectedTeam();

		return true;
	}

	return setSelectedTeam(indexOfTeamInfo(*teamInfo));
}

bool OrganizedModCollection::selectRandomTeam() {
	if(m_teams.empty()) {
		return false;
	}

	return setSelectedTeam(Utilities::randomInteger(0, m_teams.size() - 1));
}

void OrganizedModCollection::clearSelectedTeam() {
	bool shouldOrganize = false;

	if(m_selectedTeam != nullptr) {
		m_selectedTeam = nullptr;

		notifySelectedTeamChanged();

		shouldOrganize = true;
	}

	if(!areCurrentSortOptionsValidInCurrentContext()) {
		m_sortType = OrganizedModCollection::DEFAULT_SORT_TYPE;
		m_sortDirection = OrganizedModCollection::DEFAULT_SORT_DIRECTION;

		notifySortOptionsChanged();

		shouldOrganize = true;
	}

	if(shouldOrganize) {
		organize();
	}
}

size_t OrganizedModCollection::numberOfAuthors() const {
	return m_authors.size();
}

bool OrganizedModCollection::hasAuthorInfo(const std::string & name) const {
	if(name.empty()) {
		return false;
	}

	for(std::vector<std::shared_ptr<ModAuthorInformation>>::const_iterator i = m_authors.begin(); i != m_authors.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			return true;
		}
	}

	return false;
}

size_t OrganizedModCollection::indexOfAuthorInfo(const ModAuthorInformation & authorInfo) const {
	return indexOfAuthorInfo(authorInfo.getName());
}

size_t OrganizedModCollection::indexOfAuthorInfo(const std::string & name) const {
	for(std::vector<std::shared_ptr<ModAuthorInformation>>::const_iterator i = m_authors.begin(); i != m_authors.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			return i - m_authors.begin();
		}
	}

	return std::numeric_limits<size_t>::max();
}

std::shared_ptr<ModAuthorInformation> OrganizedModCollection::getAuthorInfo(size_t index) const {
	if(index >= m_authors.size()) {
		return nullptr;
	}

	return m_authors[index];
}

std::shared_ptr<ModAuthorInformation> OrganizedModCollection::getAuthorInfo(const std::string & name) const {
	if(name.empty()) {
		return nullptr;
	}

	for(std::vector<std::shared_ptr<ModAuthorInformation>>::const_iterator i = m_authors.begin(); i != m_authors.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			return *i;
		}
	}

	return nullptr;
}

void OrganizedModCollection::incrementAuthorModCount(size_t index) {
	if(index >= m_authors.size()) {
		return;
	}

	m_authors[index]->incrementModCount();
}

void OrganizedModCollection::incrementAuthorModCount(const std::string & name) {
	if(name.empty()) {
		return;
	}

	for(std::vector<std::shared_ptr<ModAuthorInformation>>::const_iterator i = m_authors.begin(); i != m_authors.end(); ++i) {
		if(Utilities::areStringsEqualIgnoreCase((*i)->getName(), name)) {
			(*i)->incrementModCount();

			return;
		}
	}

	m_authors.push_back(std::make_shared<ModAuthorInformation>(name));
}

bool OrganizedModCollection::hasSelectedAuthor() const {
	return m_selectedAuthor != nullptr;
}

std::shared_ptr<ModAuthorInformation> OrganizedModCollection::getSelectedAuthor() const {
	return m_selectedAuthor;
}

bool OrganizedModCollection::setSelectedAuthor(size_t index) {
	if(index >= m_authors.size()) {
		return false;
	}

	bool shouldOrganize = m_selectedAuthor != m_authors[index] ||
						  m_filterType != FilterType::Authors;

	m_selectedAuthor = m_authors[index];
	m_filterType = FilterType::Authors;

	if(!areCurrentSortOptionsValidInCurrentContext()) {
		m_sortType = OrganizedModCollection::DEFAULT_SORT_TYPE;
		m_sortDirection = OrganizedModCollection::DEFAULT_SORT_DIRECTION;

		shouldOrganize = true;
	}

	if(shouldOrganize) {
		organize();
	}

	return true;
}

bool OrganizedModCollection::setSelectedAuthor(const std::string & name) {
	if(name.empty()) {
		clearSelectedAuthor();

		return true;
	}

	return setSelectedAuthor(indexOfAuthorInfo(name));
}

bool OrganizedModCollection::setSelectedAuthor(const ModAuthorInformation * authorInfo) {
	if(authorInfo == nullptr) {
		clearSelectedAuthor();

		return true;
	}

	return setSelectedAuthor(indexOfAuthorInfo(*authorInfo));
}

bool OrganizedModCollection::selectRandomAuthor() {
	if(m_authors.empty()) {
		return false;
	}

	return setSelectedAuthor(Utilities::randomInteger(0, m_authors.size() - 1));
}

void OrganizedModCollection::clearSelectedAuthor() {
	bool shouldOrganize = false;

	if(m_selectedAuthor != nullptr) {
		m_selectedAuthor = nullptr;

		notifySelectedAuthorChanged();

		shouldOrganize = true;
	}

	if(!areCurrentSortOptionsValidInCurrentContext()) {
		m_sortType = OrganizedModCollection::DEFAULT_SORT_TYPE;
		m_sortDirection = OrganizedModCollection::DEFAULT_SORT_DIRECTION;

		notifySortOptionsChanged();

		shouldOrganize = true;
	}

	if(shouldOrganize) {
		organize();
	}
}

void OrganizedModCollection::modCollectionUpdated() {
	updateGameVersionModCounts();
	updateTeamList();
	updateAuthorList();

	organize();
}

void OrganizedModCollection::favouriteModCollectionUpdated() {
	if(m_filterType == FilterType::Favourites) {
		organize();
	}
}

void OrganizedModCollection::gameVersionCollectionUpdated() {
	updateGameVersionList();

	if(m_filterType == FilterType::SupportedGameVersions || m_filterType == FilterType::CompatibleGameVersions) {
		organize();
	}
}

void OrganizedModCollection::organize() {
	if(m_mods == nullptr) {
		return;
	}

	applyFilter();

	sort();
}

void OrganizedModCollection::applyFilter() {
	if(m_mods == nullptr) {
		return;
	}

	m_organizedMods.clear();

	if(m_filterType == FilterType::Favourites &&
	   (m_favourites == nullptr || m_favourites->numberOfFavourites() == 0)) {
		return;
	}

	if((m_filterType == FilterType::SupportedGameVersions || m_filterType == FilterType::CompatibleGameVersions) &&
	   (m_gameVersions == nullptr || m_gameVersions->numberOfGameVersions() == 0)) {
		return;
	}

	if(m_filterType == FilterType::None) {
		for(size_t i = 0; i < m_mods->numberOfMods(); i++) {
			m_organizedMods.push_back(m_mods->getMod(i));
		}
	}
	else if(m_filterType == FilterType::Favourites) {
		for(size_t i = 0; i < m_mods->numberOfMods(); i++) {
			if(m_favourites->hasFavourite(m_mods->getMod(i)->getName())) {
				m_organizedMods.push_back(m_mods->getMod(i));
			}
		}
	}
	else if(m_filterType == FilterType::SupportedGameVersions) {
		if(m_selectedGameVersion != nullptr) {
			std::shared_ptr<Mod> mod;

			for(size_t i = 0; i < m_mods->numberOfMods(); i++) {
				mod = m_mods->getMod(i);

				if(mod->isGameVersionSupported(*m_selectedGameVersion)) {
					m_organizedMods.push_back(mod);
				}
			}
		}
	}
	else if(m_filterType == FilterType::CompatibleGameVersions) {
		if(m_selectedGameVersion != nullptr) {
			std::shared_ptr<Mod> mod;

			for(size_t i = 0; i < m_mods->numberOfMods(); i++) {
				mod = m_mods->getMod(i);

				if(mod->isGameVersionCompatible(*m_selectedGameVersion)) {
					m_organizedMods.push_back(mod);
				}
			}
		}
	}
	else if(m_filterType == FilterType::Teams) {
		if(m_selectedTeam != nullptr) {
			std::shared_ptr<ModTeam> team;

			for(size_t i = 0; i < m_mods->numberOfMods(); i++) {
				team = m_mods->getMod(i)->getTeam();

				if(team != nullptr && Utilities::areStringsEqual(team->getName(), m_selectedTeam->getName())) {
					m_organizedMods.push_back(m_mods->getMod(i));
				}
			}
		}
	}
	else if(m_filterType == FilterType::Authors) {
		if(m_selectedAuthor != nullptr) {
			std::shared_ptr<ModTeam> team;

			for(size_t i = 0; i < m_mods->numberOfMods(); i++) {
				team = m_mods->getMod(i)->getTeam();

				if(team != nullptr && team->numberOfMembers() != 0) {
					for(size_t j=0;j<team->numberOfMembers();j++) {
						if(Utilities::areStringsEqual(team->getMember(j)->getName(), m_selectedAuthor->getName())) {
							m_organizedMods.push_back(m_mods->getMod(i));
						}
					}
				}
			}
		}
	}

	notifyOrganizedModCollectionChanged();
}

void OrganizedModCollection::sort() {
	if(m_mods == nullptr || m_sortType == SortType::Unsorted) {
		return;
	}

	switch(m_filterType) {
		case FilterType::None:
		case FilterType::Favourites:
		case FilterType::SupportedGameVersions:
		case FilterType::CompatibleGameVersions: {
			m_organizedMods = mergeSortMods(m_organizedMods);
			m_organizedGameVersions = mergeSortGameVersions(m_organizedGameVersions);

			notifyOrganizedModCollectionChanged();
			notifyOrganizedModGameVersionCollectionChanged();

			break;
		}
		case FilterType::Teams: {
			m_teams = mergeSortAuthors(m_teams);

			notifyOrganizedModTeamCollectionChanged();

			break;
		}
		case FilterType::Authors: {
			m_authors = mergeSortAuthors(m_authors);

			notifyOrganizedModAuthorCollectionChanged();

			break;
		}
	}
}

void OrganizedModCollection::updateGameVersionList() {
	if(m_gameVersions == nullptr) {
		return;
	}

	m_organizedGameVersions.clear();

	for(size_t i = 0; i < m_gameVersions->numberOfGameVersions(); i++) {
		m_organizedGameVersions.push_back(m_gameVersions->getGameVersion(i));
	}

	updateGameVersionModCounts();

	notifyOrganizedModGameVersionCollectionChanged();
}

void OrganizedModCollection::updateTeamList() {
	if(m_mods == nullptr) {
		return;
	}

	m_teams.clear();

	std::shared_ptr<ModTeam> team;

	for(size_t i = 0; i < m_mods->numberOfMods(); i++) {
		team = m_mods->getMod(i)->getTeam();

		if(team != nullptr) {
			incrementTeamModCount(team->getName());
		}
	}

	notifyOrganizedModTeamCollectionChanged();
}

void OrganizedModCollection::updateAuthorList() {
	if(m_mods == nullptr) {
		return;
	}

	m_authors.clear();

	std::shared_ptr<ModTeam> team;

	for(size_t i = 0; i < m_mods->numberOfMods(); i++) {
		team = m_mods->getMod(i)->getTeam();

		if(team != nullptr) {
			for(size_t j = 0; j < team->numberOfMembers(); j++) {
				incrementAuthorModCount(team->getMember(j)->getName());
			}
		}
	}

	notifyOrganizedModAuthorCollectionChanged();
}

void OrganizedModCollection::updateGameVersionModCounts() {
	updateGameVersionSupportedModCounts();
	updateGameVersionCompatibleModCounts();
}

void OrganizedModCollection::updateGameVersionSupportedModCounts() {
	if(m_gameVersions == nullptr) {
		return;
	}

	std::shared_ptr<Mod> mod;
	std::shared_ptr<GameVersion> gameVersion;

	m_gameVersionSupportedModCountMap.clear();

	for(size_t i = 0; i < m_gameVersions->numberOfGameVersions(); i++) {
		gameVersion = m_gameVersions->getGameVersion(i);

		m_gameVersionSupportedModCountMap[gameVersion->getName()] = 0;

		for(size_t j = 0; j < m_mods->numberOfMods(); j++) {
			mod = m_mods->getMod(j);

			if(mod->isGameVersionSupported(*gameVersion)) {
				m_gameVersionSupportedModCountMap[gameVersion->getName()]++;
			}
		}
	}
}

void OrganizedModCollection::updateGameVersionCompatibleModCounts() {
	if(m_gameVersions == nullptr) {
		return;
	}

	std::shared_ptr<Mod> mod;
	std::shared_ptr<GameVersion> gameVersion;

	m_gameVersionCompatibleModCountMap.clear();

	for(size_t i = 0; i < m_gameVersions->numberOfGameVersions(); i++) {
		gameVersion = m_gameVersions->getGameVersion(i);

		m_gameVersionCompatibleModCountMap[gameVersion->getName()] = 0;

		for(size_t j = 0; j < m_mods->numberOfMods(); j++) {
			mod = m_mods->getMod(j);

			if(mod->isGameVersionCompatible(*gameVersion)) {
				m_gameVersionCompatibleModCountMap[gameVersion->getName()]++;
			}
		}
	}
}

bool OrganizedModCollection::areCurrentSortOptionsValidInCurrentContext() {
	return areSortOptionsValidInContext(m_sortType, m_filterType, m_selectedGameVersion != nullptr, m_selectedTeam != nullptr, m_selectedAuthor != nullptr);
}

bool OrganizedModCollection::areSortOptionsValidInCurrentContext(SortType sortType, FilterType filterType) {
	return areSortOptionsValidInContext(sortType, filterType, m_selectedGameVersion != nullptr, m_selectedTeam != nullptr, m_selectedAuthor != nullptr);
}

bool OrganizedModCollection::areSortOptionsValidInContext(SortType sortType, FilterType filterType, bool hasSelectedGameVersion, bool hasSelectedTeam, bool hasSelectedAuthor) {
	if(sortType == SortType::Unsorted ||
	   sortType == SortType::Name ||
	   sortType == SortType::Random) {
		return true;
	}

	if(  filterType == FilterType::None ||
	     filterType == FilterType::Favourites ||
	   ((filterType == FilterType::SupportedGameVersions || filterType == FilterType::CompatibleGameVersions) && hasSelectedGameVersion) ||
	    (filterType == FilterType::Teams && hasSelectedTeam) ||
	    (filterType == FilterType::Authors && hasSelectedAuthor)) {
		return sortType == SortType::InitialReleaseDate ||
			   sortType == SortType::LatestReleaseDate ||
			   sortType == SortType::NumberOfVersions;
	}
	else if((filterType == FilterType::SupportedGameVersions || filterType == FilterType::CompatibleGameVersions) && !hasSelectedGameVersion) {
		return  sortType == SortType::NumberOfSupportedMods ||
			   sortType == SortType::NumberOfCompatibleMods;
	}
	else if((filterType == FilterType::Teams && !hasSelectedTeam) ||
			(filterType == FilterType::Authors && !hasSelectedAuthor)) {
		return sortType == SortType::NumberOfMods;
	}

	return false;
}

std::vector<std::shared_ptr<Mod>> OrganizedModCollection::mergeSortMods(std::vector<std::shared_ptr<Mod>> mods) {
	if(mods.size() <= 1) {
		return mods;
	}

	std::vector<std::shared_ptr<Mod>> left;
	std::vector<std::shared_ptr<Mod>> right;

	size_t mid = mods.size() / 2;

	for(size_t i = 0; i < mid; i++) {
		left.push_back(mods[i]);
	}

	for(size_t i=mid;i<mods.size();i++) {
		right.push_back(mods[i]);
	}

	left = mergeSortMods(left);
	right = mergeSortMods(right);

	return mergeMods(left, right);
}

std::vector<std::shared_ptr<Mod>> OrganizedModCollection::mergeMods(std::vector<std::shared_ptr<Mod>> left, std::vector<std::shared_ptr<Mod>> right) {
	std::vector<std::shared_ptr<Mod>> result;

	bool pushLeft = true;

	while(!left.empty() && !right.empty()) {
		if(m_sortType == SortType::Name) {
			if(m_sortDirection == SortDirection::Ascending) {
				pushLeft = Utilities::compareStringsIgnoreCase(left[0]->getName(), right[0]->getName()) <= 0;
			}
			else {
				pushLeft = Utilities::compareStringsIgnoreCase(left[0]->getName(), right[0]->getName()) > 0;
			}
		}
		else if(m_sortType == SortType::InitialReleaseDate || m_sortType == SortType::LatestReleaseDate) {
			std::optional<Date> leftReleaseDate(m_sortType == SortType::InitialReleaseDate ? left[0]->getInitialReleaseDate() : left[0]->getLatestReleaseDate());
			std::optional<Date> rightReleaseDate(m_sortType == SortType::InitialReleaseDate ? right[0]->getInitialReleaseDate() : right[0]->getLatestReleaseDate());

			if(m_sortDirection == SortDirection::Ascending) {
				if(!leftReleaseDate.has_value()) {
					pushLeft = false;
				}
				else if(!rightReleaseDate.has_value()) {
					pushLeft = true;
				}
				else {
					pushLeft = leftReleaseDate <= rightReleaseDate;
				}
			}
			else {
				if(!leftReleaseDate.has_value()) {
					pushLeft = false;
				}
				else if(!rightReleaseDate.has_value()) {
					pushLeft = true;
				}
				else {
					pushLeft = leftReleaseDate > rightReleaseDate;
				}
			}
		}
		else if(m_sortType == SortType::NumberOfVersions) {
			if(m_sortDirection == SortDirection::Ascending) {
				pushLeft = left[0]->numberOfVersions() <= right[0]->numberOfVersions();
			}
			else {
				pushLeft = left[0]->numberOfVersions() > right[0]->numberOfVersions();
			}
		}
		else if(m_sortType == SortType::Random) {
			pushLeft = Utilities::randomInteger(0, 1) == 0;
		}

		if(pushLeft) {
			result.push_back(left[0]);
			left.erase(left.begin());
		}
		else {
			result.push_back(right[0]);
			right.erase(right.begin());
		}
	}

	for(size_t i = 0; i < left.size(); i++) {
		result.push_back(left[i]);
	}

	for(size_t i = 0; i < right.size(); i++) {
		result.push_back(right[i]);
	}

	return result;
}

std::vector<std::shared_ptr<GameVersion>> OrganizedModCollection::mergeSortGameVersions(std::vector<std::shared_ptr<GameVersion>> gameVersions) {
	if(gameVersions.size() <= 1) {
		return gameVersions;
	}

	std::vector<std::shared_ptr<GameVersion>> left;
	std::vector<std::shared_ptr<GameVersion>> right;

	size_t mid = gameVersions.size() / 2;

	for(size_t i = 0; i < mid; i++) {
		left.push_back(gameVersions[i]);
	}

	for(size_t i = mid; i < gameVersions.size(); i++) {
		right.push_back(gameVersions[i]);
	}

	left = mergeSortGameVersions(left);
	right = mergeSortGameVersions(right);

	return mergeGameVersions(left, right);
}

std::vector<std::shared_ptr<GameVersion>> OrganizedModCollection::mergeGameVersions(std::vector<std::shared_ptr<GameVersion>> left, std::vector<std::shared_ptr<GameVersion>> right) {
	std::vector<std::shared_ptr<GameVersion>> result;

	bool pushLeft = true;

	while(!left.empty() && !right.empty()) {
		if(m_sortType == SortType::Name) {
			if(m_sortDirection == SortDirection::Ascending) {
				pushLeft = Utilities::compareStringsIgnoreCase(left[0]->getName(), right[0]->getName()) <= 0;
			}
			else {
				pushLeft = Utilities::compareStringsIgnoreCase(left[0]->getName(), right[0]->getName()) > 0;
			}
		}
		else if(m_sortType == SortType::NumberOfSupportedMods) {
			if(m_sortDirection == SortDirection::Ascending) {
				pushLeft = getSupportedModCountForGameVersion(left[0]->getName()) <= getSupportedModCountForGameVersion(right[0]->getName());
			}
			else {
				pushLeft = getSupportedModCountForGameVersion(left[0]->getName()) > getSupportedModCountForGameVersion(right[0]->getName());
			}
		}
		else if(m_sortType == SortType::NumberOfCompatibleMods) {
			if(m_sortDirection == SortDirection::Ascending) {
				pushLeft = getCompatibleModCountForGameVersion(left[0]->getName()) <= getCompatibleModCountForGameVersion(right[0]->getName());
			}
			else {
				pushLeft = getCompatibleModCountForGameVersion(left[0]->getName()) > getCompatibleModCountForGameVersion(right[0]->getName());
			}
		}
		else if(m_sortType == SortType::Random) {
			pushLeft = Utilities::randomInteger(0, 1) == 0;
		}

		if(pushLeft) {
			result.push_back(left[0]);
			left.erase(left.begin());
		}
		else {
			result.push_back(right[0]);
			right.erase(right.begin());
		}
	}

	for(size_t i = 0; i < left.size(); i++) {
		result.push_back(left[i]);
	}

	for(size_t i = 0; i < right.size(); i++) {
		result.push_back(right[i]);
	}

	return result;
}

std::vector<std::shared_ptr<ModAuthorInformation>> OrganizedModCollection::mergeSortAuthors(std::vector<std::shared_ptr<ModAuthorInformation>> authors) {
	if(authors.size() <= 1) {
		return authors;
	}

	std::vector<std::shared_ptr<ModAuthorInformation>> left;
	std::vector<std::shared_ptr<ModAuthorInformation>> right;

	size_t mid = authors.size() / 2;

	for(size_t i = 0; i < mid; i++) {
		left.push_back(authors[i]);
	}

	for(size_t i = mid; i < authors.size(); i++) {
		right.push_back(authors[i]);
	}

	left = mergeSortAuthors(left);
	right = mergeSortAuthors(right);

	return mergeAuthors(left, right);
}

std::vector<std::shared_ptr<ModAuthorInformation>> OrganizedModCollection::mergeAuthors(std::vector<std::shared_ptr<ModAuthorInformation>> left, std::vector<std::shared_ptr<ModAuthorInformation>> right) {
	std::vector<std::shared_ptr<ModAuthorInformation>> result;

	bool pushLeft = true;

	while(!left.empty() && !right.empty()) {
		if(m_sortType == SortType::Name) {
			if(m_sortDirection == SortDirection::Ascending) {
				pushLeft = Utilities::compareStringsIgnoreCase(left[0]->getName(), right[0]->getName()) <= 0;
			}
			else {
				pushLeft = Utilities::compareStringsIgnoreCase(left[0]->getName(), right[0]->getName()) > 0;
			}
		}
		else if(m_sortType == SortType::NumberOfMods) {
			if(m_sortDirection == SortDirection::Ascending) {
				pushLeft = left[0]->getModCount() <= right[0]->getModCount();
			}
			else {
				pushLeft = left[0]->getModCount() > right[0]->getModCount();
			}
		}
		else if(m_sortType == SortType::Random) {
			pushLeft = Utilities::randomInteger(0, 1) == 0;
		}

		if(pushLeft) {
			result.push_back(left[0]);
			left.erase(left.begin());
		}
		else {
			result.push_back(right[0]);
			right.erase(right.begin());
		}
	}

	for(size_t i = 0; i < left.size(); i++) {
		result.push_back(left[i]);
	}

	for(size_t i = 0; i < right.size(); i++) {
		result.push_back(right[i]);
	}

	return result;
}

OrganizedModCollection::Listener::~Listener() { }

void OrganizedModCollection::Listener::filterTypeChanged(FilterType filterType) { }

void OrganizedModCollection::Listener::sortOptionsChanged(SortType sortType, SortDirection sortDirection) { }

void OrganizedModCollection::Listener::selectedModChanged(const std::shared_ptr<Mod> & mod) { }

void OrganizedModCollection::Listener::selectedGameVersionChanged(const std::shared_ptr<GameVersion> & gameVersion) { }

void OrganizedModCollection::Listener::selectedTeamChanged(const std::shared_ptr<ModAuthorInformation> & team) { }

void OrganizedModCollection::Listener::selectedAuthorChanged(const std::shared_ptr<ModAuthorInformation> & author) { }

void OrganizedModCollection::Listener::organizedModCollectionChanged(const std::vector<std::shared_ptr<Mod>> & organizedMods) { }

void OrganizedModCollection::Listener::organizedModGameVersionCollectionChanged(const std::vector<std::shared_ptr<GameVersion>> & organizedMods) { }

void OrganizedModCollection::Listener::organizedModTeamCollectionChanged(const std::vector<std::shared_ptr<ModAuthorInformation>> & organizedMods) { }

void OrganizedModCollection::Listener::organizedModAuthorCollectionChanged(const std::vector<std::shared_ptr<ModAuthorInformation>> & organizedMods) { }

size_t OrganizedModCollection::numberOfListeners() const {
	return m_listeners.size();
}

bool OrganizedModCollection::hasListener(const Listener & listener) const {
	for(std::vector<Listener *>::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i) {
		if(*i == &listener) {
			return true;
		}
	}

	return false;
}

size_t OrganizedModCollection::indexOfListener(const Listener & listener) const {
	for(size_t i = 0; i < m_listeners.size(); i++) {
		if(m_listeners[i] == &listener) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

OrganizedModCollection::Listener * OrganizedModCollection::getListener(size_t index) const {
	if(index >= m_listeners.size()) {
		return nullptr;
	}

	return m_listeners[index];
}

bool OrganizedModCollection::addListener(Listener & listener) {
	if(!hasListener(listener)) {
		m_listeners.push_back(&listener);

		return true;
	}

	return false;
}

bool OrganizedModCollection::removeListener(size_t index) {
	if(index >= m_listeners.size()) {
		return false;
	}

	m_listeners.erase(m_listeners.begin() + index);

	return true;
}

bool OrganizedModCollection::removeListener(const Listener & listener) {
	for(std::vector<Listener *>::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i) {
		if(*i == &listener) {
			m_listeners.erase(i);

			return true;
		}
	}

	return false;
}

void OrganizedModCollection::clearListeners() {
	m_listeners.clear();
}

void OrganizedModCollection::notifyFilterTypeChanged() {
	for(Listener * listener : m_listeners) {
		listener->filterTypeChanged(m_filterType);
	}
}

void OrganizedModCollection::notifySortOptionsChanged() {
	for(Listener * listener : m_listeners) {
		listener->sortOptionsChanged(m_sortType, m_sortDirection);
	}
}

void OrganizedModCollection::notifySelectedModChanged() {
	for(Listener * listener : m_listeners) {
		listener->selectedModChanged(m_selectedMod);
	}
}

void OrganizedModCollection::notifySelectedGameVersionChanged() {
	for(Listener * listener : m_listeners) {
		listener->selectedGameVersionChanged(m_selectedGameVersion);
	}
}

void OrganizedModCollection::notifySelectedTeamChanged() {
	for(Listener * listener : m_listeners) {
		listener->selectedTeamChanged(m_selectedTeam);
	}
}

void OrganizedModCollection::notifySelectedAuthorChanged() {
	for(Listener * listener : m_listeners) {
		listener->selectedAuthorChanged(m_selectedAuthor);
	}
}

void OrganizedModCollection::notifyOrganizedModCollectionChanged() {
	for(Listener * listener : m_listeners) {
		listener->organizedModCollectionChanged(m_organizedMods);
	}
}

void OrganizedModCollection::notifyOrganizedModGameVersionCollectionChanged() {
	for(Listener * listener : m_listeners) {
		listener->organizedModGameVersionCollectionChanged(m_organizedGameVersions);
	}
}

void OrganizedModCollection::notifyOrganizedModTeamCollectionChanged() {
	for(Listener * listener : m_listeners) {
		listener->organizedModTeamCollectionChanged(m_teams);
	}
}

void OrganizedModCollection::notifyOrganizedModAuthorCollectionChanged() {
	for(Listener * listener : m_listeners) {
		listener->organizedModAuthorCollectionChanged(m_authors);
	}
}

bool OrganizedModCollection::operator == (const OrganizedModCollection & m) const {
	if((m_mods == nullptr && m.m_mods != nullptr) ||
	   (m_mods != nullptr && m.m_mods == nullptr) ||
	   (m_favourites == nullptr && m.m_favourites != nullptr) ||
	   (m_favourites != nullptr && m.m_favourites == nullptr) ||
	   (m_gameVersions == nullptr && m.m_gameVersions != nullptr) ||
	   (m_gameVersions != nullptr && m.m_gameVersions == nullptr) ||
	   (m_selectedTeam == nullptr && m.m_selectedTeam != nullptr) ||
	   (m_selectedTeam != nullptr && m.m_selectedTeam == nullptr) ||
	   (m_selectedAuthor == nullptr && m.m_selectedAuthor != nullptr) ||
	   (m_selectedAuthor != nullptr && m.m_selectedAuthor == nullptr) ||
	   m_organizedMods.size() != m.m_organizedMods.size() ||
	   m_organizedGameVersions.size() != m.m_organizedGameVersions.size() ||
	   m_teams.size() != m.m_teams.size() ||
	   m_authors.size() != m.m_authors.size() ||
	   m_filterType != m.m_filterType ||
	   m_sortType != m.m_sortType ||
	   m_sortDirection != m.m_sortDirection ||
	   (m_mods != nullptr && m.m_mods != nullptr && *m_mods != *m.m_mods) ||
	   (m_favourites != nullptr && m.m_favourites != nullptr && *m_favourites != *m.m_favourites) ||
	   (m_gameVersions != nullptr && m.m_gameVersions != nullptr && *m_gameVersions != *m.m_gameVersions)) {
		return false;
	}

	for(size_t i = 0; i < m_organizedMods.size(); i++) {
		if(m_organizedMods[i] != m.m_organizedMods[i]) {
			return false;
		}
	}

	for(size_t i = 0; i < m_organizedGameVersions.size(); i++) {
		if(m_organizedGameVersions[i] != m.m_organizedGameVersions[i]) {
			return false;
		}
	}

	for(size_t i = 0; i < m_teams.size(); i++) {
		if(m_teams[i] != m.m_teams[i]) {
			return false;
		}
	}

	for(size_t i = 0; i < m_authors.size(); i++) {
		if(m_authors[i] != m.m_authors[i]) {
			return false;
		}
	}

	return true;
}

bool OrganizedModCollection::operator != (const OrganizedModCollection & m) const {
	return !operator == (m);
}
