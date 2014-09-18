#include "Settings Manager/SettingsManager.h"

SettingsManager * SettingsManager::instance = NULL;

const char * SettingsManager::defaultSettingsFileName = "Duke Nukem 3D Mod Manager.ini";
const char * SettingsManager::defaultModListFileName = "Duke Nukem 3D Mod List.xml";
const char * SettingsManager::defaultFavouritesListFileName = "Duke Nukem 3D Favourite Mods.xml";
const char * SettingsManager::defaultModsDirectoryName = "Mods";
const char * SettingsManager::defaultModsDirectoryPath = "..";
const char * SettingsManager::defaultDataDirectoryName = "Data";
const char * SettingsManager::defaultGameFileName = "DUKE3D.EXE";
const char * SettingsManager::defaultSetupFileName = "SETUP.EXE";
const char * SettingsManager::defaultKextractFileName = "KEXTRACT.EXE";
const char * SettingsManager::defaultGamePath = ".";
const char * SettingsManager::defaultDOSBoxFileName = "dosbox.exe";
const char * SettingsManager::defaultDOSBoxPath = "DOSBox";
const char * SettingsManager::defaultDOSBoxArgs = "-noconsole";
const char * SettingsManager::defaultDOSBoxGameScriptFileName = "duke3d.conf";
const char * SettingsManager::defaultDOSBoxSetupScriptFileName = "duke3d_setup.conf";
const char * SettingsManager::defaultDOSBoxClientScriptFileName = "duke3d_client.conf";
const char * SettingsManager::defaultDOSBoxServerScriptFileName = "duke3d_server.conf";
const char * SettingsManager::defaultWindowsGameScriptFileName = "duke3d.bat";
const char * SettingsManager::defaultWindowsSetupScriptFileName = "duke3d_setup.bat";
const ModManagerModes::ModManagerMode SettingsManager::defaultModManagerMode = ModManagerModes::defaultMode;
const GameTypes::GameType SettingsManager::defaultGameType = GameTypes::defaultGameType;
const char * SettingsManager::defaultServerIPAddress = "127.0.0.1";

SettingsManager::SettingsManager()
	: m_variables(new VariableCollection())
	, modListFileName(NULL)
	, favouritesListFileName(NULL)
	, modsDirectoryName(NULL)
	, modsDirectoryPath(NULL)
	, dataDirectoryName(NULL)
	, gameFileName(NULL)
	, setupFileName(NULL)
	, kextractFileName(NULL)
	, gamePath(NULL)
	, DOSBoxFileName(NULL)
	, DOSBoxPath(NULL)
	, DOSBoxArgs(NULL)
	, DOSBoxGameScriptFileName(NULL)
	, DOSBoxSetupScriptFileName(NULL)
	, DOSBoxClientScriptFileName(NULL)
	, DOSBoxServerScriptFileName(NULL)
	, windowsGameScriptFileName(NULL)
	, windowsSetupScriptFileName(NULL)
	, modManagerMode(defaultModManagerMode)
	, gameType(defaultGameType)
	, serverIPAddress(NULL) {
	if(instance == NULL) {
		instance = this;
	}
	reset();
}

