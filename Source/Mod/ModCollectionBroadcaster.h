#ifndef _MOD_COLLECTION_BROADCASTER_H_
#define _MOD_COLLECTION_BROADCASTER_H_

#include <cstdint>
#include <memory>
#include <vector>

class ModCollectionListener;

class ModCollectionBroadcaster {
public:
	ModCollectionBroadcaster();
	ModCollectionBroadcaster(ModCollectionBroadcaster && m) noexcept;
	ModCollectionBroadcaster(const ModCollectionBroadcaster & m);
	ModCollectionBroadcaster & operator = (ModCollectionBroadcaster && m) noexcept;
	ModCollectionBroadcaster & operator = (const ModCollectionBroadcaster & m);
	virtual ~ModCollectionBroadcaster();

	size_t numberOfListeners() const;
	bool hasListener(const ModCollectionListener & listener) const;
	size_t indexOfListener(const ModCollectionListener & listener) const;
	ModCollectionListener * getListener(size_t index) const;
	bool addListener(ModCollectionListener & listener);
	bool removeListener(size_t index);
	bool removeListener(const ModCollectionListener & listener);
	void clearListeners();

private:
	std::vector<ModCollectionListener *> m_listeners;
};

#endif // _MOD_COLLECTION_BROADCASTER_H_
