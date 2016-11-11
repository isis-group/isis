// kate: indent-width 4; auto-insert-doxygen on
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

namespace isis
{
namespace util
{
API_EXCLUDE_BEGIN;
/// @cond _internal
namespace _internal
{

struct MapStrAdapter: boost::static_visitor<PropertyValue> {
	PropertyValue operator()( const PropertyValue &val )const {return val;}
	PropertyValue operator()( const PropertyMap &map )const {
		return PropertyValue( std::string( "[[PropertyMap with " ) + std::to_string( map.container.size() ) + " entries]]" );
	}
};
struct RemoveEqualCheck: boost::static_visitor<bool> {
	bool removeNeeded;
	RemoveEqualCheck( bool _removeNeeded ): removeNeeded( _removeNeeded ) {}
	bool operator()( PropertyValue &first, const PropertyValue &second )const { // if both are Values
		if( first != second ) {
			return false;
		} else // if they're not unequal (note empty values are neither equal nor unequal)
			return removeNeeded || !first.isNeeded();
	}
	bool operator()( PropertyMap &thisMap, const PropertyMap &otherMap )const { // recurse if both are subtree
		thisMap.removeEqual( otherMap );
		return false;
	}
	template<typename T1, typename T2> bool operator()( T1 &first, const T2 &second )const {return false;} // any other case
};
struct JoinTreeVisitor: boost::static_visitor<bool> {
	bool overwrite,delsource;
	PropertyMap::PathSet &rejects;
	const PropertyMap::PropPath &prefix, &name;
	JoinTreeVisitor( bool _overwrite, bool _delsource, PropertyMap::PathSet &_rejects, const PropertyMap::PropPath &_prefix, const PropertyMap::PropPath &_name )
		: overwrite( _overwrite ), delsource(_delsource), rejects( _rejects ), prefix( _prefix ), name( _name ) {}
	bool operator()( PropertyValue &first, PropertyValue &second )const { // if both are Values
		if( first.isEmpty() || overwrite ) { // if ours is empty or overwrite is enabled
			//replace ours by the other
			if(delsource)first.transfer(second,true);
			else first = second; 
			return true;
		} else { // otherwise put the other into rejected if its unequal to ours
			if( first != second && !second.isEmpty())
				rejects.insert( rejects.end(), prefix / name );
			return false;
		}
	}
	bool operator()( PropertyMap &thisMap, PropertyMap &otherMap )const { // recurse if both are subtree
		thisMap.joinTree( otherMap, overwrite, delsource, prefix / name, rejects );
		return rejects.empty();
	}
	template<typename T1, typename T2> bool operator()( T1 &first, const T2 &second )const {// reject any other case
		rejects.insert( rejects.end(), prefix / name );
		return false;
	} 
};
struct FlatMapMaker: boost::static_visitor<void> {
	PropertyMap::FlatMap &out;
	const PropertyMap::PropPath &name;
	FlatMapMaker( PropertyMap::FlatMap &_out, const PropertyMap::PropPath &_name ): out( _out ), name( _name ) {}
	void operator()( const PropertyValue &value )const {
		out[name] = value;
	}
	void operator()( const PropertyMap &map )const {
		for ( PropertyMap::container_type::const_iterator i = map.container.begin(); i != map.container.end(); i++ ) {
			boost::apply_visitor( FlatMapMaker( out, name / i->first ), i->second );
		}
	}
};
struct TreeInvalidCheck: boost::static_visitor<bool> {
	bool operator()( PropertyMap::container_type::const_reference ref ) const {
		return boost::apply_visitor( *this, ref.second );
	}//recursion
	bool operator()( const PropertyValue &val )const {
		return PropertyMap::InvalidP()( val );
	}
	bool operator()( const PropertyMap &sub )const { //call my own recursion for each element
		return std::find_if( sub.container.begin(), sub.container.end(), *this ) != sub.container.end();
	}
};

}
/// @endcond _internal
API_EXCLUDE_END;

///////////////////////////////////////////////////////////////////
// PropPath impl
///////////////////////////////////////////////////////////////////


PropertyMap::PropPath::PropPath() {}
PropertyMap::PropPath::PropPath( const char *key ): std::list<key_type>( stringToList<key_type>( istring( key ), pathSeperator ) ) {}
PropertyMap::PropPath::PropPath( const key_type &key ): std::list<key_type>( stringToList<key_type>( key, pathSeperator ) ) {}
PropertyMap::PropPath::PropPath( const std::list<key_type> &path ): std::list<key_type>( path ) {}
PropertyMap::PropPath &PropertyMap::PropPath::operator/=( const PropertyMap::PropPath &s )
{
	insert( end(), s.begin(), s.end() );
	return *this;
}
PropertyMap::PropPath PropertyMap::PropPath::operator/( const PropertyMap::PropPath &s )const {return PropPath( *this ) /= s;}

size_t PropertyMap::PropPath::length()const
{
	if( empty() )return 0;

	size_t ret = 0;
	for( const_reference ref :  *this )
	ret += ref.length();
	return ret + size() - 1;
}

std::string PropertyMap::PropPath::toString()const
{
	std::stringstream out;
	out << *this;
	return out.str();
}
///////////////////////////////////////////////////////////////////
// Contructors
///////////////////////////////////////////////////////////////////

PropertyMap::PropertyMap( const PropertyMap::container_type &src ): container( src ) {}
PropertyMap::PropertyMap() {}


///////////////////////////////////////////////////////////////////
// The core tree traversal functions
///////////////////////////////////////////////////////////////////
PropertyMap::mapped_type &PropertyMap::fetchEntry( const PropPath &path ) throw( boost::bad_get )
{
	assert(!path.empty());
	return fetchEntry( container, path.begin(), path.end() );
}
PropertyMap::mapped_type &PropertyMap::fetchEntry( container_type &root, const propPathIterator at, const propPathIterator pathEnd ) throw( boost::bad_get )
{
	PropPath::const_iterator next = at;
	next++;
	container_type::iterator found = root.find( *at );

	if ( next != pathEnd ) {//we are not at the end of the path (a proposed leaf in the PropMap)
		if ( found != root.end() ) {//and we found the entry
			return fetchEntry( boost::get<PropertyMap>( found->second ).container, next, pathEnd ); //continue there
		} else { // if we should create a sub-map
			//insert a empty branch (aka PropMap) at "*at" (and fetch the reference of that)
			LOG( Debug, verbose_info ) << "Creating an empty branch " << *at << " trough fetching";
			return fetchEntry( boost::get<PropertyMap>( root[*at] = PropertyMap() ).container, next, pathEnd ); // and continue there (default value of the variant is PropertyValue, so init it to PropertyMap)
		}
	} else { //if its the leaf
		return root[*at]; // (create and) return that entry
	}
}

optional<const PropertyMap::mapped_type &> PropertyMap::findEntry( const PropPath &path  )const throw( boost::bad_get )
{
	assert(!path.empty());
	return findEntryImpl<const mapped_type,const container_type>( container, path.begin(), path.end() );
}
optional<PropertyMap::mapped_type &> PropertyMap::findEntry( const PropPath &path  )throw( boost::bad_get )
{
	assert(!path.empty());
	return findEntryImpl<mapped_type,container_type>( container, path.begin(), path.end() );
}

bool PropertyMap::recursiveRemove( container_type &root, const propPathIterator pathIt, const propPathIterator pathEnd )throw( boost::bad_get )
{
	bool ret = false;

	if ( pathIt != pathEnd ) {
		propPathIterator next = pathIt;
		next++;
		const container_type::iterator found = root.find( *pathIt );

		if ( found != root.end() ) {
			if ( next != pathEnd ) {
				PropertyMap &ref = boost::get<PropertyMap>( found->second );
				ret = recursiveRemove( ref.container, next, pathEnd );

				if ( ref.isEmpty() )
					root.erase( found ); // remove the now empty branch
			} else {
				LOG_IF( found->second.type() == typeid( PropertyMap ) && !boost::get<PropertyMap>( found->second ).isEmpty(), Debug, warning )
						<< "Deleting non-empty branch " << MSubject( found->first );
				root.erase( found );
				ret = true;
			}
		} else {
			LOG( Debug, warning ) << "Ignoring unknown entry " << *pathIt;
		}
	}

	return ret;
}


/////////////////////////////////////////////////////////////////////////////////////
// Interface for accessing elements
////////////////////////////////////////////////////////////////////////////////////

boost::optional< PropertyValue& > PropertyMap::queryProperty(const PropertyMap::PropPath& path)
{
	return tryFindEntry<PropertyValue>( path );
}
boost::optional< const PropertyValue& > PropertyMap::queryProperty(const PropertyMap::PropPath& path) const
{
	return tryFindEntry<PropertyValue>( path );
}
PropertyValue &PropertyMap::touchProperty( const PropertyMap::PropPath &path )
{
	return tryFetchEntry<PropertyValue>( path ).get();
}
const PropertyValue PropertyMap::property(const PropertyMap::PropPath& path) const
{
	const boost::optional< const PropertyValue& > prop = queryProperty(path);
	return prop ? prop.get() : PropertyValue();
}

boost::optional< PropertyMap& > PropertyMap::queryBranch(const PropertyMap::PropPath& path)
{
	return tryFindEntry<PropertyMap>( path );
}
boost::optional< const PropertyMap& > PropertyMap::queryBranch(const PropertyMap::PropPath& path) const
{
	return tryFindEntry<PropertyMap>( path );
}
PropertyMap& PropertyMap::touchBranch(const PropertyMap::PropPath& path)
{
	return tryFetchEntry<PropertyMap>( path ).get();
}
PropertyMap PropertyMap::branch( const PropertyMap::PropPath &path ) const
{
	const boost::optional< const PropertyMap& > branch=queryBranch( path );
	return branch ? branch.get() : PropertyMap();
}

bool PropertyMap::remove( const PropPath &path )
{
	try {
		return recursiveRemove( container, path.begin(), path.end() );
	} catch( const boost::bad_get &e ) {
		LOG( Runtime, error ) << "Got errror " << e.what() << " when removing " << path << ", aborting the removal.";
		return false;
	}
}

bool PropertyMap::remove( const PathSet &removeList, bool keep_needed )
{
	bool ret = true;
	for( PathSet::const_reference key :  removeList ) {
		optional< PropertyValue& > found = tryFindEntry<PropertyValue>( key );

		if( found ) { // remove everything which is there
			if( !(found->isNeeded() && keep_needed) ) { // if its not needed or keep_need is not true
				ret &= remove( key );
			}
		} else {
			LOG( Debug, notice ) << "Can't remove property " << key << " as its not there";
		}
	}
	return ret;
}


bool PropertyMap::remove( const PropertyMap &removeMap, bool keep_needed )
{
	container_type::iterator thisIt = container.begin();
	bool ret = true;

	//remove everything that is also in second
	for ( container_type::const_iterator otherIt = removeMap.container.begin(); otherIt != removeMap.container.end(); otherIt++ ) {
		//find the closest match for otherIt->first in this (use the value-comparison-functor of PropMap)
		if ( continousFind( thisIt, container.end(), *otherIt, container.value_comp() ) ) { //thisIt->first == otherIt->first - so its the same property or propmap
			if ( thisIt->second.type() == typeid( PropertyMap ) && otherIt->second.type() == typeid( PropertyMap ) ) { //both are a branch => recurse
				PropertyMap &mySub = boost::get<PropertyMap>( thisIt->second );
				const PropertyMap &otherSub = boost::get<PropertyMap>( otherIt->second );
				ret &= mySub.remove( otherSub );

				if( mySub.isEmpty() ) // delete my branch, if its empty
					container.erase( thisIt++ );
			} else if( thisIt->second.type() == typeid( PropertyValue ) && otherIt->second.type() == typeid( PropertyValue ) ) {
				container.erase( thisIt++ ); // so delete this (they are equal - kind of)
			} else { // this is a leaf
				LOG( Debug, warning ) << "Not deleting branch " << MSubject( thisIt->first ) << " because its no subtree on one side";
				ret = false;
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
	return !_internal::TreeInvalidCheck()( *this );
}

bool PropertyMap::isEmpty() const
{
	return container.empty();
}

PropertyMap::DiffMap PropertyMap::getDifference( const PropertyMap &other ) const
{
	PropertyMap::DiffMap ret;
	diffTree( other.container, ret, PropPath() );
	return ret;
}

void PropertyMap::diffTree( const container_type& other, DiffMap& ret, const PropPath& prefix ) const
{
	container_type::const_iterator otherIt = other.begin();

	//insert everything that is in this, but not in second or is on both but differs
	for ( container_type::const_iterator thisIt = container.begin(); thisIt != container.end(); thisIt++ ) {
		//find the closest match for thisIt->first in other (use the value-comparison-functor of the container)
		if ( continousFind( otherIt, other.end(), *thisIt, container.value_comp() ) ) { //otherIt->first == thisIt->first - so its the same property
			if( thisIt->second.type() == typeid( PropertyMap ) && otherIt->second.type() == typeid( PropertyMap ) ) { // both are branches -- recursion step
				const PropertyMap &thisMap = boost::get<PropertyMap>( thisIt->second ), &refMap = boost::get<PropertyMap>( otherIt->second );
				thisMap.diffTree( refMap.container, ret, prefix / thisIt->first );
			} else if( thisIt->second.type() == typeid( PropertyValue ) && otherIt->second.type() == typeid( PropertyValue ) ) { // both are PropertyValue
				const PropertyValue &thisVal = boost::get<PropertyValue>( thisIt->second ), &otherVal = boost::get<PropertyValue>( otherIt->second );
				if( thisVal != otherVal ) // if they are different
					ret.insert( // add (propertyname|(value1|value2))
						ret.end(),      // we know it has to be at the end
						std::make_pair(
							prefix / thisIt->first,   //the key
							std::make_pair( thisVal, otherVal ) //pair of both values
						)
					);
			} else { // obviously different just stuff it in
				ret.insert( ret.end(), std::make_pair( prefix / thisIt->first, std::make_pair(
						boost::apply_visitor( _internal::MapStrAdapter(), thisIt->second ),
						boost::apply_visitor( _internal::MapStrAdapter(), otherIt->second )
													   ) ) );
			}
		} else { // if ref is not in the other map 
			const PropertyValue firstVal = boost::apply_visitor( _internal::MapStrAdapter(), thisIt->second );
			ret.insert( // add (propertyname|(value1|[empty]))
				ret.end(),      // we know it has to be at the end
				std::make_pair(
					prefix / thisIt->first,
					std::make_pair( firstVal, PropertyValue() )
				)
			);
		}
	}

	//insert everything that is in second but not in this
	container_type::const_iterator thisIt = container.begin();

	for ( otherIt = other.begin(); otherIt != other.end(); otherIt++ ) {
		if ( ! continousFind( thisIt, container.end(), *otherIt, container.value_comp() ) ) { //there is nothing in this which has the same key as ref

			const PropertyValue secondVal = boost::apply_visitor( _internal::MapStrAdapter(), otherIt->second );
			ret.insert(
				std::make_pair( // add (propertyname|([empty]|value2))
					prefix / otherIt->first,
					std::make_pair( PropertyValue(), secondVal )
				)
			);
		}
	}
}

void PropertyMap::removeEqual ( const PropertyMap &other, bool removeNeeded )
{
	container_type::iterator thisIt = container.begin();

	//remove everything that is also in second and equal (or also empty)
	for ( container_type::const_iterator otherIt = other.container.begin(); otherIt != other.container.end(); otherIt++ ) {
		//find the closest match for otherIt->first in this (use the value-comparison-functor of PropMap)
		if ( continousFind( thisIt, container.end(), *otherIt, container.value_comp() ) ) { //thisIt->first == otherIt->first  - so its the same property

			//          if(thisIt->second.type()==typeid(PropertyValue) && otherIt->second.type()==typeid(PropertyValue)){ // if both are Values
			if( boost::apply_visitor( _internal::RemoveEqualCheck( removeNeeded ), thisIt->second, otherIt->second ) ) {
				container.erase( thisIt++ ); // so delete this if both are empty _or_ equal
				continue; // keep iterator from incrementing again
			} else
				thisIt++;
		}
	}
}


PropertyMap::PathSet PropertyMap::join( const PropertyMap &other, bool overwrite)
{
	PathSet rejects;
	joinTree( const_cast<PropertyMap &>(other), overwrite, false, PropPath(), rejects );
	LOG_IF(!rejects.empty(),Debug,info) << "The properties " << MSubject(rejects) << " where rejected during the join";
	return rejects;
}
PropertyMap::PathSet PropertyMap::transfer(PropertyMap& other, int overwrite)
{
	PathSet rejects;
	joinTree( other, overwrite, true, PropPath(), rejects );
	LOG_IF(!rejects.empty(),Debug,info) << "The properties " << MSubject(rejects) << " where rejected during the transfer";
	return rejects;
}
PropertyMap::PathSet PropertyMap::transfer(PropertyMap& other, const PropPath &path, bool overwrite)
{
	optional< PropertyMap& > b=other.queryBranch(path);
	PathSet rejects;
	if(b){
		rejects=transfer(*b,overwrite);
		if(rejects.empty())
			other.remove(path);
	} else
		LOG(Runtime,error) << "Branch " << path << " does not exist, won't do anything";
	return rejects;
}


void PropertyMap::joinTree( PropertyMap &other, bool overwrite, bool delsource, const PropPath &prefix, PathSet &rejects )
{
	container_type::iterator thisIt = container.begin();

	for ( container_type::iterator otherIt = other.container.begin(); otherIt != other.container.end(); ) { //iterate through the elements of other
		if ( continousFind( thisIt, container.end(), *otherIt, container.value_comp() ) ) { // if the element is allready here
			if(
				boost::apply_visitor( _internal::JoinTreeVisitor( overwrite, delsource, rejects, prefix, thisIt->first ), thisIt->second, otherIt->second ) &&
				delsource
			){// if the join was complete and delsource is true
				other.container.erase(otherIt++); // remove the entry from the source
			} else {
				otherIt++;
			}
				
		} else { // ok we dont have that - just insert it
			if(delsource){ // if we don't need the source anymore
				otherIt->second.swap(container[otherIt->first]); //swap it with the empty (because newly created) entry in the destination
				other.container.erase(otherIt++);//remove now empty entry
			} else { // insert a copy
				const std::pair<container_type::const_iterator, bool> inserted = container.insert( *otherIt );
				LOG_IF( !inserted.second, Debug, warning ) << "Failed to insert property " << MSubject( *inserted.first );
				otherIt++;
			}
		}
	}
}

PropertyMap::FlatMap PropertyMap::getFlatMap() const
{
	FlatMap buff;
	_internal::FlatMapMaker( buff, PropPath() ).operator()( *this );
	return buff;
}


bool PropertyMap::transform( const PropPath &from,  const PropPath &to, uint16_t dstID)
{
	const boost::optional< PropertyValue& > src = queryProperty( from );
	if(!src || src->isEmpty())
		return false;
	
	if ( src->getTypeID() == dstID ) { //same type - just rename it
		if( from != to ) // if its not at the same place anyway
			return rename(from,to);
		else
			LOG( Debug, info ) << "Not transforming " << MSubject( src ) << " into same type at same place.";
	} else { // if not the same -- convert
		LOG_IF( from == to, Debug, notice ) << "Transforming " << MSubject( src ) << " in place.";
		PropertyValue buff= src->copyByID( dstID );

		if( !buff.isEmpty() ){
			touchProperty( to ).swap(buff);
			if(from!=to)remove( from );
			return true;
		}
	}
	return false;
}


PropertyMap::PathSet PropertyMap::getKeys()const   {return genKeyList<TrueP>();}
PropertyMap::PathSet PropertyMap::getMissing()const {return genKeyList<InvalidP>();}

PropertyMap::PathSet PropertyMap::getLocalBranches() const
{
	return getLocal<PropertyMap>();
}
PropertyMap::PathSet PropertyMap::getLocalProps() const
{
	return getLocal<PropertyValue>();
}


void PropertyMap::addNeeded( const PropPath &path )
{
	touchProperty( path ).needed() = true;
}


size_t PropertyMap::hasProperty( const PropPath &path ) const
{
	const optional< const PropertyValue& > ref = queryProperty( path );
	return ref ? ref->size():0; 
}

PropertyMap::PropPath PropertyMap::find( const key_type &key, bool allowProperty, bool allowBranch ) const
{
	const PropPath name(key);
	if( name.empty() ) {
		LOG( Debug, error ) << "Search key " << MSubject( name ) << " is invalid, won't search";
		return key_type();
	}

	LOG_IF( name.size() > 1, Debug, warning ) << "Stripping search key " << MSubject( name ) << " to " << name.back();

	// if the searched key is on this brach return its name
	container_type::const_iterator found = container.find( name.back() );

	if( found != container.end() &&
		( ( found->second.type() == typeid( PropertyValue ) && allowProperty ) || ( found->second.type() == typeid( PropertyMap ) && allowBranch ) )
	  ) {
		return found->first;
	} else { // otherwise search in the branches
		for( container_type::const_reference ref :  container ) {
			if( ref.second.type() == typeid( PropertyMap ) ) {
				const PropPath found = boost::get<PropertyMap>( ref.second ).find( name.back(), allowProperty, allowBranch );

				if( !found.empty() ) // if the key is found abort search and return it with its branch-name
					return PropPath( ref.first ) / found;
			}
		}
	}

	return PropPath(); // nothing found
}

bool PropertyMap::hasBranch( const PropPath &path ) const
{
	const optional< const PropertyMap & > q=queryBranch( path );
	return q && !q->isEmpty();
}

bool PropertyMap::rename( const PropPath &oldname,  const PropPath &newname, bool overwrite )
{
	optional<mapped_type &> old_e = findEntry( oldname );

	if ( !old_e ) {//abort if oldname is not there
		LOG( Runtime, error ) << "Cannot rename " << oldname << " it does not exist";
		return false;
	}
	if(newname != oldname){
		mapped_type &new_e = fetchEntry( newname );
		const bool empty=boost::apply_visitor(IsEmpty(), new_e);
		if(!empty && !overwrite){ //abort if we're no supposed to overwrite'
			LOG(Runtime,warning) << "Not overwriting " << std::make_pair( newname, new_e ) << " with " << *old_e;
			return false;
		}
		LOG_IF( !empty , Runtime, warning ) << "Overwriting " << std::make_pair( newname, new_e ) << " with " << *old_e;
		new_e.swap(*old_e);
		remove( oldname ); //can only fail if oldname is not there -- and if it wasn't we'd have aborted already 
	} else { //if its a lexical rename get the data out of the map
		mapped_type buff;old_e->swap(buff);
		remove( oldname ); 
		fetchEntry(newname).swap(buff);//and re-insert it with newname
	}
	return true;
}

void PropertyMap::removeUncommon( PropertyMap &common )const
{
#pragma message("getDifference is waste of time here")
	const DiffMap difference = common.getDifference( *this ); 
	for( const DiffMap::value_type & ref :  difference ) {
		if ( ! ref.second.first.isEmpty() ) {
			LOG( Debug, verbose_info ) << "Detected difference in " << ref << " removing from common";
			common.remove( ref.first );//if there is something in common, remove it
		}
	}
}

std::ostream &PropertyMap::print( std::ostream &out, bool label )const
{
	FlatMap buff = getFlatMap();
	size_t key_len = 0;

	for ( FlatMap::const_iterator i = buff.begin(); i != buff.end(); i++ )
		if ( key_len < i->first.length() )
			key_len = i->first.length();

	for ( FlatMap::const_iterator i = buff.begin(); i != buff.end(); i++ )
		out << i->first << std::string( key_len - i->first.length(), ' ' ) + ":" << i->second.toString( label ) << std::endl;

	return out;
}

PropertyValue& PropertyMap::setValue(const PropPath &path, const ValueBase &val, size_t at){
	PropertyValue &ret = touchProperty( path );

	if( ret.size() <= at ) {
		LOG_IF(at, Debug, info ) << "Extending " << MSubject( std::make_pair( path, ret ) ) << " to fit length " << MSubject( at ); //dont tell about extending empty property
		ret.resize( at + 1, val ); //resizing will insert the value
	} else {
		if( ret[at].apply( val ) ){ // override same type or convert if possible
			LOG_IF( ret.getTypeID()!=val.getTypeID(), Debug, warning ) << "Storing " << val.toString( true ) << " as " << ret.toString( true ) << " as Property already has that type";
		} else {
			LOG( Runtime, error ) << "Property " << path << " is already set to " << ret.toString( true ) << " won't overwrite with " << val.toString( true );
		}
	}

	return ret;
}
PropertyValue& PropertyMap::setValue( const PropPath &path, const ValueBase &val ) {
	PropertyValue &ret = touchProperty( path );

	if( ret.isEmpty() ) { // set an empty property
		ret = val;
	} else if( ret.size() == 1 ) {
		if( ret[0].apply( val ) ){ // override same type or convert if possible
			LOG_IF( ret.getTypeID()!=val.getTypeID(), Debug, warning ) << "Storing " << val.toString( true ) << " as " << ret.toString( true ) << " as Property already has that type";
		} else {
			LOG( Runtime, error ) << "Property " << path << " is already set to " << ret.toString( true ) << " won't overwrite with " << val.toString( true );
		}
	} else
		LOG( Runtime, error ) << "Won't override multivalue property " << path << " with " << val.toString( true );

	return ret;
}


PropertyValue& PropertyMap::setValueAs(const PropertyMap::PropPath& path, const char* val, size_t at){
	return setValueAs<std::string>(path,val,at);
}
PropertyValue& PropertyMap::setValueAs(const PropertyMap::PropPath& path, const char* val){
	return setValueAs<std::string>(path,val);
}


/// @cond _internal
bool PropertyMap::TrueP::operator()( const PropertyValue &/*ref*/ ) const {return true;}
bool PropertyMap::InvalidP::operator()( const PropertyValue &ref ) const {return ref.isNeeded() && ref.isEmpty();}
bool PropertyMap::EmptyP::operator()(const PropertyValue& ref) const {return ref.isEmpty();};

/// @endcond _internal

}
}
