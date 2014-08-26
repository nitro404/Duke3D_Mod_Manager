#ifndef MOD_COLLECTION_LISTENER
#define MOD_COLLECTION_LISTENER

class ModCollectionListener {
public:
	virtual ~ModCollectionListener() { }
	virtual void modCollectionUpdated() = 0;
	virtual void favouriteModCollectionUpdated() = 0;
};

#endif // MOD_COLLECTION_LISTENER
