#ifndef ISISPROPMAP_HPP
#define ISISPROPMAP_HPP

#include <map>
#include <string>
#include <strings.h>

#include "common.hpp"
#include "property.hpp"

namespace isis{ 
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util{

/// @cond _internal
namespace _internal {
struct nocase_less{
	bool operator() (const std::string& a, const std::string& b) const;
};
}
/// @endcond
	
class PropMap : public std::map<std::string,PropertyValue,_internal::nocase_less>{
public:
	typedef std::list<key_type> key_list;
	typedef std::map<key_type,std::pair<mapped_type,mapped_type>,_internal::nocase_less> diff_map;
	bool valid()const;
	key_list missing()const;
	diff_map diff(const PropMap &second)const;
	std::ostream& print(std::ostream &out,bool label=false);
};

}
/** @} */
}

#endif
