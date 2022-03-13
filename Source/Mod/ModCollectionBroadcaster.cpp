#include "ModCollectionBroadcaster.h"

ModCollectionBroadcaster::ModCollectionBroadcaster() { }

ModCollectionBroadcaster::ModCollectionBroadcaster(ModCollectionBroadcaster && m) noexcept
	: m_listeners(std::move(m.m_listeners)) { }

ModCollectionBroadcaster::ModCollectionBroadcaster(const ModCollectionBroadcaster & m) {
	for(std::vector<ModCollectionListener *>::const_iterator i = m.m_listeners.begin(); i != m.m_listeners.end(); ++i) {
		m_listeners.push_back(*i);
	}
}

ModCollectionBroadcaster & ModCollectionBroadcaster::operator = (ModCollectionBroadcaster && m) noexcept {
	if(this != &m) {
		m_listeners = std::move(m.m_listeners);
	}

	return *this;
}

ModCollectionBroadcaster & ModCollectionBroadcaster::operator = (const ModCollectionBroadcaster & m) {
	m_listeners.clear();

	for(std::vector<ModCollectionListener *>::const_iterator i = m.m_listeners.begin(); i != m.m_listeners.end(); ++i) {
		m_listeners.push_back(*i);
	}

	return *this;
}

ModCollectionBroadcaster::~ModCollectionBroadcaster() { }

size_t ModCollectionBroadcaster::numberOfListeners() const {
	return m_listeners.size();
}

bool ModCollectionBroadcaster::hasListener(const ModCollectionListener & listener) const {
	for(std::vector<ModCollectionListener *>::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i) {
		if(*i == &listener) {
			return true;
		}
	}

	return false;
}

size_t ModCollectionBroadcaster::indexOfListener(const ModCollectionListener & listener) const {
	for(size_t i = 0; i < m_listeners.size(); i++) {
		if(m_listeners[i] == &listener) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

ModCollectionListener * ModCollectionBroadcaster::getListener(size_t index) const {
	if(index >= m_listeners.size()) {
		return nullptr;
	}

	return m_listeners[index];
}

bool ModCollectionBroadcaster::addListener(ModCollectionListener & listener) {
	if(!hasListener(listener)) {
		m_listeners.push_back(&listener);

		return true;
	}

	return false;
}

bool ModCollectionBroadcaster::removeListener(size_t index) {
	if(index >= m_listeners.size()) {
		return false;
	}

	m_listeners.erase(m_listeners.begin() + index);

	return true;
}

bool ModCollectionBroadcaster::removeListener(const ModCollectionListener & listener) {
	for(std::vector<ModCollectionListener *>::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i) {
		if(*i == &listener) {
			m_listeners.erase(i);

			return true;
		}
	}

	return false;
}

void ModCollectionBroadcaster::clearListeners() {
	m_listeners.clear();
}
