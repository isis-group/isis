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

#include "sortedchunklist.hpp"

namespace isis{
namespace data{
namespace _internal{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// sorting algorithm implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SortedChunkList::sortComparator::sortComparator(const std::string& prop_name):propertyName(prop_name){}
SortedChunkList::sortComparator::~sortComparator(){}

SortedChunkList::fvectorCompare::fvectorCompare(const std::string& prop_name): sortComparator(prop_name){}
SortedChunkList::scalarPropCompare::scalarPropCompare(const std::string& prop_name): sortComparator(prop_name){}

bool SortedChunkList::fvectorCompare::operator()(const isis::util::PropertyValue& a, const isis::util::PropertyValue& b) const
{
	const util::fvector4 &posA = a->cast_to_Type<util::fvector4>();
	const util::fvector4 &posB = b->cast_to_Type<util::fvector4>();
	
	if ( posA.lexical_less_reverse( posB ) ) { //if chunk is "under" the other - put it there
		LOG( Debug, verbose_info ) << "Successfully sorted chunks by " << propertyName  << " (" << posA << " below " << posB << ")";
		return true;
	}
	return false;
}
bool SortedChunkList::scalarPropCompare::operator()(const isis::util::PropertyValue& a, const isis::util::PropertyValue& b) const
{
	const util::_internal::TypeBase &aTime = *a;
	const util::_internal::TypeBase &bTime = *b;
	
	if ( aTime.lt(bTime)) {
		LOG( Debug, info ) << "Successfully sorted chunks by " << propertyName << " (" << aTime.toString(false) << " before " << bTime.toString(false) << ")";
		return true;
	}
	else
		return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Chunk operators
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SortedChunkList::chunkPtrOperator::operator()(boost::shared_ptr< Chunk >& ptr)
{
	operator()(const_cast<const boost::shared_ptr< Chunk >& >(ptr));
}
void SortedChunkList::chunkPtrOperator::operator()(const boost::shared_ptr< Chunk >& ptr)
{
	LOG(Debug,error) << "Empty chunk operation, you should at least override \"operator()(const boost::shared_ptr< Chunk >& ptr)\"";
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SortedChunkList implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// constructor
SortedChunkList::SortedChunkList(std::string fvectorPropName, std::string comma_separated_equal_props ):
primarySort(fvectorPropName),chunks(primarySort),equalProps(util::string2list<std::string>(comma_separated_equal_props,','))
{}


// low level finding
boost::shared_ptr<Chunk> SortedChunkList::secondaryFind(const util::PropertyValue& key, SortedChunkList::SecondaryMap& map)
{
	const SecondaryMap::iterator found=map.find(key);
	return found!=map.end() ? found->second:boost::shared_ptr<Chunk>();
}
SortedChunkList::SecondaryMap* SortedChunkList::primaryFind(const util::fvector4& key)
{
	const PrimaryMap::iterator found=chunks.find(key);
	return found!=chunks.end() ? &found->second:NULL;
}

// low level insert
std::pair<boost::shared_ptr<Chunk>,bool> SortedChunkList::secondaryInsert(SecondaryMap& map, const Chunk& ch)
{
	std::string propName=map.key_comp().propertyName;
	if(ch.hasProperty(propName)){
		//check, if there is already a chunk
		boost::shared_ptr<Chunk> &pos=map[ch.propertyValue(propName)];
		bool inserted=false;
		//if not. put oures there
		if(!pos){
			pos.reset(new Chunk(ch));
			inserted=true;
		}
		assert(pos); // here it must have some content
		return std::make_pair(pos,inserted);
	} else{
		LOG(Runtime, warning) << "Cannot insert chunk. It's lacking the property " << util::MSubject( propName ) << " which is needed for primary sorting";
		return std::pair<boost::shared_ptr<Chunk>,bool>(boost::shared_ptr<Chunk>(),false);
	}
}
std::pair<boost::shared_ptr<Chunk>,bool> SortedChunkList::primaryInsert(const Chunk& ch)
{
	const std::string &propName = primarySort.propertyName;
	LOG_IF(secondarySort.empty(),Debug,error) << "There is no known secondary sorting left. Chunksort will fail.";
	
	if(ch.hasProperty(propName)){
		const util::PropertyValue &prop=ch.propertyValue(propName);
		if(prop->is<util::fvector4>()){
			const util::fvector4 &key=prop->cast_to_Type<util::fvector4>();
			const scalarPropCompare &secondaryComp=secondarySort.top();

			// get the reference of the secondary map for "key" (create and insert a new if neccessary)
			SecondaryMap &subMap=chunks.insert(std::make_pair(key,SecondaryMap(secondaryComp))).first->second;

			// run insert on that
			return secondaryInsert(subMap,ch); // insert ch into the right secondary map
		} else {
			LOG(Runtime, warning) << "Cannot insert chunk. It's property " << propName << " has wrong type " << prop->typeName()
			<< ". it should be " << util::Type<util::fvector4>::staticName();
		}
	} else
		LOG(Runtime, warning) << "Cannot insert chunk. It's lacking the property " << util::MSubject( propName ) << " which is needed for primary sorting";
	return std::pair<boost::shared_ptr<Chunk>,bool>(boost::shared_ptr<Chunk>(),false);;
}

// high level insert
bool SortedChunkList::insert(const Chunk& ch)
{
	LOG_IF(secondarySort.empty(),Debug,error) << "Inserting will fail without any secondary sort. Use chunks.addSecondarySort at least once.";
	const std::string &prop1=primarySort.propertyName;
	if(!empty())
	{
		// compare some attributes of the first chunk and the one which shall be inserted
		Chunk &first=*(chunks.begin()->second.begin()->second);
		if ( first.sizeToVector() != ch.sizeToVector() ) { // if they have different size - do not insert
			LOG( Debug, info )
			<< "Ignoring chunk with different size. (" << ch.sizeToString() << "!=" << first.sizeToString() << ")";
			return false;
		}
		BOOST_FOREACH(std::string &ref,equalProps){ // check all properties which where given to the constructor of the list
			// if at least one of them has the property and they are not equal - do not insert
			if ( (first.hasProperty( ref ) || ch.hasProperty( ref )) && first.propertyValue( ref ) != ch.propertyValue( ref ) ) {
				LOG( Debug, info )
					<< "Ignoring chunk with different "<< ref << ". (" << ch.propertyValue( ref ) << "!=" << first.propertyValue( ref ) << ")";
				return false;
			}
		}
		
	} else {
		LOG( Debug, verbose_info ) << "Inserting 1st chunk";
		std::stack<scalarPropCompare> backup=secondarySort;

		while(!ch.hasProperty( secondarySort.top().propertyName )){
			const std::string temp=secondarySort.top().propertyName;
			if ( secondarySort.size()>1) {
				secondarySort.pop();
			} else {
				LOG( Debug, warning )
				<< "First chunk is missing the last secondary sort-property fallback (" << util::MSubject(temp) << "), won't insert.";
				secondarySort=backup;
				return false;
			}
		}
		LOG( Debug, info )	<< "Using " << secondarySort.top().propertyName << " for secondary sorting, determined by the first chunk";
	}
	
	const std::string &prop2=secondarySort.top().propertyName;
	
	std::pair<boost::shared_ptr<Chunk>,bool> inserted=primaryInsert(ch);
	LOG_IF(inserted.first && !inserted.second,Debug,info)
		<< "Not inserting chunk because there is allready one with the equal properties "
		<< prop1 << ":" << ch.propertyValue(prop1) << " and " << prop2 << ":" << ch.propertyValue(prop2);
	return inserted.second;
}

void SortedChunkList::addSecondarySort(const std::string& cmp)
{
	secondarySort.push(scalarPropCompare(cmp));
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
	if(empty())return true;
	size_t images=getHorizontalSize();
	BOOST_FOREACH(PrimaryMap::reference outer,chunks){
		if(outer.second.size()!=images)
			return false;
	}
	return true;
}
size_t SortedChunkList::getHorizontalSize()
{
	if(empty())return 0;
	else return chunks.begin()->second.size();
}

std::vector< boost::weak_ptr< Chunk > > SortedChunkList::getLookup()
{
	LOG_IF(!isRectangular(),Debug,error) << "Running getLookup on an non rectangular chunk-list is not defined";

	if(!empty()){
		PrimaryMap::iterator iP=chunks.begin();
		const size_t horizontal=chunks.size();
		const size_t vertical=iP->second.size();

		std::vector< boost::weak_ptr< Chunk > > ret(horizontal*vertical);
		
		for(size_t h=0;h<horizontal;h++,iP++){ // outer loop iterates horizontaly (through the primary sorting)
			assert(iP!=chunks.end());
			SecondaryMap::iterator iS=iP->second.begin();
			for(size_t v=0;v<vertical;v++,iS++){ // inner loop iterates verticaly (through the secondary sorting)
				assert(iS!=iP->second.end());
				boost::weak_ptr<Chunk> ptr=iS->second;
				ret[h+v*horizontal]=ptr; // insert horizontally - primary sorting is the fastest running index (read the sorting matrix horizontaly)
			}
		}
		return ret;
	} else 
		return std::vector< boost::weak_ptr< Chunk > >();
}

void SortedChunkList::forall_ptr(chunkPtrOperator& op)
{
	BOOST_FOREACH(PrimaryMap::reference outer,chunks){
		BOOST_FOREACH(SecondaryMap::reference inner,outer.second){
			op(inner.second);
		}
	}
}


}}}