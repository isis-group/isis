/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "DataStorage/numeric_convert.hpp"

#ifdef ISIS_USE_LIBOIL
extern "C" {
	#include <liboil/liboil.h>
}


namespace isis{namespace data{namespace _internal{
	
/** some liboil based explicit implementations of numeric_convert_impl(const Src* src, Dst* dst, unsigned int count) */

#define IMPL_CONVERT(SRC,DST,SRC_KEY,DST_KEY)                                           \
template<> void numeric_convert_impl<SRC,DST>( const SRC *src, DST *dst, size_t count ){\
LOG( Runtime, info )                                                                    \
	<< "using generic convert " << TypePtr<SRC>::staticName() << " => "                 \
	<< TypePtr<DST>::staticName() << " without scaling";                                \
	oil_conv_ ## DST_KEY ## _ ## SRC_KEY (dst,sizeof(DST),src,sizeof(SRC),count);       \
}

#define IMPL_SCALED_CONVERT(SRC,DST,SRC_KEY,DST_KEY)                                                                 \
template<> void numeric_convert_impl<SRC,DST>( const SRC *src, DST *dst, size_t count, double scale, double offset ){\
	LOG( Runtime, info )                                                                                             \
		<< "using optimized scaling convert " << TypePtr<SRC>::staticName() << "=>" << TypePtr<DST>::staticName()    \
		<< " with scale/offset " << std::fixed << scale << "/" << offset;                                            \
	oil_scaleconv_ ## DST_KEY ## _ ## SRC_KEY (dst,src,count,&offset,&scale);                                        \
}

//>>s32
IMPL_CONVERT(float,int32_t,f32,s32)
IMPL_CONVERT(double,int32_t,f64,s32)
IMPL_CONVERT(uint32_t,int32_t,u32,s32)
IMPL_CONVERT(int16_t,int32_t,s16,s32)
IMPL_CONVERT(uint16_t,int32_t,u16,s32)
IMPL_CONVERT(int8_t,int32_t,s8,s32)
IMPL_CONVERT(uint8_t,int32_t,u8,s32)

//>>u32
//IMPL_CONVERT(float,uint32_t,f32,u32)
//IMPL_CONVERT(double,uint32_t,f64,u32)
IMPL_CONVERT(int32_t,uint32_t,s32,u32)
//IMPL_CONVERT(int16_t,uint32_t,s16,u32) ** Not available in liboil - but should be imho
IMPL_CONVERT(uint16_t,uint32_t,u16,u32)
//IMPL_CONVERT(int8_t,uint32_t,s8,u32) ** Not available in liboil - but should be imho
IMPL_CONVERT(uint8_t,uint32_t,u8,u32)

//>>s16
IMPL_CONVERT(float,int16_t,f32,s16)
IMPL_CONVERT(double,int16_t,f64,s16)
IMPL_CONVERT(int32_t,int16_t,s32,s16)
IMPL_CONVERT(uint32_t,int16_t,u32,s16)
IMPL_CONVERT(uint16_t,int16_t,u16,s16)
IMPL_CONVERT(int8_t,int16_t,s8,s16)
IMPL_CONVERT(uint8_t,int16_t,u8,s16)

//>>u16
IMPL_CONVERT(float,uint16_t,f32,u16)
IMPL_CONVERT(double,uint16_t,f64,u16)
IMPL_CONVERT(int32_t,uint16_t,s32,u16)
IMPL_CONVERT(uint32_t,uint16_t,u32,u16)
IMPL_CONVERT(int16_t,uint16_t,s16,u16)
//IMPL_CONVERT(int8_t,uint16_t,s8,u16) ** Not available in liboil - but should be imho
IMPL_CONVERT(uint8_t,uint16_t,u8,u16)

//>>s8
IMPL_CONVERT(float,int8_t,f32,s8)
IMPL_CONVERT(double,int8_t,f64,s8)
IMPL_CONVERT(int32_t,int8_t,s32,s8)
IMPL_CONVERT(uint32_t,int8_t,u32,s8)
IMPL_CONVERT(int16_t,int8_t,s16,s8)
IMPL_CONVERT(uint16_t,int8_t,u16,s8)
IMPL_CONVERT(uint8_t,int8_t,u8,s8)

//>>u8
IMPL_CONVERT(float,uint8_t,f32,u8)
IMPL_CONVERT(double,uint8_t,f64,u8)
IMPL_CONVERT(int32_t,uint8_t,s32,u8)
IMPL_CONVERT(uint32_t,uint8_t,u32,u8)
IMPL_CONVERT(int16_t,uint8_t,s16,u8)
IMPL_CONVERT(uint16_t,uint8_t,u16,u8)
IMPL_CONVERT(int8_t,uint8_t,s8,u8)

//>>f32
IMPL_CONVERT(double,float,f64,f32)
IMPL_CONVERT(int32_t,float,s32,f32)
IMPL_CONVERT(uint32_t,float,u32,f32)
IMPL_CONVERT(int16_t,float,s16,f32)
IMPL_CONVERT(uint16_t,float,u16,f32)
IMPL_CONVERT(int8_t,float,s8,f32)
IMPL_CONVERT(uint8_t,float,u8,f32)

//>>f64
IMPL_CONVERT(float,double,f32,f64)
IMPL_CONVERT(int32_t,double,s32,f64)
IMPL_CONVERT(uint32_t,double,u32,f64)
IMPL_CONVERT(int16_t,double,s16,f64)
IMPL_CONVERT(uint16_t,double,u16,f64)
IMPL_CONVERT(int8_t,double,s8,f64)
IMPL_CONVERT(uint8_t,double,u8,f64)


//scale>>s32
IMPL_SCALED_CONVERT(float,int32_t,f32,s32)
IMPL_SCALED_CONVERT(double,int32_t,f64,s32)

//scale>>u32
//IMPL_SCALED_CONVERT(float,uint32_t,f32,u32)
//IMPL_SCALED_CONVERT(double,uint32_t,f64,u32)

//scale>>s16
IMPL_SCALED_CONVERT(float,int16_t,f32,s16)
IMPL_SCALED_CONVERT(double,int16_t,f64,s16)

//scale>>u16
IMPL_SCALED_CONVERT(float,uint16_t,f32,u16)
IMPL_SCALED_CONVERT(double,uint16_t,f64,u16)

//scale>>s8
IMPL_SCALED_CONVERT(float,int8_t,f32,s8)
IMPL_SCALED_CONVERT(double,int8_t,f64,s8)

//scale>>u8
IMPL_SCALED_CONVERT(float,uint8_t,f32,u8)
IMPL_SCALED_CONVERT(double,uint8_t,f64,u8)

//scale>>f32
IMPL_SCALED_CONVERT(int32_t,float,s32,f32)
IMPL_SCALED_CONVERT(uint32_t,float,u32,f32)
IMPL_SCALED_CONVERT(int16_t,float,s16,f32)
IMPL_SCALED_CONVERT(uint16_t,float,u16,f32)
IMPL_SCALED_CONVERT(int8_t,float,s8,f32)
IMPL_SCALED_CONVERT(uint8_t,float,u8,f32)

//scale>>f64
IMPL_SCALED_CONVERT(int32_t,double,s32,f64)
IMPL_SCALED_CONVERT(uint32_t,double,u32,f64)
IMPL_SCALED_CONVERT(int16_t,double,s16,f64)
IMPL_SCALED_CONVERT(uint16_t,double,u16,f64)
IMPL_SCALED_CONVERT(int8_t,double,s8,f64)
IMPL_SCALED_CONVERT(uint8_t,double,u8,f64)

#undef IMPL_CONVERT
#undef IMPL_SCALED_CONVERT
}}}
#endif //ISIS_USE_LIBOIL

