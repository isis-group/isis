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


//       template<typename ValueType> ValueType as()const{
//         ValueType const *ptr=boost::any_cast<ValueType>(this);
//         if(ptr)
//           return *ptr;
//         else
//           return lexical_cast_to<ValueType>();
//       }
//       template<typename ValueType> bool is()const{
//         return is(typeid(ValueType));
//       }
//   };
//
//   template<bool PrintTypeName=false> class PropertyVector:public std::vector<Property<PrintTypeName> >{
//   public:
//     template<typename _CharT, typename _Traits> std::basic_ostream<_CharT, _Traits> &
//       write(std::basic_ostream<_CharT, _Traits> &o,std::string delim=" ",std::string prefix="",std::string suffix="")const{
//       return write_list(this->begin(),this->end(),o,delim,prefix,suffix);
//     }
//     PropertyVector(size_t size):std::vector<Property<PrintTypeName> >(size){}
//   };

}
// template<typename _CharT, typename _Traits>
// std::basic_ostream<_CharT, _Traits> &operator<<(
//     std::basic_ostream<_CharT, _Traits> &o,
//     const isis::Property<false> &prop
// ){
//   o <<  prop.as<std::string>();
//   return o;
// }
//
// template<typename _CharT, typename _Traits>
// std::basic_ostream<_CharT, _Traits> &operator<<(
//   std::basic_ostream<_CharT, _Traits> &o,
//   const isis::Property<true> &prop
// ){
//   o <<  prop.as<std::string>() << "("<< prop.typeName() << ")";
//   return o;
// }
//
// template<typename _CharT, typename _Traits, bool PrintTypeName>
//     std::basic_ostream<_CharT, _Traits> & operator<<(
//         std::basic_ostream<_CharT, _Traits> &o,
//         const isis::PropertyVector<PrintTypeName> &vect
// ){
//   vect.write(o);
//   return o;
// }

#endif


