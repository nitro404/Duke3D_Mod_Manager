#include "OrganizedModCollection.h"

#include "Download/DownloadManager.h"
#include "FavouriteModCollection.h"
#include "Game/GameVersion.h"
#include "Game/GameVersionCollection.h"
#include "Mod.h"
#include "ModAuthorInformation.h"
#include "ModCollection.h"
#include "ModTeam.h"
#include "ModTeamMember.h"
#include "Mod/ModIdentifier.h"

#include <Utilities/Utilities.h>
#include <Utilities/StringUtilities.h>

#include <magic_enum.hpp>

#include <sstream>

const OrganizedModCollection::FilterType OrganizedModCollection::DEFAULT_FILTER_TYPE = FilterType::None;
const OrganizedModCollection::SortType OrganizedModCollection::DEFAULT_SORT_TYPE = SortType::Name;
const OrganizedModCollection::SortDirection OrganizedModCollection::DEFAULT_SORT_DIRECTION = SortDirection::Ascending;

OrganizedModCollection::OrganizedModCollection(std::shared_ptr<ModCollection> mods, std::shared_ptr<FavouriteModCollection> favourites, std::shared_ptr<GameVersionCollection> gameVersions)
	: m_localMode(false)
	, m_filterType(OrganizedModCollection::DEFAULT_FILTER_TYPE)
	, m_sortType(OrganizedModCollection::DEFAULT_SORT_TYPE)
	, m_sortDirection(OrganizedModCollection::DEFAULT_SORT_DIRECTION)
	, m_mods(mods)
	, m_favouriteMods(favourites)
	, m_gameVersions(gameVersions) {
	if(m_mods != nullptr) {
		m_modCollectionUpdatedConnection = m_mods->updated.connect(std::bind(&OrganizedModCollection::onModCollectionUpdated, this, std::placeholders::_1));

		updateGameVersionList();
		updateTeamList();
		updateAuthorList();
	}

	if(m_favouriteMods != nullptr) {
		m_favouriteModCollectionUpdatedConnection = m_favouriteMods->updated.connect(std::bind(&OrganizedModCollection::onFavouriteModCollectionUpdated, this, std::placeholders::_1));
	}

	if(m_gameVersions != nullptr) {
		m_gameVersionCollectionSizeChangedConnection = m_gameVersions->sizeChanged.connect(std::bind(&OrganizedModCollection::onGameVersionCollectionSizeChanged, this, std::placeholders::_1));
		m_gameVersionCollectionItemModifiedConnection = m_gameVersions->itemModified.connect(std::bind(&OrganizedModCollection::onGameVersionCollectionItemModified, this, std::placeholders::_1, std::placeholders::_2));
	}

	organize();
}

OrganizedModCollection::OrganizedModCollection(OrganizedModCollection && m) noexcept
	: m_localMode(m.m_localMode)
	, m_filterType(m.m_filterType)
	, m_sortType(m.m_sortType)
	, m_sortDirection(m.m_sortDirection)
	, m_downloadManager(m.m_downloadManager)
	, m_mods(m.m_mods == nullptr ? nullptr : std::move(m.m_mods))
	, m_favouriteMods(m.m_favouriteMods == nullptr ? nullptr : std::move(m.m_favouriteMods))
	, m_gameVersions(m.m_gameVersions == nullptr ? nullptr : std::move(m.m_gameVersions))
	, m_organizedMods(std::move(m.m_organizedMods))
	, m_organizedGameVersions(std::move(m.m_organizedGameVersions))
	, m_teams(std::move(m.m_teams))
	, m_authors(std::move(m.m_authors))
	, m_selectedTeam(m.m_selectedTeam == nullptr ? nullptr : std::move(m.m_selectedTeam))
	, m_selectedAuthor(m.m_selectedAuthor == nullptr ? nullptr : std::move(m.m_selectedAuthor)) {
	if(m_mods != nullptr) {
		m_modCollectionUpdatedConnection = m_mods->updated.connect(std::bind(&OrganizedModCollection::onModCollectionUpdated, this, std::placeholders::_1));
	}

	if(m_favouriteMods != nullptr) {
		m_favouriteModCollectionUpdatedConnection = m_favouriteMods->updated.connect(std::bind(&OrganizedModCollection::onFavouriteModCollectionUpdated, this, std::placeholders::_1));
	}

	if(m_gameVersions != nullptr) {
		m_gameVersionCollectionSizeChangedConnection = m_gameVersions->sizeChanged.connect(std::bind(&OrganizedModCollection::onGameVersionCollectionSizeChanged, this, std::placeholders::_1));
		m_gameVersionCollectionItemModifiedConnection = m_gameVersions->itemModified.connect(std::bind(&OrganizedModCollection::onGameVersionCollectionItemModified, this, std::placeholders::_1, std::placeholders::_2));
	}
}

OrganizedModCollection::OrganizedModCollection(const OrganizedModCollection & m)
	: m_localMode(m.m_localMode)
	, m_filterType(m.m_filterType)
	, m_sortType(m.m_sortType)
	, m_sortDirection(m.m_sortDirection)
	, m_downloadManager(m.m_downloadManager)
	, m_mods(m.m_mods)
	, m_favouriteMods(m.m_favouriteMods)
	, m_gameVersions(m.m_gameVersions) {
	if(m_mods != nullptr) {
		m_modCollectionUpdatedConnection = m_mods->updated.connect(std::bind(&OrganizedModCollection::onModCollectionUpdated, this, std::placeholders::_1));

		updateGameVersionList();
		updateTeamList();
		updateAuthorList();

		m_selectedTeam = m.m_selectedTeam == nullptr ? nullptr : getTeamInfo(m.m_selectedTeam->getName());
		m_selectedAuthor = m.m_selectedAuthor == nullptr ? nullptr : getAuthorInfo(m.m_selectedAuthor->getName());
	}

	if(m_favouriteMods != nullptr) {
		m_favouriteModCollectionUpdatedConnection = m_favouriteMods->updated.connect(std::bind(&OrganizedModCollection::onFavouriteModCollectionUpdated, this, std::placeholders::_1));
	}

	if(m_gameVersions != nullptr) {
		m_gameVersionCollectionSizeChangedConnection = m_gameVersions->sizeChanged.connect(std::bind(&OrganizedModCollection::onGameVersionCollectionSizeChanged, this, std::placeholders::_1));
		m_gameVersionCollectionItemModifiedConnection = m_gameVersions->itemModified.connect(std::bind(&OrganizedModCollection::onGameVersionCollectionItemModified, this, std::placeholders::_1, std::placeholders::_2));
	}

	organize();
}

