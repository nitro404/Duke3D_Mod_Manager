#ifndef _MOD_COLLECTION_LISTENER_
#define _MOD_COLLECTION_LISTENER_

class ModCollectionListener {
public:
	virtual ~ModCollectionListener();

	virtual void modCollectionUpdated() = 0;
	virtual void favouriteModCollectionUpdated() = 0;
};

#endif // _MOD_COLLECTION_LISTENER_
