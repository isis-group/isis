#ifndef TYPES_HPP_INCLUDED
#define TYPES_HPP_INCLUDED

#include <boost/mpl/vector.hpp>
#include <boost/mpl/distance.hpp>
#include <boost/mpl/find.hpp>
#include <stdint.h>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/contains.hpp>

#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "vector.hpp"


namespace isis{ namespace util{
	
class PropMap; // predef PropMap

namespace _internal{

typedef boost::mpl::vector<
int8_t,uint8_t,int16_t,uint16_t,int32_t,uint32_t,int64_t,uint64_t
,float,double
,fvector4,dvector4,ivector4,std::string
,PropMap
,boost::posix_time::ptime,boost::gregorian::date
> types;

template<class T> struct TypeId{
	typedef typename boost::mpl::distance<
	typename boost::mpl::begin<types>::type,
	typename boost::mpl::find<types, T>::type
	>::type type;
	static const int value=type::value;
};
}

/**
 * Templated pseudo struct to check for availability of a type at compile time.
 * Instanciating this with any datatype will cause the compiler to check if this datatype is in the list of the supported types.
 * If it isn't a compile-error will be raised.
 */
template< typename T > struct check_type
{
	BOOST_MPL_ASSERT_MSG(
	(boost::mpl::contains<_internal::types,T>::value)
	, TYPE_IS_NOT_KNOWN
	, (T)
	);
};
}}

#endif //TYPES_HPP_INCLUDED
