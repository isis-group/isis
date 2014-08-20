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
#include <boost/type_traits/conditional.hpp>

namespace isis
{
namespace util
{

template<class TYPE > class Value;

API_EXCLUDE_BEGIN;
/// @cond _internal
namespace _internal
{

/**
 * Generic value operation class.
 * This generic class does nothing, and the ()-operator will allways fail with an error send to the debug-logging.
 * It has to be (partly) specialized for the regarding type.
 */
template<typename OPERATOR,bool modifying,bool enable> struct type_op
{
	typedef typename OPERATOR::result_type result_type;
	typedef boost::integral_constant<bool,enable> enabled;
	typedef typename boost::conditional<modifying,util::Value<typename OPERATOR::first_argument_type>,const util::Value<typename OPERATOR::first_argument_type> >::type lhs;
	
	result_type operator()( lhs &first, const ValueBase &second )const {
		LOG( Debug, error ) << "operator " << typeid(OPERATOR).name() << " is not supportet for " << first.getTypeName()  << " and "<< second.getTypeName();
		throw std::domain_error("operation not available");
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
template<typename OPERATOR,bool modifying> struct type_op<OPERATOR,modifying,true>
{
	typedef typename boost::conditional<modifying,util::Value<typename OPERATOR::first_argument_type>,const util::Value<typename OPERATOR::first_argument_type> >::type lhs;
	typedef typename util::Value<typename OPERATOR::second_argument_type> rhs;
	typedef typename OPERATOR::result_type result_type;
	typedef boost::integral_constant<bool,true> enabled;
	
	virtual result_type posOverflow()const {throw std::domain_error("positive overflow");}
	virtual result_type negOverflow()const {throw std::domain_error("negative overflow");}
	virtual result_type inRange( lhs &first, const rhs &second )const {
		return OPERATOR()(first,second);
	} 
	result_type operator()(lhs &first, const ValueBase &second )const {
		// ask second for a converter from itself to Value<T>
		const ValueBase::Converter conv = second.getConverterTo( rhs::staticID );
		
		if ( conv ) {
			//try to convert second into T and handle results
			rhs buff;
			
			switch ( conv->convert( second, buff ) ) {
				case boost::numeric::cPosOverflow:return posOverflow();
				case boost::numeric::cNegOverflow:return negOverflow();
				case boost::numeric::cInRange:
					LOG_IF(second.isFloat() && second.as<float>()!=static_cast<ValueBase&>(buff).as<float>(), Debug,warning) //we can't really use Value<T> yet, so make it ValueBase
					<< "Using " << second.toString( true ) << " as " << buff.toString( true ) << " for operation on " << first.toString( true )
					<< " you might loose precision";
					return inRange( first, buff );
			}
		}
		throw std::domain_error(rhs::staticName()+" not convertible to "+second.getTypeName());
	}
};

// compare operators (overflows are no error here)
template<typename OPERATOR,bool enable> struct type_comp_base : type_op<OPERATOR,false,enable>{
	typename OPERATOR::result_type posOverflow()const {return false;}
	typename OPERATOR::result_type negOverflow()const {return false;}
};
template<typename T> struct type_eq   : type_comp_base<std::equal_to<T>,true>{};
template<typename T> struct type_less : type_comp_base<std::less<T>,    has_op<T>::lt>
{
	//getting a positive overflow when trying to convert second into T, obviously means first is less 
	typename std::less<T>::result_type posOverflow()const {return true;}
};
template<typename T> struct type_greater : type_comp_base<std::greater<T>,has_op<T>::gt>
{
	//getting an negative overflow when trying to convert second into T, obviously means first is greater
	typename std::greater<T>::result_type negOverflow()const {return true;}
};

// on-self operations .. we return void because the result won't be used anyway
template<typename OP> struct op_base : std::binary_function <typename OP::first_argument_type,typename OP::second_argument_type,void>{};
template<typename T> struct plus_op :  op_base<std::plus<T> >      {void operator() (typename std::plus<T>::first_argument_type& x,       typename std::plus<T>::second_argument_type const& y)       const {x+=y;}};
template<typename T> struct minus_op : op_base<std::minus<T> >     {void operator() (typename std::minus<T>::first_argument_type& x,      typename std::minus<T>::second_argument_type const& y)      const {x-=y;}};
template<typename T> struct mult_op :  op_base<std::multiplies<T> >{void operator() (typename std::multiplies<T>::first_argument_type& x, typename std::multiplies<T>::second_argument_type const& y) const {x*=y;}};
template<typename T> struct div_op :   op_base<std::divides<T> >   {void operator() (typename std::divides<T>::first_argument_type& x,    typename std::divides<T>::second_argument_type const& y)    const {x/=y;}};

template<typename T> struct type_plus :  type_op<plus_op<T>,true, has_op<T>::plus>{};
template<typename T> struct type_minus : type_op<minus_op<T>,true,has_op<T>::minus>{};
template<typename T> struct type_mult :  type_op<mult_op<T>,true, has_op<T>::mult>{};
template<typename T> struct type_div :   type_op<div_op<T>,true,  has_op<T>::div>{};

}
/// @endcond _internal
API_EXCLUDE_END;

/**
 * Generic class for type aware variables.
 * This generic approach makes it possible to handle all the types of Properties for the different
 * data these library can handle. On the other side it's more complex to read and write with these kind of types.
 * \note For supported types see types.hpp
 * \note For type conversion see type_converter.cpp
 */

template<typename TYPE> class Value: public ValueBase
{
	TYPE m_val;
	static const char m_typeName[];
	template<typename OP, typename RET> RET operatorWrapper(const OP& op,const ValueBase &rhs,const RET &default_ret)const{
		try{
			return op(*this,rhs);
		} catch(const std::domain_error &e){ // return default value on failure
			LOG(Runtime,error) << "Operation " << MSubject( typeid(OP).name() ) << " on " << MSubject( getTypeName() ) << " and " << MSubject( rhs.getTypeName() ) << " failed with \"" << e.what()
			<< "\", will return " << MSubject( Value<RET>(default_ret).toString(true) );
		return default_ret;
		}
	}
	template<typename OP> ValueBase& operatorWrapper_me(const OP& op,const ValueBase &rhs){
		try{
			op(*this,rhs);
		} catch(const std::domain_error &e){
			LOG(Runtime,error) << "Operation " << MSubject( typeid(OP).name() ) << " on " << MSubject( getTypeName() ) << " and " << MSubject( rhs.getTypeName() ) << " failed with \"" << e.what()
			<< "\", wont change value (" << MSubject( this->toString(true) ) << ")";
		} 
		return *this;
	}
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

	bool gt( const ValueBase &ref )const {return operatorWrapper(_internal::type_greater<TYPE>(),ref,false );}
	bool lt( const ValueBase &ref )const {return operatorWrapper(_internal::type_less<TYPE>(),   ref,false );}
	bool eq( const ValueBase &ref )const {return operatorWrapper(_internal::type_eq<TYPE>(),     ref, false );}

	ValueBase& add( const ValueBase &ref )        {return operatorWrapper_me(_internal::type_plus<TYPE>(), ref );}
	ValueBase& substract( const ValueBase &ref )  {return operatorWrapper_me(_internal::type_minus<TYPE>(),ref );}
	ValueBase& multiply_me( const ValueBase &ref ){return operatorWrapper_me(_internal::type_mult<TYPE>(), ref );}
	ValueBase& divide_me( const ValueBase &ref )  {return operatorWrapper_me(_internal::type_div<TYPE>(),  ref);}
	
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
