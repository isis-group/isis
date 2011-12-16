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
#include "../CoreUtils/log.hpp"
#include "../CoreUtils/propmap.hpp"
#include "common.hpp"
#include <string.h>
#include <list>
#include "ndimensional.hpp"
#include "../CoreUtils/vector.hpp"

#include <boost/numeric/ublas/matrix.hpp>

namespace isis
{
namespace data
{

class Chunk;

namespace _internal
{
class ChunkBase : public NDimensional<4>, public util::PropertyMap
{
protected:
	static const char *neededProperties;
	ChunkBase() {}; //do not use this
public:
	//  static const dimensions dimension[n_dims]={rowDim,columnDim,sliceDim,timeDim};
	typedef isis::util::_internal::GenericReference<ChunkBase > Reference;

	ChunkBase( size_t nrOfColumns, size_t nrOfRows, size_t nrOfSlices, size_t nrOfTimesteps );
	virtual ~ChunkBase(); //needed to make it polymorphic
};
}

/// Base class for operators used for foreachVoxel
template <typename TYPE> class VoxelOp: std::unary_function<bool, TYPE>
{
public:
	virtual bool operator()( TYPE &vox, const util::FixedVector<size_t, 4> &pos ) = 0;
};

/**
 * Main class for four-dimensional random-access data blocks.
 * Like in ValuePtr, the copy of a Chunk will reference the same data. (cheap copy)
 * (If you want to make a memory based deep copy of a Chunk create a MemChunk from it)
 */
class Chunk : public _internal::ChunkBase, protected ValuePtrReference
{
	friend class Image;
	friend class std::vector<Chunk>;
protected:
	/**
	 * Creates an data-block from existing data.
	 * \param src is a pointer to the existing data. This data will automatically be deleted. So don't use this pointer afterwards.
	 * \param d is the deleter to be used for deletion of src. It must define operator(TYPE *), which than shall free the given pointer.
	 * \param nrOfColumns size in the first dimension (usually read-encoded dim)
	 * \param nrOfRows size in the second dimension (usually phase-encoded dim)
	 * \param nrOfSlices size in the third dimension (usually slice-encoded dim)
	 * \param nrOfTimesteps size in the fourth dimension
	 */
	template<typename TYPE, typename D> Chunk( TYPE *src, D d, size_t nrOfColumns, size_t nrOfRows = 1, size_t nrOfSlices = 1, size_t nrOfTimesteps = 1 ):
		_internal::ChunkBase( nrOfColumns, nrOfRows, nrOfSlices, nrOfTimesteps ), ValuePtrReference( ValuePtr<TYPE>( src, getVolume(), d ) ) {}

	Chunk() {}; //do not use this
public:
	Chunk( const ValuePtrReference &src, size_t nrOfColumns, size_t nrOfRows = 1, size_t nrOfSlices = 1, size_t nrOfTimesteps = 1 );

	/**
	 * Gets a reference to the element at a given index.
	 * If index is invalid, behaviour is undefined. Most probably it will crash.
	 * If _ENABLE_DATA_DEBUG is true an error message will be send (but it will _not_ stop).
	 */
	template<typename TYPE> TYPE &voxel( size_t nrOfColumns, size_t nrOfRows = 0, size_t nrOfSlices = 0, size_t nrOfTimesteps = 0 ) {
		const size_t idx[] = {nrOfColumns, nrOfRows, nrOfSlices, nrOfTimesteps};
		LOG_IF( ! isInRange( idx ), Debug, isis::error )
				<< "Index " << util::FixedVector<size_t, 4>( idx ) << " is out of range " << getSizeAsString();
		ValuePtr<TYPE> &ret = asValuePtr<TYPE>();
		return ret[getLinearIndex( idx )];
	}

