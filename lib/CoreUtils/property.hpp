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

class PropertyValue:public _internal::TypeContainer{
	bool m_needed;
public:
	/**
	 * Default constructor.
	 * Creates and stores a value from any known type.
	 * If the type is not known (there is no Type\<type\> available) an compiler error will be raised.
	 */ 
	template<typename T> PropertyValue(const T& ref,bool _needed = false):_internal::TypeContainer(new Type<T>(ref)),m_needed(_needed){ }
	/**
	 * Empty constructor.
	 * Creates an empty property value. So PropertyValue().empty() is allways true;
	 */
	PropertyValue(bool _needed = false):m_needed(_needed){ }
	/**
	 * Implicit conversion of the property value to its actual type.
	 * Does a TypeBase::cast_to_type\<T\>() and returns a copy of its value.
	 * Trying to convert to another type that the property was created from will raise a bad_cast exception at runtime.
	 * \returns a copy of the stored value
	 */
	template<typename T> operator T()const{
		const _internal::TypeBase *dummy=get();
		const Type<T> ret=dummy->cast_to_Type<T>();
		return (T)ret;
	}
	/// Accessor for the needed flag
	bool &needed(){
		return m_needed;
	}
};

}
/** @} */
}

#endif


