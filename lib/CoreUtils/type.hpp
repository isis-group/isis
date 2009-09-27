#ifndef ISISTYPE_HPP
#define ISISTYPE_HPP

#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include "log.hpp"

namespace isis{ 
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util{

template<typename TYPE> class Type;
template<typename TYPE> class TypePtr;

/// @cond _internal
namespace _internal{
template<typename TYPE,typename T> TYPE __cast_to(Type<TYPE> *dest,const T& value){
	return boost::lexical_cast<TYPE>(value);
}
template<typename TYPE> TYPE __cast_to(Type<TYPE> *dest,const TYPE& value){
	return value;
}

class GenericType{
protected:
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
			LOG(CoreLog,error) << "Cannot cast " << typeName() << " to " << T::staticName() << ". Throwing bad_cast"<< std::endl;
			throw(std::bad_cast());
		}
		return *ret;
	}
	
public:
	/// Returns true if the stored value is of type T.
	template<typename T> bool is(){return is(typeid(T));}
	virtual bool is(const std::type_info & t)const = 0;
	/// Returns the value represented as text.
	virtual std::string toString(bool labeled=false)const=0;
	/// Returns the name of its actual type
	virtual std::string typeName()const=0;
	/// Returns the id of its actual type
	virtual unsigned short typeID()const=0;
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
	bool empty(){
		return this->get()!=NULL;
	}
};

class TypeBase : public GenericType{
public:
	typedef TypeReference<TypeBase> Reference;
	//operators
	template<typename T> bool operator ==(const Type<T> &src){
		MAKE_LOG(CoreLog);
		if(typeID()==Type<T>::staticId()){
			return this->cast_to_Type<T>() == src;
		} else {
			LOG(CoreLog,warning) 
				<< "Doing unstable lexical cast to compare " << this->toString(true) << " and " << src.toString(true) << std::endl;
			return (T)src==this->as<T>();
		}
	};
	template<typename T> Type<T> operator +(const Type<T> &src)const throw(std::bad_cast){
		return this->cast_to_Type<T>() + src;
	};
	template<typename T> Type<T> operator -(const Type<T> &src)const throw(std::bad_cast){
		return this->cast_to_Type<T>() - src;
	};
	template<typename T> Type<T> operator *(const Type<T> &src)const throw(std::bad_cast){
		return this->cast_to_Type<T>() * src;
	};
	template<typename T> Type<T> operator /(const Type<T> &src)const throw(std::bad_cast){
		return this->cast_to_Type<T>() / src;
	};
	
	/**
	 * Interpret the value as value of any (other) type.
	 * This is a runtime-based cast via string. The value is converted into a string, which is then parsed as the requestet type.
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

/// Generic class for type aware variables
template<typename TYPE> class Type: public _internal::TypeBase{
	TYPE m_val;
	static std::string m_typeName;
	static unsigned short m_typeID;
public:
	/**
	 * Create a Type from any type of value-type.
	 * The type of the parameter is not the same as the content type of the object, the system tries to do a type conversion.
	 * If that fails, boost::bad_lexical_cast is thrown.
	 */
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
	
	/**
	 * Implicit conversion of Type to its value type.
	 * Only the actual type is allowed.
	 * However, the following is valid:
	 * \code 
	 * Type<int> i(5);
	 * float f=i;
	 * \endcode
	 * The this case this function returns int which is then also implicitely converted to float.
	 * \return the stored value
	 */
	operator TYPE()const{return m_val;}

	virtual ~Type(){}
};

/**
 * Generic class for type (and length) - aware pointers.
 * The class is designed for arrays, but you can also "point" to an single element
 * by just use "1" for the length.
 * The pointers are reference counted and will be deleted automatically by a customizable deleter.
 */
template<typename TYPE> class TypePtr: public _internal::TypePtrBase{
	boost::shared_ptr<TYPE> m_val;
	static std::string m_typeName;
	static unsigned short m_typeID;
	const size_t m_len;
	template<typename T> TypePtr(const Type<T>& value); // Dont do this
public:
	/// Default delete-functor for c-arrays (uses free()).
	struct BasicDeleter{
		virtual void operator()(TYPE *p){
			MAKE_LOG(CoreDebug);
			LOG(CoreDebug,info) << "Freeing pointer " << p << " (" << TypePtr<TYPE>::staticName() << ") " << std::endl;
			free(p);
		};
	};
	/// Default delete-functor for arrays of objects (uses delete[]).
	struct ObjectArrayDeleter{
		virtual void operator()(TYPE *p){
			MAKE_LOG(CoreDebug);
			LOG(CoreDebug,info) << "Deleting object array at " << p << " (" << TypePtr<TYPE>::staticName() << ") " << std::endl;
			delete[] p;
		};
	};
	/**
	 * Creates TypePtr from a pointer of type TYPE.
	 * The pointers are automatically deleted by an instance of BasicDeleter and should not be used outside once used here.
	 * If ptr is a pointer to C++ objects (delete[] needed) you must use 
	 * TypePtr(ptr,len,TypePtr\<TYPE\>::ObjectArrayDeleter())!
	 * The usual dereferencing pointer interface ("*" and "->") is supported.
	 * \param ptr the pointer to the used array
	 * \param len the length of the used array (TypePtr does NOT check for length, 
	 * this is just here for child classes which may want to check)
	 */
	TypePtr(TYPE* ptr,size_t len):m_val(ptr,BasicDeleter()),m_len(len){}
	/**
	 * Creates TypePtr from a pointer of type TYPE.
	 * The pointers are automatically deleted by an copy of d and should not be used outside once used here.
	 * The usual dereferencing pointer interface ("*" and "->") is supported.
	 * D must implement operator()(TYPE *p).
	 * \param ptr the pointer to the used array
	 * \param len the length of the used array (TypePtr does NOT check for length,
	 * \param d the deleter to be used when the data shall be deleted ( d() is called then )
	 */
	template<typename D> TypePtr(TYPE* ptr,size_t len,D d):m_val(ptr,d),m_len(len){}
	/// @copydoc Type::is()
	virtual bool is(const std::type_info & t)const{
		return t==typeid(TYPE*);
	}
	/// @copydoc Type::toString()
	virtual std::string toString(bool labeled=false)const{
		std::string ret;
		if(m_len){
			const TYPE* ptr=m_val.get();
			for(size_t i=0;i<m_len-1;i++)
				ret+=Type<TYPE>(ptr[i]).toString(false)+"|";
			ret+=Type<TYPE>(ptr[m_len-1]).toString(true);
		}
		return boost::lexical_cast<std::string>(m_len) +"#"+ret;
	}
	/// @copydoc Type::typeName()
	virtual std::string typeName()const{
		return staticName();
	}
	/// @copydoc Type::typeID()
	virtual unsigned short typeID()const{
		return staticId();
	}
	/// @copydoc Type::staticID()
	static unsigned short staticId(){return m_typeID;}
	/// @copydoc Type::staticName()
	static std::string staticName(){return m_typeName;}
	
	/**
	 * Reference element at at given index.
	 * If index is invalid, behaviour is undefined. Probably it will crash.
	 * \return reference to element at at given index.
	 */
	TYPE& operator[](size_t idx){
		return (m_val.get())[idx];
	}
	/**
	 * Implicit conversion to boost::shared_ptr\<TYPE\>
	 * The returned smart pointer will be part of the reference-counting and will correctly delete the data 
	 * (using the given deleter) if required.
	 * \return boost::shared_ptr\<TYPE\> handling same data as the object.
	 */
	operator boost::shared_ptr<TYPE>(){return m_val;}
};

}
/** @} */
}

#endif //DATATYPE_INC
