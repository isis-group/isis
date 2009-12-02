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


PropMap::diff_map PropMap::diff(const PropMap& second,key_list ignore) const{
	PropMap::diff_map ret;

	//insert everything that is in this, but not in second or is on both but differs
 	BOOST_FOREACH(const_reference ref,*this){
		if(ignore.find(ref.first)!=ignore.end()) // if its in the ignore list, skip it
			continue;
		const_iterator found=second.find(ref.first);
		if(found == second.end())// if its not in second 
			ret.insert(std::make_pair( // add (propertyname|(value1|[empty]))
				ref.first,
				std::make_pair(ref.second,PropertyValue())
			));
		else { //if it is in second as well
			const PropertyValue &first=found->second,&second = ref.second;
			if(!(first.empty() || second.empty() || first==second)) // if they are not empty, but not equal
				ret.insert(std::make_pair( // add (propertyname|(value1|value2))
					ref.first,
					std::make_pair(ref.second,found->second)
				));
		}
	}
	//insert everything that is in second but not in this
 	BOOST_FOREACH(const_reference ref,second){
		const_iterator found=find(ref.first);
		if(found == end()) // if its not in ref
			ret.insert(std::make_pair( // add (propertyname|([empty]|value2))
				ref.first,
				std::make_pair(PropertyValue(),ref.second)
			));
	}
	return ret;
}

void PropMap::make_unique (const util::PropMap& second, PropMap::key_list ignore ) {
	//remove everything that is also in second and equal
	BOOST_FOREACH(const_reference ref,second){
		if(ignore.find(ref.first)==ignore.end())
			continue;
		iterator found=find(ref.first);
		if(found != second.end() && found->second.operator==(ref.second))
			erase(found);
	}
}

void PropMap::join(const isis::util::PropMap& second, bool overwrite, PropMap::key_list ignore) {
	MAKE_LOG(CoreDebug);
	LOG(CoreDebug,info)
		<< "Comparing " << list2string(this->begin(),this->end()) << " and "
		<< list2string(second.begin(),second.end()) << std::endl;
	BOOST_FOREACH(const_reference ref,second){
		if(ignore.find(ref.first)!=ignore.end()){//skip any prop from the ignore-list
			LOG(CoreDebug,verbose_info) << "Ignoring " << ref << std::endl;
			continue;
		}
		iterator found=find(ref.first);
		if(found != this->end()){ // if its allready here
			if(found->second.empty() || overwrite){
				LOG(CoreDebug,verbose_info) << "Replacing " << found->first << " by " << ref.second << std::endl;
				found->second=ref.second;
			}
		} else {
			LOG(CoreDebug,verbose_info) << "Inserting " << ref << std::endl;
			insert(ref);
		}
	}
}


PropMap::key_list PropMap::missing() const{
	PropMap::key_list ret;
 	BOOST_FOREACH(const_reference ref,*this){
		if(ref.second.needed() && ref.second.empty())
			ret.insert(ref.first);
	}
	return ret;
}


std::ostream& PropMap::print ( std::ostream& out,bool label ) {
	BOOST_FOREACH(const_reference ref,*this)
		out << ref.first << ": " << ref.second->toString(label) << std::endl;
	return out;
}


}}