SettingsManager::SettingsManager(const SettingsManager & s)
	: m_variables(new VariableCollection(*s.m_variables))
	, modListFileName(NULL)
	, favouritesListFileName(NULL)
	, modsDirectoryName(NULL)
	, modsDirectoryPath(NULL)
	, dataDirectoryName(NULL)
	, gameFileName(NULL)
	, setupFileName(NULL)
	, kextractFileName(NULL)
	, gamePath(NULL)
	, DOSBoxFileName(NULL)
	, DOSBoxPath(NULL)
	, DOSBoxArgs(NULL)
	, DOSBoxGameScriptFileName(NULL)
	, DOSBoxSetupScriptFileName(NULL)
	, DOSBoxClientScriptFileName(NULL)
	, DOSBoxServerScriptFileName(NULL)
	, windowsGameScriptFileName(NULL)
	, windowsSetupScriptFileName(NULL)
	, modManagerMode(s.modManagerMode)
	, gameType(s.gameType)
	, serverIPAddress(NULL) {
	if(instance == NULL) {
		instance = this;
	}

	modListFileName = Utilities::copyString(s.modListFileName);
	favouritesListFileName = Utilities::copyString(s.favouritesListFileName);
	modsDirectoryName = Utilities::copyString(s.modsDirectoryName);
	modsDirectoryPath = Utilities::copyString(s.modsDirectoryPath);
	dataDirectoryName = Utilities::copyString(s.dataDirectoryName);
	gameFileName = Utilities::copyString(s.gameFileName);
	setupFileName = Utilities::copyString(s.setupFileName);
	kextractFileName = Utilities::copyString(s.kextractFileName);
	gamePath = Utilities::copyString(s.gamePath);
	DOSBoxFileName = Utilities::copyString(s.DOSBoxFileName);
	DOSBoxPath = Utilities::copyString(s.DOSBoxPath);
	DOSBoxArgs = Utilities::copyString(s.DOSBoxArgs);
	DOSBoxGameScriptFileName = Utilities::copyString(s.DOSBoxGameScriptFileName);
	DOSBoxSetupScriptFileName = Utilities::copyString(s.DOSBoxSetupScriptFileName);
	DOSBoxClientScriptFileName = Utilities::copyString(s.DOSBoxClientScriptFileName);
	DOSBoxServerScriptFileName = Utilities::copyString(s.DOSBoxServerScriptFileName);
	windowsGameScriptFileName = Utilities::copyString(s.windowsGameScriptFileName);
	windowsSetupScriptFileName = Utilities::copyString(s.windowsSetupScriptFileName);
	serverIPAddress = Utilities::copyString(s.serverIPAddress);
}

SettingsManager & SettingsManager::operator = (const SettingsManager & s) {
	if(instance == NULL) {
		instance = this;
	}

	if(modListFileName != NULL)				{ delete [] modListFileName; }
	if(favouritesListFileName != NULL)		{ delete [] favouritesListFileName; }
	if(modsDirectoryName != NULL)			{ delete [] modsDirectoryName; }
	if(modsDirectoryPath != NULL)			{ delete [] modsDirectoryPath; }
	if(dataDirectoryName != NULL)			{ delete [] dataDirectoryName; }
	if(gameFileName != NULL)				{ delete [] gameFileName; }
	if(setupFileName != NULL)				{ delete [] setupFileName; }
	if(kextractFileName != NULL)			{ delete [] kextractFileName; }
	if(gamePath != NULL)					{ delete [] gamePath; }
	if(DOSBoxFileName != NULL)				{ delete [] DOSBoxFileName; }
	if(DOSBoxPath != NULL)					{ delete [] DOSBoxPath; }
	if(DOSBoxArgs != NULL)					{ delete [] DOSBoxArgs; }
	if(DOSBoxGameScriptFileName != NULL)	{ delete [] DOSBoxGameScriptFileName; }
	if(DOSBoxSetupScriptFileName != NULL)	{ delete [] DOSBoxSetupScriptFileName; }
	if(DOSBoxClientScriptFileName != NULL)	{ delete [] DOSBoxClientScriptFileName; }
	if(DOSBoxServerScriptFileName != NULL)	{ delete [] DOSBoxServerScriptFileName; }
	if(windowsGameScriptFileName != NULL)	{ delete [] windowsGameScriptFileName; }
	if(windowsSetupScriptFileName != NULL)	{ delete [] windowsSetupScriptFileName; }
	if(serverIPAddress != NULL)				{ delete [] serverIPAddress; }

	delete m_variables;

	m_variables = new VariableCollection(*s.m_variables);

	modManagerMode = s.modManagerMode;
	gameType = s.gameType;

	modListFileName = Utilities::copyString(s.modListFileName);
	favouritesListFileName = Utilities::copyString(s.favouritesListFileName);
	modsDirectoryName = Utilities::copyString(s.modsDirectoryName);
	modsDirectoryPath = Utilities::copyString(s.modsDirectoryPath);
	dataDirectoryName = Utilities::copyString(s.dataDirectoryName);
	gameFileName = Utilities::copyString(s.gameFileName);
	setupFileName = Utilities::copyString(s.setupFileName);
	kextractFileName = Utilities::copyString(s.kextractFileName);
	gamePath = Utilities::copyString(s.gamePath);
	DOSBoxFileName = Utilities::copyString(s.DOSBoxFileName);
	DOSBoxPath = Utilities::copyString(s.DOSBoxPath);
	DOSBoxArgs = Utilities::copyString(s.DOSBoxArgs);
	DOSBoxGameScriptFileName = Utilities::copyString(s.DOSBoxGameScriptFileName);
	DOSBoxSetupScriptFileName = Utilities::copyString(s.DOSBoxSetupScriptFileName);
	DOSBoxClientScriptFileName = Utilities::copyString(s.DOSBoxClientScriptFileName);
	DOSBoxServerScriptFileName = Utilities::copyString(s.DOSBoxServerScriptFileName);
	windowsGameScriptFileName = Utilities::copyString(s.windowsGameScriptFileName);
	windowsSetupScriptFileName = Utilities::copyString(s.windowsSetupScriptFileName);
	serverIPAddress = Utilities::copyString(s.serverIPAddress);

	return *this;
}