	/**
	 * Gets a const reference of the element at a given index.
	 * \copydetails Chunk::voxel
	 */
	template<typename TYPE> const TYPE &voxel( size_t nrOfColumns, size_t nrOfRows = 0, size_t nrOfSlices = 0, size_t nrOfTimesteps = 0 )const {
		const size_t idx[] = {nrOfColumns, nrOfRows, nrOfSlices, nrOfTimesteps};

		if ( !isInRange( idx ) ) {
			LOG( Debug, isis::error )
					<< "Index " << nrOfColumns << "|" << nrOfRows << "|" << nrOfSlices << "|" << nrOfTimesteps
					<< " is out of range (" << getSizeAsString() << ")";
		}

		const ValuePtr<TYPE> &ret = getValuePtr<TYPE>();

		return ret[getLinearIndex( idx )];
	}

	void copySlice( size_t thirdDimS, size_t fourthDimS, Chunk &dst, size_t thirdDimD, size_t fourthDimD ) const;

	/**
	 * Run a functor on every Voxel in the chunk.
	 * If the data of the chunk are not of type TYPE, behaviour is undefined.
	 * (If _DEBUG_LOG is enabled you will get an error message before the progrmm will crash).
	 * \param op a functor inheriting from VoxelOp
	 * \param offset offset to be added to the voxel position before op is called
	 * \returns amount of operations which returned false - so 0 is good!
	 */
	template <typename TYPE> size_t foreachVoxel( VoxelOp<TYPE> &op, util::FixedVector<size_t, 4> offset ) {
		const util::FixedVector<size_t, 4> imagesize = getSizeAsVector();
		util::FixedVector<size_t, 4> pos;
		TYPE *vox = &asValuePtr<TYPE>()[0];
		size_t ret = 0;

		for( pos[timeDim] = 0; pos[timeDim] < imagesize[timeDim]; pos[timeDim]++ )
			for( pos[sliceDim] = 0; pos[sliceDim] < imagesize[sliceDim]; pos[sliceDim]++ )
				for( pos[columnDim] = 0; pos[columnDim] < imagesize[columnDim]; pos[columnDim]++ )
					for( pos[rowDim] = 0; pos[rowDim] < imagesize[rowDim]; pos[rowDim]++ ) {
						if( op( *( vox++ ), pos + offset ) == false )
							++ret;
					}

		return ret;
	}

	/**
	 * Run a functor on every Voxel in the chunk.
	 * \param op a functor inheriting from VoxelOp
	 * \returns amount of operations which returned false - so 0 is good!
	 */
	template<typename TYPE> size_t foreachVoxel( VoxelOp<TYPE> &op ) {
		return foreachVoxel<TYPE>( op, util::FixedVector<size_t, 4>() );
	}

	_internal::ValuePtrBase &asValuePtrBase() {return operator*();}
	const _internal::ValuePtrBase &getValuePtrBase()const {return operator*();}

	template<typename TYPE> ValuePtr<TYPE> &asValuePtr() {return asValuePtrBase().castToValuePtr<TYPE>();}
	template<typename TYPE> const ValuePtr<TYPE> getValuePtr()const {return getValuePtrBase().castToValuePtr<TYPE>();}

	/// \returns the number of cheap-copy-chunks using the same memory as this
	size_t useCount()const;
	Chunk cloneToNew( size_t nrOfColumns, size_t nrOfRows = 1, size_t nrOfSlices = 1, size_t nrOfTimesteps = 1 )const;

	/**
	 * Ensure, the chunk has the type with the requested ID.
	 * If the typeID of the chunk is not equal to the requested ID, the data of the chunk is replaced by an converted version.
	 * \returns false if there was an error
	 */
	bool convertToType( short unsigned int ID, scaling_pair scaling = scaling_pair() );

	template<typename T> bool copyToMem( T *dst, size_t len, scaling_pair scaling = scaling_pair() )const {
		return getValuePtrBase().copyToMem<T>( dst, len,  scaling ); // use copyToMem of ValuePtrBase
	}

	///get the scaling (and offset) which would be used in an conversion to the given type
	scaling_pair getScalingTo( unsigned short typeID, autoscaleOption scaleopt = autoscale )const;
	scaling_pair getScalingTo( unsigned short typeID, const std::pair<util::ValueReference, util::ValueReference> &minmax, autoscaleOption scaleopt = autoscale )const;

	size_t bytesPerVoxel()const;
	std::string getTypeName()const;
	unsigned short getTypeID()const;
	template<typename T> bool is()const {return getValuePtrBase().is<T>();}

