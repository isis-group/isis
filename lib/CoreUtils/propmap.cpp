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
	template<> bool lexical_stream<std::string, iUtil::PropMap, std::ostringstream::traits_type>::operator<<(const iUtil::PropMap& s){
		iUtil::PropMap::const_iterator i=s.begin();
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

