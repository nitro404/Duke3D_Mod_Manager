#ifndef _GAME_LOCATOR_WINDOWS_H_
#define _GAME_LOCATOR_WINDOWS_H_

#include "Game/GameLocator.h"

class GameLocatorWindows final : public GameLocator {
public:
	GameLocatorWindows();
	virtual ~GameLocatorWindows();

	// GameLocator Virtuals
	virtual std::vector<std::string> getGameSearchPaths() override;

private:
	GameLocatorWindows(const GameLocatorWindows &) = delete;
	GameLocatorWindows(GameLocatorWindows &&) noexcept = delete;
	const GameLocatorWindows & operator = (const GameLocatorWindows &) = delete;
	const GameLocatorWindows & operator = (GameLocatorWindows &&) noexcept = delete;
};

#endif // _GAME_LOCATOR_WINDOWS_H_
