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
#include "../util/vector.hpp"
#include <stack>
#include <memory>

using boost::optional;

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
		util::PropertyMap::key_type propertyName;
		scalarPropCompare( const util::PropertyMap::key_type &prop_name );
		bool operator()( const util::PropertyValue &a, const util::PropertyValue &b ) const;
	};
	struct posCompare {
		bool operator()( const util::fvector3 &a, const util::fvector3 &b ) const;
	};
	struct chunkPtrOperator {
		virtual std::shared_ptr<Chunk> operator()( const std::shared_ptr<Chunk> &ptr ) = 0;
		virtual ~chunkPtrOperator();
	};
	struct getproplist:public std::set<util::PropertyValue>
	{
		util::PropertyMap::PropPath name;
		getproplist(util::PropertyMap::PropPath _name):name(_name){}
		template<typename T> void operator()(const std::shared_ptr<T> &p){operator()(*p);}
		void operator()(const util::PropertyMap &c);
	};
	
private:
	typedef std::map<util::PropertyValue, std::shared_ptr<Chunk>, scalarPropCompare> SecondaryMap;
	typedef std::map<util::fvector3, SecondaryMap, posCompare> PrimaryMap;

	std::stack<scalarPropCompare> secondarySort;
	PrimaryMap chunks;

	// low level finding
	std::shared_ptr<Chunk> secondaryFind( const util::PropertyValue &key, SecondaryMap &map );
	SecondaryMap *primaryFind( const util::fvector3 &key );

	// low level inserting
	std::pair<std::shared_ptr<Chunk>, bool> secondaryInsert( SecondaryMap &map, const Chunk &ch );
	std::pair<std::shared_ptr<Chunk>, bool> primaryInsert( const Chunk &ch );

	std::list<util::PropertyMap::PropPath> equalProps;
	
	bool insert_impl( const Chunk &ch );

public:

	//initialisation
	/**
	 * Creates a sorted list and sets primary sorting as well as properties which should be equal across all chunks.
	 */
	SortedChunkList( util::PropertyMap::key_type comma_separated_equal_props );

	/**
	 * Adds a property for secondary sorting.
	 * At least one secondary sorting is needed.
	 */
	void addSecondarySort( const util::PropertyMap::key_type &cmp );

	// utils

	///runs op on all entries of the list (the order is not defined) and replaces the entries by the return value
	void transform( chunkPtrOperator &op );
	
	///runs op on all entries of the list (the order is not defined)
	template<typename T> void forall( T &func)const
	{
		for( PrimaryMap::const_reference outer : chunks ) {
			for( SecondaryMap::const_reference inner : outer.second ) {
				func( inner.second );
			}
		}
	}
	

	/// Tries to insert a chunk (a cheap copy of the chunk is done when inserted)
	bool insert( const Chunk &ch );

	/// \returns true if there is no chunk in the list
	bool isEmpty()const;

	/// Empties the list.
	void clear();

	/// \returns a ordered vector of pointers to the chunks in the list
	std::vector<std::shared_ptr<Chunk> > getLookup();

	/**
	 * Get the list of the different lengths of the primary sorted "columns".
	 * This list should only hold one value if the amount of secondary sorted entries is equal across all primary entries. Which it is in a consistent dataset.
	 * \returns a one element set with the amount of secondary sorted entries across all primary entries if image is consistent
	 * \returns a set with the different amounts of secondary sorted entries across all primary entries otherwise
	 **/
	std::set<size_t> getShape();

	/**
	 * Make image rectangular by dropping secondary sorted entries from all primary entries until their amount is equal.
	 * \returns amount of dropped entries
	 **/
	size_t makeRectangular(optional< util::slist& > rejected=optional< util::slist& >());

	/// \returns the amount secondary sorted entries
	size_t getHorizontalSize();
	
	/**
	 * Generate a string identifying the stored chunks
	 * The identifier is made of
	 * - sequenceNumber
	 * - sequenceDescription if available
	 * - the common path of all chunk-sources (or the source file, if there is only one) if withpath is true
	 * - sequenceStart if available
	 * \param withpath add the common path of all sources to the identifying string
	 */
	std::string identify( bool withpath = true, bool withdate=true, 
						  getproplist source=getproplist("source"),
						  getproplist seqNum=getproplist("sequenceNumber"),
						  getproplist seqDesc=getproplist("sequenceDescription"),
						  getproplist seqStart=getproplist("sequenceStart") )const;
};


}
}
}
/// @endcond _internal

#endif // SORTEDCHUNKLIST_HPP
