#ifndef SINGLETONS_HPP_INCLUDED
#define SINGLETONS_HPP_INCLUDED

#include <map>
#include <boost/noncopyable.hpp>
#include <string>
#include <iostream>
#include <typeinfo>

namespace isis
{
namespace util
{
	
/**
 * Static class to handle singletons of a given type and priority.
 *
 * The special issues for these Singletons are: \n
 * 1) it's a template class - can be used for every type \n
 * 2) they have a priority used for destroying the Singletons AFTER the application ends:
 * - singletons are deleted in ascending order of int values (0 first, INT_MAX last)
 * - singletons of the same priority are deleted in the opposite order they where created (LIFO)
 * By this, one can count for dependencies of destroying objects, e.g. general objects as the log module
 * to be deleted latest.
 *
 * \code
 * Singletons::get < MyClass, INT_MAX - 1 >
 * \endcode
 * This generates a Singleton of MyClass with highest priority
 */
class Singletons
{
	template <typename C> class Singleton
	{
		static void destruct()
		{
			if(_instance)delete _instance;
			_instance = 0;
		}
		static C* _instance;
		Singleton () { }
	public:
		friend class Singletons;
	};
	
	typedef void(*destructer)();
	typedef std::multimap<int, destructer> prioMap;
	
	prioMap map;
	Singletons();
	virtual ~Singletons();
	static Singletons &getMaster();
public:
	/**
	 * The first call creates a singleton of type T with the priority PRIO (ascending order),
	 * all repetetive calls return this object.
	 * \return a reference to the same object of type T.
	 */
	template<typename T, int PRIO> static T &get() {
		if (!Singleton<T>::_instance){
			Singleton<T>::_instance = new T();
			prioMap &map = getMaster().map;
			map.insert( map.find( PRIO ), std::make_pair( PRIO, Singleton<T>::destruct ) );
		}
		return *Singleton<T>::_instance;
	}
};
template <typename C> C* Singletons::Singleton<C>::_instance = 0;

}
}
#endif //SINGLETONS_HPP_INCLUDED
