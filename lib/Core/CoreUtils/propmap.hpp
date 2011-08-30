//
// C++ Interface: propmap
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef ISISPROPMAP_HPP
#define ISISPROPMAP_HPP

#include <map>
#include <string>

#include "common.hpp"
#include "property.hpp"
#include "log.hpp"
#include "istring.hpp"
#include <set>
#include <algorithm>

namespace isis
{
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util
{
namespace _internal
{
class treeNode; //predeclare treeNode -- we'll need it in PropertyMap
}
/**
 * This class contains a mapping tree to store all kinds of properties (path/key : value)
 * It's also a generic class to have the probability to handle
 * different modalities in upper levels (in our special case we'll have to handle things like images or other
 * time series of data each having different properties in its standard or wherever from). Here, you can hierarchilly
 * setup a property tree to sort all these things. To ensure essential properties for a special case, you'll need to define
 * the needed properties. For all the other play-around with PropertyMaps see extensive documentation below!!!
 *
 */
class PropertyMap : private std::map<util::istring, _internal::treeNode>
{
public:
	/**
	 * type of the used keys
	 */
	typedef key_type KeyType;
	/**
	 * a list to store keys only (without the corresponding values)
	 */
	typedef std::set<KeyType, key_compare> KeyList;
	/**
	 * a map to match keys to pairs of values
	 */
	typedef std::map<KeyType, std::pair<PropertyValue, PropertyValue>, key_compare> DiffMap;
	/**
	 * a map, using complete key-paths as keys for the corresponding values
	 */
	typedef std::map<KeyType, PropertyValue> FlatMap;
private:
	typedef std::map<KeyType, mapped_type, key_compare> Container;
	typedef std::list<KeyType> propPath;
	typedef propPath::const_iterator propPathIterator;

	static const char pathSeperator = '/';
	static const mapped_type emptyEntry;//dummy to be able to return an empty Property/branch


	/////////////////////////////////////////////////////////////////////////////////////////
	// internal predicats
	/////////////////////////////////////////////////////////////////////////////////////////
	/// allways true
	struct trueP {  bool operator()( const_reference ref )const;};
	/// true when entry is a leaf, needed and empty
	struct invalidP {   bool operator()( const_reference ref )const;};
	/// true when entry is a leaf, needed and empty of entry is a invalid branch
	struct treeInvalidP {   bool operator()( const_reference ref )const;};
	/////////////////////////////////////////////////////////////////////////////////////////
	// internal functors
	/////////////////////////////////////////////////////////////////////////////////////////
	template<class Predicate> struct walkTree;

	/////////////////////////////////////////////////////////////////////////////////////////
	// internal tool-backends
	/////////////////////////////////////////////////////////////////////////////////////////
	/// internal recursion-function for join
	void joinTree( const isis::util::PropertyMap &other, bool overwrite, KeyType prefix, PropertyMap::KeyList &rejects );
	/// internal recursion-function for diff
	void diffTree( const PropertyMap &other, PropertyMap::DiffMap &ret, KeyType prefix ) const;

	static mapped_type &fetchEntry( util::PropertyMap &root, const propPathIterator at, const propPathIterator pathEnd );
	mapped_type &fetchEntry( const KeyType &key );

	static const mapped_type *findEntry( const util::PropertyMap &root, const propPathIterator at, const propPathIterator pathEnd );
	const mapped_type *findEntry( const KeyType &key )const;

	/// internal recursion-function for remove
	bool recursiveRemove( util::PropertyMap &root, const propPathIterator at, const propPathIterator pathEnd );
protected:
	/////////////////////////////////////////////////////////////////////////////////////////
	// rw-backends
	/////////////////////////////////////////////////////////////////////////////////////////
	/// create a list of keys for every entry for which the given scalar predicate is true.
	template<class Predicate> const KeyList genKeyList()const {
		KeyList k;
		std::for_each( begin(), end(), walkTree<Predicate>( k ) );
		return k;
	}
	/**
	* Make Properties given by a space separated list needed.
	* \param needed string made of space serparated property-names which
	* will (if neccessary) be added to the PropertyMap and flagged as needed.
	*/
	void addNeededFromString( const std::string &needed );
	/**
	* Adds a property with status needed.
	* \param key identifies the property to be added or if already existsing to be flagged as needed
	*/
	void addNeeded( const KeyType &key );

	/**
	 * Remove every PropertyValue which is also in the other PropertyMap and where operator== returns true.
	 * \param other the other PropertyMap to compare to
	 * \param removeNeeded if a PropertyValue should also be deleted if they're needed
	 */
	void removeEqual( const isis::util::PropertyMap &other, bool removeNeeded = false );

	/**
	 * Get common and unique properties from the map.
	 * For every entry of the map this checks if it is common/unique and removes/adds it accordingly.
	 * This is done by:
	 * - generating a difference (using diff) between the current common and the map
	 * - the resulting diff_map contains all newly unique properties (properties which has been in common, but are not euqual in the map)
	 * - these newly diffent properties are removed from common and added to unique.
	 * - if init is true uniques is cleared and common is replaced by a copy of the map (shall be done at first step/map)
	 * \param common reference of the common-map
	 * \param uniques reference of the unique-map
	 * \param init if initialisation shall be done instead of normal seperation
	 */
	void toCommonUnique( PropertyMap &common, std::set<KeyType> &uniques, bool init )const;

	///copy the tree into a flat key/property-map
	void makeFlatMap( FlatMap &out, KeyType key_prefix = "" )const;

	/**
	 * Access the property vector referenced by the path-key.
	 * If the property does not exist, an empty dummy will returned.
	 * \param key the "path" to the property
	 * \returns a reference to the PropertyValue
	 */
	const std::vector<PropertyValue> &propertyValueVec( const KeyType &key )const;

	/**
	 * Access the property vector referenced by the path-key, create it if its not there.
	 * \param key the "path" to the property
	 * \returns a reference to the PropertyValue
	 */
	std::vector<PropertyValue> &propertyValueVec( const KeyType &key );
public:
	/////////////////////////////////////////////////////////////////////////////////////////
	// constructors
	/////////////////////////////////////////////////////////////////////////////////////////
	PropertyMap( const Container &src );
	PropertyMap();

	bool operator==( const PropertyMap &src )const;

	/////////////////////////////////////////////////////////////////////////////////////////
	// Common rw-accessors
	/////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Access the property referenced by the path-key.
	 * If the property does not exist, an empty dummy will returned.
	 * \param key the "path" to the property
	 * \returns a reference to the PropertyValue
	 */
	const PropertyValue &propertyValue( const KeyType &key )const;

	/**
	 * Access the property referenced by the path-key, create it if its not there.
	 * \param key the "path" to the property
	 * \returns a reference to the PropertyValue
	 */
	PropertyValue &propertyValue( const KeyType &key );

	/**
	 * Access the branch referenced by the path-key, create it if its not there.
	 * \param key the "path" to the branch
	 * \returns a reference to the branching PropertyMap
	 */
	PropertyMap &branch( const KeyType &key );

	/**
	 * Access the branch referenced by the path-key.
	 * If the branch does not exist, an empty dummy will returned.
	 * \param key the "path" to the branch
	 * \returns a reference to the branching PropertyMap
	 */
	const PropertyMap &branch( const KeyType &key )const;

	/**
	 * Remove the property adressed by the key.
	 * This actually only removes properties.
	 * Non-empty branches are not deleted.
	 * And if an branch becomes empty after deletion of its last entry, it is deleted automatically.
	 * \param key the "path" to the property
	 * \returns true if successful, false otherwise
	 */
	bool remove( const KeyType &key );

	/**
	 * remove every property which is also in the given map (regardless of the value)
	 * \param removeMap the map of properties to be removed
	 * \param keep_needed flag
	 * \returns true if all properties removed succesfully, false otherwise
	 */
	bool remove( const isis::util::PropertyMap &removeMap, bool keep_needed = false );

	/**
	 * check if property is available
	 * \param key the "path" to the property
	 * \returns true if the given property does exist and is not empty, false otherwise
	 */
	bool hasProperty( const KeyType &key )const;

	/**
	 * Search for the property/branch in the whole Tree.
	 * \param key the single key for the branch/property to search for (paths will be stripped to the rightmost key)
	 * \param allowProperty if false the search will ignore properties
	 * \param allowBranch if false the search will ignore branches (it will still search into the branches, but the branches themself won't be considered a valid finding)
	 * \returns full "path" to the property (including the properties name) if it is found, empty string elsewhise
	 */
	KeyType find(KeyType key,bool allowProperty=true,bool allowBranch=false)const;

	/**
	 * check if branch of the tree is available
	 * \param key the "path" to the branch
	 * \returns true if the given branch does exist and is not empty, false otherwise
	 */
	bool hasBranch( const KeyType &key )const;

	////////////////////////////////////////////////////////////////////////////////////////
	// tools
	/////////////////////////////////////////////////////////////////////////////////////////
	/**
	* Check if every needed property is set.
	* \returns true if ALL needed properties are NOT empty, false otherwhise.
	*/
	bool isValid()const;

	/**
	 * \returns true if the PropertyMap is empty, false otherwhise
	 */
	bool isEmpty()const;

	/**
	 * Get a list of all properties.
	 * \returns a flat list of the "paths" to all properties in the PropertyMap
	 */
	const KeyList getKeys()const;

	/**
	 * Get a list of missing properties.
	 * \returns a list of all needed and empty properties.
	 */
	const KeyList getMissing()const;

	/**
	 * Get a difference map of actual object and another PropertyMap.
	 * Out of the names of differing properties a map of std::pair\<PropertyValue,PropertyValue\> is created with following rules:
	 * - If a Property is empty in actual object but set in second,
	 *   it will be added with the first PropertyValue emtpy and the second PropertyValue
	 *   taken from second
	 * - If a Property is set in actual object but empty in second,
	 *   it will be added with the first PropertyValue taken from this and the second PropertyValue empty
	 * - If a Property is set in both, but not equal, it will be added with the first PropertyValue taken from this
	 *   and the second PropertyValue taken from second
	 * - If a Property is set in both and equal, it wont be added
	 * - If a Property is empty in both, it wont be added
	 * \param second the "other" PropertyMap to compare with
	 * \returns a map of property keys and pairs of the corresponding different values
	 */
	DiffMap getDifference( const PropertyMap &second )const;

	/**
	* Add Properties from another PropertyMap.
	* \param other the "other" PropertyMap to join with
	* \param overwrite if existing properties shall be replaced
	* \returns a list of the rejected properties that couldn't be inserted, for success this should be empty
	*/
	PropertyMap::KeyList join( const isis::util::PropertyMap &other, bool overwrite = false );

	/**
	 * Transform an existing property into another.
	 * Converts the value of the given property into the requested type and stores it with the given new key.
	 * \param from the key of the property to be transformed
	 * \param to the key for the new property
	 * \param dstID the type-ID of the new property value
	 * \param delSource if the original property shall be deleted after the tramsformation was done
	 * \returns true if the transformation was done
	 */
	bool transform( KeyType from, KeyType to, int dstID, bool delSource = true );

	/**
	 * Transform an existing property into another (statically typed version).
	 * Converts the value of the given property into the requested type and stores it with the given new key.
	 * A compile-time check is done to test if the requested type is available.
	 * \param from the key of the property to be transformed
	 * \param to the key for the new property
	 * \param delSource if the original property shall be deleted after the tramsformation was done
	 * \returns true if the transformation was done, false otherwise
	 */
	template<typename DST> bool transform( KeyType from, KeyType to, bool delSource = true ) {
		checkType<DST>();
		return transform( from, to, Value<DST>::staticID, delSource );
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// Additional get/set - Functions
	//////////////////////////////////////////////////////////////////////////////////////
	//@todo make sure the type specific behaviour is as documented
	/**
	 * Set the given property to a given value/type.
	 * The needed flag (if set) will be kept.
	 * If the property is already set to a value of another type an error is send to Runtime and nothing will be set.
	 * \code
	 * setPropertyAs("MyPropertyName", isis::util::fvector4(1,0,1,0))
	 * \endcode
	 * \param key the "path" to the property
	 * \param val the value to set of type T
	 * \returns a reference to the PropertyValue (this can be used later, e.g. if a vector is filled step by step
	 * the reference can be used to not ask for the Property everytime)
	 */
	template<typename T> PropertyValue &setPropertyAs( const KeyType &key, const T &val ) {
		PropertyValue &ret = propertyValue( key );

		if( ret.isEmpty() ) {
			const bool needed = ret.isNeeded();
			( ret = val ).needed() = needed;
		} else if( ret->is<T>() ) {
			ret->castTo<T>() = val;
		} else { // don't overwrite already set properties with a different type
			LOG( Runtime, error ) << "Property " << MSubject( key ) << " is already set to " << MSubject( ret.toString( true ) ) << " won't override with " << MSubject( Value<T>( val ).toString( true ) );
		}

		return ret;
	}

	/**
	 * Request a property via the given key in the given type.
	 * If the requested type is not equal to type the property is stored with, an automatic conversion is done.
	 * \code
	 * getPropertyAs<isis::util::fvector4>( "MyPropertyName" );
	 * \endcode
	 * \param key the "path" to the property
	 * \returns the property with given type, if not set yet T() is returned.
	 */
	template<typename T> T getPropertyAs( const KeyType &key )const;

	/**
	 * Rename a given property/branch.
	 * This is implemented as copy+delete and can also be used between branches.
	 * - if the target exist a warning will be send, but it will still be overwritten
	 * - if the source does not exist a warning will be send and nothing is done
	 * \returns true if renaming/moving was successful
	 */
	bool rename( KeyType oldname, KeyType newname );

	/**
	 * \returns a flat representation of the whole property tree
	 */
	FlatMap getFlatMap( )const;

	/**
	 * "Print" the PropertyMap.
	 * Will send the name and the result of PropertyValue->toString(label) to the given ostream.
	 * Is equivalent to common streaming operation but has the option to print the type of the printed properties.
	 * \param out the output stream to use
	 * \param label print the type of the property (see Value::toString())
	 */
	std::ostream &print( std::ostream &out, bool label = false )const;
};
}
/** @} */
}

namespace std //predeclare streaming output -- we'll need it in treeNode
{
/// Streaming output for PropertyMap::node
template<typename charT, typename traits>
basic_ostream<charT, traits>& operator<<( basic_ostream<charT, traits> &out, const isis::util::_internal::treeNode &s );
/// Streaming output for PropertyMap
template<typename charT, typename traits>
basic_ostream<charT, traits>& operator<<( basic_ostream<charT, traits> &out, const isis::util::PropertyMap &s );
}


// OK, lets define treeNode
namespace isis
{
namespace util
{
namespace _internal
{
/**
 * Class treeNode is a basic class for PropertyMap to check some basic graph stuff for each node of the property tree
 */
class treeNode
{
	PropertyMap m_branch;
	std::vector<PropertyValue> m_leaf;
public:
	treeNode():m_leaf(1) {}
	bool empty()const {
		return m_branch.isEmpty() && m_leaf[0].isEmpty();
	}
	bool is_leaf()const {
		LOG_IF( ! ( m_branch.isEmpty() || m_leaf[0].isEmpty() ), Debug, error ) << "There is a non empty leaf at a branch. This should not be.";
		return m_branch.isEmpty();
	}
	const PropertyMap &getBranch()const {
		return m_branch;
	}
	PropertyMap &getBranch() {
		return m_branch;
	}
	std::vector<PropertyValue> &getLeaf() {
		assert( is_leaf() );
		return m_leaf;
	}
	const std::vector<PropertyValue> &getLeaf()const {
		assert( is_leaf() );
		return m_leaf;
	}
	bool operator==( const treeNode &ref )const {
		return m_branch == ref.m_branch && m_leaf == ref.m_leaf;
	}
	std::string toString()const {
		std::ostringstream o;
		o << *this;
		return o.str();
	}
};
}

////////////////////////////////////////////////////////////////////////////////////////
// and now we can define walkTree (needs treeNode to be defined)
////////////////////////////////////////////////////////////////////////////////////////

/**
 * Walks the whole tree and inserts any key into out for which the given scalar predicate is true.
 */
template<class Predicate> struct PropertyMap::walkTree {
	KeyList &m_out;
	const KeyType m_prefix;
	walkTree( KeyList &out, const KeyType &prefix ): m_out( out ), m_prefix( prefix ) {}
	walkTree( KeyList &out ): m_out( out ) {}
	void operator()( const_reference ref ) const {
		if ( ref.second.is_leaf() ) {
			if ( Predicate()( ref ) )
				m_out.insert( m_out.end(), ( m_prefix != "" ? m_prefix + "/" : "" ) + ref.first );
		} else {
			const PropertyMap &sub = ref.second.getBranch();
			std::for_each( sub.begin(), sub.end(), walkTree<Predicate>( m_out, ref.first ) );
		}
	}
};

// as well as PropertyMap::getProperty ...
template<typename T> T PropertyMap::getPropertyAs( const KeyType &key ) const
{
	const mapped_type *entry = findEntry( util::istring( key ) );

	if( entry ) {
		const PropertyValue &ref = entry->getLeaf()[0];

		if( !ref.isEmpty() )
			return ref->as<T>();
	}

	LOG( Debug, warning ) << "Returning " << Value<T>().toString( true ) << " because property " << key << " does not exist";
	return T();
}

}
}

// and finally define the streaming output for treeNode
namespace std
{
/// Streaming output for PropertyMap::node
template<typename charT, typename traits>
basic_ostream<charT, traits>& operator<<( basic_ostream<charT, traits> &out, const isis::util::_internal::treeNode &s )
{
	if( s.is_leaf() ){
		vector< isis::util::PropertyValue > vec=s.getLeaf();
		isis::util::listToOStream(vec.begin(),vec.end(),out);
	} else
		out << "[[Subtree with " << s.getBranch().getKeys().size() << " elements]]";

	return out;
}
/// Streaming output for PropertyMap
template<typename charT, typename traits>
basic_ostream<charT, traits>& operator<<( basic_ostream<charT, traits> &out, const isis::util::PropertyMap &s )
{
	isis::util::PropertyMap::FlatMap buff = s.getFlatMap();
	isis::util::listToOStream( buff.begin(), buff.end(), out );
	return out;
}
}

#endif