	void copyRange( const size_t source_start[], const size_t source_end[], Chunk &dst, const size_t destination[] )const;

	size_t compare( const Chunk &dst )const;
	size_t compareRange( const size_t source_start[], const size_t source_end[], const Chunk &dst, const size_t destination[] )const;

	std::pair<util::ValueReference, util::ValueReference> getMinMax()const;

	Chunk &operator=( const Chunk &ref );

	/**
	 * Splices the chunk at the uppermost dimension and automatically sets indexOrigin and acquisitionNumber appropriately.
	 * This automatically selects the upermost dimension of the chunk to be spliced and will compute the correct offsets
	 * for indexOrigin and acquisitionNumberOffset which will be applied to the resulting splices.
	 * E.g. splice\(1\) on a chunk of the size 512x512x128, the rowVec 1,0,0, the columnVec 0,1,0 and the indexOrigin 0,0,0
	 * will result in 128 chunks of the size 512x512x1, the rowVec 1,0,0, the columnVec 0,1,0 and the indexOrigin 0,0,0 to 0,0,128.
	 * (If voxelSize is 1,1,1 and voxelGap is 0,0,0)
	 * (acquisitionNumber will be reset to a simple incrementing counter starting at acquisitionNumberOffset)
	 */
	std::list<Chunk> autoSplice( uint32_t acquisitionNumberStride = 0 )const;

	/**
	 * Splices the chunk at the given dimension and all dimensions above.
	 * As this will not set or use any property
	 * - they have to be modified afterwards
	 * - this can be done on chunks without any property (aka invalid Chunks)
	 * E.g. splice\(columnDim\) on a chunk of the size 512x512x128 will result in 512*128 chunks of the size 512x1x1
	 */
	std::list<Chunk> splice( dimensions atDim )const;

	/**
	 * Transforms the image coordinate system into an other system by multiplying
	 * the orientation matrix with a user defined transformation matrix. Additionally,
	 * the index origin will be transformed into the new coordinate system. This
	 * function only changes the
	 *
	 * <B>IMPORTANT!</B>: If you call this function with a matrix other than the
	 * identidy matrix, it's not guaranteed that the image is still in ISIS space
	 * according to the DICOM conventions. Maybe some ISIS algorithms that
	 * depend on correct image orientations won't work as expected. Use this method
	 * with caution!
	 */
	bool transformCoords( boost::numeric::ublas::matrix<float> transform_matrix, bool transformCenterIsImageCenter = false ) {
		if( hasProperty( "rowVec" ) && hasProperty( "columnVec" ) && hasProperty( "sliceVec" )
			&& hasProperty( "voxelSize" ) && hasProperty( "indexOrigin" ) ) {
			if( !isis::data::_internal::transformCoords( *this, getSizeAsVector(), transform_matrix, transformCenterIsImageCenter ) ) {
				LOG( Runtime, error ) << "Error during transforming the coords of the chunk.";
				return false;
			}

			return true;
		}

		return true;
	}
	/**
	  * Swaps the image along a dimension dim in image space.
	  */
	void swapAlong( const dimensions dim ) const;

