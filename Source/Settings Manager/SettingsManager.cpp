#include "Settings Manager/SettingsManager.h"

const char * SettingsManager::defaultSettingsFileName = "Duke Nukem 3D Mod Manager.ini";
const char * SettingsManager::defaultModListFileName = "Duke Nukem 3D Mod List.ini";
const char * SettingsManager::defaultFavouritesListFileName = "Duke Nukem 3D Favourite Mods.xml";
const char * SettingsManager::defaultModsDirectoryName = "Mods";
const char * SettingsManager::defaultDataDirectoryName = "Data";
const char * SettingsManager::defaultGameFileName = "DUKE3D.EXE";
const char * SettingsManager::defaultSetupFileName = "SETUP.EXE";
const char * SettingsManager::defaultKextractFileName = "KEXTRACT.EXE";
const char * SettingsManager::defaultDOSBoxPath = "./DOSBox/dosbox.exe";
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
	: modListFileName(NULL)
	, favouritesListFileName(NULL)
	, modsDirectoryName(NULL)
	, dataDirectoryName(NULL)
	, gameFileName(NULL)
	, setupFileName(NULL)
	, kextractFileName(NULL)
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
	m_variables = new VariableSystem();
	reset();
}

SettingsManager::~SettingsManager() {
	delete [] modListFileName;
	delete [] favouritesListFileName;
	delete [] modsDirectoryName;
	delete [] dataDirectoryName;
	delete [] gameFileName;
	delete [] setupFileName;
	delete [] kextractFileName;
	delete [] DOSBoxPath;
	delete [] DOSBoxArgs;
	delete [] DOSBoxGameScriptFileName;
	delete [] DOSBoxSetupScriptFileName;
	delete [] DOSBoxClientScriptFileName;
	delete [] DOSBoxServerScriptFileName;
	delete [] windowsGameScriptFileName;
	delete [] windowsSetupScriptFileName;
	delete [] serverIPAddress;

	delete m_variables;
}

