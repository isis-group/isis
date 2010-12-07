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

namespace isis
{
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util
{

/**
 * Common property value class.
 * TypeValue may store a value of any type (defined in types.cpp) and are equal-compareable.
 * If they dont have a value they are empty. Empty TypeValues are never equal to anything (not even to empty TypeValues).
 * They only store a value but no name, because they will get a name when they are stored (in PropMap).
 * Note, that for this class "not equal" (not ==) does NOT mean "unequal" (!=) - check the documentation of the respective operators.
 * @author Enrico Reimer
 */
class TypeValue: public TypeReference
{
	bool m_needed;
public:
	/**
	 * Default constructor.
	 * Creates and stores a value from any known type.
	 * If the type is not known (there is no Type\<type\> available) an compiler error will be raised.
	 * \param ref the value to be stored
	 * \param _needed flag if this TypeValue is needed an thus not allowed to be empty (a.k.a. undefined)
	 */
	template<typename T> TypeValue( const T &ref, bool _needed = false ):
		TypeReference( new Type<T>( ref ) ), m_needed( _needed ) {
		check_type<T>();
	}
	template<typename T> TypeValue( const Type<T>& ref, bool _needed = false ):
		TypeReference( new Type<T>( ref ) ), m_needed( _needed ) {
		check_type<T>();
	}
	/**
	 * Empty constructor.
	 * Creates an empty property value. So TypeValue().empty() will allways be true.
	 * \param _needed flag if this TypeValue is needed an thus not allowed to be empty (a.k.a. undefined)
	 */
	TypeValue( bool _needed = false );
	/// Accessor for the needed flag
	bool &needed();
	///\copydoc needed
	bool needed()const;

	/**
	 * Equality to another TypeValue.
	 * Properties are equal if, and only if:
	 * - both properties are not empty
	 * - both properties contain the same value type T
	 * - the stored values are equal
	 * \returns (this->cast_to_type\<T\>() == second->cast_to_type\<T\>()) if both contain a value of type T, false otherwise.
	 */
	bool operator ==( const TypeValue &second )const;
	/**
	 * Unequality to another TypeValue.
	 * Properties are unequal if:
	 * - only one of both properties is empty
	 * - or both properties contain the different value of same type T
	 * - or they contain different types
	 * \returns not(*this == second) and not(this->empty() and second.empty()).
	 */
	bool operator !=( const TypeValue &second )const;
	/**
	 * Equality to another Type-Object.
	 * Properties are equal to Type-Object if, and only if:
	 * - the property is not empty
	 * - the property and the Type-Object contain the same value type T
	 * - both stored values are equal
	 * \returns (this->cast_to_type\<T\>() == second->cast_to_type\<T\>()) if both contain a value of type T, false otherwise.
	 */
	bool operator ==( const _internal::TypeBase &second )const;
	/**
	 * Equality to a Value of type T.
	 * Properties are equal to Values if, and only if:
	 * - the property is not empty
	 * - the property contains the value type T or is convertible into it
	 * - stored/converted value is equal to the given value
	 * \warning because of rounding in the conversion the following will be true.
	 * \code TypeValue(4.5)==5 \endcode
	 * If Debug is enabled and its loglevel is at least warning, a message will be send to the logger.
	 * \returns (this->cast_to_type\<T\>() == second) if the property contains a value of type T.
	 * \returns converted_property == second if its convertible
	 * \return false otherwise
	 */
	template<typename T> bool operator ==( const T &second )const {
		check_type<T>();

		if ( get()->is<T>() ) { // If I'm of the same type as the comparator
			const T &cmp = get()->castTo<T>();
			return second == cmp; //compare our values
		} else if ( ! empty() ) { // otherwise try to make me T and compare that
			LOG( Debug, info )
					<< *this << " is not " << Type<T>::staticName() << " trying to convert.";
			TypeReference dst = ( *this )->copyToNewByID( Type<T>::staticID );

			if ( !dst.empty() )
				return dst->castTo<T>() == second;
		}

		return false;
	}
};

}
/** @} */
}

#endif


