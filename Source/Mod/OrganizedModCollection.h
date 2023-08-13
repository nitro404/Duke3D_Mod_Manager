#ifndef _ORGANIZED_MOD_COLLECTION_H_
#define _ORGANIZED_MOD_COLLECTION_H_

#include <boost/signals2.hpp>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

class DownloadManager;
class FavouriteModCollection;
class GameVersion;
class GameVersionCollection;
class Mod;
class ModAuthorInformation;
class ModCollection;
class ModIdentifier;

class OrganizedModCollection final {
public:
	enum class SortType {
		Unsorted,
		Name,
		InitialReleaseDate,
		LatestReleaseDate,
		NumberOfMods,
		NumberOfVersions,
		NumberOfSupportedMods,
		NumberOfCompatibleMods,
		Random
	};

	enum class SortDirection {
		Ascending,
		Descending
	};

	enum class FilterType {
		None,
		Favourites,
		Downloaded,
		SupportedGameVersions,
		CompatibleGameVersions,
		StandAlone,
		Teams,
		Authors
	};

	OrganizedModCollection(std::shared_ptr<ModCollection> mods = nullptr, std::shared_ptr<FavouriteModCollection> favourites = nullptr, std::shared_ptr<GameVersionCollection> gameVersions = nullptr);
	OrganizedModCollection(OrganizedModCollection && m) noexcept;
	OrganizedModCollection(const OrganizedModCollection & m);
	OrganizedModCollection & operator = (OrganizedModCollection && m) noexcept;
	OrganizedModCollection & operator = (const OrganizedModCollection & m);
	~OrganizedModCollection();

	bool isUsingLocalMode() const;
	void setLocalMode(bool localMode);
	std::shared_ptr<ModCollection> getModCollection() const;
	const std::vector<std::shared_ptr<Mod>> & getOrganizedMods() const;
	const std::vector<std::shared_ptr<ModIdentifier>> & getOrganizedFavouriteMods() const;
	const std::vector<std::shared_ptr<GameVersion>> & getOrganizedGameVersions() const;
	std::shared_ptr<FavouriteModCollection> getFavouriteModCollection() const;
	std::shared_ptr<GameVersionCollection> getGameVersionCollection() const;
	std::vector<std::string> getOrganizedItemDisplayNames(bool prependItemNumber = true) const;
	FilterType getFilterType() const;
	SortType getSortType() const;
	SortDirection getSortDirection() const;
	void setDownloadManager(std::shared_ptr<DownloadManager> downloadManager);

	void setModCollection(std::shared_ptr<ModCollection> mods);
	void setFavouriteModCollection(std::shared_ptr<FavouriteModCollection> favourites);
	void setGameVersionCollection(std::shared_ptr<GameVersionCollection> gameVersions);
	bool setFilterType(FilterType filterType);
	bool setSortType(SortType sortType);
	bool setSortTypeByIndex(size_t sortTypeIndex);
	bool setSortDirection(SortDirection sortDirection);
	bool setSortOptions(SortType sortType, SortDirection sortDirection);

	bool shouldDisplayMods() const;
	bool shouldDisplayFavouriteMods() const;
	bool shouldDisplayGameVersions() const;
	bool shouldDisplayTeams() const;
	bool shouldDisplayAuthors() const;
	size_t numberOfItems() const;
	size_t indexOfSelectedItem() const;
	bool selectItem(size_t index);
	bool selectRandomItem();
	void clearSelectedItems();

	size_t numberOfMods() const;
	bool hasMod(const Mod & mod) const;
	bool hasModWithID(const std::string & id) const;
	bool hasModWithName(const std::string & name) const;
	size_t indexOfMod(const Mod & mod) const;
	size_t indexOfModWithID(const std::string & id) const;
	size_t indexOfModWithName(const std::string & name) const;
	std::shared_ptr<Mod> getMod(size_t index) const;
	std::shared_ptr<Mod> getModWithID(const std::string & id) const;
	std::shared_ptr<Mod> getModWithName(const std::string & name) const;
	std::shared_ptr<Mod> getSelectedMod() const;
	bool setSelectedMod(size_t index);
	bool setSelectedModByID(const std::string & id);
	bool setSelectedModByName(const std::string & name);
	bool setSelectedMod(const Mod * mod);
	bool selectRandomMod();
	void clearSelectedMod();

	size_t numberOfFavouriteMods();
	bool hasFavouriteMod(const ModIdentifier & favouriteMod) const;
	bool hasFavouriteMod(const std::string & name, const std::string & version = {}, const std::string & versionType = {}) const;
	size_t indexOfFavouriteMod(const ModIdentifier & favouriteMod) const;
	size_t indexOfFavouriteMod(const std::string & name, const std::string & version = {}, const std::string & versionType = {}) const;
	std::shared_ptr<ModIdentifier> getFavouriteMod(size_t index) const;
	std::shared_ptr<ModIdentifier> getFavouriteMod(const std::string & name, const std::string & version = {}, const std::string & versionType = {}) const;
	std::shared_ptr<ModIdentifier> getSelectedFavouriteMod() const;
	bool setSelectedFavouriteMod(size_t index);
	bool setSelectedFavouriteMod(const ModIdentifier & favouriteMod);
	bool selectRandomFavouriteMod();
	void clearSelectedFavouriteMod();

