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
#include <string>
#include "common.hpp"
#include "../util/vector.hpp"

namespace isis
{
namespace data
{
namespace _internal
{
/// @cond _internal

template<unsigned short DIM> size_t __dimStride( const size_t dim[] )
{
	return __dimStride < DIM - 1 > ( dim ) * dim[DIM - 1];
}

template<unsigned short DIM> size_t __dim2index( const size_t d[], const size_t dim[] )
{
	return d[DIM] * __dimStride<DIM>( dim ) + __dim2index < DIM - 1 > ( d, dim );
}

template<unsigned short DIM> void __index2dim( const size_t index, size_t d[], const size_t dim[], size_t vol )
{
	d[DIM] = index / vol;
	__index2dim < DIM - 1 > ( index % vol, d, dim, vol / dim[DIM - 1] );
}

template<unsigned short DIM> bool __rangeCheck( const size_t d[], const size_t dim[] )
{
	return ( d[DIM] < dim[DIM] ) && __rangeCheck < DIM - 1 > ( d, dim );
}

template<> inline size_t __dimStride<0>( const size_t[] /*dim*/ ) {return 1;}
template<> inline size_t __dim2index<0>( const size_t d[], const size_t dim[] ) {return d[0] * __dimStride<0>( dim );}
template<> inline void __index2dim<0>( const size_t index, size_t d[], const size_t[], size_t /*vol*/ ) {d[0] = index;}
template<> inline bool   __rangeCheck<0>( const size_t d[], const size_t dim[] ) {return d[0] < dim[0];}

/// @endcond

/// Base class for anything that has dimensional size
template<unsigned short DIMS> class NDimensional
{
	std::array<size_t,DIMS> m_dim;
protected:
	static constexpr size_t dims = DIMS;
	NDimensional() {}
public:
	/**
	 * Initializes the size-vector.
	 * This must be done before anything else, or behaviour will be undefined.
	 * \param d array with sizes to use. (d[0] is most iterating element / lowest dimension)
	 */
	void init(const std::array<size_t,DIMS> &d ) {
		m_dim = d;
		LOG_IF( getVolume() == 0, Runtime, error ) << "Creating object with volume of 0";
	}
	NDimensional( const NDimensional &src ) {//@todo default copier should do the job
		init( src.m_dim );
	}
	/**
	 * Compute linear index from n-dimensional index,
	 * \param coord array of indexes (d[0] is most iterating element / lowest dimension)
	 */
	size_t getLinearIndex( const std::array<size_t,DIMS> &coord )const {
		return __dim2index < DIMS - 1 > ( coord.data(), m_dim.data() );
	}
	/**
	 * Compute coordinates from linear index,
	 * \param coord array to put the computed coordinates in (d[0] will be most iterating element / lowest dimension)
	 * \param index the linear index to compute the coordinates from
	 */
	void getCoordsFromLinIndex( const size_t index, std::array<size_t,DIMS> &coord )const {
		__index2dim < DIMS - 1 > ( index, coord.data(), m_dim.data(), getVolume() / m_dim[DIMS - 1] );
	}
	/**
	 * Check if index fits into the dimensional size of the object.
	 * \param coord index to be checked (d[0] is most iterating element / lowest dimension)
	 * \returns true if given index will get a reasonable result when used for getLinearIndex
	 */
	bool isInRange( const std::array<size_t,DIMS> &coord )const {
		return __rangeCheck < DIMS - 1 > ( coord.data(), m_dim.data() );
	}
	/**
	 * Get the size of the object in elements of TYPE.
	 * \returns \f$ \prod_{i=0}^{DIMS-1} getDimSize(i) \f$
	 */
	size_t getVolume()const {
		return __dimStride<DIMS>( m_dim.data() );
	}
	///\returns the size of the object in the given dimension
	size_t getDimSize( size_t idx )const {
		return m_dim[idx];
	}

	/// generates a string representing the size
	std::string getSizeAsString( std::string delim = "x" )const {
		return util::listToString( std::begin(m_dim), std::end(m_dim), delim, "", "" );
	}

	/// generates a FixedVector\<DIMS\> representing the size
	util::FixedVector<size_t, DIMS> getSizeAsVector()const {
		return util::FixedVector<size_t, DIMS>( m_dim );
	}

	/**
	 * get amount of relevant dimensions (last dim with size>1)
	 * e.g. on a slice (1x64x1x1) it will be 2
	 */
	size_t getRelevantDims()const {
		size_t ret = 0;

		for ( unsigned short i = DIMS; i; i-- ) {
			if ( m_dim[i - 1] > 1 ) {
				ret = i;
				break;
			}
		}

		return ret;
	}
	util::FixedVector<float, DIMS> getFoV( const util::FixedVector<float, DIMS> &voxelSize, const util::FixedVector<float, DIMS> &voxelGap )const {
		LOG_IF( getVolume() == 0, DataLog, warning ) << "Calculating FoV of empty data";
		const util::FixedVector<size_t, DIMS> voxels = getSizeAsVector();
		const util::FixedVector<float, DIMS> gapSize = voxelGap * ( voxels - 1 );
		return voxelSize * voxels + gapSize;
	}
	
	template<typename ITER> void swapDim(size_t dim_a,size_t dim_b,ITER at){
		std::vector<bool> visited(getVolume());
		data::_internal::NDimensional<DIMS> oldshape=*this;

		//reshape myself
		std::swap(m_dim[dim_a],m_dim[dim_b]);
		ITER cycle = at,last=cycle+getVolume();
		std::array<size_t,DIMS> currIndex;

		while(++cycle != last){
			size_t i=cycle-at;
			if(visited[i])continue;
			
			do{
				oldshape.getCoordsFromLinIndex(i,currIndex);
				std::swap(currIndex[dim_a],currIndex[dim_b]);
				i=getLinearIndex(currIndex);
				std::swap(at[i],*cycle);
				visited[i] = true;
			} while (at+i != cycle);
		}
	}
};

}
}
}

#endif // NDIMENSIONAL_H
