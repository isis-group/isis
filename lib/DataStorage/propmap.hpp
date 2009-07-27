#ifndef ISISPROPMAP_HPP
#define ISISPROPMAP_HPP

#include <map>
#include <string>

#include "property.hpp"

namespace isis {

class PropMap : public std::map<std::string,Property>{
};

}

namespace boost{
namespace detail{
	template<> bool lexical_stream<std::string, isis::PropMap>::operator<<(const isis::PropMap& s);
}}

/*
std::ostream& operator << (std::ostream& os, const isis::PropMap& s)
{
	for(isis::PropMap::const_iterator i=s.begin();i!=s.end();i++){
		os << i->first << ": " << i->second->toString(true) << std::endl;
	}
    return os;
}
*/
#endif
