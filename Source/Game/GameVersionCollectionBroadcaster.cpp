#include "GameVersionCollectionBroadcaster.h"

GameVersionCollectionBroadcaster::GameVersionCollectionBroadcaster() { }

GameVersionCollectionBroadcaster::GameVersionCollectionBroadcaster(GameVersionCollectionBroadcaster && m) noexcept
	: m_listeners(std::move(m.m_listeners)) { }

GameVersionCollectionBroadcaster::GameVersionCollectionBroadcaster(const GameVersionCollectionBroadcaster & m) {
	for(std::vector<GameVersionCollectionListener *>::const_iterator i = m.m_listeners.begin(); i != m.m_listeners.end(); ++i) {
		m_listeners.push_back(*i);
	}
}

GameVersionCollectionBroadcaster & GameVersionCollectionBroadcaster::operator = (GameVersionCollectionBroadcaster && m) noexcept {
	if(this != &m) {
		m_listeners = std::move(m.m_listeners);
	}

	return *this;
}

GameVersionCollectionBroadcaster & GameVersionCollectionBroadcaster::operator = (const GameVersionCollectionBroadcaster & m) {
	m_listeners.clear();

	for(std::vector<GameVersionCollectionListener *>::const_iterator i = m.m_listeners.begin(); i != m.m_listeners.end(); ++i) {
		m_listeners.push_back(*i);
	}

	return *this;
}

GameVersionCollectionBroadcaster::~GameVersionCollectionBroadcaster() { }

size_t GameVersionCollectionBroadcaster::numberOfListeners() const {
	return m_listeners.size();
}

bool GameVersionCollectionBroadcaster::hasListener(const GameVersionCollectionListener & listener) const {
	for(std::vector<GameVersionCollectionListener *>::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i) {
		if(*i == &listener) {
			return true;
		}
	}

	return false;
}

size_t GameVersionCollectionBroadcaster::indexOfListener(const GameVersionCollectionListener & listener) const {
	for(size_t i = 0; i < m_listeners.size(); i++) {
		if(m_listeners[i] == &listener) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

GameVersionCollectionListener * GameVersionCollectionBroadcaster::getListener(size_t index) const {
	if(index >= m_listeners.size()) {
		return nullptr;
	}

	return m_listeners[index];
}

bool GameVersionCollectionBroadcaster::addListener(GameVersionCollectionListener & listener) {
	if(!hasListener(listener)) {
		m_listeners.push_back(&listener);

		return true;
	}

	return false;
}

bool GameVersionCollectionBroadcaster::removeListener(size_t index) {
	if(index >= m_listeners.size()) {
		return false;
	}

	m_listeners.erase(m_listeners.begin() + index);

	return true;
}

bool GameVersionCollectionBroadcaster::removeListener(const GameVersionCollectionListener & listener) {
	for(std::vector<GameVersionCollectionListener *>::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i) {
		if(*i == &listener) {
			m_listeners.erase(i);

			return true;
		}
	}

	return false;
}

void GameVersionCollectionBroadcaster::clearListeners() {
	m_listeners.clear();
}
