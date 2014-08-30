#ifndef MOD_MANAGER_H
#define MOD_MANAGER_H

#include <QString.h>
#include <QTextStream.h>
#include <QXmlStream.h>
#include "Singleton/Singleton.h"
#include "Mod Manager/ModManagerMode.h"
#include "Mod Manager/GameType.h"
#include "Mod Collection/Mod.h"
#include "Mod Collection/ModCollection.h"
#include "Mod Collection/FavouriteModCollection.h"
#include "Mod Collection/OrganizedModCollection.h"
#include "Script/Script.h"

class ModManager : public Singleton<ModManager> {
	friend class Singleton<ModManager>;
	
protected:
	ModManager();
	~ModManager();

public:
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
	int searchForAndSelectMod(const char * query);
	int searchForAndSelectMod(const QString & query);
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
	void runSelectedMod(const ArgumentParser * args = NULL);
	bool updateScriptArgs();

	bool handleArguments(const ArgumentParser * args, bool start);

	int checkForUnlinkedModFiles() const;
	int checkModForMissingFiles(const char * modName, int versionIndex = -1) const;
	int checkModForMissingFiles(const QString & modName, int versionIndex = -1) const;
	int checkModForMissingFiles(const Mod & mod, int versionIndex = -1) const;
	int checkAllModsForMissingFiles() const;
	static int checkForMissingExecutables();

	static void displayArgumentHelp();
	
private:
	bool m_initialized;
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
