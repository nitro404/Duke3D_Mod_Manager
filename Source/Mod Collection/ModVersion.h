#ifndef MOD_VERSION_H
#define MOD_VERSION_H

#include <QString.h>
#include <QVector.h>
#include "Utilities/Utilities.h"
#include "Mod Collection/GameVersion.h"
#include "Mod Collection/ModGameVersion.h"

class ModVersion {
public:
	ModVersion(const char * version = NULL);
	ModVersion(const QString & version);
	ModVersion(const ModVersion & m);
	ModVersion & operator = (const ModVersion & m);
	virtual ~ModVersion();

	const char * getVersion() const;
	void setVersion(const char * version);
	void setVersion(const QString & version);

	int numberOfGameVersions() const;
	bool hasGameVersion(GameVersions::GameVersion version) const;
	bool hasGameVersion(const char * data) const;
	bool hasGameVersion(const QString & data) const;
	bool hasGameVersion(const ModGameVersion * version) const;
	int indexOfGameVersion(GameVersions::GameVersion version) const;
	int indexOfGameVersion(const char * data) const;
	int indexOfGameVersion(const QString & data) const;
	int indexOfGameVersion(const ModGameVersion * version) const;
	const ModGameVersion * getGameVersion(int index) const;
	const ModGameVersion * getGameVersion(GameVersions::GameVersion version) const;
	const ModGameVersion * getGameVersion(const char * data) const;
	const ModGameVersion * getGameVersion(const QString & data) const;
	bool addGameVersion(ModGameVersion * version);
	bool removeGameVersion(int index);
	bool removeGameVersion(GameVersions::GameVersion version);
	bool removeGameVersion(const char * data);
	bool removeGameVersion(const QString & data);
	bool removeGameVersion(const ModGameVersion * version);
	void clearGameVersions();

	bool isValid() const;
	static bool isValid(const ModVersion * m);
	
	bool operator == (const ModVersion & m) const;
	bool operator != (const ModVersion & m) const;

private:
	char * m_version;
	QVector<ModGameVersion *> m_gameVersions;
};

#endif // MOD_VERSION_H
