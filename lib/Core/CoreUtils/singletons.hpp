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
 * - singletons are deleted in ascending order of int values
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
	class SingletonBase: public boost::noncopyable
	{
	public:
		virtual ~SingletonBase();
	};
	template<typename BASE> class Singleton: public SingletonBase, public BASE {};

	typedef std::multimap<int, SingletonBase *const> prioMap;
	prioMap map;
	Singletons();
	virtual ~Singletons();
	template<typename T> Singleton<T> *const create( int priority ) {
		Singleton<T> * const ret( new Singleton<T> );
		map.insert( map.find( priority ), std::make_pair( priority, ret ) );
		return ret;
	}
	template<typename T> static Singleton<T>& request( int priority ) {
		static Singleton<T> *s = getMaster().create<T>( priority );
		//ok this might become a dead ref as well. But its no complex object, and therefore won't be "destructed".
		return *s;
	}
	static Singletons &getMaster();
public:
	/**
	 * The first call creates a singleton of type T with the priority PRIO, 
	 * all repetetive calls return this object.
	 * \param T  type of the Singleton object
	 * \param PRIO the priority of the Singleton object (ascending order)
	 * \return a reference to the same object of type T.
	 */
	template<typename T, int PRIO> static Singleton<T> &get() {
		return request<T>( PRIO );
	}
};
}
}
#endif //SINGLETONS_HPP_INCLUDED
