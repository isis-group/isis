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
namespace _internal
{
/**
 * Continously searches in a sorted list using the given less-than comparison.
 * It starts at current and increments it until the referenced value is not less than the compare-value anymore.
 * Than it returns.
 * \param current the current-position-iterator for the sorted list.
 * This value is changed directly, so after the function returns is references the first entry of the list
 * which does not compare less than compare or, if such a value does not exit in the list, it will be equal to end.
 * \param end the end of the list
 * \param compare the compare-value
 * \param compOp the comparison functor. It must provide "bool operator()(T,T)".
 * \returns true if the value current currently refers to is equal to compare
 */
template<typename ForwardIterator, typename T, typename CMP> bool
continousFind( ForwardIterator &current, const ForwardIterator end, const T &compare, CMP compOp )
{
	//find the first iterator which is does not compare less
	current = std::lower_bound( current, end, compare, compOp );

	if ( current == end //if we're at the end
		|| compOp( compare, *current ) //or compare is less than that iterator
	)
	return false;//we didn't find a match
	else
		return true;//not(current <> compare) makes compare == current
}
}
const PropertyMap::mapped_type PropertyMap::emptyEntry;//dummy to be able to return an empty Property


///////////////////////////////////////////////////////////////////
// Contructors
///////////////////////////////////////////////////////////////////

PropertyMap::PropertyMap( const isis::util::PropertyMap::Container &src ): Container( src ) {}

bool PropertyMap::operator==( const PropertyMap &src )const
{
	const Container &other = src, &me = *this;
	return me == other;
}

PropertyMap::PropertyMap() {}


///////////////////////////////////////////////////////////////////
// The core tree traversal functions
///////////////////////////////////////////////////////////////////
PropertyMap::mapped_type &PropertyMap::fetchEntry( const key_type &key )
{
	const propPath path = util::string2list<key_type>( key, pathSeperator );
	return fetchEntry( *this, path.begin(), path.end() );
}
/**
 * Follow a "Path" to a property to get it.
 * This will create branches on its way if necessary.
 */
PropertyMap::mapped_type &PropertyMap::fetchEntry(
	PropertyMap &root,
	const isis::util::PropertyMap::propPathIterator at, const isis::util::PropertyMap::propPathIterator pathEnd )
{
	propPath::const_iterator next = at;
	next++;
	Container &rootRef = root;
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
			LOG(Debug,verbose_info) << "Creating an empty branch " << *at << " trough fetching";
			return fetchEntry( rootRef[*at].getBranch(), next, pathEnd ); // and continue there
		}
	} else { //if its the leaf
		LOG_IF(rootRef.find(*at)==rootRef.end() ,Debug,verbose_info) << "Creating an empty entry " << *at << " trough fetching";
		return rootRef[*at]; // (create and) return that entry
	}
}

const PropertyMap::mapped_type *PropertyMap::findEntry( const key_type &key )const
{
	const propPath path = util::string2list<key_type>( key, pathSeperator );
	return findEntry( *this, path.begin(), path.end() );
}
/**
 * Find property following the given "path".
 * If the "path" or the property does not exist NULL is returned.
 */
