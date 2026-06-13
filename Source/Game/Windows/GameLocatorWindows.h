#ifndef _GAME_LOCATOR_WINDOWS_H_
#define _GAME_LOCATOR_WINDOWS_H_

#include "Game/GameLocator.h"

class ModManager;

class GameLocatorWindows final : public GameLocator {
	friend class ModManager;

public:
	~GameLocatorWindows() override;

	// GameLocator Virtuals
	virtual std::vector<std::pair<std::string, std::string>> getGameSearchPaths() override;

private:
	GameLocatorWindows();

	GameLocatorWindows(const GameLocatorWindows &) = delete;
	const GameLocatorWindows & operator = (const GameLocatorWindows &) = delete;
};

#endif // _GAME_LOCATOR_WINDOWS_H_
