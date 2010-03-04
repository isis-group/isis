#ifndef SINGLETONS_HPP_INCLUDED
#define SINGLETONS_HPP_INCLUDED

#include <map>
#include <boost/noncopyable.hpp>

namespace isis{ namespace util{

class Singletons;


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
	template<typename T,int PRIO> static T& get(){
		return *static_cast<T*>(request<T>(PRIO));
	}
	template<typename T,int PRIO,typename T2> static T& get(const T2 &initValue){
		return *static_cast<T*>(request<T>(PRIO,initValue));
	}
};
}}
#endif //SINGLETONS_HPP_INCLUDED