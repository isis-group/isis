//
// C++ Interface: ndimensional
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef NDIMENSIONAL_H
#define NDIMENSIONAL_H

#define __need_size_t
#include <stddef.h>
#include <algorithm>
#include <boost/static_assert.hpp>
#include <string>
#include "DataStorage/common.hpp"
#include "CoreUtils/vector.hpp"

namespace isis
{
namespace data
{
namespace _internal
{
/// @cond _hidden

template<unsigned short DIM> size_t __dimStride( const size_t dim[] )
{
	return __dimStride < DIM - 1 > ( dim ) * dim[DIM-1];
}

template<unsigned short DIM> size_t __dim2index( const size_t d[], const size_t dim[] )
{
	return d[DIM] * __dimStride<DIM>( dim ) + __dim2index < DIM - 1 > ( d, dim );
}

template<unsigned short DIM> bool __rangeCheck( const size_t d[], const size_t dim[] )
{
	return ( d[DIM] < dim[DIM] ) && __rangeCheck < DIM - 1 > ( d, dim );
}

template<> inline size_t __dimStride<0>( const size_t dim[] ) {return 1;}
template<> inline size_t __dim2index<0>( const size_t d[], const size_t dim[] ) {return d[0] * __dimStride<0>( dim );}
template<> inline bool   __rangeCheck<0>( const size_t d[], const size_t dim[] ) {return d[0] < dim[0];}

/// @endcond

/// Base class for anything that has dimensional size
template<unsigned short DIMS> class NDimensional
{
	size_t dim[DIMS];
protected:
	NDimensional() {}
public:
	static const size_t n_dims = DIMS;
	/**
	 * Initializes the size-vector.
	 * This must be done before anything else, or behaviour will be undefined.
	 * \param d array with sizes to use. (d[0] is most iterating element / lowest dimension)
	 */
	void init( const size_t d[DIMS] ) {
		std::copy( d, d + DIMS, dim );
		LOG_IF( volume() == 0, Runtime, error ) << "Creating object with volume of 0";
	}
	void init( const util::FixedVector<size_t, DIMS>& d ) {
		d.copyTo( dim );
		LOG_IF( volume() == 0, Runtime, error ) << "Creating object with volume of 0";
	}
	NDimensional( const NDimensional &src ) {//@todo default copier should do the job
		init( src.dim );
	}
	/**
	 * Compute linear index from n-dimensional index,
	 * \param d array of indexes (d[0] is most iterating element / lowest dimension)
	 */
	size_t getLinearIndex( const size_t d[DIMS] )const {
		return __dim2index < DIMS - 1 > ( d, dim );
	}
	/**
	 * Check if index fits into the dimensional size of the object.
	 * \param d index to be checked (d[0] is most iterating element / lowest dimension)
	 * \returns true if given index will get a reasonable result when used for getLinearIndex
	 */
	bool isInRange( const size_t d[DIMS] )const {
		return __rangeCheck < DIMS - 1 > ( d, dim );
	}
	/**
	 * Get the size of the object in elements of TYPE.
	 * \returns \f$ \prod_{i=0}^{DIMS-1} dimSize(i) \f$
	 */
	size_t volume()const {
		return __dimStride<DIMS>( dim );
	}
	///\returns the size of the object in the given dimension
	size_t dimSize( size_t idx )const {
		return dim[idx];
	}

	/// generates a string representing the size
	std::string getSizeAsString( std::string delim = "x" )const {
		return util::list2string( dim, dim + DIMS, delim );
	}

	/// generates a FixedVector\<DIMS\> representing the size
	util::FixedVector<size_t, DIMS> sizeToVector()const {
		return util::FixedVector<size_t, DIMS>( dim );
	}

	/**
	 * get amount of relevant dimensions (last dim with size>1)
	 * e.g. on a slice (64x64x1x1) it will be 2
	 */
	size_t relevantDims()const {
		size_t ret = 0;

		for ( unsigned short i = DIMS; i; i-- ) {
			if ( dim[i-1] > 1 ) {
				ret = i;
				break;
			}
		}

		return ret;
	}
	util::FixedVector<float, DIMS> getFoV( const util::FixedVector<float, DIMS> &voxelSize, const util::FixedVector<float, DIMS> &voxelGap )const {
		DISABLE_WARN( 4244 );
		LOG_IF( volume() == 0, DataLog, warning ) << "Calculating FoV of empty data";
		const util::FixedVector<size_t, DIMS> voxels = sizeToVector();
		const util::fvector4 gapSize = voxelGap * ( voxels - 1 );
		return voxelSize * voxels + gapSize;
	}
};

}
}
}

#endif // NDIMENSIONAL_H
