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

PropertyObject::PropertyObject ( const char* needed[] ) {
	const size_t len=sizeof(needed)/sizeof(util::PropMap::key_type);
	LOG(DataDebug,util::verbose_info)	<< "Adding " << len << " needed properties ("<< util::list2string(needed,needed+len) << ")";
	for(size_t i=0;i<len;i++)
		addNeeded(needed[i]);
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

bool PropertyObject::hasProperty ( const std::string& key )const {
	return (not getPropertyValue(key).empty());
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