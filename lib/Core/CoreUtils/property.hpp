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

#include <boost/ptr_container/ptr_vector.hpp>
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
class PropertyValue
{
	bool m_needed;
	boost::ptr_vector<ValueBase,ValueBase::heap_clone_allocator> container;
public:
	typedef boost::ptr_vector<ValueBase,ValueBase::heap_clone_allocator>::iterator iterator;
	typedef boost::ptr_vector<ValueBase,ValueBase::heap_clone_allocator>::const_iterator const_iterator;
	typedef boost::ptr_vector<ValueBase,ValueBase::heap_clone_allocator>::reference reference;
	typedef boost::ptr_vector<ValueBase,ValueBase::heap_clone_allocator>::const_reference const_reference;
	typedef PropertyValue value_type;
	/**
	 * Explicit constructor.
	 * Creates a property and stores a value from any known type.
	 * If the type is not known (there is no Value\<type\> available) an compiler error will be raised.
	 * \param ref the value to be stored
	 * \param _needed flag if this PropertyValue is needed an thus not allowed to be empty (a.k.a. undefined)
	 */
	template<typename T> explicit PropertyValue( const T &ref, bool _needed = false ):m_needed( _needed ),container(1) {
		container.push_back(new Value<T>( ref ));
	}
	/// Create a property and store the given single value object.
	template<typename T> PropertyValue( const Value<T> &ref, bool _needed = false ):m_needed( _needed ),container(1) {push_back(ref );}
	/// Create a property and store the given single value object.
	PropertyValue( const ValueBase& ref, bool _needed = false ):m_needed( _needed ),container(1) {push_back(ref);}
    virtual ~PropertyValue(){}

	////////////////////////////////////////////////////////////////////////////
	// List operations
	////////////////////////////////////////////////////////////////////////////
	void push_back(const PropertyValue& ref);
	void push_back(const ValueBase& ref);
	template<typename T> typename std::enable_if<knowType<T>::value >::type push_back(const T& ref){insert(end(),ref);}

	iterator insert(iterator at,const ValueBase& ref);
	void insert(iterator at,const PropertyValue& ref);
	
	template<typename T> typename std::enable_if<knowType<T>::value, iterator >::type insert(iterator at,const T& ref){
		LOG_IF(!isEmpty() && getTypeID()!=Value<T>::staticID(),Debug,error) << "Inserting inconsistent type " << MSubject(Value<T>(ref).toString(true)) << " in " << MSubject(*this);
		return container.insert(at,new Value<T>(ref));
	}

	iterator erase( size_t at );
	
	template<typename InputIterator> void insert( iterator position, InputIterator first, InputIterator last ){container.insert(position,first,last);}
	iterator erase( iterator first, iterator last );
	
	void reserve(size_t size);
	void resize( size_t size, const ValueBase& clone );
	template<typename T> typename std::enable_if<knowType<T>::value,ValueBase& >::type set(size_t idx,const T& val){
		if(size()<=idx)
			resize(idx,Value<T>());
		return *container.replace(idx,new Value<T>(val));
	}

	ValueBase&        operator[]( size_t n );
	const ValueBase&  operator[]( size_t n ) const;
	ValueBase&        at( size_t n );
	const ValueBase&  at( size_t n ) const;
	ValueBase&        front();
	const ValueBase&  front()const;

	iterator begin();
	const_iterator begin()const;

	iterator end();
	const_iterator end()const;

	/**
	 * Splice up the property value.
	 * Distribute entries into new PropertyValues of given equal length.
	 * \note the last PropertyValue may have less entries (aka remainder)
	 * \note this is a transfer function, so *this will be empty afterwards.
	 * \returns a vector of (mostly) equally sized PropertyValues.
	 */
	std::vector<PropertyValue> splice(const size_t len);

	/// Amount of values in this PropertyValue
	size_t size()const;
	
	/// Copy a list of ValueReference into the PropertyValue.
	template<typename ITER> void copy(ITER first,ITER last){
		while(first!=last)
			push_back(**(++first));
	}
	/**
	 * Transfer properties from one PropertyValue into another.
	 * The transfered data will be inserted at idx.
	 * The properties in the target will not be removed. Thus the PropertyValue will grow.
	 * The source will be empty afterwards.
	 */
	void transfer(iterator idx,PropertyValue &ref);
	/**
	 * Transfer properties from one PropertyValue to another.
	 * The transfered data will replace the properties in the target.
	 * The source will be empty afterwards.
	 */
	void transfer(PropertyValue &src);

	/// Swap properties from one PropertyValue with another.
	void swap(PropertyValue &src);

	/// Transform all contained properties into type T
	bool transform( uint16_t dstID );
	template<typename T> bool transform(){return transform(Value<T>::staticID());}
	
	/**
	 * Empty constructor.
	 * Creates an empty property value. So PropertyValue().isEmpty() will allways be true.
	 */
	PropertyValue();
	/**
	 * Copy operator.
	 * Copies the content of another Property.
	 * \param other the source to copy from
	 * \note the needed state wont change, regardless of what it is in other
	 */
	PropertyValue &operator=(const PropertyValue &other);
	
