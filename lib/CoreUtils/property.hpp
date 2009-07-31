#ifndef ISISPROPERTY_HPP
#define ISISPROPERTY_HPP

#include <boost/shared_ptr.hpp>
#include <map>
#include "type.hpp"

namespace isis {

/**
common property class

	@author Enrico Reimer
*/


class Property:public boost::shared_ptr<TypeBase>{
  public:
  template<typename T> Property(const T& ref):boost::shared_ptr <TypeBase >(new Type<T>(ref)){ }
  Property(){ }
  template<typename T> operator T()const{
    const Type<T>* ret=dynamic_cast<Type<T>*>(get());
    assert(ret);
    return (T)(*ret);
  }
};

}

#endif


