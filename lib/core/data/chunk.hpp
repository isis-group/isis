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

#include "../util/vector.hpp"
#include "valuearray.hpp"
#include "../util/log.hpp"
#include "../util/propmap.hpp"
#include "common.hpp"
#include <string.h>
#include <list>
#include "ndimensional.hpp"

namespace isis
{
namespace data
{

class Chunk;

/// @cond _internal
namespace _internal
{
}
/// @endcond _internal

/// Base class for operators used for foreachVoxel
template <typename TYPE> class VoxelOp: std::unary_function<bool, TYPE>
{
public:
	virtual bool operator()( TYPE &vox, const util::vector4<size_t> &pos ) = 0;
	virtual ~VoxelOp() {}
};

/**
 * Main class for four-dimensional random-access data blocks.
 * Like in ValueArray, the copy of a Chunk will reference the same data. (cheap copy)
 * (If you want to make a memory based deep copy of a Chunk create a MemChunk from it)
 */
class Chunk : public _internal::NDimensional<4>, public util::PropertyMap, protected ValueArrayReference
{
	friend class Image;
	friend class std::vector<Chunk>;
	Chunk() {}; //do not use this
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
	template<typename TYPE, typename D> Chunk( TYPE *src, D d, size_t nrOfColumns, size_t nrOfRows = 1, size_t nrOfSlices = 1, size_t nrOfTimesteps = 1, bool fakeValid = false  ):
		Chunk(ValueArray<TYPE>( src, getVolume(), d ), nrOfColumns, nrOfRows, nrOfSlices, nrOfTimesteps,fakeValid)
		{}

public:
	static const char *neededProperties;
	typedef isis::util::_internal::GenericReference<Chunk> Reference;

	typedef ValueArrayBase::value_iterator iterator;
	typedef ValueArrayBase::const_value_iterator const_iterator;
	typedef iterator::reference reference;
	typedef const_iterator::reference const_reference;

	Chunk( const ValueArrayReference &src, size_t nrOfColumns, size_t nrOfRows = 1, size_t nrOfSlices = 1, size_t nrOfTimesteps = 1, bool fakeValid = false );

	/**
	 * Gets a reference to the element at a given index.
	 * If index is invalid, behaviour is undefined. Most probably it will crash.
	 * If _ENABLE_DATA_DEBUG is true an error message will be send (but it will _not_ stop).
	 */
	template<typename TYPE> TYPE &voxel( size_t nrOfColumns, size_t nrOfRows = 0, size_t nrOfSlices = 0, size_t nrOfTimesteps = 0 ) {
		LOG_IF( ! isInRange( {nrOfColumns, nrOfRows, nrOfSlices, nrOfTimesteps} ), Debug, isis::error )
				<< "Index " << util::vector4<size_t>( {nrOfColumns, nrOfRows, nrOfSlices, nrOfTimesteps} ) << " is out of range " << getSizeAsString();
		ValueArray<TYPE> &ret = asValueArray<TYPE>();
		return ret[getLinearIndex( {nrOfColumns, nrOfRows, nrOfSlices, nrOfTimesteps} )];
	}

	const util::ValueReference getVoxelValue( size_t nrOfColumns, size_t nrOfRows = 0, size_t nrOfSlices = 0, size_t nrOfTimesteps = 0 )const;
	void setVoxelValue( const util::ValueReference &val, size_t nrOfColumns, size_t nrOfRows = 0, size_t nrOfSlices = 0, size_t nrOfTimesteps = 0 );

