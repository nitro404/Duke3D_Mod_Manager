#ifndef _GAME_VERSION_COLLECTION_LISTENER_
#define _GAME_VERSION_COLLECTION_LISTENER_

class GameVersion;
class GameVersionCollection;

class GameVersionCollectionListener {
public:
	virtual ~GameVersionCollectionListener();

	virtual void gameVersionCollectionSizeChanged(GameVersionCollection & gameVersionCollection) = 0;
	virtual void gameVersionCollectionItemModified(GameVersionCollection & gameVersionCollection, GameVersion & gameVersion) = 0;
};

#endif // _GAME_VERSION_COLLECTION_LISTENER_
