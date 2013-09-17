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
namespace util
{
/// @cond _internal
namespace _internal
{
class treeNode; //predeclare treeNode -- we'll need it in PropertyMap
}
/// @endcond _internal
/**
 * This class forms a mapping tree to store all kinds of properties (path : value), where:
 * - value is:
 *   - a util::PropertyValue-container holding the value of the property (this may be empty/unset)
 *   - another PropertyMap containing a subtree (a branch of the mapping tree)
 * - path is one or more case insensitive key names separated by "/" to locate both, branches or properties, in the tree
 *
 * Nevertheless there are separate access functions for branches and properties.
 * Trying to access a branch as a property value,or to access a property value as a branch will cause error messages and give empty results.
 *
 * Paths can be created from other paths and from strings (c-strings and util::istring, but not std::string).
 * So both can be used for functions which expect paths, but the usage of c-strings is slower.
 *
 * To describe the minimum of needed metadata needed by specific data structures / subclasses
 * properties can be marked as "needed" and there are functions to verify that they are not empty.
 */
class PropertyMap : protected std::map<util::istring, _internal::treeNode>
{
public:
	/// type of the keys forming a path
	typedef key_type KeyType;
	///a list to store keys only (without the corresponding values)
	typedef std::set<KeyType, key_compare> KeyList;
	///a flat map matching keys to pairs of values
	typedef std::map<KeyType, std::pair<PropertyValue, PropertyValue>, key_compare> DiffMap;
	///a flat map, matching complete paths as keys to the corresponding values
	typedef std::map<KeyType, PropertyValue> FlatMap;

	/// "Path" type used to locate entries in the tree
	struct PropPath: public std::list<KeyType> {
		PropPath();
		PropPath( const char *key );
		PropPath( const KeyType &key );
		PropPath( const std::list<KeyType> &path );
        PropPath operator/(const PropPath &s)const;
        PropPath& operator/=(const PropPath &s);
        PropPath operator/(KeyType s)const;
        PropPath& operator/=(KeyType s);
	};
private:
	typedef std::map<KeyType, mapped_type, key_compare> Container;
	typedef PropPath::const_iterator propPathIterator;

	static const char pathSeperator = '/';

	/////////////////////////////////////////////////////////////////////////////////////////
	// internal predicats
	/////////////////////////////////////////////////////////////////////////////////////////
	/// allways true
	struct trueP {  bool operator()( const_reference ref )const;};
	/// true when entry is a leaf, needed and empty
	struct invalidP {   bool operator()( const_reference ref )const;};
	/// true when entry is a leaf, needed and empty of entry is a invalid branch
	struct treeInvalidP {   bool operator()( const_reference ref )const;};
	/// true when entry is not a scalar
	struct listP {  bool operator()( const_reference ref )const;};

	/////////////////////////////////////////////////////////////////////////////////////////
	// internal functors
	/////////////////////////////////////////////////////////////////////////////////////////
	template<class Predicate> struct walkTree;

	/////////////////////////////////////////////////////////////////////////////////////////
	// internal tool-backends
	/////////////////////////////////////////////////////////////////////////////////////////
	/// internal recursion-function for join
	void joinTree( const PropertyMap &other, bool overwrite, KeyType prefix, KeyList &rejects );
	/// internal recursion-function for diff
	void diffTree( const PropertyMap &other, PropertyMap::DiffMap &ret, KeyType prefix ) const;

	static mapped_type &fetchEntry( PropertyMap &root, const propPathIterator at, const propPathIterator pathEnd );
	mapped_type &fetchEntry( const PropPath &path );

	static const mapped_type *findEntry( const util::PropertyMap &root, const propPathIterator at, const propPathIterator pathEnd );
	const mapped_type *findEntry( const PropPath &path )const;

	/// internal recursion-function for remove
	bool recursiveRemove( PropertyMap &root, const propPathIterator at, const propPathIterator pathEnd );

protected:
	template<typename T> class NeededsList: public std::list<PropPath>
	{
	public:
		NeededsList() {
			const std::list< PropertyMap::KeyType > buff = util::stringToList<PropertyMap::KeyType>( T::neededProperties ); //@todo really bad voodoo
			assign( buff.begin(), buff.end() );
		}
		void applyTo( PropertyMap &props ) {
			BOOST_FOREACH( const PropPath & ref, *this ) {
				props.addNeeded( ref );
			}
		}
	};
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
	 * Adds a property with status needed.
	 * \param path identifies the property to be added or if already existsing to be flagged as needed
	 */
	void addNeeded( const PropPath &path );

