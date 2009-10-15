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
/// @cond _hidden
namespace _internal{
struct image_chunk_order: binary_chunk_comarison{
	virtual bool operator() ( const Chunk& a, const Chunk& b );
};
}
/// @endcond

class Image;

class Image:
	public std::set<Chunk,_internal::image_chunk_order>,
	protected _internal::NDimensional<4>,
	public _internal::PropertyObject
{
	isis::util::PropMap properties;
protected:
	static const isis::util::PropMap::key_type needed[];
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
	Chunk getChunk(
		const size_t &first,
		const size_t &second,
		const size_t &third,
		const size_t &fourth)const;

	/**
	 * Inserts a Chunk into the Image.
	 * The insertion is sorted and unique. So the Chunk will be inserted behind a geometrically "lower" Chunk if there is one.
	 * If there is allready a Chunk at the proposed position this Chunk wont be inserted.
	 *
	 * \param chunk The Chunk to be inserted
	 * \returns true if the Chunk was inserted
	 */
	bool insertChunk(const Chunk &chunk);
	
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
