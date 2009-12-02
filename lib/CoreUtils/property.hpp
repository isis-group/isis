//
// C++ Interface: property
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef ISISPROPERTY_HPP
#define ISISPROPERTY_HPP

#include <boost/shared_ptr.hpp>
#include <map>
#include "type.hpp"
#include "log.hpp"

namespace isis{ 
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util{

/**
 * Common property value class.
 * This only has a value (no name) because it will get a name when it is stored (in PropMap).
 * @author Enrico Reimer
 */

class PropertyValue:public _internal::TypeBase::Reference{
	bool m_needed;
public:
	/**
	 * Default constructor.
	 * Creates and stores a value from any known type.
	 * If the type is not known (there is no Type\<type\> available) an compiler error will be raised.
	 * \param ref the value to be stored
	 * \param _needed flag if this PropertyValue is needed an thus not allowed to be empty (a.k.a. undefined)
	 */ 
	template<typename T> PropertyValue(const T& ref,bool _needed = false):
	_internal::TypeBase::Reference(new Type<T>(ref)),m_needed(_needed){ }
	/**
	 * Empty constructor.
	 * Creates an empty property value. So PropertyValue().empty() will allways be true.
	 * \param _needed flag if this PropertyValue is needed an thus not allowed to be empty (a.k.a. undefined)
	 */
	PropertyValue(bool _needed = false);
	/// Accessor for the needed flag
	bool &needed();
	bool needed()const;

	/**
	 * Equality to another PropertyValue.
	 * Properties are equal if, and only if:
	 * - both properties are not empty
	 * - both properties contain the same value type T
	 * - the stored values are equal
	 * \returns (this->cast_to_type\<T\>() == second->cast_to_type\<T\>()) if both contain a value of type T, false otherwise.
	 */
	bool operator ==(const PropertyValue &second)const;
	/**
	 * Equality to another Type-Object.
	 * Properties are equal to Type-Object if, and only if:
	 * - the property is not empty
	 * - the property and the Type-Object contain the same value type T
	 * - both stored values are equal
	 * \returns (this->cast_to_type\<T\>() == second->cast_to_type\<T\>()) if both contain a value of type T, false otherwise.
	 */
	bool operator ==(const _internal::TypeBase &second)const;
	/**
	 * Equality to a Value of type T.
	 * Properties are equal to Values if, and only if:
	 * - the property is not empty
	 * - the property contains the value type T
	 * - both stored value is equal to the given value
	 * \returns (this->cast_to_type\<T\>() == second) if the property contains a value of type T, false otherwise.
	 */
	template<typename T> bool operator ==(const T &second)const{
		if(get()->is<T>())
			return second == get()->cast_to_Type<T>();
		else
			return false;
	}
};

}
/** @} */
}

#endif


