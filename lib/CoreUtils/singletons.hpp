#ifndef SINGLETONS_HPP_INCLUDED
#define SINGLETONS_HPP_INCLUDED

#include <map>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <string>
#include <iostream>
#include <typeinfo>

namespace isis{ namespace util{

/**
 * Static class to handle singletons.
 * This class can be used to create singletons with a specified priority.
 * It keeps track of them and deletes them automatically based of their priority.
 */
class Singletons{
	class SingletonBase:public boost::noncopyable{};
	template<typename BASE> class Singleton:public SingletonBase,public BASE{};
	
	typedef std::multimap<int,boost::shared_ptr<SingletonBase> > prioMap;
	prioMap map;
	Singletons();
	~Singletons();
	template<typename T> boost::shared_ptr<Singleton<T> > create(int priority){
		const boost::shared_ptr<Singleton<T> > ret(new Singleton<T>);
		map.insert(map.find(priority), std::make_pair(priority,boost::static_pointer_cast<SingletonBase>(ret)));
		return ret;
	}
	template<typename T> static boost::weak_ptr<Singleton<T> > request(int priority){
		static boost::weak_ptr<Singleton<T> > s=getMaster().create<T>(priority);
		return s;
	}
	static Singletons& getMaster();
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
	 * \return allways a shared pointer to the same object of type T.
	 */
	template<typename T,int PRIO> static boost::shared_ptr<Singleton<T> > get(){
		return request<T>(PRIO).lock();
	}
	/**
	 * \copydoc get()
	 * \param initValue value to be forwarded to the constructor of T if its called.
	 */
	template<typename T,int PRIO,typename T2> static T& get(const T2 &initValue){
		return *static_cast<T*>(request<T>(PRIO,initValue));
	}
};
}}
#endif //SINGLETONS_HPP_INCLUDED
