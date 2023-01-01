#ifndef _ORGANIZED_MOD_COLLECTION_H_
#define _ORGANIZED_MOD_COLLECTION_H_

#include "Game/GameVersionCollectionListener.h"
#include "ModCollectionListener.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

class FavouriteModCollection;
class GameVersion;
class GameVersionCollection;
class Mod;
class ModAuthorInformation;
class ModCollection;

class OrganizedModCollection final : public ModCollectionListener,
                                     public GameVersionCollectionListener {
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
		SupportedGameVersions,
		CompatibleGameVersions,
		Teams,
		Authors
	};

	class Listener {
	public:
		virtual ~Listener();

		virtual void filterTypeChanged(FilterType filterType);
		virtual void sortOptionsChanged(SortType sortType, SortDirection sortDirection);
		virtual void selectedModChanged(const std::shared_ptr<Mod> & mod);
		virtual void selectedGameVersionChanged(const std::shared_ptr<GameVersion> & gameVersion);
		virtual void selectedTeamChanged(const std::shared_ptr<ModAuthorInformation> & team);
		virtual void selectedAuthorChanged(const std::shared_ptr<ModAuthorInformation> & author);
		virtual void organizedModCollectionChanged(const std::vector<std::shared_ptr<Mod>> & organizedMods);
		virtual void organizedModGameVersionCollectionChanged(const std::vector<std::shared_ptr<GameVersion>> & organizedMods);
		virtual void organizedModTeamCollectionChanged(const std::vector<std::shared_ptr<ModAuthorInformation>> & organizedMods);
		virtual void organizedModAuthorCollectionChanged(const std::vector<std::shared_ptr<ModAuthorInformation>> & organizedMods);
	};

	OrganizedModCollection(std::shared_ptr<ModCollection> mods = nullptr, std::shared_ptr<FavouriteModCollection> favourites = nullptr, std::shared_ptr<GameVersionCollection> gameVersions = nullptr);
	OrganizedModCollection(OrganizedModCollection && m) noexcept;
	OrganizedModCollection(const OrganizedModCollection & m);
	OrganizedModCollection & operator = (OrganizedModCollection && m) noexcept;
	OrganizedModCollection & operator = (const OrganizedModCollection & m);
	virtual ~OrganizedModCollection();

	std::shared_ptr<ModCollection> getModCollection() const;
	const std::vector<std::shared_ptr<Mod>> & getOrganizedMods() const;
	const std::vector<std::shared_ptr<GameVersion>> & getOrganizedGameVersions() const;
	std::shared_ptr<FavouriteModCollection> getFavouriteModCollection() const;
	std::shared_ptr<GameVersionCollection> getGameVersionCollection() const;
	std::vector<std::string> getOrganizedItemDisplayNames(bool prependItemNumber = true) const;
	FilterType getFilterType() const;
	SortType getSortType() const;
	SortDirection getSortDirection() const;

	void setModCollection(std::shared_ptr<ModCollection> mods);
	void setFavouriteModCollection(std::shared_ptr<FavouriteModCollection> favourites);
	void setGameVersionCollection(std::shared_ptr<GameVersionCollection> gameVersions);
	bool setFilterType(FilterType filterType);
	bool setSortType(SortType sortType);
	bool setSortDirection(SortDirection sortDirection);
	bool setSortOptions(SortType sortType, SortDirection sortDirection);

	bool shouldDisplayMods() const;
	bool shouldDisplayGameVersions() const;
	bool shouldDisplayTeams() const;
	bool shouldDisplayAuthors() const;
	size_t indexOfSelectedItem() const;
	bool selectItem(size_t index);
	bool selectRandomItem();
	void clearSelectedItems();

	size_t numberOfMods() const;
	bool hasMod(const Mod & mod) const;
	bool hasMod(const std::string & id) const;
	bool hasModWithName(const std::string & name) const;
	size_t indexOfMod(const Mod & mod) const;
	size_t indexOfMod(const std::string & id) const;
	size_t indexOfModWithName(const std::string & name) const;
	std::shared_ptr<Mod> getMod(size_t index) const;
	std::shared_ptr<Mod> getMod(const std::string & id) const;
	std::shared_ptr<Mod> getModWithName(const std::string & name) const;
	std::shared_ptr<Mod> getSelectedMod() const;
	bool setSelectedMod(size_t index);
	bool setSelectedMod(const std::string & name);
	bool setSelectedMod(const Mod * mod);
	bool selectRandomMod();
	void clearSelectedMod();

	size_t numberOfGameVersions() const;
	bool hasGameVersion(const GameVersion & gameVersion) const;
	bool hasGameVersion(const std::string & gameVersion) const;
	size_t indexOfGameVersion(const GameVersion & gameVersion) const;
	size_t indexOfGameVersion(const std::string & gameVersion) const;
	std::shared_ptr<GameVersion> getGameVersion(size_t index) const;
	std::shared_ptr<GameVersion> getGameVersion(const std::string & gameVersion) const;
	bool hasSelectedGameVersion() const;
	std::shared_ptr<GameVersion> getSelectedGameVersion() const;
	bool setSelectedGameVersion(size_t index);
	bool setSelectedGameVersion(const std::string & gameVersion);
	bool setSelectedGameVersion(const GameVersion * gameVersion);
	bool selectRandomGameVersion();
	void clearSelectedGameVersion();

	size_t getSupportedModCountForGameVersion(const GameVersion & gameVersion) const;
	size_t getSupportedModCountForGameVersion(const std::string & gameVersion) const;
	size_t getCompatibleModCountForGameVersion(const GameVersion & gameVersion) const;
	size_t getCompatibleModCountForGameVersion(const std::string & gameVersion) const;

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

	virtual void modCollectionUpdated() override;
	virtual void favouriteModCollectionUpdated() override;
	virtual void gameVersionCollectionUpdated() override;

	void organize();

	bool areCurrentSortOptionsValidInCurrentContext();
	bool areSortOptionsValidInCurrentContext(SortType sortType, FilterType filterType);
	static bool areSortOptionsValidInContext(SortType sortType, FilterType filterType, bool hasSelectedGameVersion, bool hasSelectedTeam, bool hasSelectedAuthor);

	size_t numberOfListeners() const;
	bool hasListener(const Listener & listener) const;
	size_t indexOfListener(const Listener & listener) const;
	Listener * getListener(size_t index) const;
	bool addListener(Listener & listener);
	bool removeListener(size_t index);
	bool removeListener(const Listener & listener);
	void clearListeners();
	void notifyFilterTypeChanged();
	void notifySortOptionsChanged();
	void notifySelectedModChanged();
	void notifySelectedGameVersionChanged();
	void notifySelectedTeamChanged();
	void notifySelectedAuthorChanged();
	void notifyOrganizedModCollectionChanged();
	void notifyOrganizedModGameVersionCollectionChanged();
	void notifyOrganizedModTeamCollectionChanged();
	void notifyOrganizedModAuthorCollectionChanged();

	bool operator == (const OrganizedModCollection & m) const;
	bool operator != (const OrganizedModCollection & m) const;

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

	std::vector<std::shared_ptr<Mod>> mergeSortMods(std::vector<std::shared_ptr<Mod>> mods);
	std::vector<std::shared_ptr<Mod>> mergeMods(std::vector<std::shared_ptr<Mod>> left, std::vector<std::shared_ptr<Mod>> right);
	std::vector<std::shared_ptr<GameVersion>> mergeSortGameVersions(std::vector<std::shared_ptr<GameVersion>> gameVersions);
	std::vector<std::shared_ptr<GameVersion>> mergeGameVersions(std::vector<std::shared_ptr<GameVersion>> left, std::vector<std::shared_ptr<GameVersion>> right);
	std::vector<std::shared_ptr<ModAuthorInformation>> mergeSortAuthors(std::vector<std::shared_ptr<ModAuthorInformation>> authors);
	std::vector<std::shared_ptr<ModAuthorInformation>> mergeAuthors(std::vector<std::shared_ptr<ModAuthorInformation>> left, std::vector<std::shared_ptr<ModAuthorInformation>> right);

	FilterType m_filterType;
	SortType m_sortType;
	SortDirection m_sortDirection;
	std::shared_ptr<ModCollection> m_mods;
	std::shared_ptr<FavouriteModCollection> m_favourites;
	std::shared_ptr<GameVersionCollection> m_gameVersions;
	std::vector<std::shared_ptr<Mod>> m_organizedMods;
	std::vector<std::shared_ptr<GameVersion>> m_organizedGameVersions;
	std::vector<std::shared_ptr<ModAuthorInformation>> m_teams;
	std::vector<std::shared_ptr<ModAuthorInformation>> m_authors;
	std::map<std::string, size_t> m_gameVersionSupportedModCountMap;
	std::map<std::string, size_t> m_gameVersionCompatibleModCountMap;
	std::shared_ptr<Mod> m_selectedMod;
	std::shared_ptr<GameVersion> m_selectedGameVersion;
	std::shared_ptr<ModAuthorInformation> m_selectedTeam;
	std::shared_ptr<ModAuthorInformation> m_selectedAuthor;
	std::vector<Listener *> m_listeners;
};

#endif // _ORGANIZED_MOD_COLLECTION_H_
