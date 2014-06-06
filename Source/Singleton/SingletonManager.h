#ifndef SINGLETON_MANAGER_H
#define SINGLETON_MANAGER_H

#include <QMap.h>

template <class T> class Singleton;

class SingletonManager {
protected:
	SingletonManager();
	SingletonManager(const SingletonManager & singletonManager);
	SingletonManager & operator = (const SingletonManager & singletonManager);
	~SingletonManager();

public:
	static void create();
	static void destroy();

	static void * getSingleton(const char * className);

	static bool registerSingleton(const char * className, void * instance);
	static bool unregisterSingleton(const char * className);

private:
	typedef QMap<const char *, void *> SingletonMap;

	static SingletonMap * m_singletons;
};

#endif // SINGLETON_MANAGER_H
