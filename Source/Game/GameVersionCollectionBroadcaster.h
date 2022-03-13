#ifndef _GAME_VERSION_COLLECTION_BROADCASTER_H_
#define _GAME_VERSION_COLLECTION_BROADCASTER_H_

#include <cstdint>
#include <memory>
#include <vector>

class GameVersionCollectionListener;

class GameVersionCollectionBroadcaster {
public:
	GameVersionCollectionBroadcaster();
	GameVersionCollectionBroadcaster(GameVersionCollectionBroadcaster && m) noexcept;
	GameVersionCollectionBroadcaster(const GameVersionCollectionBroadcaster & m);
	GameVersionCollectionBroadcaster & operator = (GameVersionCollectionBroadcaster && m) noexcept;
	GameVersionCollectionBroadcaster & operator = (const GameVersionCollectionBroadcaster & m);
	virtual ~GameVersionCollectionBroadcaster();

	size_t numberOfListeners() const;
	bool hasListener(const GameVersionCollectionListener & listener) const;
	size_t indexOfListener(const GameVersionCollectionListener & listener) const;
	GameVersionCollectionListener * getListener(size_t index) const;
	bool addListener(GameVersionCollectionListener & listener);
	bool removeListener(size_t index);
	bool removeListener(const GameVersionCollectionListener & listener);
	void clearListeners();

private:
	std::vector<GameVersionCollectionListener *> m_listeners;
};

#endif // _GAME_VERSION_COLLECTION_BROADCASTER_H_