void SettingsManager::reset() {
	if(modListFileName != NULL) { delete [] modListFileName; }
	modListFileName = new char[Utilities::stringLength(defaultModListFileName) + 1];
	Utilities::copyString(modListFileName, Utilities::stringLength(defaultModListFileName) + 1, defaultModListFileName);

	if(favouritesListFileName != NULL) { delete [] favouritesListFileName; }
	favouritesListFileName = new char[Utilities::stringLength(defaultFavouritesListFileName) + 1];
	Utilities::copyString(favouritesListFileName, Utilities::stringLength(defaultFavouritesListFileName) + 1, defaultFavouritesListFileName);

	if(modsDirectoryName != NULL) { delete [] modsDirectoryName; }
	modsDirectoryName = new char[Utilities::stringLength(defaultModsDirectoryName) + 1];
	Utilities::copyString(modsDirectoryName, Utilities::stringLength(defaultModsDirectoryName) + 1, defaultModsDirectoryName);
	
	if(dataDirectoryName != NULL) { delete [] dataDirectoryName; }
	dataDirectoryName = new char[Utilities::stringLength(defaultDataDirectoryName) + 1];
	Utilities::copyString(dataDirectoryName, Utilities::stringLength(defaultDataDirectoryName) + 1, defaultDataDirectoryName);

	if(gameFileName != NULL) { delete [] gameFileName; }
	gameFileName = new char[Utilities::stringLength(defaultGameFileName) + 1];
	Utilities::copyString(gameFileName, Utilities::stringLength(defaultGameFileName) + 1, defaultGameFileName);

	if(setupFileName != NULL) { delete [] setupFileName; }
	setupFileName = new char[Utilities::stringLength(defaultSetupFileName) + 1];
	Utilities::copyString(setupFileName, Utilities::stringLength(defaultSetupFileName) + 1, defaultSetupFileName);

	if(kextractFileName != NULL) { delete [] kextractFileName; }
	kextractFileName = new char[Utilities::stringLength(defaultKextractFileName) + 1];
	Utilities::copyString(kextractFileName, Utilities::stringLength(defaultKextractFileName) + 1, defaultKextractFileName);
	
	if(DOSBoxPath != NULL) { delete [] DOSBoxPath; }
	DOSBoxPath = new char[Utilities::stringLength(defaultDOSBoxPath) + 1];
	Utilities::copyString(DOSBoxPath, Utilities::stringLength(defaultDOSBoxPath) + 1, defaultDOSBoxPath);

	if(DOSBoxArgs != NULL) { delete [] DOSBoxArgs; }
	DOSBoxArgs = new char[Utilities::stringLength(defaultDOSBoxArgs) + 1];
	Utilities::copyString(DOSBoxArgs, Utilities::stringLength(defaultDOSBoxArgs) + 1, defaultDOSBoxArgs);
	
	if(DOSBoxGameScriptFileName != NULL) { delete [] DOSBoxGameScriptFileName; }
	DOSBoxGameScriptFileName = new char[Utilities::stringLength(defaultDOSBoxGameScriptFileName) + 1];
	Utilities::copyString(DOSBoxGameScriptFileName, Utilities::stringLength(defaultDOSBoxGameScriptFileName) + 1, defaultDOSBoxGameScriptFileName);
	
	if(DOSBoxSetupScriptFileName != NULL) { delete [] DOSBoxSetupScriptFileName; }
	DOSBoxSetupScriptFileName = new char[Utilities::stringLength(defaultDOSBoxSetupScriptFileName) + 1];
	Utilities::copyString(DOSBoxSetupScriptFileName, Utilities::stringLength(defaultDOSBoxSetupScriptFileName) + 1, defaultDOSBoxSetupScriptFileName);
	
	if(DOSBoxClientScriptFileName != NULL) { delete [] DOSBoxClientScriptFileName; }
	DOSBoxClientScriptFileName = new char[Utilities::stringLength(defaultDOSBoxClientScriptFileName) + 1];
	Utilities::copyString(DOSBoxClientScriptFileName, Utilities::stringLength(defaultDOSBoxClientScriptFileName) + 1, defaultDOSBoxClientScriptFileName);
	
	if(DOSBoxServerScriptFileName != NULL) { delete [] DOSBoxServerScriptFileName; }
	DOSBoxServerScriptFileName = new char[Utilities::stringLength(defaultDOSBoxServerScriptFileName) + 1];
	Utilities::copyString(DOSBoxServerScriptFileName, Utilities::stringLength(defaultDOSBoxServerScriptFileName) + 1, defaultDOSBoxServerScriptFileName);

	if(windowsGameScriptFileName != NULL) { delete [] windowsGameScriptFileName; }
	windowsGameScriptFileName = new char[Utilities::stringLength(defaultWindowsGameScriptFileName) + 1];
	Utilities::copyString(windowsGameScriptFileName, Utilities::stringLength(defaultWindowsGameScriptFileName) + 1, defaultWindowsGameScriptFileName);
	
	if(windowsSetupScriptFileName != NULL) { delete [] windowsSetupScriptFileName; }
	windowsSetupScriptFileName = new char[Utilities::stringLength(defaultWindowsSetupScriptFileName) + 1];
	Utilities::copyString(windowsSetupScriptFileName, Utilities::stringLength(defaultWindowsSetupScriptFileName) + 1, defaultWindowsSetupScriptFileName);

	modManagerMode = defaultModManagerMode;

	gameType = defaultGameType;

	if(serverIPAddress != NULL) { delete [] serverIPAddress; }
	serverIPAddress = new char[Utilities::stringLength(defaultServerIPAddress) + 1];
	Utilities::copyString(serverIPAddress, Utilities::stringLength(defaultServerIPAddress) + 1, defaultServerIPAddress);
}

