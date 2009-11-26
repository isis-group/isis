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

namespace isis{ namespace data
{
/// @cond _hidden
namespace _internal{
struct image_chunk_order: chunk_comarison{
	virtual bool operator() ( const Chunk& a, const Chunk& b )const;
};
}
/// @endcond

class Image;

class Image:
	protected _internal::NDimensional<4>,
	public _internal::PropertyObject
{
public:
	typedef std::set<Chunk,_internal::image_chunk_order> ChunkSet;
	typedef ChunkSet::iterator ChunkIterator;
	enum dimensions{read=0,phase,slice,time,n_dims};
	
private:
	ChunkSet set;
	std::vector<ChunkIterator> lookup;
	bool clean;
	size_t chunkVolume;

	/** 
	 * Computes chunk- and voxel- indices.
	 * The returned chunk-index applies to the lookup-table (getChunkAt), and the voxel-index to this chunk.
	 * Behaviour will be undefined if:
	 * - the image is not clean (not indexed)
	 * - the image is empty
	 * - the coordinates are not in the image
	 *
	 * Additionally an error will be sent if DataDebug is enabled.
	 * \returns a std::pair\<chunk-index,voxel-index\>
	 */
	std::pair<size_t,size_t>
	commonGet(size_t first,size_t second,size_t third,size_t fourth)const;

protected:
	static const isis::util::PropMap::key_type needed[];

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
	size_t getChunkStride(size_t base_stride=1);
	
	///Access a chunk via index (and the lookup table)
	Chunk& getChunkAt(size_t at);
	///Access a chunk via index (and the lookup table)
	const Chunk& getChunkAt(size_t at)const;

public:
	/**
	 * Creates an empty Image object.
	 * \param lt copmarison functor used to sort the chunks 
	 */
	Image(_internal::image_chunk_order lt=_internal::image_chunk_order());
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
	template <typename T> T& voxel(size_t first,size_t second=0,size_t third=0,size_t fourth=0)
	{
		MAKE_LOG(DataDebug);
		if(not clean){
			LOG(DataDebug,util::info)
			<< "Image is not clean. Running reIndex ..." << std::endl;
			reIndex();
		}
		
		const std::pair<size_t,size_t> index=commonGet(first,second,third,fourth);
		util::TypePtr<T> &data=getChunkAt(index.first).asTypePtr<T>();
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
	template <typename T> T voxel(size_t first,size_t second=0,size_t third=0,size_t fourth=0)const
	{
		const std::pair<size_t,size_t> index=commonGet(first,second,third,fourth);
		const util::TypePtr<T> &data=(lookup[index.first])->getTypePtr<T>();
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
	const Chunk& getChunk(size_t first,size_t second=0,size_t third=0,size_t fourth=0)const;
	/**
	 * Get the chunk that contains the voxel at the given coordinates.
	 * If the image is not clean Image::reIndex() will be run.
	 *
	 * \param first The first coordinate in voxel space. Usually the x value.
	 * \param second The second coordinate in voxel space. Usually the y value.
	 * \param third The third coordinate in voxel space. Ususally the z value.
	 * \param fourth The fourth coordinate in voxel space. Usually the time value.
	 * \returns a reference of the chunk that contains the voxel at the given coordinates.
	 */
	Chunk& getChunk(size_t first,size_t second=0,size_t third=0,size_t fourth=0);
					   
	/**
	 * Insert a Chunk into the Image.
	 * The insertion is sorted and unique. So the Chunk will be inserted behind a geometrically "lower" Chunk if there is one.
	 * If there is allready a Chunk at the proposed position this Chunk wont be inserted.
	 *
	 * \param chunk The Chunk to be inserted
	 * \returns true if the Chunk was inserted, false otherwise.
	 */
	bool insertChunk(const Chunk &chunk);
	/**
	 * (Re)computes the image layout and metadata.
	 * The image will be "clean" on success.
	 * \returns true if the image was successfully reindexed, false otherwise. 
	 */
	bool reIndex();

	/**
	 * Get a list of the properties of the chunks for the given key
	 * \param key the name of the property to search for
	 * \param unique when true empty or consecutive duplicates wont be added
	 */
	std::list<util::PropMap::mapped_type> getChunksProperties(const util::PropMap::key_type &key,bool unique=false)const;

	ChunkIterator chunksBegin();
	ChunkIterator chunksEnd();
};

/// @cond _internal
namespace _internal{
class ImageList : public std::list< boost::shared_ptr<Image> >
{
public:

	/**
	 * Create a number of images out of a Chunk list.
	 */
	ImageList(const ChunkList src);

};

}
/// @endcond
}}

#endif // IMAGE_H
