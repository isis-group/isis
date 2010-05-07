#ifndef NUMERIC_CONVERT_HPP
#define NUMERIC_CONVERT_HPP

#include <limits>
#include <assert.h>
#include <boost/numeric/conversion/converter.hpp>
#include "common.hpp"
#include "type.hpp"

namespace isis
{
namespace util
{
namespace _internal
{
template<typename SRC, typename DST> static void numeric_convert_impl( const SRC *src, DST *dst, size_t count, double scale, double offset )
{
	LOG( Debug, info )
			<< "using generic scaling convert " << Type<SRC>::staticName() << "=>" << Type<DST>::staticName()
			<< " with scale/offset " << scale << "/" << offset;
	static boost::numeric::converter <
	DST, double,
	   boost::numeric::conversion_traits<DST, double>,
	   boost::numeric::def_overflow_handler,
	   boost::numeric::RoundEven<double>
	   > converter;

	for ( size_t i = 0; i < count; i++ )
		dst[i] = converter( src[i] * scale + offset );
}
template<typename SRC, typename DST> static void numeric_convert_impl( const SRC *src, DST *dst, size_t count )
{
	LOG( Debug, info ) << "using generic convert " << Type<SRC>::staticName() << " => " << Type<DST>::staticName() << " without scaling";
	static boost::numeric::converter <
	DST, SRC,
	   boost::numeric::conversion_traits<DST, SRC>,
	   boost::numeric::def_overflow_handler,
	   boost::numeric::RoundEven<SRC>
	   > converter;

	for ( size_t i = 0; i < count; i++ )
		dst[i] = converter( src[i] );
}
}

enum autoscaleOption {noscale, autoscale, noupscale};

/**
 * Converts data from 'src' to the type of 'dst' and stores them there.
 * If the value range of src does not fit into the domain of dst they will be scaled using the following rules:
 * - will scale "around 0" if 0 is part of the source value range.
 * - elsewise values will be offset towards 0.
 * - if destination is unsigned values will be offset to be in positive domain if necessary.
 * - if destination is floating point no scaling is done at all.
 * If dst is shorter than src, no conversion is done.
 * If src is shorter than dst a warning is send to CoreLog.
 */
template<typename SRC, typename DST> void numeric_convert( const TypePtr<SRC> &src, TypePtr<DST> &dst, const _internal::TypeBase &min, const _internal::TypeBase &max, autoscaleOption scaleopt = noupscale )
{
	if ( src.len() > dst.len() ) {
		LOG( Runtime, error ) << "The " << src.len() << " elements of src wont fit into the destination with the size " << dst.len();
		return;
	}

	LOG_IF( src.len() < dst.len(), Runtime, warning ) << "Source is shorter than destination. Will only convert " << src.len() << " values";

	if ( src.len() == 0 )return;

	double scale = 1.0;
	double offset = 0.0;
	size_t srcsize = src.len();
	bool doScale = ( scaleopt != noscale && std::numeric_limits<DST>::is_integer );

	if ( doScale ) {
		const DST domain_min = std::numeric_limits<DST>::min();//negative value domain of this dst
		const DST domain_max = std::numeric_limits<DST>::max();//positive value domain of this dst
		Type<SRC> minval, maxval;

		if( min > max ) {
			LOG( Debug, info ) << "Computing source range on my own";
			src.getMinMax( minval, maxval );
		} else {
			LOG_IF( min.typeID() != Type<SRC>::staticID, Debug, info )
					<< "The given minimum for src Range is not of the same type as the data ("
					<< min.typeName() << "!=" << Type<SRC>::staticName() << ")";
			LOG_IF( max.typeID() != Type<SRC>::staticID, Debug, info )
					<< "The given maximum for src Range is not of the same type as the data ("
					<< max.typeName() << "!=" << Type<SRC>::staticName() << ")";
			minval = min.as<SRC>();
			maxval = max.as<SRC>();
		}

		LOG( Debug, info ) << "src Range:" << minval << "=>" << maxval;
		LOG( Debug, info ) << "dst Domain:" << domain_min << "=>" << domain_max;
		assert( domain_min < domain_max );//we also should assume this
		//set offset for src
		//if all src is completly on positive domain, or if there is no negative domain use minval
		//else if src is completly on negative dmain, or if there is no positive domain use maxval
		//elsewise leave it at 0 and scale both sides
		if ( minval > 0 || !domain_min ) {
			if ( ( maxval - domain_max ) > 0 ) // if the values completely fit into the domain we dont have to offset them
				offset = -minval;
		} else if ( ( 0 - maxval ) > 0 || !domain_max ) {
			if ( ( domain_min - minval ) > 0  ) // if the values completely fit into the domain we dont have to offset them
				offset = -maxval;
		}

		//calculate range of values which will be on postive/negative domain when offset is applied
		const double range_max = maxval + offset; //allways >=0
		const double range_min = minval + offset; //allways <=0
		//set scaling factor to fit src-range into dst domain
		//some compilers dont make x/0 = inf, so we use std::numeric_limits<double>::max() instead, in this case
		const double scale_max =
			range_max ? domain_max / range_max :
			std::numeric_limits<double>::max();
		const double scale_min =
			range_min ? domain_min / range_min :
			std::numeric_limits<double>::max();
		LOG( Debug, info ) << "scale_min/scale_max=" << scale_min << "/" << scale_max;
		scale = std::min( scale_max ? : std::numeric_limits<double>::max(), scale_min ? : std::numeric_limits<double>::max() );//get the smaller scaling factor which is not zero so the bigger range will fit into his domain

		if ( scale < 1 ) {
			LOG( Runtime, warning ) << "Downscaling your values by Factor " << scale << " you might lose information.";
		} else if ( scaleopt == noupscale ) {
			if ( scale > 1 ) {
				LOG( Runtime, info ) << "upscale not given, clamping scale " << scale << " to 1";
				scale = 1;
			}
		}

		doScale = ( scale != 1. || offset );
		offset *= scale;//calc offset for dst
	}

	if ( doScale )
		_internal::numeric_convert_impl( &src[0], &dst[0], srcsize, scale, offset );
	else
		_internal::numeric_convert_impl( &src[0], &dst[0], srcsize );
}

}
}


#endif // NUMERIC_CONVERT_HPP
