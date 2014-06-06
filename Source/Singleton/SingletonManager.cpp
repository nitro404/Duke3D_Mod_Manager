#include "Singleton/SingletonManager.h"

SingletonManager::SingletonMap * SingletonManager::m_singletons = NULL;

void SingletonManager::create() {
	if(m_singletons != NULL) { return; }

	m_singletons = new SingletonMap();
}

void SingletonManager::destroy() {
	if(m_singletons != NULL) {
		delete m_singletons;
		m_singletons = NULL;
	}
}

void * SingletonManager::getSingleton(const char * className) {
	if(m_singletons == NULL) { return NULL; }

	SingletonMap::const_iterator i = m_singletons->find(className);

	if(i == m_singletons->end()) { return NULL; }

	return i.value();
}

bool SingletonManager::registerSingleton(const char * className, void * instance) {
	if(m_singletons == NULL || m_singletons->find(className) != m_singletons->end()) { return false; }

	(*m_singletons)[className] = instance;

	return true;
}

bool SingletonManager::unregisterSingleton(const char * className) {
	if(m_singletons == NULL) { return false; }

	SingletonMap::iterator i = m_singletons->find(className);

	if(i == m_singletons->end()) { return false; }

	m_singletons->erase(i);

	return true;
}
