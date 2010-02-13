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

const util::PropertyValue PropMap::emptyProp;//dummy to be able to return an empty Property


///////////////////////////////////////////////////////////////////
// Contructors
///////////////////////////////////////////////////////////////////

PropMap::PropMap(const isis::util::PropMap::base_type& src):
std::map< std::string, PropertyValue, _internal::caselessStringLess >(src){}

bool PropMap::operator==(const PropMap& src)const
{
	const std::map< std::string, PropertyValue, _internal::caselessStringLess > &other=src,&me=*this;
	return me==other;
}

PropMap::PropMap() {}


///////////////////////////////////////////////////////////////////
// The core tree traversal functions
///////////////////////////////////////////////////////////////////
PropertyValue& PropMap::fetchProperty(
	PropMap& root,
	const isis::util::PropMap::propPathIterator at, const isis::util::PropMap::propPathIterator pathEnd)
{
	std::list< std::string >::const_iterator next=at;next++;
	util::PropMap::iterator found=root.find(*at);
	if(next!=pathEnd){//we are not at the end of the path (aka the leaf)
		if(found!=root.end()){//and we found the entry
			util::PropertyValue &ref=found->second;
			LOG_IF(not ref->is<util::PropMap>(),CoreLog,util::error)
			<< util::MSubject(found->first) << " is a leaf, but requested as a branch in "
			<< util::MSubject(util::list2string(at,pathEnd,"/")) << " programm will stop";
			return fetchProperty(ref->cast_to_Type<util::PropMap>(),next,pathEnd); //continue there
		} else { // if we should create a sub-map
			//insert a empty branch (aka PropMap) at "*at" (and fetch the reference of that)
			util::PropMap &inserted= (root[*at]=util::PropMap())->cast_to_Type<util::PropMap>();
			return fetchProperty(inserted,next,pathEnd); // and continue there
		}
	} else { //if its the leaf
		std::map<std::string,PropertyValue,_internal::caselessStringLess> &ref=root;
		return ref[*at]; // (create and) return that entry
	}
}
const PropertyValue* PropMap::searchBranch(
	const PropMap& root,
	const propPathIterator at,const propPathIterator pathEnd)
{
	propPathIterator next=at;next++;
	util::PropMap::const_iterator found=root.find(*at);
	if(next!=pathEnd){//we are not at the end of the path (aka the leaf)
		if(found!=root.end()){//and we found the entry
			const util::PropertyValue &ref=found->second;
			LOG_IF(not ref->is<util::PropMap>(),CoreLog,util::error)
			<< util::MSubject(found->first) << " is a leaf, but requested as a branch in "
			<< util::MSubject(util::list2string(at,pathEnd,"/")) << " programm will stop";
			return searchBranch(ref->cast_to_Type<util::PropMap>(),next,pathEnd); //continue there
		}
	} else if(found!=root.end()){// if its the leaf and we found the entry
		return &found->second; // return that entry
	}
	return NULL;
}
bool PropMap::recursiveRemove(PropMap& root,const propPathIterator at, const propPathIterator pathEnd)
{
	bool ret=true;
	if(at!=pathEnd){
		propPathIterator next=at;next++;
		util::PropMap::iterator found=root.find(*at);
		if(found!=root.end()){
			util::PropertyValue &ref=found->second;
			if(ref->is<util::PropMap>()){
				util::PropMap &sub=ref->cast_to_Type<util::PropMap>();
				ret=recursiveRemove(sub,next,pathEnd);
				if(sub.empty())
					root.erase(found);
			} else
				root.erase(found);
		} else {
			LOG(CoreLog,util::warning)<< "Entry " << util::MSubject(*at) << " not found, skipping it";
			ret=false;
		}
	}
	return ret;
}


/////////////////////////////////////////////////////////////////////////////////////
// Generic interface for accessing elements
////////////////////////////////////////////////////////////////////////////////////
const isis::util::PropertyValue* PropMap::findPropVal(const std::string& key) const {
	const propPath path=util::string2list<std::string>(key,pathSeperator);
	return searchBranch(*this,path.begin(),path.end());
}

PropertyValue& PropMap::operator[](const std::string& key) {
	const propPath path=util::string2list<std::string>(key,pathSeperator);
	return fetchProperty(*this,path.begin(),path.end());
}

bool PropMap::remove(const std::string& key) {
	const propPath path=util::string2list<std::string>(key,pathSeperator);
	return recursiveRemove(*this,path.begin(),path.end());
}

bool PropMap::exists(const std::string& key)const {
	return findPropVal(key)!=NULL;
}


