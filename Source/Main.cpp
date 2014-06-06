#include "Utilities/Utilities.h"
#include "Mod Manager/ArgumentParser.h"
#include "Settings Manager/SettingsManager.h"
#include "Mod Manager/ModManager.h"
#include "Mod Collection/ModCollection.h"

#if _DEBUG
#include <vld.h>
#endif // _DEBUG

int main(int argc, char * argv[]) {
	// -s "settings.ini"
	// -m [dosbox]/megaton/windows (mode)
	// -t [game]/setup/client/server (type)
	// -ip ip.address
	// -g "duke3d.grp"
	// -x "game.con"
	// -q "Evil" (query)
	// -r (random)
	//
	// NOTE: (r OR m OR x+g)

	ArgumentParser args(argc, argv);

	SingletonManager::create();

	SettingsManager::createInstance();
	ModManager::createInstance();

	SettingsManager::getInstance()->load(&args);
	if(!ModManager::getInstance()->init(true, &args)) { return 0; }

	ModManager::getInstance()->uninit();
	SettingsManager::getInstance()->save(&args);

	ModManager::destroyInstance();
	SettingsManager::destroyInstance();

	SingletonManager::destroy();

	Utilities::pause(); // for dramatic effect
	
	return 0;
}
