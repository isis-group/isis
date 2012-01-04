//
// C++ Interface: type
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef ISISTYPE_HPP
#define ISISTYPE_HPP

#include "type_base.hpp"
#include "string.h"

#include <string>
#include <functional>
#include <boost/type_traits/is_float.hpp>
#include <boost/type_traits/is_integral.hpp>

namespace isis
{
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util
{

template<class TYPE > class Value;

namespace _internal
{

/// @cond _hidden
/**
 * Generic value comparison class for Value.
 * This generic class does nothing, and the ()-operator will allways fail with an error send to the debug-logging.
 * It has to be (partly) specialized for the regarding type.
 */
template<typename T, bool isNumber> class type_compare
{
public:
	type_compare() {} // c++11 says we need a user defined constructor here
	bool operator()( const util::Value<T> &/*first*/, const ValueBase &/*second*/ )const {
		LOG( Debug, error ) << "comparison of " << util::Value<T>::staticName() << " is not supportet";
		return false;
	}
};

/**
 * Half-generic value comparison class for numeric types.
 * This generic class does compares numeric Value's by converting the second
 * Value-object to the type of the first Value-object. Then:
 * - if the conversion was successfull (the second value can be represented in the type of the first) the "inRange"-comparison is used
 * - if the conversion failed with an positive or negative overflow (the second value is to high/low to fit into the type of the first) a info sent to the debug-logging and the posOverflow/negOverflow comarison us used
 * - if there is no known conversion from second to first an error is sent to the debug-logging and false is returned
 * The comparison functions (inRange/posOverflow,negOverflow) here are only stubs and will allways return false.
 * So, these class has to be further specialized for the regarding compare operation.
 */
template<typename T> class type_compare<T, true>
{
protected:
	virtual bool posOverflow( const util::Value<T> &/*first*/, const util::Value<T> &/*second*/ )const {return false;} //default to false
	virtual bool negOverflow( const util::Value<T> &/*first*/, const util::Value<T> &/*second*/ )const {return false;} //default to false
	virtual bool inRange( const util::Value<T> &/*first*/, const util::Value<T> &/*second*/ )const {return false;} //default to false
public:
	bool operator()( const util::Value<T> &first, const ValueBase &second )const {
		// ask second for a converter from itself to Value<T>
		const ValueBase::Converter conv = second.getConverterTo( util::Value<T>::staticID );

		if ( conv ) {
			//try to convert second into T and handle results
			util::Value<T> buff;

			switch ( conv->convert( second, buff ) ) {
			case boost::numeric::cPosOverflow:
				LOG( Debug, info ) << "Positive overflow when converting " << second.toString( true ) << " to " << util::Value<T>::staticName() << ".";
				return posOverflow( first, buff );
			case boost::numeric::cNegOverflow:
				LOG( Debug, info ) << "Negative overflow when converting " << second.toString( true ) << " to " << util::Value<T>::staticName() << ".";
				return negOverflow( first, buff );
			case boost::numeric::cInRange:
				return inRange( first, buff );
			}
		} else {
			LOG( Debug, error ) << "No conversion of " << second.getTypeName() << " to " << util::Value<T>::staticName() << " available";
			return false;
		}

		return false;
	}
	type_compare() {} // c++11 says we need a user defined constructor here
	virtual ~type_compare() {}
};

template<typename T, bool isNumber> class type_less : public type_compare<T, isNumber>
{
public: // c++11 says we need a user defined constructor here
	type_less() {}
};// we are going to specialize this for numeric T below
template<typename T, bool isNumber> class type_greater : public type_compare<T, isNumber>
{
public: // c++11 says we need a user defined constructor here
	type_greater() {}
};
template<typename T, bool isNumber> class type_eq : public type_compare<T, isNumber>
{
protected:
	bool inRange( const util::Value<T> &first, const util::Value<T> &second )const {
		return static_cast<const T &>( first ) == static_cast<const T &>( second );
	}
public: // c++11 says we need a user defined constructor here
	type_eq() {}
};

/// less-than comparison for arithmetic types
template<typename T> class type_less<T, true> : public type_compare<T, true>
{
protected:
	bool posOverflow( const util::Value<T> &/*first*/, const util::Value<T> &/*second*/ )const {
		return true; //getting an positive overflow when trying to convert second into T, obviously means first is less
	}
	bool inRange( const util::Value<T> &first, const util::Value<T> &second )const {
		return static_cast<const T &>( first ) < static_cast<const T &>( second );
	}
public: // c++11 says we need a user defined constructor here
	type_less() {}
};

/// greater-than comparison for arithmetic types
template<typename T> class type_greater<T, true> : public type_compare<T, true>
{
protected:
	bool negOverflow( const util::Value<T> &/*first*/, const util::Value<T> &/*second*/ )const {
		return true; //getting an negative overflow when trying to convert second into T, obviously means first is greater
	}
	bool inRange( const util::Value<T> &first, const util::Value<T> &second )const {
		return static_cast<const T &>( first ) > static_cast<const T &>( second );
	}
public: // c++11 says we need a user defined constructor here
	type_greater() {}
};

/// @endcond _hidden
}

/**
 * Generic class for type aware variables.
 * Only this generic approach for types makes it possible to handle all the types of Properties for the different
 * data these library can handle. On the other side it's more complex to read and write with these kind of types.
 * Please don't bother about and look carefully at further comments on functionality and examples in use,
 * e.g. with PropertyValue.\n
 * For supported types see types.hpp \n
 * Another advantage is the available type conversion, for further information how to do this and
 * limitations see type_converter.hpp
 */

template<typename TYPE> class Value: public _internal::ValueBase
{
	TYPE m_val;
	static const char m_typeName[];
protected:
	ValueBase *clone() const {
		return new Value<TYPE>( *this );
	}
public:
	static const unsigned short staticID = _internal::TypeID<TYPE>::value;
	Value(): m_val() {
		BOOST_MPL_ASSERT_RELATION( staticID, < , 0xFF );
		checkType<TYPE>();
	}
	/**
	 * Create a Value from any type.
	 * If the type of the parameter is not the same as the content type of the object, the system tries to do a lexical cast.
	 * - The lexical cast is _not_ a conversion so no rounding or range check is done
	 * - _All_ types which can be lexically casted are allowed, not only types known to isis. But not all types will work.
	 * - If the lexical cast fails, boost::bad_lexical_cast is thrown.
	 */
	template<typename T> Value( const T &value ) {
		m_val = _internal::__cast_to<TYPE>()( this, value );
		BOOST_MPL_ASSERT_RELATION( staticID, < , 0xFF );
		checkType<TYPE>();
	}
	/**
	 * Get the string representation of the Value.
	 * This tries to use the isis type conversion to create a string from the Value.
	 * If thats no available, it will fall back to boost::lexical_cast. And it will send a warning to CoreDebug.
	 * If the lexical cast fails as well, boost::bad_lexical_cast is thrown.
	 * \param labeled if true the typename will be appended to the resulting string in brackets.
	 */
	std::string toString( bool labeled = false )const {
		std::string ret;
		Reference ref = copyByID( Value<std::string>::staticID );

		if ( ref.isEmpty() ) {
			LOG( Debug, warning ) << "Automatic conversion of " << *this << " to string failed. Falling back to boost::lexical_cast<std::string>";
			ret = boost::lexical_cast<std::string>( m_val );
		} else {
			ret = ref->castTo<std::string>();
		}

		if ( labeled )ret += "(" + staticName() + ")";

		return ret;
	}

	std::string getTypeName()const {return staticName();}
	unsigned short getTypeID()const {return staticID;}
	bool isFloat() const {return boost::is_float< TYPE >::value;}
	bool isInteger() const {return boost::is_integral< TYPE >::value;}

	/// \returns true if and only if this and second do contain the same value of the same type
	virtual bool operator==( const ValueBase &second )const {
		if ( second.is<TYPE>() ) {
			return m_val == second.castTo<TYPE>();
		} else
			return  false;
	}

	/// \returns the name of the type
	static std::string staticName() {return m_typeName;}

	/**
	 * Implicit conversion of Value to its value type.
	 * Only the actual type is allowed.
	 * However, the following is valid:
	 * \code
	 * Value<int> i(5);
	 * float f=i;
	 * \endcode
	 * In this case the function returns int which is then also implicitely converted to float.
	 * \return a const reference to the stored value
	 */
	operator const TYPE &()const {return m_val;}

	/**
	 * Implicit conversion of Value to its value type.
	 * Only the actual type is allowed.
	 * However, the following is valid:
	 * \code
	 * Value<int> i(5);
	 * float f=i;
	 * \endcode
	 * In this case the function returns int which is then also implicitely converted to float.
	 * \return a reference to the stored value
	 */
	operator TYPE &() {return m_val;}

	/**
	 * Check if the value of this is greater than ref converted to TYPE.
	 * The function tries to convert ref to the type of this and compare the result.
	 * If there is no conversion an error is send to the debug logging, and false is returned.
	 * \retval value_of_this>converted_value_of_ref if the conversion was successfull
	 * \retval true if the conversion failed because the value of ref was to low for TYPE (negative overflow)
	 * \retval false if the conversion failed because the value of ref was to high for TYPE (positive overflow)
	 * \retval false if there is no know conversion from ref to TYPE
	 */
	bool gt( const _internal::ValueBase &ref )const {
		static const _internal::type_greater<TYPE, boost::is_arithmetic<TYPE>::value > greater;
		return greater.operator()( *this, ref );
	}

	/**
	 * Check if the value of this is less than ref converted to TYPE.
	 * The funkcion tries to convert ref to the type of this and compare the result.
	 * If there is no conversion an error is send to the debug logging, and false is returned.
	 * \retval value_of_this<converted_value_of_ref if the conversion was successfull
	 * \retval false if the conversion failed because the value of ref was to low for TYPE (negative overflow)
	 * \retval true if the conversion failed because the value of ref was to high for TYPE (positive overflow)
	 * \retval false if there is no know conversion from ref to TYPE
	 */
	bool lt( const _internal::ValueBase &ref )const {
		static const _internal::type_less<TYPE, boost::is_arithmetic<TYPE>::value > less;
		return less( *this, ref );
	}

	/**
	 * Check if the value of this is equal to ref converted to TYPE.
	 * The funktion tries to convert ref to the type of this and compare the result.
	 * If there is no conversion an error is send to the debug logging, and false is returned.
	 * \retval value_of_this==converted_value_of_ref if the conversion was successfull
	 * \retval false if the conversion failed because the value of ref was to low for TYPE (negative overflow)
	 * \retval false if the conversion failed because the value of ref was to high for TYPE (positive overflow)
	 * \retval false if there is no know conversion from ref to TYPE
	 */
	bool eq( const _internal::ValueBase &ref )const {
		static const _internal::type_eq<TYPE, boost::is_arithmetic<TYPE>::value > equal;
		return equal( *this, ref );
	}

	virtual ~Value() {}
};

template<typename T> const util::Value<T>& _internal::ValueBase::castToType() const
{
	checkType<T>();
	return m_cast_to<util::Value<T> >();
}
template<typename T> const T &_internal::ValueBase::castTo() const
{
	const util::Value<T> &ret = castToType<T>();
	return ret.operator const T & ();
}
template<typename T> util::Value<T>& _internal::ValueBase::castToType()
{
	checkType<T>();
	return m_cast_to<util::Value<T> >();
}
template<typename T> T &_internal::ValueBase::castTo()
{
	util::Value<T> &ret = castToType<T>();
	return ret.operator T & ();
}

template<typename T> bool _internal::ValueBase::is()const
{
	checkType<T>();
	return getTypeID() == util::Value<T>::staticID;
}

}
/// @}
}

#endif //DATATYPE_INC