/////////////////////////////////////////////////////////////////////////////////////
// utilities
////////////////////////////////////////////////////////////////////////////////////
bool PropMap::valid() const
{
	//iterate through the whole map and return false as soon as we find something needed _and_ empty
	const const_iterator found=std::find_if(begin(),end(),treeInvalidP());
	return found==end();
}

PropMap::diff_map PropMap::diff(const PropMap& other) const{
	PropMap::diff_map ret;
	diffTree(other,ret,"");
	return ret;
}
	
void PropMap::diffTree(const PropMap& other,PropMap::diff_map &ret,std::string prefix) const{
	const_iterator otherIt=other.begin();

	//insert everything that is in this, but not in second or is on both but differs
 	for(const_iterator thisIt=begin();thisIt!=end();thisIt++){
		const std::string pathname=prefix+thisIt->first;
		//find the closest match for thisIt->first in other (use the value-comparison-functor of PropMap)
		if (continousFind(otherIt, other.end(),*thisIt, value_comp()))
		{ //otherIt->first == thisIt->first - so its the same property
			const PropertyValue &first=thisIt->second,&second = otherIt->second;
			if(
				not (first.empty() or second.empty())
				and  first->is<PropMap>() and second->is<PropMap>()
			) {
				PropMap &thisMap=first->cast_to_Type<PropMap>();
				PropMap &refMap=second->cast_to_Type<PropMap>();
				thisMap.diffTree(refMap,ret,pathname+"/");
			} else if( not(first.empty() and second.empty())and not(first==second)){ // if they are not both empty, but not equal
				ret.insert(// add (propertyname|(value1|value2))
					ret.end(),		// we know it has to be at the end
					std::make_pair(
						pathname,	//the key
						std::make_pair(first,second) //pair of both values
					)
				);
			}
		}else // if ref is not in the other map
			ret.insert(// add (propertyname|(value1|[empty]))
				ret.end(),		// we know it has to be at the end
				std::make_pair(
					pathname,
					std::make_pair(thisIt->second,PropertyValue())
				)
			);
	}
	//insert everything that is in second but not in this
	const_iterator thisIt=begin();
	for(otherIt=other.begin();otherIt!=other.end();otherIt++){
		const std::string pathname=prefix+otherIt->first;
		if (not continousFind(thisIt, end(),*otherIt, value_comp()))//there is nothing in this which has the same key as ref
			ret.insert(
				std::make_pair( // add (propertyname|([empty]|value2))
					pathname,
					std::make_pair(PropertyValue(),otherIt->second)
				)
			);
	}
}

void PropMap::make_unique (const util::PropMap& other ) {
	iterator thisIt=begin();
	
	//remove everything that is also in second and equal
	for(const_iterator otherIt=other.begin();otherIt!=other.end();otherIt++){
		//find the closest match for otherIt->first in this (use the value-comparison-functor of PropMap)
		if (
			continousFind(thisIt, end(),*otherIt, value_comp()) //thisIt->first == otherIt->first  - so its the same property
			and thisIt->second.operator==(otherIt->second) //if the values of this prop are equal (they are not equal if they are empty)
		){
			if(thisIt->second->is<PropMap>() && otherIt->second->is<PropMap>()){
				PropMap &thisMap=thisIt->second->cast_to_Type<PropMap>();
				PropMap &refMap=otherIt->second->cast_to_Type<PropMap>();
				thisMap.make_unique(refMap);
			} else
				erase(thisIt);
		}
	}
}


PropMap::key_list PropMap::join(const isis::util::PropMap& other, bool overwrite) {
	key_list rejects;
	joinTree(other, overwrite, "", rejects);
	return rejects;
}

void PropMap::joinTree(const isis::util::PropMap& other, bool overwrite, std::string prefix, PropMap::key_list &rejects) {
	iterator thisIt=begin();
	
	for(const_iterator otherIt=other.begin();otherIt!=other.end();otherIt++){
		//find the closest match for otherIt->first in ignore (use the comparison-functor of PropMap)
		if(continousFind(thisIt, end(),*otherIt, value_comp()))
		{ // if its allready here
			if(thisIt->second.empty()){
				LOG(CoreDebug,verbose_info) << "Replacing empty property " << MSubject(thisIt->first) << " by " << MSubject(otherIt->second);
				thisIt->second=otherIt->second;
			} else if(thisIt->second->is<PropMap>() && otherIt->second->is<PropMap>()){
				PropMap &thisMap=thisIt->second->cast_to_Type<PropMap>();
				PropMap &refMap=otherIt->second->cast_to_Type<PropMap>();
				thisMap.joinTree(refMap,overwrite,prefix+thisIt->first+"/",rejects);
			} else if(overwrite) {
				LOG(CoreDebug,info) << "Replacing property " << MSubject(*thisIt) << " by " << MSubject(otherIt->second);
				thisIt->second=otherIt->second;
			} else if(not (thisIt->second == otherIt->second)){
				LOG(CoreDebug,info)
					<< "Rejecting property " << MSubject(*otherIt)
					<< " because "<< MSubject(thisIt->second) << " is allready there";
				rejects.insert(rejects.end(),prefix+otherIt->first);
			}
		} else {
			std::pair<const_iterator,bool> inserted=insert(*otherIt);
			LOG_IF(inserted.second,CoreDebug,verbose_info) << "Inserted property " << MSubject(*inserted.first) << ".";
		}
	}
}


