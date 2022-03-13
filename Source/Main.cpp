#include "Manager/ModManager.h"

int main(int argc, char * argv[]) {
	ModManager modManager;

	modManager.initialize(argc, argv);

	modManager.uninitialize();

	return 0;
}