SettingsManager::~SettingsManager() {
	if(modListFileName != NULL)				{ delete [] modListFileName; }
	if(favouritesListFileName != NULL)		{ delete [] favouritesListFileName; }
	if(modsDirectoryName != NULL)			{ delete [] modsDirectoryName; }
	if(modsDirectoryPath != NULL)			{ delete [] modsDirectoryPath; }
	if(dataDirectoryName != NULL)			{ delete [] dataDirectoryName; }
	if(gameFileName != NULL)				{ delete [] gameFileName; }
	if(setupFileName != NULL)				{ delete [] setupFileName; }
	if(kextractFileName != NULL)			{ delete [] kextractFileName; }
	if(gamePath != NULL)					{ delete [] gamePath; }
	if(DOSBoxFileName != NULL)				{ delete [] DOSBoxFileName; }
	if(DOSBoxPath != NULL)					{ delete [] DOSBoxPath; }
	if(DOSBoxArgs != NULL)					{ delete [] DOSBoxArgs; }
	if(DOSBoxGameScriptFileName != NULL)	{ delete [] DOSBoxGameScriptFileName; }
	if(DOSBoxSetupScriptFileName != NULL)	{ delete [] DOSBoxSetupScriptFileName; }
	if(DOSBoxClientScriptFileName != NULL)	{ delete [] DOSBoxClientScriptFileName; }
	if(DOSBoxServerScriptFileName != NULL)	{ delete [] DOSBoxServerScriptFileName; }
	if(windowsGameScriptFileName != NULL)	{ delete [] windowsGameScriptFileName; }
	if(windowsSetupScriptFileName != NULL)	{ delete [] windowsSetupScriptFileName; }
	if(serverIPAddress != NULL)				{ delete [] serverIPAddress; }

	delete m_variables;
}

SettingsManager * SettingsManager::getInstance() {
	return instance;
}

void SettingsManager::updateInstance() {
	instance = this;
}

