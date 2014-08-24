#ifndef ORGANIZED_MOD_COLLECTION_H
#define ORGANIZED_MOD_COLLECTION_H

#include "Mod Collection/ModCollection.h"
#include "Mod Collection/ModFilterType.h"
#include "Mod Collection/ModSortType.h"
#include "Mod Collection/ModSortDirection.h"
#include "Mod Collection/ModCollectionListener.h"

class OrganizedModCollection : public ModCollectionListener {
public:
	OrganizedModCollection(ModCollection * mods = NULL);
	OrganizedModCollection(const OrganizedModCollection & m);
	OrganizedModCollection & operator = (const OrganizedModCollection & m);
	virtual ~OrganizedModCollection();

	ModCollection * getModCollection() const;
	void setModCollection(ModCollection * mods);

	ModFilterTypes::ModFilterType getFilterType() const;
	ModSortTypes::ModSortType getSortType() const;
	ModSortDirections::ModSortDirection getSortDirection() const;
	
	bool setFilterType(ModFilterTypes::ModFilterType filterType);
	bool setSortType(ModSortTypes::ModSortType sortType);
	bool setSortDirection(ModSortDirections::ModSortDirection sortDirection);

	virtual void modCollectionUpdated();
	bool organizeModCollection();
	
	bool operator == (const OrganizedModCollection & m) const;
	bool operator != (const OrganizedModCollection & m) const;

private:
	ModCollection * m_mods;
	ModFilterTypes::ModFilterType m_filterType;
	ModSortTypes::ModSortType m_sortType;
	ModSortDirections::ModSortDirection m_sortDirection;
};

#endif // ORGANIZED_MOD_COLLECTION_H
