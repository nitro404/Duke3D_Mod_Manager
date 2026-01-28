#ifndef _GAME_LOCATOR_WINDOWS_H_
#define _GAME_LOCATOR_WINDOWS_H_

#include "Game/GameLocator.h"

class GameLocatorWindows final : public GameLocator {
public:
	GameLocatorWindows();
	virtual ~GameLocatorWindows();

	// GameLocator Virtuals
	virtual std::vector<std::pair<std::string, std::string>> getGameSearchPaths() override;

private:
	GameLocatorWindows(const GameLocatorWindows &) = delete;
	const GameLocatorWindows & operator = (const GameLocatorWindows &) = delete;
};

#endif // _GAME_LOCATOR_WINDOWS_H_