void SettingsManager::reset() {
	if(modListFileName != NULL)				{ delete [] modListFileName; }
	if(favouritesListFileName != NULL)		{ delete [] favouritesListFileName; }
	if(modsDirectoryName != NULL)			{ delete [] modsDirectoryName; }
	if(modsDirectoryPath != NULL)			{ delete [] modsDirectoryPath; }
	if(dataDirectoryName != NULL)			{ delete [] dataDirectoryName; }
	if(gameFileName != NULL)				{ delete [] gameFileName; }
	if(setupFileName != NULL)				{ delete [] setupFileName; }
	if(kextractFileName != NULL)			{ delete [] kextractFileName; }
	if(gamePath != NULL)					{ delete [] gamePath; }
	if(DOSBoxFileName != NULL)				{ delete [] DOSBoxFileName; }
	if(DOSBoxPath != NULL)					{ delete [] DOSBoxPath; }
	if(DOSBoxArgs != NULL)					{ delete [] DOSBoxArgs; }
	if(DOSBoxGameScriptFileName != NULL)	{ delete [] DOSBoxGameScriptFileName; }
	if(DOSBoxSetupScriptFileName != NULL)	{ delete [] DOSBoxSetupScriptFileName; }
	if(DOSBoxClientScriptFileName != NULL)	{ delete [] DOSBoxClientScriptFileName; }
	if(DOSBoxServerScriptFileName != NULL)	{ delete [] DOSBoxServerScriptFileName; }
	if(windowsGameScriptFileName != NULL)	{ delete [] windowsGameScriptFileName; }
	if(windowsSetupScriptFileName != NULL)	{ delete [] windowsSetupScriptFileName; }
	if(serverIPAddress != NULL)				{ delete [] serverIPAddress; }

	modListFileName = Utilities::copyString(defaultModListFileName);
	favouritesListFileName = Utilities::copyString(defaultFavouritesListFileName);
	modsDirectoryName = Utilities::copyString(defaultModsDirectoryName);
	modsDirectoryPath = Utilities::copyString(defaultModsDirectoryPath);
	dataDirectoryName = Utilities::copyString(defaultDataDirectoryName);
	gameFileName = Utilities::copyString(defaultGameFileName);
	setupFileName = Utilities::copyString(defaultSetupFileName);
	kextractFileName = Utilities::copyString(defaultKextractFileName);
	gamePath = Utilities::copyString(defaultGamePath);
	DOSBoxFileName = Utilities::copyString(defaultDOSBoxFileName);
	DOSBoxPath = Utilities::copyString(defaultDOSBoxPath);
	DOSBoxArgs = Utilities::copyString(defaultDOSBoxArgs);
	DOSBoxGameScriptFileName = Utilities::copyString(defaultDOSBoxGameScriptFileName);
	DOSBoxSetupScriptFileName = Utilities::copyString(defaultDOSBoxSetupScriptFileName);
	DOSBoxClientScriptFileName = Utilities::copyString(defaultDOSBoxClientScriptFileName);
	DOSBoxServerScriptFileName = Utilities::copyString(defaultDOSBoxServerScriptFileName);
	windowsGameScriptFileName = Utilities::copyString(defaultWindowsGameScriptFileName);
	windowsSetupScriptFileName = Utilities::copyString(defaultWindowsSetupScriptFileName);
	serverIPAddress = Utilities::copyString(defaultServerIPAddress);

	modManagerMode = defaultModManagerMode;
	gameType = defaultGameType;
}

bool SettingsManager::load(const ArgumentParser * args) {
	if(args != NULL) {
		QString altSettingsFileName = args->getValue("f");
		if(altSettingsFileName.length() > 0) {
			QByteArray fileNameBytes = altSettingsFileName.toLocal8Bit();

			printf("Loading settings from alternate file: %s\n", fileNameBytes.data());

			bool loadedSettings = loadFrom(altSettingsFileName);
			if(!loadedSettings) {
				printf("Failed to load settings from alt settings file: %s\n", fileNameBytes.data());
			}
			return loadedSettings;
		}
	}

	return loadFrom(defaultSettingsFileName);
}

bool SettingsManager::save(const ArgumentParser * args) const {
	if(args != NULL) {
		QString altSettingsFileName = args->getValue("f");
		if(altSettingsFileName.length() > 0) {
			QByteArray fileNameBytes = altSettingsFileName.toLocal8Bit();

			printf("Saving settings to alternate file: %s\n", fileNameBytes.data());

			bool savedSettings = saveTo(altSettingsFileName);
			if(!savedSettings) {
				printf("Failed to save settings to alternate file: %s\n", fileNameBytes.data());
			}
			return savedSettings;
		}
	}

	return saveTo(defaultSettingsFileName);
}

