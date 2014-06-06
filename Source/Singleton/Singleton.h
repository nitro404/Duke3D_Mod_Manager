#ifndef SINGLETON_H
#define SINGLETON_H

#include <typeinfo>
#include "Singleton/SingletonManager.h"

template <class T>
class Singleton {
protected:
	Singleton();
	virtual ~Singleton();

public:
	static void createInstance();
	static void destroyInstance();
	static T * getInstance();
};

template <class T>
inline Singleton<T>::Singleton() {
	
}

template <class T>
inline Singleton<T>::~Singleton() {
	
}

template <class T>
inline void Singleton<T>::createInstance() {
	const char * className = typeid(T).name();
	if(SingletonManager::getSingleton(className) != NULL) { return; }
	SingletonManager::registerSingleton(className, new T());
}

template <class T>
inline void Singleton<T>::destroyInstance() {
	const char * className = typeid(T).name();
	T * instance = reinterpret_cast<T *>(SingletonManager::getSingleton(className));
	if(instance != NULL) { delete instance; }
	SingletonManager::unregisterSingleton(className);
}

template <class T>
inline T * Singleton<T>::getInstance() {
	const char * className = typeid(T).name();
	static T * instance = reinterpret_cast<T *>(SingletonManager::getSingleton(className));
	return instance;
}

#endif // SINGLETON_H
