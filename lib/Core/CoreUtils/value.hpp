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

#include "value_base.hpp"

#include <string>
#include <functional>
#include <boost/type_traits/is_float.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/has_operator.hpp>

namespace isis
{
namespace util
{

template<class TYPE > class Value;

API_EXCLUDE_BEGIN
/// @cond _internal
namespace _internal
{

/**
 * Generic value operation class.
 * This generic class does nothing, and the ()-operator will allways fail with an error send to the debug-logging.
 * It has to be (partly) specialized for the regarding type.
 */
template<typename OPERATOR,bool enable> struct type_op
{
	typedef typename OPERATOR::result_type result_type;
	typedef boost::integral_constant<bool,enable> enabled;
	
	result_type operator()( const util::Value<typename OPERATOR::first_argument_type> &first, const ValueBase &second )const {
		LOG( Debug, error ) << "operator " << typeid(OPERATOR).name() << " is not supportet for " << first.getTypeName()  << " and "<< second.getTypeName();
		throw std::domain_error("operation not available");
		return result_type();
	}
};

/**
 * Half-generic value operation class.
 * This generic class does math operations on Values by converting the second Value-object to the type of the first Value-object. Then:
 * - if the conversion was successfull (the second value can be represented in the type of the first) the "inRange"-operation is used
 * - if the conversion failed with an positive or negative overflow (the second value is to high/low to fit into the type of the first) a info sent to the debug-logging and the posOverflow/negOverflow operation is used
 * - if there is no known conversion from second to first an error is sent to the debug-logging and std::domain_error is thrown.
 * \note The functions (posOverflow,negOverflow) here are only stubs and will allways throw std::domain_error.
 * \note inRange will return OPERATOR()(first,second)
 * These class can be further specialized for the regarding operation.
 */
template<typename OPERATOR> struct type_op<OPERATOR,true>
{
	typedef typename util::Value<typename OPERATOR::first_argument_type>  lhs;
	typedef typename util::Value<typename OPERATOR::second_argument_type> rhs;
	typedef typename OPERATOR::result_type result_type;
	typedef boost::integral_constant<bool,true> enabled;
	
	virtual result_type posOverflow( const lhs &first, const rhs &/*second*/ )const {throw std::domain_error("positive overflow"); return result_type();}
	virtual result_type negOverflow( const lhs &first, const rhs &/*second*/ )const {throw std::domain_error("negative overflow"); return result_type();}
	virtual result_type inRange( const lhs &first, const rhs &second )const {
		return OPERATOR()(first,second);
	} 
	result_type operator()(const lhs &first, const ValueBase &second )const {
		// ask second for a converter from itself to Value<T>
		const ValueBase::Converter conv = second.getConverterTo( rhs::staticID );
		
		if ( conv ) {
			//try to convert second into T and handle results
			rhs buff;
			
			switch ( conv->convert( second, buff ) ) {
				case boost::numeric::cPosOverflow:
					LOG( Debug, info ) << "Positive overflow when converting " << second.toString( true ) << " to " << rhs::staticName() << ".";
					return posOverflow( first, buff );
				case boost::numeric::cNegOverflow:
					LOG( Debug, info ) << "Negative overflow when converting " << second.toString( true ) << " to " << rhs::staticName() << ".";
					return negOverflow( first, buff );
				case boost::numeric::cInRange:
					LOG_IF(second.isFloat() && second.as<float>()!=static_cast<ValueBase&>(buff).as<float>(), Debug,warning) //we can't really use Value<T> yet, so make it ValueBase
					<< "Using " << second.toString( true ) << " as " << buff.toString( true ) << " for operation on " << first.toString( true )
					<< " you might loose precision";
					return inRange( first, buff );
			}
		} else {
			LOG( Debug, error ) << "No conversion of " << second.getTypeName() << " to " << rhs::staticName() << " available";
			throw std::domain_error(rhs::staticName()+" not convertible to "+second.getTypeName());
		}
		return result_type();
	}
};

/// equal comparison
template<typename T> struct type_plus : type_op<std::plus<T>,boost::has_plus<T>::value>{};
template<typename T> struct type_minus : type_op<std::minus<T>,boost::has_minus<T>::value>{};
template<typename T> struct type_mult : type_op<std::multiplies<T>,boost::has_multiplies<T>::value>{};
template<typename T> struct type_div : type_op<std::divides<T>,boost::has_divides<T>::value>{};
template<typename T> struct type_eq : type_op<std::equal_to<T>,boost::has_equal_to<T>::value>{};

template<> struct type_plus<boost::gregorian::date> : type_op<std::plus<boost::gregorian::date>,false>{};
template<> struct type_minus<boost::gregorian::date> : type_op<std::minus<boost::gregorian::date>,false>{};
template<> struct type_mult<boost::gregorian::date> : type_op<std::multiplies<boost::gregorian::date>,false>{};

template<> struct type_plus<boost::posix_time::ptime> : type_op<std::plus<boost::posix_time::ptime>,false>{};
template<> struct type_minus<boost::posix_time::ptime> : type_op<std::minus<boost::posix_time::ptime>,false>{};
template<> struct type_mult<boost::posix_time::ptime> : type_op<std::multiplies<boost::posix_time::ptime>,false>{};

/// less-than comparison (overrides posOverflow)
template<typename T> struct type_less : type_op<std::less<T>,boost::has_less<T>::value>
{
	//getting a positive overflow when trying to convert second into T, obviously means first is less and negative means not less
	typename std::less<T>::result_type posOverflow( const util::Value<T> &/*first*/, const util::Value<T> &/*second*/ )const {return true;}
	typename std::less<T>::result_type negOverflow( const util::Value<T> &/*first*/, const util::Value<T> &/*second*/ )const {return false;}
};

/// greater-than comparison (overrides negOverflow)
template<typename T> struct type_greater : type_op<std::greater<T>,boost::has_greater<T>::value>
{
	//getting an negative overflow when trying to convert second into T, obviously means first is greater and a positive overflow means it's not
	typename std::greater<T>::result_type posOverflow( const util::Value<T> &/*first*/, const util::Value<T> &/*second*/ )const {return false;}
	typename std::greater<T>::result_type negOverflow( const util::Value<T> &/*first*/, const util::Value<T> &/*second*/ )const {return true;}
};

}
/// @endcond _internal
API_EXCLUDE_END

/**
 * Generic class for type aware variables.
 * This generic approach makes it possible to handle all the types of Properties for the different
 * data these library can handle. On the other side it's more complex to read and write with these kind of types.
 * Look carefully at further comments on functionality and examples in use, e.g. with PropertyValue.
 * \note For supported types see types.hpp
 * \note For type conversion see type_converter.cpp
 */

template<typename TYPE> class Value: public ValueBase
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
		BOOST_STATIC_ASSERT( staticID < 0xFF );
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
		BOOST_STATIC_ASSERT( staticID < 0xFF );
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