	size_t numberOfGameVersions() const;
	bool hasGameVersion(const GameVersion & gameVersion) const;
	bool hasGameVersionWithID(const std::string & gameVersionID) const;
	size_t indexOfGameVersion(const GameVersion & gameVersion) const;
	size_t indexOfGameVersionWithID(const std::string & gameVersionID) const;
	std::shared_ptr<GameVersion> getGameVersion(size_t index) const;
	std::shared_ptr<GameVersion> getGameVersionWithID(const std::string & gameVersionID) const;
	bool hasSelectedGameVersion() const;
	std::shared_ptr<GameVersion> getSelectedGameVersion() const;
	bool setSelectedGameVersion(size_t index);
	bool setSelectedGameVersionByID(const std::string & gameVersionID);
	bool setSelectedGameVersion(const GameVersion * gameVersion);
	bool selectRandomGameVersion();
	void clearSelectedGameVersion();

	size_t getSupportedModCountForGameVersion(const GameVersion & gameVersion) const;
	size_t getSupportedModCountForGameVersionWithID(const std::string & gameVersionID) const;
	size_t getCompatibleModCountForGameVersion(const GameVersion & gameVersion) const;
	size_t getCompatibleModCountForGameVersionWithID(const std::string & gameVersionID) const;

	size_t numberOfTeams() const;
	bool hasTeamInfo(const std::string & name) const;
	size_t indexOfTeamInfo(const ModAuthorInformation & teamInfo) const;
	size_t indexOfTeamInfo(const std::string & name) const;
	std::shared_ptr<ModAuthorInformation> getTeamInfo(size_t index) const;
	std::shared_ptr<ModAuthorInformation> getTeamInfo(const std::string & name) const;
	void incrementTeamModCount(size_t index);
	void incrementTeamModCount(const std::string & name);
	bool hasSelectedTeam() const;
	std::shared_ptr<ModAuthorInformation> getSelectedTeam() const;
	bool setSelectedTeam(size_t index);
	bool setSelectedTeam(const std::string & name);
	bool setSelectedTeam(const ModAuthorInformation * teamInfo);
	bool selectRandomTeam();
	void clearSelectedTeam();

	size_t numberOfAuthors() const;
	bool hasAuthorInfo(const std::string & name) const;
	size_t indexOfAuthorInfo(const ModAuthorInformation & authorInfo) const;
	size_t indexOfAuthorInfo(const std::string & name) const;
	std::shared_ptr<ModAuthorInformation> getAuthorInfo(size_t index) const;
	std::shared_ptr<ModAuthorInformation> getAuthorInfo(const std::string & name) const;
	void incrementAuthorModCount(size_t index);
	void incrementAuthorModCount(const std::string & name);
	bool hasSelectedAuthor() const;
	std::shared_ptr<ModAuthorInformation> getSelectedAuthor() const;
	bool setSelectedAuthor(size_t index);
	bool setSelectedAuthor(const std::string & name);
	bool setSelectedAuthor(const ModAuthorInformation * authorInfo);
	bool selectRandomAuthor();
	void clearSelectedAuthor();

	void organize();

	size_t indexOfCurrentSortType() const;
	size_t indexOfSortType(SortType sortType) const;
	std::vector<SortType> getValidSortTypesInCurrentContext() const;
	std::vector<SortType> getInvalidSortTypesInCurrentContext() const;
	bool areCurrentSortOptionsValidInCurrentContext() const;
	bool areSortOptionsValidInCurrentContext(SortType sortType, FilterType filterType) const;
	static bool areSortOptionsValidInContext(SortType sortType, FilterType filterType, bool hasSelectedGameVersion, bool hasSelectedTeam, bool hasSelectedAuthor);

	bool operator == (const OrganizedModCollection & m) const;
	bool operator != (const OrganizedModCollection & m) const;

