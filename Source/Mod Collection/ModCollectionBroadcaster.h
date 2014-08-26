#ifndef MOD_COLLECTION_BROADCASTER_H
#define MOD_COLLECTION_BROADCASTER_H

#include <QVector.h>
#include "Utilities/Utilities.h"
#include "Mod Collection/ModCollectionListener.h"

class ModCollectionBroadcaster {
public:
	ModCollectionBroadcaster();
	ModCollectionBroadcaster(const ModCollectionBroadcaster & m);
	ModCollectionBroadcaster & operator = (const ModCollectionBroadcaster & m);
	virtual ~ModCollectionBroadcaster();

	int numberOfListeners() const;
	bool hasListener(const ModCollectionListener * listener) const;
	int indexOfListener(const ModCollectionListener * listener) const;
	ModCollectionListener * getListener(int index) const;
	bool addListener(ModCollectionListener * listener);
	bool removeListener(int index);
	bool removeListener(const ModCollectionListener * listener);
	void clearListeners();
	
	bool operator == (const ModCollectionBroadcaster & m) const;
	bool operator != (const ModCollectionBroadcaster & m) const;

private:
	QVector<ModCollectionListener *> m_listeners;
};

#endif // MOD_COLLECTION_BROADCASTER_H
