#ifndef ISISTYPE_HPP
#define ISISTYPE_HPP

#include <string>
#include <boost/lexical_cast.hpp>

namespace isis{

class TypeBase{
  public:
    template<typename T> bool is(){return is(typeid(T));}
    virtual bool is(const std::type_info & t)const = 0;
    virtual std::string toString(bool labeled=false)const=0;
    virtual ~TypeBase(){}
};

template<typename TYPE> class Type;

template<typename TYPE,typename T> TYPE __cast_to_type(Type<TYPE> *dest,const T& value){
	return boost::lexical_cast<TYPE>(value);
}
template<typename TYPE> TYPE __cast_to_type(Type<TYPE> *dest,const TYPE& value){
	return value;
}


template<typename TYPE> class Type: public TypeBase{
  TYPE val;
  static std::string typeName;
  static unsigned int typeID;

  public:
    template<typename T> Type(const T& value){
      val = __cast_to_type(this,value);
    }
    template<typename T> T as()const{
      return boost::lexical_cast<T>(val);
    }
    virtual bool is(const std::type_info & t)const{ return t==typeid(TYPE);}
    virtual std::string toString(bool labeled=false)const{
      std::string ret=as<std::string>();
      if(labeled)ret+="("+name()+")";
      return ret;
    }
    static unsigned int id(){return typeID;}
    static std::string name(){return typeName;}
    operator TYPE()const{return val;}

    virtual ~Type(){}
};

}

#endif //DATATYPE_INC
