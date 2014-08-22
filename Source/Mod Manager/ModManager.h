#ifndef MOD_MANAGER_H
#define MOD_MANAGER_H

#include <QString.h>
#include <QTextStream.h>
#include <QXmlStream.h>
#include "Singleton/Singleton.h"
#include "Mod Manager/ModInformation.h"
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
	bool init(const ArgumentParser * args = NULL);
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

// TODO: add category filter enum & functions

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

	void runMenu();
	void runGameTypePrompt();
	void runModePrompt();
	void runIPAddressPrompt();
	void runSearchPrompt();
	bool runModVersionSelectionPrompt();
	void runSelectedMod(const ArgumentParser * args = NULL);
	bool updateScriptArgs();

	bool handleArguments(const ArgumentParser * args);

	int checkForUnlinkedModFiles() const;
	int checkModForMissingFiles(const char * modName, int versionIndex = -1) const;
	int checkModForMissingFiles(const QString & modName, int versionIndex = -1) const;
	int checkModForMissingFiles(const Mod & mod, int versionIndex = -1) const;
	int checkAllModsForMissingFiles() const;
	static int checkForMissingExecutables();
	
private:
	bool m_initialized;
	ModManagerModes::ModManagerMode m_mode;
	GameTypes::GameType m_gameType;
	const Mod * m_selectedMod;
	int m_selectedModVersionIndex;
	ModCollection m_mods;
	QVector<ModInformation *> m_favourites;
	ScriptArguments m_scriptArgs;
};

#endif // MOD_MANAGER_H
