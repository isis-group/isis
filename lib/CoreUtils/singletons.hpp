#ifndef SINGLETONS_HPP_INCLUDED
#define SINGLETONS_HPP_INCLUDED

#include <map>
#include <boost/noncopyable.hpp>

namespace isis{ namespace util{

/**
 * Static class to handle singletons.
 * This class can be used to create singletons with a specified priority.
 * It keeps track of them and deletes the automatically based of their priority.
 */
class Singletons{
	class SingletonBase:public boost::noncopyable{
	public:
		virtual ~SingletonBase(){};
	};
	template<typename BASE> class Singleton:public BASE,public SingletonBase{
	public:
		Singleton(){}
		template<typename T> Singleton(const T &init):BASE(init){}
	};
	
	typedef std::multimap<int,SingletonBase* > prioMap;
	prioMap map;
	Singletons(){};
	~Singletons();
	template<typename T,typename T2> Singleton<T>* create(int priority,const T2 &initValue){
		Singleton<T>* ret=new Singleton<T>(initValue);
		map.insert(map.find(priority), std::make_pair(priority,static_cast<SingletonBase*>(ret)));
		return ret;
	}
	template<typename T> Singleton<T>* create(int priority){
		Singleton<T>* ret=new Singleton<T>;
		map.insert(map.find(priority), std::make_pair(priority,static_cast<SingletonBase*>(ret)));
		return ret;
	}
	//@todo what happens to primitve static variables in static functions - will we get a dead reference here as well ?
	template<typename T> static Singleton<T>* request(int priority){
		static Singleton<T>* s=getMaster().create<T>(priority);
		return s;
	}
	template<typename T,typename T2> static Singleton<T>* request(int priority,const T2 &initValue){
		static Singleton<T>* s=getMaster().create<T>(priority,initValue);
		return s;
	}
	static Singletons& getMaster();
public:
	/**
	 * Get a singleton of type T and priority PRIO.
	 * If called the first time this creates a singleton of type T with the priority PRIO.
	 * In any other case it just returns the same object which was created at the first call.
	 * Singletons created using this are deleted based on the following rules:
	 * - singletons are deleted _after_ the programm ends
	 * - singletons not deleted before any singleton of a lower priority
	 * - singletons of the same priority are deleted in the opposite order they where created. (LIFO)
	 *
	 * \return allways a reference to the same object of type T.
	 */
	template<typename T,int PRIO> static T& get(){
		return *static_cast<T*>(request<T>(PRIO));
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