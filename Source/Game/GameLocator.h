#ifndef _GAME_LOCATOR_H_
#define _GAME_LOCATOR_H_

#include <Singleton/Singleton.h>

#include <cstdint>
#include <string>
#include <vector>

class GameLocator : public Singleton<GameLocator> {
public:
	virtual ~GameLocator();

	bool locateGames();

	size_t numberOfGamePaths() const;
	bool hasGamePath(const std::string & gamePath) const;
	size_t indexOfGamePath(const std::string & gamePath) const;
	const std::string * getGamePath(size_t index) const;
	const std::vector<std::string> & getGamePaths();
	bool removeGamePath(size_t index);
	bool removeGamePath(const std::string & gamePath);
	void clearGamePaths();

	virtual std::vector<std::string> getGameSearchPaths() = 0;

protected:
	GameLocator();

private:
	std::vector<std::string> m_gamePaths;

	GameLocator(const GameLocator &) = delete;
	GameLocator(GameLocator &&) noexcept = delete;
	const GameLocator & operator = (const GameLocator &) = delete;
	const GameLocator & operator = (GameLocator &&) noexcept = delete;
};

#endif // _GAME_LOCATOR_H_
