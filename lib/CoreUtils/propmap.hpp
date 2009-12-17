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

class PropMap : public std::map<std::string,PropertyValue,caselessStringLess>{
public:
	typedef std::set<key_type,caselessStringLess> key_list;
	typedef std::map<key_type,std::pair<mapped_type,mapped_type>,caselessStringLess> diff_map;
private:
	struct trueP{
		bool operator()(const_reference ref)const{return true;}
	};
	struct empty{
		bool operator()(const_reference ref)const{return ref.second.empty();}
	};
	struct needed{
		bool operator()(const_reference ref)const{return ref.second.needed();}
	};
	struct invalidP{
		bool operator()(const_reference ref)const{return ref.second.needed() && ref.second.empty();}
	};
	struct validP{
		bool operator()(const_reference ref)const{return not invalidP().operator()(ref);}
	};
	template<class Predicate> struct insertKey{
		key_list &out;
		const Predicate pred;
		insertKey(key_list &_out):out(_out),pred(){}
		void operator()(const_reference ref){
			if(pred(ref))
				out.insert(out.end(),ref.first);
		};
	};
protected:
	template<class Predicate> const key_list genKeyList()const{
		key_list k;
		std::for_each(begin(),end(),insertKey<Predicate>(k));
		return k;
	}
public:
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
	 * \param ignore a list of properties to ignore when generating the difference
	 * \return a map of property names and value-pairs
	 */
	diff_map diff(const PropMap &second,key_list ignore=key_list())const;
	/// Remove everything that is also in second and equal.
	void make_unique(const PropMap &second,key_list ignore=key_list());
	/// Add Properties from another PropMap
	PropMap::key_list join(const isis::util::PropMap& other, bool overwrite = false, isis::util::PropMap::key_list ignore = key_list());
	/**
	 * "Print" the PropMap.
	 * Will send the name and the result of PropertyValue->toString(label) to the given ostream.
	 * Is equivalent to common streaming operation but has the option to print the type of the printed properties.
	 * \param out the output stream to use
	 * \param label print the type of the property (see Type::toString())
	 */
	std::ostream& print(std::ostream &out,bool label=false);
};

}
/** @} */
}

#endif
