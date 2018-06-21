#ifndef MOD_MANAGER_H
#define MOD_MANAGER_H

#include <QString.h>
#include <QTextStream.h>
#include <QRegExp.h>
#include <QXmlStream.h>
#include "Mod Manager/ModManagerMode.h"
#include "Mod Manager/GameType.h"
#include "Mod Collection/Mod.h"
#include "Mod Collection/ModCollection.h"
#include "Mod Collection/FavouriteModCollection.h"
#include "Mod Collection/OrganizedModCollection.h"
#include "Script/Script.h"

class ModManager {
public:
	ModManager();
	~ModManager();

	bool init(int argc, char * argv[], bool start = true);
	bool init(const ArgumentParser * args = NULL, bool start = true);
	bool uninit();

	void run();

	ModManagerModes::ModManagerMode getMode() const;
	const char * getModeName() const;
	bool setMode(const char * modeName);
	bool setMode(const QString & modeName);
	bool setMode(int mode);
	bool setMode(ModManagerModes::ModManagerMode mode);

	GameTypes::GameType getGameType() const;
	const char * getGameTypeName() const;
	bool setGameType(const char * gameTypeName);
	bool setGameType(const QString & gameTypeName);
	bool setGameType(int gameType);
	bool setGameType(GameTypes::GameType gameType);

	const char * getServerIPAddress() const;
	void setServerIPAddress(const char * ipAddress);
	void setServerIPAddress(const QString & ipAddress);

	const Mod * getSelectedMod() const;
	const char * getSelectedModName() const;
	bool setSelectedMod(const char * name);
	bool setSelectedMod(const QString & name);
	void selectRandomMod();
	bool selectRandomTeam();
	bool selectRandomAuthor();
	int searchForAndSelectMod(const char * query);
	int searchForAndSelectMod(const QString & query);
	int searchForAndSelectTeam(const char * query);
	int searchForAndSelectTeam(const QString & query);
	int searchForAndSelectAuthor(const char * query);
	int searchForAndSelectAuthor(const QString & query);
	void clearSelectedMod();

	void runMenu();
	void runFilterPrompt(const QString & args = QString());
	void runSortPrompt(const QString & args = QString());
	void runGameTypePrompt(const QString & args = QString());
	void runModePrompt(const QString & args = QString());
	void runIPAddressPrompt(const QString & args = QString());
	void runSelectRandomModPrompt();
	void runSearchPrompt(const QString & args = QString());
	bool runModVersionSelectionPrompt();
	bool runSelectedMod(const ArgumentParser * args = NULL);
	bool updateScriptArgs();

	bool handleArguments(const ArgumentParser * args, bool start);

	int checkForUnlinkedModFiles() const;
	int checkModForMissingFiles(const char * modName, int versionIndex = -1) const;
	int checkModForMissingFiles(const QString & modName, int versionIndex = -1) const;
	int checkModForMissingFiles(const Mod & mod, int versionIndex = -1) const;
	int checkAllModsForMissingFiles() const;
	static int checkForMissingExecutables();

	static void displayArgumentHelp();

	static const char * VERSION;
	
private:
	bool m_initialized;
	ArgumentParser * m_arguments;
	SettingsManager m_settings;
	ModManagerModes::ModManagerMode m_mode;
	GameTypes::GameType m_gameType;
	const Mod * m_selectedMod;
	int m_selectedModVersionIndex;
	ModCollection m_mods;
	OrganizedModCollection m_organizedMods;
	FavouriteModCollection m_favouriteMods;
	ScriptArguments m_scriptArgs;
};

#endif // MOD_MANAGER_H
