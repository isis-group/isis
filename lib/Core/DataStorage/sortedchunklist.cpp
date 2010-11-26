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

#include "DataStorage/sortedchunklist.hpp"

namespace isis
{
namespace data
{
namespace _internal
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// sorting algorithm implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SortedChunkList::scalarPropCompare::scalarPropCompare( const util::PropMap::KeyType &prop_name ): propertyName( prop_name ) {}

bool SortedChunkList::posCompare::operator()( const util::fvector4 &posA, const util::fvector4 &posB ) const
{
	if ( posA.lexical_less_reverse( posB ) ) { //if chunk is "under" the other - put it there
		LOG( Debug, verbose_info ) << "Successfully sorted chunks by position (" << posA << " below " << posB << ")";
		return true;
	}

	return false;
}
bool SortedChunkList::scalarPropCompare::operator()( const isis::util::PropertyValue &a, const isis::util::PropertyValue &b ) const
{
	const util::_internal::TypeBase &aScal = *a;
	const util::_internal::TypeBase &bScal = *b;

	if ( aScal.lt( bScal ) ) {
		LOG( Debug, verbose_info ) << "Successfully sorted chunks by " << propertyName << " (" << aScal.toString( false ) << " before " << bScal.toString( false ) << ")";
		return true;
	} else
		return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Chunk operators
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SortedChunkList::chunkPtrOperator::~chunkPtrOperator() {}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SortedChunkList implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// constructor
SortedChunkList::SortedChunkList( util::PropMap::KeyType fvectorPropName, util::PropMap::KeyType comma_separated_equal_props ):
	equalProps( util::string2list<util::PropMap::KeyType>( comma_separated_equal_props, ',' ) )
{}


// low level finding
boost::shared_ptr<Chunk> SortedChunkList::secondaryFind( const util::PropertyValue &key, SortedChunkList::SecondaryMap &map )
{
	const SecondaryMap::iterator found = map.find( key );
	return found != map.end() ? found->second : boost::shared_ptr<Chunk>();
}
SortedChunkList::SecondaryMap *SortedChunkList::primaryFind( const util::fvector4 &key )
{
	const PrimaryMap::iterator found = chunks.find( key );
	return found != chunks.end() ? &found->second : NULL;
}

// low level insert
std::pair<boost::shared_ptr<Chunk>, bool> SortedChunkList::secondaryInsert( SecondaryMap &map, const Chunk &ch )
{
	util::PropMap::KeyType propName = map.key_comp().propertyName;

	if( ch.hasProperty( propName ) ) {
		//check, if there is already a chunk
		boost::shared_ptr<Chunk> &pos = map[ch.propertyValue( propName )];
		bool inserted = false;

		//if not. put oures there
		if( !pos ) {
			pos.reset( new Chunk( ch ) );
			inserted = true;
		}

		assert( pos ); // here it must have some content
		return std::make_pair( pos, inserted );
	} else {
		LOG( Runtime, warning ) << "Cannot insert chunk. It's lacking the property " << util::MSubject( propName ) << " which is needed for primary sorting";
		return std::pair<boost::shared_ptr<Chunk>, bool>( boost::shared_ptr<Chunk>(), false );
	}
}
std::pair<boost::shared_ptr<Chunk>, bool> SortedChunkList::primaryInsert( const Chunk &ch )
{
	LOG_IF( secondarySort.empty(), Debug, error ) << "There is no known secondary sorting left. Chunksort will fail.";
	assert( ch.isValid() );
	// compute the position of the chunk in the image space
	// we dont have this position, but we have the position in scanner-space (indexOrigin)
	const util::fvector4 &origin = ch.propertyValue( "indexOrigin" )->castTo<util::fvector4>();
	// and we have the transformation matrix
	// [ readVec ]
	// [ phaseVec]
	// [ sliceVec]
	// [ 0 0 0 1 ]
	const util::fvector4 &readVec = ch.propertyValue( "readVec" )->castTo<util::fvector4>();
	const util::fvector4 &phaseVec = ch.propertyValue( "phaseVec" )->castTo<util::fvector4>();
	util::fvector4 sliceVec;

	if( ch.hasProperty( "sliceVec" ) )
		sliceVec = ch.propertyValue( "sliceVec" )->castTo<util::fvector4>();
	else {
		sliceVec = util::fvector4(
					   readVec[1] * phaseVec[2] - readVec[2] * phaseVec[1],
					   readVec[2] * phaseVec[0] - readVec[0] * phaseVec[2],
					   readVec[0] * phaseVec[1] - readVec[1] * phaseVec[0]
				   );
	}

	const util::fvector4 key( origin.dot( readVec ), origin.dot( phaseVec ), origin.dot( sliceVec ), origin[3] );

	// this is actually not the complete transform (it lacks the scaling for the voxel size), but its enough
	const scalarPropCompare &secondaryComp = secondarySort.top();

	// get the reference of the secondary map for "key" (create and insert a new if neccessary)
	SecondaryMap &subMap = chunks.insert( std::make_pair( key, SecondaryMap( secondaryComp ) ) ).first->second;

	// run insert on that
	return secondaryInsert( subMap, ch ); // insert ch into the right secondary map
}

// high level insert
bool SortedChunkList::insert( const Chunk &ch )
{
	LOG_IF( secondarySort.empty(), Debug, error ) << "Inserting will fail without any secondary sort. Use chunks.addSecondarySort at least once.";

	if( !empty() ) {
		// compare some attributes of the first chunk and the one which shall be inserted
		Chunk &first = *( chunks.begin()->second.begin()->second );

		if ( first.getSizeAsVector() != ch.getSizeAsVector() ) { // if they have different size - do not insert
			LOG( Debug, verbose_info )
					<< "Ignoring chunk with different size. (" << ch.getSizeAsString() << "!=" << first.getSizeAsString() << ")";
			return false;
		}

		BOOST_FOREACH( util::PropMap::KeyType & ref, equalProps ) { // check all properties which where given to the constructor of the list
			// if at least one of them has the property and they are not equal - do not insert
			if ( ( first.hasProperty( ref ) || ch.hasProperty( ref ) ) && first.propertyValue( ref ) != ch.propertyValue( ref ) ) {
				LOG( Debug, verbose_info )
						<< "Ignoring chunk with different " << ref << ". (" << ch.propertyValue( ref ) << "!=" << first.propertyValue( ref ) << ")";
				return false;
			}
		}
	} else {
		LOG( Debug, verbose_info ) << "Inserting 1st chunk";
		std::stack<scalarPropCompare> backup = secondarySort;

		while( !ch.hasProperty( secondarySort.top().propertyName ) ) {
			const util::PropMap::KeyType temp = secondarySort.top().propertyName;

			if ( secondarySort.size() > 1 ) {
				secondarySort.pop();
			} else {
				LOG( Debug, warning )
						<< "First chunk is missing the last secondary sort-property fallback (" << util::MSubject( temp ) << "), won't insert.";
				secondarySort = backup;
				return false;
			}
		}

		LOG( Debug, info )  << "Using " << secondarySort.top().propertyName << " for secondary sorting, determined by the first chunk";
	}

	const util::PropMap::KeyType &prop2 = secondarySort.top().propertyName;

	std::pair<boost::shared_ptr<Chunk>, bool> inserted = primaryInsert( ch );

	LOG_IF( inserted.first && !inserted.second, Debug, verbose_info )
			<< "Not inserting chunk because there is allready a Chunk at the same position (" << ch.propertyValue( "indexOrigin" ) << ") with the equal property "
			<< std::make_pair( prop2, ch.propertyValue( prop2 ) );

	LOG_IF(
		inserted.first && !inserted.second &&
		ch.hasProperty( "source" ) && inserted.first->hasProperty( "source" ) &&
		!( ch.propertyValue( "source" ) == inserted.first->propertyValue( "source" ) ),
		Debug, verbose_info )
			<< "The conflicting chunks where " << ch.propertyValue( "source" ).toString( false ) << " and " << inserted.first->propertyValue( "source" ).toString( false );

	return inserted.second;
}

void SortedChunkList::addSecondarySort( const util::PropMap::KeyType &cmp )
{
	secondarySort.push( scalarPropCompare( cmp ) );
}
bool SortedChunkList::empty()const
{
	return chunks.empty() || chunks.begin()->second.empty(); // if there is no subMap or nothing in the first subMap ... its empty
}
void SortedChunkList::clear()
{
	chunks.clear();
}
bool SortedChunkList::isRectangular()
{
	if( empty() )return true;

	size_t images = getHorizontalSize();
	BOOST_FOREACH( PrimaryMap::reference outer, chunks ) {
		if( outer.second.size() != images )
			return false;
	}
	return true;
}
size_t SortedChunkList::getHorizontalSize()
{
	if( empty() )return 0;
	else return chunks.begin()->second.size();
}

std::vector< boost::shared_ptr< Chunk > > SortedChunkList::getLookup()
{
	LOG_IF( !isRectangular(), Debug, error ) << "Running getLookup on an non rectangular chunk-list is not defined";

	if( !empty() ) {
		PrimaryMap::iterator iP = chunks.begin();
		const size_t horizontal = chunks.size();
		const size_t vertical = iP->second.size();
		std::vector< boost::shared_ptr< Chunk > > ret( horizontal * vertical );

		for( size_t h = 0; h < horizontal; h++, iP++ ) { // outer loop iterates horizontaly (through the primary sorting)
			assert( iP != chunks.end() );
			SecondaryMap::iterator iS = iP->second.begin();

			for( size_t v = 0; v < vertical; v++, iS++ ) { // inner loop iterates verticaly (through the secondary sorting)
				assert( iS != iP->second.end() );
				ret[h+v *horizontal] = iS->second; // insert horizontally - primary sorting is the fastest running index (read the sorting matrix horizontaly)
			}
		}

		return ret;
	} else
		return std::vector< boost::shared_ptr< Chunk > >();
}

void SortedChunkList::transform( chunkPtrOperator &op )
{
	BOOST_FOREACH( PrimaryMap::reference outer, chunks ) {
		BOOST_FOREACH( SecondaryMap::reference inner, outer.second ) {
			inner.second = op( inner.second );
		}
	}
}


}
}
}
