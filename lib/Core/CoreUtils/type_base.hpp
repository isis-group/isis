//
// C++ Interface: type_base
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef ISISTYPE_BASE_HPP
#define ISISTYPE_BASE_HPP

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include "types.hpp"
#include "type_converter.hpp"
#include "generic_type.hpp"
#include "common.hpp"


/*! \addtogroup util
*  Additional documentation for group `mygrp'
*  @{
*/
namespace isis
{
namespace util
{

namespace _internal
{
/// @cond _hidden
template<typename TYPE> struct __cast_to {
	template<typename SOURCE> TYPE operator()( Value<TYPE>*, const SOURCE &value ) {
		return boost::lexical_cast<TYPE>( value ); //generic version types are different - so do lexical cast
	}
	TYPE operator()( Value<TYPE>*, const TYPE &value ) {
		return value; //special version types are same - so just return the value
	}
};
template<> struct __cast_to<uint8_t> { // we cannot lexical_cast to uint8_t - we'll get "characters" (1 => '1' == 49)
	template<typename SOURCE> uint8_t operator()( Value<uint8_t>*, const SOURCE &value ) {
		// have to check by hand because the lexical cast will only check against unsigned short
		if( value > std::numeric_limits<uint8_t>::max() ) {
			throw boost::bad_lexical_cast( typeid( SOURCE ), typeid( uint8_t ) );
		}

		// lexical cast to unsigned short and then static_cast to uint8_t
		return static_cast<uint8_t>( boost::lexical_cast<unsigned short>( value ) );
	}
	uint8_t operator()( Value<uint8_t> *, const uint8_t &value ) {
		return value; //special version types are same - so just return the value
	}
};
/// @endcond

/*
 * This is the mostly abstract base class for all scalar values (see types.hpp).
 * Additionally, there's the ValuePtrBase containing the more complex data handling stuff with abstract values.
 * Both are derived from GenericValue containing the description of the actual value type.
 */

class ValueBase : public GenericValue
{
	static const ValueConverterMap &converters();
	friend class ValueReference<ValueBase>;
protected:
	/**
	* Create a copy of this.
	* Creates a new Value/ValuePtr an stores a copy of its value there.
	* Makes ValueBase-pointers copyable without knowing their type.
	* \returns a ValueBase-pointer to a newly created Value/ValuePtr.
	*/
	virtual ValueBase *clone()const = 0;
public:
	typedef ValueReference<ValueBase> Reference;
	typedef ValueConverterMap::mapped_type::mapped_type Converter;

	template<typename T> bool is()const;

	const Converter &getConverterTo( unsigned short ID )const;

	/**
	 * Convert the content of one Value to another.
	 * This will use the automatic conversion system to transform the value one Value-Object into another.
	 * The types of both objects can be unknown.
	 * \param from the Value-object containing the value which should be converted
	 * \param to the Value-object which will contain the converted value if conversion was successfull
	 * \returns false if the conversion failed for any reason, true otherwise
	 */
	static bool convert( const ValueBase &from, ValueBase &to );

	/**
	* Interpret the value as value of any (other) type.
	* This is a runtime-based cast via automatic conversion.
	* \code
	* ValueBase *mephisto=new Value<std::string>("666");
	* int devil=mephisto->as<int>();
	* \endcode
	* If you know the type of source and destination at compile time you should use Value\<DEST_TYPE\>((SOURCE_TYPE)src).
	* \code
	* Value<std::string> mephisto("666");
	* Value<int> devil((std::string)devil);
	* \endcode
	* \return value of any requested type parsed from toString(false).
	*/
	template<class T> T as()const {
		if( is<T>() )
			return castTo<T>();

		Reference ret = copyToNewByID( Value<T>::staticID );

		if ( ret.isEmpty() ) {
			LOG( Debug, error )
					<< "Interpretation of " << toString( true ) << " as " << Value<T>::staticName()
					<< " failed. Returning " << Value<T>().toString() << ".";
			return T();
		} else
			return ret->castTo<T>();
	}

	/**
	 * Dynamically cast the ValueBase down to its actual Value\<T\>. Constant version.
	 * Will throw std::bad_cast if T is not the actual type.
	 * Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	 * \returns a constant reference of the Value\<T\> object.
	 */
	template<typename T> const Value<T>& castToType() const;

	/**
	 * Dynamically cast the ValueBase up to its actual value of type T. Constant version.
	 * Will throw std::bad_cast if T is not the actual type.
	 * Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	 * \returns a constant reference of the stored value.
	 */
	template<typename T> const T &castTo() const;

	/**
	 * Dynamically cast the ValueBase up to its actual Value\<T\>. Referenced version.
	 * Will throw std::bad_cast if T is not the actual type.
	 * Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	 * \returns a reference of the Value\<T\> object.
	 */
	template<typename T> Value<T>& castToType();

	/**
	 * Dynamically cast the ValueBase up to its actual value of type T. Referenced version.
	 * Will throw std::bad_cast if T is not the actual type.
	 * Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	 * \returns a reference of the stored value.
	 */
	template<typename T> T &castTo();

	/// \returns true if and only if the types of this and second are equal and the values are equal
	virtual bool operator==( const ValueBase &second )const = 0;

	/// creates a copy of the stored value using a type referenced by its id
	Reference copyToNewByID( unsigned short ID ) const;

	/**
	 * Check if the stored value would also fit into another type referenced by its id
	 * \returns true if the stored value would fit into the target type, false otherwise
	 */
	bool fitsInto( unsigned short ID ) const;

	virtual ~ValueBase();

	virtual bool gt( const _internal::ValueBase &ref )const = 0;
	virtual bool lt( const _internal::ValueBase &ref )const = 0;
	virtual bool eq( const _internal::ValueBase &ref )const = 0;
};

}

typedef _internal::ValueBase::Reference ValueReference;

}
}

namespace std
{
/// Streaming output for Value - classes
template<typename charT, typename traits> basic_ostream<charT, traits>&
operator<<( basic_ostream<charT, traits> &out, const isis::util::_internal::GenericValue &s )
{
	return out << s.toString();
}
/// /// Streaming output for Value referencing classes
template<typename charT, typename traits, typename TYPE_TYPE> basic_ostream<charT, traits>&
operator<<( basic_ostream<charT, traits> &out, const isis::util::_internal::ValueReference<TYPE_TYPE> &s )
{
	return out << s.toString( true );
}
}

/// }@

#endif //ISISTYPE_BASE_HPP
