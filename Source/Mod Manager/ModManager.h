#ifndef MOD_MANAGER_H
#define MOD_MANAGER_H

#include <QString.h>
#include <QTextStream.h>
#include "Singleton/Singleton.h"
#include "Mod Manager/ModManagerMode.h"
#include "Mod Manager/GameType.h"
#include "Mod Collection/Mod.h"
#include "Mod Collection/ModCollection.h"
#include "Script/Script.h"

class ModManager : public Singleton<ModManager> {
	friend class Singleton<ModManager>;
	
protected:
	ModManager();
	~ModManager();

public:
	bool init(bool openMenu = true, const ArgumentParser * args = NULL);
	bool uninit();

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

// TODO: add category filter enum & functions
// TODO: favourites container functions
	bool readFavourites();
	bool writeFavourites();

	void runMenu();
	void runGameTypePrompt();
	void runModePrompt();
	void runIPAddressPrompt();
	void runSearchPrompt();
	void runSelectedMod(const ArgumentParser * args = NULL);
	void updateScriptArgs();

	bool handleArguments(const ArgumentParser * args);

	int checkForUnlinkedModFiles() const;
	int checkModForMissingFiles(const char * modName) const;
	int checkModForMissingFiles(const QString & modName) const;
	int checkModForMissingFiles(const Mod & mod) const;
	int checkAllModsForMissingFiles() const;
	static int checkForMissingExecutables();
	
private:
	bool m_initialized;
	ModManagerModes::ModManagerMode m_mode;
	GameTypes::GameType m_gameType;
	const Mod * m_selectedMod;
	ModCollection m_mods;
	QVector<const Mod *> m_favourites;
	ScriptArguments m_scriptArgs;
};

#endif // MOD_MANAGER_H
