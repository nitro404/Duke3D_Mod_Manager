#ifndef _GAME_DOWNLOAD_COLLECTION_LISTENER_
#define _GAME_DOWNLOAD_COLLECTION_LISTENER_

class GameDownloadCollection;

class GameDownloadCollectionListener {
public:
	virtual ~GameDownloadCollectionListener();

	virtual void gameDownloadCollectionUpdated(GameDownloadCollection & gameDownloadCollection) = 0;
};

#endif // _GAME_DOWNLOAD_COLLECTION_LISTENER_
