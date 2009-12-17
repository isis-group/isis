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
	
bool PropMap::valid() const
{
	//iterate through the whole map and return false as soon as we find something needed _and_ empty
	const const_iterator found=std::find_if(begin(),end(),invalidP());
	return found==end();
}


PropMap::diff_map PropMap::diff(const PropMap& other,key_list ignore) const{
	PropMap::diff_map ret;
	key_list::const_iterator ignoreIt= ignore.begin();
	const_iterator otherIt=other.begin();
	
	//insert everything that is in this, but not in second or is on both but differs
 	BOOST_FOREACH(const_reference ref,*this){
		//find the closest match for ref.first in ignore (use the comparison-functor of PropMap)
		if (continousFind(ignoreIt, ignore.end(),ref.first, key_comp()))
			continue; //skip if ref.first == *ignore
		
		const_iterator found=other.find(ref.first);
		if(found == other.end())// if its not in second 
			ret.insert(std::make_pair( // add (propertyname|(value1|[empty]))
				ref.first,
				std::make_pair(ref.second,PropertyValue())
			));
		else { //if it is in second as well
			const PropertyValue &first=found->second,&second = ref.second;
			if(!(first.empty() || second.empty() || first==second)) // if they are not empty, but not equal
				ret.insert(std::make_pair( // add (propertyname|(value1|value2))
					ref.first,
					std::make_pair(first,second)
				));
		}
	}
	//insert everything that is in second but not in this
 	BOOST_FOREACH(const_reference ref,other){
		const_iterator found=find(ref.first);
		if(found == end()) // if its not in ref
			ret.insert(std::make_pair( // add (propertyname|([empty]|value2))
				ref.first,
				std::make_pair(PropertyValue(),ref.second)
			));
	}
	return ret;
}

void PropMap::make_unique (const util::PropMap& other, PropMap::key_list ignore ) {
	//remove everything that is also in second and equal
	BOOST_FOREACH(const_reference ref,other){
		if(ignore.find(ref.first)==ignore.end())
			continue;
		iterator found=find(ref.first);
		if(found != other.end() && found->second.operator==(ref.second))
			erase(found);
	}
}

void PropMap::join(const isis::util::PropMap& other, bool overwrite, PropMap::key_list ignore) {
	BOOST_FOREACH(const_reference ref,other){
		if(ignore.find(ref.first)!=ignore.end()){//skip any prop from the ignore-list
			LOG(CoreDebug,verbose_info) << "Ignoring " << ref;
			continue;
		}
		iterator found=find(ref.first);
		if(found != this->end()){ // if its allready here
			if(found->second.empty() || overwrite){
				LOG(CoreDebug,verbose_info) << "Replacing " << MSubject(*found) << " by " << MSubject(ref.second);
				found->second=ref.second;
			}
		} else {
			LOG(CoreDebug,verbose_info) << "Inserting " << ref;
			insert(ref);
		}
	}
}


const PropMap::key_list PropMap::keys()const
{
	return genKeyList<trueP>();
}

const PropMap::key_list PropMap::missing() const{
	return genKeyList<invalidP>();
}

std::ostream& PropMap::print ( std::ostream& out,bool label ) {
	BOOST_FOREACH(const_reference ref,*this)
		out << ref.first << ": " << ref.second->toString(label) << std::endl;
	return out;
}


}}