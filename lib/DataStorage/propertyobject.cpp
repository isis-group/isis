//
// C++ Implementation: propertyobject
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "propertyobject.h"
#include <boost/foreach.hpp>

namespace isis{ namespace data { namespace _internal {

	
PropertyObject::PropertyObject(){}


void PropertyObject::addNeededFromString(const std::string& needed)
{
	const std::list<std::string> needList=util::string2list<std::string>(needed);
	LOG(DataDebug,util::verbose_info)	<< "Adding " << util::list2string(needList.begin(),needList.end()) << " as needed";
	BOOST_FOREACH(std::list<std::string>::const_reference ref,needList)
		addNeeded(ref);
}

void PropertyObject::addNeeded ( const std::string& key )
{
	properties[key].needed()=true;
}


const util::PropMap::mapped_type&
PropertyObject::getPropertyValue ( const std::string& key ) const
{
	util::PropMap::const_iterator found=properties.find(key);
	return found!=properties.end() ? found->second: emptyProp;
}

bool PropertyObject::hasProperty ( const std::string& key )const
{
	return (not getPropertyValue(key).empty());
}

void PropertyObject::delProperty(const std::string& key)
{
	properties.erase(key);
}

bool PropertyObject::sufficient()const
{
	return properties.valid();
}

util::PropMap::key_list PropertyObject::missing()const {
	return properties.missing();
}


const util::PropMap& PropertyObject::propMap()const {
	return properties;
}


const util::PropMap::mapped_type PropertyObject::emptyProp;

}}}