	/// accessor to mark as (not) needed
	bool &needed();
	/// returns true if PropertyValue is marked as needed, false otherwise
	bool isNeeded ()const;

	/**
	 * Equality to another PropertyValue.
	 * Properties are ONLY equal if:
	 * - both properties are not empty
	 * - both properties contain the same amount of values
	 * - \link Value::operator== \endlink is true for all stored values
	 * - (which also means equal types)
	 * \note Empty properties are neither equal nor unequal
	 * \returns true if both contain the same values of the same type, false otherwise.
	 */
	bool operator ==( const PropertyValue &second )const;
	/**
	 * Unequality to another PropertyValue.
	 * Properties are unequal if:
	 * - they have different amounts of values stored
	 * - or operator!= is true on any stored value
	 * \note Empty properties are neither equal nor unequal
	 * \returns false if operator!= is not true for all stored value and amount of values is equal
	 * \returns true otherwise
	 */
	bool operator !=( const PropertyValue &second )const;
	/**
	 * Equality to another Value-Object
	 * \returns Value::operator== if the property has exactly one value, false otherwise.
	 */
	bool operator ==( const ValueBase &second )const;
	/**
	 * Unequality to another Value-Object 
	 * \returns Value::operator!= if the property has exactly one value, false otherwise.
	 */
	bool operator !=( const ValueBase &second )const;
	
	/**
	 * (re)set property to one specific value of a specific type
	 * \note The needed flag won't be affected by that.
	 * \note To prevent accidential use this can only be used explicetly. \code util::PropertyValue propA; propA=5; \endcode is valid. But \code util::PropertyValue propA=5; \endcode is not,
	 */
	template<typename T> typename std::enable_if<knowType<T>::value,PropertyValue&>::type operator=( const T &ref){
		container.clear();
		container.push_back(new Value<T>(ref));
		return *this;
	}
	PropertyValue& operator=( const ValueBase &ref){
		container.clear();
		push_back(ref);
		return *this;
	}

	/**
	 * Explicit cast to ValueReference.
	 * \returns A ValueReference containing a copy of the first stored Value.
	 * \note Applies only on the first value. Other Values are ignored (use the []-operator to access them).
	 * \note An exception is thrown if the PropertyValue is empty.
	 */
	ValueReference operator()()const;
	
	/**
	 * creates a copy of the stored values using a type referenced by its ID
	 * \returns a new PropertyValue containing all values converted to the requested type
	 */
	PropertyValue copyByID( unsigned short ID ) const;

	/// \returns the value(s) represented as text.
	virtual std::string toString( bool labeled = false )const;

	/// \returns true if, and only if no value is stored
	bool isEmpty()const;
	
	
	////////////////////////////////////////////////////////////////////////////
	// ValueBase interface
	////////////////////////////////////////////////////////////////////////////

	/**
	 * \copybrief ValueBase::as
	 * hook for \link ValueBase::as \endlink called on the first value
	 * \note Applies only on the first value. Other Values are ignored (use the []-operator to access them).
	 * \note An exception is thrown if the PropertyValue is empty.
	 */
	template<class T> T as()const {return front().as<T>();}

	/**
	 * \copybrief ValueBase::is
	 * hook for \link ValueBase::is \endlink
	 * \note Applies only on the first value. Other Values are ignored (use the []-operator to access them).
	 * \note An exception is thrown if the PropertyValue is empty.
	 */
	template<class T> bool is()const {return container.front().is<T>();}

	/**
	 * \copybrief ValueBase::getTypeName
	 * hook for \link ValueBase::getTypeName \endlink
	 * \note Applies only on the first value. Other Values are ignored (use the []-operator to access them).
	 * \note An exception is thrown if the PropertyValue is empty.
	 */
	std::string getTypeName()const;

	/**
	 * \copybrief ValueBase::getTypeID
	 * hook for \link ValueBase::getTypeID \endlink
	 * \note Applies only on the first value. Other Values are ignored (use the []-operator to access them).
	 * \note An exception is thrown if the PropertyValue is empty.
	 */
	unsigned short getTypeID()const;

	/**
	 * \copybrief ValueBase::castTo
	 * hook for \link ValueBase::castTo \endlink
	 * \note Applies only on the first value. Other Values are ignored (use the []-operator to access them).
	 * \note An exception is thrown if the PropertyValue is empty.
	 */
	template<typename T> T &castTo(){return front().castTo<T>();}
	template<typename T> const T &castTo() const{return front().castTo<T>();}

	/**
	 * \copybrief ValueBase::castToType
	 * hook for \link ValueBase::castToType \endlink
	 * \note Applies only on the first value. Other Values are ignored (use the []-operator to access them).
	 * \note An exception is thrown if the PropertyValue is empty.
	 */
	template<typename T> Value<T>& castToType(){return front().castToType<T>();}
	template<typename T> const Value<T>& castToType() const{return front().castToType<T>();}
	
