#ifndef ISISPROPMAP_HPP
#define ISISPROPMAP_HPP

#include <map>
#include <string>

#include "property.hpp"

namespace isis{ 
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util{

class PropMap : public std::map<std::string,Property>{};

}
/** @} */
}

//make PropMap printable
namespace boost{namespace detail{
	template<> bool lexical_stream<std::string, isis::util::PropMap, std::ostringstream::traits_type>::operator<<(const isis::util::PropMap& s);
}}

#endif
