#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include "Utilities/Utilities.h"
#include "Variable System/VariableCollection.h"
#include "Mod Manager/ArgumentParser.h"
#include "Mod Manager/ModManagerMode.h"
#include "Mod Manager/GameType.h"

class SettingsManager {
public:
	SettingsManager();
	SettingsManager(const SettingsManager & s);
	SettingsManager & operator = (const SettingsManager & s);
	~SettingsManager();

	static SettingsManager * getInstance();
	void updateInstance();
	
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
	const static char * defaultModsDirectoryPath;
	const static char * defaultDataDirectoryName;
	const static char * defaultGameFileName;
	const static char * defaultSetupFileName;
	const static char * defaultKextractFileName;
	const static char * defaultGamePath;
	const static char * defaultDOSBoxFileName;
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
	const static int defaultLocalServerPort;
	const static int defaultRemoteServerPort;
	
	char * modListFileName;
	char * favouritesListFileName;
	char * modsDirectoryName;
	char * modsDirectoryPath;
	char * dataDirectoryName;
	char * gameFileName;
	char * setupFileName;
	char * kextractFileName;
	char * gamePath;
	char * DOSBoxFileName;
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
	int localServerPort;
	int remoteServerPort;

private:
	static SettingsManager * instance;
	VariableCollection * m_variables;
};

#endif // SETTINGS_MANAGER_H
