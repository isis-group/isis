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

namespace isis{ namespace data
{

class Image;

namespace _internal{
struct image_lt{
	bool operator() (const ::isis::data::Image& a, const ::isis::data::Image& b) const;
};
	
}
class Image: public std::set<_internal::ChunkReference,_internal::image_lt>, _internal::NDimensional<4>{
	isis::util::PropMap properties;
public:
	Image(_internal::image_lt  lt=_internal::image_lt());
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
	template <typename T> T& voxel(
		const size_t &first,
		const size_t &second,
		const size_t &third,
		const size_t &fourth);

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
	template <typename T> T voxel(
		const size_t &first,
		const size_t &second,
		const size_t &third,
		const size_t &fourth)const;

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
	template <typename T> Chunk<T> getChunk(
		const size_t &first,
		const size_t &second,
		const size_t &third,
		const size_t &fourth)const;

	/**
	 * This method merges the content of a ChunkList eith the Image's internal chunks.
	 *
	 * The given ChunkList will be traversed and each chunk will be inserted in the right
	 * place according to its properties. The chunks properties must be set correctly
	 * to match the image's voxel sort order.
	 *
	 * Maybe in the near future, this function throws an exception.
	 *
	 * \param chunks A list of chunks that should be merged with the image's internal
	 *  chunk list.
	 */
	void insertChunkList(const ChunkList &chunks);
	void insertChunk(const _internal::ChunkReference chunk);
	
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
