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

private:
	ChunkSet set;
	std::vector<ChunkIterator> lookup;
	bool clean;
	size_t chunkVolume;

	std::pair<size_t,size_t>
	commonGet(size_t first,size_t second,size_t third,size_t fourth)const;

protected:
	static const isis::util::PropMap::key_type needed[];

	/**
	 * Get the ammount of chunks before dimensional switch.
	 * For example for an image of 2D-chunks (slices) getChunkStride(1) will 
	 * get the number of slices (size of third dim) and  getChunkStride(slices) 
	 * will get the number of timesteps
	 * \param base_stride the stride of the dimension before
	 * \returns overall ammount of chunks in one slice of this dimension or 0 in case of an error
	 */
	size_t getChunkStride(size_t base_stride=1);
	Chunk& getChunkAt(size_t at);
	const Chunk& getChunkAt(size_t at)const;

public:
	/**
	 * Creates an empty Image object
	 * \param lt copmarison functor used to sort the chunks 
	 */
	Image(_internal::image_chunk_order lt=_internal::image_chunk_order());
	/**
	 * This method returns a reference to the voxel value at the given coordinates.
	 *
	 * The voxel reference provides reading and writing access to the refered
	 * value.
	 *
	 * \param first The first coordinate in voxel space. Usually the x value.
	 * \param second The second coordinate in voxel space. Usually the y value.
	 * \param third The third coordinate in voxel space. Ususally the z value.
	 * \param fourth The fourth coordinate in voxel space. Usually the time value.
	 *
	 * \return A reference to the addressed voxel value. Reading and writing access
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
	 * This method returns a reference to the voxel value at the given coordinates.
	 *
	 * The const voxel reference provides only reading access to the refered
	 * value.
	 *
	 * \param first The first coordinate in voxel space. Usually the x value.
	 * \param second The second coordinate in voxel space. Usually the y value.
	 * \param third The third coordinate in voxel space. Ususally the z value.
	 * \param fourth The fourth coordinate in voxel space. Usually the time value.
	 *
	 * \return A reference to the addressed voxel value. Only reading access is provided
	 */
	template <typename T> T voxel(size_t first,size_t second=0,size_t third=0,size_t fourth=0)const
	{
		const std::pair<size_t,size_t> index=commonGet(first,second,third,fourth);
		const util::TypePtr<T> &data=(lookup[index.first])->getTypePtr<T>();
		return data[index.second];
	}

	/**
	 * Returns a copy of the chunk that contains the voxel at the given coordinates.
	 *
	 * \param first The first coordinate in voxel space. Usually the x value.
	 * \param second The second coordinate in voxel space. Usually the y value.
	 * \param third The third coordinate in voxel space. Ususally the z value.
	 * \param fourth The fourth coordinate in voxel space. Usually the time value.
	 *
	 *
	 */
	const Chunk& getChunk(size_t first,size_t second=0,size_t third=0,size_t fourth=0)const;
	Chunk& getChunk(size_t first,size_t second=0,size_t third=0,size_t fourth=0);
					   
	/**
	 * Inserts a Chunk into the Image.
	 * The insertion is sorted and unique. So the Chunk will be inserted behind a geometrically "lower" Chunk if there is one.
	 * If there is allready a Chunk at the proposed position this Chunk wont be inserted.
	 *
	 * \param chunk The Chunk to be inserted
	 * \returns true if the Chunk was inserted
	 */
	bool insertChunk(const Chunk &chunk);
	bool reIndex();
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
