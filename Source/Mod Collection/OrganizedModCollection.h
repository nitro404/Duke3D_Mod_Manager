#ifndef ORGANIZED_MOD_COLLECTION_H
#define ORGANIZED_MOD_COLLECTION_H

#include <QMap.h>
#include <QString.h>
#include <QStringList.h>
#include "Utilities/Utilities.h"
#include "Mod Collection/ModCollection.h"
#include "Mod Collection/FavouriteModCollection.h"
#include "Mod Collection/ModAuthorInformation.h"
#include "Mod Collection/ModFilterType.h"
#include "Mod Collection/ModSortType.h"
#include "Mod Collection/ModSortDirection.h"
#include "Mod Collection/ModCollectionListener.h"

class OrganizedModCollection : public ModCollectionListener {
public:
	OrganizedModCollection(ModCollection * mods = NULL, FavouriteModCollection * favourites = NULL);
	OrganizedModCollection(const OrganizedModCollection & m);
	OrganizedModCollection & operator = (const OrganizedModCollection & m);
	virtual ~OrganizedModCollection();

	void init(ModCollection * mods, FavouriteModCollection * favourites);
	void uninit();

	ModCollection * getModCollection() const;
	FavouriteModCollection * getFavouriteModCollection() const;

	void setModCollection(ModCollection * mods);
	void setFavouriteModCollection(FavouriteModCollection * favourites);

	ModFilterTypes::ModFilterType getFilterType() const;
	ModSortTypes::ModSortType getSortType() const;
	ModSortDirections::ModSortDirection getSortDirection() const;
	
	bool setFilterType(ModFilterTypes::ModFilterType filterType);
	bool setSortType(ModSortTypes::ModSortType sortType);
	bool setSortDirection(ModSortDirections::ModSortDirection sortDirection);

	int numberOfMods() const;
	bool hasMod(const Mod & mod) const;
	bool hasMod(const char * name) const;
	bool hasMod(const QString & name) const;
	int indexOfMod(const Mod & mod) const;
	int indexOfMod(const char * name) const;
	int indexOfMod(const QString & name) const;
	const Mod * getMod(int index) const;
	const Mod * getMod(const char * name) const;
	const Mod * getMod(const QString & name) const;

	int numberOfTeams() const;
	bool hasTeamInfo(const char * name) const;
	bool hasTeamInfo(const QString & name) const;
	const ModAuthorInformation * getTeamInfo(int index) const;
	const ModAuthorInformation * getTeamInfo(const char * name) const;
	const ModAuthorInformation * getTeamInfo(const QString & name) const;
	void incrementTeamModCount(int index);
	void incrementTeamModCount(const char * name);
	void incrementTeamModCount(const QString & name);
	bool setSelectedTeam(int index);
	bool setSelectedTeam(const char * name);
	bool setSelectedTeam(const QString & name);
	bool setSelectedTeam(const ModAuthorInformation * teamInfo);
	void clearSelectedTeam();

	int numberOfAuthors() const;
	bool hasAuthorInfo(const char * name) const;
	bool hasAuthorInfo(const QString & name) const;
	const ModAuthorInformation * getAuthorInfo(int index) const;
	const ModAuthorInformation * getAuthorInfo(const char * name) const;
	const ModAuthorInformation * getAuthorInfo(const QString & name) const;
	void incrementAuthorModCount(int index);
	void incrementAuthorModCount(const char * name);
	void incrementAuthorModCount(const QString & name);
	bool setSelectedAuthor(int index);
	bool setSelectedAuthor(const char * name);
	bool setSelectedAuthor(const QString & name);
	bool setSelectedAuthor(const ModAuthorInformation * authorInfo);
	void clearSelectedAuthor();

	virtual void modCollectionUpdated();
	virtual void favouriteModCollectionUpdated();

	void organizeModCollection();
	void filterModCollection();
	void sortModCollection();
	void updateTeamList();
	void updateAuthorList();
	
	bool operator == (const OrganizedModCollection & m) const;
	bool operator != (const OrganizedModCollection & m) const;

private:
	QVector<const Mod *> mergeSortMods(QVector<const Mod *> mods);
	QVector<const Mod *> mergeMods(QVector<const Mod *> left, QVector<const Mod *> right);
	QVector<ModAuthorInformation *> mergeSortAuthors(QVector<ModAuthorInformation *> authors);
	QVector<ModAuthorInformation *> mergeAuthors(QVector<ModAuthorInformation *> left, QVector<ModAuthorInformation *> right);

private:
	ModCollection * m_mods;
	FavouriteModCollection * m_favourites;
	QVector<const Mod *> m_organizedMods;
	QVector<ModAuthorInformation *> m_teams;
	QVector<ModAuthorInformation *> m_authors;
	const ModAuthorInformation * m_selectedTeam;
	const ModAuthorInformation * m_selectedAuthor;
	ModFilterTypes::ModFilterType m_filterType;
	ModSortTypes::ModSortType m_sortType;
	ModSortDirections::ModSortDirection m_sortDirection;
};

#endif // ORGANIZED_MOD_COLLECTION_H
