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
	template<typename SOURCE> TYPE operator()( Type<TYPE> *dest, const SOURCE &value ) {
		return boost::lexical_cast<TYPE>( value ); //generic version types are different - so do lexical cast
	}
	TYPE operator()( Type<TYPE> *dest, const TYPE &value ) {
		return value; //special version types are same - so just return the value
	}
};
template<> struct __cast_to<uint8_t> { // we cannot lexical_cast to uint8_t - we'll get "characters" (1 => '1' == 49)
	template<typename SOURCE> uint8_t operator()( Type<uint8_t> *dest, const SOURCE &value ) {
		// have to check by hand because the lexical cast will only check against unsigned short
		if( value > std::numeric_limits<uint8_t>::max() ) {
			throw boost::bad_lexical_cast( typeid( SOURCE ), typeid( uint8_t ) );
		}

		// lexical cast to unsigned short and then static_cast to uint8_t
		return static_cast<uint8_t>( boost::lexical_cast<unsigned short>( value ) );
	}
	uint8_t operator()( Type<uint8_t> *dest, const uint8_t &value ) {
		return value; //special version types are same - so just return the value
	}
};
/// @endcond
	
/*
 * This is the mostly abstract base class for all scalar values (see types.hpp).
 * Additionally, there's the TypePtrBase containing the more complex data handling stuff with abstract values. 
 * Both are derived from GenericType containing the description of the actual value type.
 */

class TypeBase : public GenericType
{
	static const TypeConverterMap &converters();
	friend class TypeReference<TypeBase>;
protected:
	/**
	* Create a copy of this.
	* Creates a new Type/TypePtr an stores a copy of its value there.
	* Makes TypeBase-pointers copyable without knowing their type.
	* \returns a TypeBase-pointer to a newly created Type/TypePtr.
	*/
	virtual TypeBase *clone()const = 0;
public:
	typedef TypeReference<TypeBase> Reference;
	typedef TypeConverterMap::mapped_type::mapped_type Converter;

	template<typename T> bool is()const;

	const Converter &getConverterTo( unsigned short ID )const;

	/**
	 * Convert the content of one Type to another.
	 * This will use the automatic conversion system to transform the value one Type-Object into another.
	 * The types of both objects can be unknown.
	 * \param from the Type-object containing the value which should be converted
	 * \param to the Type-object which will contain the converted value if conversion was successfull
	 * \returns false if the conversion failed for any reason, true otherwise
	 */
	static bool convert( const TypeBase &from, TypeBase &to );

	/**
	* Interpret the value as value of any (other) type.
	* This is a runtime-based cast via automatic conversion.
	* \code
	* TypeBase *mephisto=new Type<std::string>("666");
	* int devil=mephisto->as<int>();
	* \endcode
	* If you know the type of source and destination at compile time you should use Type\<DEST_TYPE\>((SOURCE_TYPE)src).
	* \code
	* Type<std::string> mephisto("666");
	* Type<int> devil((std::string)devil);
	* \endcode
	* \return value of any requested type parsed from toString(false).
	*/
	template<class T> T as()const {
		if( is<T>() )
			return castTo<T>();

		Reference ret = copyToNewByID( Type<T>::staticID );

		if ( ret.isEmpty() ) {
			LOG( Debug, error )
					<< "Interpretation of " << toString( true ) << " as " << Type<T>::staticName()
					<< " failed. Returning " << Type<T>().toString() << ".";
			return T();
		} else
			return ret->castTo<T>();
	}

	/**
	 * Dynamically cast the TypeBase down to its actual Type\<T\>. Constant version.
	 * Will throw std::bad_cast if T is not the actual type.
	 * Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	 * \returns a constant reference of the Type\<T\> object.
	 */
	template<typename T> const Type<T>& castToType() const;

	/**
	 * Dynamically cast the TypeBase up to its actual value of type T. Constant version.
	 * Will throw std::bad_cast if T is not the actual type.
	 * Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	 * \returns a constant reference of the stored value.
	 */
	template<typename T> const T &castTo() const;

	/**
	 * Dynamically cast the TypeBase up to its actual Type\<T\>. Referenced version.
	 * Will throw std::bad_cast if T is not the actual type.
	 * Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	 * \returns a reference of the Type\<T\> object.
	 */
	template<typename T> Type<T>& castToType();

	/**
	 * Dynamically cast the TypeBase up to its actual value of type T. Referenced version.
	 * Will throw std::bad_cast if T is not the actual type.
	 * Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	 * \returns a reference of the stored value.
	 */
	template<typename T> T &castTo();

	/// \returns true if and only if the types of this and second are equal and the values are equal
	virtual bool operator==( const TypeBase &second )const = 0;

	/// creates a copy of the stored value using a type referenced by its id
	Reference copyToNewByID( unsigned short ID ) const;

	/**
	 * Check if the stored value would also fit into another type referenced by its id
	 * \returns true if the stored value would fit into the target type, false otherwise
	 */
	bool fitsInto( unsigned short ID ) const;

	virtual ~TypeBase();

	virtual bool gt( const _internal::TypeBase &ref )const = 0;
	virtual bool lt( const _internal::TypeBase &ref )const = 0;
	virtual bool eq( const _internal::TypeBase &ref )const = 0;
};

}

typedef _internal::TypeBase::Reference TypeReference;

}
}

namespace std
{
/// Streaming output for Type - classes
template<typename charT, typename traits> basic_ostream<charT, traits>&
operator<<( basic_ostream<charT, traits> &out, const isis::util::_internal::GenericType &s )
{
	return out << s.toString();
}
/// /// Streaming output for Type referencing classes
template<typename charT, typename traits, typename TYPE_TYPE> basic_ostream<charT, traits>&
operator<<( basic_ostream<charT, traits> &out, const isis::util::_internal::TypeReference<TYPE_TYPE> &s )
{
	return out << s.toString( true );
}
}

/// }@

#endif //ISISTYPE_BASE_HPP