	/**
	 * \returns true if \link ValueBase::fitsInto \endlink is true for all values
	 * \note Operation is done on all values. For comparing single values access them via the []-operator.
	 */
	bool fitsInto( unsigned short ID ) const;
	
	/**
	 * \returns true if \link ValueBase::gt \endlink is true for all values
	 * \note Operation is done on all values. For working on single values access them via the []-operator.
	 * \note An exception is thrown if this has less values than the target (the opposite case is ignored).
	 */
	bool gt( const PropertyValue &ref )const;
	/**
	 * \returns true if \link ValueBase::lt \endlink is true for all values
	 * \copydetails PropertyValue::gt
	 */
	bool lt( const PropertyValue &ref )const;
	/**
	 * \returns true if \link ValueBase::eq \endlink is true for all values
	 * \copydetails PropertyValue::gt
	 */
	bool eq( const PropertyValue &ref )const;
	
	/**
	 * \returns a PropertyValue with the results of \link ValueBase::plus \endlink done on all value pairs from this and the target
	 * \copydetails PropertyValue::gt
	 */
	PropertyValue plus( const PropertyValue &ref )const;
	/**
	 * \returns a PropertyValue with the results of \link ValueBase::minus \endlink done on all value pairs from this and the target
	 * \copydetails PropertyValue::gt
	 */
	PropertyValue minus( const PropertyValue &ref )const;
	/**
	 * \returns a PropertyValue with the results of \link ValueBase::multiply \endlink done on all value pairs from this and the target
	 * \copydetails PropertyValue::gt
	 */
	PropertyValue multiply( const PropertyValue &ref )const;
	/**
	 * \returns a PropertyValue with the results of \link ValueBase::divide \endlink done on all value pairs from this and the target
	 * \copydetails PropertyValue::gt
	 */
	PropertyValue divide( const PropertyValue &ref )const;
	
	/**
	 * \copydetails PropertyValue::gt
	 */
	PropertyValue& add( const PropertyValue &ref );
	/**
	 * \copydetails PropertyValue::gt
	 */
	PropertyValue& substract( const PropertyValue &ref );
	/**
	 * \copydetails PropertyValue::gt
	 */
	PropertyValue& multiply_me( const PropertyValue &ref );
	/**
	 * \copydetails PropertyValue::gt
	 */
	PropertyValue& divide_me( const PropertyValue &ref );

	////////////////////////////////////////////////////////////////////////////
	// operators on "normal" values
	////////////////////////////////////////////////////////////////////////////
	/**
	 * Equality to a basic value.
	 * Properties equal to basic values if:
	 * - the property contains exactly one value
	 * - \link Value::eq \endlink is true for that value
	 * \warning This is using the more fuzzy Value::eq. So the type won't be compared and rounding might be done (which will send a warning to Debug).
	 * \returns front().eq(second) if the property contains exactly one value, false otherwise
	 */
	template<typename T> typename std::enable_if<knowType<T>::value,bool>::type operator ==( const T &second )const{return size()==1 && front().eq(Value<T>(second));}
	/**
	 * Unequality to a basic value.
	 * Properties are unequal to basic values if:
	 * - the property contains exactly one value
	 * - \link Value::eq \endlink is false for that value
	 * \warning This is using the more fuzzy Value::eq. So the type won't be compared and rounding might be done (which will send a warning to Debug).
	 * \returns !front().eq(second) if the property contains exactly one value, false otherwise
	 */
	template<typename T> typename std::enable_if<knowType<T>::value,bool>::type operator !=( const T &second )const{return size()==1 && !front().eq(Value<T>(second));}
	
	template<typename T> PropertyValue& operator +=( const T &second ){front().add(Value<T>(second));return *this;}
	template<typename T> PropertyValue& operator -=( const T &second ){front().substract(Value<T>(second));return *this;}
	template<typename T> PropertyValue& operator *=( const T &second ){front().multiply_me(Value<T>(second));return *this;}
	template<typename T> PropertyValue& operator /=( const T &second ){front().divide_me(Value<T>(second));return *this;}

	template<typename T> PropertyValue operator+( const T &rhs )const {PropertyValue lhs(*this); return lhs+=rhs;}
	template<typename T> PropertyValue operator-( const T &rhs )const {PropertyValue lhs(*this); return lhs-=rhs;}
	template<typename T> PropertyValue operator*( const T &rhs )const {PropertyValue lhs(*this); return lhs*=rhs;}
	template<typename T> PropertyValue operator/( const T &rhs )const {PropertyValue lhs(*this); return lhs/=rhs;}
	
};

}
}

namespace std
{
	/// streaming output for PropertyValue
	template<typename charT, typename traits>
	basic_ostream<charT, traits>& operator<<( basic_ostream<charT, traits> &out, const isis::util::PropertyValue &s )
	{
		return out<<s.toString(true);
	}
	template<> void swap<isis::util::PropertyValue>(isis::util::PropertyValue &a,isis::util::PropertyValue &b);
}
#endif


