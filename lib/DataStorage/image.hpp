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
	std::vector<boost::weak_ptr<Chunk> > lookup;
private:
	bool clean;
	size_t chunkVolume;

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
	///Access a chunk via index (and the lookup table)
	Chunk &chunkAt( size_t at );
	///Access a chunk via index (and the lookup table)
	const Chunk &chunkAt( size_t at )const;

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
		const TypePtr<T> &data = chunkAt( index.first ).getTypePtr<T>();
		return data[index.second];
	}

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

	std::vector<boost::shared_ptr<Chunk> > getChunkList();

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
	template<typename TYPE> Chunk getChunkAs( TYPE min, TYPE max, size_t first, size_t second = 0, size_t third = 0, size_t fourth = 0 )const {
		const Chunk &ref = getChunk( first, second, third, fourth );

		if( TypePtr<TYPE>::staticID == ref.typeID() ) { //OK its the right type - just return that
			return ref;
		} else { //we have to to a conversion
			return MemChunk<TYPE>( ref, util::Type<TYPE>( min ), util::Type<TYPE>( max ) );
		}
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

	bool empty()const;

	/**
	 * Get a list of the properties of the chunks for the given key
	 * \param key the name of the property to search for
	 * \param unique when true empty or consecutive duplicates wont be added
	 */
	std::list<util::PropertyValue> getChunksProperties( const std::string &key, bool unique = false )const;

	/// get the size of every voxel (in bytes)
	size_t bytes_per_voxel()const;

	template<typename T> void getMinMax( T &min, T &max )const {
		util::check_type<T>();// works only for T from _internal::types
		util::_internal::TypeBase::Reference _min, _max;
		getMinMax( _min, _max );
		min = _min->as<T>();
		max = _max->as<T>();
	}
	void getMinMax( util::_internal::TypeBase::Reference &min, util::_internal::TypeBase::Reference &max )const;
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

};

template<typename T> class MemImage: public Image
{
public:
	MemImage( const Image &src ): Image( src ) { // ok we just copied the whole image
		//we want copies of the chunks, and we want them to be of type T
		struct : _internal::SortedChunkList::chunkPtrOperator {
			util::_internal::TypeBase::Reference min, max;
			void operator()( boost::shared_ptr< Chunk >& ptr ) {
				ptr.reset( new MemChunk<T>( *ptr, *min, *max ) );
			}
		} conv_op;
		src.getMinMax( conv_op.min, conv_op.max );
		LOG( Debug, info ) << "Computed value range of the source image: [" << conv_op.min << ".." << conv_op.max << "]";
		set.forall_ptr( conv_op );
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
