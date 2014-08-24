#include "Mod Collection/OrganizedModCollection.h"

OrganizedModCollection::OrganizedModCollection(ModCollection * mods)
	: m_mods(mods)
	, m_filterType(ModFilterTypes::defaultFilterType)
	, m_sortType(ModSortTypes::defaultSortType)
	, m_sortDirection(ModSortDirections::defaultSortDirection) {
	if(m_mods != NULL) {
		m_mods->addListener(dynamic_cast<ModCollectionListener *>(this));
	}

	organizeModCollection();
}

OrganizedModCollection::OrganizedModCollection(const OrganizedModCollection & m)
	: m_mods(m.m_mods)
	, m_filterType(m.m_filterType)
	, m_sortType(m.m_sortType)
	, m_sortDirection(m.m_sortDirection) {
	if(m_mods != NULL) {
		m_mods->addListener(dynamic_cast<ModCollectionListener *>(this));
	}

	organizeModCollection();
}

OrganizedModCollection & OrganizedModCollection::operator = (const OrganizedModCollection & m) {
	bool organize = m_mods != m.m_mods ||
					m_filterType != m.m_filterType ||
					m_sortType != m.m_sortType ||
					m_sortDirection != m.m_sortDirection;

	if(m_mods != NULL) {
		m_mods->removeListener(dynamic_cast<const ModCollectionListener *>(this));
	}

	m_mods = m.m_mods;
	m_filterType = m.m_filterType;
	m_sortType = m.m_sortType;
	m_sortDirection = m.m_sortDirection;

	if(m_mods != NULL) {
		m_mods->addListener(dynamic_cast<ModCollectionListener *>(this));
	}

	if(organize) {
		organizeModCollection();
	}

	return *this;
}

OrganizedModCollection::~OrganizedModCollection() {
	m_mods->removeListener(dynamic_cast<const ModCollectionListener *>(this));
}

ModCollection * OrganizedModCollection::getModCollection() const {
	return m_mods;
}

void OrganizedModCollection::setModCollection(ModCollection * mods) {
	bool organize = m_mods != mods;

	if(m_mods != NULL) {
		m_mods->removeListener(dynamic_cast<const ModCollectionListener *>(this));
	}

	m_mods = mods;

	if(m_mods != NULL) {
		m_mods->addListener(dynamic_cast<ModCollectionListener *>(this));
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
}

bool OrganizedModCollection::setSortType(ModSortTypes::ModSortType sortType) {
	if(!ModSortTypes::isValid(sortType)) {
		return false;
	}

	bool organize = m_sortType != sortType;
	m_sortType = sortType;

	if(organize) {
		organizeModCollection();
	}
}

bool OrganizedModCollection::setSortDirection(ModSortDirections::ModSortDirection sortDirection) {
	if(!ModSortDirections::isValid(sortDirection)) {
		return false;
	}

	bool organize = m_sortDirection != sortDirection;
	m_sortDirection = sortDirection;

	if(organize) {
		organizeModCollection();
	}
}

void OrganizedModCollection::modCollectionUpdated() {
	organizeModCollection();
}

bool OrganizedModCollection::organizeModCollection() {
	if(m_mods == NULL) {
		return false;
	}

// TODO: filter and sort mod collection

	return true;
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
