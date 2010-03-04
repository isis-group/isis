#ifndef SINGLETONS_HPP_INCLUDED
#define SINGLETONS_HPP_INCLUDED

#include <map>
#include <boost/noncopyable.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/mpl/assert.hpp>

namespace isis{ namespace util{

namespace _internal{
class Singleton:public boost::noncopyable{
protected:
	Singleton(){}
public:
	virtual ~Singleton(){};
};
}

class Singletons{
	typedef std::multimap<int,_internal::Singleton* > prioMap;
	prioMap map;
	Singletons(){};
	~Singletons();
	template<typename T> T* create(int priority){
		BOOST_MPL_ASSERT((boost::is_base_of< _internal::Singleton, T>));
		T* ret=new T;
		map.insert(map.find(priority), std::make_pair(priority,static_cast<_internal::Singleton*>(ret)));
		return ret;
	}
	template<typename T> static T* request(int priority){
		//@todo what happens to primitve static variables in static functions - will we get a dead reference here as well ?
		static T* s=getMaster().create<T>(priority);
		return s;
	}
	static Singletons& getMaster();
public:
	template<typename T,int PRIO> static T& get(){
		return *request<T>(PRIO);
	}
};
}}
#endif //SINGLETONS_HPP_INCLUDED