	boost::signals2::signal<void (FilterType /* filterType */)> filterTypeChanged;
	boost::signals2::signal<void (SortType /* sortType */, SortDirection /* sortDirection */)> sortOptionsChanged;
	boost::signals2::signal<void (std::shared_ptr<Mod> /* mod */)> selectedModChanged;
	boost::signals2::signal<void (std::shared_ptr<ModIdentifier> /* favouriteMod */)> selectedFavouriteModChanged;
	boost::signals2::signal<void (std::shared_ptr<GameVersion> /* gameVersion */)> selectedGameVersionChanged;
	boost::signals2::signal<void (std::shared_ptr<ModAuthorInformation> /* team */)> selectedTeamChanged;
	boost::signals2::signal<void (std::shared_ptr<ModAuthorInformation> /* author */)> selectedAuthorChanged;
	boost::signals2::signal<void (const std::vector<std::shared_ptr<Mod>> & /* organizedMods */)> organizedModCollectionChanged;
	boost::signals2::signal<void (const std::vector<std::shared_ptr<ModIdentifier>> & /* organizedFavouriteMods */)> organizedFavouriteModCollectionChanged;
	boost::signals2::signal<void (const std::vector<std::shared_ptr<GameVersion>> & /* organizedGameVersions */)> organizedModGameVersionCollectionChanged;
	boost::signals2::signal<void (const std::vector<std::shared_ptr<ModAuthorInformation>> & /* organizedTeams */)> organizedModTeamCollectionChanged;
	boost::signals2::signal<void (const std::vector<std::shared_ptr<ModAuthorInformation>> & /* organizedAuthors */)> organizedModAuthorCollectionChanged;

	static const FilterType DEFAULT_FILTER_TYPE;
	static const SortType DEFAULT_SORT_TYPE;
	static const SortDirection DEFAULT_SORT_DIRECTION;

private:
	void applyFilter();
	void sort();
	void updateGameVersionList();
	void updateTeamList();
	void updateAuthorList();
	void updateGameVersionModCounts();
	void updateGameVersionSupportedModCounts();
	void updateGameVersionCompatibleModCounts();
	void onModCollectionUpdated(ModCollection & mods);
	void onFavouriteModCollectionUpdated(FavouriteModCollection & favouriteMods);
	void onGameVersionCollectionSizeChanged(GameVersionCollection & gameVersionCollection);
	void onGameVersionCollectionItemModified(GameVersionCollection & gameVersionCollection, GameVersion & gameVersion);

	std::vector<std::shared_ptr<Mod>> mergeSortMods(std::vector<std::shared_ptr<Mod>> mods);
	std::vector<std::shared_ptr<Mod>> mergeMods(std::vector<std::shared_ptr<Mod>> left, std::vector<std::shared_ptr<Mod>> right);
	std::vector<std::shared_ptr<ModIdentifier>> mergeSortModIdentifiers(std::vector<std::shared_ptr<ModIdentifier>> authors);
	std::vector<std::shared_ptr<ModIdentifier>> mergeModIdentifiers(std::vector<std::shared_ptr<ModIdentifier>> left, std::vector<std::shared_ptr<ModIdentifier>> right);
	std::vector<std::shared_ptr<GameVersion>> mergeSortGameVersions(std::vector<std::shared_ptr<GameVersion>> gameVersions);
	std::vector<std::shared_ptr<GameVersion>> mergeGameVersions(std::vector<std::shared_ptr<GameVersion>> left, std::vector<std::shared_ptr<GameVersion>> right);
	std::vector<std::shared_ptr<ModAuthorInformation>> mergeSortAuthors(std::vector<std::shared_ptr<ModAuthorInformation>> authors);
	std::vector<std::shared_ptr<ModAuthorInformation>> mergeAuthors(std::vector<std::shared_ptr<ModAuthorInformation>> left, std::vector<std::shared_ptr<ModAuthorInformation>> right);

	bool m_localMode;
	FilterType m_filterType;
	SortType m_sortType;
	SortDirection m_sortDirection;
	std::shared_ptr<ModCollection> m_mods;
	boost::signals2::connection m_modCollectionUpdatedConnection;
	std::shared_ptr<FavouriteModCollection> m_favouriteMods;
	boost::signals2::connection m_favouriteModCollectionUpdatedConnection;
	std::shared_ptr<DownloadManager> m_downloadManager;
	std::shared_ptr<GameVersionCollection> m_gameVersions;
	boost::signals2::connection m_gameVersionCollectionSizeChangedConnection;
	boost::signals2::connection m_gameVersionCollectionItemModifiedConnection;
	std::vector<std::shared_ptr<Mod>> m_organizedMods;
	std::vector<std::shared_ptr<ModIdentifier>> m_organizedFavouriteMods;
	std::vector<std::shared_ptr<GameVersion>> m_organizedGameVersions;
	std::vector<std::shared_ptr<ModAuthorInformation>> m_teams;
	std::vector<std::shared_ptr<ModAuthorInformation>> m_authors;
	std::map<std::string, size_t> m_gameVersionSupportedModCountMap;
	std::map<std::string, size_t> m_gameVersionCompatibleModCountMap;
	std::shared_ptr<Mod> m_selectedMod;
	std::shared_ptr<ModIdentifier> m_selectedFavouriteMod;
	std::shared_ptr<GameVersion> m_selectedGameVersion;
	std::shared_ptr<ModAuthorInformation> m_selectedTeam;
	std::shared_ptr<ModAuthorInformation> m_selectedAuthor;
};

#endif // _ORGANIZED_MOD_COLLECTION_H_
