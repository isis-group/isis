#ifndef ISISTYPE_HPP
#define ISISTYPE_HPP

#include <string>
#include <boost/lexical_cast.hpp>
#include <map>
#include "log.hpp"

namespace iUtil{

template<typename TYPE> class Type;

namespace _internal{
template<typename TYPE,typename T> TYPE __cast_to(Type<TYPE> *dest,const T& value){
	return boost::lexical_cast<TYPE>(value);
}
template<typename TYPE> TYPE __cast_to(Type<TYPE> *dest,const TYPE& value){
	return value;
}

class TypeBase{
protected:
	std::map<unsigned short,TypeBase*> _references;
public:
	template<typename T> bool is(){return is(typeid(T));}
	virtual bool is(const std::type_info & t)const = 0;
	virtual std::string toString(bool labeled=false)const=0;
	virtual std::string typeName()const=0;
	virtual unsigned short typeID()const=0;

	template<class T> T as(){
		Type<T> ret(this->toString());
		return (T)ret;
	}

	TypeBase *clone(unsigned short typeID);
	template<typename T> const Type<T> cast_to_type() const{
		MAKE_LOG(CoreLog);
		const Type<T>* ret=dynamic_cast<const Type<T>* >(this);
		if(ret){
		return *ret;
		} else {
		LOG(CoreLog,0) << "Cannot cast " << typeName() << " to " << Type<T>::staticName() << ". Returning \"" << Type<T>(T()).toString() << "\"." << std::endl;
		return Type<T>(T());
		}
	}
	template<typename T> Type<T>& cast_to_type() throw(std::bad_cast){
		MAKE_LOG(CoreLog);
		Type<T>* ret=dynamic_cast<Type<T>* >(this);
		if(!ret){
		LOG(CoreLog,0) << "Cannot cast " << typeName() << " to " << Type<T>::staticName() << ". Throwing bad_cast"<< std::endl;
		throw(std::bad_cast());
		}
		return *ret;
	}
};
}

/// Generic class for type aware variables
template<typename TYPE> class Type: public _internal::TypeBase{
	TYPE m_val;
	static std::string m_typeName;
	static unsigned short m_typeID;

public:
	template<typename T> Type(const T& value){
		m_val = __cast_to(this,value);
	}
	virtual bool is(const std::type_info & t)const{
		return t==typeid(TYPE);
	}
	virtual std::string toString(bool labeled=false)const{
		std::string ret=boost::lexical_cast<std::string>(m_val);
		if(labeled)ret+="("+staticName()+")";
		return ret;
	}
	virtual std::string typeName()const{
		return staticName();
	}
	virtual unsigned short typeID()const{
		return staticId();
	}
	static unsigned short staticId(){return m_typeID;}
	static std::string staticName(){return m_typeName;}
	operator TYPE()const{return m_val;}

	virtual ~Type(){}
};
}

#endif //DATATYPE_INC
