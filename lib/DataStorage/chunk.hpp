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

#include "typeptr.hpp"
#include "CoreUtils/log.hpp"
#include "CoreUtils/propmap.hpp"
#include "common.hpp"
#include <string.h>
#include <list>
#include "ndimensional.hpp"
#include "CoreUtils/vector.hpp"

#include <boost/numeric/ublas/matrix.hpp>

namespace isis
{
namespace data
{

class Chunk;
typedef std::list<Chunk> ChunkList;


namespace _internal
{
class ChunkBase : public NDimensional<4>, public util::PropMap
{
protected:
	static const char *needed;
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
class Chunk : public _internal::ChunkBase, protected _internal::TypePtrBase::Reference
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
	template<typename TYPE, typename D> Chunk( TYPE *src, D d, size_t firstDim, size_t secondDim = 1, size_t thirdDim = 1, size_t fourthDim = 1 ):
		_internal::ChunkBase( firstDim, secondDim, thirdDim, fourthDim ),
		util::_internal::TypeReference<_internal::TypePtrBase>( new TypePtr<TYPE>( src, volume(), d ) ) {}
	Chunk( const _internal::TypePtrBase::Reference &src, size_t firstDim, size_t secondDim = 1, size_t thirdDim = 1, size_t fourthDim = 1 );
public:
	/**
	 * Gets a reference to the element at a given index.
	 * If index is invalid, behaviour is undefined. Most probably it will crash.
	 * If _ENABLE_DATA_DEBUG is true an error message will be send (but it will _not_ stop).
	 */
	template<typename TYPE> TYPE &voxel( size_t firstDim, size_t secondDim = 0, size_t thirdDim = 0, size_t fourthDim = 0 ) {
		const size_t idx[] = {firstDim, secondDim, thirdDim, fourthDim};
		LOG_IF( ! rangeCheck( idx ), Debug, isis::error )
				<< "Index " << util::ivector4( firstDim, secondDim, thirdDim, fourthDim )
				<< " is out of range " << sizeToString();
		TypePtr<TYPE> &ret = asTypePtr<TYPE>();
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

		const TypePtr<TYPE> &ret = getTypePtr<TYPE>();

		return ret[dim2Index( idx )];
	}
	_internal::TypePtrBase &asTypePtrBase() {
		return operator*();
	}
	const _internal::TypePtrBase &getTypePtrBase()const {
		return operator*();
	}
	template<typename TYPE> TypePtr<TYPE> &asTypePtr() {
		return asTypePtrBase().cast_to_TypePtr<TYPE>();
	}
	template<typename TYPE> const TypePtr<TYPE> getTypePtr()const {
		return getTypePtrBase().cast_to_TypePtr<TYPE>();
	}
	Chunk cloneToMem( size_t firstDim = 0, size_t secondDim = 0, size_t thirdDim = 0, size_t fourthDim = 0 )const;
	Chunk copyToMem()const;
	size_t bytes_per_voxel()const;
	std::string typeName()const;
	unsigned short typeID()const;

	void copyRange( const size_t source_start[], const size_t source_end[], Chunk &dst, const size_t destination[] )const;
	void copyLine( size_t secondDimS, size_t thirdDimS, size_t fourthDimS, Chunk &dst, size_t secondDimD, size_t thirdDimD, size_t fourthDimD )const;
	void copySlice( size_t thirdDimS, size_t fourthDimS, isis::data::Chunk &dst, size_t thirdDimD, size_t fourthDimD )const;

	size_t cmpRange( size_t start, size_t end, const Chunk &dst, size_t destination )const;
	size_t cmpRange( const size_t source_start[], const size_t source_end[], const Chunk &dst, const size_t destination[] )const;
	size_t cmpLine( size_t secondDimS, size_t thirdDimS, size_t fourthDimS, const Chunk &dst, size_t secondDimD, size_t thirdDimD, size_t fourthDimD )const;
	size_t cmpSlice( size_t thirdDimS, size_t fourthDimS, const Chunk &dst, size_t thirdDimD, size_t fourthDimD )const;

	void getMinMax( util::_internal::TypeBase::Reference &min, util::_internal::TypeBase::Reference &max )const;
	template<typename T> void convertTo( T *dst, size_t len )const {
		getTypePtrBase().convertTo( dst );
	}

	Chunk &operator=( const Chunk &ref );

	/**
	 * Splices the chunk at the uppermost dimension and automatically sets indexOrigin and acquisitionNumber appropriately.
	 * This automatically selects the upermost dimension of the chunk to be spliced and will compute the correct offsets
	 * for indexOrigin and acquisitionNumberOffset which will be applied to the resulting splices.
	 * E.g. splice\(1\) on a chunk of the size 512x512x128, the readVec 1,0,0, the phaseVec 0,1,0 and the indexOrigin 0,0,0 
	 * will result in 128 chunks of the size 512x512x1, the readVec 1,0,0, the phaseVec 0,1,0 and the indexOrigin 0,0,0 to 0,0,128.
	 * (If voxelSize is 1,1,1 and voxelGap is 0,0,0)
	 */
	ChunkList autoSplice(int32_t acquisitionNumberOffset=1);
	
