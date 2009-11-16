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
	util::PropMap properties;
	static const util::PropMap::mapped_type emptyProp;//dummy to be able to return an empty Property
public:
	/// Create an PropertyObject without any property.
	PropertyObject();
	/**
	 * Create an PropertyObject and make the given properties needed.
	 * As this adds needed but empty properties, the resulting object will be insufficient.
	 * \code
	 * const isis::util::PropMap::key_type needed[]={"Prop1","Prop2"};
	 * assert(PropertyObject(needed).sufficient()==false);
	 * \endcode
	 * \param needed list of properties which will be added to the PropertyMap and flagged as emtpy and needed.
	 */
	PropertyObject(const util::PropMap::key_type needed[]);
	/**
	 * Sets a given property to a given value.
	 * If the property is allready set, it will be reset (but setting a different type will fail).
	 * If the property does not exist it will be replaced.
	 * \param key the name of the property to be set
	 * \param val the value the property should be set to
	 */
	//@todo make shure the type specific behaviour is as documented
	template<typename T> void setProperty(const util::PropMap::key_type &key,const T &val){
		properties[key]=val;
	}
	/**
	 * Get the given property.
	 * \returns a reference of the stored PropertyValue
	 */
	const util::PropMap::mapped_type& getPropertyValue(const util::PropMap::key_type &key)const;
	/**
	 * Get the value of the given Property.
	 * If DataLog is enabled and the stored type is not T an error will be send.
	 * \returns a Type\<T\> containing a copy of the value stored for given property if the type of the stored property is T.
	 * \return Type\<T\>() otherwhise.
	 */
	template<typename T> util::Type<T> getProperty(const util::PropMap::key_type &key)const{
		MAKE_LOG(DataLog);
		const util::PropMap::mapped_type &value=getPropertyValue(key);
		if(value.empty()){
			const util::Type<T> dummy=T();
			LOG(DataLog,util::error)
				<< "Requested Property " << key << " is not set! Returning " << dummy.toString(true) << std::endl;
			return dummy;
		}
		else
			return value->cast_to_Type<T>();
	}
	/// \returns true is the given property does exist and is not empty.
	bool hasProperty(const util::PropMap::key_type &key)const;
	/**
	 * Adds a property as needed.
	 * If the given property allready exists, it is just flagged as needed.
	 */
	void addNeeded(const util::PropMap::key_type &key);
	/// \returns false if there is any needed and empty property, true otherwhise.
	bool sufficient()const;
	/// \returns a list of all needed and empty properties.
	util::PropMap::key_list missing()const;
};
}}}

#endif // PROPERTYOBJECT_H
