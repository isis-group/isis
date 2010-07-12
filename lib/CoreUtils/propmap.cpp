//
// C++ Implementation: propmap
//
// Description:
//
//
// Author:  <Enrico Reimer>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "propmap.hpp"
#include <boost/foreach.hpp>

namespace isis
{
namespace util
{

const PropMap::mapped_type PropMap::emptyEntry;//dummy to be able to return an empty Property

///////////////////////////////////////////////////////////////////
// Contructors
///////////////////////////////////////////////////////////////////

PropMap::PropMap( const isis::util::PropMap::base_type &src ): base_type( src ) {}

bool PropMap::operator==( const PropMap &src )const
{
	const base_type &other = src, &me = *this;
	return me == other;
}

PropMap::PropMap() {}


///////////////////////////////////////////////////////////////////
// The core tree traversal functions
///////////////////////////////////////////////////////////////////
PropMap::mapped_type &PropMap::fetchEntry( const std::string &key )
{
	const propPath path = util::string2list<std::string>( key, pathSeperator );
	return fetchEntry( *this, path.begin(), path.end() );
}
/**
 * Follow a "Path" to a property to get it.
 * This will create branches on its way if necessary.
 */
PropMap::mapped_type &PropMap::fetchEntry(
	PropMap &root,
	const isis::util::PropMap::propPathIterator at, const isis::util::PropMap::propPathIterator pathEnd )
{
	std::list< std::string >::const_iterator next = at;
	next++;
	base_type &rootRef = root;
	iterator found = root.find( *at );

	if ( next != pathEnd ) {//we are not at the end of the path (a proposed leaf in the PropMap)
		if ( found != root.end() ) {//and we found the entry
			mapped_type &ref = found->second;
			LOG_IF( ref.is_leaf(), Runtime, error )
					<< util::MSubject( found->first ) << " is a leaf, but was requested as a branch in "
					<< util::MSubject( util::list2string( at, pathEnd, "/" ) );
			return fetchEntry( ref.getBranch(), next, pathEnd ); //continue there
		} else { // if we should create a sub-map
			//insert a empty branch (aka PropMap) at "*at" (and fetch the reference of that)
			return fetchEntry( rootRef[*at].getBranch(), next, pathEnd ); // and continue there
		}
	} else { //if its the leaf
		return rootRef[*at]; // (create and) return that entry
	}
}

const PropMap::mapped_type *PropMap::findEntry( const std::string &key )const
{
	const propPath path = util::string2list<std::string>( key, pathSeperator );
	return findEntry( *this, path.begin(), path.end() );
}
/**
 * Find property following the given "path".
 * If the "path" or the property does not exist NULL is returned.
 */
const PropMap::mapped_type *PropMap::findEntry(
	const PropMap &root,
	const propPathIterator at, const propPathIterator pathEnd )
{
	propPathIterator next = at;
	next++;
	util::PropMap::const_iterator found = root.find( *at );

	if ( next != pathEnd ) {//we are not at the end of the path (aka the leaf)
		if ( found != root.end() ) {//and we found the entry
			const mapped_type &ref = found->second;
			return findEntry( ref.getBranch(), next, pathEnd ); //continue there
		}
	} else if ( found != root.end() ) {// if its the leaf and we found the entry
		return &found->second; // return that entry
	}

	return NULL;
}
bool PropMap::recursiveRemove( PropMap &root, const propPathIterator at, const propPathIterator pathEnd )
{
	bool ret = true;

	if ( at != pathEnd ) {
		propPathIterator next = at;
		next++;
		iterator found = root.find( *at );

		if ( found != root.end() ) {
			mapped_type &ref = found->second;

			if ( ! ref.is_leaf() ) {
				ret = recursiveRemove( ref.getBranch(), next, pathEnd );

				if ( ref.getBranch().empty() )
					root.erase( found ); // remove the now empty branch
			} else
				root.erase( found );
		} else {
			LOG( Runtime, warning ) << "Entry " << util::MSubject( *at ) << " not found, skipping it";
			ret = false;
		}
	}

	return ret;
}


/////////////////////////////////////////////////////////////////////////////////////
// Generic interface for accessing elements
////////////////////////////////////////////////////////////////////////////////////
const PropertyValue &PropMap::propertyValue( const std::string &key )const
{
	const propPath path = util::string2list<std::string>( key, pathSeperator );
	const mapped_type *ref = findEntry( *this, path.begin(), path.end() );

	if( ref && ref->is_leaf() ) {
		return ref->getLeaf();
	} else {
		LOG( Debug, warning ) << "Property " << key << " not found. Returning empty property.";
		return emptyEntry.getLeaf();
	}
}

PropertyValue &PropMap::propertyValue( const std::string &key )
{
	const propPath path = util::string2list<std::string>( key, pathSeperator );
	mapped_type &n = fetchEntry( *this, path.begin(), path.end() );
	LOG_IF( ! n.is_leaf(), Debug, error ) << "Using branch " << key << " as PropertyValue";
	return n.getLeaf();
}

const PropMap &PropMap::branch( const std::string &key ) const
{
	const propPath path = util::string2list<std::string>( key, pathSeperator );
	const mapped_type *ref = findEntry( *this, path.begin(), path.end() );

	if( ! ref ) {
		LOG( Runtime, warning ) << "Trying to access non existing branch " << key << ".";
		return emptyEntry.getBranch();
	} else {
		LOG_IF( ref->getBranch().empty(), Runtime, warning ) << "Accessing empty branch " << key;
		return ref->getBranch();
	}
}
PropMap &PropMap::branch( const std::string &key )
{
	const propPath path = util::string2list<std::string>( key, pathSeperator );
	mapped_type &n = fetchEntry( *this, path.begin(), path.end() );
	return n.getBranch();
}

bool PropMap::remove( const std::string &key )
{
	const propPath path = util::string2list<std::string>( key, pathSeperator );
	return recursiveRemove( *this, path.begin(), path.end() );
}

bool PropMap::remove( const isis::util::PropMap &removeMap, bool keep_needed )
{
	iterator thisIt = begin();
	bool ret = true;

	//remove everything that is also in second
	for ( const_iterator otherIt = removeMap.begin(); otherIt != removeMap.end(); otherIt++ ) {
		//find the closest match for otherIt->first in this (use the value-comparison-functor of PropMap)
		if ( continousFind( thisIt, end(), *otherIt, value_comp() ) ) { //thisIt->first == otherIt->first - so its the same property or propmap
			if ( ! thisIt->second.is_leaf() ) { //this is a branch
				if ( ! otherIt->second.is_leaf() ) { // recurse if its a branch in the removal map as well
					PropMap &mySub = thisIt->second.getBranch();
					const PropMap &otherSub = otherIt->second.getBranch();
					ret &= mySub.remove( otherSub );

					if( mySub.empty() ) // delete my branch, if its empty
						erase( thisIt++ );
				} else {
					LOG( Debug, warning ) << "Not deleting branch " << MSubject( thisIt->first ) << " because its no subtree in the removal map";
					ret = false;
				}
			} else if( !( thisIt->second.getLeaf().needed() && keep_needed ) ) { // this is a leaf
				LOG( Debug, verbose_info ) << "Removing " << MSubject( *thisIt ) << " as requested";
				erase( thisIt++ ); // so delete this (they are equal - kind of)
			}
		}
	}

	return ret;
}


/////////////////////////////////////////////////////////////////////////////////////
// utilities
////////////////////////////////////////////////////////////////////////////////////
bool PropMap::valid() const
{
	//iterate through the whole map and return false as soon as we find something needed _and_ empty
	const const_iterator found = std::find_if( begin(), end(), treeInvalidP() );
	return found == end();
}

bool PropMap::empty() const
{
	return base_type::empty();
}

PropMap::diff_map PropMap::getDifference( const PropMap &other ) const
{
	PropMap::diff_map ret;
	diffTree( other, ret, "" );
	return ret;
}

void PropMap::diffTree( const PropMap &other, PropMap::diff_map &ret, std::string prefix ) const
{
	const_iterator otherIt = other.begin();

	//insert everything that is in this, but not in second or is on both but differs
	for ( const_iterator thisIt = begin(); thisIt != end(); thisIt++ ) {
		const std::string pathname = prefix + thisIt->first;

		//find the closest match for thisIt->first in other (use the value-comparison-functor of PropMap)
		if ( continousFind( otherIt, other.end(), *thisIt, value_comp() ) ) { //otherIt->first == thisIt->first - so its the same property
			const mapped_type &first = thisIt->second, &second = otherIt->second;

			if ( ! ( first.is_leaf() || second.is_leaf() ) ) { // if both are a branch
				const PropMap &thisMap = first.getBranch();
				const PropMap &refMap = second.getBranch();
				thisMap.diffTree( refMap, ret, pathname + "/" );
			} else if ( ! ( first == second )  ) { // if they are not equal
				const PropertyValue firstVal = first.is_leaf() ? first.getLeaf() : PropertyValue( Type<std::string>( first.toString() ) );
				const PropertyValue secondVal = second.is_leaf() ? second.getLeaf() : PropertyValue( Type<std::string>( second.toString() ) );
				ret.insert( // add (propertyname|(value1|value2))
					ret.end(),      // we know it has to be at the end
					std::make_pair(
						pathname,   //the key
						std::make_pair( firstVal, secondVal ) //pair of both values
					)
				);
			}
		} else { // if ref is not in the other map
			const PropertyValue firstVal = thisIt->second.is_leaf() ? thisIt->second.getLeaf() : PropertyValue( Type<std::string>( thisIt->second.toString() ) );
			ret.insert( // add (propertyname|(value1|[empty]))
				ret.end(),      // we know it has to be at the end
				std::make_pair(
					pathname,
					std::make_pair( firstVal, PropertyValue() )
				)
			);
		}
	}

	//insert everything that is in second but not in this
	const_iterator thisIt = begin();

	for ( otherIt = other.begin(); otherIt != other.end(); otherIt++ ) {
		const std::string pathname = prefix + otherIt->first;

		if ( ! continousFind( thisIt, end(), *otherIt, value_comp() ) ) { //there is nothing in this which has the same key as ref
			const PropertyValue secondVal = otherIt->second.is_leaf() ? otherIt->second.getLeaf() : PropertyValue( Type<std::string>( otherIt->second.toString() ) );
			ret.insert(
				std::make_pair( // add (propertyname|([empty]|value2))
					pathname,
					std::make_pair( PropertyValue(), secondVal )
				)
			);
		}
	}
}

void PropMap::make_unique ( const util::PropMap &other, bool removeNeeded )
{
	iterator thisIt = begin();

	//remove everything that is also in second and equal (or also empty)
	for ( const_iterator otherIt = other.begin(); otherIt != other.end(); otherIt++ ) {
		//find the closest match for otherIt->first in this (use the value-comparison-functor of PropMap)
		if ( continousFind( thisIt, end(), *otherIt, value_comp() ) ) { //thisIt->first == otherIt->first  - so its the same property
			if ( ! removeNeeded && thisIt->second.getLeaf().needed() ) {//Skip needed
				thisIt++;
				continue;
			}

			if ( thisIt->second.empty() ) { //this is empty
				if ( otherIt->second.empty() ) { //the other is empty
					erase( thisIt++ ); // so delete this (they are equal - kind of)
				} else {
					LOG( Debug, verbose_info ) << "Keeping the empty " << thisIt->first << " because its is not empty in the other (" << *otherIt << ")";
					thisIt++;
				}
			} else if ( ! otherIt->second.empty() ) { // if the other is not empty as well
				if ( thisIt->second == otherIt->second ) { //delete this, if they are equal
					LOG( Debug, verbose_info ) << "Removing " << *thisIt << " because its equal with the other (" << *otherIt << ")";
					erase( thisIt++ ); // so delete this (they are equal - kind of)
				} else if ( ! ( thisIt->second.is_leaf() || otherIt->second.is_leaf() ) ) { //but maybe they are branches
					PropMap &thisMap = thisIt->second.getBranch();
					const PropMap &otherMap = otherIt->second.getBranch();
					thisMap.make_unique( otherMap );
					thisIt++;
				}
			} else {//only the other is empty
				LOG( Debug, verbose_info ) << "Keeping " << *thisIt << " because the other is empty";
				thisIt++;
			}
		}
	}
}


PropMap::key_list PropMap::join( const isis::util::PropMap &other, bool overwrite )
{
	key_list rejects;
	joinTree( other, overwrite, "", rejects );
	return rejects;
}

void PropMap::joinTree( const isis::util::PropMap &other, bool overwrite, std::string prefix, PropMap::key_list &rejects )
{
	iterator thisIt = begin();

	for ( const_iterator otherIt = other.begin(); otherIt != other.end(); otherIt++ ) { //iterate through the elements of other
		if ( continousFind( thisIt, end(), *otherIt, value_comp() ) ) { // if the element is allready here
			if ( thisIt->second.empty() ) { // if ours is empty
				LOG( Debug, verbose_info ) << "Replacing empty property " << MSubject( thisIt->first ) << " by " << MSubject( otherIt->second );
				thisIt->second = otherIt->second;
			} else if ( ! ( thisIt->second.is_leaf() || otherIt->second.is_leaf() ) ) { // if both are a subtree
				PropMap &thisMap = thisIt->second.getBranch();
				const PropMap &refMap = otherIt->second.getBranch();
				thisMap.joinTree( refMap, overwrite, prefix + thisIt->first + "/", rejects ); //recursion
			} else if ( overwrite ) { // otherwise replace ours by the other (if we shall overwrite)
				LOG( Debug, info ) << "Replacing property " << MSubject( *thisIt ) << " by " << MSubject( otherIt->second );
				thisIt->second = otherIt->second;
			} else if ( ! ( thisIt->second == otherIt->second ) ) { // otherwise put the other into rejected if its not equal to our
				LOG( Debug, info )
						<< "Rejecting property " << MSubject( *otherIt )
						<< " because " << MSubject( thisIt->second ) << " is allready there";
				rejects.insert( rejects.end(), prefix + otherIt->first );
			}
		} else { // ok we dont have that - just insert it
			std::pair<const_iterator, bool> inserted = insert( *otherIt );
			LOG_IF( inserted.second, Debug, verbose_info ) << "Inserted property " << MSubject( *inserted.first ) << ".";
		}
	}
}


void PropMap::linearize( flat_map &out, std::string key_prefix ) const
{
	for ( const_iterator i = begin(); i != end(); i++ ) {
		std::string key = ( key_prefix.empty() ? "" : key_prefix + pathSeperator ) + i->first;

		if ( i->second.is_leaf()  ) {
			out.insert( std::make_pair( key, i->second.getLeaf() ) );
		} else {
			i->second.getBranch().linearize( out, key );
		}
	}
}

bool PropMap::transform( std::string from,  std::string to, int dstId, bool delSource )
{
	LOG_IF( from == to, Debug, error ) << "Sorry source and destination shall not be the same";
	const PropertyValue &found = propertyValue( from );
	bool ret = false;

	if( ! found.empty() ) {
		if ( found->typeID() == dstId ) {
			propertyValue( to ) = found ;
			ret = true;
		} else
			ret = found.transformTo( propertyValue( to ), dstId );
	}

	if ( ret && delSource )remove( from );

	return ret;
}


const PropMap::key_list PropMap::getKeys()const
{
	PropMap::key_list ret;
	std::for_each( begin(), end(), walkTree<trueP>( ret ) );
	return ret;
}

const PropMap::key_list PropMap::getMissing() const
{
	PropMap::key_list ret;
	std::for_each( begin(), end(), walkTree<invalidP>( ret ) );
	return ret;
}


void PropMap::addNeeded( const std::string &key )
{
	propertyValue( key ).needed() = true;
}


void PropMap::addNeededFromString( const std::string &needed )
{
	const std::list<std::string> needList = util::string2list<std::string>( needed ); 
	//@todo util::string2list<std::string>( needed,' ' ) would be faster but less robust
	LOG( Debug, verbose_info ) << "Adding " << needed << " as needed";
	BOOST_FOREACH( std::list<std::string>::const_reference ref, needList )
	addNeeded( ref );
}

/// \returns true if a leaf exists at the given path and the property is not empty
bool PropMap::hasProperty( const std::string &key ) const
{
	const propPath path = util::string2list<std::string>( key, pathSeperator );
	const mapped_type *ref = findEntry( *this, path.begin(), path.end() );
	return ( ref && ref->is_leaf() && ! ref->getLeaf().empty() );
}
/// \returns true if a leaf exists at the given path and the property is not empty
bool PropMap::hasBranch( const std::string &key ) const
{
	const propPath path = util::string2list<std::string>( key, pathSeperator );
	const mapped_type *ref = findEntry( *this, path.begin(), path.end() );
	return ( ref && ! ref->is_leaf()  );
}

bool PropMap::rename( std::string oldname, std::string newname )
{
	const mapped_type *old_e = findEntry( oldname );
	const mapped_type *new_e = findEntry( newname );

	if ( old_e ) {
		LOG_IF( new_e && ! new_e->empty(), Runtime, warning )
				<< "Overwriting " << std::make_pair( newname, *new_e ) << " with " << *old_e;
		fetchEntry( newname ) = *old_e;
		return remove( oldname );
	} else {
		LOG( Runtime, warning )
				<< "Cannot rename " << oldname << " it does not exist";
		return false;
	}
}

void PropMap::toCommonUnique( PropMap &common, std::set<std::string> &uniques, bool init )const
{
	if ( init ) {
		common = *this;
		uniques.clear();
		return;
	} else {
		const diff_map difference = common.getDifference( *this );
		BOOST_FOREACH( const diff_map::value_type & ref, difference ) {
			uniques.insert( ref.first );

			if ( ! ref.second.first.empty() )common.remove( ref.first );//if there is something in common, remove it
		}
	}
}

std::ostream &PropMap::print( std::ostream &out, bool label )const
{
	flat_map buff;
	linearize( buff );
	size_t key_len = 0;

	for ( flat_map::const_iterator i = buff.begin(); i != buff.end(); i++ )
		if ( key_len < i->first.length() )
			key_len = i->first.length();

	for ( flat_map::const_iterator i = buff.begin(); i != buff.end(); i++ )
		out << i->first << std::string( key_len - i->first.length(), ' ' ) + ":" << i->second.toString( label ) << std::endl;

	return out;
}

bool PropMap::trueP::operator()( const PropMap::value_type& ref ) const
{
	return true;
}
bool PropMap::invalidP::operator()( const PropMap::value_type& ref ) const
{
	return ref.second.getLeaf().needed() && ref.second.getLeaf().empty();
}
bool PropMap::treeInvalidP::operator()( const PropMap::value_type& ref ) const
{
	if ( ref.second.is_leaf() ) {
		const PropertyValue &val = ref.second.getLeaf();
		return val.needed() && val.empty();
	} else  {
		return ! ref.second.getBranch().valid();
	}
}

}
}
