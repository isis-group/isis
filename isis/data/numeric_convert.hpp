#ifndef NUMERIC_CONVERT_HPP
#define NUMERIC_CONVERT_HPP

#include <limits>
#include <assert.h>
#include "common.hpp"
#include "valuearray.hpp"


namespace isis
{
namespace data
{
enum autoscaleOption;
API_EXCLUDE_BEGIN;
/// @cond _internal
namespace _internal
{

template<typename T> T round_impl( double x, boost::mpl::bool_<true> )
{
	const double ret( x < 0 ? x - 0.5 : x + 0.5 );
	return static_cast<T>( ret );
}
template<typename T> T round_impl( double x, boost::mpl::bool_<false> )
{
	return static_cast<T>( x ); //@todo find a proper way to round from double to floating point (with range check !!)
}
template<typename T> T round( double x )
{
	static_assert( std::is_arithmetic<T>::value, "Cannot round non-number" );
	return round_impl<T>( x, boost::mpl::bool_<std::numeric_limits<T>::is_integer>() ); //we do use overloading intead of (forbidden) partial specialization
}

template<typename SRC, typename DST> void numeric_convert_impl( const SRC *src, DST *dst, size_t count, double scale, double offset )
{
	LOG( Runtime, info )
			<< "using generic scaling convert " << ValueArray<SRC>::staticName() << "=>" << ValueArray<DST>::staticName()
			<< " with scale/offset " << std::fixed << scale << "/" << offset;

	static const std::pair<DST, DST> limits(
		std::numeric_limits<DST>::max(),
		std::numeric_limits<DST>::has_denorm ? -std::numeric_limits<DST>::max() : std::numeric_limits<DST>::min() //for types with denormalization min is _not_ the lowest value
	);

	for ( size_t i = 0; i < count; i++ ) {
		auto scaled=src[i] * scale + offset;
		dst[i] = 
			scaled<limits.second ? 
				limits.second : scaled > limits.first ? 
					limits.first : round<DST>( scaled );
	}
}

template<typename SRC, typename DST> void numeric_convert_impl( const SRC *src, DST *dst, size_t count )
{
	LOG( Runtime, info ) << "using generic convert " << ValueArray<SRC>::staticName() << " => " << ValueArray<DST>::staticName() << " without scaling";

	for ( size_t i = 0; i < count; i++ )
		dst[i] = round<DST>( src[i] );
}
template<typename SRC, typename DST> void numeric_convert_impl( const std::complex<SRC> *src, std::complex<DST> *dst, size_t count, double /*scale*/, double /*offset*/ )
{
	LOG( Debug, error ) << "complex conversion with scaling is not yet supportet";
	numeric_convert_impl( src, dst, count );
}

template<typename T> void numeric_copy_impl( const T *src, T *dst, size_t count )
{
	LOG( Runtime, info )    << "using memcpy-copy of " << ValueArray<T>::staticName() << " without scaling";
	memcpy( dst, src, count * sizeof( T ) );
}
template<typename T> void numeric_copy_impl( const T *src, T *dst, size_t count, double scale, double offset )
{
	LOG( Runtime, info )    << "using generic scaling copy of " << ValueArray<T>::staticName() << " with scale/offset " << std::fixed << scale << "/" << offset;
	numeric_convert_impl<T,T>(src,dst,count,scale,offset);
}

#ifdef ISIS_USE_LIBOIL
#define DECL_CONVERT(SRC_TYPE,DST_TYPE)        template<> void numeric_convert_impl<SRC_TYPE,DST_TYPE>( const SRC_TYPE *src, DST_TYPE *dst, size_t count )
#define DECL_SCALED_CONVERT(SRC_TYPE,DST_TYPE) template<> void numeric_convert_impl<SRC_TYPE,DST_TYPE>( const SRC_TYPE *src, DST_TYPE *dst, size_t count, double scale, double offset )
// storage class for explicit specilisations is not allowed (http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#605)

//>>s32
DECL_CONVERT( float, int32_t );
DECL_CONVERT( double, int32_t );
DECL_CONVERT( uint32_t, int32_t );
DECL_CONVERT( int16_t, int32_t );
DECL_CONVERT( uint16_t, int32_t );
DECL_CONVERT( int8_t, int32_t );
DECL_CONVERT( uint8_t, int32_t );

//>>u32
//DECL_CONVERT(float,uint32_t); conversion to u32 is broken (https://bugs.freedesktop.org/show_bug.cgi?ID=16524)
//DECL_CONVERT(double,uint32_t);
DECL_CONVERT( int32_t, uint32_t );
//DECL_CONVERT(int16_t,uint32_t); ** Not available in liboil - but should be imho
DECL_CONVERT( uint16_t, uint32_t );
//DECL_CONVERT(int8_t,uint32_t); ** Not available in liboil - but should be imho
DECL_CONVERT( uint8_t, uint32_t );

//>>s16
DECL_CONVERT( float, int16_t );
DECL_CONVERT( double, int16_t );
DECL_CONVERT( int32_t, int16_t );
DECL_CONVERT( uint32_t, int16_t );
DECL_CONVERT( uint16_t, int16_t );
DECL_CONVERT( int8_t, int16_t );
DECL_CONVERT( uint8_t, int16_t );

//>>u16
DECL_CONVERT( float, uint16_t );
DECL_CONVERT( double, uint16_t );
DECL_CONVERT( int32_t, uint16_t );
DECL_CONVERT( uint32_t, uint16_t );
DECL_CONVERT( int16_t, uint16_t );
//DECL_CONVERT(int8_t,uint16_t); ** Not available in liboil - but should be imho
DECL_CONVERT( uint8_t, uint16_t );

//>>s8
DECL_CONVERT( float, int8_t );
DECL_CONVERT( double, int8_t );
DECL_CONVERT( int32_t, int8_t );
DECL_CONVERT( uint32_t, int8_t );
DECL_CONVERT( int16_t, int8_t );
DECL_CONVERT( uint16_t, int8_t );
DECL_CONVERT( uint8_t, int8_t );

//>>u8
DECL_CONVERT( float, uint8_t );
DECL_CONVERT( double, uint8_t );
DECL_CONVERT( int32_t, uint8_t );
DECL_CONVERT( uint32_t, uint8_t );
DECL_CONVERT( int16_t, uint8_t );
DECL_CONVERT( uint16_t, uint8_t );
DECL_CONVERT( int8_t, uint8_t );

//>>f32
DECL_CONVERT( double, float );
DECL_CONVERT( int32_t, float );
DECL_CONVERT( uint32_t, float );
DECL_CONVERT( int16_t, float );
DECL_CONVERT( uint16_t, float );
DECL_CONVERT( int8_t, float );
DECL_CONVERT( uint8_t, float );

//>>f64
DECL_CONVERT( float, double );
DECL_CONVERT( int32_t, double );
DECL_CONVERT( uint32_t, double );
DECL_CONVERT( int16_t, double );
DECL_CONVERT( uint16_t, double );
DECL_CONVERT( int8_t, double );
DECL_CONVERT( uint8_t, double );

//scale>>s32
DECL_SCALED_CONVERT( float, int32_t );
DECL_SCALED_CONVERT( double, int32_t );

//scale>>u32
//DECL_SCALED_CONVERT(float,uint32_t); conversion to u32 is broken (https://bugs.freedesktop.org/show_bug.cgi?ID=16524)
//DECL_SCALED_CONVERT(double,uint32_t);

//scale>>s16
DECL_SCALED_CONVERT( float, int16_t );
DECL_SCALED_CONVERT( double, int16_t );

//scale>>u16
DECL_SCALED_CONVERT( float, uint16_t );
DECL_SCALED_CONVERT( double, uint16_t );

//scale>>s8
DECL_SCALED_CONVERT( float, int8_t );
DECL_SCALED_CONVERT( double, int8_t );

//scale>>u8
DECL_SCALED_CONVERT( float, uint8_t );
DECL_SCALED_CONVERT( double, uint8_t );

//scale>>f32
DECL_SCALED_CONVERT( int32_t, float );
DECL_SCALED_CONVERT( uint32_t, float );
DECL_SCALED_CONVERT( int16_t, float );
DECL_SCALED_CONVERT( uint16_t, float );
DECL_SCALED_CONVERT( int8_t, float );
DECL_SCALED_CONVERT( uint8_t, float );

//scale>>f64
DECL_SCALED_CONVERT( int32_t, double );
DECL_SCALED_CONVERT( uint32_t, double );
DECL_SCALED_CONVERT( int16_t, double );
DECL_SCALED_CONVERT( uint16_t, double );
DECL_SCALED_CONVERT( int8_t, double );
DECL_SCALED_CONVERT( uint8_t, double );

#undef DECL_CONVERT
#undef DECL_SCALED_CONVERT
#endif //ISIS_USE_LIBOIL

}
/// @endcond _internal
API_EXCLUDE_END;
/**
 * Computes scaling and offset between two scalar value domains.
 * The rules are:
 * - If the value range defined by min and max does not fit into the domain of dst they will be scaled
 * - scale "around 0" if 0 is part of the source value range.
 * - elsewise values will be offset towards 0 if the value range of the source does not fit the destination.
 * - if destination is unsigned, values will be offset to be in positive domain if necessary.
 * - if destination is floating point no scaling is done at all (scale factor will be 1, offset will be 0).
 * The scaling strategies are:
 * - autoscale: do not scale up (scale <=1) if src is an integer type, otherwise also do upscaling
 * - noupscale: never scale up (scale <=1)
 * - upscale: enforce upscaling even if SRC is an integer type
 * - noscale: do not scale at all (scale==1)
 * \param min the smallest value of the source data
 * \param max the biggest value of the source data
 * \param scaleopt enum to tweak the scaling strategy
 */
template<typename SRC, typename DST> std::pair<double, double>
getNumericScaling( const util::ValueBase &min, const util::ValueBase &max, autoscaleOption scaleopt = autoscale )
{
	double scale = 1.0;
	double offset = 0.0;
	bool doScale = ( scaleopt != noscale && std::numeric_limits<DST>::is_integer ); //only do scale if scaleopt!=noscale and the target is an integer (scaling into float is useless)

	if ( scaleopt == autoscale && std::numeric_limits<SRC>::is_integer ) {
		LOG( Debug, verbose_info ) << "Won't upscale, because the source datatype is discrete (" << util::Value<SRC>::staticName() << ")";
		scaleopt = noupscale; //dont scale up if SRC is an integer
	}

	if ( doScale ) {
		const DST domain_min = std::numeric_limits<DST>::min();//negative value domain of this dst [min .. -1]
		const DST domain_max = std::numeric_limits<DST>::max();//positive value domain of this dst [0 .. max]
		double minval, maxval;
		minval = min.as<double>();
		maxval = max.as<double>();
		assert( minval <= maxval );
		LOG( Debug, info ) << "src Range:" << minval << "=>" << maxval;
		LOG( Debug, info ) << "dst Domain:" << static_cast<double>( domain_min ) << "=>" << static_cast<double>( domain_max );
		assert( domain_min < domain_max );//we also should assume this

		//set offset for src
		//if all src is completly on positive domain, or if there is no negative domain use minval
		//else if src is completly on negative dmain, or if there is no positive domain use maxval
		//elsewise leave it at 0 and scale both sides
		if ( minval > 0 || !domain_min ) {
			//          if ( ( maxval - domain_max ) > 0 ) // if the values completely fit into the domain we dont have to offset them
			offset = -minval;
		} else if ( ( 0 - maxval ) > 0 || !domain_max ) {
			//          if ( ( domain_min - minval ) > 0  ) // if the values completely fit into the domain we dont have to offset them
			offset = -maxval;
		}

		//calculate range of values which will be on postive/negative domain when offset is applied
		const double range_max = maxval + offset; //allways >=0
		const double range_min = minval + offset; //allways <=0
		//set scaling factor to fit src-range into dst domain
		//some compilers dont make x/0 = inf, so we use std::numeric_limits<double>::max() instead, in this case
		const double scale_max =
			range_max != 0 ? domain_max / range_max :
			std::numeric_limits<double>::max();
		const double scale_min =
			range_min != 0 ? domain_min / range_min :
			std::numeric_limits<double>::max();
		scale = std::min( scale_max ? scale_max : std::numeric_limits<double>::max(), scale_min ? scale_min : std::numeric_limits<double>::max() );//get the smaller scaling factor which is not zero so the bigger range will fit into his domain

		if ( scaleopt == noupscale && scale > 1 ) {
			LOG( Runtime, info ) << "upscale not given, clamping scale " << scale << " to 1";
			scale = 1;
		}

		if( scale == 1 ) {
			if( ( minval - domain_min ) > 0 && ( domain_max - maxval ) > 0 )
				offset = 0; // if the source does fit into the domain, and we do not scale - we wont need an offset
		} else
			offset *= scale;//calc offset for dst
	}

	return std::make_pair( scale, offset );
}

/**
 * Converts data from 'src' to the type of 'dst' and stores them there.
 * If the value range defined by min and max does not fit into the domain of dst they will be scaled using the following rules:
 * - scale "around 0" if 0 is part of the source value range.
 * - elsewise values will be offset towards 0 if the value range of the source does not fit the destination.
 * - if destination is unsigned, values will be offset to be in positive domain if necessary.
 * - if destination is floating point no scaling is done at all.
 * If dst is shorter than src, no conversion is done.
 * If src is shorter than dst a warning is send to CoreLog.
 * The conversion itself is equivalent to dst[i] = round( src[i] * scale + offset )
 * \param src data to be converted
 * \param dst target where to convert src to
 * \param size the amount of elements to be converted
 * \param scale the scaling factor
 * \param offset the offset
 */
template<typename SRC, typename DST> void numeric_convert( const SRC *src, DST *dst, size_t size, const double scale, const double offset )
{
	if ( ( scale != 1. || offset ) )
		_internal::numeric_convert_impl( src, dst, size, scale, offset );
	else
		_internal::numeric_convert_impl( src, dst, size );
}
template<typename T> void numeric_copy( const T *src, T *dst, size_t size )
{
	_internal::numeric_copy_impl<T>( src, dst, size );
}

}
}


#endif // NUMERIC_CONVERT_HPP
