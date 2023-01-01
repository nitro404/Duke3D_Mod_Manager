#include "Manager/ModManager.h"
#include "CLI/ModManagerCLI.h"

int main(int argc, char * argv[]) {
	ModManager modManager;

	if(modManager.initialize(argc, argv) && modManager.isInitialized()) {
		ModManagerCLI cli(&modManager);

		cli.runMenu();
	}

	modManager.uninitialize();

	return 0;
}
