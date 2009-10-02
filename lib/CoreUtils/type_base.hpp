#ifndef ISISTYPE_BASE_HPP
#define ISISTYPE_BASE_HPP

#include "log.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

/*! \addtogroup util
*  Additional documentation for group `mygrp'
*  @{
*/
namespace isis{
namespace util{

template<typename TYPE> class Type;
template<typename TYPE> class TypePtr;

/// @cond _hidden
namespace _internal{
template<typename TYPE,typename T> TYPE __cast_to(Type<TYPE> *dest,const T& value){
	return boost::lexical_cast<TYPE>(value);
}
template<typename TYPE> TYPE __cast_to(Type<TYPE> *dest,const TYPE& value){
	return value;
}
/// @endcond

/// @cond _internal
class GenericType{
protected:
	template<typename T> const T m_cast_to(T defaultVal) const{
		MAKE_LOG(CoreLog);
		const T* ret=dynamic_cast<const T* >(this);
		if(ret){
			return *ret;
		} else {
			LOG(CoreLog,error) 
				<< "Cannot cast " << typeName() << " to " << T::staticName()
				<< ". Returning \"" << defaultVal.toString() << "\"." << std::endl;
			return defaultVal;
		}
	}
	template<typename T> T& m_cast_to() throw(std::bad_cast){
		MAKE_LOG(CoreLog);
		T* ret=dynamic_cast<T* >(this);
		if(!ret){
			LOG(CoreLog,error)
				<< "Cannot cast " << typeName() << " to " << T::staticName()
				<< ". Throwing bad_cast"<< std::endl;
			throw(std::bad_cast());
		}
		return *ret;
	}

public:
	/// \returns true if the stored value is of type T.
	template<typename T> bool is()const{return is(typeid(T));}
	virtual bool is(const std::type_info & t)const = 0;

	/// \returns the value represented as text.
	virtual std::string toString(bool labeled=false)const=0;

	/// \returns the name of its actual type
	virtual std::string typeName()const=0;

	/// \returns the id of its actual type
	virtual unsigned short typeID()const=0;

	/// \returns true if type of this and second are equal
	bool isSameType(const GenericType &second)const;
};

/**
* Base class to store and handle references to Type and TypePtr objects.
* The values (Type or TypePtr) are refernced as smart pointers to TypeBase.
* So the references are counted and data are automatically deleted if necessary.
* The usual dereferencing pointer interface ("*" and "->") is supported.
* This class is designed as base class for specialisations, it should not be used directly.
* Because of that, the contructors of this class are protected.
*/
template<typename TYPE_TYPE> class TypeReference:public boost::shared_ptr<TYPE_TYPE>{
protected:
	//dont use this directly
	TypeReference(TYPE_TYPE *t):boost::shared_ptr <TYPE_TYPE>(t){}
	TypeReference(){}
public:
	/// \returns true if "contained" type has no value (a.k.a. is undefined)
	bool empty()const{
		return this->get()==NULL;
	}
};

class TypeBase : public GenericType{
public:
	typedef TypeReference<TypeBase> Reference;

	/**
	* Interpret the value as value of any (other) type.
	* This is a runtime-based cast via string. The value is converted into a string, which is then parsed as the requestet
type.
	* \code
	* TypeBase *mephisto=new Type<std::string>("666");
	* int devil=mephisto->as<int>();
	* \endcode
	* If you know the type of source and destination at compile time you should use Type\<DEST_TYPE\>((SOURCE_TYPE)src).
	* \code
	* Type<std::string> mephisto("666");
	* Type<int> devil((std::string)devil);
	* \endcode
	* \return value of any requested type parsed from toString().
	*/
	template<class T> T as()const{
		MAKE_LOG(CoreLog);
		if(typeID()==Type<T>::staticId()){
			LOG(CoreLog,info)
			<< "Doing dynamic cast instead of useless lexical cast from " << toString(true)
			<< " to " << Type<T>::staticName() << std::endl;
			return this->cast_to_Type<T>();
		}
		return Type<T>(this->toString());
	}

	/**
	* Dynamically cast the TypeBase up to its actual Type\<T\>. Constant version.
	* Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	* \returns a copy of the stored value.
	* \returns T() if T is not the actual type.
	*/
	template<typename T> const Type<T> cast_to_Type() const{
		return m_cast_to<Type<T> >(Type<T>(T()));
	}
	/**
	* Dynamically cast the TypeBase up to its actual Type\<T\>. Referenced version.
	* Will throw std::bad_cast if T is not the actual type.
	* Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	* \returns a reference of the stored value.
	*/
	template<typename T> Type<T>& cast_to_Type() throw(std::bad_cast){
		return m_cast_to<Type<T> >();
	}
	virtual bool eq(const TypeBase &second)=0;
};

class TypePtrBase : public GenericType{
public:
	typedef TypeReference<TypePtrBase> Reference;
	/**
	* Dynamically cast the TypeBase up to its actual TypePtr\<T\>. Constant version.
	* Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	* \returns a copy of the pointer.
	* \returns TypePtr\<T\\>(NULL) if T is not the actual type.
	*/
	template<typename T> const TypePtr<T> cast_to_TypePtr() const{
		return m_cast_to<TypePtr<T> >(TypePtr<T>((T*)0));
	}
	/**
	* Dynamically cast the TypeBase up to its actual TypePtr\<T\>. Referenced version.
	* Will throw std::bad_cast if T is not the actual type.
	* Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	* \returns a reference of the pointer.
	*/
	template<typename T> TypePtr<T>& cast_to_TypePtr() throw(std::bad_cast){
		return m_cast_to<TypePtr<T> >();
	}
};

}
/// @endcond
}}

namespace std {
/// Streaming output for Type - classes
template<typename charT, typename traits> basic_ostream<charT, traits>&
operator<<(basic_ostream<charT, traits> &out,const isis::util::_internal::GenericType &s){
	return out<< s.toString();
}
/// /// Streaming output for Type referencing classes
template<typename charT, typename traits,typename TYPE_TYPE> basic_ostream<charT, traits>&
operator<<(basic_ostream<charT, traits> &out,const isis::util::_internal::TypeReference<TYPE_TYPE> &s){
	if(!s.empty())
		out << s->toString(true);
	return out;
}
}

/// }@

#endif //ISISTYPE_BASE_HPP