	/**
	 * Remove every PropertyValue which is also in the other PropertyMap and where operator== returns true.
	 * \param other the other property tree to compare to
	 * \param removeNeeded if a property should also be deleted it is needed
	 */
	void removeEqual( const PropertyMap &other, bool removeNeeded = false );

	/**
	 * Get common and unique properties from the tree.
	 * For every entry of the tree this checks if it is common/unique and removes/adds it accordingly.
	 * This is done by:
	 * - generating a difference (using diff) between the current common and the tree
	 *   - the resulting diff_map contains all newly unique properties (properties which has been in common, but are not euqual in the tree)
	 * - these newly diffent properties are removed from common and added to unique.
	 * - if init is true uniques is cleared and common is replaced by a copy of the tree (shall be done at first step)
	 * \param common reference of the common-tree
	 * \param uniques reference of the unique-tree
	 * \param init if initialisation shall be done instead of normal seperation
	 */
	void toCommonUnique( PropertyMap &common, std::set<KeyType> &uniques, bool init )const;

	///copy the tree into a flat key/property-map
	void makeFlatMap( FlatMap &out, KeyType key_prefix = "" )const;

	/**
	 * Access the property vector referenced by the path.
	 * If the property does not exist, an empty dummy will be returned.
	 * \param path the path to the property
	 * \returns a reference to the PropertyValue
	 */
	const std::vector<PropertyValue> &propertyValueVec( const PropPath &path )const;

	/**
	 * Access the property vector referenced by the path-key, create it if its not there.
	 * \param path the path to the property
	 * \returns a reference to the PropertyValue
	 */
	std::vector<PropertyValue> &propertyValueVec( const PropPath &path );
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
	 * Access the property referenced by the path.
	 * If the property does not exist, an empty dummy will returned.
	 * \param path the path to the property
	 * \returns a reference to the PropertyValue
	 */
	const PropertyValue &propertyValue( const PropPath &path )const;

	/**
	 * Access the property referenced by the path, create it if its not there.
	 * \param path the path to the property
	 * \returns a reference to the PropertyValue
	 */
	PropertyValue &propertyValue( const PropPath &path );

	/**
	 * Access the branch referenced by the path, create it if its not there.
	 * \param path the path to the branch
	 * \returns a reference to the branching PropertyMap
	 */
	PropertyMap &branch( const PropPath &path );

	/**
	 * Access the branch referenced by the path.
	 * If the branch does not exist, an empty dummy will returned.
	 * \param path the path to the branch
	 * \returns a reference to the branching PropertyMap
	 */
	const PropertyMap &branch( const PropPath &path )const;

	/**
	 * Remove the property adressed by the path.
	 * This actually only removes properties.
	 * Non-empty branches are not deleted.
	 * And if an branch becomes empty after deletion of its last entry, it is deleted automatically.
	 * \param path the path to the property
	 * \returns true if successful, false otherwise
	 */
	bool remove( const PropPath &path );

	/**
	 * remove every property which is also in the given tree (regardless of the value)
	 * \param removeMap the tree of properties to be removed
	 * \param keep_needed when true needed properties are kept even if they would be removed otherwise
	 * \returns true if all properties removed succesfully, false otherwise
	 */
	bool remove( const PropertyMap &removeMap, bool keep_needed = false );

	/**
	 * remove every property which is also in the given list (regardless of the value)
	 * \param removeList a list of paths naming the properties to be removed
	 * \param keep_needed when true needed properties are kept even if they would be removed otherwise
	 * \returns true if all properties removed succesfully, false otherwise
	 */
	bool remove( const KeyList &removeList, bool keep_needed = false );

	/**
	 * check if a property is available
	 * \param path the path to the property
	 * \returns true if the given property does exist and is not empty, false otherwise
	 */
	bool hasProperty( const PropPath &path )const;

	/**
	 * Search for a property/branch in the whole Tree.
	 * \param key the single key for the branch/property to search for (paths will be stripped to the rightmost key)
	 * \param allowProperty if false the search will ignore properties
	 * \param allowBranch if false the search will ignore branches (it will still search into the branches, but the branches themself won't be considered a valid finding)
	 * \returns full path to the property (including the properties name) if it is found, empty string elsewhise
	 */
	KeyType find( KeyType key, bool allowProperty = true, bool allowBranch = false )const;

	/// find all non scalar entries
	KeyList findLists()const;

	/**
	 * check if branch of the tree is available
	 * \param path the path to the branch
	 * \returns true if the given branch does exist and is not empty, false otherwise
	 */
	bool hasBranch( const PropPath &path )const;