bool SettingsManager::load(const ArgumentParser * args) {
	if(args != NULL) {
		QString altSettingsFileName = args->getValue("s");
		if(altSettingsFileName.length() > 0) {
			QByteArray fileNameBytes = altSettingsFileName.toLocal8Bit();
			const char * fileNameData = fileNameBytes.data();

			printf("Loading settings from alternate file: %s\n", fileNameData);

			bool loadedSettings = loadFrom(altSettingsFileName);
			if(!loadedSettings) {
				printf("Failed to load settings from alt settings file: %s\n", fileNameData);
			}
			return loadedSettings;
		}
	}

	return loadFrom(defaultSettingsFileName);
}

bool SettingsManager::save(const ArgumentParser * args) const {
	if(args != NULL) {
		QString altSettingsFileName = args->getValue("s");
		if(altSettingsFileName.length() > 0) {
			QByteArray fileNameBytes = altSettingsFileName.toLocal8Bit();
			const char * fileNameData = fileNameBytes.data();

			printf("Saving settings to alternate file: %s\n", fileNameData);

			bool savedSettings = saveTo(altSettingsFileName);
			if(!savedSettings) {
				printf("Failed to save settings to alternate file: %s\n", fileNameData);
			}
			return savedSettings;
		}
	}

	return saveTo(defaultSettingsFileName);
}

bool SettingsManager::loadFrom(const QString & fileName) {
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();

	return loadFrom(fileNameData);
}

