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
#include "../util/progressfeedback.hpp"

namespace isis
{
namespace data
{

/// Base class for anything that has dimensional size
template<unsigned short DIMS> class NDimensional
{
	std::array<size_t,DIMS> m_dim;
	constexpr size_t _dimStride( unsigned short dim )const
	{
		return dim ? _dimStride( dim-1 ) * m_dim[dim - 1]:1;
	}
	constexpr size_t _dim2index( const std::array<size_t,DIMS> &d, unsigned short DIM )const
	{
		return DIM ?
			d[DIM] * _dimStride( DIM ) + _dim2index( d, DIM - 1 ):
			d[0]   * _dimStride( 0 );
	}
	constexpr std::array<size_t,DIMS> _index2dim( const size_t index, unsigned short DIM, size_t vol )const
	{
		if(DIM){
			std::array<size_t,DIMS> ret=_index2dim( index % vol, DIM - 1, vol / m_dim[DIM - 1] );
			ret[DIM] = index / vol;
			return ret;
		} else {
			assert(vol == 1);
			return {index};
		}
	}
	constexpr bool _rangeCheck( const std::array<size_t,DIMS> &d, unsigned short DIM )const
	{
		return DIM ?
			( d[DIM] < m_dim[DIM] ) && _rangeCheck( d, DIM - 1 ):
			d[0] < m_dim[0];
	}



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
		return _dim2index( coord, DIMS - 1 );
	}
	/**
	 * Compute coordinates from linear index,
	 * \param coord array to put the computed coordinates in (d[0] will be most iterating element / lowest dimension)
	 * \param index the linear index to compute the coordinates from
	 */
	std::array<size_t,DIMS> getCoordsFromLinIndex( const size_t index )const {
		return _index2dim( index, DIMS - 1, getVolume() / m_dim[DIMS - 1] );
	}
	/**
	 * Check if index fits into the dimensional size of the object.
	 * \param coord index to be checked (d[0] is most iterating element / lowest dimension)
	 * \returns true if given index will get a reasonable result when used for getLinearIndex
	 */
	bool isInRange( const std::array<size_t,DIMS> &coord )const {
		return _rangeCheck( coord, DIMS - 1 );
	}
	/**
	 * Get the size of the object in elements of TYPE.
	 * \returns \f$ \prod_{i=0}^{DIMS-1} getDimSize(i) \f$
	 */
	size_t getVolume()const {
		return _dimStride( DIMS );
	}
	///\returns the size of the object in the given dimension
	size_t getDimSize( size_t idx )const {
		return m_dim[idx];
	}

	/// generates a string representing the size
	std::string getSizeAsString( const char *delim = "x" )const {
		return util::listToString( std::begin(m_dim), std::end(m_dim), delim, "", "" );
	}

	/// generates a FixedVector\<DIMS\> representing the size
	std::array<size_t, DIMS> getSizeAsVector()const {
		return std::array<size_t, DIMS>( m_dim );
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
	std::array<float, DIMS> getFoV( const std::array<float, DIMS> &voxelSize, const std::array<float, DIMS> &voxelGap )const {
		LOG_IF( getVolume() == 0, DataLog, warning ) << "Calculating FoV of empty data";
		std::array<size_t, DIMS> voxels = getSizeAsVector();
		for(size_t &v:voxels)
			--v;

		const std::array<float, DIMS> gapSize = voxelGap * voxels;
		return voxelSize * voxels + gapSize;
	}
	
	template<typename ITER> void swapDim(size_t dim_a,size_t dim_b,ITER at, std::shared_ptr<util::ProgressFeedback> feedback=std::shared_ptr<util::ProgressFeedback>()){
		std::vector<bool> visited(getVolume());
		data::NDimensional<DIMS> oldshape=*this;

		//reshape myself
		std::swap(m_dim[dim_a],m_dim[dim_b]);
		ITER cycle = at,last=cycle+getVolume();
		if(feedback)
			feedback->show(getVolume()-1, std::string("Swapping ")+std::to_string(getVolume())+" voxels");

		while(++cycle != last){
			if(feedback) 
				feedback->progress();
			size_t i=cycle-at;
			if(visited[i])continue;
			
			do{
				std::array<size_t,DIMS> currIndex=oldshape.getCoordsFromLinIndex(i);
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

#endif // NDIMENSIONAL_H
