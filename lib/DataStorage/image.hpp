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
#include <stack>
#include "sortedchunklist.hpp"

namespace isis
{
namespace data
{

class Image:
	public _internal::NDimensional<4>,
	public util::PropMap
{
public:
	enum orientation {axial, reversed_axial, sagittal, reversed_sagittal, coronal, reversed_coronal};

protected:
	_internal::SortedChunkList set;
	std::vector<boost::shared_ptr<Chunk> > lookup;
private:
	bool clean;
	size_t chunkVolume;

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
		LOG_IF( set.empty(), Debug, error )
				<< "Getting data from a empty image will result in undefined behavior.";
		LOG_IF( !rangeCheck( idx ), Debug, isis::error )
				<< "Index " << util::list2string( idx, idx + 4, "|" ) << " is out of range (" << sizeToString() << ")";
		const size_t index = dim2Index( idx );
		return std::make_pair( index / chunkVolume, index % chunkVolume );
	}


protected:
	static const char *needed;

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
	template<typename T> struct makeTypedChunk: _internal::SortedChunkList::chunkPtrOperator {
		util::TypeReference min, max;
		boost::shared_ptr<Chunk> operator()(const boost::shared_ptr< Chunk >& ptr){
			return boost::shared_ptr<Chunk>( ptr->is<T>() ?
				new Chunk(*ptr) : // replace by a cheap copy - type is right
				new MemChunk<T>( *ptr, *min, *max ) // replace by a converted deep copy
			);
		}
	};
	/**
	 * Access a chunk via index (and the lookup table)
	 * The Chunk will only have metadata which are unique to it - so it might be invalid
	 * (run join on it using the image as parameter to insert all non-unique-metadata).
	 */
	Chunk &chunkAt( size_t at );
public:
	/**
	 * Creates an empty Image object.
	 */
	Image();
	/**
	 * This method returns a reference to the voxel value at the given coordinates.
	 *
	 * The voxel reference provides reading and writing access to the refered
	 * value.
	 *
	 * If the image is not clean, reIndex will be run.
	 *
	 * \param first The first coordinate in voxel space. Usually the x value / the read-encoded position..
	 * \param second The second coordinate in voxel space. Usually the y value / the phase-encoded position.
	 * \param third The third coordinate in voxel space. Ususally the z value / the time-encoded position.
	 * \param fourth The fourth coordinate in voxel space. Usually the time value.
	 *
	 * \returns A reference to the addressed voxel value. Reading and writing access
	 * is provided.
	 */
	template <typename T> T &voxel( size_t first, size_t second = 0, size_t third = 0, size_t fourth = 0 ) {
		if ( ! clean ) {
			LOG( Debug, info ) << "Image is not clean. Running reIndex ...";

			if( !reIndex() ) {
				LOG( Runtime, error ) << "Reindexing failed -- undefined behavior ahead ...";
			}
		}

		const std::pair<size_t, size_t> index = commonGet( first, second, third, fourth );

		TypePtr<T> &data = chunkAt( index.first ).asTypePtr<T>();

		return data[index.second];
	}

	/**
	 * Get the value of the voxel value at the given coordinates.
	 *
	 * The voxel reference provides reading and writing access to the refered
	 * value.
	 *
	 * \param first The first coordinate in voxel space. Usually the x value / the read-encoded position..
	 * \param second The second coordinate in voxel space. Usually the y value / the phase-encoded position.
	 * \param third The third coordinate in voxel space. Ususally the z value / the time-encoded position.
	 * \param fourth The fourth coordinate in voxel space. Usually the time value.
	 *
	 * \returns A reference to the addressed voxel value. Only reading access is provided
	 */
	template <typename T> T voxel( size_t first, size_t second = 0, size_t third = 0, size_t fourth = 0 )const {
		const std::pair<size_t, size_t> index = commonGet( first, second, third, fourth );
		const TypePtr<T> &data = chunkPtrAt( index.first )->getTypePtr<T>();
		return data[index.second];
	}


	/**
	 * Get the typeID of the chunk with "biggest" type
	 */
	unsigned short typeID() const;

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
	 * \param second The second coordinate in voxel space. Usually the y value / the phase-encoded position.
	 * \param third The third coordinate in voxel space. Ususally the z value / the slice-encoded position.
	 * \param fourth The fourth coordinate in voxel space. Usually the time value.
	 * \returns a copy of the chunk that contains the voxel at the given coordinates.
	 * (Reminder: Chunk-copies are cheap, so the data are NOT copied)
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
	 * \returns a copy of the chunk that contains the voxel at the given coordinates.
	 * (Reminder: Chunk-copies are cheap, so the data are NOT copied)
	 */
	Chunk getChunk( size_t first, size_t second = 0, size_t third = 0, size_t fourth = 0, bool copy_metadata = true );

	/**
	 * Get a sorted list of pointers to the chunks of the image.
	 * Note: this chunks only have metadata which are unique to them - so they might be invalid.
	 * (run join on copies of them using the image as parameter to insert all non-unique-metadata).
	 */
	std::vector<boost::shared_ptr<Chunk> > getChunkList();
	/// \copydoc getChunkList
	std::vector<boost::shared_ptr<const Chunk> > getChunkList()const;

