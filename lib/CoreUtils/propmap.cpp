//
// C++ Implementation: propmap
//
// Description:
//
//
// Author:  <Enrico Reimer>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "propmap.hpp"
#include <boost/foreach.hpp>

namespace isis{ namespace util{ 

bool _internal::nocase_less::operator() (const std::string& a, const std::string& b) const	{
	return (strcasecmp (a.c_str ( ), b.c_str ( )) < 0);
}

bool PropMap::valid() const {
	//iterate through the whole map and return false as soon as we find something needed _and_ empty
 	BOOST_FOREACH(const_reference ref,*this)
		if(ref.second.needed() && ref.second.empty())
			return false;
	return true;
}


PropMap::string_map PropMap::diff(const PropMap& second) const{
	//iterate through the whole map and return false as soon as we find something needed _and_ empty
	PropMap::string_map ret;

	//insert everything that is in this but not in second or is on both but differs
 	BOOST_FOREACH(const_reference ref,*this){
		std::string str=">" + ref.second->toString(true)+"<>";
		const_iterator found=second.find(ref.first);
		if(found == second.end())
			ret.insert(std::make_pair(ref.first,str+"<"));
		else if(!found->second.operator==(ref.second))
			ret.insert(std::make_pair(ref.first,str+found->second->toString(true)+"<"));
	}
	//insert everything that is in second but not this
 	BOOST_FOREACH(const_reference ref,second){
		const std::string str="><>"+ref.second->toString(true)+"<";
		const_iterator found=find(ref.first);
		if(found == end())
			ret.insert(std::make_pair(ref.first,str));
	}
	return ret;
}

PropMap::key_list PropMap::missing() const{
	PropMap::key_list ret;
 	BOOST_FOREACH(const_reference ref,*this){
		if(ref.second.needed() && ref.second.empty())
			ret.push_back(ref.first);
	}
	return ret;
}

}}