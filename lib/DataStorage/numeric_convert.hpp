#ifndef NUMERIC_CONVERT_HPP
#define NUMERIC_CONVERT_HPP

#include <limits>
#include <assert.h>
#include <boost/numeric/conversion/converter.hpp>
#include "common.hpp"
#include "typeptr.hpp"

namespace isis
{
namespace data
{
namespace _internal
{
template<typename SRC, typename DST> static void numeric_convert_impl( const SRC *src, DST *dst, size_t count, double scale, double offset )
{
	LOG( Runtime, info )
			<< "using generic scaling convert " << TypePtr<SRC>::staticName() << "=>" << TypePtr<DST>::staticName()
			<< " with scale/offset " << std::fixed << scale << "/" << offset;
	static boost::numeric::converter <
	DST, double,
	   boost::numeric::conversion_traits<DST, double>,
	   boost::numeric::def_overflow_handler,
	   boost::numeric::RoundEven<double>
	   > converter;

	for ( size_t i = 0; i < count; i++ ) {
		dst[i] = converter( src[i] * scale + offset );
	}
}
template<typename SRC, typename DST> static void numeric_convert_impl( const SRC *src, DST *dst, size_t count )
{
	LOG( Runtime, info ) << "using generic convert " << TypePtr<SRC>::staticName() << " => " << TypePtr<DST>::staticName() << " without scaling";
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

enum autoscaleOption {noscale, autoscale, noupscale, upscale};

/**
 * Converts data from 'src' to the type of 'dst' and stores them there.
 * If the value range defined by min and max does not fit into the domain of dst they will be scaled using the following rules:
 * - scale "around 0" if 0 is part of the source value range.
 * - elsewise values will be offset towards 0 if the value range of the source does not fit the destination.
 * - if destination is unsigned, values will be offset to be in positive domain if necessary.
 * - if destination is floating point no scaling is done at all.
 * If dst is shorter than src, no conversion is done.
 * If src is shorter than dst a warning is send to CoreLog.
 * \param src data to be converted
 * \param dst target where to convert src to
 * \param min lowest value in src
 * \param max highest value in src
 * \param scaleopt enum to tweak the scaling strategy
 */
template<typename SRC, typename DST> void numeric_convert( const TypePtr<SRC> &src, TypePtr<DST> &dst, const util::_internal::TypeBase &min, const util::_internal::TypeBase &max, autoscaleOption scaleopt = autoscale )
{
	LOG_IF( src.len() > dst.len(), Runtime, error ) << "The " << src.len() << " elements of src wont fit into the destination. Will only convert " << dst.len() << " elements.";
	LOG_IF( src.len() < dst.len(), Runtime, warning ) << "Source is shorter than destination. Will only convert " << src.len() << " values";

	if ( src.len() == 0 )return;

	double scale = 1.0;
	double offset = 0.0;
	size_t size = std::min( src.len(), dst.len() );
	bool doScale = ( scaleopt != noscale && std::numeric_limits<DST>::is_integer ); //only do scale if scaleopt!=noscale and the target is an integer (scaling into float is useless)

	if ( scaleopt == autoscale && std::numeric_limits<SRC>::is_integer ) {
		LOG( Debug, verbose_info ) << "Won't upscale, because the source datatype is discrete (" << util::Type<SRC>::staticName() << ")";
		scaleopt = noupscale; //dont scale up if SRC is an integer
	}

	if ( doScale ) {
		const DST domain_min = std::numeric_limits<DST>::min();//negative value domain of this dst
		const DST domain_max = std::numeric_limits<DST>::max();//positive value domain of this dst
		double minval, maxval;
		LOG_IF( min.typeID() != util::Type<SRC>::staticID, Debug, info )
				<< "The given minimum for src Range is not of the same type as the data ("  << min.typeName() << "!=" << util::Type<SRC>::staticName() << ")";
		LOG_IF( max.typeID() != util::Type<SRC>::staticID, Debug, info )
				<< "The given maximum for src Range is not of the same type as the data ("  << max.typeName() << "!=" << util::Type<SRC>::staticName() << ")";
		minval = min.as<double>();
		maxval = max.as<double>();
		assert( minval < maxval );
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
			range_max != 0 ? domain_max / range_max :
			std::numeric_limits<double>::max();
		const double scale_min =
			range_min != 0 ? domain_min / range_min :
			std::numeric_limits<double>::max();
		LOG( Debug, info ) << "scale_min/scale_max=" << std::fixed << scale_min << "/" << scale_max;
		scale = std::min( scale_max ? scale_max : std::numeric_limits<double>::max(), scale_min ? scale_min : std::numeric_limits<double>::max() );//get the smaller scaling factor which is not zero so the bigger range will fit into his domain

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
		_internal::numeric_convert_impl( &src[0], &dst[0], size, scale, offset );
	else
		_internal::numeric_convert_impl( &src[0], &dst[0], size );
}

}
}


#endif // NUMERIC_CONVERT_HPP
