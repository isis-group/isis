//
// C++ Implementation: types
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "type.hpp"
#include "property.hpp"
#include "propmap.hpp"

namespace isis{ namespace util{

#define DEF_TYPE(TYPE,NAME,ID)  \
  template<> const std::string Type<TYPE>::m_typeName=#NAME;\
  template<> const unsigned short Type<TYPE>::m_typeID=ID; \
  template<> const std::string TypePtr<TYPE>::m_typeName=#NAME + std::string("*");\
  template<> const unsigned short TypePtr<TYPE>::m_typeID=ID<<2; 
  
DEF_TYPE(char,s8bit,0x1);
DEF_TYPE(unsigned char,u8bit,0x2);

DEF_TYPE(short,s16bit,0x3);
DEF_TYPE(unsigned short,u16bit,0x4);

DEF_TYPE(int,s32bit,0x5);
DEF_TYPE(unsigned int,u32bit,0x6);

DEF_TYPE(float,float,0x7);
DEF_TYPE(double,double,0x8);

DEF_TYPE(std::string,string,0x20);
DEF_TYPE(PropMap,PropertyMap,0xF0);
}}
