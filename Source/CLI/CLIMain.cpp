#include "Manager/ModManager.h"
#include "CLI/ModManagerCLI.h"

#include <Application/ComponentRegistry.h>

int main(int argc, char * argv[]) {
	ComponentRegistry::getInstance().registerGlobalComponents();

	ModManager modManager;

	if(modManager.initialize(argc, argv) && modManager.isInitialized()) {
		ModManagerCLI cli(&modManager);

		cli.runMenu();
	}

	modManager.uninitialize();

	ComponentRegistry::getInstance().deleteAllGlobalComponents();

	return 0;
}
