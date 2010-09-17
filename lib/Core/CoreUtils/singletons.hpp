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
 * Static class to handle singletons.
 * This class can be used to create singletons with a specified priority.
 * It keeps track of them and deletes them automatically based of their priority.
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
	 * Get a singleton of type T and priority PRIO.
	 * If called the first time this creates a singleton of type T with the priority PRIO.
	 * In any other case it just returns the same object which was created at the first call.
	 * Singletons created by this function are automatically deleted based on the following rules:
	 * - singletons are deleted _after_ the program ends
	 * - singletons are not deleted before any singleton of a lower priority
	 * - singletons of the same priority are deleted in the opposite order they where created. (LIFO)
	 *
	 * \return allways a reference to the same object of type T.
	 */
	template<typename T, int PRIO> static Singleton<T> &get() {
		return request<T>( PRIO );
	}
};
}
}
#endif //SINGLETONS_HPP_INCLUDED
