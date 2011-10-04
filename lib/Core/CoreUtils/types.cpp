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

#ifdef _MSC_VER
#pragma warning(disable:4800 4996)
#endif

#include "type.hpp"
#include "../DataStorage/typeptr.hpp"
#include "types.hpp"
#include <complex>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/foreach.hpp>

namespace isis
{
namespace util
{

/*
 * Define types for the Value<>-System here.
 * There must be a streaming output available for every type used here.
 * template<typename charT, typename traits,typename TYPE > basic_ostream<charT, traits>& operator<<(basic_ostream<charT, traits> &out,const TYPE& s)
 */


#define DEF_TYPE(TYPE,NAME)  \
	template<> const char Value<TYPE>::m_typeName[]=#NAME

DEF_TYPE( bool, boolean );

DEF_TYPE( int8_t, s8bit );
DEF_TYPE( uint8_t, u8bit );

DEF_TYPE( int16_t, s16bit );
DEF_TYPE( uint16_t, u16bit );

DEF_TYPE( int32_t, s32bit );
DEF_TYPE( uint32_t, u32bit );

DEF_TYPE( int64_t, s64bit );
DEF_TYPE( uint64_t, u64bit );

DEF_TYPE( float, float );
DEF_TYPE( double, double );

DEF_TYPE( color24, color24 );
DEF_TYPE( color48, color48 );

DEF_TYPE( fvector4, fvector4 );
DEF_TYPE( dvector4, dvector4 );
DEF_TYPE( ivector4, ivector4 );

DEF_TYPE( ilist, list<int32_t> );
DEF_TYPE( dlist, list<double> );
DEF_TYPE( slist, list<string> );

DEF_TYPE( std::complex<float>, complex<float> );
DEF_TYPE( std::complex<double>, complex<double> );

DEF_TYPE( std::string, string );
DEF_TYPE( Selection, selection );
DEF_TYPE( boost::posix_time::ptime, timestamp );
DEF_TYPE( boost::gregorian::date, date );

namespace _internal
{
struct type_lister {
	std::map< unsigned short, std::string > &m_map;
	bool m_withValues, m_withValuePtrs;
	type_lister( std::map< unsigned short, std::string > &map, bool withValues, bool withValuePtrs ): m_map( map ), m_withValues( withValues ), m_withValuePtrs( withValuePtrs ) {}
	template<typename SRC> void operator()( SRC ) {//will be called by the mpl::for_each
		if( m_withValues )m_map.insert( std::make_pair( util::Value<SRC>::staticID, util::Value<SRC>::staticName() ) );

		if( m_withValuePtrs )m_map.insert( std::make_pair( data::ValuePtr<SRC>::staticID, data::ValuePtr<SRC>::staticName() ) );
	}
};

}

std::map< unsigned short, std::string > getTypeMap( bool withValues, bool withValuePtrs )
{
	std::map< unsigned short, std::string > ret;
	boost::mpl::for_each<_internal::types>( _internal::type_lister( ret, withValues, withValuePtrs ) );
	return ret;
}

std::map< std::string, unsigned short > getTransposedTypeMap( bool withValues, bool withValuePtrs )
{
	typedef std::map< std::string, unsigned short> transposedMapType;
	typedef std::map< unsigned short, std::string > mapType;
	transposedMapType ret;
	BOOST_FOREACH( mapType::const_reference ref, util::getTypeMap( withValues, withValuePtrs ) ) {
		ret[ref.second] = ref.first;
	}
	return ret;
}

}
}

/// @endcond