	bool gt( const ValueBase &ref )const {return _internal::type_greater<TYPE>()( *this, ref );}
	bool lt( const ValueBase &ref )const {return _internal::type_less<TYPE>()( *this, ref );}
	bool eq( const ValueBase &ref )const {return _internal::type_eq<TYPE>()( *this, ref );}

	Reference plus( const ValueBase &ref )const {return new Value<TYPE>( _internal::type_plus<TYPE>()( *this, ref ) );}
	Reference minus( const ValueBase &ref )const {return new Value<TYPE>( _internal::type_minus<TYPE>()( *this, ref ) );}
	Reference multiply( const ValueBase &ref )const {return new Value<TYPE>( _internal::type_mult<TYPE>()( *this, ref ) );}
	Reference divide( const ValueBase &ref )const {return new Value<TYPE>( _internal::type_div<TYPE>()( *this, ref ) );}
	
	Reference add( const ValueBase &ref ) {
		const TYPE result=_internal::type_plus<TYPE>()( *this, ref ); //do operation before to trigger error message if its invalid
		if(_internal::type_plus<TYPE>::enabled::value)
			static_cast<TYPE&>(*this) = result;
		return *this;
	}
	Reference substract( const ValueBase &ref ) {
		const TYPE result=_internal::type_minus<TYPE>()( *this, ref ); //do operation before to trigger error message if its invalid
		if(_internal::type_minus<TYPE>::enabled::value)
			static_cast<TYPE&>(*this)=result;
		return *this;
	}
	Reference multiply_me( const ValueBase &ref ) {
		const TYPE result=_internal::type_mult<TYPE>()( *this, ref ); //do operation before to trigger error message if its invalid
		if(_internal::type_mult<TYPE>::enabled::value)
			static_cast<TYPE&>(*this) = result;
		return *this;
	}
	Reference divide_me( const ValueBase &ref ) {
		const TYPE result=_internal::type_div<TYPE>()( *this, ref ); //do operation before to trigger error message if its invalid
		if(_internal::type_div<TYPE>::enabled::value)
			static_cast<TYPE&>(*this)=result;
		return *this;
	}
	
	
	virtual ~Value() {}
};

template<typename T> const util::Value<T>& ValueBase::castToType() const
{
	checkType<T>();
	return m_cast_to<util::Value<T> >();
}
template<typename T> const T &ValueBase::castTo() const
{
	const util::Value<T> &ret = castToType<T>();
	return ret.operator const T & ();
}
template<typename T> util::Value<T>& ValueBase::castToType()
{
	checkType<T>();
	return m_cast_to<util::Value<T> >();
}
template<typename T> T &ValueBase::castTo()
{
	util::Value<T> &ret = castToType<T>();
	return ret.operator T & ();
}

template<typename T> bool ValueBase::is()const
{
	checkType<T>();
	return getTypeID() == util::Value<T>::staticID;
}

}
}

#endif //DATATYPE_INC
