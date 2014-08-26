#ifndef FAVOURITE_MOD_COLLECTION_H
#define FAVOURITE_MOD_COLLECTION_H

#include <QVector.h>
#include <QString.h>
#include <QFileInfo.h>
#include <QXmlStreamReader>
#include "Utilities/Utilities.h"
#include "Settings Manager/SettingsManager.h"
#include "Mod Collection/Mod.h"
#include "Mod Collection/ModCollection.h"
#include "Mod Collection/ModInformation.h"

class FavouriteModCollection : public ModCollectionBroadcaster {
public:
	FavouriteModCollection(ModCollection * mods = NULL);
	FavouriteModCollection(const FavouriteModCollection & m);
	FavouriteModCollection & operator = (const FavouriteModCollection & m);
	virtual ~FavouriteModCollection();

	bool init(ModCollection * mods, bool loadFavouriteMods = true);
	void uninit(bool saveFavouriteMods = true);

	int numberOfFavourites();
	bool hasFavourite(const ModInformation & favourite) const;
	bool hasFavourite(const char * name) const;
	bool hasFavourite(const QString & name) const;
	bool hasFavourite(const char * name, const char * version) const;
	bool hasFavourite(const QString & name, const QString & version) const;
	int indexOfFavourite(const ModInformation & favourite) const;
	int indexOfFavourite(const char * name) const;
	int indexOfFavourite(const QString & name) const;
	int indexOfFavourite(const char * name, const char * version) const;
	int indexOfFavourite(const QString & name, const QString & version) const;
	const ModInformation * getFavourite(int index) const;
	const ModInformation * getFavourite(const char * name) const;
	const ModInformation * getFavourite(const QString & name) const;
	const ModInformation * getFavourite(const char * name, const char * version) const;
	const ModInformation * getFavourite(const QString & name, const QString & version) const;
	bool addFavourite(ModInformation * favourite);
	bool removeFavourite(int index);
	bool removeFavourite(const ModInformation & favourite);
	bool removeFavourite(const char * name);
	bool removeFavourite(const QString & name);
	bool removeFavourite(const char * name, const char * version);
	bool removeFavourite(const QString & name, const QString & version);
	void clearFavourites();

	bool loadFavourites();
	bool loadFavourites(const QString & fileName);
	bool loadFavourites(const char * fileName);
	bool loadFavouritesList(const QString & fileName);
	bool loadFavouritesList(const char * fileName);
	bool loadFavouritesXML(const QString & fileName);
	bool loadFavouritesXML(const char * fileName);
	bool saveFavourites();
	bool saveFavourites(const QString & fileName);
	bool saveFavourites(const char * fileName);
	bool saveFavouritesList(const QString & fileName);
	bool saveFavouritesList(const char * fileName);
	bool saveFavouritesXML(const QString & fileName);
	bool saveFavouritesXML(const char * fileName);

	int checkForMissingFavouriteMods() const;

	bool operator == (const FavouriteModCollection & m) const;
	bool operator != (const FavouriteModCollection & m) const;

private:
	void notifyFavouriteModsChanged() const;

private:
	ModCollection * m_mods;
	QVector<ModInformation *> m_favourites;
	bool m_initialized;
};

#endif // FAVOURITE_MOD_COLLECTION_H
