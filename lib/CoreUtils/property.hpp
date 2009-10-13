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
	 * \param ref the value to be stored
	 * \param _needed flag if this PropertyValue is needed an thus not allowed to be empty (a.k.a. undefined)
	 */
	PropertyValue(bool _needed = false);
	/**
	 * Implicit conversion of the property value to its actual type.
	 * Does a TypeBase::cast_to_type\<T\>() and returns a copy of its value.
	 * Trying to convert to another type that the property was created from will raise a bad_cast exception at runtime.
	 * \returns a copy of the stored value
	 */
	template<typename T> operator T()const{
		const element_type *dummy=get();
		const Type<T> ret=dummy->cast_to_Type<T>();
		return (T)ret;
	}
	/// Accessor for the needed flag
	bool &needed();
	bool needed()const;

	bool operator ==(const PropertyValue &second)const;
	bool operator ==(const _internal::TypeBase &second)const;
	template<typename T> bool operator ==(const T &second)const{
		if(get()->is<T>())
			return get()->cast_to_Type<T>() == second;
		else
			return false;
	}
};

}
/** @} */
}

#endif