	/**
	 * Get the chunk that contains the voxel at the given coordinates in the given type.
	 * If the accordant chunk has type T a cheap copy is returned.
	 * Otherwise a MemChunk of the requested type is created from it.
	 * In this case min and max are used as value range for the conversion.
	 *
	 * \param min The minimum of the value-range of the image (use getMinMax to get this).
	 * \param max The maximum of the value-range of the image (use getMinMax to get this).
	 * \param first The first coordinate in voxel space. Usually the x value.
	 * \param second The second coordinate in voxel space. Usually the y value.
	 * \param third The third coordinate in voxel space. Ususally the z value.
	 * \param fourth The fourth coordinate in voxel space. Usually the time value.
	 * \returns a chunk contains the (maybe converted) voxel value at the given coordinates.
	 */
	template<typename TYPE> Chunk getChunkAs( const util::_internal::TypeBase &min, const  util::_internal::TypeBase &max, size_t first, size_t second = 0, size_t third = 0, size_t fourth = 0 )const {
		const Chunk &ref = getChunk( first, second, third, fourth );
		if( ref.is<TYPE>() ) { //OK its the right type - just return that
			return ref;
		} else { //we have to do a conversion
			return MemChunk<TYPE>( ref, min, max );
		}
	}
	/**
	* Get the chunk that contains the voxel at the given coordinates in the given type.
	* If the accordant chunk has type T a cheap copy is returned.
	* Otherwise a MemChunk of the requested type is created from it.
	* In this case the minimum and maximum values of the image are computed and used for the MemChunk constructor.
	*
	* \param first The first coordinate in voxel space. Usually the x value.
	* \param second The second coordinate in voxel space. Usually the y value.
	* \param third The third coordinate in voxel space. Ususally the z value.
	* \param fourth The fourth coordinate in voxel space. Usually the time value.
	* \returns a chunk contains the (maybe converted) voxel value at the given coordinates.
	*/
	template<typename TYPE> Chunk getChunkAs( size_t first, size_t second = 0, size_t third = 0, size_t fourth = 0 )const {
		const boost::shared_ptr<Chunk> &ptr = chunkPtrAt( commonGet( first, second, third, fourth ).first);
		return makeTypedChunk<TYPE>()(ptr);
	}
	
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
	bool empty()const;

	/**
	 * Get a list of the properties of the chunks for the given key
	 * \param key the name of the property to search for
	 * \param unique when true empty or consecutive duplicates wont be added
	 */
	std::list<util::PropertyValue> getChunksProperties( const std::string &key, bool unique = false )const;

	/// get the size of every voxel (in bytes)
	size_t bytes_per_voxel()const;

	/**
	 * Get the maximum and the minimum voxel value of the image.
	 * The results are stored as type T, if they dont fit an error ist send.
	 */
	template<typename T> void getMinMax( T &min, T &max )const {
		util::check_type<T>();// works only for T from _internal::types
		util::TypeReference _min, _max;
		getMinMax( _min, _max );
		min = _min->as<T>();
		max = _max->as<T>();
	}

	/// Get the maximum and the minimum voxel value of the image and store them as Type-object in the given references.
	void getMinMax( util::TypeReference& min, util::TypeReference& max )const;

	/**
	 * Compares the voxel-values of this image to the given.
	 * \returns the amount of the different voxels
	 */
	size_t cmp( const Image &comp )const;
	
	orientation getMainOrientation()const;
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
	 * Copy all voxel data of the image into memory.
	 * If neccessary a conversion into T is done using min/max of the image.
	 */
	template<typename T> void copyToMem(T *dst)const{
		util::TypeReference min, max;
		getMinMax(min,max);
		// we could do this using makeTypedChunk - but this does not any additional temporary memory
		BOOST_FOREACH(boost::shared_ptr<Chunk> &ref,lookup){ 
			// wrap the raw memory at the "cursor" into an non-deleting TypePtr of the length of the chunk
			TypePtr<T> dstPtr(dst,ref->volume(),TypePtr<T>::NonDeleter()); 
			ref->getTypePtrBase().convertTo(dstPtr,min,max); // copy-convert the data into dstPtr
			dst+=dstPtr.len();// increment the cursor
		}
	}
};

template<typename T> class MemImage: public Image
{
public:
	MemImage( const Image &src ): Image( src ) { // ok we just copied the whole image
		//we want copies of the chunks, and we want them to be of type T
		struct : _internal::SortedChunkList::chunkPtrOperator {
			util::TypeReference min, max;
			boost::shared_ptr<Chunk> operator()(const boost::shared_ptr< Chunk >& ptr){
				return boost::shared_ptr<Chunk>( new MemChunk<T>( *ptr, *min, *max ) );
			}
		} conv_op;
		src.getMinMax( conv_op.min, conv_op.max );
		LOG( Debug, info ) << "Computed value range of the source image: [" << conv_op.min << ".." << conv_op.max << "]";
		set.transform( conv_op );
		lookup = set.getLookup(); // the lookup table still points to the old chunks
	}
};
template<typename T> class TypedImage: public Image
{
public:
	TypedImage( const Image &src ): Image( src ) { // ok we just copied the whole image
		//we want chunks, and we want them to be of type T
		makeTypedChunk<T> conv_op;
		src.getMinMax( conv_op.min, conv_op.max );
		LOG( Debug, info ) << "Computed value range of the source image: [" << conv_op.min << ".." << conv_op.max << "]";
		set.transform( conv_op ); // apply op to all chunks
		lookup = set.getLookup(); // the lookup table still points to the old chunks
	}
};

class ImageList : public std::list< boost::shared_ptr<Image> >
{
public:
	ImageList();
	/**
	 * Create a number of images out of a Chunk list.
	 */
	ImageList( ChunkList src );
};
}
}

#endif // IMAGE_H
