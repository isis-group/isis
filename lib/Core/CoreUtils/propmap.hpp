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
class treeNode; //predeclare treeNode -- we'll need it in PropMap
}
/// A mapping tree to store properties (keys / values)
class PropMap : protected std::map<util::istring, _internal::treeNode>
{
public:
	typedef std::map<key_type, mapped_type, key_compare> base_type;
	typedef std::set<key_type, key_compare> key_list;
	typedef std::map<key_type, std::pair<PropertyValue, PropertyValue>, key_compare> diff_map;
	typedef std::map<key_type, PropertyValue> flat_map;
	typedef key_type pname_type;
private:
	typedef std::list<key_type> propPath;
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
	///Walks the whole tree and inserts any key into out for which the given scalar predicate is true.
	template<class Predicate> struct walkTree;

	/////////////////////////////////////////////////////////////////////////////////////////
	// internal tool-backends
	/////////////////////////////////////////////////////////////////////////////////////////
	/// internal recursion-function for join
	void joinTree( const isis::util::PropMap &other, bool overwrite, key_type prefix, PropMap::key_list &rejects );
	/// internal recursion-function for diff
	void diffTree( const PropMap &other, PropMap::diff_map &ret, key_type prefix ) const;

	static mapped_type &fetchEntry( util::PropMap &root, const propPathIterator at, const propPathIterator pathEnd );
	mapped_type &fetchEntry( const key_type &key );

	static const mapped_type *findEntry( const util::PropMap &root, const propPathIterator at, const propPathIterator pathEnd );
	const mapped_type *findEntry( const key_type &key )const;

