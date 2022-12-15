#ifndef _GAME_DOWNLOAD_COLLECTION_BROADCASTER_H_
#define _GAME_DOWNLOAD_COLLECTION_BROADCASTER_H_

#include <cstdint>
#include <memory>
#include <vector>

class GameDownloadCollectionListener;

class GameDownloadCollectionBroadcaster {
public:
	GameDownloadCollectionBroadcaster();
	GameDownloadCollectionBroadcaster(GameDownloadCollectionBroadcaster && m) noexcept;
	GameDownloadCollectionBroadcaster(const GameDownloadCollectionBroadcaster & m);
	GameDownloadCollectionBroadcaster & operator = (GameDownloadCollectionBroadcaster && m) noexcept;
	GameDownloadCollectionBroadcaster & operator = (const GameDownloadCollectionBroadcaster & m);
	virtual ~GameDownloadCollectionBroadcaster();

	size_t numberOfListeners() const;
	bool hasListener(const GameDownloadCollectionListener & listener) const;
	size_t indexOfListener(const GameDownloadCollectionListener & listener) const;
	GameDownloadCollectionListener * getListener(size_t index) const;
	bool addListener(GameDownloadCollectionListener & listener);
	bool removeListener(size_t index);
	bool removeListener(const GameDownloadCollectionListener & listener);
	void clearListeners();

private:
	std::vector<GameDownloadCollectionListener *> m_listeners;
};

#endif // _GAME_DOWNLOAD_COLLECTION_BROADCASTER_H_
