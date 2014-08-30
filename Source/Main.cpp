#include "Utilities/Utilities.h"
#include "Mod Manager/ArgumentParser.h"
#include "Settings Manager/SettingsManager.h"
#include "Mod Manager/ModManager.h"
#include "Mod Collection/ModCollection.h"

#if _DEBUG
#include <vld.h>
#endif // _DEBUG

int main(int argc, char * argv[]) {
	ModManager modManager;

	modManager.init(argc, argv);

	modManager.uninit();
	
	return 0;
}
