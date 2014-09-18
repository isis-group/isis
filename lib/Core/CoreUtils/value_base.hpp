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

#include <type_traits>
#include <memory>
#include <boost/lexical_cast.hpp>

#include "types.hpp"
#include "value_converter.hpp"
#include "generic_value.hpp"
#include "common.hpp"


namespace isis
{
namespace util
{
API_EXCLUDE_BEGIN;
/// @cond _internal
namespace _internal
{
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
		// lexical cast to unsigned short
		const unsigned short val = boost::lexical_cast<unsigned short>( value );

		// have to check by hand because the lexical cast will only check against unsigned short
		if( val > std::numeric_limits<uint8_t>::max() ) {
			throw boost::bad_lexical_cast( typeid( SOURCE ), typeid( uint8_t ) );
		}

		// and then static_cast to uint8_t
		return static_cast<uint8_t>( val );
	}
	uint8_t operator()( Value<uint8_t> *, const uint8_t &value ) {
		return value; //special version types are same - so just return the value
	}
};
}
/// @endcond
API_EXCLUDE_END;

/*
 * This is the mostly abstract base class for all scalar values (see types.hpp).
 * Additionally, there's the ValueArrayBase containing the more complex data handling stuff with abstract values.
 * Both are derived from GenericValue containing the description of the actual value type.
 */

class ValueBase : public _internal::GenericValue
{
	static const _internal::ValueConverterMap &converters();
	friend class _internal::GenericReference<ValueBase>;
protected:
	/**
	* Create a copy of this.
	* Creates a new Value/ValueArray an stores a copy of its value there.
	* Makes ValueBase-pointers copyable without knowing their type.
	* \returns a ValueBase-pointer to a newly created Value/ValueArray.
	*/
	virtual ValueBase *clone()const = 0;
	ValueBase &operator=(const ValueBase &ref);//prevent direct usage
public:
	/// used for boost::ptr_vector
	struct heap_clone_allocator
	{
		static ValueBase* allocate_clone( const ValueBase& r );
		static void deallocate_clone( const ValueBase* r );
	};

	typedef _internal::GenericReference<ValueBase> Reference;
	typedef _internal::ValueConverterMap::mapped_type::mapped_type Converter;

	/// \return true if the stored type is T
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
	* Value<int> devil((std::string)mephisto);
	* \endcode
	* \return this value converted to the requested type if conversion was successfull.
	*/
	template<class T> T as()const {
		if( is<T>() )
			return castTo<T>();

		Reference ret = copyByID( Value<T>::staticID() );

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
	/// \returns !operator==
	bool operator!=( const ValueBase &second )const;

	/// creates a copy of the stored value using a type referenced by its ID
	Reference copyByID( unsigned short ID ) const;

	/**
	 * Check if the stored value would also fit into another type referenced by its ID
	 * \returns true if the stored value would fit into the target type, false otherwise
	 */
	bool fitsInto( unsigned short ID ) const;

	virtual ~ValueBase();

	/**
	 * Check if the value of this is greater than ref converted to TYPE.
	 * The function tries to convert ref to the type of this and compares the result.
	 * If there is no conversion an error is send to the debug logging, and false is returned.
	 * \retval value_of_this>converted_value_of_ref if the conversion was successfull
	 * \retval true if the conversion failed because the value of ref was to low for TYPE (negative overflow)
	 * \retval false if the conversion failed because the value of ref was to high for TYPE (positive overflow)
	 * \retval false if there is no know conversion from ref to TYPE
	 */
	virtual bool gt( const ValueBase &ref )const = 0;

	/**
	 * Check if the value of this is less than ref converted to TYPE.
	 * The funkcion tries to convert ref to the type of this and compare the result.
	 * If there is no conversion an error is send to the debug logging, and false is returned.
	 * \retval value_of_this<converted_value_of_ref if the conversion was successfull
	 * \retval false if the conversion failed because the value of ref was to low for TYPE (negative overflow)
	 * \retval true if the conversion failed because the value of ref was to high for TYPE (positive overflow)
	 * \retval false if there is no know conversion from ref to TYPE
	 */
	virtual bool lt( const ValueBase &ref )const = 0;

	/**
	 * Check if the value of this is equal to ref converted to TYPE.
	 * The funktion tries to convert ref to the type of this and compare the result.
	 * If there is no conversion an error is send to the debug logging, and false is returned.
	 * \retval value_of_this==converted_value_of_ref if the conversion was successfull
	 * \retval false if the conversion failed because the value of ref was to low for TYPE (negative overflow)
	 * \retval false if the conversion failed because the value of ref was to high for TYPE (positive overflow)
	 * \retval false if there is no known conversion from ref to TYPE
	 */
	virtual bool eq( const ValueBase &ref )const = 0;

	Reference plus( const ValueBase &ref )const;
	Reference minus( const ValueBase &ref )const;
	Reference multiply( const ValueBase &ref )const;
	Reference divide( const ValueBase &ref )const;

	template<typename T> typename std::enable_if<knowType<T>::value,bool>::type apply(const T &val){
		return convert(Value<T>(val),*this);
	}

	virtual ValueBase& add( const ValueBase &ref ) =0;
	virtual ValueBase& substract( const ValueBase &ref ) =0;
	virtual ValueBase& multiply_me( const ValueBase &ref ) = 0;
	virtual ValueBase& divide_me( const ValueBase &ref ) = 0;

	/**
	 * Get the string representation of the Value.
	 * This tries to use the isis type conversion to create a string from the Value.
	 * If thats no available, it will fall back to boost::lexical_cast. And it will send a warning to CoreDebug.
	 * If the lexical cast fails as well, boost::bad_lexical_cast is thrown.
	 * \param labeled if true the typename will be appended to the resulting string in brackets.
	 */
	virtual std::string toString(bool labeled = false) const;
};


typedef ValueBase::Reference ValueReference;

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
operator<<( basic_ostream<charT, traits> &out, const isis::util::_internal::GenericReference<TYPE_TYPE> &s )
{
	return out << s.toString( true );
}
}

#endif //ISISTYPE_BASE_HPP
