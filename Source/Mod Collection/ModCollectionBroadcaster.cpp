#include "Mod Collection/ModCollectionBroadcaster.h"

ModCollectionBroadcaster::ModCollectionBroadcaster() {

}

ModCollectionBroadcaster::ModCollectionBroadcaster(const ModCollectionBroadcaster & m) {
	for(int i=0;i<m.m_listeners.size();i++) {
		m_listeners.push_back(m.m_listeners[i]);
	}
}

ModCollectionBroadcaster & ModCollectionBroadcaster::operator = (const ModCollectionBroadcaster & m) {
	m_listeners.clear();
	for(int i=0;i<m.m_listeners.size();i++) {
		m_listeners.push_back(m.m_listeners[i]);
	}

	return *this;
}

ModCollectionBroadcaster::~ModCollectionBroadcaster() {

}

int ModCollectionBroadcaster::numberOfListeners() const {
	return m_listeners.size();
}

bool ModCollectionBroadcaster::hasListener(const ModCollectionListener * listener) const {
	if(listener == NULL) {
		return false;
	}

	for(int i=0;i<m_listeners.size();i++) {
		if(m_listeners[i] == listener) {
			return true;
		}
	}
	return false;
}

int ModCollectionBroadcaster::indexOfListener(const ModCollectionListener * listener) const {
	if(listener == NULL) {
		return -1;
	}

	for(int i=0;i<m_listeners.size();i++) {
		if(m_listeners[i] == listener) {
			return i;
		}
	}
	return -1;
}

ModCollectionListener * ModCollectionBroadcaster::getListener(int index) const {
	if(index < 0 || index >= m_listeners.size()) {
		return NULL;
	}

	return m_listeners[index];
}

bool ModCollectionBroadcaster::addListener(ModCollectionListener * listener) {
	if(listener == NULL) {
		return false;
	}

	if(!hasListener(listener)) {
		m_listeners.push_back(listener);
		return true;
	}
	return false;
}

bool ModCollectionBroadcaster::removeListener(int index) {
	if(index < 0 || index >= m_listeners.size()) {
		return false;
	}

	m_listeners.remove(index);

	return true;
}

bool ModCollectionBroadcaster::removeListener(const ModCollectionListener * listener) {
	if(listener == NULL) {
		return false;
	}

	for(int i=0;i<m_listeners.size();i++) {
		if(m_listeners[i] == listener) {
			m_listeners.remove(i);
			return true;
		}
	}

	return false;
}

void ModCollectionBroadcaster::clearListeners() {
	m_listeners.clear();
}

bool ModCollectionBroadcaster::operator == (const ModCollectionBroadcaster & m) const {
	if(m_listeners.size() != m.m_listeners.size()) { return false; }
	
	for(int i=0;i<m.m_listeners.size();i++) {
		if(!hasListener(m.m_listeners[i])) {
			return false;
		}
	}
	return true;
}

bool ModCollectionBroadcaster::operator != (const ModCollectionBroadcaster & m) const {
	return !operator == (m);
}
