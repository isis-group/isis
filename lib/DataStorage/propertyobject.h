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
	static const util::PropMap::mapped_type emptyProp;//dummy to be able to return an empty Property
protected:
	util::PropMap properties;
	/**
	 * Make Properties given by a space separated list needed.
	 * \param needed string made of space serparated property-names which
	 * will (if neccessary) be added to the PropertyMap and flagged as needed.
	 */
	void addNeededFromString(const std::string &needed);
public:
	/// Create an PropertyObject without any property.
	PropertyObject();
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
	const util::PropMap::mapped_type &getPropertyValue(const util::PropMap::key_type &key)const;
	/**
	 * Get the value of the given Property.
	 * If DataLog is enabled and the stored type is not T an error will be send.
	 * \returns a Type\<T\> containing a copy of the value stored for given property if the type of the stored property is T.
	 * \return Type\<T\>() otherwhise.
	 */
	template<typename T> util::Type<T> getProperty(const util::PropMap::key_type &key)const{
		const util::PropMap::mapped_type &value=getPropertyValue(key);
		if(value.empty()){
			const util::Type<T> dummy=T();
			LOG(DataLog,util::error)
				<< "Requested Property " << key << " is not set! Returning " << dummy.toString(true);
			return dummy;
		}
		else
			return value->cast_to_Type<T>();
	}
	/// \returns true is the given property does exist and is not empty.
	bool hasProperty(const util::PropMap::key_type &key)const;
	/// Removes the given property if its there.
	void delProperty(const util::PropMap::key_type &key);
	/**
	 * Adds a property as needed.
	 * If the given property allready exists, it is just flagged as needed.
	 */
	void addNeeded(const util::PropMap::key_type &key);
	/**
	* Check if every needed property is set.
	* \returns false if there is any needed and empty property, true otherwhise.
	*/
	bool sufficient()const;
	/**
	 * Get a list of missing properties.
	 * \returns a list of all needed and empty properties.
	 */
	util::PropMap::key_list missing()const;
	const util::PropMap &propMap()const;
};
}}}

#endif // PROPERTYOBJECT_H
