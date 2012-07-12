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
#include "value.hpp"
#include "log.hpp"

namespace isis
{
namespace util
{

/**
 * A very generic class to store values of properties.
 * PropertyValue may store a value of any type (defined in types.cpp) otherwise it's empty.
 * Non-empty ValueValues are equal-compareable.
 * But empty PropertyValues are neigther equal nor unequal to anything (not even to empty ValueValues).
 * @author Enrico Reimer
 */
class PropertyValue: public ValueReference
{
	bool m_needed;
	//prevent usage of ValueReference's dereferencing (the hooks below should be used) (use "*" if you really have to)
	ValueBase *operator->();
	const ValueBase *operator->()const;
public:
	/**
	 * Default constructor.
	 * Creates and stores a value from any known type.
	 * If the type is not known (there is no Value\<type\> available) an compiler error will be raised.
	 * \param ref the value to be stored
	 * \param _needed flag if this PropertyValue is needed an thus not allowed to be empty (a.k.a. undefined)
	 */
	template<typename T> PropertyValue( const T &ref, bool _needed = false ):
		ValueReference( new Value<T>( ref ) ), m_needed( _needed ) {
		checkType<T>();
	}
	template<typename T> PropertyValue( const Value<T>& ref, bool _needed = false ):
		ValueReference( new Value<T>( ref ) ), m_needed( _needed ) {
		checkType<T>();
	}
	/**
	 * Empty constructor.
	 * Creates an empty property value. So PropertyValue().isEmpty() will allways be true.
	 */
	PropertyValue();
	/// accessor to mark as (not) needed
	bool &needed();
	/// returns true if PropertyValue is marked as needed, false otherwise
	bool isNeeded ()const;

	/**
	 * Equality to another PropertyValue.
	 * Properties are ONLY equal if:
	 * - both properties are not empty
	 * - both properties contain the same value type T
	 * - the stored values are equal
	 * \returns true if both contain the same value of type T, false otherwise.
	 */
	bool operator ==( const PropertyValue &second )const;
	/**
	 * Unequality to another PropertyValue.
	 * Properties are ONLY unequal if:
	 * - only one of both properties is empty
	 * - or both properties contain a different value of same type T
	 * - or they contain different types
	 * If both are empty, they are not unequal
	 * \returns true if they differ in type T or in value of same type, false otherwise.
	 */
	bool operator !=( const PropertyValue &second )const;
	/**
	 * Equality to another Value-Object (this cannot be empty but PropertyValue can).
	 * Properties are ONLY equal to Value-Object if:
	 * - the property and the Value-Object contain the same value type T
	 * - both stored values are equal
	 * \returns true if both contain the same value of type T, false otherwise.
	 */
	bool operator ==( const ValueBase &second )const;
	/**
	 * Equality to a Value of type T (convenience function).
	 * Properties are ONLY equal to Values if:
	 * - the property is not empty
	 * - the property contains the value type T or is convertible into it
	 * - stored/converted value is equal to the given value
	 * \warning because of rounding in the conversion the following will be true.
	 * \code PropertyValue(4.5)==5 \endcode
	 * If Debug is enabled and its loglevel is at least warning, a message will be send to the logger.
	 * \returns true if both contain the same value of type T, false otherwise.
	 */
	template<typename T> bool operator ==( const T &second )const {
		checkType<T>();

		if( isEmpty() ) {
			return false;
		}

		if ( get()->is<T>() ) { // If I'm of the same type as the comparator
			const T &cmp = get()->castTo<T>();
			return second == cmp; //compare our values
		} else if ( ! isEmpty() ) { // otherwise try to make me T and compare that
			LOG( Debug, info )
					<< *this << " is not " << Value<T>::staticName() << ", trying to convert.";
			ValueReference dst = ( **this ).copyByID( Value<T>::staticID );

			if ( !dst.isEmpty() )
				return dst->castTo<T>() == second;
		}

		return false;
	}

	/**
	 * \copybrief ValueBase::as
	 * hook for \link ValueBase::as \endlink
	 */
	template<class T> T as()const {
		LOG_IF(!*this, Debug,error) << "Doing as<" << util::Value<T>::staticName() <<  ">() on empty property, this will crash";
		return ( **this ).as<T>();
	}

	/**
	 * \copybrief ValueBase::is
	 * hook for \link ValueBase::is \endlink
	 */
	template<class T> bool is()const {
		LOG_IF(!*this, Debug,error) << "Doing is<" << util::Value<T>::staticName() <<  ">() on empty property, this will crash";
		return ( **this ).is<T>();
	}

	/**
	 * \copybrief ValueBase::getTypeName
	 * hook for \link ValueBase::getTypeName \endlink
	 */
	std::string getTypeName()const;

	/**
	 * \copybrief ValueBase::getTypeID
	 * hook for \link ValueBase::getTypeID \endlink
	 */
	unsigned short getTypeID()const;
	
	/**
	 * \copybrief ValueBase::castTo
	 * hook for \link ValueBase::castTo \endlink
	 */
	template<class T> T &castTo()const {
		LOG_IF(!*this, Debug,error) << "Casting empty property to " << util::MSubject(util::Value<T>::staticName()) << ", this will crash";
		return ( **this ).castTo<T>();
	}
};

}
}
#endif


