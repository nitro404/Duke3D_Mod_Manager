#include "GameDownloadCollectionBroadcaster.h"

GameDownloadCollectionBroadcaster::GameDownloadCollectionBroadcaster() { }

GameDownloadCollectionBroadcaster::GameDownloadCollectionBroadcaster(GameDownloadCollectionBroadcaster && m) noexcept
	: m_listeners(std::move(m.m_listeners)) { }

GameDownloadCollectionBroadcaster::GameDownloadCollectionBroadcaster(const GameDownloadCollectionBroadcaster & m) {
	for(std::vector<GameDownloadCollectionListener *>::const_iterator i = m.m_listeners.begin(); i != m.m_listeners.end(); ++i) {
		m_listeners.push_back(*i);
	}
}

GameDownloadCollectionBroadcaster & GameDownloadCollectionBroadcaster::operator = (GameDownloadCollectionBroadcaster && m) noexcept {
	if(this != &m) {
		m_listeners = std::move(m.m_listeners);
	}

	return *this;
}

GameDownloadCollectionBroadcaster & GameDownloadCollectionBroadcaster::operator = (const GameDownloadCollectionBroadcaster & m) {
	m_listeners.clear();

	for(std::vector<GameDownloadCollectionListener *>::const_iterator i = m.m_listeners.begin(); i != m.m_listeners.end(); ++i) {
		m_listeners.push_back(*i);
	}

	return *this;
}

GameDownloadCollectionBroadcaster::~GameDownloadCollectionBroadcaster() { }

size_t GameDownloadCollectionBroadcaster::numberOfListeners() const {
	return m_listeners.size();
}

bool GameDownloadCollectionBroadcaster::hasListener(const GameDownloadCollectionListener & listener) const {
	for(std::vector<GameDownloadCollectionListener *>::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i) {
		if(*i == &listener) {
			return true;
		}
	}

	return false;
}

size_t GameDownloadCollectionBroadcaster::indexOfListener(const GameDownloadCollectionListener & listener) const {
	for(size_t i = 0; i < m_listeners.size(); i++) {
		if(m_listeners[i] == &listener) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

GameDownloadCollectionListener * GameDownloadCollectionBroadcaster::getListener(size_t index) const {
	if(index >= m_listeners.size()) {
		return nullptr;
	}

	return m_listeners[index];
}

bool GameDownloadCollectionBroadcaster::addListener(GameDownloadCollectionListener & listener) {
	if(!hasListener(listener)) {
		m_listeners.push_back(&listener);

		return true;
	}

	return false;
}

bool GameDownloadCollectionBroadcaster::removeListener(size_t index) {
	if(index >= m_listeners.size()) {
		return false;
	}

	m_listeners.erase(m_listeners.begin() + index);

	return true;
}

bool GameDownloadCollectionBroadcaster::removeListener(const GameDownloadCollectionListener & listener) {
	for(std::vector<GameDownloadCollectionListener *>::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i) {
		if(*i == &listener) {
			m_listeners.erase(i);

			return true;
		}
	}

	return false;
}

void GameDownloadCollectionBroadcaster::clearListeners() {
	m_listeners.clear();
}