	////////////////////////////////////////////////////////////////////////////////////////
	// tools
	/////////////////////////////////////////////////////////////////////////////////////////
	/**
	* Check if every property marked as needed is set.
	* \returns true if ALL needed properties are NOT empty, false otherwhise.
	*/
	bool isValid()const;

	/// \returns true if the PropertyMap is empty, false otherwhise
	bool isEmpty()const;

	/**
	 * Get a list of the paths of all properties.
	 * \returns a flat list of the paths to all properties in the PropertyMap
	 */
	const KeyList getKeys()const;

	/**
	 * Get a list of missing properties.
	 * \returns a list of the paths for all properties which are marked as needed and but are empty.
	 */
	const KeyList getMissing()const;

	/**
	 * Get a difference map of this tree and another.
	 * Out of the names of differing properties a mapping from paths to std::pair\<PropertyValue,PropertyValue\> is created with following rules:
	 * - if a Property is empty in this tree but set in second,
	 *   - it will be added with the first PropertyValue emtpy and the second PropertyValue
	 *   taken from second
	 * - if a Property is set in this tree but empty in second,
	 *   - it will be added with the first PropertyValue taken from this and the second PropertyValue empty
	 * - if a Property is set in both, but not equal,
	 *   - it will be added with the first PropertyValue taken from this and the second PropertyValue taken from second
	 * - if a Property is set in both and equal, or is empty in both,
	 *   - it wont be added
	 * \param second the other tree to compare with
	 * \returns a map of property paths and pairs of the corresponding different values
	 */
	DiffMap getDifference( const PropertyMap &second )const;

	/**
	 * Add Properties from another tree.
	 * \param other the other tree to join with
	 * \param overwrite if existing properties shall be replaced
	 * \returns a list of the rejected properties that couldn't be inserted, for success this should be empty
	 */
	PropertyMap::KeyList join( const PropertyMap &other, bool overwrite = false );

	/**
	 * Transform an existing property into another.
	 * Converts the value of the given property into the requested type and stores it with the given new path.
	 * \param from the path of the property to be transformed
	 * \param to the path for the new property
	 * \param dstID the type-ID of the new property value
	 * \param delSource if the original property shall be deleted after the tramsformation was done
	 * \returns true if the transformation was done, false it failed
	 */
	bool transform( const PropPath &from, const PropPath &to, int dstID, bool delSource = true );