size_t PropMap::linearize(isis::util::PropMap::base_type& out, std::string key_prefix) const
{
	for(const_iterator i=begin();i!=end();i++){
		std::string key= (key_prefix.empty() ? "":key_prefix+pathSeperator)+i->first;
		if((not i->second.empty()) and i->second->is<PropMap>()){
			util::PropMap &sub=i->second->cast_to_Type<util::PropMap>();
			sub.linearize(out,key);
		} else
			out.insert(std::make_pair(key,i->second));
	}
	
}

bool PropMap::transform( std::string from,  std::string to, int dstId,bool delSource) {
	LOG_IF(from==to,CoreDebug,error) << "Sorry source and destination shall not be the same";
	const PropertyValue *found=findPropVal(from);
	bool ret=false;
	if(found and not found->empty()){
		if((*found)->typeID()==dstId){
			setPropertyValue(to,*found);
			ret = true;
		} else 
			ret=found->transformTo(operator[](to),dstId);
	}
	if(ret and delSource)remove(from);
	return ret;
}


const PropMap::key_list PropMap::keys()const
{
	PropMap::key_list ret;
	std::for_each(begin(),end(),walkTree<trueP>(ret));
	return ret;
}

const PropMap::key_list PropMap::missing() const{
	PropMap::key_list ret;
	std::for_each(begin(),end(),walkTree<invalidP>(ret));
	return ret;
}


void PropMap::addNeeded(const std::string& key)
{
	operator[](key).needed()=true;
}


void PropMap::addNeededFromString(const std::string& needed)
{
	const std::list<std::string> needList=util::string2list<std::string>(needed);
	LOG(CoreDebug,util::verbose_info)	<< "Adding " << util::list2string(needList.begin(),needList.end()) << " as needed";
	BOOST_FOREACH(std::list<std::string>::const_reference ref,needList)
		addNeeded(ref);
}

bool PropMap::hasProperty(const std::string& key) const {
	const PropertyValue* found=findPropVal(key);
	return ( found and not found->empty() );
}


const isis::util::PropertyValue& PropMap::getPropertyValue(const std::string& key) const {
	const PropertyValue* found=findPropVal(key);
	return found ? *found: emptyProp;
}


PropertyValue& PropMap::setPropertyValue(const std::string& key, const PropertyValue& val)
{
	PropertyValue &ret= operator[](key)=val;
	return ret;
}

bool PropMap::renameProperty(std::string oldname,std::string newname) {
	const PropertyValue* found=findPropVal(oldname);
	if(found){
		LOG_IF(hasProperty(newname),CoreLog,warning)
			<< "Overwriting " << std::make_pair(newname,getPropertyValue(newname)) << " with " << *found;
		operator[](newname)=*found;
		return remove(oldname);
	} else {
		LOG(CoreLog,warning)
			<< "Cannot rename " << oldname << " it does not exist";
		return false;
	}
}


void PropMap::toCommonUnique(PropMap& common,std::set<std::string> &uniques,bool init)const {
	if(init){
		common= *this;
		uniques.clear();
		return;
	} else {
		const util::PropMap::diff_map difference=common.diff(*this);
		BOOST_FOREACH(const util::PropMap::diff_map::value_type &ref,difference){
			uniques.insert(ref.first);
			if(not ref.second.first.empty())common.remove(ref.first);//if there is something in common, remove it
		}	
	}
}


std::ostream& PropMap::print( std::ostream& out,bool label)const {
	base_type buff;
	linearize(buff);
	size_t key_len=0;
	for(base_type::const_iterator i=buff.begin();i!=buff.end();i++)
		if(key_len < i->first.length())
			key_len = i->first.length();

	for(base_type::const_iterator i=buff.begin();i!=buff.end();i++)
		out << i->first << std::string(key_len-i->first.length(),' ')+":" << i->second.toString(label) << std::endl;
	return out;
}


}}
