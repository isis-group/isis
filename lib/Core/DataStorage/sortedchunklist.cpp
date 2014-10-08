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

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#include "sortedchunklist.hpp"

/// @cond _internal
namespace isis
{
namespace data
{
namespace _internal
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// sorting algorithm implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SortedChunkList::scalarPropCompare::scalarPropCompare( const util::PropertyMap::key_type &prop_name ): propertyName( prop_name ) {}

bool SortedChunkList::posCompare::operator()( const util::fvector3 &posA, const util::fvector3 &posB ) const
{
	if ( posA.lexical_less_reverse( posB ) ) { //if chunk is "under" the other - put it there
		LOG( Debug, verbose_info ) << "Successfully sorted chunks by in-image position (" << posA << " below " << posB << ")";
		return true;
	}

	return false;
}
bool SortedChunkList::scalarPropCompare::operator()( const isis::util::PropertyValue &a, const isis::util::PropertyValue &b ) const
{
	const util::ValueBase &aScal = a.front();
	const util::ValueBase &bScal = b.front();

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
SortedChunkList::SortedChunkList( util::PropertyMap::key_type comma_separated_equal_props )
{
	const std::list< isis::util::PropertyMap::key_type > p_list = util::stringToList<util::PropertyMap::key_type>( comma_separated_equal_props, ',' );
	equalProps.insert( equalProps.end(), p_list.begin(), p_list.end() );
}


// low level finding
std::shared_ptr<Chunk> SortedChunkList::secondaryFind( const util::PropertyValue &key, SortedChunkList::SecondaryMap &map )
{
	const SecondaryMap::iterator found = map.find( key );
	return found != map.end() ? found->second : std::shared_ptr<Chunk>();
}
SortedChunkList::SecondaryMap *SortedChunkList::primaryFind( const util::fvector3 &key )
{
	const PrimaryMap::iterator found = chunks.find( key );
	return found != chunks.end() ? &found->second : NULL;
}

// low level insert
std::pair<std::shared_ptr<Chunk>, bool> SortedChunkList::secondaryInsert( SecondaryMap &map, const Chunk &ch )
{
	util::PropertyMap::key_type propName = map.key_comp().propertyName;

	if( ch.hasProperty( propName ) ) {
		//check, if there is already a chunk
		std::shared_ptr<Chunk> &pos = map[ch.property( propName )];
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
		return std::pair<std::shared_ptr<Chunk>, bool>( std::shared_ptr<Chunk>(), false );
	}
}
std::pair<std::shared_ptr<Chunk>, bool> SortedChunkList::primaryInsert( const Chunk &ch )
{
	static const util::PropertyMap::PropPath rowVecProb( "rowVec" ), columnVecProb( "columnVec" ), sliceVecProb( "sliceVec" ), indexOriginProb( "indexOrigin" );
	LOG_IF( secondarySort.empty(), Debug, error ) << "There is no known secondary sorting left. Chunksort will fail.";
	assert( ch.isValid() );
	// compute the position of the chunk in the image space
	// we dont have this position, but we have the position in scanner-space (indexOrigin)
	const util::fvector3 origin = ch.getValueAs<util::fvector3>( indexOriginProb );
	// and we have the transformation matrix
	// [ rowVec ]
	// [ columnVec]
	// [ sliceVec]
	// [ 0 0 0 1 ]
	const util::fvector3 rowVec = ch.getValueAs<util::fvector3>( rowVecProb );
	const util::fvector3 columnVec = ch.getValueAs<util::fvector3>( columnVecProb );
	const util::fvector3 sliceVec = ch.getValueAsOr<util::fvector3>(
		sliceVecProb,
		util::fvector3(
			rowVec[1] * columnVec[2] - rowVec[2] * columnVec[1],
			rowVec[2] * columnVec[0] - rowVec[0] * columnVec[2],
			rowVec[0] * columnVec[1] - rowVec[1] * columnVec[0]
		)
	);

	// this is actually not the complete transform (it lacks the scaling for the voxel size), but its enough
	const util::fvector3 key( origin.dot( rowVec ), origin.dot( columnVec ), origin.dot( sliceVec ) );
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
	LOG_IF( !ch.isValid(), Debug, error ) << "You're trying insert an invalid chunk. The missing properties are " << ch.getMissing();
	LOG_IF( !ch.isValid(), Debug, error ) << "You should definitively check the chunks validity (use the function Chunk::valid) before calling this funktion. Aborting now..";
	assert( ch.isValid() );

	if( !isEmpty() ) {
		// compare some attributes of the first chunk and the one which shall be inserted
		Chunk &first = *( chunks.begin()->second.begin()->second );

		if ( first.getSizeAsVector() != ch.getSizeAsVector() ) { // if they have different size - do not insert
			LOG( Debug, verbose_info )
					<< "Ignoring chunk with different size. (" << ch.getSizeAsString() << "!=" << first.getSizeAsString() << ")";
			return false;
		}

		for(const util::PropertyMap::PropPath & ref :  equalProps ) { // check all properties which where given to the constructor of the list
			// if at least one of them has the property and they are not equal - do not insert
			if ( ( first.hasProperty( ref ) || ch.hasProperty( ref ) ) && first.property( ref ) != ch.property( ref ) ) {
				LOG( Debug, verbose_info )
						<< "Ignoring chunk with different " << ref << ". Is " << util::MSubject( ch.property( ref ) )
						<< " but chunks already in the list have " << util::MSubject( first.property( ref ) );
				return false;
			}
		}
	} else {
		LOG( Debug, verbose_info ) << "Inserting 1st chunk";
		std::stack<scalarPropCompare> backup = secondarySort;

		if( ch.getDimSize( sliceDim ) > 1 ) {
			LOG( Runtime, info ) << "We're dealing with volume chunks, considering indexOrigin as equal across Images";
			equalProps.push_back( "indexOrigin" );
		}

		while( !ch.hasProperty( secondarySort.top().propertyName ) ) {
			const util::PropertyMap::key_type temp = secondarySort.top().propertyName;

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

	std::pair<std::shared_ptr<Chunk>, bool> inserted = primaryInsert( ch );

	LOG_IF( inserted.first && !inserted.second, Debug, verbose_info )
			<< "Not inserting chunk because there is already a Chunk at the same position (" << ch.property( "indexOrigin" ) << ") with the equal property "
			<< std::make_pair( secondarySort.top().propertyName, ch.property( secondarySort.top().propertyName ) );

	LOG_IF(
		inserted.first && !inserted.second &&
		ch.hasProperty( "source" ) && inserted.first->hasProperty( "source" ) &&
		!( ch.property( "source" ) == inserted.first->property( "source" ) ),
		Debug, verbose_info )
			<< "The conflicting chunks where " << ch.property( "source" ).toString( false ) << " and " << inserted.first->property( "source" ).toString( false );

	return inserted.second;
}

void SortedChunkList::addSecondarySort( const util::PropertyMap::key_type &cmp )
{
	secondarySort.push( scalarPropCompare( cmp ) );
}
bool SortedChunkList::isEmpty()const
{
	return chunks.empty() || chunks.begin()->second.empty(); // if there is no subMap or nothing in the first subMap ... its empty
}
void SortedChunkList::clear()
{
	chunks.clear();
}
std::set<size_t> SortedChunkList::getShape()
{
	std::set<size_t> images;

	for( PrimaryMap::iterator c = chunks.begin(); c != chunks.end(); c++ ) {
		images.insert( c->second.size() );
	}

	return images;
}

size_t SortedChunkList::makeRectangular(optional< util::slist& > rejected)
{
	const std::set<size_t> images = getShape();//get lenghts of all primary sorted "columns" -- as set is sorted the smallest will be at begin
	size_t dropped = 0;

	if( images.size() > 1 ) { //cut down everything else if there is some
		if( chunks.begin()->second.begin()->second->getRelevantDims() >= 4 ) {
			#warning test me
			size_t resize = *images.rbegin();

			for( PrimaryMap::iterator c = chunks.begin(); c != chunks.end(); ) {
				if( c->second.size() != resize ) {
					if(rejected)
						for(const SecondaryMap::value_type &ch:c->second)
							rejected->push_back(ch.second->getValueAs<std::string>("source"));
					dropped += c->second.size();
					chunks.erase( c++ );
				} else
					c++;
			}
			LOG( Runtime, warning ) << "Fourth dimension already used, dropping all but " << resize << " volumes (" << dropped << ") to make " << identify(true,false) << " rectagular";

		} else {
			size_t resize = *images.begin();

			for( PrimaryMap::iterator c = chunks.begin(); c != chunks.end(); c++ ) { // in every "column"
				SecondaryMap &it = c->second;

				if( it.size() > resize ) { //remove everything behind the shortest length
					dropped += it.size() - resize;
					SecondaryMap::iterator firstinvalid = it.begin();
					std::advance( firstinvalid, resize );
					if(rejected)
						for(SecondaryMap::iterator i=firstinvalid;i!=it.end();i++)
							rejected->push_back(i->second->getValueAs<std::string>("source"));
					it.erase( firstinvalid, it.end() );
				}

				assert( it.size() == resize );
			}

			LOG_IF( dropped, Runtime, warning ) << "Dropped " << dropped << " chunks to make " << identify(true,false) << " rectagular";
		}
	}

	return dropped;
}

size_t SortedChunkList::getHorizontalSize()
{
	if( isEmpty() )return 0;
	else return chunks.begin()->second.size();
}

std::vector< std::shared_ptr< Chunk > > SortedChunkList::getLookup()
{
	LOG_IF( getShape().size() != 1, Debug, error ) << "Running getLookup on an non rectangular chunk-list is not defined";

	if( !isEmpty() ) {
		PrimaryMap::iterator iP = chunks.begin();
		const size_t horizontal = chunks.size();
		const size_t vertical = iP->second.size();
		std::vector< std::shared_ptr< Chunk > > ret( horizontal * vertical );

		for( size_t h = 0; h < horizontal; h++, iP++ ) { // outer loop iterates horizontaly (through the primary sorting)
			assert( iP != chunks.end() );
			SecondaryMap::iterator iS = iP->second.begin();

			for( size_t v = 0; v < vertical; v++, iS++ ) { // inner loop iterates verticaly (through the secondary sorting)
				assert( iS != iP->second.end() );
				ret[h + v * horizontal] = iS->second; // insert horizontally - primary sorting is the fastest running index (read the sorting matrix horizontaly)
			}
		}

		return ret;
	} else
		return std::vector< std::shared_ptr< Chunk > >();
}

void SortedChunkList::transform( chunkPtrOperator &op )
{
	for( PrimaryMap::reference outer :  chunks ) {
		for( SecondaryMap::reference inner :  outer.second ) {
			inner.second = op( inner.second );
		}
	}
}
std::string SortedChunkList::identify(bool withpath, bool withdate, getproplist seqNum, getproplist seqDesc, getproplist seqStart) const
{
	forall(seqNum);
	forall(seqDesc);
	forall(seqStart);
	
	std::string ret;
	if(seqNum.size()==1)
		ret+=std::string("S")+seqNum.begin()->toString();
	if(seqDesc.size()==1)
		ret+= (ret.empty() ? std::string():std::string("_"))+seqDesc.begin()->toString();
	if(withpath){
		ret+=(ret.empty() ? std::string():std::string(" "))+( std::string( "from " ) + getCommonSource().native() );
	}
	if(withdate && seqStart.size()==1)
		ret+=(ret.empty() ? std::string():std::string(" "))+std::string("taken at ") + seqStart.begin()->toString();
	return ret;
}
boost::filesystem::path SortedChunkList::getCommonSource() const
{
	std::list<boost::filesystem::path> sources;
	getproplist paths("source");
	forall(paths);
		
	for( const util::PropertyValue & ref : paths ) {
		sources.push_back( ref.as<std::string>() );
	}
	return util::getRootPath( sources );
}


}
}
}
/// @endcond _internal
