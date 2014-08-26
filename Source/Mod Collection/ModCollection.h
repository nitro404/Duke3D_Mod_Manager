#ifndef MOD_COLLECTION_H
#define MOD_COLLECTION_H

#include <QVector.h>
#include <QString.h>
#include <QFileInfo.h>
#include <QXmlStreamReader>
#include "Utilities/Utilities.h"
#include "Settings Manager/SettingsManager.h"
#include "Mod Collection/Mod.h"
#include "Mod Collection/ModCollectionBroadcaster.h"

class ModCollection : public ModCollectionBroadcaster {
public:
	ModCollection();
	ModCollection(const ModCollection & m);
	ModCollection & operator = (const ModCollection & m);
	virtual ~ModCollection();

	const char * getID() const;

	void setID(const char * id);
	void setID(const QString & id);
	
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
	bool addMod(Mod * mod);
	bool removeMod(int index);
	bool removeMod(const Mod & mod);
	bool removeMod(const char * name);
	bool removeMod(const QString & name);
	void clearMods();
	
	bool load();
	bool loadFrom(const QString & fileName);
	bool loadFrom(const char * fileName);
	bool loadFromINI(const QString & fileName);
	bool loadFromINI(const char * fileName);
	bool loadFromXML(const QString & fileName);
	bool loadFromXML(const char * fileName);
	
	bool operator == (const ModCollection & m) const;
	bool operator != (const ModCollection & m) const;

private:
	void notifyCollectionChanged() const;

private:
	char * m_id;
	QVector<Mod *> m_mods;
};

#endif // MOD_COLLECTION_H
