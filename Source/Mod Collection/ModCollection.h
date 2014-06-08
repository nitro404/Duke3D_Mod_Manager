#ifndef MOD_COLLECTION_H
#define MOD_COLLECTION_H

#include <QVector.h>
#include <QString.h>
#include <QFileInfo.h>
#include <QXmlStreamReader>
#include "Utilities/Utilities.h"
#include "Settings Manager/SettingsManager.h"
#include "Mod Collection/Mod.h"

class ModCollection {
public:
	ModCollection();
	ModCollection(const ModCollection & m);
	ModCollection & operator = (const ModCollection & m);
	~ModCollection();
	
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
	bool removeMod(const char * name);
	bool removeMod(const QString & name);
	bool removeMod(const Mod & mod);
	void clear();
	
	bool load();
	bool loadFrom(const char * fileName);
	bool loadFromINI(const char * fileName);
	bool loadFromXML(const char * fileName);

public:
	bool operator == (const ModCollection & m) const;
	bool operator != (const ModCollection & m) const;

private:
	QVector<Mod *> m_mods;
};

#endif // MOD_COLLECTION_H
