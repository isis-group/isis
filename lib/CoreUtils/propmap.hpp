#ifndef ISISPROPMAP_HPP
#define ISISPROPMAP_HPP

#include <map>
#include <string>
#include <ostream>

#include "property.hpp"

namespace isis{ 
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util{

class PropMap : public std::map<std::string,PropertyValue>{};

}
/** @} */
}

//make PropMap printable
namespace std {
template<typename charT, typename traits> basic_ostream<charT, traits>& 
operator<<(basic_ostream<charT, traits> &out,const isis::util::PropMap& s){
	isis::util::PropMap::const_iterator i=s.begin();
	if(i!=s.end()){
		out << i->first << ": " << i->second->toString(true);
		for(i++;i!=s.end();i++){
			out << std::endl << i->first << ": " << i->second->toString(true);
		}
	}
	return out;
}
}

#endif