const PropertyMap::mapped_type *PropertyMap::findEntry(
	const PropertyMap &root,
	const propPathIterator at, const propPathIterator pathEnd )
{
	propPathIterator next = at;
	next++;
	util::PropertyMap::const_iterator found = root.find( *at );

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
bool PropertyMap::recursiveRemove( PropertyMap &root, const propPathIterator at, const propPathIterator pathEnd )
{
	bool ret = false;

	if ( at != pathEnd ) {
		propPathIterator next = at;
		next++;
		iterator found = root.find( *at );

		if ( found != root.end() ) {
			mapped_type &ref = found->second;

			if ( ! ref.is_leaf() ) {
				ret = recursiveRemove( ref.getBranch(), next, pathEnd );

				if ( ref.getBranch().isEmpty() )
					root.erase( found ); // remove the now empty branch
			} else {
				root.erase( found );
				ret = true;
			}
		} else {
			LOG( Runtime, warning ) << "Entry " << util::MSubject( *at ) << " not found, skipping it";
		}
	}

	return ret;
}


/////////////////////////////////////////////////////////////////////////////////////
// Generic interface for accessing elements
////////////////////////////////////////////////////////////////////////////////////
const TypeValue &PropertyMap::propertyValue( const key_type &key )const
{
	const propPath path = util::string2list<key_type>( key, pathSeperator );
	const mapped_type *ref = findEntry( *this, path.begin(), path.end() );

	if( ref && ref->is_leaf() ) {
		return ref->getLeaf();
	} else {
		LOG( Debug, warning ) << "Property " << key << " not found. Returning empty property.";
		return emptyEntry.getLeaf();
	}
}

TypeValue &PropertyMap::propertyValue( const key_type &key )
{
	const propPath path = util::string2list<key_type>( key, pathSeperator );
	mapped_type &n = fetchEntry( *this, path.begin(), path.end() );
	LOG_IF( ! n.is_leaf(), Debug, error ) << "Using branch " << key << " as TypeValue";
	return n.getLeaf();
}

const PropertyMap &PropertyMap::branch( const key_type &key ) const
{
	const propPath path = util::string2list<key_type>( key, pathSeperator );
	const mapped_type *ref = findEntry( *this, path.begin(), path.end() );

	if( ! ref ) {
		LOG( Runtime, warning ) << "Trying to access non existing branch " << key << ".";
		return emptyEntry.getBranch();
	} else {
		LOG_IF( ref->getBranch().isEmpty(), Runtime, warning ) << "Accessing empty branch " << key;
		return ref->getBranch();
	}
}
PropertyMap &PropertyMap::branch( const key_type &key )
{
	const propPath path = util::string2list<key_type>( key, pathSeperator );
	mapped_type &n = fetchEntry( *this, path.begin(), path.end() );
	return n.getBranch();
}

bool PropertyMap::remove( const key_type &key )
{
	const propPath path = util::string2list<key_type>( key, pathSeperator );
	return recursiveRemove( *this, path.begin(), path.end() );
}

bool PropertyMap::remove( const isis::util::PropertyMap &removeMap, bool keep_needed )
{
	iterator thisIt = begin();
	bool ret = true;

	//remove everything that is also in second
	for ( const_iterator otherIt = removeMap.begin(); otherIt != removeMap.end(); otherIt++ ) {
		//find the closest match for otherIt->first in this (use the value-comparison-functor of PropMap)
		if ( continousFind( thisIt, end(), *otherIt, value_comp() ) ) { //thisIt->first == otherIt->first - so its the same property or propmap
			if ( ! thisIt->second.is_leaf() ) { //this is a branch
				if ( ! otherIt->second.is_leaf() ) { // recurse if its a branch in the removal map as well
					PropertyMap &mySub = thisIt->second.getBranch();
					const PropertyMap &otherSub = otherIt->second.getBranch();
					ret &= mySub.remove( otherSub );

					if( mySub.isEmpty() ) // delete my branch, if its empty
						erase( thisIt++ );
				} else {
					LOG( Debug, warning ) << "Not deleting branch " << MSubject( thisIt->first ) << " because its no subtree in the removal map";
					ret = false;
				}
			} else if( !( thisIt->second.getLeaf().needed() && keep_needed ) ) { // this is a leaf
				erase( thisIt++ ); // so delete this (they are equal - kind of)
			}
		}
	}

	return ret;
}


/////////////////////////////////////////////////////////////////////////////////////
// utilities
////////////////////////////////////////////////////////////////////////////////////
bool PropertyMap::isValid() const
{
	//iterate through the whole map and return false as soon as we find something needed _and_ empty
	const const_iterator found = std::find_if( begin(), end(), treeInvalidP() );
	return found == end();
}

bool PropertyMap::isEmpty() const
{
	return Container::empty();
}

PropertyMap::DiffMap PropertyMap::getDifference( const PropertyMap &other ) const
{
	PropertyMap::DiffMap ret;
	diffTree( other, ret, "" );
	return ret;
}

void PropertyMap::diffTree( const PropertyMap &other, PropertyMap::DiffMap &ret, istring prefix ) const
{
	const_iterator otherIt = other.begin();

	//insert everything that is in this, but not in second or is on both but differs
	for ( const_iterator thisIt = begin(); thisIt != end(); thisIt++ ) {
		const propPath::value_type pathname = prefix + thisIt->first;

		//find the closest match for thisIt->first in other (use the value-comparison-functor of PropMap)
		if ( _internal::continousFind( otherIt, other.end(), *thisIt, value_comp() ) ) { //otherIt->first == thisIt->first - so its the same property
			const mapped_type &first = thisIt->second, &second = otherIt->second;

			if ( ! ( first.is_leaf() || second.is_leaf() ) ) { // if both are a branch
				const PropertyMap &thisMap = first.getBranch();
				const PropertyMap &refMap = second.getBranch();
				thisMap.diffTree( refMap, ret, pathname + "/" );
			} else if ( ! ( first == second )  ) { // if they are not equal
				const TypeValue firstVal = first.is_leaf() ? first.getLeaf() : TypeValue( Type<std::string>( first.toString() ) );
				const TypeValue secondVal = second.is_leaf() ? second.getLeaf() : TypeValue( Type<std::string>( second.toString() ) );
				ret.insert( // add (propertyname|(value1|value2))
					ret.end(),      // we know it has to be at the end
					std::make_pair(
						pathname,   //the key
						std::make_pair( firstVal, secondVal ) //pair of both values
					)
				);
			}
		} else { // if ref is not in the other map
			const TypeValue firstVal = thisIt->second.is_leaf() ? thisIt->second.getLeaf() : TypeValue( Type<std::string>( thisIt->second.toString() ) );
			ret.insert( // add (propertyname|(value1|[empty]))
				ret.end(),      // we know it has to be at the end
				std::make_pair(
					pathname,
					std::make_pair( firstVal, TypeValue() )
				)
			);
		}
	}

	//insert everything that is in second but not in this
	const_iterator thisIt = begin();

	for ( otherIt = other.begin(); otherIt != other.end(); otherIt++ ) {
		const propPath::value_type pathname = prefix + otherIt->first;

		if ( ! _internal::continousFind( thisIt, end(), *otherIt, value_comp() ) ) { //there is nothing in this which has the same key as ref
			const TypeValue secondVal = otherIt->second.is_leaf() ? otherIt->second.getLeaf() : TypeValue( Type<std::string>( otherIt->second.toString() ) );
			ret.insert(
				std::make_pair( // add (propertyname|([empty]|value2))
					pathname,
					std::make_pair( TypeValue(), secondVal )
				)
			);
		}
	}
}

void PropertyMap::removeEqual ( const util::PropertyMap &other, bool removeNeeded )
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
					PropertyMap &thisMap = thisIt->second.getBranch();
					const PropertyMap &otherMap = otherIt->second.getBranch();
					thisMap.removeEqual( otherMap );
					thisIt++;
				}
			} else {//only the other is empty
				LOG( Debug, verbose_info ) << "Keeping " << *thisIt << " because the other is empty";
				thisIt++;
			}
		}
	}
}