	/**
	 * Splices the chunk at the given dimension and all dimensions above.
	 * As this will not set or use any property
	 * - they have to be modified afterwards
	 * - this can be done on chunks without any property (aka invalid Chunks)
	 * E.g. splice\(phaseDim\) on a chunk of the size 512x512x128 will result in 512*128 chunks of the size 512x1x1
	 */
	ChunkList splice( dimensions atDim );

	/**
	 * Transforms the image coordinate system into an other system by multiplying
	 * the orientation matrix with a user defined transformation matrix. Additionally,
	 * the index origin will be transformed into the new coordinate system. This
	 * function only changes the
	 *
	 * <b>IMPORTANT!<\b>: If you call this function with a matrix other than the
	 * identidy matrix, it's not guaranteed that the image is still in ISIS space
	 * according to the DICOM conventions. Eventuelly some ISIS algorithms that
	 * depend on correct image orientations won't work as expected. Use this method
	 * with caution!
	 */
	void transformCoords( boost::numeric::ublas::matrix<float> transform );

	/**
	 * Swaps the image along a dimension dim in image space. If convertTransform is true,
	 * the transform will be converted in a way that the image is the same in physical space
	 * as it was prior to swapping.
	 */

	bool swapAlong( Chunk &, const size_t dim = 0, bool convertTransform = true ) const;

};

/// Chunk class for memory-based buffers
template<typename TYPE> class MemChunk : public Chunk
{
public:
	/// Create an empty MemChunk with the given size
	MemChunk( size_t firstDim, size_t secondDim = 1, size_t thirdDim = 1, size_t fourthDim = 1 ):
		Chunk(
			( TYPE * )calloc( fourthDim *thirdDim *secondDim *firstDim, sizeof( TYPE ) ),
			typename TypePtr<TYPE>::BasicDeleter(),
			firstDim, secondDim, thirdDim, fourthDim
		) {}
	/**
	 * Create a MemChunk as copy of a given raw memory block
	 * This will create a MemChunk of the given size and fill it with the data at the given address.
	 * No range check will be done.
	 * \param org pointer to the raw data which shall be copied
	 * \param firstDim
	 * \param secondDim
	 * \param thirdDim
	 * \param fourthDim size of the resulting image
	 */
	MemChunk( const TYPE *const org, size_t firstDim, size_t secondDim = 1, size_t thirdDim = 1, size_t fourthDim = 1 ):
		Chunk(
			( TYPE * )malloc( sizeof( TYPE )*fourthDim *thirdDim *secondDim *firstDim ),
			typename TypePtr<TYPE>::BasicDeleter(),
			firstDim, secondDim, thirdDim, fourthDim
		) {
		asTypePtr<TYPE>().copyFromMem( org, volume() );
	}
	/// Create a deep copy of a given Chunk (automatic conversion will be used if datatype does not fit)
	MemChunk( const Chunk &ref ): Chunk( ref ) {
		operator=( ref );
	}
	/**
	 * Create a deep copy of a given Chunk.
	 * An automatic conversion used if datatype does not fit
	 * \param ref the source chunk
	 * \param min
	 * \param max the value range of the source to used when the scaling for the conversion is computed
	 */
	MemChunk( const Chunk &ref, const util::_internal::TypeBase &min, const  util::_internal::TypeBase &max ): Chunk( ref ) {
		_internal::ChunkBase::operator=( static_cast<const _internal::ChunkBase &>( ref ) ); //copy the metadate of ref
		LOG( Debug, verbose_info )
				<< "Copy converting a Chunk of size " << util::MSubject( ref.sizeToVector() )
				<< " and type " << ref.typeName() << " with the properties " << static_cast<const util::PropMap &>( ref ).getKeys();
		//get rid of my TypePtr and make a new copying/converting the data of ref (use the reset-function of the scoped_ptr Chunk is made of)
		_internal::TypePtrBase::Reference::reset( new TypePtr<TYPE>( ref.getTypePtrBase().copyToNew<TYPE>( min, max ) ) );
	}
	/// Create a deep copy of a given MemChunk of the same type (default copy constructor)
	MemChunk( const MemChunk<TYPE> &ref ): Chunk( ref ) {
		operator=( ref );
	}
	MemChunk &operator=( const MemChunk<TYPE> &ref ) {
		_internal::ChunkBase::operator=( static_cast<const _internal::ChunkBase &>( ref ) ); //copy the metadate of ref
		//get rid of my TypePtr and make a new copying the data of ref (use the reset-function of the scoped_ptr Chunk is made of)
		_internal::TypePtrBase::Reference::reset( new TypePtr<TYPE>(
					static_cast<const Chunk &>( ref ).getTypePtrBase().copyToMem()->cast_to_TypePtr<TYPE>()
				) );
		return *this;
	}
	MemChunk &operator=( const Chunk &ref ) {
		_internal::ChunkBase::operator=( static_cast<const _internal::ChunkBase &>( ref ) ); //copy the metadate of ref
		//get rid of my TypePtr and make a new copying/converting the data of ref (use the reset-function of the scoped_ptr Chunk is made of)
		_internal::TypePtrBase::Reference::reset( new TypePtr<TYPE>( ref.getTypePtrBase().copyToNew<TYPE>() ) );
		return *this;
	}
};
}
}
#endif // CHUNK_H