	/// internal recursion-function for remove
	bool recursiveRemove( util::PropMap &root, const propPathIterator at, const propPathIterator pathEnd );
protected:
	/////////////////////////////////////////////////////////////////////////////////////////
	// rw-backends
	/////////////////////////////////////////////////////////////////////////////////////////
	/// create a list of keys for every entry for which the given scalar predicate is true.
	template<class Predicate> const key_list genKeyList()const {
		key_list k;
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
	* Adds a property as needed.
	* If the given property allready exists, it is just flagged as needed.
	*/
	void addNeeded( const key_type &key );
public:
	/////////////////////////////////////////////////////////////////////////////////////////
	// constructors
	/////////////////////////////////////////////////////////////////////////////////////////
	PropMap( const base_type &src );
	PropMap();

	bool operator==( const PropMap &src )const;

	/////////////////////////////////////////////////////////////////////////////////////////
	// Common rw-accessors
	/////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Access the property referenced by the path-key.
	 * If the property does not exist, an empty dummy will returned.
	 * \param key the "path" to the property
	 * \returns a reference to the PropertyValue
	 */
	const PropertyValue &propertyValue( const key_type &key )const;
	/**
	 * Access the property referenced by the path-key, create it if its not there.
	 * \param key the "path" to the property
	 * \returns a reference to the PropertyValue
	 */
	PropertyValue &propertyValue( const key_type &key );
	/**
	 * Access the branch referenced by the path-key, create it if its not there.
	 * \param key the "path" to the branch
	 * \returns a reference to the branching PropMap
	 */
	PropMap &branch( const key_type &key );
	/**
	 * Access the branch referenced by the path-key.
	 * If the branch does not exist, an empty dummy will returned.
	 * \param key the "path" to the branch
	 * \returns a reference to the branching PropMap
	 */
	const PropMap &branch( const key_type &key )const;
	/// remove the property/branch adressed by the key
	bool remove( const key_type &key );
	/// remove every property which is also in the given map (regardless of the value)
	bool remove( const isis::util::PropMap &removeMap, bool keep_needed = false );
	/// \returns true is the given property does exist and is not empty.
	bool hasProperty( const key_type &key )const;
	/// \returns true is the given branch does exist and is not empty.
	bool hasBranch( const key_type &key )const;

	////////////////////////////////////////////////////////////////////////////////////////
	// tools
	/////////////////////////////////////////////////////////////////////////////////////////
	/**
	* Check if every needed property is set.
	* \returns false if there is any needed and empty property, true otherwhise.
	*/
	bool valid()const;
	/// \returns true if the PropMap is empty, false otherwhise
	bool empty()const;
	/// get a flat list of the "paths" to all properties in the PropMap
	const key_list getKeys()const;
	/**
	 * Get a list of missing properties.
	 * \returns a list of all needed and empty properties.
	 */
	const key_list getMissing()const;
	/**
	 * Get a difference map of this and the given PropMap.
	 * Creates a map out of the Name of differencing properties and their difference, which is a std::pair\<PropertyValue,PropertyValue\>.
	 * - If a Property is empty in this but set in second,
	 *   it will be added with the first PropertyValue emtpy and the second PropertyValue
	 *   taken from second
	 * - If a Property is set in this but empty in second,
	 *   it will be added with the first PropertyValue taken from this and the second PropertyValue empty
	 * - If a Property is set in both, but not equal, it will be added with the first PropertyValue taken from this
	 *   and the second PropertyValue taken from second
	 * - If a Property is set in both and equal, it wont be added
	 * - If a Property is empty in both, it wont be added
	 * \param second the "other" PropMap to compare with
	 * \return a map of property keys and value-pairs
	 */
	diff_map getDifference( const PropMap &second )const;
	/// Remove everything that is also in second and equal.
	void make_unique( const isis::util::PropMap &other, bool removeNeeded = false );
	/**
	* Add Properties from another PropMap.
	* \param other the other PropMap
	* \param overwrite if existing properties shall be replaced
	*/
	PropMap::key_list join( const isis::util::PropMap &other, bool overwrite = false );

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
	void toCommonUnique( PropMap &common, std::set<key_type> &uniques, bool init )const;

	///copy the tree into a flat key/property-map
	void linearize( flat_map &out, key_type key_prefix = "" )const;

	/**
	 * Transform an existing property into another.
	 * Converts the value of the given property into the requested type and stores it with the given new key.
	 * \param from the key of the property to be transformed
	 * \param to the key for the new property
	 * \param dstId the type-id of the new property value
	 * \param delSource if the original property shall be deleted after the tramsformation was done
	 * \returns true if the transformation was done
	 */
	bool transform( key_type from, key_type to, int dstId, bool delSource = true );

	/**
	* Transform an existing property into another (statically typed version).
	* Converts the value of the given property into the requested type and stores it with the given new key.
	* A compile-time check is done to test if the requested type is available.
	* \param from the key of the property to be transformed
	* \param to the key for the new property
	* \param delSource if the original property shall be deleted after the tramsformation was done
	* \returns true if the transformation was done
	*/
	template<typename DST> bool transform( key_type from, key_type to, bool delSource = true ) {
		check_type<DST>();
		return transform( from, to, Type<DST>::staticID, delSource );
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// Additional get/set - Functions
	//////////////////////////////////////////////////////////////////////////////////////
	//@todo make sure the type specific behaviour is as documented
	/**
	 * Set the given property to a given value/type.
	 * The needed flag (if set) will be kept.
	 * If the property is already set to a value of another type an error is send to Runtime an nothing will be set.
	 */
	template<typename T> PropertyValue &setProperty( const key_type &key, const T &val ) {
		PropertyValue &ret = propertyValue( key );

		if( ret.empty() ) {
			const bool needed = ret.needed();
			( ret = val ).needed() = needed;
		} else if( ret->is<T>() ) {
			ret->cast_to<T>() = val;
		} else { // don't overwrite allready set properties with a different type
			LOG( Runtime, error ) << "Property " << MSubject( key ) << " is allready set to " << MSubject( ret.toString( true ) ) << " won't override with " << MSubject( Type<T>( val ).toString( true ) );
		}

		return ret;
	}
	/**
	 * Request a property via the given key in the given type.
	 * If the requested type is not equal to type the property is stored with, an automatic conversion is done.
	 * If the property is not set yet T() is returned.
	 */
	template<typename T> T getProperty( const key_type &key )const;
	/**
	 * Rename a given property/branch.
	 * This is implemented as copy+delete and can also be used between branches.
	 * - if the target exist a warning will be send, but it will still be overwritten
	 * - if the source does not exist a warning will be send and nothing is done
	 * \returns true if renaming/moving was successful
	 */
	bool rename( key_type oldname, key_type newname );

	/**
	 * "Print" the PropMap.
	 * Will send the name and the result of PropertyValue->toString(label) to the given ostream.
	 * Is equivalent to common streaming operation but has the option to print the type of the printed properties.
	 * \param out the output stream to use
	 * \param label print the type of the property (see Type::toString())
	 */
	std::ostream &print( std::ostream &out, bool label = false )const;
};
}
/** @} */
}