	/**
	 * Access properties of the next lower dimension (e.g. slice-timings in volumes)
	 * This is there for effiency on IO only (you don't have to split up chunks just to store some properties).
	 * It will be resolved by reindexing anyway. So, in an clean Image Chunks will never have such sub-properties.
	 */
	util::PropertyValue &propertyValueAt( const PropertyMap::KeyType &key, size_t at );
	/// \copydoc propertyValueAt
	const util::PropertyValue &propertyValueAt( const PropertyMap::KeyType &key, size_t at )const;
};

/// Chunk class for memory-based buffers
template<typename TYPE> class MemChunk : public Chunk
{
public:
	/// Create an empty MemChunk with the given size
	MemChunk( size_t nrOfColumns, size_t nrOfRows = 1, size_t nrOfSlices = 1, size_t nrOfTimesteps = 1 ):
		Chunk( ValuePtrReference( ValuePtr<TYPE>( nrOfColumns *nrOfRows *nrOfSlices *nrOfTimesteps ) ), nrOfColumns, nrOfRows, nrOfSlices, nrOfTimesteps ) {}
	/**
	 * Create a MemChunk as copy of a given raw memory block
	 * This will create a MemChunk of the given size and fill it with the data at the given address.
	 * No range check will be done.
	 * An automatic conversion will be done if necessary.
	 * \param org pointer to the raw data which shall be copied
	 * \param nrOfColumns
	 * \param nrOfRows
	 * \param nrOfSlices
	 * \param nrOfTimesteps size of the resulting image
	 */
	template<typename T> MemChunk( const T *const org, size_t nrOfColumns, size_t nrOfRows = 1, size_t nrOfSlices = 1, size_t nrOfTimesteps = 1 ):
		Chunk( ValuePtrReference( ValuePtr<TYPE>( nrOfColumns *nrOfRows *nrOfSlices *nrOfTimesteps ) ), nrOfColumns, nrOfRows, nrOfSlices, nrOfTimesteps ) {
		util::checkType<T>();
		asValuePtrBase().copyFromMem( org, getVolume() );
	}
	/// Create a deep copy of a given Chunk (automatic conversion will be used if datatype does not fit)
	MemChunk( const Chunk &ref ): Chunk( ref ) {
		//get rid of my ValuePtr and make a new copying/converting the data of ref (use the reset-function of the scoped_ptr Chunk is made of)
		ValuePtrReference::operator=( ref.getValuePtrBase().copyByID( ValuePtr<TYPE>::staticID ) );
	}
	/**
	 * Create a deep copy of a given Chunk.
	 * An automatic conversion is used if datatype does not fit
	 * \param ref the source chunk
	 * \param scaling the scaling (scale and offset) to be used if a conversion to the requested type is neccessary.
	 */
	MemChunk( const Chunk &ref, const scaling_pair &scaling ): Chunk( ref ) {
		//get rid of my ValuePtr and make a new copying/converting the data of ref (use the reset-function of the scoped_ptr Chunk is made of)
		ValuePtrReference::operator=( ref.getValuePtrBase().copyByID( ValuePtr<TYPE>::staticID, scaling ) );
	}
	MemChunk( const MemChunk<TYPE> &ref ): Chunk( ref ) { //this is needed, to prevent generation of default-copy constructor
		//get rid of my ValuePtr and make a new copying/converting the data of ref (use the reset-function of the scoped_ptr Chunk is made of)
		ValuePtrReference::operator=( ref.getValuePtrBase().copyByID( ValuePtr<TYPE>::staticID ) );
	}
	/// Create a deep copy of a given Chunk (automatic conversion will be used if datatype does not fit)
	MemChunk &operator=( const Chunk &ref ) {
		LOG_IF( useCount() > 1, Debug, warning )
				<< "Not overwriting current chunk memory (which is still used by " << useCount() - 1 << " other chunk(s)).";
		Chunk::operator=( ref ); //copy the chunk of ref
		//get rid of my ValuePtr and make a new copying/converting the data of ref (use the reset-function of the scoped_ptr Chunk is made of)
		ValuePtrReference::operator=( ref.getValuePtrBase().copyByID( ValuePtr<TYPE>::staticID ) );
		return *this;
	}
	/// Create a deep copy of a given MemChunk (automatic conversion will be used if datatype does not fit)
	MemChunk &operator=( const MemChunk<TYPE> &ref ) { //this is needed, to prevent generation of default-copy operator
		return operator=( static_cast<const Chunk &>( ref ) );
	}
};


template<typename TYPE> class MemChunkNonDel : public Chunk
{
public:
	//
	/// Create an empty MemChunkNoDel with the given size
	MemChunkNonDel( size_t nrOfColumns, size_t nrOfRows = 1, size_t nrOfSlices = 1, size_t nrOfTimesteps = 1 ):
		Chunk(
			( TYPE * )calloc( nrOfTimesteps *nrOfSlices *nrOfRows *nrOfColumns, sizeof( TYPE ) ),
			typename ValuePtr<TYPE>::NonDeleter(),
			nrOfColumns, nrOfRows, nrOfSlices, nrOfTimesteps
		) {}
	/**
	 * Create a MemChunkNoDel as copy of a given raw memory block
	 * This chunk won't be deleted automatically - HAVE TO BE DELETED MANUALLY
	 * This will create a MemChunkNoDel of the given size and fill it with the data at the given address.
	 * No range check will be done.
	 * An automatic conversion will be done if necessary.
	 * \param org pointer to the raw data which shall be copied
	 * \param nrOfColumns
	 * \param nrOfRows
	 * \param nrOfSlices
	 * \param nrOfTimesteps size of the resulting image
	 */
	template<typename T> MemChunkNonDel( const T *const org, size_t nrOfColumns, size_t nrOfRows = 1, size_t nrOfSlices = 1, size_t nrOfTimesteps = 1 ):
		Chunk(
			( TYPE * )malloc( sizeof( TYPE )*nrOfTimesteps *nrOfSlices *nrOfRows *nrOfColumns ),
			typename ValuePtr<TYPE>::NonDeleter(),
			nrOfColumns, nrOfRows, nrOfSlices, nrOfTimesteps
		) {
		util::checkType<T>();
		asValuePtrBase().copyFromMem( org, getVolume() );
	}
	/// Create a deep copy of a given Chunk (automatic conversion will be used if datatype does not fit)
	MemChunkNonDel( const Chunk &ref ): Chunk( ref ) {
		//get rid of my ValuePtr and make a new copying/converting the data of ref (use the reset-function of the scoped_ptr Chunk is made of)
		ValuePtrReference::operator=( ref.getValuePtrBase().copyByID( ValuePtr<TYPE>::staticID ) );
	}
	/**
	 * Create a deep copy of a given Chunk.
	 * An automatic conversion used if datatype does not fit
	 * \param ref the source chunk
	 * \param min
	 * \param max the value range of the source to be used when the scaling for the conversion is computed
	 */
	MemChunkNonDel( const Chunk &ref, const util::_internal::ValueBase &min, const  util::_internal::ValueBase &max ): Chunk( ref ) {
		//get rid of my ValuePtr and make a new copying/converting the data of ref (use the reset-function of the scoped_ptr Chunk is made of)
		ValuePtrReference::operator=( ref.getValuePtrBase().copyByID( ValuePtr<TYPE>::staticID, min, max ) );
	}
	MemChunkNonDel( const MemChunk<TYPE> &ref ): Chunk( ref ) { //this is needed, to prevent generation of default-copy constructor
		//get rid of my ValuePtr and make a new copying/converting the data of ref (use the reset-function of the scoped_ptr Chunk is made of)
		ValuePtrReference::operator=( ref.getValuePtrBase().copyByID( ValuePtr<TYPE>::staticID ) );
	}
	/// Create a deep copy of a given Chunk (automatic conversion will be used if datatype does not fit)
	MemChunkNonDel &operator=( const Chunk &ref ) {
		LOG_IF( useCount() > 1, Debug, warning )
				<< "Not overwriting current chunk memory (which is still used by " << useCount() - 1 << " other chunk(s)).";
		Chunk::operator=( ref ); //copy the chunk of ref
		//get rid of my ValuePtr and make a new copying/converting the data of ref (use the reset-function of the scoped_ptr Chunk is made of)
		ValuePtrReference::operator=( ref.getValuePtrBase().copyByID( ValuePtr<TYPE>::staticID ) );
		return *this;
	}
	/// Create a deep copy of a given MemChunk (automatic conversion will be used if datatype does not fit)
	MemChunkNonDel &operator=( const MemChunkNonDel<TYPE> &ref ) { //this is needed, to prevent generation of default-copy operator
		return operator=( static_cast<const Chunk &>( ref ) );
	}
};
}
}
namespace std
{
/// Streaming output for Chunk (forward to PropertyMap)
template<typename charT, typename traits>
basic_ostream<charT, traits>& operator<<( basic_ostream<charT, traits> &out, const isis::data::Chunk &s )
{
	return out << static_cast<const isis::util::PropertyMap &>( s );
}
}
#endif // CHUNK_H
