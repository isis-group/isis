/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef TYPEPTR_HPP
#define TYPEPTR_HPP

#include "valuearray_base.hpp"
#include "valuearray_converter.hpp"
#include "value.hpp"
#include "common.hpp"
#include "endianess.hpp"

namespace isis
{
namespace data
{

namespace _internal
{
/// @cond _internal
template<typename T, uint8_t STEPSIZE> std::pair<T, T> calcMinMax( const T *data, size_t len )
{
	static_assert( std::numeric_limits<T>::has_denorm != std::denorm_indeterminate, "denormisation not known" ); //well we're pretty f**ed in this case
	std::pair<T, T> result(
		std::numeric_limits<T>::max(),
		std::numeric_limits<T>::has_denorm ? -std::numeric_limits<T>::max() : std::numeric_limits<T>::min() //for types with denormalization min is _not_ the lowest value
	);
	LOG( Runtime, verbose_info ) << "using generic min/max computation for " << util::Value<T>::staticName();

	for ( const T *i = data; i < data + len; i += STEPSIZE ) {
		if(
			std::numeric_limits<T>::has_infinity &&
			( *i == std::numeric_limits<T>::infinity() || *i == -std::numeric_limits<T>::infinity() )
		)
			continue; // skip this one if its inf

		if(std::isnan(*i))
			continue; // skip this one if its NaN
			
		if ( *i > result.second )result.second = *i; //*i is the new max if its bigger than the current (gets rid of nan as well)

		if ( *i < result.first )result.first = *i; //*i is the new min if its smaller than the current (gets rid of nan as well)
	}

	LOG_IF(std::numeric_limits<T>::has_infinity && result.first>result.second,Runtime,warning) 
		<< "Skipped all elements of this array, as they all are inf. Results will be invalid.";

	return result;
}

#ifdef __SSE2__
////////////////////////////////////////////////
// specialize calcMinMax for (u)int(8,16,32)_t /
////////////////////////////////////////////////

template<> std::pair< uint8_t,  uint8_t> calcMinMax< uint8_t, 1>( const  uint8_t *data, size_t len );
template<> std::pair<uint16_t, uint16_t> calcMinMax<uint16_t, 1>( const uint16_t *data, size_t len );
template<> std::pair<uint32_t, uint32_t> calcMinMax<uint32_t, 1>( const uint32_t *data, size_t len );

template<> std::pair< int8_t,  int8_t> calcMinMax< int8_t, 1>( const  int8_t *data, size_t len );
template<> std::pair<int16_t, int16_t> calcMinMax<int16_t, 1>( const int16_t *data, size_t len );
template<> std::pair<int32_t, int32_t> calcMinMax<int32_t, 1>( const int32_t *data, size_t len );
#endif //__SSE2__

API_EXCLUDE_BEGIN;
template<typename T, bool isNumber> struct getMinMaxImpl { // fallback for unsupported types
	std::pair<T, T> operator()( const ValueArray<T> &/*ref*/ ) const {
		LOG( Debug, error ) << "min/max computation of " << util::Value<T>::staticName() << " is not supported";
		return std::pair<T, T>();
	}
};

template<typename T> struct getMinMaxImpl<T, true> { // generic min-max for numbers (this _must_ not be run on empty ValueArray)
	std::pair<T, T> operator()( const ValueArray<T> &ref ) const {
		return calcMinMax<T, 1>( &ref[0], ref.getLength() );
	}
};

template<typename T> struct getMinMaxImpl<util::color<T>, false> { // generic min-max for color (get bounding box in color space)
	std::pair<util::color<T> , util::color<T> > operator()( const ValueArray<util::color<T> > &ref ) const {
		std::pair<util::color<T> , util::color<T> > ret;

		for( uint_fast8_t i = 0; i < 3; i++ ) {
			const std::pair<T, T> buff = calcMinMax<T, 3>( &ref[0].r + i, ref.getLength() * 3 );
			*( &ret.first.r + i ) = buff.first;
			*( &ret.second.r + i ) = buff.second;
		}

		return ret;
	}
};
template<typename T> struct getMinMaxImpl<std::complex<T>, false> { // generic min-max for complex values (get min/max of abs(x))
	std::pair<T, T> operator()( const ValueArray<std::complex<T> > &ref ) const {
		//use compute min/max of magnitute / phase
		auto any_nan= [](const std::complex<T> &v){return std::isnan(v.real()) || std::isnan(v.imag());};
		const auto start=std::find_if_not(std::begin(ref),std::end(ref),any_nan);
		if(start==std::end(ref)){
			LOG(Runtime,error) << "Array is all NaN, returning NaN/NaN as minimum/maximum";
			return std::make_pair(std::numeric_limits<T>::quiet_NaN(),std::numeric_limits<T>::quiet_NaN());
		} else {
			T ret_min_sqmag=std::norm(*start),ret_max_sqmag=std::norm(*start);
			
			for(auto i=start;i!=std::end(ref);i++){
				if(!any_nan(*i)){
					const T &sqmag=std::norm(*i);
					if(ret_min_sqmag>sqmag)ret_min_sqmag=sqmag;
					if(ret_max_sqmag<sqmag)ret_max_sqmag=sqmag;
				}
			}
			return std::make_pair(std::sqrt(ret_min_sqmag),std::sqrt(ret_max_sqmag));
		}
	}
};
/// @endcond
API_EXCLUDE_END;

/**
 * Basic iterator for ValueArray.
 * This is a common iterator following the random access iterator model.
 * It is not part of the reference counting used in ValueArray. So make sure you keep the ValueArray you created it from while you use this iterator.
 */
template<typename TYPE> class ValueArrayIterator: public std::iterator<std::random_access_iterator_tag, TYPE>
{
	TYPE *p;
	typedef typename std::iterator<std::random_access_iterator_tag, TYPE>::difference_type distance;
	friend class ValueArrayIterator<const TYPE>;
public:
	ValueArrayIterator(): p( NULL ) {}
	ValueArrayIterator( TYPE *_p ): p( _p ) {}
	ValueArrayIterator( const ValueArrayIterator<typename std::remove_const<TYPE>::type > &src ): p( src.p ) {}

	ValueArrayIterator<TYPE>& operator++() {++p; return *this;}
	ValueArrayIterator<TYPE>& operator--() {--p; return *this;}

	ValueArrayIterator<TYPE>  operator++( int ) {ValueArrayIterator<TYPE> tmp = *this; ++*this; return tmp;}
	ValueArrayIterator<TYPE>  operator--( int ) {ValueArrayIterator<TYPE> tmp = *this; --*this; return tmp;}

	TYPE &operator*() const { return *p; }
	TYPE *operator->() const { return p; }

	bool operator==( const ValueArrayIterator<TYPE> &cmp )const {return p == cmp.p;}
	bool operator!=( const ValueArrayIterator<TYPE> &cmp )const {return !( *this == cmp );}

	bool operator>( const ValueArrayIterator<TYPE> &cmp )const {return p > cmp.p;}
	bool operator<( const ValueArrayIterator<TYPE> &cmp )const {return p < cmp.p;}

	bool operator>=( const ValueArrayIterator<TYPE> &cmp )const {return p >= cmp.p;}
	bool operator<=( const ValueArrayIterator<TYPE> &cmp )const {return p <= cmp.p;}

	ValueArrayIterator<TYPE> operator+( distance n )const {return ValueArrayIterator<TYPE>( p + n );}
	ValueArrayIterator<TYPE> operator-( distance n )const {return ValueArrayIterator<TYPE>( p - n );}

	distance operator-( const ValueArrayIterator<TYPE> &cmp )const {return p - cmp.p;}

	ValueArrayIterator<TYPE> &operator+=( distance n ) {p += n; return *this;}
	ValueArrayIterator<TYPE> &operator-=( distance n ) {p -= n; return *this;}

	TYPE &operator[]( distance n )const {return *( p + n );}

	operator TYPE*(){return p;}
};

}

/**
 * Generic class for type (and length) - aware pointers.
 * The class is designed for arrays, but you can also "point" to an single element
 * by just use "1" for the length.
 * The pointers are reference counted and will be deleted automatically by a customizable deleter.
 * The copy is cheap, thus the copy of a ValueArray will reference the same data.
 * The usual pointer dereferencing interface ("*", "->" and "[]") is supported.
 */
template<typename TYPE> class ValueArray: public ValueArrayBase
{
	std::shared_ptr<TYPE> m_val;
	static const util::ValueReference getValueFrom( const void *p ) {
		return util::Value<TYPE>( *reinterpret_cast<const TYPE *>( p ) );
	}
	static void setValueInto( void *p, const util::ValueBase &val ) {
		*reinterpret_cast<TYPE *>( p ) = val.as<TYPE>();
	}
protected:
	ValueArray() {} // should only be used by child classed who initialize the pointer them self
	ValueArrayBase *clone() const override {
		return new ValueArray( *this );
	}
public:
	typedef _internal::ValueArrayIterator<TYPE> iterator;
	typedef _internal::ValueArrayIterator<const TYPE> const_iterator;
	typedef typename iterator::reference reference;
	typedef typename const_iterator::reference const_reference;

	constexpr static unsigned short staticID(){
		return util::Value<TYPE>::staticID()<<8 ;
	}
	/// delete-functor which does nothing (in case someone else manages the data).
	struct NonDeleter {
		void operator()( TYPE *p ) {
			//we have to cast the pointer to void* here, because in case of uint8_t it will try to print the "string"
			LOG( Debug, verbose_info ) << "Not freeing pointer " << ( void * )p << " (" << ValueArray<TYPE>::staticName() << ") as automatic deletion was disabled for it";
		};
	};
	/// Default delete-functor for c-arrays (uses free()).
	struct BasicDeleter {
		void operator()( TYPE *p ) {
			//we have to cast the pointer to void* here, because in case of uint8_t it will try to print the "string"
			LOG( Debug, verbose_info ) << "Freeing pointer " << ( void * )p << " (" << ValueArray<TYPE>::staticName() << ") ";
			free( p );
		};
	};
	/**
	 * Creates a ValueArray pointing to a newly allocated array of elements of the given type.
	 * The array is zero-initialized.
	 * If the requested length is 0 no memory will be allocated and the pointer will be "empty".
	 * \param length amount of elements in the new array
	 */
	ValueArray( size_t length ): ValueArray(( TYPE * )calloc( length, sizeof( TYPE ) ),  length ) {}

	/**
	 * Creates ValueArray from a std::shared_ptr of the same type.
	 * It will inherit the deleter of the shared_ptr.
	 * \param ptr the shared_ptr to share the data with
	 * \param length the length of the used array (ValueArray does NOT check for length,
	 * this is just here for child classes which may want to check)
	 */
	ValueArray( const std::shared_ptr<TYPE> &ptr, size_t length ): ValueArrayBase( length ), m_val( ptr ) {
		util::checkType<TYPE>();
		static_assert(!std::is_const<TYPE>::value,"ValueArray type must not be const");
		LOG_IF( length == 0, Debug, warning )
			<< "Creating an empty (lenght==0) ValueArray of type " << util::MSubject( staticName() )
			<< " you should overwrite it with a useful pointer before using it";
		LOG_IF(length != 0 && !ptr,Debug,error) << "Creating invalid ValueArray from null pointer";
	}

	/**
	 * Creates ValueArray from a pointer of type TYPE.
	 * The pointers are automatically deleted by an instance of BasicDeleter and should not be used outside once used here.
	 * \param ptr the pointer to the used array
	 * \param length the length of the used array (ValueArray does NOT check for length,
	 * this is just here for child classes which may want to check)
	 */
	ValueArray( TYPE *const ptr, size_t length ): ValueArray(std::shared_ptr<TYPE>( ptr, BasicDeleter() ), length ) {}

	/**
	 * Creates ValueArray from a pointer of type TYPE.
	 * The pointers are automatically deleted by an copy of d and should not be used outside once used here
	 * (this does not apply, if d does not delete).
	 * D must implement operator()(TYPE *p).
	 * \param ptr the pointer to the used array
	 * \param length the length of the used array in elements (ValueArray does NOT check for length),
	 * \param d the deleter to be used when the data shall be deleted ( d() is called then )
	 */

	template<typename D> ValueArray( TYPE *const ptr, size_t length, D d ): ValueArrayBase( length ), m_val( ptr, d ) {
		util::checkType<TYPE>();
		static_assert(!std::is_const<TYPE>::value,"ValueArray type must not be const");
	}

	virtual ~ValueArray() {}

	std::shared_ptr<const void> getRawAddress( size_t offset = 0 )const override {
		if( offset ) {
			DelProxy proxy( *this );
			const uint8_t *const b_ptr = reinterpret_cast<const uint8_t *>( m_val.get() ) + offset;
			return std::shared_ptr<const void>( b_ptr, proxy );
		} else
			return std::static_pointer_cast<const void>( m_val );
	}
	std::shared_ptr<void> getRawAddress( size_t offset = 0 ) override { // use the const version and cast away the const
		return std::const_pointer_cast<void>( const_cast<const ValueArray *>( this )->getRawAddress( offset ) );
	}
	virtual value_iterator beginGeneric() override {
		return value_iterator( ( uint8_t * )m_val.get(), ( uint8_t * )m_val.get(), bytesPerElem(), getValueFrom, setValueInto );
	}
	virtual const_value_iterator beginGeneric()const override {
		return const_value_iterator( ( uint8_t * )m_val.get(), ( uint8_t * )m_val.get(), bytesPerElem(), getValueFrom, setValueInto );
	}

	iterator begin() {return iterator( m_val.get() );}
	iterator end() {return begin() + m_len;};
	const_iterator begin()const {return const_iterator( m_val.get() );}
	const_iterator end()const {return begin() + m_len;}

	/// @copydoc util::Value::toString
	virtual std::string toString( bool labeled = false, std::string formatting="" )const override {
		std::string ret;

		if ( m_len ) {
			for ( const_iterator i = begin(); i < end() - 1; i++ )
				ret += util::Value<TYPE>( *i ).toString( false, formatting ) + "|";


			ret += util::Value<TYPE>( *( end() - 1 ) ).toString( labeled, formatting );
		}

		return std::to_string( m_len ) + "#" + ret;
	}

	std::string getTypeName()const override {return staticName();}
	unsigned short getTypeID()const override {return staticID();}
	bool isFloat() const override {return std::is_floating_point< TYPE >::value;}
	bool isInteger() const override {return std::is_integral< TYPE >::value;}

	/// @copydoc util::Value::staticName
	static std::string staticName() {
		return std::string( util::Value<TYPE>::staticName() ) + "*";
	}

	/**
	 * Reference element at at given index.
	 * If index is invalid, behaviour is undefined. Probably it will crash.
	 * \return reference to element at at given index.
	 */
	TYPE &operator[]( size_t idx ) {
		return begin()[idx];
	}
	const TYPE &operator[]( size_t idx )const {
		return begin()[idx];
	}
	/**
	 * Implicit conversion to std::shared_ptr\<TYPE\>
	 * The returned smart pointer will be part of the reference-counting and will correctly delete the data
	 * (using the given deleter) if required.
	 * \return std::shared_ptr\<TYPE\> handling same data as the object.
	 */
	operator std::shared_ptr<TYPE>&() {return m_val;}
	operator const std::shared_ptr<TYPE>&()const {return m_val;}

	size_t bytesPerElem()const override {return sizeof( TYPE );}

	std::pair<util::ValueReference, util::ValueReference> getMinMax()const override {
		if ( getLength() == 0 ) {
			LOG( Debug, error ) << "Skipping computation of min/max on an empty ValueArray";
			return std::pair<util::ValueReference, util::ValueReference>();
		} else {

			const auto result = _internal::getMinMaxImpl<TYPE, std::is_arithmetic<TYPE>::value>()( *this );

			return std::pair<util::ValueReference, util::ValueReference>( 
				util::Value<decltype(result.first)>( result.first ), 
				util::Value<decltype(result.second)>( result.second ) 
			);
		}
	}

	std::vector<Reference> splice( size_t size )const override {
		if ( size >= getLength() ) {
			LOG( Debug, warning )
					<< "splicing data of the size " << getLength() << " up into blocks of the size " << size << " is kind of useless ...";
		}

		const size_t fullSplices = getLength() / size;

		const size_t lastSize = getLength() % size;//rest of the division - size of the last splice

		const size_t splices = fullSplices + ( lastSize ? 1 : 0 );

		std::vector<Reference> ret( splices );

		DelProxy proxy( *this );

		for ( size_t i = 0; i < fullSplices; i++ )
			ret[i].reset( new ValueArray( m_val.get() + i * size, size, proxy ) );

		if ( lastSize )
			ret.back().reset( new ValueArray( m_val.get() + fullSplices * size, lastSize, proxy ) );

		return ret;
	}
	//
	scaling_pair getScalingTo( unsigned short typeID, autoscaleOption scaleopt = autoscale )const override {
		if( typeID == staticID() && scaleopt == autoscale ) { // if id is the same and autoscale is requested
			static const util::Value<uint8_t> one( 1 );
			static const util::Value<uint8_t> zero( 0 );
			return std::pair<util::ValueReference, util::ValueReference>( one, zero ); // the result is always 1/0
		} else { // get min/max and compute the scaling
			std::pair<util::ValueReference, util::ValueReference> minmax = getMinMax();
			assert( ! ( minmax.first.isEmpty() || minmax.second.isEmpty() ) );
			return ValueArrayBase::getScalingTo( typeID, minmax, scaleopt );
		}
	}
	void endianSwap() override {
		if(bytesPerElem()>1)
			data::endianSwapArray( begin(), end(), begin() );
	}
};
template<typename T> bool ValueArrayBase::is()const
{
	util::checkType<T>();
	return getTypeID() == ValueArray<T>::staticID();
}


}
}
#endif // TYPEPTR_HPP