bool SettingsManager::loadFrom(const QString & fileName) {
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	return loadFrom(fileNameBytes.data());
}

bool SettingsManager::loadFrom(const char * fileName) {
	VariableCollection * newVariables = VariableCollection::readFrom(fileName);
	if(newVariables == NULL) { return false; }

	delete m_variables;
	m_variables = newVariables;

	const char * data = NULL;

	data = m_variables->getValue("Mod List File Name", "Paths");
	if(data != NULL) {
		if(modListFileName != NULL) { delete [] modListFileName; }
		modListFileName = Utilities::copyString(data);
	}

	data = m_variables->getValue("Favourites List File Name", "Paths");
	if(data != NULL) {
		if(favouritesListFileName != NULL) { delete [] favouritesListFileName; }
		favouritesListFileName = Utilities::copyString(data);
	}

	data = m_variables->getValue("Mods Directory Name", "Paths");
	if(data != NULL) {
		if(modsDirectoryName != NULL) { delete [] modsDirectoryName; }
		modsDirectoryName = Utilities::copyString(data);
	}

	data = m_variables->getValue("Mods Directory Path", "Paths");
	if(data != NULL) {
		if(modsDirectoryPath != NULL) { delete [] modsDirectoryPath; }
		modsDirectoryPath = Utilities::copyString(data);
	}
	
	data = m_variables->getValue("Data Directory Name", "Paths");
	if(data != NULL) {
		if(dataDirectoryName != NULL) { delete [] dataDirectoryName; }
		dataDirectoryName = Utilities::copyString(data);
	}

	data = m_variables->getValue("Game File Name", "Paths");
	if(data != NULL) {
		if(gameFileName != NULL) { delete [] gameFileName; }
		gameFileName = Utilities::copyString(data);
	}

	data = m_variables->getValue("Setup File Name", "Paths");
	if(data != NULL) {
		if(setupFileName != NULL) { delete [] setupFileName; }
		setupFileName = Utilities::copyString(data);
	}

	data = m_variables->getValue("Kextract File Name", "Paths");
	if(data != NULL) {
		if(kextractFileName != NULL) { delete [] kextractFileName; }
		kextractFileName = Utilities::copyString(data);
	}
	
	data = m_variables->getValue("Game Path", "Paths");
	if(data != NULL) {
		if(gamePath != NULL) { delete [] gamePath; }
		gamePath = Utilities::copyString(data);
	}

	data = m_variables->getValue("DOSBox File Name", "Paths");
	if(data != NULL) {
		if(DOSBoxFileName != NULL) { delete [] DOSBoxFileName; }
		DOSBoxFileName = Utilities::copyString(data);
	}
	
	data = m_variables->getValue("DOSBox Path", "Paths");
	if(data != NULL) {
		if(DOSBoxPath != NULL) { delete [] DOSBoxPath; }
		DOSBoxPath = Utilities::copyString(data);
	}

	data = m_variables->getValue("DOSBox Arguments", "Arguments");
	if(data != NULL) {
		if(DOSBoxArgs != NULL) { delete [] DOSBoxArgs; }
		DOSBoxArgs = Utilities::copyString(data);
	}
	
	data = m_variables->getValue("DOSBox Game Script File Name", "Scripts");
	if(data != NULL) {
		if(DOSBoxGameScriptFileName != NULL) { delete [] DOSBoxGameScriptFileName; }
		DOSBoxGameScriptFileName = Utilities::copyString(data);
	}
	
	data = m_variables->getValue("DOSBox Setup Script File Name", "Scripts");
	if(data != NULL) {
		if(DOSBoxSetupScriptFileName != NULL) { delete [] DOSBoxSetupScriptFileName; }
		DOSBoxSetupScriptFileName = Utilities::copyString(data);
	}
	
	data = m_variables->getValue("DOSBox Client Script File Name", "Scripts");
	if(data != NULL) {
		if(DOSBoxClientScriptFileName != NULL) { delete [] DOSBoxClientScriptFileName; }
		DOSBoxClientScriptFileName = Utilities::copyString(data);
	}
	
	data = m_variables->getValue("DOSBox Server Script File Name", "Scripts");
	if(data != NULL) {
		if(DOSBoxServerScriptFileName != NULL) { delete [] DOSBoxServerScriptFileName; }
		DOSBoxServerScriptFileName = Utilities::copyString(data);
	}

	data = m_variables->getValue("Windows Game Script File Name", "Scripts");
	if(data != NULL) {
		if(windowsGameScriptFileName != NULL) { delete [] windowsGameScriptFileName; }
		windowsGameScriptFileName = Utilities::copyString(data);
	}
	
	data = m_variables->getValue("Windows Setup Script File Name", "Scripts");
	if(data != NULL) {
		if(windowsSetupScriptFileName != NULL) { delete [] windowsSetupScriptFileName; }
		windowsSetupScriptFileName = Utilities::copyString(data);
	}

	data = m_variables->getValue("Mod Manager Mode", "Options");
	if(data != NULL) {
		ModManagerModes::ModManagerMode newModManagerMode = ModManagerModes::parseFrom(data);
		if(ModManagerModes::isValid(newModManagerMode)) {
			modManagerMode = newModManagerMode;
		}
	}

	data = m_variables->getValue("Game Type", "Options");
	if(data != NULL) {
		GameTypes::GameType newGameType = GameTypes::parseFrom(data);
		if(GameTypes::isValid(newGameType)) {
			gameType = newGameType;
		}
	}

	data = m_variables->getValue("Server IP Address", "Options");
	if(data != NULL) {
		if(serverIPAddress != NULL) { delete [] serverIPAddress; }
		serverIPAddress = Utilities::copyString(data);
	}
	
	return true;
}

