#ifndef TYPES_HPP_INCLUDED
#define TYPES_HPP_INCLUDED

#include <boost/mpl/vector/vector30.hpp>
#include <boost/mpl/distance.hpp>
#include <boost/mpl/find.hpp>
#include <stdint.h>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/contains.hpp>

#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "vector.hpp"
#include "color.hpp"
#include "selection.hpp"


namespace isis{ namespace util{
	
class PropMap; // predef PropMap

typedef std::list<int> ilist;
typedef std::list<double> dlist;
typedef std::list<std::string> slist;

/// @cond _internal
namespace _internal{

/// the supported types as mpl-vector
typedef boost::mpl::vector23< //increase this if a type is added (if <30 consider including vector40 above)
int8_t,uint8_t,int16_t,uint16_t,int32_t,uint32_t,int64_t,uint64_t
,float,double
,rgb_color24,rgb_color48
,fvector4,dvector4,ivector4
,ilist,dlist,slist
,PropMap,std::string,Selection
,boost::posix_time::ptime,boost::gregorian::date
> types;

/**
 * Templated pseudo struct to generate the id of a supported type.
 * The id is stored in TypeId\<T\>::value.
 * This is a compile-time-constant, so it can be used as a template parameter and has no impact at the runtime.
 */
template<class T> struct TypeId{
	typedef typename boost::mpl::distance<
	typename boost::mpl::begin<types>::type,
	typename boost::mpl::find<types, T>::type
	>::type type;
	static const int value=type::value;
};
}
/// @endcond
/**
 * Templated pseudo struct to check for availability of a type at compile time.
 * Instanciating this with any datatype (eg: check_type\<short\>() ) will cause the 
 * compiler to raise an error if this datatype is not in the list of the supported types.
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
