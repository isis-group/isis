//
// C++ Interface: propertyobject
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef PROPERTYOBJECT_H
#define PROPERTYOBJECT_H

#include "CoreUtils/propmap.hpp"
#include "common.hpp"

namespace isis{ namespace data{ namespace _internal {
class PropertyObject {
	::isis::util::PropMap properties;
public:
	template<typename T> void setProperty(const ::isis::util::PropMap::key_type &key,const T &val){
		properties[key]=val;
	}
	::isis::util::PropMap::mapped_type getPropertyValue(const ::isis::util::PropMap::key_type &key)const;
	template<typename T> isis::util::Type<T> getProperty(const ::isis::util::PropMap::key_type &key)const{
		MAKE_LOG(DataLog);
		const ::isis::util::PropMap::mapped_type value=getPropertyValue(key);
		if(value.empty()){
			const isis::util::Type<T> dummy=T();
			LOG(DataLog,isis::util::error)
				<< "Requested Property " << key << " is not set! Returning " << dummy.toString(true) << std::endl;
			return dummy;
		}
		else
			return value->cast_to_Type<T>();
	}
	bool hasProperty(const ::isis::util::PropMap::key_type &key)const;
	void addNeeded(const ::isis::util::PropMap::key_type &key);
};
}}}

#endif // PROPERTYOBJECT_H
