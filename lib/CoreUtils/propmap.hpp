#ifndef ISISPROPMAP_HPP
#define ISISPROPMAP_HPP

#include <map>
#include <string>

#include "property.hpp"

namespace isis {

class PropMap : public std::map<std::string,Property>{};
template<> TypeBase &Type<PropMap>::fromString(std::string val);

}

//make PropMap printable
namespace boost{namespace detail{
	template<> bool lexical_stream<std::string, isis::PropMap, std::ostringstream::traits_type>::operator<<(const isis::PropMap& s);
}}

#endif
