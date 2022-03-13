#ifndef _GAME_VERSION_COLLECTION_LISTENER_
#define _GAME_VERSION_COLLECTION_LISTENER_

class GameVersionCollectionListener {
public:
	virtual ~GameVersionCollectionListener();

	virtual void gameVersionCollectionUpdated() = 0;
};

#endif // _GAME_VERSION_COLLECTION_LISTENER_
