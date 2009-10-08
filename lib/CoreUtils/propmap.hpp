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
	bool valid()const;
	key_list missing()const;
	diff_map diff(const PropMap &second,key_list ignore=key_list())const;
	void make_unique(const PropMap &second,key_list ignore=key_list());
	std::ostream& print(std::ostream &out,bool label=false);
};

}
/** @} */
}

#endif
