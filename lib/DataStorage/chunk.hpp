//
// C++ Interface: chunk
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef CHUNK_H
#define CHUNK_H

#include "CoreUtils/type.hpp"
#include "CoreUtils/log.hpp"
#include "CoreUtils/propmap.hpp"
#include "common.hpp"
#include <string.h>
#include <list>
#include "ndimensional.hpp"
#include "CoreUtils/vector.hpp"

namespace isis
{
namespace data
{

namespace _internal
{
class ChunkBase : public NDimensional<4>, public util::PropMap
{
protected:
	static const char* needed;
public:
//  static const dimensions dimension[n_dims]={readDim,phaseDim,sliceDim,timeDim};
	typedef isis::util::_internal::TypeReference <ChunkBase > Reference;

	ChunkBase( size_t firstDim, size_t secondDim, size_t thirdDim, size_t fourthDim );
	virtual ~ChunkBase(); //needed to make it polymorphic
};
}

/**
 * Main class for four-dimensional random-access data blocks.
 * Like in TypePtr, the copy of a Chunk will reference the same data.
 * (If you want to make a memory based deep copy of a Chunk create a MemChunk from it)
 */
class Chunk : public _internal::ChunkBase, protected util::_internal::TypePtrBase::Reference
{
protected:
	/**
	 * Creates an data-block from existing data.
	 * \param src is a pointer to the existing data. This data will automatically be deleted. So don't use this pointer afterwards.
	 * \param d is the deleter to be used for deletion of src. It must define operator(TYPE *), which than shall free the given pointer.
	 * \param firstDim size in the first dimension (usually read-encoded dim)
	 * \param secondDim size in the second dimension (usually phase-encoded dim)
	 * \param thirdDim size in the third dimension (usually slice-encoded dim)
	 * \param fourthDim size in the fourth dimension
	 */
	template<typename TYPE, typename D> Chunk( TYPE* src, D d, size_t firstDim, size_t secondDim = 1, size_t thirdDim = 1, size_t fourthDim = 1 ):
			_internal::ChunkBase( firstDim, secondDim, thirdDim, fourthDim ),
			util::_internal::TypeReference<util::_internal::TypePtrBase>( new util::TypePtr<TYPE>( src, volume(), d ) ) {}
	Chunk( const util::_internal::TypePtrBase::Reference &src, size_t firstDim, size_t secondDim = 1, size_t thirdDim = 1, size_t fourthDim = 1 );
public:
	/**
	 * Gets a reference to the element at a given index.
	 * If index is invalid, behaviour is undefined. Most probably it will crash.
	 * If _ENABLE_DATA_DEBUG is true an error message will be send (but it will _not_ stop).
	 */
	template<typename TYPE> TYPE &voxel( size_t firstDim, size_t secondDim = 0, size_t thirdDim = 0, size_t fourthDim = 0 ) {
		const size_t idx[] = {firstDim, secondDim, thirdDim, fourthDim};
		LOG_IF( not rangeCheck( idx ), Debug, isis::error )
		<< "Index " << util::ivector4( firstDim, secondDim, thirdDim, fourthDim )
		<< " is out of range " << sizeToString();
		util::TypePtr<TYPE> &ret = asTypePtr<TYPE>();
		return ret[dim2Index( idx )];
	}
	/**
	 * Gets a copy of the element at a given index.
	 * \copydetails Chunk::voxel
	 */
	template<typename TYPE> TYPE voxel( size_t firstDim, size_t secondDim = 0, size_t thirdDim = 0, size_t fourthDim = 0 )const {
		const size_t idx[] = {firstDim, secondDim, thirdDim, fourthDim};

		if ( !rangeCheck( idx ) ) {
			LOG( Debug, isis::error )
			<< "Index " << firstDim << "|" << secondDim << "|" << thirdDim << "|" << fourthDim
			<< " is out of range (" << sizeToString() << ")";
		}

		const util::TypePtr<TYPE> &ret = getTypePtr<TYPE>();

		return ret[dim2Index( idx )];
	}
	util::_internal::TypePtrBase& asTypePtrBase() {
		return operator*();
	}
	const util::_internal::TypePtrBase& getTypePtrBase()const {
		return operator*();
	}
	template<typename TYPE> util::TypePtr<TYPE> &asTypePtr() {
		return asTypePtrBase().cast_to_TypePtr<TYPE>();
	}
	template<typename TYPE> const util::TypePtr<TYPE> getTypePtr()const {
		return getTypePtrBase().cast_to_TypePtr<TYPE>();
	}
	Chunk cloneToMem( size_t firstDim = 0, size_t secondDim = 0, size_t thirdDim = 0, size_t fourthDim = 0 )const;
	Chunk copyToMem()const;
	size_t bytes_per_voxel()const;
	std::string typeName()const;
	unsigned short typeID()const;