namespace std ///predeclare streaming output -- we'll need it in treeNode
{
/// Streaming output for PropMap::node
template<typename charT, typename traits>
basic_ostream<charT, traits>& operator<<( basic_ostream<charT, traits> &out, const isis::util::_internal::treeNode &s );
/// Streaming output for PropMap
template<typename charT, typename traits>
basic_ostream<charT, traits>& operator<<( basic_ostream<charT, traits> &out, const isis::util::PropMap &s );
}


// OK, lets define treeNode
namespace isis
{
namespace util
{
namespace _internal
{
class treeNode
{
	PropMap m_branch;
	PropertyValue m_leaf;
public:
	bool empty()const {
		return m_branch.empty() && m_leaf.empty();
	}
	bool is_leaf()const {
		LOG_IF( ! ( m_branch.empty() || m_leaf.empty() ), Debug, error ) << "There is a non empty leaf at a branch. This should not be.";
		return m_branch.empty();
	}
	const PropMap &getBranch()const {
		return m_branch;
	}
	PropMap &getBranch() {
		return m_branch;
	}
	PropertyValue &getLeaf() {
		assert( is_leaf() );
		return m_leaf;
	}
	const PropertyValue &getLeaf()const {
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

// and now we can define walkTree (needs treeNode to be defined)
template<class Predicate> struct PropMap::walkTree {
	key_list &m_out;
	const key_type m_prefix;
	walkTree( key_list &out, const key_type &prefix ): m_out( out ), m_prefix( prefix ) {}
	walkTree( key_list &out ): m_out( out ) {}
	void operator()( const_reference ref ) const {
		if ( ref.second.is_leaf() ) {
			if ( Predicate()( ref ) )
				m_out.insert( m_out.end(), ( m_prefix != "" ? m_prefix + "/" : "" ) + ref.first );
		} else {
			const PropMap &sub = ref.second.getBranch();
			std::for_each( sub.begin(), sub.end(), walkTree<Predicate>( m_out, ref.first ) );
		}
	}
};

// as well as PropMap::getProperty ...
template<typename T> T PropMap::getProperty( const key_type &key ) const
{
	const mapped_type *entry = findEntry( util::istring( key ) );

	if( entry ) {
		const PropertyValue &ref = entry->getLeaf();

		if( !ref.empty() )
			return ref->as<T>();
	}

	LOG( Debug, warning ) << "Returning " << Type<T>().toString( true ) << " because property " << key << " does not exist";
	return T();
}

}
}

// and finally define the streaming output for treeNode
namespace std
{
/// Streaming output for PropMap::node
template<typename charT, typename traits>
basic_ostream<charT, traits>& operator<<( basic_ostream<charT, traits> &out, const isis::util::_internal::treeNode &s )
{
	if( s.is_leaf() )
		out << s.getLeaf();
	else
		out << "[[Subtree with " << s.getBranch().getKeys().size() << " elements]]";

	return out;
}
/// Streaming output for PropMap
template<typename charT, typename traits>
basic_ostream<charT, traits>& operator<<( basic_ostream<charT, traits> &out, const isis::util::PropMap &s )
{
	isis::util::PropMap::flat_map buff;
	s.linearize( buff );
	isis::util::write_list( buff.begin(), buff.end(), out );
	return out;
}
}

#endif