OrganizedModCollection & OrganizedModCollection::operator = (OrganizedModCollection && m) noexcept {
	if(this != &m) {
		m_modCollectionUpdatedConnection.disconnect();
		m_favouriteModCollectionUpdatedConnection.disconnect();
		m_gameVersionCollectionSizeChangedConnection.disconnect();
		m_gameVersionCollectionItemModifiedConnection.disconnect();

		m_localMode = m.m_localMode;
		m_filterType = m.m_filterType;
		m_sortType = m.m_sortType;
		m_sortDirection = m.m_sortDirection;
		m_downloadManager = m.m_downloadManager;
		m_mods = m.m_mods == nullptr ? nullptr : std::move(m.m_mods);
		m_favouriteMods = m.m_favouriteMods == nullptr ? nullptr : std::move(m.m_favouriteMods);
		m_gameVersions = m.m_gameVersions == nullptr ? nullptr : std::move(m.m_gameVersions);
		m_teams = std::move(m.m_teams);
		m_authors = std::move(m.m_authors);
		m_selectedTeam = m.m_selectedTeam == nullptr ? nullptr : std::move(m.m_selectedTeam);
		m_selectedAuthor = m.m_selectedAuthor == nullptr ? nullptr : std::move(m.m_selectedAuthor);

		if(m_mods != nullptr) {
			m_modCollectionUpdatedConnection = m_mods->updated.connect(std::bind(&OrganizedModCollection::onModCollectionUpdated, this, std::placeholders::_1));
		}

		if(m_favouriteMods != nullptr) {
			m_favouriteModCollectionUpdatedConnection = m_favouriteMods->updated.connect(std::bind(&OrganizedModCollection::onFavouriteModCollectionUpdated, this, std::placeholders::_1));
		}

		if(m_gameVersions != nullptr) {
			m_gameVersionCollectionSizeChangedConnection = m_gameVersions->sizeChanged.connect(std::bind(&OrganizedModCollection::onGameVersionCollectionSizeChanged, this, std::placeholders::_1));
			m_gameVersionCollectionItemModifiedConnection = m_gameVersions->itemModified.connect(std::bind(&OrganizedModCollection::onGameVersionCollectionItemModified, this, std::placeholders::_1, std::placeholders::_2));
		}
	}

	return *this;
}

