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
#include <strings.h>

#include "common.hpp"
#include "property.hpp"
#include <set>
#include <algorithm>

namespace isis{ 
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util{

/// A mapping tree to store properties (keys / values)
class PropMap : protected std::map<std::string,PropertyValue,_internal::caselessStringLess>{
public:
	typedef std::map<std::string,PropertyValue,_internal::caselessStringLess> base_type;
	typedef std::set<std::string,_internal::caselessStringLess> key_list;
	typedef std::map<std::string,std::pair<PropertyValue,PropertyValue>,_internal::caselessStringLess> diff_map;
private:
	typedef std::list<std::string> propPath;
	typedef propPath::const_iterator propPathIterator;
	
	static const char pathSeperator = '/';
	static const util::PropertyValue emptyProp;//dummy to be able to return an empty Property

	/////////////////////////////////////////////////////////////////////////////////////////
	// internal functors
	/////////////////////////////////////////////////////////////////////////////////////////
	struct trueP{
		bool operator()(const_reference ref)const{return true;}
	};
	struct emptyP{
		bool operator()(const_reference ref)const{return ref.second.empty();}
	};
	struct neededP{
		bool operator()(const_reference ref)const{return ref.second.needed();}
	};
	struct invalidP{
		bool operator()(const_reference ref)const{return ref.second.needed() && ref.second.empty();}
	};
	struct treeInvalidP{
		bool operator()(const_reference ref)const{
			if(ref.second.empty()){
				return ref.second.needed();
			} else if(ref.second->is<PropMap>()){
				const PropMap &sub=ref.second->cast_to_Type<PropMap>();
				return not sub.valid();
			} else
				return false;
		}
	};
	///Walks the whole tree and inserts any key into out for which the given scalar predicate is true.
	template<class Predicate> struct walkTree{
		key_list &m_out;
		const std::string m_prefix;
		walkTree(key_list &out,const std::string &prefix):m_out(out),m_prefix(prefix){}
		walkTree(key_list &out):m_out(out){}
		void operator()(const_reference ref){
			if((not ref.second.empty()) and ref.second->is<PropMap>()){
				PropMap &sub=ref.second->cast_to_Type<PropMap>();
				std::for_each(sub.begin(),sub.end(),walkTree<Predicate>(m_out));
			} else {
				if(Predicate()(ref))
					m_out.insert(m_out.end(),(m_prefix!="" ? m_prefix+"/":"")+ref.first);
			}
		};
	};

	/////////////////////////////////////////////////////////////////////////////////////////
	// internal tool-backends
	/////////////////////////////////////////////////////////////////////////////////////////
	/// internal recursion-function for join
	void joinTree(const isis::util::PropMap& other, bool overwrite, std::string prefix, PropMap::key_list &rejects);
	/// internal recursion-function for diff
	void diffTree(const PropMap& other,PropMap::diff_map &ret,std::string prefix) const;
	/// internal helper for operator[]
	static PropertyValue& fetchProperty(util::PropMap &root,const propPathIterator at,const propPathIterator pathEnd);
	/// internal helper for findPropVal
	static const PropertyValue* searchBranch(const util::PropMap &root,const propPathIterator at,const propPathIterator pathEnd);
	/// internal recursion-function for remove
	bool recursiveRemove(util::PropMap &root,const propPathIterator at,const propPathIterator pathEnd);
protected:
	/////////////////////////////////////////////////////////////////////////////////////////
	// rw-backends
	/////////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Find the property referenced by the path-key.
	 * \param key the "path" to the property
	 * \returns a pointer to the PropertyValue, NULL if the property was not found
	 */
	const PropertyValue* findPropVal(const std::string &key)const;
	/// create a list of keys for every entry for which the given scalar predicate is true.
	template<class Predicate> const key_list genKeyList()const{
		key_list k;
		std::for_each(begin(),end(),walkTree<Predicate>(k));
		return k;
	}
	/**
	* Make Properties given by a space separated list needed.
	* \param needed string made of space serparated property-names which
	* will (if neccessary) be added to the PropertyMap and flagged as needed.
	*/
	void addNeededFromString(const std::string &needed);
	/// \returns true if a given property exists (also if its empty)
	bool exists(const std::string& key)const;
public:
	/////////////////////////////////////////////////////////////////////////////////////////
	// constructors
	/////////////////////////////////////////////////////////////////////////////////////////
	PropMap(const base_type &src);
	PropMap();

	bool operator==(const PropMap &src)const;

	/////////////////////////////////////////////////////////////////////////////////////////
	// Common rw-accessors
	/////////////////////////////////////////////////////////////////////////////////////////
	/**
	* Find the property referenced by the path-key, create it if its not there.
	* \param key the "path" to the property
	* \returns a reference to the PropertyValue
	*/
	PropertyValue& operator[](const std::string& key);
	bool remove(const std::string& key);
	/**
	* Adds a property as needed.
	* If the given property allready exists, it is just flagged as needed.
	*/
	void addNeeded(const std::string &key);
	/// \returns true is the given property does exist and is not empty.
	bool hasProperty(const std::string &key)const;

	////////////////////////////////////////////////////////////////////////////////////////
	// tools
	/////////////////////////////////////////////////////////////////////////////////////////
	/**
	* Check if every needed property is set.
	* \returns false if there is any needed and empty property, true otherwhise.
	*/
	bool valid()const;
	const key_list keys()const;
	/**
	* Get a list of missing properties.
	* \returns a list of all needed and empty properties.
	*/
	const key_list missing()const;
	/**
	 * Get a difference map of this and the given PropMap.
	 * Creates a map out of the Name of differencing properties and their difference, which is a std::pair\<PropertyValue,PropertyValue\>.
	 * - If a Property is empty in this but set in second,
	 *   it will be added with the first PropertyValue emtpy and the second PropertyValue
	 *	 taken from second
	 * - If a Property is set in this but empty in second,
	 *   it will be added with the first PropertyValue taken from this and the second PropertyValue empty
	 * - If a Property is set in both, but not equal, it will be added with the first PropertyValue taken from this
	 *   and the second PropertyValue taken from second
	 * - If a Property is set in both and equal, it wont be added
	 * - If a Property is empty in both, it wont be added
	 * \param second the "other" PropMap to compare with
	 * \return a map of property names and value-pairs
	 */
	diff_map diff(const PropMap &second)const;
	/// Remove everything that is also in second and equal.
	void make_unique(const isis::util::PropMap& other);
	/**
	* Add Properties from another PropMap.
	* \param other the other PropMap
	* \param overwrite if existing properties shall be replaced
	*/
	PropMap::key_list join(const isis::util::PropMap& other, bool overwrite = false);

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
	void toCommonUnique(PropMap& common,std::set<std::string> &uniques,bool init)const;

	///copy the tree into a flat key/property-map
	size_t linearize(base_type &out,std::string key_prefix="")const;

	/**
	 * Transform an existing property into another.
	 * Converts the value of the given property into the requested type and stores it with the given new key.
	 * \param from the key of the property to be transformed
	 * \param to the key for the new property
	 * \param dstId the type-id of the new property value
	 * \param delSource if the original property shall be deleted after the tramsformation was done
	 * \returns true if the transformation was done
	 */
	bool transform(std::string from, std::string to, int dstId, bool delSource = true);

	/**
	* Transform an existing property into another (statically typed version).
	* Converts the value of the given property into the requested type and stores it with the given new key.
	* A compile-time check is done to test if the requested type is available.
	* \param from the key of the property to be transformed
	* \param to the key for the new property
	* \param delSource if the original property shall be deleted after the tramsformation was done
	* \returns true if the transformation was done
	*/
	template<typename DST> bool transform(std::string from, std::string to, bool delSource = true)
	{
		check_type<DST>();
		transform(from,to,Type<DST>::staticId(),delSource);
	}
	
	//////////////////////////////////////////////////////////////////////////////////////
	// Additional get/set - Functions
	//////////////////////////////////////////////////////////////////////////////////////
	/**
	* Sets a given property to a given value.
	* If the property is allready set, it will be reset (but setting a different type will fail).
	* If the property does not exist it will be replaced.
	* \param key the name of the property to be set
	* \param val the value the property should be set to
	*/
	PropertyValue& setPropertyValue(const std::string &key,const PropertyValue &val);
	//@todo make shure the type specific behaviour is as documented
	template<typename T> T& setProperty(const std::string &key,const T &val){
		PropertyValue &ret= operator[](key)=val;
		return ret->cast_to_Type<T>();
	}
	/**
	* Get the given property.
	* \returns a reference of the stored PropertyValue
	*/
	const util::PropertyValue &getPropertyValue(const std::string &key)const;
	/**
	* Get the value of the given Property.
	* If DataLog is enabled and the stored type is not T an error will be send.
	* \returns a Type\<T\> containing a copy of the value stored for given property if the type of the stored property is T.
	* \return Type\<T\>() otherwhise.
	*/
	template<typename T> util::Type<T> getPropertyType(const std::string &key)const{
		const PropertyValue &value=getPropertyValue(key);
		if(value.empty()){
			const util::Type<T> dummy=T();
			LOG(CoreLog,util::error)
			<< "Requested Property " << key << " is not set! Returning " << dummy.toString(true);
			return dummy;
		}
		else
			return value->cast_to_Type<T>();
	}
	template<typename T> T getProperty(const std::string &key)const{
		return (T)getPropertyType<T>(key);
	}
	bool renameProperty(std::string oldname,std::string newname);
	
	/**
	 * "Print" the PropMap.
	 * Will send the name and the result of PropertyValue->toString(label) to the given ostream.
	 * Is equivalent to common streaming operation but has the option to print the type of the printed properties.
	 * \param out the output stream to use
	 * \param label print the type of the property (see Type::toString())
	 */
	std::ostream& print(std::ostream& out, bool label = false)const;
};

}
/** @} */
}

namespace std {
/// Streaming output for PropMap
template<typename charT, typename traits>
basic_ostream<charT, traits>& operator<<(basic_ostream<charT, traits> &out,const isis::util::PropMap& s)
{
	isis::util::PropMap::base_type buff;
	s.linearize(buff);
	isis::util::write_list(buff.begin(),buff.end(),out);
	return out;
}
}
#endif