bool SettingsManager::loadFrom(const char * fileName) {
	VariableSystem * newVariables = VariableSystem::readFrom(fileName);
	if(newVariables == NULL) { return false; }

	delete m_variables;
	m_variables = newVariables;

	const char * data = NULL;

	data = m_variables->getValue("Mod List File Name", "Paths");
	if(data != NULL) {
		if(modListFileName != NULL) { delete [] modListFileName; }
		modListFileName = new char[Utilities::stringLength(data) + 1];
		Utilities::copyString(modListFileName, Utilities::stringLength(data) + 1, data);
	}

	data = m_variables->getValue("Favourites List File Name", "Paths");
	if(data != NULL) {
		if(favouritesListFileName != NULL) { delete [] favouritesListFileName; }
		favouritesListFileName = new char[Utilities::stringLength(data) + 1];
		Utilities::copyString(favouritesListFileName, Utilities::stringLength(data) + 1, data);
	}

	data = m_variables->getValue("Mods Directory Name", "Paths");
	if(data != NULL) {
		if(modsDirectoryName != NULL) { delete [] modsDirectoryName; }
		modsDirectoryName = new char[Utilities::stringLength(data) + 1];
		Utilities::copyString(modsDirectoryName, Utilities::stringLength(data) + 1, data);
	}
	
	data = m_variables->getValue("Data Directory Name", "Paths");
	if(data != NULL) {
		if(dataDirectoryName != NULL) { delete [] dataDirectoryName; }
		dataDirectoryName = new char[Utilities::stringLength(data) + 1];
		Utilities::copyString(dataDirectoryName, Utilities::stringLength(data) + 1, data);
	}

	data = m_variables->getValue("Game File Name", "Paths");
	if(data != NULL) {
		if(gameFileName != NULL) { delete [] gameFileName; }
		gameFileName = new char[Utilities::stringLength(data) + 1];
		Utilities::copyString(gameFileName, Utilities::stringLength(data) + 1, data);
	}

	data = m_variables->getValue("Setup File Name", "Paths");
	if(data != NULL) {
		if(setupFileName != NULL) { delete [] setupFileName; }
		setupFileName = new char[Utilities::stringLength(data) + 1];
		Utilities::copyString(setupFileName, Utilities::stringLength(data) + 1, data);
	}

	data = m_variables->getValue("Kextract File Name", "Paths");
	if(data != NULL) {
		if(kextractFileName != NULL) { delete [] kextractFileName; }
		kextractFileName = new char[Utilities::stringLength(data) + 1];
		Utilities::copyString(kextractFileName, Utilities::stringLength(data) + 1, data);
	}
	
	data = m_variables->getValue("DOSBox Path", "Paths");
	if(data != NULL) {
		if(DOSBoxPath != NULL) { delete [] DOSBoxPath; }
		DOSBoxPath = new char[Utilities::stringLength(data) + 1];
		Utilities::copyString(DOSBoxPath, Utilities::stringLength(data) + 1, data);
	}

	data = m_variables->getValue("DOSBox Arguments", "Arguments");
	if(data != NULL) {
		if(DOSBoxArgs != NULL) { delete [] DOSBoxArgs; }
		DOSBoxArgs = new char[Utilities::stringLength(data) + 1];
		Utilities::copyString(DOSBoxArgs, Utilities::stringLength(data) + 1, data);
	}
	
	data = m_variables->getValue("DOSBox Game Script File Name", "Scripts");
	if(data != NULL) {
		if(DOSBoxGameScriptFileName != NULL) { delete [] DOSBoxGameScriptFileName; }
		DOSBoxGameScriptFileName = new char[Utilities::stringLength(data) + 1];
		Utilities::copyString(DOSBoxGameScriptFileName, Utilities::stringLength(data) + 1, data);
	}
	
	data = m_variables->getValue("DOSBox Setup Script File Name", "Scripts");
	if(data != NULL) {
		if(DOSBoxSetupScriptFileName != NULL) { delete [] DOSBoxSetupScriptFileName; }
		DOSBoxSetupScriptFileName = new char[Utilities::stringLength(data) + 1];
		Utilities::copyString(DOSBoxSetupScriptFileName, Utilities::stringLength(data) + 1, data);
	}
	
	data = m_variables->getValue("DOSBox Client Script File Name", "Scripts");
	if(data != NULL) {
		if(DOSBoxClientScriptFileName != NULL) { delete [] DOSBoxClientScriptFileName; }
		DOSBoxClientScriptFileName = new char[Utilities::stringLength(data) + 1];
		Utilities::copyString(DOSBoxClientScriptFileName, Utilities::stringLength(data) + 1, data);
	}
	
	data = m_variables->getValue("DOSBox Server Script File Name", "Scripts");
	if(data != NULL) {
		if(DOSBoxServerScriptFileName != NULL) { delete [] DOSBoxServerScriptFileName; }
		DOSBoxServerScriptFileName = new char[Utilities::stringLength(data) + 1];
		Utilities::copyString(DOSBoxServerScriptFileName, Utilities::stringLength(data) + 1, data);
	}

	data = m_variables->getValue("Windows Game Script File Name", "Scripts");
	if(data != NULL) {
		if(windowsGameScriptFileName != NULL) { delete [] windowsGameScriptFileName; }
		windowsGameScriptFileName = new char[Utilities::stringLength(data) + 1];
		Utilities::copyString(windowsGameScriptFileName, Utilities::stringLength(data) + 1, data);
	}
	
	data = m_variables->getValue("Windows Setup Script File Name", "Scripts");
	if(data != NULL) {
		if(windowsSetupScriptFileName != NULL) { delete [] windowsSetupScriptFileName; }
		windowsSetupScriptFileName = new char[Utilities::stringLength(data) + 1];
		Utilities::copyString(windowsSetupScriptFileName, Utilities::stringLength(data) + 1, data);
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
		serverIPAddress = new char[Utilities::stringLength(data) + 1];
		Utilities::copyString(serverIPAddress, Utilities::stringLength(data) + 1, data);
	}
	
	return true;
}

bool SettingsManager::saveTo(const QString & fileName) const {
	QByteArray fileNameBytes = fileName.toLocal8Bit();
	const char * fileNameData = fileNameBytes.data();
	
	return saveTo(fileNameData);
}

bool SettingsManager::saveTo(const char * fileName) const {
	m_variables->setValue("Mod List File Name", modListFileName, "Paths");
	m_variables->setValue("Favourites List File Name", favouritesListFileName, "Paths");
	m_variables->setValue("Mods Directory Name", modsDirectoryName, "Paths");
	m_variables->setValue("Data Directory Name", dataDirectoryName, "Paths");
	m_variables->setValue("Game File Name", gameFileName, "Paths");
	m_variables->setValue("Setup File Name", setupFileName, "Paths");
	m_variables->setValue("Kextract File Name", kextractFileName, "Paths");
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