	void copyRange( const size_t source_start[], const size_t source_end[], Chunk& dst, const size_t destination[] )const;
	void copyLine( size_t secondDimS, size_t thirdDimS, size_t fourthDimS, Chunk& dst, size_t secondDimD, size_t thirdDimD, size_t fourthDimD )const;
	void copySlice( size_t thirdDimS, size_t fourthDimS, isis::data::Chunk& dst, size_t thirdDimD, size_t fourthDimD )const;

	size_t cmpRange( size_t start, size_t end, const Chunk& dst, size_t destination )const;
	size_t cmpRange( const size_t source_start[], const size_t source_end[], const Chunk& dst, const size_t destination[] )const;
	size_t cmpLine( size_t secondDimS, size_t thirdDimS, size_t fourthDimS, const Chunk& dst, size_t secondDimD, size_t thirdDimD, size_t fourthDimD )const;
	size_t cmpSlice( size_t thirdDimS, size_t fourthDimS, const Chunk& dst, size_t thirdDimD, size_t fourthDimD )const;

	template<typename T> void getMinMax( T &min, T &max )const {
		return operator*().getMinMax( min, max );
	}
	template<typename T> size_t convertTo( T *dst, size_t len )const {
		getTypePtrBase().convertTo( dst );
	}
};

/// @cond _internal
namespace _internal
{

struct chunk_comparison : public std::binary_function< Chunk, Chunk, bool> {
	virtual bool operator() ( const Chunk& a, const Chunk& b )const = 0;
	virtual ~chunk_comparison() {}
};
}
/// @endcond

typedef std::list<Chunk> ChunkList;

/// Chunk class for memory-based buffers
template<typename TYPE> class MemChunk : public Chunk
{
public:
	/// Create an empty MemChunk with the given size
	MemChunk( size_t firstDim, size_t secondDim = 1, size_t thirdDim = 1, size_t fourthDim = 1 ):
			Chunk(
				( TYPE* )calloc( fourthDim*thirdDim*secondDim*firstDim, sizeof( TYPE ) ),
				typename ::isis::util::TypePtr<TYPE>::BasicDeleter(),
				firstDim, secondDim, thirdDim, fourthDim
			) {}
	/// Create a MemChunk as copy of a given raw memory block (no range check will be done)
	MemChunk( const TYPE*const org, size_t firstDim, size_t secondDim = 1, size_t thirdDim = 1, size_t fourthDim = 1 ):
			Chunk(
				( TYPE* )malloc( sizeof( TYPE )*fourthDim*thirdDim*secondDim*firstDim ),
				typename ::isis::util::TypePtr<TYPE>::BasicDeleter(),
				firstDim, secondDim, thirdDim, fourthDim
			) {
		asTypePtr<TYPE>().copyFromMem( org, volume() );
	}
	/// Create a deep copy of a given Chunk (automatic conversion will be used if datatype does not fit)
	MemChunk( const Chunk &ref ): Chunk( ref ) {
		//get rid of ref's TypePtr and make my own from it  (use the reset-function of the scoped_ptr Chunk is made of)
		util::_internal::TypePtrBase::Reference::reset( new util::TypePtr<TYPE>( ref.getTypePtrBase().copyToNew<TYPE>() ) );
	}
};
}
}
#endif // CHUNK_H
