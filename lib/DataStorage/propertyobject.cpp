#include "propertyobject.h"

namespace isis{ namespace data { namespace _internal {

void PropertyObject::addNeeded ( const std::string& key )
{
	properties[key].needed()=true;
}


::isis::util::PropMap::mapped_type
PropertyObject::getPropertyValue ( const std::string& key ) const
{
	::isis::util::PropMap::const_iterator found=properties.find(key);
	return found!=properties.end() ? found->second: ::isis::util::PropMap::mapped_type();
}

bool PropertyObject::hasProperty ( const std::string& key )const {
	return (not getPropertyValue(key).empty());
}

}}}