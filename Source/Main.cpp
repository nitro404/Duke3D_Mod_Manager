#include "Utilities/Utilities.h"
#include "Mod Manager/ArgumentParser.h"
#include "Settings Manager/SettingsManager.h"
#include "Mod Manager/ModManager.h"
#include "Mod Collection/ModCollection.h"

#if _DEBUG
#include <vld.h>
#endif // _DEBUG

int main(int argc, char * argv[]) {
	ArgumentParser args(argc, argv);

	SingletonManager::create();

	SettingsManager::createInstance();
	ModManager::createInstance();

	SettingsManager::getInstance()->load(&args);
	ModManager::getInstance()->init(&args);

	ModManager::getInstance()->uninit();
	SettingsManager::getInstance()->save(&args);

	ModManager::destroyInstance();
	SettingsManager::destroyInstance();

	SingletonManager::destroy();

	Utilities::pause(); // for dramatic effect
	
	return 0;
}
