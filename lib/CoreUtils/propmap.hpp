#ifndef ISISPROPMAP_HPP
#define ISISPROPMAP_HPP

#include <map>
#include <string>

#include "property.hpp"

namespace iUtil {

class PropMap : public std::map<std::string,Property>{};

}

//make PropMap printable
namespace boost{namespace detail{
	template<> bool lexical_stream<std::string, iUtil::PropMap, std::ostringstream::traits_type>::operator<<(const iUtil::PropMap& s);
}}

#endif
