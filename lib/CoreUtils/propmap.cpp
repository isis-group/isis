//
// C++ Implementation: propmap
//
// Description:
//
//
// Author:  <>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "propmap.hpp"

namespace boost{namespace detail{
	template<> bool lexical_stream<std::string, isis::util::PropMap, std::ostringstream::traits_type>::operator<<(const isis::util::PropMap& s){
		isis::util::PropMap::const_iterator i=s.begin();
		if(i==s.end())
			return false;
		else
			stream << i->first << ": " << i->second->toString(true);
		for(i++;i!=s.end();i++){
			stream << std::endl << i->first << ": " << i->second->toString(true);
		}
		return true;
	}
}}

