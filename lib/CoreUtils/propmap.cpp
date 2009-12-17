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
		//find the closest match for ref.first in ignore (use the key-comparison-functor of PropMap)
		if (continousFind(ignoreIt, ignore.end(),ref.first, key_comp()))
			continue; //skip if ref.first == *ignore
		
		//find the closest match for ref.first in other (use the value-comparison-functor of PropMap)
		if (continousFind(otherIt, other.end(),ref, value_comp()))
		{ //otherIt->first == ref.first - so its the same property
			const PropertyValue &first=ref.second,&second = otherIt->second;
			if(!(first.empty() || second.empty() || first==second)) // if they are not both empty, but not equal
				ret.insert(// add (propertyname|(value1|value2))
					ret.end(),		// we know it has to be at the end
					std::make_pair(
						ref.first,		//the key
						std::make_pair(first,second) //pair of both values
					)
				);
		}else // if ref is not in the other map
			ret.insert(// add (propertyname|(value1|[empty]))
				ret.end(),		// we know it has to be at the end
				std::make_pair(
					ref.first,
					std::make_pair(ref.second,PropertyValue())
				)
			);
	}
	//insert everything that is in second but not in this
	const_iterator thisIt=begin();
	BOOST_FOREACH(const_reference ref,other){
		if (not continousFind(thisIt, end(),ref, value_comp()))//there is nothing in this which has the same key as ref
			ret.insert(
				std::make_pair( // add (propertyname|([empty]|value2))
					ref.first,
					std::make_pair(PropertyValue(),ref.second)
				)
			);
	}
	return ret;
}

void PropMap::make_unique (const util::PropMap& other, PropMap::key_list ignore ) {
	key_list::const_iterator ignoreIt= ignore.begin();
	iterator thisIt=begin();
	
	//remove everything that is also in second and equal
	BOOST_FOREACH(const_reference ref,other){
		//find the closest match for ref.first in ignore (use the comparison-functor of PropMap)
		if (continousFind(ignoreIt, ignore.end(),ref.first, key_comp()))
			continue; //skip if ref.first == *ignore
			
		//find the closest match for ref.first in this (use the value-comparison-functor of PropMap)
		if (
			continousFind(thisIt, end(),ref, value_comp()) //ref.first == otherIt->first  - so its the same property
			and thisIt->second.operator==(ref.second) //if the values of this prop are equal
		)
			erase(thisIt);
	}
}

PropMap::key_list PropMap::join(const isis::util::PropMap& other, bool overwrite, PropMap::key_list ignore) {
	key_list::const_iterator ignoreIt= ignore.begin();
	key_list rejects;
	iterator thisIt=begin();
	
	BOOST_FOREACH(const_reference ref,other){
		//find the closest match for ref.first in ignore (use the comparison-functor of PropMap)
		if (continousFind(ignoreIt, ignore.end(),ref.first, key_comp()))
			continue; //skip if ref.first == *ignore

		if(continousFind(thisIt, end(),ref, value_comp()))
		{ // if its allready here
			if(thisIt->second.empty() || overwrite){
				LOG(CoreDebug,verbose_info) << "Replacing " << MSubject(*thisIt) << " by " << MSubject(ref.second);
				thisIt->second=ref.second;
			} else
				rejects.insert(rejects.end(),ref.first);

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