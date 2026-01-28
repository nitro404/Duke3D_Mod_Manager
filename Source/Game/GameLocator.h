#ifndef _GAME_LOCATOR_H_
#define _GAME_LOCATOR_H_

#include <Singleton/Singleton.h>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

class GameLocator : public Singleton<GameLocator> {
public:
	virtual ~GameLocator();

	bool locateGames();

	size_t numberOfGamePaths() const;
	bool hasGamePath(const std::string & gamePath) const;
	bool hasGamePathWithID(const std::string & gameVersionID) const;
	size_t indexOfGamePath(const std::string & gamePath) const;
	size_t indexOfGamePathWithID(const std::string & gameVersionID) const;
	const std::string * getGamePath(size_t index) const;
	const std::string * getGamePathWithID(const std::string & gameVersionID) const;
	const std::vector<std::pair<std::string, std::string>> & getGamePaths() const;
	bool removeGamePath(size_t index);
	bool removeGamePath(const std::string & gamePath);
	bool removeGamePathWithID(const std::string & gameVersionID);
	void clearGamePaths();

	virtual std::vector<std::pair<std::string, std::string>> getGameSearchPaths() = 0;

protected:
	GameLocator();

private:
	std::vector<std::pair<std::string, std::string>> m_gamePaths;

	GameLocator(const GameLocator &) = delete;
	const GameLocator & operator = (const GameLocator &) = delete;
};

#endif // _GAME_LOCATOR_H_