	/**
	 * Transform an existing property into another (statically typed version).
	 * Converts the value of the given property into the requested type and stores it with the given new path.
	 * A compile-time check is done to test if the requested type is available.
	 * \param from the path of the property to be transformed
	 * \param to the path for the new property
	 * \param delSource if the original property shall be deleted after the tramsformation was done
	 * \returns true if the transformation was done, false it failed
	 */
	template<typename DST> bool transform( const PropPath &from, const PropPath &to, bool delSource = true ) {
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
	 * The property will be set to the one given value if
	 * - the property is empty or
	 * - the property is already set to one value of the same type (value will be overridden)
	 * - the property is already set to another value and the new value can be converted (value will be overridden but type will be kept)
	 * The property will not be set and an error will be send to Runtime if
	 * - the property is already set to one value of another type an and no conversion is available
	 * - the property is set to more that one value
	 * \code
	 * setPropertyAs("MyPropertyName", isis::util::fvector4(1,0,1,0))
	 * \endcode
	 * \param path the path to the property
	 * \param val the value to set of type T
	 * \returns a reference to the PropertyValue (this can be used later, e.g. if a vector is filled step by step
	 * the reference can be used to not ask for the Property each time)
	 */
	template<typename T> PropertyValue &setPropertyAs( const PropPath &path, const T &val ) {
		PropertyValue &ret = propertyValue( path );

		if( ret.isEmpty() ) { // set an empty property
			ret = val;
		} else if(size()==1){
			if( ret.is<T>() ) { // override same type
				ret.castTo<T>() = val;
			} else {
				const util::Value<T> vval(val);
				const unsigned short dstID=ret.getTypeID();
				if(vval.fitsInto(dstID)){// allow store if value is convertible into already stored type
					LOG(Debug,warning) << "Storing " << vval << " as " << ret.getTypeName() << " as old value was already stored in that type";
					ret=*(vval.copyByID(dstID));
				}else {
					LOG( Runtime, error ) << "Property " << MSubject( path ) << " is already set to " << MSubject( ret.toString( true ) ) << " won't override with " << MSubject( Value<T>( val ).toString( true ) );
				}
			}
			
		} else
			LOG( Runtime, error ) << "Won't override multivalue property " << MSubject( path ) << " with " << MSubject( Value<T>( val ).toString( true ) );

		return ret;
	}

	/**
	 * Request a property via the given key in the given type.
	 * If the requested type is not equal to type the property is stored with, an automatic conversion is done.
	 * If that conversion failes an error is send to Runtime.
	 * \code
	 * getPropertyAs<fvector4>( "MyPropertyName" );
	 * \endcode
	 * \param path the path to the property
	 * \returns the property with given type, if not set yet T() is returned.
	 */
	template<typename T> T getPropertyAs( const PropPath &path )const;

	/**
	 * Rename a given property/branch.
	 * This is implemented as copy+delete and can also be used between branches.
	 * - if the target exist a warning will be send, but it will still be overwritten
	 * - if the source does not exist a warning will be send and nothing is done
	 * \param from the path of the existing property to be moved
	 * \param to the destinaton path of the move
	 * \returns true if renaming/moving was successful
	 */
	bool rename( const PropPath &from, const PropPath &to );

	/// \returns a flat representation of the whole property tree
	FlatMap getFlatMap( )const;

	/**
	 * "Print" the property tree.
	 * Will send the name and the result of PropertyValue->toString(label) to the given ostream.
	 * Is equivalent to common streaming operation but has the option to print the type of the printed properties.
	 * \param out the output stream to use
	 * \param label print the type of the property (see Value::toString())
	 */
	std::ostream &print( std::ostream &out, bool label = false )const;
};
}
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
API_EXCLUDE_BEGIN
/// @cond _internal
namespace _internal
{
/**
 * Basic container class for the "values" inside the property tree.
 * This can hold a list of PropertyValues or another PropertyMap.
 */
class treeNode
{
	PropertyMap m_branch;
	std::vector<PropertyValue> m_leaf;
public:
	treeNode(): m_leaf( 1 ) {}
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
	void insert( const treeNode &ref ) {
		m_branch = ref.m_branch;
		m_leaf.resize( ref.m_leaf.size() );
		std::vector<PropertyValue>::iterator dst = m_leaf.begin();
		const bool needed = dst->isNeeded();
		BOOST_FOREACH( std::vector<PropertyValue>::const_reference src, ref.m_leaf ) {
			( *(dst++) = src ).needed() = needed;;
		}
	}
	std::string toString()const {
		std::ostringstream o;
		o << *this;
		return o.str();
	}
};
}
/// @endcond
API_EXCLUDE_END

////////////////////////////////////////////////////////////////////////////////////////
// and now we can define walkTree (needs treeNode to be defined)
////////////////////////////////////////////////////////////////////////////////////////

/// Walks the whole tree and inserts any key into out for which the given scalar predicate is true.
template<class Predicate> struct PropertyMap::walkTree {
	KeyList &m_out;
	const KeyType m_prefix;
	walkTree( KeyList &out, const KeyType &prefix ): m_out( out ), m_prefix( prefix ) {}
	walkTree( KeyList &out ): m_out( out ) {}
	void operator()( const_reference ref ) const {
		const KeyType name = ( m_prefix != "" ? m_prefix + "/" : "" ) + ref.first;

		if ( ref.second.is_leaf() ) {
			if ( Predicate()( ref ) )
				m_out.insert( m_out.end(), name );
		} else {
			const PropertyMap &sub = ref.second.getBranch();
			std::for_each( sub.begin(), sub.end(), walkTree<Predicate>( m_out, name ) );
		}
	}
};

// as well as PropertyMap::getProperty ...
template<typename T> T PropertyMap::getPropertyAs( const PropPath &path ) const
{
	const mapped_type *entry = findEntry( path );

	if( entry ) {
		const PropertyValue &ref = entry->getLeaf()[0];

		if( !ref.isEmpty() )
			return ref.as<T>();
	}

	LOG( Debug, warning ) << "Returning " << Value<T>().toString( true ) << " because property " << path << " does not exist";
	return T();
}

}
}

// and finally define the streaming output for treeNode
namespace std
{
/// Streaming output for PropertyMap::PropPath
template<typename charT, typename traits>
basic_ostream<charT, traits>& operator<<( basic_ostream<charT, traits> &out, const isis::util::PropertyMap::PropPath &s )
{
	isis::util::listToOStream( s.begin(), s.end(), out, "/", "", "" );
	return out;
}
/// Streaming output for PropertyMap::node
template<typename charT, typename traits>
basic_ostream<charT, traits>& operator<<( basic_ostream<charT, traits> &out, const isis::util::_internal::treeNode &s )
{
	if( s.is_leaf() ) {
		vector< isis::util::PropertyValue > vec = s.getLeaf();
		isis::util::listToOStream( vec.begin(), vec.end(), out );
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
