/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

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
class Image: public std::set<_internal::ChunkReference,_internal::image_lt>{
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
