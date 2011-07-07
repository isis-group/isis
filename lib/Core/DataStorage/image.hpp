//
// C++ Interface: image
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef IMAGE_H
#define IMAGE_H

#include "chunk.hpp"

#include <set>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <stack>
#include "sortedchunklist.hpp"
#include "common.hpp"

namespace isis
{
namespace data
{

/// Base class for operators used for foreachChunk
class ChunkOp : std::unary_function<Chunk &, bool>
{
public:
	virtual bool operator()( Chunk &, util::FixedVector<size_t, 4> posInImage ) = 0;
};

/// Main class for generic 4D-images
class Image:
	public _internal::NDimensional<4>,
	public util::PropertyMap
{
	dimensions minIndexingDim;
public:
	/**
	 * Enforce indexing to start at a given dimension.
	 * Normally indexing starts at the dimensionality of the inserted chunks.
	 * So, an Image of 2D-Chunks (slices) will start indexing at the 3rd dimension.
	 * If the dimension given here is bigger than the dimensionality of the chunks reindexing will override that and start indexing at the given dimension.
	 * E.g. setIndexingDim(timeDim) will enforce indexing of a Image of 10 30x30-slices at the time dimension resulting in an 30x30x1x10 image instead of an 30x30x10x1 image.
	 * If the indexing dimension is set after the Image was indexed it will be indexed again.
	 * \param d the minimal indexing dimension to be used
	 */
	void setIndexingDim( dimensions d = rowDim );
	enum orientation {axial, reversed_axial, sagittal, reversed_sagittal, coronal, reversed_coronal};

protected:
	_internal::SortedChunkList set;
	std::vector<boost::shared_ptr<Chunk> > lookup;
private:
	size_t chunkVolume;

	void deduplicateProperties();

	/**
	 * Get the pointer to the chunk in the internal lookup-table at position at.
	 * The Chunk will only have metadata which are unique to it - so it might be invalid
	 * (run join on it using the image as parameter to insert all non-unique-metadata).
	 */
	const boost::shared_ptr<Chunk> &chunkPtrAt( size_t at )const;

	/**
	 * Computes chunk- and voxel- indices.
	 * The returned chunk-index applies to the lookup-table (chunkAt), and the voxel-index to this chunk.
	 * Behaviour will be undefined if:
	 * - the image is not clean (not indexed)
	 * - the image is empty
	 * - the coordinates are not in the image
	 *
	 * Additionally an error will be sent if Debug is enabled.
	 * \returns a std::pair\<chunk-index,voxel-index\>
	 */
	inline std::pair<size_t, size_t> commonGet ( size_t first, size_t second, size_t third, size_t fourth ) const {
		const size_t idx[] = {first, second, third, fourth};
		LOG_IF( ! clean, Debug, error )
				<< "Getting data from a non indexed image will result in undefined behavior. Run reIndex first.";
		LOG_IF( set.isEmpty(), Debug, error )
				<< "Getting data from a empty image will result in undefined behavior.";
		LOG_IF( !isInRange( idx ), Debug, isis::error )
				<< "Index " << util::listToString( idx, idx + 4, "|" ) << " is out of range (" << getSizeAsString() << ")";
		const size_t index = getLinearIndex( idx );
		return std::make_pair( index / chunkVolume, index % chunkVolume );
	}


protected:
	bool clean;
	static const char *neededProperties;

	/**
	 * Search for a dimensional break in all stored chunks.
	 * This function searches for two chunks whose (geometrical) distance is more than twice
	 * the distance between the first and the second chunk. It wll assume a dimensional break
	 * at this position.
	 *
	 * Normally chunks are beneath each other (like characters in a text) so their distance is
	 * more or less constant. But if there is a dimensional break (analogous to the linebreak
	 * in a text) the distance between this particular chunks/characters is bigger than twice
	 * the normal distance
	 *
	 * For example for an image of 2D-chunks (slices) getChunkStride(1) will
	 * get the number of slices (size of third dim) and  getChunkStride(slices)
	 * will get the number of timesteps
	 * \param base_stride the base_stride for the iteration between chunks (1 for the first
	 * dimension, one "line" for the second and soon...)
	 * \returns the length of this chunk-"line" / the stride
	 */
	size_t getChunkStride( size_t base_stride = 1 );
	/**
	 * Access a chunk via index (and the lookup table)
	 * The Chunk will only have metadata which are unique to it - so it might be invalid
	 * (run join on it using the image as parameter to insert all non-unique-metadata).
	 */
	Chunk &chunkAt( size_t at );
	/// Creates an empty Image object.
	Image();



	util::fvector4 m_RowVec;
	util::fvector4 m_RowVecInv;
	util::fvector4 m_ColumnVec;
	util::fvector4 m_ColumnVecInv;
	util::fvector4 m_SliceVec;
	util::fvector4 m_SliceVecInv;
	util::fvector4 m_Offset;
public:
	/**
	 * Copy constructor.
	 * Copies all elements, only the voxel-data (in the chunks) are referenced.
	 */
	Image( const Image &ref );

	/**
	 * Create image from a list of Chunks or objects with the base Chunk.
	 * Removes used chunks from the given list. So afterwards the list consists of the rejected chunks.
	 */
	template<typename T> Image( std::list<T> &chunks, dimensions min_dim = rowDim ) :
		_internal::NDimensional<4>(), util::PropertyMap(), minIndexingDim( min_dim ),
		set( "sequenceNumber,rowVec,columnVec,sliceVec,coilChannelMask,DICOM/EchoNumbers" ),
		clean( false ) {
		addNeededFromString( neededProperties );
		set.addSecondarySort( "acquisitionNumber" );
		set.addSecondarySort( "acquisitionTime" );
		insertChunksFromContainer( chunks );
	}
	/**
	 * Create image from a vector of Chunks or objects with the base Chunk.
	 * Removes used chunks from the given list. So afterwards the list consists of the rejected chunks.
	 */
	template<typename T> Image( std::vector<T> &chunks, dimensions min_dim = rowDim ) :
		_internal::NDimensional<4>(), util::PropertyMap(),
		set( "sequenceNumber,rowVec,columnVec,sliceVec,coilChannelMask,DICOM/EchoNumbers" ),
		clean( false ), minIndexingDim( min_dim ) {
		addNeededFromString( neededProperties );
		set.addSecondarySort( "acquisitionNumber" );
		set.addSecondarySort( "acquisitionTime" );
		insertChunksFromContainer( chunks );
	}

	/**
	 * Insert Chunks or objects with the base Chunk from a sequence container into the Image.
	 * Removes used chunks from the given sequence container. So afterwards the container consists of the rejected chunks.
	 * \returns amount of successfully inserted chunks
	 */
	template<typename T> size_t insertChunksFromContainer( T &chunks ) {
		BOOST_STATIC_ASSERT( ( boost::is_base_of<Chunk, typename T::value_type >::value ) );
		size_t cnt = 0;

		for ( typename T::iterator i = chunks.begin(); i != chunks.end(); ) { // for all remaining chunks
			if ( insertChunk( *i ) ) {
				chunks.erase( i++ );
				cnt++;
			} else {
				i++;
			}
		}

		if ( ! isEmpty() ) {
			LOG( Debug, info ) << "Reindexing image with " << cnt << " chunks.";

			if( !reIndex() ) {
				LOG( Runtime, error ) << "Failed to create image from " << cnt << " chunks.";
			} else {
				LOG_IF( !getMissing().empty(), Debug, warning )
						<< "The created image is missing some properties: " << getMissing() << ". It will be invalid.";
			}
		} else {
			LOG( Debug, warning ) << "Image is empty after inserting chunks.";
		}

		return cnt;
	}


	/**
	 * Create image from a single chunk.
	 */
	Image( const Chunk &chunk, dimensions min_dim = rowDim );

	/**
	 * Copy operator.
	 * Copies all elements, only the voxel-data (in the chunks) are referenced.
	 */
	Image &operator=( const Image &ref );

	bool checkMakeClean();
	bool isClean()const;
	/**
	 * This method returns a reference to the voxel value at the given coordinates.
	 *
	 * The voxel reference provides reading and writing access to the refered
	 * value.
	 *
	 * If the image is not clean, reIndex will be run.
	 * If the requested voxel is not of type T, an error will be raised.
	 *
	 * \param first The first coordinate in voxel space. Usually the x value / the read-encoded position..
	 * \param second The second coordinate in voxel space. Usually the y value / the column-encoded position.
	 * \param third The third coordinate in voxel space. Ususally the z value / the time-encoded position.
	 * \param fourth The fourth coordinate in voxel space. Usually the time value.
	 *
	 * \returns A reference to the addressed voxel value. Reading and writing access
	 * is provided.
	 */
	template <typename T> T &voxel( size_t first, size_t second = 0, size_t third = 0, size_t fourth = 0 ) {
		checkMakeClean();
		const std::pair<size_t, size_t> index = commonGet( first, second, third, fourth );
		ValuePtr<T> &data = chunkAt( index.first ).asValuePtr<T>();
		return data[index.second];
	}

	/**
	 * Get a const reference to the voxel value at the given coordinates.
	 *
	 * \param first The first coordinate in voxel space. Usually the x value / the read-encoded position..
	 * \param second The second coordinate in voxel space. Usually the y value / the column-encoded position.
	 * \param third The third coordinate in voxel space. Ususally the z value / the time-encoded position.
	 * \param fourth The fourth coordinate in voxel space. Usually the time value.
	 *
	 * If the requested voxel is not of type T, an error will be raised.
	 *
	 * \returns A reference to the addressed voxel value. Only reading access is provided
	 */
	template <typename T> const T &voxel( size_t first, size_t second = 0, size_t third = 0, size_t fourth = 0 )const {
		const std::pair<size_t, size_t> index = commonGet( first, second, third, fourth );
		const ValuePtr<T> &data = chunkPtrAt( index.first )->getValuePtr<T>();
		return data[index.second];
	}


	/**
	 * Get the type of the chunk with "biggest" type.
	 * Determines the minimum and maximum of the image, (and with that the types of these limits).
	 * If they are not the same, the type which can store the other type is selected.
	 * E.g. if min is "-5(int8_t)" and max is "1000(int16_t)" "int16_t" is selected.
	 * Warning1: this will fail if min is "-5(int8_t)" and max is "70000(uint16_t)"
	 * Warning2: the cost of this is O(n) while Chunk::getTypeID is O(1) - so do not use it in loops
	 * Warning3: the result is not exact - so never use it to determine the type for Image::voxel (Use TypedImage to get an image with an guaranteed type)
	 * \returns a number which is equal to the ValuePtr::staticID of the selected type.
	 */
	unsigned short getMajorTypeID() const;
	/// \returns the typename correspondig to the result of typeID
	std::string getMajorTypeName() const;

	/**
	 * Get a chunk via index (and the lookup table).
	 * The returned chunk will be a cheap copy of the original chunk.
	 * If copy_metadata is true the metadata of the image is copied into the chunk.
	 */
	Chunk getChunkAt( size_t at, bool copy_metadata = true )const;

	/**
	 * Get the chunk that contains the voxel at the given coordinates.
	 *
	 * If the image is not clean, behaviour is undefined. (See Image::commonGet).
	 *
	 * \param first The first coordinate in voxel space. Usually the x value / the read-encoded position.
	 * \param second The second coordinate in voxel space. Usually the y value / the column-encoded position.
	 * \param third The third coordinate in voxel space. Ususally the z value / the slice-encoded position.
	 * \param fourth The fourth coordinate in voxel space. Usually the time value.
	 * \param copy_metadata if true the metadata of the image are merged into the returned chunk
	 * \returns a copy of the chunk that contains the voxel at the given coordinates.
	 * (Reminder: Chunk-copies are cheap, so the image data are NOT copied but referenced)
	 */
	const Chunk getChunk( size_t first, size_t second = 0, size_t third = 0, size_t fourth = 0, bool copy_metadata = true )const;

	/**
	 * Get the chunk that contains the voxel at the given coordinates.
	 * If the image is not clean Image::reIndex() will be run.
	 *
	 * \param first The first coordinate in voxel space. Usually the x value.
	 * \param second The second coordinate in voxel space. Usually the y value.
	 * \param third The third coordinate in voxel space. Ususally the z value.
	 * \param fourth The fourth coordinate in voxel space. Usually the time value.
	 * \param copy_metadata if true the metadata of the image are merged into the returned chunk
	 * \returns a copy of the chunk that contains the voxel at the given coordinates.
	 * (Reminder: Chunk-copies are cheap, so the image data are NOT copied but referenced)
	 */
	Chunk getChunk( size_t first, size_t second = 0, size_t third = 0, size_t fourth = 0, bool copy_metadata = true );

	/**
	 * Get the chunk that contains the voxel at the given coordinates in the given type.
	 * If the accordant chunk has type T a cheap copy is returned.
	 * Otherwise a MemChunk-copy of the requested type is created from it.
	 * In this case the minimum and maximum values of the image are computed and used for the MemChunk constructor.
	 *
	 * \param first The first coordinate in voxel space. Usually the x value.
	 * \param second The second coordinate in voxel space. Usually the y value.
	 * \param third The third coordinate in voxel space. Ususally the z value.
	 * \param fourth The fourth coordinate in voxel space. Usually the time value.
	 * \param copy_metadata if true the metadata of the image are merged into the returned chunk
	 * \returns a (maybe converted) chunk containing the voxel value at the given coordinates.
	 */
	template<typename TYPE> Chunk getChunkAs( size_t first, size_t second = 0, size_t third = 0, size_t fourth = 0, bool copy_metadata = true )const {
		return getChunkAs<TYPE>( getScalingTo( ValuePtr<TYPE>::staticID ), first, second, third, fourth, copy_metadata );
	}
	/**
	 * Get the chunk that contains the voxel at the given coordinates in the given type (fast version).
	 * \copydetails getChunkAs
	 * This version does not compute the scaling, and thus is much faster.
	 * \param scaling the scaling (scale and offset) to be used if a conversion to the requested type is neccessary.
	 * \param first The first coordinate in voxel space. Usually the x value.
	 * \param second The second coordinate in voxel space. Usually the y value.
	 * \param third The third coordinate in voxel space. Ususally the z value.
	 * \param fourth The fourth coordinate in voxel space. Usually the time value.
	 * \param copy_metadata if true the metadata of the image are merged into the returned chunk
	 * \returns a (maybe converted) chunk containing the voxel value at the given coordinates.
	 */
	template<typename TYPE> Chunk getChunkAs( const scaling_pair &scaling, size_t first, size_t second = 0, size_t third = 0, size_t fourth = 0, bool copy_metadata = true )const {
		Chunk ret = getChunk( first, second, third, fourth, copy_metadata ); // get a cheap copy
		ret.convertToType( ValuePtr<TYPE>::staticID, scaling ); // make it of type T
		return ret; //return that
	}

	///for each chunk get the scaling (and offset) which would be used in an conversion to the given type
	scaling_pair getScalingTo( unsigned short typeID, autoscaleOption scaleopt = autoscale )const;


	/**
	 * Insert a Chunk into the Image.
	 * The insertion is sorted and unique. So the Chunk will be inserted behind a geometrically "lower" Chunk if there is one.
	 * If there is allready a Chunk at the proposed position this Chunk wont be inserted.
	 *
	 * \param chunk The Chunk to be inserted
	 * \returns true if the Chunk was inserted, false otherwise.
	 */
	bool insertChunk( const Chunk &chunk );
	/**
	 * (Re)computes the image layout and metadata.
	 * The image will be "clean" on success.
	 * \returns true if the image was successfully reindexed and is valid, false otherwise.
	 */
	bool reIndex();

	/// \returns true if there is no chunk in the image
	bool isEmpty()const;

	/**
	 * Get a list of the properties of the chunks for the given key
	 * \param key the name of the property to search for
	 * \param unique when true empty or consecutive duplicates wont be added
	 */
	std::list<util::PropertyValue> getChunksProperties( const util::PropertyMap::KeyType &key, bool unique = false )const;

	/// get the size of every voxel (in bytes)
	size_t getBytesPerVoxel()const;

	/**
	 * Get the maximum and the minimum voxel value of the image.
	 * The results are converted to T. If they dont fit an error ist send.
	 * \returns a pair of T storing the minimum and maximum values of the image.
	 */
	template<typename T> std::pair<T, T> getMinMaxAs()const {
		util::checkType<T>();// works only for T from _internal::types
		std::pair<util::ValueReference, util::ValueReference> minmax = getMinMax();
		return std::make_pair( minmax.first->as<T>(), minmax.second->as<T>() );
	}

	/// Get the maximum and the minimum voxel value of the image as a pair of ValueReference-objects.
	std::pair<util::ValueReference, util::ValueReference> getMinMax()const;

	/**
	 * Compares the voxel-values of this image to the given.
	 * \returns the amount of the different voxels
	 */
	size_t compare( const Image &comp )const;

	orientation getMainOrientation()const;
	/**
	 * Transforms the image coordinate system into an other system by multiplying
	 * the orientation matrix with a user defined transformation matrix. Additionally,
	 * the index origin will be transformed into the new coordinate system. This
	 * function only changes the orientation information (rowVec, columnVec, sliceVec, indexOrigin)
	 * of the image but will not change the image itself.
	 *
	 * <B>IMPORTANT!</B>: If you call this function with a matrix other than the
	 * identidy matrix, it's not guaranteed that the image is still in ISIS space
	 * according to the DICOM conventions. Eventuelly some ISIS algorithms that
	 * depend on correct image orientations won't work as expected. Use this method
	 * with caution!
	 * \param transform_matrix the transformation matrix can be any type of rigid and affine transformation
	 * \param transformCenterIsImageCenter if this parameter is true, the center of the image will be translated to the
	 *  isocenter of the scanner prior applying the transform_matrix. Eventually, it will be translated to its
	 *  initial position. For example this is the way SPM flips its images when converting from DICOM to nifti.
	 * \return returns if the transformation was successfuly
	 */
	bool transformCoords( boost::numeric::ublas::matrix<float> transform_matrix, bool transformCenterIsImageCenter = false ) {
		std::list<std::string > neededProps;
		neededProps.push_back("indexOrigin");
		neededProps.push_back("rowVec");
		neededProps.push_back("columnVec");
		neededProps.push_back("sliceVec");
		neededProps.push_back("voxelSize");
	
		BOOST_FOREACH( std::vector<boost::shared_ptr< data::Chunk> >::reference chRef, lookup ) {
		    BOOST_FOREACH(std::list<std::string>::reference props, neededProps )
		    {
			if(hasProperty(props.c_str()) ) {
			    chRef->setPropertyAs<util::fvector4>(props.c_str(), getPropertyAs<util::fvector4>(props.c_str()));
			}
		    }
		    if( !chRef->transformCoords( transform_matrix, transformCenterIsImageCenter ) ) {
				return false;
			}
		}
		deduplicateProperties();
		if( !isis::data::_internal::transformCoords( *this, getSizeAsVector(), transform_matrix, transformCenterIsImageCenter ) ) {
			LOG( Runtime, error ) << "Error during transforming the coords of the image.";
			return false;
		}

		if( !updateOrientationMatrices() ) {
			LOG( Runtime, error ) << "Could not update the orientation matrices of the image!";
			return false;
		}

		return true;
	}

	/** Maps the given scanner Axes to the dimension with the minimal angle.
	 *  This is done by latching the orientation of the image by setting the biggest absolute
	 *  value of each orientation vector to 1 and the others to 0.
	 *  Example:
	 *          (-0.8)      (1)
	 *          ( 0.2)  ->  (0)   (this is done for the rowVec, columnVec and sliceVec)
	 *          (-0.1)      (0)
	 *
	 *  This latched orientation is used to map from the scanner axes to the dimension.
	 *  \param scannerAxes the axes of the scanner you want to map to dimension of the image.
	 *  \return the mapped image dimension
	 */

	dimensions mapScannerAxesToImageDimension( scannerAxis scannerAxes );

	/** Computes the physical coordinates (in scanner space) of the given voxel index.
	 *  This function does not perform any test if the voxel index is inside the image.
	 *  See getIndexFromPhysicalCoords for vice versa purpose.
	 *  \param index the voxel index from which you want to get the physical coordinates
	 *  \return physical coordinates associated with the given voxel index
	 */
	util::fvector4 getPhysicalCoordsFromIndex( const util::ivector4 &index ) const;


	/** Computes the voxel index of the given physical coordinates (coordinates in scanner space)
	 *  This function does not perform any test if the physical coordinates are inside the image.
	 *  See getPhysicalCoordsFromIndex for vice versa purpose.
	 *  \param physicalCoords the physical coords from which you want to get the voxel index.
	 *  \return voxel index associated with the given physicalCoords
	 */
	util::ivector4 getIndexFromPhysicalCoords( const util::fvector4 &physicalCoords ) const;

	/**
	 * Copy all voxel data of the image into memory.
	 * If neccessary a conversion into T is done using min/max of the image.
	 */
	template<typename T> void copyToMem( T *dst )const {
		if( clean ) {
			scaling_pair scale = getScalingTo( ValuePtr<T>::staticID );
			// we could do this using convertToType - but this solution does not need any additional temporary memory
			BOOST_FOREACH( const boost::shared_ptr<Chunk> &ref, lookup ) {
				if( !ref->copyToMem<T>( dst, scale ) ) {
					LOG( Runtime, error ) << "Failed to copy raw data of type " << ref->getTypeName() << " from image into memory of type " << ValuePtr<T>::staticName();
				}

				dst += ref->getVolume(); // increment the cursor
			}
		} else {
			LOG( Runtime, error ) << "Cannot copy from non clean images. Run reIndex first";
		}
	}

	/**
	 * Copy all voxel data into a new MemChunk.
	 * This creates a MemChunk\<T\> of the requested type and the same size as the Image and then copies all voxeldata of the image into that Chunk.
	 * If neccessary a conversion into T is done using min/max of the image.
	 * \returns a MemChunk\<T\> containing the voxeldata of the Image (but not its Properties)
	 */
	template<typename T> MemChunk<T> copyToMemChunk()const {
		const util::FixedVector<size_t, 4> size = getSizeAsVector();
		data::MemChunk<T> ret( size[0], size[1], size[2], size[3] );
		copyToMem<T>( &ret.voxel<T>( 0, 0, 0, 0 ) );
		return ret;
	}

	/**
	* Get a sorted list of the chunks of the image.
	* Note: These chunks are cheap copies, so changing their voxels will change the voxels of the image.
	* Make MemChunks of them to get deep copies.
	* \param copy_metadata set to false to prevent the metadata of the image to be copied into the results. This will improve performance, but the chunks may lack important properties.
	*/
	std::vector<isis::data::Chunk> copyChunksToVector( bool copy_metadata = true )const;

	/**
	 * Ensure, the image has the type with the requested ID.
	 * If the typeID of any chunk is not equal to the requested ID, the data of the chunk is replaced by an converted version.
	 * The conversion is done using the value range of the image.
	 * \returns false if there was an error
	 */
	bool convertToType( unsigned short ID );

	/**
	 * Automatically splice the given dimension and all dimensions above.
	 * e.g. spliceDownTo(sliceDim) will result in an image made of slices (aka 2d-chunks).
	 */
	size_t spliceDownTo( dimensions dim );

	/**
	 * Run a functor with the base ChunkOp on every cunk in the image.
	 * This does not check the types of the images. So if your functor needs a specific type, use TypedImage.
	 * \param op a functor object which inherits ChunkOP
	 * \param copyMetaData if true the metadata of the image are copied into the chunks before calling the functor
	 */
	size_t foreachChunk( ChunkOp &op, bool copyMetaData = false );


	/**
	 * Run a functor with the base VoxelOp on every chunk in the image.
	 * If any chunk does not have the requested type it will be converted.
	 * So the result is equivalent to TypedImage\<TYPE\>.
	 * If these conversion failes no operation is done, and false is returned.
	 * \param op a functor object which inherits ChunkOp
	 */
	template <typename TYPE> size_t foreachVoxel( VoxelOp<TYPE> &op ) {
		class _proxy: public ChunkOp
		{
			VoxelOp<TYPE> &op;
		public:
			_proxy( VoxelOp<TYPE> &_op ): op( _op ) {}
			bool operator()( Chunk &ch, util::FixedVector<size_t, 4 > posInImage ) {
				return ch.foreachVoxel<TYPE>( op, posInImage ) == 0;
			}
		};
		_proxy prx( op );
		return convertToType( data::ValuePtr<TYPE>::staticID ) && foreachChunk( prx, false );
	}

	/// \returns the number of rows of the image
	size_t getNrOfRows()const;
	/// \returns the number of columns of the image
	size_t getNrOfColumms()const;
	/// \returns the number of slices of the image
	size_t getNrOfSlices()const;
	/// \returns the number of timesteps of the image
	size_t getNrOfTimesteps()const;

	util::fvector4 getFoV()const;
	bool updateOrientationMatrices();
};

/**
 * An Image where all chunks are guaranteed to have a specific type.
 * This not necessarily means, that all chunks in this image are a deep copy of their origin.
 */
template<typename T> class TypedImage: public Image
{
protected:
	TypedImage() {} // to be used only by inheriting classes
public:
	/// cheap copy another Image and make sure all chunks have type T
	TypedImage( const Image &src ): Image( src ) { // ok we just copied the whole image
		//but we want it to be of type T
		convertToType( ValuePtr<T>::staticID );
	}
	/// cheap copy another TypedImage
	TypedImage &operator=( const TypedImage &ref ) { //its already of the given type - so just copy it
		Image::operator=( ref );
		return *this;
	}
	/// cheap copy another Image and make sure all chunks have type T
	TypedImage &operator=( const Image &ref ) { // copy the image, and make sure its of the given type
		Image::operator=( ref );
		convertToType( ValuePtr<T>::staticID );
		return *this;
	}
	void copyToMem( void *dst ) {
		Image::copyToMem<T>( ( T * )dst );
	}
	void copyToMem( void *dst )const {
		Image::copyToMem<T>( ( T * )dst );
	}
};

/**
 * An Image which always uses its own memory and a specific type.
 * Thus, creating this image from another Image allways does a deep copy (and maybe a conversion).
 */
template<typename T> class MemImage: public TypedImage<T>
{
public:
	/**
	 * Copy contructor.
	 * This makes a deep copy of the given image.
	 * The image data are converted to T if necessary.
	 */
	MemImage( const Image &src ) {
		operator=( src );
	}

	/**
	 * Copy operator.
	 * This makes a deep copy of the given image.
	 * The image data are converted to T if necessary.
	 */
	MemImage &operator=( const Image &ref ) { // copy the image, and make sure its of the given type

		Image::operator=( ref ); // ok we just copied the whole image

		//we want deep copies of the chunks, and we want them to be of type T
		struct : _internal::SortedChunkList::chunkPtrOperator {
			std::pair<util::ValueReference, util::ValueReference> scale;
			boost::shared_ptr<Chunk> operator()( const boost::shared_ptr< Chunk >& ptr ) {
				return boost::shared_ptr<Chunk>( new MemChunk<T>( *ptr, scale ) );
			}
		} conv_op;
		conv_op.scale = ref.getScalingTo( ValuePtr<T>::staticID );
		LOG( Debug, info ) << "Computed scaling for conversion from source image: [" << conv_op.scale << "]";

		this->set.transform( conv_op );

		if( ref.isClean() ) {
			this->lookup = this->set.getLookup(); // the lookup table still points to the old chunks
		} else {
			LOG( Debug, info ) << "Copied unclean image. Running reIndex on the copy.";
			this->reIndex();
		}

		return *this;
	}
};

}
}

#endif // IMAGE_H
