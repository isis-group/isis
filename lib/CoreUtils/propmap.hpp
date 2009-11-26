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

namespace isis{ 
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util{

/// @cond _hidden
namespace _internal {
struct nocase_less{
	bool operator() (const std::string& a, const std::string& b) const;
};
}
/// @endcond
	
class PropMap : public std::map<std::string,PropertyValue,_internal::nocase_less>{
public:
	typedef std::set<key_type,_internal::nocase_less> key_list;
	typedef std::map<key_type,std::pair<mapped_type,mapped_type>,_internal::nocase_less> diff_map;
	/**
	* Check if every needed property is set.
	* \returns false if there is any needed and empty property, true otherwhise.
	*/
	bool valid()const;
	/**
	* Get a list of missing properties.
	* \returns a list of all needed and empty properties.
	*/
	key_list missing()const;
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
	void join(const PropMap &second,bool overwrite=false,key_list ignore=key_list());
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
