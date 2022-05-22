#include "Manager/ModManager.h"

#include "Game/Windows/GameLocatorWindows.h"

#include <Factory/FactoryRegistry.h>

void ModManager::assignPlatformFactories() {
	FactoryRegistry & factoryRegistry = FactoryRegistry::getInstance();

	factoryRegistry.setFactory<GameLocator>([]() {
		return std::make_unique<GameLocatorWindows>();
	});
}
