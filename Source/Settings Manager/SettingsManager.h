#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include "Singleton/Singleton.h"
#include "Utilities/Utilities.h"
#include "Variable System/VariableSystem.h"
#include "Mod Manager/ArgumentParser.h"
#include "Mod Manager/ModManagerMode.h"
#include "Mod Manager/GameType.h"

class SettingsManager : public Singleton<SettingsManager> {
	friend class Singleton<SettingsManager>;

protected:
	SettingsManager();
	~SettingsManager();
	
public:
	void reset();
	
	bool load(const ArgumentParser * args = NULL);
	bool save(const ArgumentParser * args = NULL) const;
	bool loadFrom(const char * fileName);
	bool loadFrom(const QString & fileName);
	bool saveTo(const char * fileName) const;
	bool saveTo(const QString & fileName) const;

public:
	const static char * defaultSettingsFileName;
	const static char * defaultModListFileName;
	const static char * defaultFavouritesListFileName;
	const static char * defaultModsDirectoryName;
	const static char * defaultDataDirectoryName;
	const static char * defaultGameFileName;
	const static char * defaultSetupFileName;
	const static char * defaultKextractFileName;
	const static char * defaultDOSBoxPath;
	const static char * defaultDOSBoxArgs;
	const static char * defaultDOSBoxGameScriptFileName;
	const static char * defaultDOSBoxSetupScriptFileName;
	const static char * defaultDOSBoxClientScriptFileName;
	const static char * defaultDOSBoxServerScriptFileName;
	const static char * defaultWindowsGameScriptFileName;
	const static char * defaultWindowsSetupScriptFileName;
	const static ModManagerModes::ModManagerMode defaultModManagerMode;
	const static GameTypes::GameType defaultGameType;
	const static char * defaultServerIPAddress;
	
	char * modListFileName;
	char * favouritesListFileName;
	char * modsDirectoryName;
	char * dataDirectoryName;
	char * gameFileName;
	char * setupFileName;
	char * kextractFileName;
	char * DOSBoxPath;
	char * DOSBoxArgs;
	char * DOSBoxGameScriptFileName;
	char * DOSBoxSetupScriptFileName;
	char * DOSBoxClientScriptFileName;
	char * DOSBoxServerScriptFileName;
	char * windowsGameScriptFileName;
	char * windowsSetupScriptFileName;
	ModManagerModes::ModManagerMode modManagerMode;
	GameTypes::GameType gameType;
	char * serverIPAddress;

private:
	VariableSystem * m_variables;
};

#endif // SETTINGS_MANAGER_H
