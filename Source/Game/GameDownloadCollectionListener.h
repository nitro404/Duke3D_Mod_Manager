#ifndef _GAME_DOWNLOAD_COLLECTION_LISTENER_
#define _GAME_DOWNLOAD_COLLECTION_LISTENER_

class GameDownloadCollectionListener {
public:
	virtual ~GameDownloadCollectionListener();

	virtual void gameDownloadCollectionUpdated() = 0;
};

#endif // _GAME_DOWNLOAD_COLLECTION_LISTENER_