PropertyMap::KeyList PropertyMap::join( const isis::util::PropertyMap &other, bool overwrite )
{
	KeyList rejects;
	joinTree( other, overwrite, "", rejects );
	return rejects;
}

void PropertyMap::joinTree( const isis::util::PropertyMap &other, bool overwrite, util::istring prefix, PropertyMap::KeyList &rejects )
{
	iterator thisIt = begin();

	for ( const_iterator otherIt = other.begin(); otherIt != other.end(); otherIt++ ) { //iterate through the elements of other
		if ( continousFind( thisIt, end(), *otherIt, value_comp() ) ) { // if the element is allready here
			if ( thisIt->second.empty() ) { // if ours is empty
				LOG( Debug, verbose_info ) << "Replacing empty property " << MSubject( thisIt->first ) << " by " << MSubject( otherIt->second );
				thisIt->second = otherIt->second;
			} else if ( ! ( thisIt->second.is_leaf() || otherIt->second.is_leaf() ) ) { // if both are a subtree
				PropertyMap &thisMap = thisIt->second.getBranch();
				const PropertyMap &refMap = otherIt->second.getBranch();
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


void PropertyMap::makeFlatMap( FlatMap &out, key_type key_prefix ) const
{
	for ( const_iterator i = begin(); i != end(); i++ ) {
		key_type key = ( key_prefix.empty() ? "" : key_prefix + pathSeperator ) + i->first;

		if ( i->second.is_leaf()  ) {
			out.insert( std::make_pair( key, i->second.getLeaf() ) );
		} else {
			i->second.getBranch().makeFlatMap( out, key );
		}
	}
}

PropertyMap::FlatMap PropertyMap::getFlatMap() const
{
	isis::util::PropertyMap::FlatMap buff;
	makeFlatMap( buff );
	return buff;
}


bool PropertyMap::transform( key_type from,  key_type to, int dstID, bool delSource )
{
	const TypeValue &found = propertyValue( from );
	bool ret = false;

	if( ! found.empty() ) {
		util::TypeReference &dst = static_cast<util::TypeReference &>( propertyValue( to ) );

		if ( found->typeID() == dstID ) {
			if( from != to ) {
				dst = found ;
				ret = true;
			} else {
				LOG( Debug, info ) << "Not transforming " << MSubject( found ) << " into same type at same place.";
			}
		} else {
			LOG_IF( from == to, Debug, warning ) << "Transforming " << MSubject( found ) << " in place.";
			dst = found->copyToNewByID( dstID );
			ret = !dst.empty();
		}
	}

	if ( ret && delSource )remove( from );

	return ret;
}


const PropertyMap::KeyList PropertyMap::getKeys()const
{
	PropertyMap::KeyList ret;
	std::for_each( begin(), end(), walkTree<trueP>( ret ) );
	return ret;
}

const PropertyMap::KeyList PropertyMap::getMissing() const
{
	PropertyMap::KeyList ret;
	std::for_each( begin(), end(), walkTree<invalidP>( ret ) );
	return ret;
}


void PropertyMap::addNeeded( const key_type &key )
{
	propertyValue( key ).needed() = true;
}


void PropertyMap::addNeededFromString( const std::string &needed )
{
	const std::list<std::string> needList = util::string2list<std::string>( needed );
	//@todo util::string2list<std::string>( needed,' ' ) would be faster but less robust
	LOG( Debug, verbose_info ) << "Adding " << needed << " as needed";
	BOOST_FOREACH( std::list<std::string>::const_reference ref, needList ) {
		addNeeded( key_type( ref.c_str() ) );
	}
}

/// \returns true if a leaf exists at the given path and the property is not empty
bool PropertyMap::hasProperty( const key_type &key ) const
{
	const propPath path = util::string2list<key_type>( key, pathSeperator );
	const mapped_type *ref = findEntry( *this, path.begin(), path.end() );
	return ( ref && ref->is_leaf() && ! ref->getLeaf().empty() );
}
/// \returns true if a leaf exists at the given path and the property is not empty
bool PropertyMap::hasBranch( const key_type &key ) const
{
	const propPath path = util::string2list<key_type>( key, pathSeperator );
	const mapped_type *ref = findEntry( *this, path.begin(), path.end() );
	return ( ref && ! ref->is_leaf()  );
}

bool PropertyMap::rename( key_type oldname, key_type newname )
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

void PropertyMap::toCommonUnique( PropertyMap &common, std::set<key_type> &uniques, bool init )const
{
	if ( init ) {
		common = *this;
		uniques.clear();
		return;
	} else {
		const DiffMap difference = common.getDifference( *this );
		BOOST_FOREACH( const DiffMap::value_type & ref, difference ) {
			uniques.insert( ref.first );

			if ( ! ref.second.first.empty() )common.remove( ref.first );//if there is something in common, remove it
		}
	}
}

std::ostream &PropertyMap::print( std::ostream &out, bool label )const
{
	FlatMap buff;
	makeFlatMap( buff );
	size_t key_len = 0;

	for ( FlatMap::const_iterator i = buff.begin(); i != buff.end(); i++ )
		if ( key_len < i->first.length() )
			key_len = i->first.length();

	for ( FlatMap::const_iterator i = buff.begin(); i != buff.end(); i++ )
		out << i->first << std::string( key_len - i->first.length(), ' ' ) + ":" << i->second.toString( label ) << std::endl;

	return out;
}

bool PropertyMap::trueP::operator()( const PropertyMap::value_type &ref ) const
{
	return true;
}
bool PropertyMap::invalidP::operator()( const PropertyMap::value_type &ref ) const
{
	return ref.second.getLeaf().needed() && ref.second.getLeaf().empty();
}
bool PropertyMap::treeInvalidP::operator()( const PropertyMap::value_type &ref ) const
{
	if ( ref.second.is_leaf() ) {
		const TypeValue &val = ref.second.getLeaf();
		return val.needed() && val.empty();
	} else  {
		return ! ref.second.getBranch().isValid();
	}
}

}
}
