#ifndef ISISTYPE_HPP
#define ISISTYPE_HPP

#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include "log.hpp"

namespace iUtil{

template<typename TYPE> class Type;
template<typename TYPE> class TypePtr;

namespace _internal{
template<typename TYPE,typename T> TYPE __cast_to(Type<TYPE> *dest,const T& value){
	return boost::lexical_cast<TYPE>(value);
}
template<typename TYPE> TYPE __cast_to(Type<TYPE> *dest,const TYPE& value){
	return value;
}

class TypeBase{
	template<typename T> const T m_cast_to(T defaultVal) const{
		MAKE_LOG(CoreLog);
		const T* ret=dynamic_cast<const T* >(this);
		if(ret){
			return *ret;
		} else {
			LOG(CoreLog,error) << "Cannot cast " << typeName() << " to " << T::staticName() << ". Returning \"" << defaultVal.toString() << "\"." << std::endl;
			return defaultVal;
		}
	}
	template<typename T> T& m_cast_to() throw(std::bad_cast){
		MAKE_LOG(CoreLog);
		T* ret=dynamic_cast<T* >(this);
		if(!ret){
			LOG(CoreLog,0) << "Cannot cast " << typeName() << " to " << T::staticName() << ". Throwing bad_cast"<< std::endl;
			throw(std::bad_cast());
		}
		return *ret;
	}
	
public:
	template<typename T> bool is(){return is(typeid(T));}
	virtual bool is(const std::type_info & t)const = 0;
	virtual std::string toString(bool labeled=false)const=0;
	virtual std::string typeName()const=0;
	virtual unsigned short typeID()const=0;

	template<class T> T as(){
		MAKE_LOG(CoreLog);
		if(typeID() & 0xFF00){
			LOG(CoreLog,warning) 
				<< "You're trying to lexically cast a pointer (" 
				<< typeName() << ") to a value ("<< Type<T>::staticName() 
				<< ") this is most likely nonsense" << std::endl;
		}
		Type<T> ret(this->toString());
		return (T)ret;
	}

	template<typename T> const Type<T> cast_to_type() const{
		return m_cast_to<Type<T> >(Type<T>(T()));
	}
	template<typename T> const TypePtr<T> cast_to_type_ptr() const{
		return m_cast_to<TypePtr<T> >(TypePtr<T>((T*)0));
	}
	template<typename T> Type<T>& cast_to_type() throw(std::bad_cast){
		return m_cast_to<Type<T> >();
	}
	template<typename T> TypePtr<T>& cast_to_type_ptr() throw(std::bad_cast){
		return m_cast_to<TypePtr<T> >();
	}
};
}

/// Generic class for type aware variables
template<typename TYPE> class Type: public _internal::TypeBase{
	template<typename T> Type(const TypePtr<T>& value); // Dont do this
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

/// Generic class for type aware pointers
template<typename TYPE> class TypePtr: public _internal::TypeBase{
	boost::shared_ptr<TYPE> m_val;
	static std::string m_typeName;
	static unsigned short m_typeID;
	template<typename T> TypePtr(const Type<T>& value); // Dont do this
public:
	struct Deleter{
		virtual void operator()(TYPE *p){
			MAKE_LOG(CoreLog);
			LOG(CoreLog,2) << "Freeing pointer " << p << " (" << TypePtr<TYPE>::staticName() << ") " << std::endl;
			delete p;
		};
	};
	template<typename T> TypePtr(T* ptr):m_val(ptr,Deleter()){}
	template<typename T, typename D> TypePtr(T* ptr,D d):m_val(ptr,d){}
	virtual bool is(const std::type_info & t)const{
		return t==typeid(TYPE);
	}
	virtual std::string toString(bool labeled=false)const{
		//@todo implement me
		return "implement me";
	}
	virtual std::string typeName()const{
		return staticName();
	}
	virtual unsigned short typeID()const{
		return staticId();
	}
	static unsigned short staticId(){return m_typeID;}
	static std::string staticName(){return m_typeName;}
	operator TYPE*()const{return m_val;}
	
	virtual ~TypePtr(){
		//@todo implement me
	}
};

}

#endif //DATATYPE_INC