	/**
	 * Gets a const reference of the element at a given index.
	 * \copydetails Chunk::voxel
	 */
	template<typename TYPE> const TYPE &voxel( size_t nrOfColumns, size_t nrOfRows = 0, size_t nrOfSlices = 0, size_t nrOfTimesteps = 0 )const {
		if ( !isInRange( {nrOfColumns, nrOfRows, nrOfSlices, nrOfTimesteps} ) ) {
			LOG( Debug, isis::error )
					<< "Index " << util::vector4<size_t>( {nrOfColumns, nrOfRows, nrOfSlices, nrOfTimesteps} ) << nrOfTimesteps
					<< " is out of range (" << getSizeAsString() << ")";
		}

		const ValueArray<TYPE> &ret = const_cast<Chunk &>( *this ).asValueArray<TYPE>();

		return ret[getLinearIndex( {nrOfColumns, nrOfRows, nrOfSlices, nrOfTimesteps} )];
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
	template <typename TYPE> size_t foreachVoxel( VoxelOp<TYPE> &op, util::vector4<size_t> offset ) {
		const util::vector4<size_t> imagesize = getSizeAsVector();
		util::vector4<size_t> pos;
		TYPE *vox = &asValueArray<TYPE>()[0];
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
		return foreachVoxel<TYPE>( op, util::vector4<size_t>() );
	}

	iterator begin();
	iterator end();
	const_iterator begin()const;
	const_iterator end()const;

	ValueArrayBase &asValueArrayBase() {return operator*();}
	const ValueArrayBase &getValueArrayBase()const {return operator*();}

	/// reference the internal array representation of the pixel data in the chunk
	template<typename TYPE> ValueArray<TYPE> &asValueArray() {return asValueArrayBase().castToValueArray<TYPE>();}
	/// const version of asValueArray
	template<typename TYPE> const ValueArray<TYPE> getValueArray()const {return getValueArrayBase().castToValueArray<TYPE>();}

	/// \returns the number of cheap-copy-chunks using the same memory as this
	size_t useCount()const;

	/// Creates a new empty Chunk of different size and without properties, but of the same datatype as this.
	Chunk cloneToNew( size_t nrOfColumns, size_t nrOfRows = 1, size_t nrOfSlices = 1, size_t nrOfTimesteps = 1 )const;

	/// Creates a new empty Chunk without properties but of specified type and specified size.
	static Chunk createByID( short unsigned int ID, size_t nrOfColumns, size_t nrOfRows = 1, size_t nrOfSlices = 1, size_t nrOfTimesteps = 1, bool fakeValid = false );

	/**
	 * Ensure, the chunk has the type with the requested ID.
	 * If the typeID of the chunk is not equal to the requested ID, the data of the chunk is replaced by an converted version.
	 * \returns false if there was an error
	 */
	bool convertToType( short unsigned int ID, scaling_pair scaling = scaling_pair() );

	/**
	 * Copy all voxel data of the chunk into memory.
	 * If neccessary a conversion into T is done using min/max of the image.
	 * \param dst c-pointer for the memory to copy into
	 * \param len the allocated size of that memory in elements
	 * \param scaling the scaling to be used when converting the data (will be determined automatically if not given)
	 * \return true if copying was (at least partly) successful
	 */
	template<typename T> bool copyToMem( T *dst, size_t len, scaling_pair scaling = scaling_pair() )const {
		return getValueArrayBase().copyToMem<T>( dst, len,  scaling ); // use copyToMem of ValueArrayBase
	}
	/**
	 * Create a new Chunk of the requested type and copy all voxel data of the chunk into it.
	 * If neccessary a conversion into the requested type is done using the given scale.
	 * \param ID the ID of the requested type (type of the source is used if not given)
	 * \param scaling the scaling to be used when converting the data (will be determined automatically if not given)
	 * \return a new deep copied Chunk of the same size
	 */
	Chunk copyByID( unsigned short ID = 0, scaling_pair scaling = scaling_pair() )const;

	///get the scaling (and offset) which would be used in an conversion to the given type
	scaling_pair getScalingTo( unsigned short typeID, autoscaleOption scaleopt = autoscale )const;
	scaling_pair getScalingTo( unsigned short typeID, const std::pair<util::ValueReference, util::ValueReference> &minmax, autoscaleOption scaleopt = autoscale )const;

	size_t getBytesPerVoxel()const;
	std::string getTypeName()const;
	unsigned short getTypeID()const;

	/**
	 * Get a string describing the shape of the chunk.
	 * \param upper make the first letter uppercase
	 * \returns "slice" for 2D chunks
	 * \returns "volume" for 3D chunks
	 * \returns "volset" for 4D chunks
	 * \returns "chunk" otherwise
	 */
	std::string getShapeString( bool upper=false )const;
	
	template<typename T> bool is()const {return getValueArrayBase().is<T>();}

	void copyRange( const std::array< size_t, 4 >& source_start, const std::array< size_t, 4 >& source_end, isis::data::Chunk& dst, const std::array< size_t, 4 >& destination=std::array< size_t, 4 >({0,0,0,0}) )const;

	size_t compare( const Chunk &dst )const;
	size_t compareRange( const std::array<size_t,4> &source_start, const std::array<size_t,4> &source_end, Chunk &dst, const std::array<size_t,4> &destination )const;

	std::pair<util::ValueReference, util::ValueReference> getMinMax()const;

	/**
	 * Splices the chunk at the uppermost dimension and automatically sets indexOrigin and acquisitionNumber appropriately.
	 * This automatically selects the upermost dimension of the chunk to be spliced and will compute the correct offsets
	 * for indexOrigin and acquisitionNumberOffset which will be applied to the resulting splices.
	 *
	 * E.g. autoSplice() on a chunk of the size 512x512x128, with rowVec 1,0,0, columnVec 0,1,0 and indexOrigin 0,0,0
	 * will result in 128 chunks of the size 512x512x1, with constant rowVec's 1,0,0, and columnVec's 0,1,0  while the indexOrigin will be going from 0,0,0 to 0,0,128
	 * (If voxelSize is 1,1,1 and voxelGap is 0,0,0). The acquisitionNumber will be reset to a simple incrementing counter starting at acquisitionNumberOffset.
	 * \attention As this will also move alle properties into the "splinters" this chunk will be invalid afterwards
	 */
	std::list<Chunk> autoSplice( uint32_t acquisitionNumberStride = 0 );

	/**
	 * Splices the chunk at the given dimension and all dimensions above.
	 * E.g. splice\(columnDim\) on a chunk of the size 512x512x128 will result in 512*128 chunks of the size 512x1x1
	 * \attention As this will also move alle properties into the "splinters" this chunk will be invalid afterwards
	 */
	std::list<Chunk> splice( isis::data::dimensions atDim );

	/**
	  * Flips the chunk along a dimension dim in image space.
	  */
	void swapAlong( const dimensions dim ) const;

	//http://en.wikipedia.org/wiki/In-place_matrix_transposition#Non-square_matrices%3a_Following_the_cycles
	void swapDim(unsigned short dim_a,unsigned short dim_b);
};

/// Chunk class for memory-based buffers
template<typename TYPE> class MemChunk : public Chunk
{
public:
	/// Create an empty MemChunk with the given size
	MemChunk( size_t nrOfColumns, size_t nrOfRows = 1, size_t nrOfSlices = 1, size_t nrOfTimesteps = 1, bool fakeValid = false ):
		Chunk( ValueArrayReference( ValueArray<TYPE>( nrOfColumns *nrOfRows *nrOfSlices *nrOfTimesteps ) ), nrOfColumns, nrOfRows, nrOfSlices, nrOfTimesteps, fakeValid ) {}
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
	template<typename T> MemChunk( const T *const org, size_t nrOfColumns, size_t nrOfRows = 1, size_t nrOfSlices = 1, size_t nrOfTimesteps = 1, bool fakeValid = false  ):
		Chunk( ValueArrayReference( ValueArray<TYPE>( nrOfColumns *nrOfRows *nrOfSlices *nrOfTimesteps ) ), nrOfColumns, nrOfRows, nrOfSlices, nrOfTimesteps ) {
		util::checkType<T>();
		asValueArrayBase().copyFromMem( org, getVolume() );
	}
	/// Create a deep copy of a given Chunk (automatic conversion will be used if datatype does not fit)
	MemChunk( const Chunk &ref ): Chunk( ref ) {
		//get rid of my ValueArray and make a new copying/converting the data of ref (use the reset-function of the scoped_ptr Chunk is made of)
		ValueArrayReference::operator=( ref.getValueArrayBase().copyByID( ValueArray<TYPE>::staticID() ) );
	}
	/**
	 * Create a deep copy of a given Chunk.
	 * An automatic conversion is used if datatype does not fit
	 * \param ref the source chunk
	 * \param scaling the scaling (scale and offset) to be used if a conversion to the requested type is neccessary.
	 */
	MemChunk( const Chunk &ref, const scaling_pair &scaling ): Chunk( ref ) {
		//get rid of my ValueArray and make a new copying/converting the data of ref (use the reset-function of the scoped_ptr Chunk is made of)
		ValueArrayReference::operator=( ref.getValueArrayBase().copyByID( ValueArray<TYPE>::staticID(), scaling ) );
	}
	MemChunk( const MemChunk<TYPE> &ref ): Chunk( ref ) { //this is needed, to prevent generation of default-copy constructor
		//get rid of my ValueArray and make a new copying/converting the data of ref (use the reset-function of the scoped_ptr Chunk is made of)
		ValueArrayReference::operator=( ref.getValueArrayBase().copyByID( ValueArray<TYPE>::staticID() ) );
	}
	/// Create a deep copy of a given Chunk (automatic conversion will be used if datatype does not fit)
	MemChunk &operator=( const Chunk &ref ) {
		LOG_IF( useCount() > 1, Debug, warning )
				<< "Not overwriting current chunk memory (which is still used by " << useCount() - 1 << " other chunk(s)).";
		Chunk::operator=( ref ); //copy the chunk of ref
		//get rid of my ValueArray and make a new copying/converting the data of ref (use the reset-function of the scoped_ptr Chunk is made of)
		ValueArrayReference::operator=( ref.getValueArrayBase().copyByID( ValueArray<TYPE>::staticID() ) );
		return *this;
	}
	/// Create a deep copy of a given MemChunk (automatic conversion will be used if datatype does not fit)
	MemChunk &operator=( const MemChunk<TYPE> &ref ) { //this is needed, to prevent generation of default-copy operator
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
