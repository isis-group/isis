#ifndef TYPES_HPP_INCLUDED
#define TYPES_HPP_INCLUDED

#include <boost/mpl/vector/vector30.hpp>
#include <boost/mpl/distance.hpp>
#include <boost/mpl/plus.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <boost/type_traits/is_base_of.hpp>

#ifndef _MSC_VER
#include <stdint.h>
#endif

#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "vector.hpp"
#include "color.hpp"
#include "selection.hpp"

#include <complex>

namespace isis
{
namespace util
{

typedef std::list<int32_t> ilist;
typedef std::list<double> dlist;
typedef std::list<std::string> slist;

/// @cond _internal
namespace _internal
{

/// the supported types as mpl-vector
typedef boost::mpl::vector29 < //increase this if a type is added (if >30 consider including vector40 above)
bool //1
, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t // 9
, float, double // 11
, color24, color48 // 13
, fvector3, dvector3 // 15
, fvector4, dvector4, ivector4 // 18
, ilist, dlist, slist // 21
, std::string, isis::util::Selection //23
, std::complex<float>, std::complex<double> //25
, boost::posix_time::ptime, boost::gregorian::date //27
, boost::posix_time::time_duration, boost::gregorian::date_duration //29
> types;

template<bool ENABLED> struct ordered{
	static const bool lt=ENABLED,gt=ENABLED;
};
template<bool ENABLED> struct multiplicative{
	static const bool mult=ENABLED,div=ENABLED;
};
template<bool ENABLED> struct additive{
	static const bool plus=ENABLED,minus=ENABLED,negate=ENABLED;
};
}
/// @endcond

/**
 * resolves to boost::true_type if type is known, boost::false_type if not
 */
template< typename T > struct knowType :  boost::mpl::contains<_internal::types, T> {};
/**
 * Templated pseudo struct to check for availability of a type at compile time.
 * Instanciating this with any datatype (eg: checkType\<short\>() ) will cause the
 * compiler to raise an error if this datatype is not in the list of the supported types.
 */
template< typename T > struct checkType {
	BOOST_MPL_ASSERT_MSG(
		( knowType<T>::value ), TYPE_IS_NOT_KNOWN, ( T )
	);
};

/**
 * Templated pseudo struct to check if a type supports an operation at compile time.
 * The compile-time flags are:
 * - \c \b lt can compare less-than
 * - \c \b gt can compare greater-than
 * - \c \b mult multiplication is applicable
 * - \c \b div division is applicable
 * - \c \b plus addition is applicable
 * - \c \b minus substraction is applicable
 * - \c \b negate negation is applicable
 */
template<typename T> struct has_op:
	_internal::ordered<boost::mpl::and_<boost::mpl::not_<boost::is_base_of<_internal::VectorClass,T> >, knowType<T> >::value>, //vectors are not ordered
	_internal::additive<knowType<T>::value>,
	_internal::multiplicative<knowType<T>::value>
	{};

/// @cond _internal
template<> struct has_op<Selection>:_internal::ordered<true>,_internal::additive<false>,_internal::multiplicative<false>{};
template<> struct has_op<std::string>:_internal::ordered<true>,_internal::multiplicative<false>{static const bool plus=true,minus=false;};

// cannot multiply time
template<> struct has_op<boost::gregorian::date>:_internal::ordered<true>,_internal::additive<true>,_internal::multiplicative<false>{};
template<> struct has_op<boost::gregorian::date_duration>:_internal::ordered<true>,_internal::additive<true>,_internal::multiplicative<false>{};
template<> struct has_op<boost::posix_time::ptime>:_internal::ordered<true>,_internal::additive<true>,_internal::multiplicative<false>{};
template<> struct has_op<boost::posix_time::time_duration>:_internal::ordered<true>,_internal::additive<true>,_internal::multiplicative<false>{};

// multidim is not ordered
template<typename T> struct has_op<std::list<T> >:_internal::ordered<true>,_internal::additive<false>,_internal::multiplicative<false>{};
template<typename T> struct has_op<color<T> >:_internal::ordered<false>,_internal::additive<false>,_internal::multiplicative<false>{};
template<typename T> struct has_op<std::complex<T> >:_internal::ordered<false>,_internal::additive<true>,_internal::multiplicative<true>{};
/// @endcond


/**
 * Get a std::map mapping type IDs to type names.
 * \note the list is generated at runtime, so doing this excessively will be expensive.
 * \param withValues include util::Value-s in the map
 * \param withValueArrays include data::ValueArray-s in the map
 * \returns a map, mapping util::Value::staticID() and data::ValueArray::staticID() to util::Value::staticName and data::ValueArray::staticName
 */
std::map<unsigned short, std::string> getTypeMap( bool withValues = true, bool withValueArrays = true );

/**
 * Inverse of getTypeMap.
 * \note the list is generated at runtime, so doing this excessively will be expensive.
 * \param withValues include util::Value-s in the map
 * \param withValueArrays include data::ValueArray-s in the map
 * \returns a map, mapping util::Value::staticName and data::ValueArray::staticName to util::Value::staticID() and data::ValueArray::staticID()
 */
std::map< std::string, unsigned short> getTransposedTypeMap( bool withValues = true, bool withValueArrays = true );
}
}

// define +/- operations for ptime and date
namespace std{
	template<> struct plus<boost::gregorian::date>:binary_function<boost::gregorian::date,boost::gregorian::date_duration,boost::gregorian::date>{
		boost::gregorian::date operator() (const boost::gregorian::date& x, const boost::gregorian::date_duration& y) const {return x+y;}
	};
	template<> struct minus<boost::gregorian::date>:binary_function<boost::gregorian::date,boost::gregorian::date_duration,boost::gregorian::date>{
		boost::gregorian::date operator() (const boost::gregorian::date& x, const boost::gregorian::date_duration& y) const {return x-y;}
	};
	template<> struct plus<boost::posix_time::ptime>:binary_function<boost::posix_time::ptime,boost::posix_time::time_duration,boost::posix_time::ptime>{
		boost::posix_time::ptime operator() (const boost::posix_time::ptime& x, const boost::posix_time::time_duration& y) const {return x+y;}
	};
	template<> struct minus<boost::posix_time::ptime>:binary_function<boost::posix_time::ptime,boost::posix_time::time_duration,boost::posix_time::ptime>{
		boost::posix_time::ptime operator() (const boost::posix_time::ptime& x, const boost::posix_time::time_duration& y) const {return x-y;}
	};
}

#endif //TYPES_HPP_INCLUDED
