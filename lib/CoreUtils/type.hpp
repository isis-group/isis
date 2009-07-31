#ifndef ISISTYPE_HPP
#define ISISTYPE_HPP

#include <string>
#include <boost/lexical_cast.hpp>
#include <map>
#include "log.hpp"

namespace isis{

template<typename TYPE> class Type;

class TypeBase{
protected:
  std::map<unsigned short,TypeBase*> _references;
public:
  template<typename T> bool is(){return is(typeid(T));}
  virtual bool is(const std::type_info & t)const = 0;
  virtual std::string toString(bool labeled=false)const=0;
  virtual std::string typeName()const=0;
  virtual unsigned short typeID()const=0;
  virtual ~TypeBase(){}
  TypeBase *clone(unsigned short typeID);
  template<typename T> const Type<T> cast_to_type() const{
	MAKE_LOG(CoreLog);
	const Type<T>* ret=dynamic_cast<const Type<T>* >(this);
	if(ret){
	  return *ret;
	} else {
	  LOG(CoreLog,0) << "Cannot cast " << typeName() << " to " << Type<T>::name() << ". Returning \"" << Type<T>(T()).toString() << "\"." << std::endl;
	  return Type<T>(T());
	}
  }
  template<typename T> Type<T>& cast_to_type() throw(std::bad_cast){
	MAKE_LOG(CoreLog);
	Type<T>* ret=dynamic_cast<Type<T>* >(this);
	if(!ret){
	  LOG(CoreLog,0) << "Cannot cast " << typeName() << " to " << Type<T>::name() << ". Throwing bad_cast"<< std::endl;
	  throw(std::bad_cast());
	}
	return *ret;
  }
};

template<typename TYPE,typename T> TYPE __cast_to(Type<TYPE> *dest,const T& value){
	return boost::lexical_cast<TYPE>(value);
}
template<typename TYPE> TYPE __cast_to(Type<TYPE> *dest,const TYPE& value){
	return value;
}

template<typename TYPE> class Type: public TypeBase{
  TYPE _val;
  static std::string _type;
  static unsigned short _typeID;

  public:
    template<typename T> Type(const T& value){
      _val = __cast_to(this,value);
    }
    template<typename T> T as()const{
      return boost::lexical_cast<T>(_val);
    }
    virtual bool is(const std::type_info & t)const{
	  return t==typeid(TYPE);
	}
    virtual std::string toString(bool labeled=false)const{
      std::string ret=as<std::string>();
      if(labeled)ret+="("+name()+")";
      return ret;
    }
    virtual std::string typeName()const{
	  return name();
	}
    virtual unsigned short typeID()const{
	  return id();
	}
    static unsigned short id(){return _typeID;}
    static std::string name(){return _type;}
    operator TYPE()const{return _val;}

    virtual ~Type(){}
};

}

#endif //DATATYPE_INC
