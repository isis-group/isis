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

#include "typeptr_base.hpp"
#include "typeptr_converter.hpp"
#include "../CoreUtils/type.hpp"
#include "common.hpp"

namespace isis
{
namespace data
{

namespace _internal
{
template<typename T, bool isNumber> struct getMinMaxImpl { // fallback for unsupportet types
	std::pair<T, T> operator()( const ValuePtr<T> &/*ref*/ ) const {
		LOG( Debug, error ) << "min/max computation of " << util::Value<T>::staticName() << " is not supportet";
		return std::pair<T, T>();
	}
};
template<typename T> std::pair<T, T> calcMinMax( const T *data, size_t len )
{
	std::pair<T, T> result( data[0], data[0] );
	LOG( Runtime, verbose_info ) << "using generic min/max computation for " << util::Value<T>::staticName();

	while ( --len ) {
		if ( result.second < data[len] )result.second = data[len];

		if ( result.first > data[len] )result.first = data[len];
	}

	return result;
}

/// @cond _hidden

#ifdef __SSE2__
////////////////////////////////////////////////
// specialize calcMinMax for (u)int(8,16,32)_t /
////////////////////////////////////////////////

template<> std::pair< uint8_t,  uint8_t> calcMinMax( const  uint8_t *data, size_t len );
template<> std::pair<uint16_t, uint16_t> calcMinMax( const uint16_t *data, size_t len );
template<> std::pair<uint32_t, uint32_t> calcMinMax( const uint32_t *data, size_t len );

template<> std::pair< int8_t,  int8_t> calcMinMax( const  int8_t *data, size_t len );
template<> std::pair<int16_t, int16_t> calcMinMax( const int16_t *data, size_t len );
template<> std::pair<int32_t, int32_t> calcMinMax( const int32_t *data, size_t len );
#endif //__SSE2__

template<typename T> struct getMinMaxImpl<T, true> { // generic minmax for numbers (this _must_ not be run on empty ValuePtr)
	std::pair<T, T> operator()( const ValuePtr<T> &ref ) const {
		return calcMinMax( &ref[0], ref.getLength() );
	}
};
/// @endcond
}

/**
 * Generic class for type (and length) - aware pointers.
 * The class is designed for arrays, but you can also "point" to an single element
 * by just use "1" for the length.
 * The pointers are reference counted and will be deleted automatically by a customizable deleter.
 * The copy is cheap, thus the copy of a ValuePtr will reference the same data.
 * The usual pointer dereferencing interface ("*", "->" and "[]") is supported.
 */
template<typename TYPE> class ValuePtr: public _internal::ValuePtrBase
{
	boost::shared_ptr<TYPE> m_val;
protected:
	ValuePtr() {} // should only be used by child classed who initialize the pointer themself
	ValuePtrBase *clone() const {
		return new ValuePtr( *this );
	}
public:
	static const unsigned short staticID = util::_internal::TypeID<TYPE>::value << 8;
	/// delete-functor which does nothing (in case someone else manages the data).
	struct NonDeleter {
		void operator()( TYPE *p ) {
			//we have to cast the pointer to void* here, because in case of u_int8_t it will try to print the "string"
			LOG( Debug, info ) << "Not freeing pointer " << ( void * )p << " (" << ValuePtr<TYPE>::staticName() << ") ";
		};
	};
	/// Default delete-functor for c-arrays (uses free()).
	struct BasicDeleter {
		void operator()( TYPE *p ) {
			//we have to cast the pointer to void* here, because in case of u_int8_t it will try to print the "string"
			LOG( Debug, verbose_info ) << "Freeing pointer " << ( void * )p << " (" << ValuePtr<TYPE>::staticName() << ") ";
			free( p );
		};
	};
	/// Default delete-functor for arrays of objects (uses delete[]).
	struct ObjectArrayDeleter {
		void operator()( TYPE *p ) {
			//we have to cast the pointer to void* here, because in case of u_int8_t it will try to print the "string"
			LOG( Debug, info ) << "Deleting object array at " << ( void * )p << " (" << ValuePtr<TYPE>::staticName() << ") ";
			delete[] p;
		};
	};
	/**
	 * Creates a ValuePtr pointing to a newly allocated array of elements of the given type.
	 * The array is zero-initialized.
	 * If the requested length is 0 no memory will be allocated and the pointer be "empty".
	 * \param length amount of elements in the new array
	 */
	ValuePtr( size_t length ): _internal::ValuePtrBase( length ) {
		if( length )
			m_val.reset( ( TYPE * )calloc( length, sizeof( TYPE ) ), BasicDeleter() );

		LOG_IF( length == 0, Debug, warning )
				<< "Creating an empty ValuePtr of type " << util::MSubject( staticName() )
				<< " you should overwrite it with a usefull pointer before using it";
	}

	/**
	 * Creates ValuePtr from a boost:shared_ptr of the same type.
	 * It will inherit the deleter of the shared_ptr.
	 * \param ptr the shared_ptr to share the data with
	 * \param length the length of the used array (ValuePtr does NOT check for length,
	 * this is just here for child classes which may want to check)
	 */
	ValuePtr( const boost::shared_ptr<TYPE> &ptr, size_t length ): _internal::ValuePtrBase( length ), m_val( ptr ) {}

	/**
	 * Creates ValuePtr from a pointer of type TYPE.
	 * The pointers are automatically deleted by an instance of BasicDeleter and should not be used outside once used here.
	 * If ptr is a pointer to C++ objects (delete[] needed) you must use
	 * ValuePtr(ptr,len,ValuePtr\<TYPE\>::ObjectArrayDeleter())!
	 * \param ptr the pointer to the used array
	 * \param length the length of the used array (ValuePtr does NOT check for length,
	 * this is just here for child classes which may want to check)
	 */
	ValuePtr( TYPE *const ptr, size_t length ): _internal::ValuePtrBase( length ), m_val( ptr, BasicDeleter() ) {}

	/**
	 * Creates ValuePtr from a pointer of type TYPE.
	 * The pointers are automatically deleted by an copy of d and should not be used outside once used here
	 * (this does not apply, if d does not delete).
	 * D must implement operator()(TYPE *p).
	 * \param ptr the pointer to the used array
	 * \param length the length of the used array in elements (ValuePtr does NOT check for length),
	 * \param d the deleter to be used when the data shall be deleted ( d() is called then )
	 */

	template<typename D> ValuePtr( TYPE *const ptr, size_t length, D d ): _internal::ValuePtrBase( length ), m_val( ptr, d ) {}

	virtual ~ValuePtr() {}

	boost::shared_ptr<const void> getRawAddress( size_t offset = 0 )const {
		if( offset ) {
			DelProxy proxy( *this );
			const uint8_t *const b_ptr = reinterpret_cast<const uint8_t *>( m_val.get() ) + offset;
			return boost::shared_ptr<const void>( b_ptr, proxy );
		} else
			return boost::static_pointer_cast<const void>( m_val );
	}
	boost::shared_ptr<void> getRawAddress( size_t offset = 0 ) { // use the const version and cast away the const
		return boost::const_pointer_cast<void>( const_cast<const ValuePtr *>( this )->getRawAddress( offset ) );
	}

	/// @copydoc util::Value::toString
	virtual std::string toString( bool labeled = false )const {
		std::string ret;

		if ( m_len ) {
			const TYPE *ptr = m_val.get();

			for ( size_t i = 0; i < m_len - 1; i++ )
				ret += util::Value<TYPE>( ptr[i] ).toString( false ) + "|";

			ret += util::Value<TYPE>( ptr[m_len - 1] ).toString( labeled );
		}

		return boost::lexical_cast<std::string>( m_len ) + "#" + ret;
	}

	std::string getTypeName()const {return staticName();}
	unsigned short getTypeID()const {return staticID;}
	bool isFloat() const {return boost::is_float< TYPE >::value;}
	bool isInteger() const {return boost::is_integral< TYPE >::value;}

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
		return ( m_val.get() )[idx];
	}
	const TYPE &operator[]( size_t idx )const {
		return ( m_val.get() )[idx];
	}
	/**
	 * Implicit conversion to boost::shared_ptr\<TYPE\>
	 * The returned smart pointer will be part of the reference-counting and will correctly delete the data
	 * (using the given deleter) if required.
	 * \return boost::shared_ptr\<TYPE\> handling same data as the object.
	 */
	operator boost::shared_ptr<TYPE>&() {return m_val;}
	operator const boost::shared_ptr<TYPE>&()const {return m_val;}

