//
// C++ Implementation: type
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

namespace isis{

#define DEF_TYPE(TYPE,NAME,ID)  \
  template<> std::string Type<TYPE>::typeName=#NAME;\
  template<> unsigned int Type<TYPE>::typeID=ID;

  DEF_TYPE(char,s8bit,0x5);
  DEF_TYPE(unsigned char,u8bit,0x6);

  DEF_TYPE(short,s16bit,0x10);
  DEF_TYPE(unsigned short,u16bit,0x11);

  DEF_TYPE(int,s32bit,0x20);
  DEF_TYPE(unsigned int,u32bit,0x21);

  DEF_TYPE(float,float,0x30);
  DEF_TYPE(double,double,0x31);

  DEF_TYPE(std::string,string,0x40);
  DEF_TYPE(PropMap,PropertyMap,0xFE);
}