bool SettingsManager::saveTo(const QString & fileName) const {
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	return saveTo(fileNameBytes.data());
}

bool SettingsManager::saveTo(const char * fileName) const {
	m_variables->setValue("Mod List File Name", modListFileName, "Paths");
	m_variables->setValue("Favourites List File Name", favouritesListFileName, "Paths");
	m_variables->setValue("Mods Directory Name", modsDirectoryName, "Paths");
	m_variables->setValue("Mods Directory Path", modsDirectoryPath, "Paths");
	m_variables->setValue("Data Directory Name", dataDirectoryName, "Paths");
	m_variables->setValue("Game File Name", gameFileName, "Paths");
	m_variables->setValue("Setup File Name", setupFileName, "Paths");
	m_variables->setValue("Kextract File Name", kextractFileName, "Paths");
	m_variables->setValue("Game Path", gamePath, "Paths");
	m_variables->setValue("DOSBox File Name", DOSBoxFileName, "Paths");
	m_variables->setValue("DOSBox Path", DOSBoxPath, "Paths");
	m_variables->setValue("DOSBox Arguments", DOSBoxArgs, "Arguments");
	m_variables->setValue("DOSBox Game Script File Name", DOSBoxGameScriptFileName, "Scripts");
	m_variables->setValue("DOSBox Setup Script File Name", DOSBoxSetupScriptFileName, "Scripts");
	m_variables->setValue("DOSBox Client Script File Name", DOSBoxClientScriptFileName, "Scripts");
	m_variables->setValue("DOSBox Server Script File Name", DOSBoxServerScriptFileName, "Scripts");
	m_variables->setValue("Windows Game Script File Name", windowsGameScriptFileName, "Scripts");
	m_variables->setValue("Windows Setup Script File Name", windowsSetupScriptFileName, "Scripts");
	m_variables->setValue("Mod Manager Mode", ModManagerModes::toString(modManagerMode), "Options");
	m_variables->setValue("Game Type", GameTypes::toString(gameType), "Options");
	m_variables->setValue("Server IP Address", serverIPAddress, "Options");

	// group the variables by categories
	m_variables->sort();

	// update the settings file with the changes
	return m_variables->writeTo(fileName);
}
