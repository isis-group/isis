/*
    Copyright (C) 2010  reimer@cbs.mpg.de

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

#ifndef SORTEDCHUNKLIST_HPP
#define SORTEDCHUNKLIST_HPP

#include "chunk.hpp"
#include "../CoreUtils/vector.hpp"
#include <stack>
#include <boost/shared_ptr.hpp>

/// @cond _internal
namespace isis
{
namespace data
{
namespace _internal
{

/*
 * This class sorts the inserted chunks by the given properties while construction.
 * The rules of the sorting depends on the upper level using these chunkLists later on.
 * See the special classes, e.g. MRImage to find details about sorting criterias.
 */
class SortedChunkList
{
public:
	struct scalarPropCompare {
		util::PropertyMap::KeyType propertyName;
		scalarPropCompare( const util::PropertyMap::KeyType &prop_name );
		bool operator()( const util::PropertyValue &a, const util::PropertyValue &b ) const;
	};
	struct posCompare {
		bool operator()( const util::fvector4 &a, const util::fvector4 &b ) const;
	};
	struct chunkPtrOperator {
		virtual boost::shared_ptr<Chunk> operator()( const boost::shared_ptr<Chunk> &ptr ) = 0;
		virtual ~chunkPtrOperator();
	};
private:
	typedef std::map<util::PropertyValue, boost::shared_ptr<Chunk>, scalarPropCompare> SecondaryMap;
	typedef std::map<util::fvector4, SecondaryMap, posCompare> PrimaryMap;

	std::stack<scalarPropCompare> secondarySort;
	posCompare primarySort;
	PrimaryMap chunks;

	// low level finding
	boost::shared_ptr<Chunk> secondaryFind( const util::PropertyValue &key, SecondaryMap &map );
	SecondaryMap *primaryFind( const util::fvector4 &key );

	// low level inserting
	std::pair<boost::shared_ptr<Chunk>, bool> secondaryInsert( SecondaryMap &map, const Chunk &ch );
	std::pair<boost::shared_ptr<Chunk>, bool> primaryInsert( const Chunk &ch );

	std::list<util::PropertyMap::PropPath> equalProps;
public:

	//initialisation
	/**
	 * Creates a sorted list and sets primary sorting as well as properties which should be equal across all chunks.
	 */
	SortedChunkList( util::PropertyMap::KeyType comma_separated_equal_props );

	/**
	 * Adds a property for secondary sorting.
	 * At least one secondary sorting is needed.
	 */
	void addSecondarySort( const util::PropertyMap::KeyType &cmp );

	// utils

	///runs op on all entries of the list (the order is not defined) and replaces the entries by the return value
	void transform( chunkPtrOperator &op );

	/// Tries to insert a chunk (a cheap copy of the chunk is done when inserted)
	bool insert( const Chunk &ch );

	/// \returns true if there is no chunk in the list
	bool isEmpty()const;

	/// Empties the list.
	void clear();

	/// \returns a ordered vector of pointers to the chunks in the list
	std::vector<boost::shared_ptr<Chunk> > getLookup();

	/// \returns true if the list is rectangular (the amount of secondary sorted entries is equal across all primary entries)
	bool isRectangular();

	/// \returns the amount secondary sorted entries
	size_t getHorizontalSize();
};


}
}
}
/// @endcond _internal

#endif // SORTEDCHUNKLIST_HPP
