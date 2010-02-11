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
#include "types.hpp"

namespace isis{ namespace util{

/*
 * Define types for the Type<>-System here.
 * There must be a streaming output available for every type used here.
 * template<typename charT, typename traits,typename TYPE > basic_ostream<charT, traits>& operator<<(basic_ostream<charT, traits> &out,const TYPE& s)
 */

	
#define DEF_TYPE(TYPE,NAME)  \
  template<> const char Type<TYPE>::m_typeName[]=#NAME;
  
DEF_TYPE(int8_t,s8bit);
DEF_TYPE(uint8_t,u8bit);

DEF_TYPE(int16_t,s16bit);
DEF_TYPE(uint16_t,u16bit);

DEF_TYPE(int32_t,s32bit);
DEF_TYPE(uint32_t,u32bit);

DEF_TYPE(int64_t,s64bit);
DEF_TYPE(uint64_t,u64bit);

DEF_TYPE(float,float);
DEF_TYPE(double,double);

DEF_TYPE(fvector4,fvector4);
DEF_TYPE(dvector4,dvector4);
DEF_TYPE(ivector4,ivector4);

DEF_TYPE(ilist,list<int>);
DEF_TYPE(dlist,list<double>);
DEF_TYPE(slist,list<string>);

DEF_TYPE(std::string,string);
DEF_TYPE(PropMap,PropertyMap);
DEF_TYPE(boost::posix_time::ptime,timestamp);
DEF_TYPE(boost::gregorian::date,date);
}}

/// @endcond