	size_t bytesPerElem()const {return sizeof( TYPE );}

	std::pair<util::ValueReference, util::ValueReference> getMinMax()const {
		if ( getLength() == 0 ) {
			LOG( Debug, error ) << "Skipping computation of min/max on an empty ValuePtr";
			return std::pair<util::ValueReference, util::ValueReference>();
		} else {

			const std::pair<util::Value<TYPE>, util::Value<TYPE> > result = _internal::getMinMaxImpl<TYPE, boost::is_arithmetic<TYPE>::value>()( *this );

			return std::make_pair( util::ValueReference( result.first ), util::ValueReference( result.second ) );
		}
	}

	std::vector<Reference> splice( size_t size )const {
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
			ret[i].reset( new ValuePtr( m_val.get() + i * size, size, proxy ) );

		if ( lastSize )
			ret.back().reset( new ValuePtr( m_val.get() + fullSplices * size, lastSize, proxy ) );

		return ret;
	}
	//
	scaling_pair getScalingTo( unsigned short typeID, autoscaleOption scaleopt = autoscale )const {
		if( typeID == staticID && scaleopt == autoscale ) { // if id is the same and autoscale is requested
			static const util::Value<uint8_t> one( 1 );
			static const util::Value<uint8_t> zero( 0 );
			return std::pair<util::ValueReference, util::ValueReference>( one, zero ); // the result is always 1/0
		} else { // get min/max and compute the scaling
			std::pair<util::ValueReference, util::ValueReference> minmax = getMinMax();
			assert( ! ( minmax.first.isEmpty() || minmax.second.isEmpty() ) );
			return ValuePtrBase::getScalingTo( typeID, minmax, scaleopt );
		}
	}
};
/// @cond _hidden
// specialisation for complex - there shall be no scaling - and we cannot compute minmax
template<> scaling_pair ValuePtr<std::complex<float> >::getScalingTo( unsigned short /*typeID*/, autoscaleOption /*scaleopt*/ )const;
template<> scaling_pair ValuePtr<std::complex<double> >::getScalingTo( unsigned short /*typeID*/, autoscaleOption /*scaleopt*/ )const;
/// @endcond
template<typename T> bool _internal::ValuePtrBase::is()const
{
	util::checkType<T>();
	return getTypeID() == ValuePtr<T>::staticID;
}


}
}
#endif // TYPEPTR_HPP
