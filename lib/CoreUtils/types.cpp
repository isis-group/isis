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

/// @cond _hidden

#include "type.hpp"
#include "property.hpp"
#include "propmap.hpp"
#include "vector.hpp"
#include <stdint.h>

namespace isis{ namespace util{

/*
 * Define types for the Type<>-System here.
 * There must be a streaming output available for every type used here.
 * template<typename charT, typename traits,typename TYPE > basic_ostream<charT, traits>& operator<<(basic_ostream<charT, traits> &out,const TYPE& s)
 */

	
#define DEF_TYPE(TYPE,NAME,ID)  \
  template<> const char Type<TYPE>::m_typeName[]=#NAME;		\
  template<> const unsigned short Type<TYPE>::m_typeID=ID;
  
DEF_TYPE(int8_t,s8bit,0x1);
DEF_TYPE(uint8_t,u8bit,0x2);

DEF_TYPE(int16_t,s16bit,0x3);
DEF_TYPE(uint16_t,u16bit,0x4);

DEF_TYPE(int32_t,s32bit,0x5);
DEF_TYPE(uint32_t,u32bit,0x6);

DEF_TYPE(float,float,0x7);
DEF_TYPE(double,double,0x8);

DEF_TYPE(std::string,string,0x20);
DEF_TYPE(PropMap,PropertyMap,0xA0);
DEF_TYPE(fvector4,fvector4,0xA1);
}}

/// @endcond