OrganizedModCollection & OrganizedModCollection::operator = (const OrganizedModCollection & m) {
	m_modCollectionUpdatedConnection.disconnect();
	m_favouriteModCollectionUpdatedConnection.disconnect();
	m_gameVersionCollectionSizeChangedConnection.disconnect();
	m_gameVersionCollectionItemModifiedConnection.disconnect();

	m_teams.clear();
	m_authors.clear();

	m_localMode = m.m_localMode;
	m_filterType = m.m_filterType;
	m_sortType = m.m_sortType;
	m_sortDirection = m.m_sortDirection;
	m_downloadManager = m.m_downloadManager;
	m_mods = m.m_mods;
	m_favouriteMods = m.m_favouriteMods;
	m_gameVersions = m.m_gameVersions;
	m_selectedTeam.reset();
	m_selectedAuthor.reset();

	if(m_mods != nullptr) {
		m_modCollectionUpdatedConnection = m_mods->updated.connect(std::bind(&OrganizedModCollection::onModCollectionUpdated, this, std::placeholders::_1));

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

	if(m_favouriteMods != nullptr) {
		m_favouriteModCollectionUpdatedConnection = m_favouriteMods->updated.connect(std::bind(&OrganizedModCollection::onFavouriteModCollectionUpdated, this, std::placeholders::_1));
	}

	if(m_gameVersions != nullptr) {
		m_gameVersionCollectionSizeChangedConnection = m_gameVersions->sizeChanged.connect(std::bind(&OrganizedModCollection::onGameVersionCollectionSizeChanged, this, std::placeholders::_1));
		m_gameVersionCollectionItemModifiedConnection = m_gameVersions->itemModified.connect(std::bind(&OrganizedModCollection::onGameVersionCollectionItemModified, this, std::placeholders::_1, std::placeholders::_2));
	}

	organize();

	return *this;
}

OrganizedModCollection::~OrganizedModCollection() {
	m_modCollectionUpdatedConnection.disconnect();
	m_favouriteModCollectionUpdatedConnection.disconnect();
	m_gameVersionCollectionSizeChangedConnection.disconnect();
	m_gameVersionCollectionItemModifiedConnection.disconnect();
}

bool OrganizedModCollection::isUsingLocalMode() const {
	return m_localMode;
}

void OrganizedModCollection::setLocalMode(bool localMode) {
	if(m_localMode == localMode) {
		return;
	}

	m_localMode = localMode;

	if(m_filterType == FilterType::Downloaded) {
		organize();
	}
}

std::shared_ptr<ModCollection> OrganizedModCollection::getModCollection() const {
	return m_mods;
}

const std::vector<std::shared_ptr<Mod>> & OrganizedModCollection::getOrganizedMods() const {
	return m_organizedMods;
}

const std::vector<std::shared_ptr<ModIdentifier>> & OrganizedModCollection::getOrganizedFavouriteMods() const {
	return m_organizedFavouriteMods;
}

const std::vector<std::shared_ptr<GameVersion>> & OrganizedModCollection::getOrganizedGameVersions() const {
	return m_organizedGameVersions;
}

const std::vector<std::shared_ptr<ModAuthorInformation>> & OrganizedModCollection::getOrganizedTeams() const {
	return m_teams;
}

const std::vector<std::shared_ptr<ModAuthorInformation>> & OrganizedModCollection::getOrganizedAuthors() const {
	return m_authors;
}

std::shared_ptr<FavouriteModCollection> OrganizedModCollection::getFavouriteModCollection() const {
	return m_favouriteMods;
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
				modTextStream << i + 1 << ". ";
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
	else if(shouldDisplayFavouriteMods()) {
		organizedItemDisplayNames.reserve(m_organizedFavouriteMods.size());

		for(size_t i = 0; i < m_organizedFavouriteMods.size(); i++) {
			std::stringstream favouriteTextStream;

			if(prependItemNumber) {
				favouriteTextStream << i + 1 << ". ";
			}

			favouriteTextStream << m_organizedFavouriteMods[i]->getFullName();

			organizedItemDisplayNames.push_back(favouriteTextStream.str());
		}
	}
	else if(shouldDisplayGameVersions()) {
		organizedItemDisplayNames.reserve(m_organizedGameVersions.size());

		for(size_t i = 0; i < m_organizedGameVersions.size(); i++) {
			std::stringstream gameVersionTextStream;

			if(prependItemNumber) {
				gameVersionTextStream << std::to_string(i + 1) << ". ";
			}

			gameVersionTextStream << m_organizedGameVersions[i]->getLongName();

			if(m_sortType == OrganizedModCollection::SortType::NumberOfSupportedMods ||
			   (m_filterType == OrganizedModCollection::FilterType::SupportedGameVersions && m_sortType != OrganizedModCollection::SortType::NumberOfCompatibleMods)) {
				gameVersionTextStream << " (" << std::to_string(getSupportedModCountForGameVersionWithID(m_organizedGameVersions[i]->getID())) << ")";
			}
			else if(m_filterType == OrganizedModCollection::FilterType::CompatibleGameVersions || m_sortType == OrganizedModCollection::SortType::NumberOfCompatibleMods) {
				gameVersionTextStream << " (" << std::to_string(getCompatibleModCountForGameVersionWithID(m_organizedGameVersions[i]->getID())) << ")";
			}

			organizedItemDisplayNames.push_back(gameVersionTextStream.str());
		}
	}
	else if(shouldDisplayTeams()) {
		organizedItemDisplayNames.reserve(m_teams.size());

		for(size_t i = 0; i < m_teams.size(); i++) {
			std::stringstream teamTextStream;

			if(prependItemNumber) {
				teamTextStream << std::to_string(i + 1) << ". ";
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
				authorTextStream << std::to_string(i + 1) << ". ";
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

void OrganizedModCollection::setDownloadManager(std::shared_ptr<DownloadManager> downloadManager) {
	m_downloadManager = downloadManager;
}

void OrganizedModCollection::setModCollection(std::shared_ptr<ModCollection> mods) {
	bool shouldOrganize = m_mods != mods;

	m_modCollectionUpdatedConnection.disconnect();

	m_mods = mods;

	if(m_mods != nullptr) {
		m_modCollectionUpdatedConnection = m_mods->updated.connect(std::bind(&OrganizedModCollection::onModCollectionUpdated, this, std::placeholders::_1));

		updateGameVersionList();
		updateTeamList();
		updateAuthorList();
	}

	if(shouldOrganize) {
		organize();
	}
	else {
		organizedModCollectionChanged(m_organizedMods);
	}
}

void OrganizedModCollection::setFavouriteModCollection(std::shared_ptr<FavouriteModCollection> favourites) {
	bool shouldOrganize = m_favouriteMods != favourites &&
						  m_filterType == FilterType::Favourites;

	m_favouriteModCollectionUpdatedConnection.disconnect();

	m_favouriteMods = favourites;

	if(m_favouriteMods != nullptr) {
		m_favouriteModCollectionUpdatedConnection = m_favouriteMods->updated.connect(std::bind(&OrganizedModCollection::onFavouriteModCollectionUpdated, this, std::placeholders::_1));
	}

	if(shouldOrganize) {
		organize();
	}
	else {
		organizedModCollectionChanged(m_organizedMods);
	}
}

void OrganizedModCollection::setGameVersionCollection(std::shared_ptr<GameVersionCollection> gameVersions) {
	bool shouldOrganize = m_gameVersions != gameVersions &&
						  (m_filterType == FilterType::SupportedGameVersions || m_filterType == FilterType::CompatibleGameVersions);

	m_gameVersionCollectionSizeChangedConnection.disconnect();
	m_gameVersionCollectionItemModifiedConnection.disconnect();

	m_gameVersions = gameVersions;

	if(m_gameVersions != nullptr) {
		m_gameVersionCollectionSizeChangedConnection = gameVersions->sizeChanged.connect(std::bind(&OrganizedModCollection::onGameVersionCollectionSizeChanged, this, std::placeholders::_1));
		m_gameVersionCollectionItemModifiedConnection = gameVersions->itemModified.connect(std::bind(&OrganizedModCollection::onGameVersionCollectionItemModified, this, std::placeholders::_1, std::placeholders::_2));
	}

	if(shouldOrganize) {
		organize();
	}
	else {
		organizedModGameVersionCollectionChanged(m_organizedGameVersions);
	}
}

bool OrganizedModCollection::setFilterType(FilterType filterType) {
	if(m_filterType == filterType) {
		return true;
	}

	m_filterType = filterType;

	filterTypeChanged(m_filterType);

	if(filterType != FilterType::Teams && m_selectedTeam != nullptr) {
		m_selectedTeam = nullptr;

		selectedTeamChanged(m_selectedTeam);
	}

	if(filterType != FilterType::Authors && m_selectedAuthor != nullptr) {
		m_selectedAuthor = nullptr;

		selectedAuthorChanged(m_selectedAuthor);
	}

	if(!areCurrentSortOptionsValidInCurrentContext()) {
		m_sortType = OrganizedModCollection::DEFAULT_SORT_TYPE;
		m_sortDirection = OrganizedModCollection::DEFAULT_SORT_DIRECTION;

		sortOptionsChanged(m_sortType, m_sortDirection);
	}

	organize();

	return true;
}

bool OrganizedModCollection::setSortType(SortType sortType) {
	if(!OrganizedModCollection::areSortOptionsValidInCurrentContext(sortType, m_filterType)) {
		return false;
	}

	if(m_sortType == sortType) {
		return true;
	}

	m_sortType = sortType;

	sortOptionsChanged(m_sortType, m_sortDirection);

	if(m_sortType == SortType::Unsorted) {
		updateAuthorList();
		updateTeamList();
		updateGameVersionList();
		applyFilter();
	}

	sort();

	return true;
}

bool OrganizedModCollection::setSortTypeByIndex(size_t sortTypeIndex) {
	const auto & sortTypes = magic_enum::enum_values<SortType>();
	size_t currentSortTypeIndex = 0;

	for(SortType currentSortType : sortTypes) {
		if(!areSortOptionsValidInCurrentContext(currentSortType, m_filterType)) {
			continue;
		}

		if(currentSortTypeIndex == sortTypeIndex) {
			return setSortType(currentSortType);
		}

		currentSortTypeIndex++;
	}

	return false;
}

bool OrganizedModCollection::setSortDirection(SortDirection sortDirection) {
	if(m_sortDirection != sortDirection) {
		m_sortDirection = sortDirection;

		sortOptionsChanged(m_sortType, m_sortDirection);

		sort();
	}

	return true;
}

bool OrganizedModCollection::setSortOptions(SortType sortType, SortDirection sortDirection) {
	if(!OrganizedModCollection::areSortOptionsValidInCurrentContext(sortType, m_filterType)) {
		return false;
	}

	bool sortTypeChanged = m_sortType != sortType;

	if(m_sortType == sortType && m_sortDirection == sortDirection) {
		return true;
	}

	m_sortType = sortType;
	m_sortDirection = sortDirection;

	sortOptionsChanged(m_sortType, m_sortDirection);

	if(sortTypeChanged && m_sortType == SortType::Unsorted) {
		updateAuthorList();
		updateTeamList();
		updateGameVersionList();
		applyFilter();
	}

	sort();

	return true;
}

bool OrganizedModCollection::shouldDisplayMods() const {
	return  m_filterType == FilterType::None ||
		    m_filterType == FilterType::Downloaded ||
		    m_filterType == FilterType::StandAlone ||
		   (m_filterType == FilterType::Teams && m_selectedTeam != nullptr) ||
		   (m_filterType == FilterType::Authors && m_selectedAuthor != nullptr) ||
		   ((m_filterType == FilterType::SupportedGameVersions || m_filterType == FilterType::CompatibleGameVersions) && m_selectedGameVersion != nullptr);
}

bool OrganizedModCollection::shouldDisplayFavouriteMods() const {
	return m_filterType == FilterType::Favourites;
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

size_t OrganizedModCollection::numberOfItems() const {
	if(shouldDisplayMods()) {
		return m_organizedMods.size();
	}
	else if(shouldDisplayFavouriteMods()) {
		return m_organizedFavouriteMods.size();
	}
	else if(shouldDisplayGameVersions()) {
		return m_organizedGameVersions.size();
	}
	else if(shouldDisplayTeams()) {
		return m_teams.size();
	}
	else if(shouldDisplayAuthors()) {
		return m_authors.size();
	}

	return 0;
}

size_t OrganizedModCollection::indexOfSelectedItem() const {
	if(shouldDisplayMods()) {
		if(m_selectedMod == nullptr) {
			return std::numeric_limits<size_t>::max();
		}

		return indexOfMod(*m_selectedMod);
	}
	else if(shouldDisplayFavouriteMods()) {
		if(m_selectedFavouriteMod == nullptr) {
			return std::numeric_limits<size_t>::max();
		}

		return indexOfFavouriteMod(*m_selectedFavouriteMod);
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
	else if(shouldDisplayFavouriteMods()) {
		return setSelectedFavouriteMod(index);
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
	else if(shouldDisplayFavouriteMods()) {
		return selectRandomFavouriteMod();
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

		selectedModChanged(m_selectedMod);
	}

	if(m_selectedFavouriteMod != nullptr) {
		m_selectedFavouriteMod = nullptr;

		selectedFavouriteModChanged(m_selectedFavouriteMod);
	}

	if(m_selectedGameVersion != nullptr) {
		m_selectedGameVersion = nullptr;

		selectedGameVersionChanged(m_selectedGameVersion);
	}

	if(m_selectedAuthor != nullptr) {
		m_selectedAuthor = nullptr;

		selectedAuthorChanged(m_selectedAuthor);
	}

	if(m_selectedTeam != nullptr) {
		m_selectedTeam = nullptr;

		selectedTeamChanged(m_selectedTeam);
	}

	if(!areCurrentSortOptionsValidInCurrentContext()) {
		m_sortType = OrganizedModCollection::DEFAULT_SORT_TYPE;
		m_sortDirection = OrganizedModCollection::DEFAULT_SORT_DIRECTION;

		sortOptionsChanged(m_sortType, m_sortDirection);

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

bool OrganizedModCollection::hasModWithID(const std::string & id) const {
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

size_t OrganizedModCollection::indexOfModWithID(const std::string & id) const {
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

std::shared_ptr<Mod> OrganizedModCollection::getModWithID(const std::string & id) const {
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

		selectedModChanged(m_selectedMod);
	}

	return true;
}

bool OrganizedModCollection::setSelectedModByID(const std::string & id) {
	if(id.empty()) {
		clearSelectedMod();

		return true;
	}

	return setSelectedMod(indexOfModWithID(id));
}

bool OrganizedModCollection::setSelectedModByName(const std::string & name) {
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

	return setSelectedModByID(mod->getID());
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

		selectedModChanged(m_selectedMod);
	}
}

size_t OrganizedModCollection::numberOfFavouriteMods() {
	return m_organizedFavouriteMods.size();
}

bool OrganizedModCollection::hasFavouriteMod(const ModIdentifier & favouriteMod) const {
	return indexOfFavouriteMod(favouriteMod) != std::numeric_limits<size_t>::max();
}

bool OrganizedModCollection::hasFavouriteMod(const std::string & name, const std::string & version, const std::string & versionType) const {
	return indexOfFavouriteMod(name, version, versionType) != std::numeric_limits<size_t>::max();
}

size_t OrganizedModCollection::indexOfFavouriteMod(const ModIdentifier & favouriteMod) const {
	if(!favouriteMod.isValid()) {
		return std::numeric_limits<size_t>::max();
	}

	std::vector<std::shared_ptr<ModIdentifier>>::const_iterator favouriteModIterator = std::find_if(m_organizedFavouriteMods.cbegin(), m_organizedFavouriteMods.cend(), [&favouriteMod](const std::shared_ptr<ModIdentifier> & currentFavouriteMod) {
		return *currentFavouriteMod == favouriteMod;
	});

	if(favouriteModIterator == m_organizedFavouriteMods.cend()) {
		return std::numeric_limits<size_t>::max();
	}

	return favouriteModIterator - m_organizedFavouriteMods.cbegin();
}

size_t OrganizedModCollection::indexOfFavouriteMod(const std::string & name, const std::string & version, const std::string & versionType) const {
	return indexOfFavouriteMod(ModIdentifier(name, version, versionType));
}

std::shared_ptr<ModIdentifier> OrganizedModCollection::getFavouriteMod(size_t index) const {
	if(index >= m_organizedFavouriteMods.size()) {
		return nullptr;
	}

	return m_organizedFavouriteMods[index];
}

std::shared_ptr<ModIdentifier> OrganizedModCollection::getFavouriteMod(const std::string & name, const std::string & version, const std::string & versionType) const {
	return getFavouriteMod(indexOfFavouriteMod(name, version, versionType));
}

std::shared_ptr<ModIdentifier> OrganizedModCollection::getSelectedFavouriteMod() const {
	return m_selectedFavouriteMod;
}

bool OrganizedModCollection::setSelectedFavouriteMod(size_t index) {
	if(index >= m_organizedFavouriteMods.size()) {
		return false;
	}

	if(m_selectedFavouriteMod != m_organizedFavouriteMods[index]) {
		m_selectedFavouriteMod = m_organizedFavouriteMods[index];

		selectedFavouriteModChanged(m_selectedFavouriteMod);
	}

	return true;
}

bool OrganizedModCollection::setSelectedFavouriteMod(const ModIdentifier & favouriteMod) {
	if(!favouriteMod.isValid()) {
		return false;
	}

	return setSelectedFavouriteMod(indexOfFavouriteMod(favouriteMod));
}

bool OrganizedModCollection::selectRandomFavouriteMod() {
	if(m_organizedFavouriteMods.empty()) {
		return false;
	}

	setSelectedFavouriteMod(Utilities::randomInteger(0, m_organizedFavouriteMods.size() - 1));

	return true;
}

void OrganizedModCollection::clearSelectedFavouriteMod() {
	if(m_selectedFavouriteMod != nullptr) {
		m_selectedFavouriteMod = nullptr;

		selectedFavouriteModChanged(m_selectedFavouriteMod);
	}
}

size_t OrganizedModCollection::numberOfGameVersions() const {
	return m_organizedGameVersions.size();
}

bool OrganizedModCollection::hasGameVersion(const GameVersion & gameVersion) const {
	return indexOfGameVersionWithID(gameVersion.getID()) != std::numeric_limits<size_t>::max();
}

bool OrganizedModCollection::hasGameVersionWithID(const std::string & gameVersionID) const {
	return indexOfGameVersionWithID(gameVersionID) != std::numeric_limits<size_t>::max();
}

size_t OrganizedModCollection::indexOfGameVersion(const GameVersion & gameVersion) const {
	return indexOfGameVersionWithID(gameVersion.getID());
}

size_t OrganizedModCollection::indexOfGameVersionWithID(const std::string & gameVersionID) const {
	if(gameVersionID.empty()) {
		return std::numeric_limits<size_t>::max();
	}

	for(size_t i = 0; i < m_organizedGameVersions.size(); i++) {
		if(Utilities::areStringsEqualIgnoreCase(m_organizedGameVersions[i]->getID(), gameVersionID)) {
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

std::shared_ptr<GameVersion> OrganizedModCollection::getGameVersionWithID(const std::string & gameVersionID) const {
	size_t gameVersionIndex = indexOfGameVersionWithID(gameVersionID);

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

		selectedGameVersionChanged(m_selectedGameVersion);

		shouldOrganize = true;
	}

	if(!areCurrentSortOptionsValidInCurrentContext()) {
		m_sortType = OrganizedModCollection::DEFAULT_SORT_TYPE;
		m_sortDirection = OrganizedModCollection::DEFAULT_SORT_DIRECTION;

		sortOptionsChanged(m_sortType, m_sortDirection);

		shouldOrganize = true;
	}

	if(shouldOrganize) {
		organize();
	}

	return true;
}

bool OrganizedModCollection::setSelectedGameVersionByID(const std::string & gameVersionID) {
	if(gameVersionID.empty()) {
		clearSelectedGameVersion();

		return true;
	}

	return setSelectedGameVersion(indexOfGameVersionWithID(gameVersionID));
}

bool OrganizedModCollection::setSelectedGameVersion(const GameVersion * gameVersion) {
	if(gameVersion == nullptr) {
		clearSelectedGameVersion();

		return true;
	}

	return setSelectedGameVersionByID(gameVersion->getID());
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

		selectedGameVersionChanged(m_selectedGameVersion);

		shouldOrganize = true;
	}

	if(!areCurrentSortOptionsValidInCurrentContext()) {
		m_sortType = OrganizedModCollection::DEFAULT_SORT_TYPE;
		m_sortDirection = OrganizedModCollection::DEFAULT_SORT_DIRECTION;

		sortOptionsChanged(m_sortType, m_sortDirection);

		shouldOrganize = true;
	}

	if(shouldOrganize) {
		organize();
	}
}

size_t OrganizedModCollection::getSupportedModCountForGameVersion(const GameVersion & gameVersion) const {
	return getSupportedModCountForGameVersionWithID(gameVersion.getID());
}

size_t OrganizedModCollection::getSupportedModCountForGameVersionWithID(const std::string & gameVersionID) const {
	std::map<std::string, size_t>::const_iterator gameVersionSupportedModCount(m_gameVersionSupportedModCountMap.find(gameVersionID));

	return gameVersionSupportedModCount == m_gameVersionSupportedModCountMap.end() ? 0 : gameVersionSupportedModCount->second;
}

size_t OrganizedModCollection::getCompatibleModCountForGameVersion(const GameVersion & gameVersion) const {
	return getCompatibleModCountForGameVersionWithID(gameVersion.getID());
}

size_t OrganizedModCollection::getCompatibleModCountForGameVersionWithID(const std::string & gameVersionID) const {
	std::map<std::string, size_t>::const_iterator gameVersionCompatibleModCount(m_gameVersionCompatibleModCountMap.find(gameVersionID));

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

		selectedTeamChanged(m_selectedTeam);

		shouldOrganize = true;
	}

	if(m_filterType != FilterType::Teams) {
		m_filterType = FilterType::Teams;

		filterTypeChanged(m_filterType);

		shouldOrganize = true;
	}

	if(!areCurrentSortOptionsValidInCurrentContext()) {
		m_sortType = OrganizedModCollection::DEFAULT_SORT_TYPE;
		m_sortDirection = OrganizedModCollection::DEFAULT_SORT_DIRECTION;

		sortOptionsChanged(m_sortType, m_sortDirection);

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

		selectedTeamChanged(m_selectedTeam);

		shouldOrganize = true;
	}

	if(!areCurrentSortOptionsValidInCurrentContext()) {
		m_sortType = OrganizedModCollection::DEFAULT_SORT_TYPE;
		m_sortDirection = OrganizedModCollection::DEFAULT_SORT_DIRECTION;

		sortOptionsChanged(m_sortType, m_sortDirection);

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

	bool shouldOrganize = false;

	if(m_selectedAuthor != m_authors[index]) {
		m_selectedAuthor = m_authors[index];

		selectedAuthorChanged(m_selectedAuthor);

		shouldOrganize = true;
	}

	if(m_filterType != FilterType::Authors) {
		m_filterType = FilterType::Authors;

		filterTypeChanged(m_filterType);

		shouldOrganize = true;
	}

	if(!areCurrentSortOptionsValidInCurrentContext()) {
		m_sortType = OrganizedModCollection::DEFAULT_SORT_TYPE;
		m_sortDirection = OrganizedModCollection::DEFAULT_SORT_DIRECTION;

		sortOptionsChanged(m_sortType, m_sortDirection);

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

		selectedAuthorChanged(m_selectedAuthor);

		shouldOrganize = true;
	}

	if(!areCurrentSortOptionsValidInCurrentContext()) {
		m_sortType = OrganizedModCollection::DEFAULT_SORT_TYPE;
		m_sortDirection = OrganizedModCollection::DEFAULT_SORT_DIRECTION;

		sortOptionsChanged(m_sortType, m_sortDirection);

		shouldOrganize = true;
	}

	if(shouldOrganize) {
		organize();
	}
}

void OrganizedModCollection::onModCollectionUpdated(ModCollection & mods) {
	updateGameVersionModCounts();
	updateTeamList();
	updateAuthorList();

	organize();
}

void OrganizedModCollection::onFavouriteModCollectionUpdated(FavouriteModCollection & favouriteMods) {
	if(m_filterType == FilterType::Favourites) {
		organize();
	}
}

void OrganizedModCollection::onGameVersionCollectionSizeChanged(GameVersionCollection & gameVersionCollection) {
	updateGameVersionList();

	if(m_filterType == FilterType::SupportedGameVersions || m_filterType == FilterType::CompatibleGameVersions) {
		organize();
	}
}

void OrganizedModCollection::onGameVersionCollectionItemModified(GameVersionCollection & gameVersionCollection, GameVersion & gameVersion) {
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
	m_organizedFavouriteMods.clear();

	if(m_filterType == FilterType::Favourites &&
	   (m_favouriteMods == nullptr || m_favouriteMods->numberOfFavourites() == 0)) {
		return;
	}

	if((m_filterType == FilterType::SupportedGameVersions || m_filterType == FilterType::CompatibleGameVersions) &&
	   (m_gameVersions == nullptr || m_gameVersions->numberOfGameVersions() == 0)) {
		return;
	}

	switch(m_filterType) {
		case FilterType::None: {
			for(size_t i = 0; i < m_mods->numberOfMods(); i++) {
				m_organizedMods.push_back(m_mods->getMod(i));
			}

			break;
		}

		case FilterType::Favourites: {
			for(size_t i = 0; i < m_favouriteMods->numberOfFavourites(); i++) {
				m_organizedFavouriteMods.push_back(m_favouriteMods->getFavourite(i));
			}

			break;
		}

		case FilterType::Downloaded: {
			if(m_localMode || m_downloadManager != nullptr) {
				std::shared_ptr<Mod> mod;

				for(size_t i = 0; i < m_mods->numberOfMods(); i++) {
					mod = m_mods->getMod(i);

					if(m_localMode || m_downloadManager->isModDownloaded(mod.get())) {
						m_organizedMods.push_back(mod);
					}
				}
			}

			break;
		}

		case FilterType::SupportedGameVersions: {
			if(m_selectedGameVersion != nullptr) {
				std::shared_ptr<Mod> mod;

				for(size_t i = 0; i < m_mods->numberOfMods(); i++) {
					mod = m_mods->getMod(i);

					if(mod->isGameVersionSupported(*m_selectedGameVersion)) {
						m_organizedMods.push_back(mod);
					}
				}
			}

			break;
		}

		case FilterType::CompatibleGameVersions: {
			if(m_selectedGameVersion != nullptr) {
				std::shared_ptr<Mod> mod;

				for(size_t i = 0; i < m_mods->numberOfMods(); i++) {
					mod = m_mods->getMod(i);

					if(mod->isGameVersionCompatible(*m_selectedGameVersion)) {
						m_organizedMods.push_back(mod);
					}
				}
			}

			break;
		}

		case FilterType::StandAlone: {
			std::shared_ptr<Mod> mod;

			for(size_t i = 0; i < m_mods->numberOfMods(); i++) {
				mod = m_mods->getMod(i);

				if(mod->isStandAlone()) {
					m_organizedMods.push_back(mod);
				}
			}

			break;
		}

		case FilterType::Teams: {
			if(m_selectedTeam != nullptr) {
				std::shared_ptr<ModTeam> team;

				for(size_t i = 0; i < m_mods->numberOfMods(); i++) {
					team = m_mods->getMod(i)->getTeam();

					if(team != nullptr && Utilities::areStringsEqual(team->getName(), m_selectedTeam->getName())) {
						m_organizedMods.push_back(m_mods->getMod(i));
					}
				}
			}

			break;
		}

		case FilterType::Authors: {
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

			break;
		}
	}

	organizedModCollectionChanged(m_organizedMods);
}

void OrganizedModCollection::sort() {
	if(m_mods == nullptr || m_sortType == SortType::Unsorted) {
		return;
	}

	switch(m_filterType) {
		case FilterType::None:
		case FilterType::Downloaded:
		case FilterType::SupportedGameVersions:
		case FilterType::CompatibleGameVersions:
		case FilterType::StandAlone: {
			m_organizedMods = mergeSortMods(m_organizedMods);
			m_organizedGameVersions = mergeSortGameVersions(m_organizedGameVersions);

			organizedModCollectionChanged(m_organizedMods);
			organizedModGameVersionCollectionChanged(m_organizedGameVersions);

			break;
		}
		case FilterType::Favourites: {
			m_organizedFavouriteMods = mergeSortModIdentifiers(m_organizedFavouriteMods);

			organizedFavouriteModCollectionChanged(m_organizedFavouriteMods);

			break;
		}
		case FilterType::Teams: {
			if(m_selectedTeam == nullptr) {
				m_teams = mergeSortAuthors(m_teams);

				organizedModTeamCollectionChanged(m_teams);
			}
			else {
				m_organizedMods = mergeSortMods(m_organizedMods);

				organizedModCollectionChanged(m_organizedMods);
			}

			break;
		}
		case FilterType::Authors: {
			if(m_selectedAuthor == nullptr) {
				m_authors = mergeSortAuthors(m_authors);

				organizedModAuthorCollectionChanged(m_authors);
			}
			else {
				m_organizedMods = mergeSortMods(m_organizedMods);

				organizedModCollectionChanged(m_organizedMods);
			}

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

	organizedModGameVersionCollectionChanged(m_organizedGameVersions);
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

	organizedModTeamCollectionChanged(m_teams);
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

	organizedModAuthorCollectionChanged(m_authors);
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

		m_gameVersionSupportedModCountMap[gameVersion->getID()] = 0;

		for(size_t j = 0; j < m_mods->numberOfMods(); j++) {
			mod = m_mods->getMod(j);

			if(mod->isGameVersionSupported(*gameVersion)) {
				m_gameVersionSupportedModCountMap[gameVersion->getID()]++;
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

		m_gameVersionCompatibleModCountMap[gameVersion->getID()] = 0;

		for(size_t j = 0; j < m_mods->numberOfMods(); j++) {
			mod = m_mods->getMod(j);

			if(mod->isGameVersionCompatible(*gameVersion)) {
				m_gameVersionCompatibleModCountMap[gameVersion->getID()]++;
			}
		}
	}
}

size_t OrganizedModCollection::indexOfCurrentSortType() const {
	return indexOfSortType(m_sortType);
}

size_t OrganizedModCollection::indexOfSortType(SortType sortType) const {
	const auto & sortTypes = magic_enum::enum_values<SortType>();
	size_t sortTypeIndex = 0;

	for(SortType currentSortType : sortTypes) {
		if(!areSortOptionsValidInCurrentContext(currentSortType, m_filterType)) {
			continue;
		}

		if(currentSortType == sortType) {
			return sortTypeIndex;
		}

		sortTypeIndex++;
	}

	return std::numeric_limits<size_t>::max();
}

std::vector<OrganizedModCollection::SortType> OrganizedModCollection::getValidSortTypesInCurrentContext() const {
	const auto & sortTypes = magic_enum::enum_values<SortType>();
	std::vector<SortType> validSortTypes;

	for(SortType sortType : sortTypes) {
		if(areSortOptionsValidInContext(sortType, m_filterType, m_selectedGameVersion != nullptr, m_selectedTeam != nullptr, m_selectedAuthor != nullptr)) {
			validSortTypes.push_back(sortType);
		}
	}

	return validSortTypes;
}

std::vector<OrganizedModCollection::SortType> OrganizedModCollection::getInvalidSortTypesInCurrentContext() const {
	const auto & sortTypes = magic_enum::enum_values<SortType>();
	std::vector<SortType> invalidSortTypes;

	for(SortType sortType : sortTypes) {
		if(!areSortOptionsValidInContext(sortType, m_filterType, m_selectedGameVersion != nullptr, m_selectedTeam != nullptr, m_selectedAuthor != nullptr)) {
			invalidSortTypes.push_back(sortType);
		}
	}

	return invalidSortTypes;
}

bool OrganizedModCollection::areCurrentSortOptionsValidInCurrentContext() const {
	return areSortOptionsValidInContext(m_sortType, m_filterType, m_selectedGameVersion != nullptr, m_selectedTeam != nullptr, m_selectedAuthor != nullptr);
}

bool OrganizedModCollection::areSortOptionsValidInCurrentContext(SortType sortType, FilterType filterType) const {
	return areSortOptionsValidInContext(sortType, filterType, m_selectedGameVersion != nullptr, m_selectedTeam != nullptr, m_selectedAuthor != nullptr);
}

bool OrganizedModCollection::areSortOptionsValidInContext(SortType sortType, FilterType filterType, bool hasSelectedGameVersion, bool hasSelectedTeam, bool hasSelectedAuthor) {
	if(sortType == SortType::Unsorted ||
	   sortType == SortType::Name ||
	   sortType == SortType::Random) {
		return true;
	}

	if(  filterType == FilterType::None ||
		 filterType == FilterType::Downloaded ||
	   ((filterType == FilterType::SupportedGameVersions || filterType == FilterType::CompatibleGameVersions) && hasSelectedGameVersion) ||
	     filterType == FilterType::StandAlone ||
	    (filterType == FilterType::Teams && hasSelectedTeam) ||
	    (filterType == FilterType::Authors && hasSelectedAuthor)) {
		return sortType == SortType::InitialReleaseDate ||
			   sortType == SortType::LatestReleaseDate ||
			   sortType == SortType::NumberOfVersions;
	}
	else if((filterType == FilterType::SupportedGameVersions || filterType == FilterType::CompatibleGameVersions) && !hasSelectedGameVersion) {
		return sortType == SortType::NumberOfSupportedMods ||
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

std::vector<std::shared_ptr<ModIdentifier>> OrganizedModCollection::mergeSortModIdentifiers(std::vector<std::shared_ptr<ModIdentifier>> modIdentifiers) {
	if(modIdentifiers.size() <= 1) {
		return modIdentifiers;
	}

	std::vector<std::shared_ptr<ModIdentifier>> left;
	std::vector<std::shared_ptr<ModIdentifier>> right;

	size_t mid = modIdentifiers.size() / 2;

	for(size_t i = 0; i < mid; i++) {
		left.push_back(modIdentifiers[i]);
	}

	for(size_t i = mid; i < modIdentifiers.size(); i++) {
		right.push_back(modIdentifiers[i]);
	}

	left = mergeSortModIdentifiers(left);
	right = mergeSortModIdentifiers(right);

	return mergeModIdentifiers(left, right);
}

std::vector<std::shared_ptr<ModIdentifier>> OrganizedModCollection::mergeModIdentifiers(std::vector<std::shared_ptr<ModIdentifier>> left, std::vector<std::shared_ptr<ModIdentifier>> right) {
	std::vector<std::shared_ptr<ModIdentifier>> result;

	bool pushLeft = true;

	while(!left.empty() && !right.empty()) {
		if(m_sortType == SortType::Name) {
			int32_t nameComparison = Utilities::compareStringsIgnoreCase(left[0]->getName(), right[0]->getName());

			if(nameComparison != 0) {
				if(m_sortDirection == SortDirection::Ascending) {
					pushLeft = nameComparison < 0;
				}
				else {
					pushLeft = nameComparison > 0;
				}
			}
			else {
				if(!left[0]->hasVersion()) {
					pushLeft = false;
				}
				else if(!right[0]->hasVersion()) {
					pushLeft = true;
				}
				else {
					int32_t versionComparison = Utilities::compareStringsIgnoreCase(left[0]->getVersion().value(), right[0]->getVersion().value());

					if(versionComparison != 0) {
						if(m_sortDirection == SortDirection::Ascending) {
							pushLeft = versionComparison < 0;
						}
						else {
							pushLeft = versionComparison > 0;
						}
					}
					else {
						if(!left[0]->hasVersionType()) {
							pushLeft = false;
						}
						else if(!right[0]->hasVersionType()) {
							pushLeft = true;
						}
						else {
							int32_t versionTypeComparison = Utilities::compareStringsIgnoreCase(left[0]->getVersionType().value(), right[0]->getVersionType().value());

							if(m_sortDirection == SortDirection::Ascending) {
								pushLeft = versionTypeComparison <= 0;
							}
							else {
								pushLeft = versionTypeComparison > 0;
							}
						}
					}
				}
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
				pushLeft = Utilities::compareStringsIgnoreCase(left[0]->getLongName(), right[0]->getLongName()) <= 0;
			}
			else {
				pushLeft = Utilities::compareStringsIgnoreCase(left[0]->getLongName(), right[0]->getLongName()) > 0;
			}
		}
		else if(m_sortType == SortType::NumberOfSupportedMods) {
			if(m_sortDirection == SortDirection::Ascending) {
				pushLeft = getSupportedModCountForGameVersionWithID(left[0]->getID()) <= getSupportedModCountForGameVersionWithID(right[0]->getID());
			}
			else {
				pushLeft = getSupportedModCountForGameVersionWithID(left[0]->getID()) > getSupportedModCountForGameVersionWithID(right[0]->getID());
			}
		}
		else if(m_sortType == SortType::NumberOfCompatibleMods) {
			if(m_sortDirection == SortDirection::Ascending) {
				pushLeft = getCompatibleModCountForGameVersionWithID(left[0]->getID()) <= getCompatibleModCountForGameVersionWithID(right[0]->getID());
			}
			else {
				pushLeft = getCompatibleModCountForGameVersionWithID(left[0]->getID()) > getCompatibleModCountForGameVersionWithID(right[0]->getID());
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

bool OrganizedModCollection::operator == (const OrganizedModCollection & m) const {
	if((m_mods == nullptr && m.m_mods != nullptr) ||
	   (m_mods != nullptr && m.m_mods == nullptr) ||
	   (m_favouriteMods == nullptr && m.m_favouriteMods != nullptr) ||
	   (m_favouriteMods != nullptr && m.m_favouriteMods == nullptr) ||
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
	   (m_favouriteMods != nullptr && m.m_favouriteMods != nullptr && *m_favouriteMods != *m.m_favouriteMods) ||
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
