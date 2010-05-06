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

#include "log.hpp"
#include "type_base.hpp"
#include "string.h"

#include <string>
#include <functional>

// @todo we need to know this for lexical_cast (toString)
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>



namespace isis
{
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util
{

template<class TYPE > class Type;


namespace _internal
{
template<typename T, bool isNumber> struct type_less {
	bool operator()( const Type<T> &first, const TypeBase &second )const {
		LOG( Debug, error ) << "less comparison of " << Type<T>::staticName() << " is not supportet";
		return false;
	}
};
template<typename T, bool isNumber> struct type_greater {
	bool operator()( const Type<T> &first, const TypeBase &second )const {
		LOG( Debug, error ) << "greater than comparison of " << Type<T>::staticName() << " is not supportet";
		return false;
	}
};
template<typename T> struct type_less<T, true> {
	bool operator()( const Type<T> &first, const TypeBase &second )const {return ( T )first < second.as<T>();}
};
template<typename T> struct type_greater<T, true> {
	bool operator()( const Type<T> &first, const TypeBase &second )const {return ( T )first > second.as<T>();}
};
template<typename T, bool isNumber> struct getMinMaxImpl {
	std::pair<T, T> operator()( const TypePtr<T> &ref ) const {
		LOG( Debug, error ) << "min/max comparison of " << Type<T>::staticName() << " is not supportet";
		return std::pair<T, T>();
	}
};
template<typename T> struct getMinMaxImpl<T, true> {
	std::pair<T, T> operator()( const TypePtr<T> &ref ) const {
		std::pair<T, T> result;

		for ( size_t i = 0; i < ref.len(); i++ ) {
			if ( result.second < ref[i] )result.second = ref[i];

			if ( result.first > ref[i] )result.first = ref[i];
		}

		return result;
	}
};

}

/// Generic class for type aware variables
template<typename TYPE> class Type: public _internal::TypeBase
{
	TYPE m_val;
	static const char m_typeName[];
protected:
	TypeBase* clone() const {
		return new Type<TYPE>( *this );
	}
public:
	static const unsigned short staticID = _internal::TypeId<TYPE>::value;
	Type() {
		BOOST_MPL_ASSERT_RELATION( staticID, < , 0xFF );
		check_type<TYPE>();
	}
	/**
	 * Create a Type from any type of value-type.
	 * If the type of the parameter is not the same as the content type of the object, the system tries to do a type conversion.
	 * If that fails, boost::bad_lexical_cast is thrown.
	 */
	template<typename T> Type( const T& value ): m_val( __cast_to( this, value ) ) {
		BOOST_MPL_ASSERT_RELATION( staticID, < , 0xFF );
		check_type<TYPE>();
	}
	virtual bool is( const std::type_info & t )const {
		return t == typeid( TYPE );
	}
	virtual std::string toString( bool labeled = false )const {
		const Converter &conv = getConverterTo( Type<std::string>::staticID );
		std::string ret;

		if ( conv ) {
			Type<std::string> buff;
			conv->convert( *this, buff );
			ret = buff;
		} else {
			LOG( Debug, warning ) << "Missing conversion from " << typeName() << " to string falling back to boost::lexical_cast<std::string>";
			ret = boost::lexical_cast<std::string>( m_val );
		}

		if ( labeled )ret += "(" + staticName() + ")";

		return ret;
	}
	virtual std::string typeName()const {
		return staticName();
	}
	virtual unsigned short typeID()const {
		return staticID;
	}

	/// \returns true if this and second contain the same value of the same type
	virtual bool eq( const TypeBase &second )const {
		if ( second.is<TYPE>() ) {
			const TYPE &otherVal = ( TYPE )second.cast_to_Type<TYPE>();
			return m_val == otherVal;
		} else
			return  false;
	}

	/// \returns the name of the type
	static std::string staticName() {return m_typeName;}

	/**
	 * Implicit conversion of Type to its value type.
	 * Only the actual type is allowed.
	 * However, the following is valid:
	 * \code
	 * Type<int> i(5);
	 * float f=i;
	 * \endcode
	 * In this case the function returns int which is then also implicitely converted to float.
	 * \return the stored value
	 */
	operator const TYPE&()const {return m_val;}
	operator TYPE&() {return m_val;}

	bool operator >( const _internal::TypeBase &ref )const {
		return _internal::type_greater<TYPE, boost::is_arithmetic<TYPE>::value >()( *this, ref );
	}
	bool operator <( const _internal::TypeBase &ref )const {
		return _internal::type_less<TYPE, boost::is_arithmetic<TYPE>::value >()( *this, ref );
	}

	virtual ~Type() {}
};

/**
 * Generic class for type (and length) - aware pointers.
 * The class is designed for arrays, but you can also "point" to an single element
 * by just use "1" for the length.
 * The pointers are reference counted and will be deleted automatically by a customizable deleter.
 * The copy is cheap, thus the copy of a TypePtr will reference the same data.
 */
template<typename TYPE> class TypePtr: public _internal::TypePtrBase
{
	boost::shared_ptr<TYPE> m_val;
	template<typename T> TypePtr( const Type<T>& value ); // Dont do this
protected:
	const boost::weak_ptr<void> address()const {
		return boost::weak_ptr<void>( m_val );
	}
	TypePtrBase* clone() const {
		return new TypePtr( *this );
	}
public:
	static const unsigned short staticID = _internal::TypeId<TYPE>::value << 8;
	/// Proxy-Deleter to encapsulate the real deleter/shared_ptr when creating shared_ptr for parts of a shared_ptr
	class DelProxy : public boost::shared_ptr<TYPE>
	{
	public:
		/**
		 * Create a proxy for a given master shared_ptr
		 * This increments the use_count of the master and thus keeps the
		 * master from being deleted while parts of it are still in use.
		 */
		DelProxy( const TypePtr<TYPE> &master ): boost::shared_ptr<TYPE>( master ) {
			LOG( Debug, verbose_info ) << "Creating DelProxy for " << this->get();
		}
		/// decrement the use_count of the master when a specific part is not referenced anymore
		void operator()( TYPE *at ) {
			LOG( Debug, verbose_info )
					<< "Deletion for " << this->get() << " called from splice at offset "   << at - this->get()
					<< ", current use_count: " << this->use_count();
			this->reset();//actually not needed, but we keep it here to keep obfuscation low
		}
	};
	/// delete-functor which does nothing (in case someone else manages the data).
	struct NonDeleter {
		void operator()( TYPE *p ) {
			//we have to cast the pointer to void* here, because in case of u_int8_t it will try to print the "string"
			LOG( Debug, info ) << "Not freeing pointer " << ( void* )p << " (" << TypePtr<TYPE>::staticName() << ") ";
		};
	};
	/// Default delete-functor for c-arrays (uses free()).
	struct BasicDeleter {
		void operator()( TYPE *p ) {
			//we have to cast the pointer to void* here, because in case of u_int8_t it will try to print the "string"
			LOG( Debug, info ) << "Freeing pointer " << ( void* )p << " (" << TypePtr<TYPE>::staticName() << ") ";
			free( p );
		};
	};
	/// Default delete-functor for arrays of objects (uses delete[]).
	struct ObjectArrayDeleter {
		void operator()( TYPE *p ) {
			//we have to cast the pointer to void* here, because in case of u_int8_t it will try to print the "string"
			LOG( Debug, info ) << "Deleting object array at " << ( void* )p << " (" << TypePtr<TYPE>::staticName() << ") ";
			delete[] p;
		};
	};
	/**
	* Contructor for empty pointer.
	* length will be 0 and every attempt to dereference it will raise an exception.
	*/
	TypePtr() {}
	/**
	 * Creates TypePtr from a pointer of type TYPE.
	 * The pointers are automatically deleted by an instance of BasicDeleter and should not be used outside once used here.
	 * If ptr is a pointer to C++ objects (delete[] needed) you must use
	 * TypePtr(ptr,len,TypePtr\<TYPE\>::ObjectArrayDeleter())!
	 * The usual dereferencing pointer interface ("*" and "->") is supported.
	 * \param ptr the pointer to the used array
	 * \param length the length of the used array (TypePtr does NOT check for length,
	 * this is just here for child classes which may want to check)
	 */
	TypePtr( TYPE* const ptr, size_t length ):
		_internal::TypePtrBase( length ), m_val( ptr, BasicDeleter() ) {}
	/**
	 * Creates TypePtr from a pointer of type TYPE.
	 * The pointers are automatically deleted by an copy of d and should not be used outside once used here
	 * (this does not apply, if d does not delete).
	 * The usual dereferencing pointer interface ("*" and "->") is supported.
	 * D must implement operator()(TYPE *p).
	 * \param ptr the pointer to the used array
	 * \param length the length of the used array in elements (TypePtr does NOT check for length),
	 * \param d the deleter to be used when the data shall be deleted ( d() is called then )
	 */

	template<typename D> TypePtr( TYPE* const ptr, size_t length, D d ):
		_internal::TypePtrBase( length ), m_val( ptr, d ) {}

	virtual ~TypePtr() {}

	/// Copy elements from raw memory
	void copyFromMem( const TYPE* const src, size_t length ) {
		LOG_IF( length > len(), Runtime, error )
				<< "Amount of the elements to copy from memory (" << length << ") exceeds the length of the array (" << len() << ")";
		TYPE &dest = this->operator[]( 0 );
		LOG( Debug, info ) << "Copying " << length*sizeof( TYPE ) << " bytes of " << typeName() << " from " << src << " to " << &dest;
		memcpy( &dest, src, length * sizeof( TYPE ) );
	}
	/// Copy elements within a range [start,end] to raw memory
	void copyToMem( size_t start, size_t end, const TYPE* const dst )const {
		assert( start <= end );
		const size_t length = end - start + 1;
		LOG_IF( end >= len(), Runtime, error )
				<< "End of the range (" << end << ") is behind the end of this TypePtr (" << len() << ")";
		const TYPE &source = this->operator[]( start );
		memcpy( dst, &source, length * sizeof( TYPE ) );
	}
	size_t cmp( size_t start, size_t end, const isis::util::_internal::TypePtrBase& dst, size_t dst_start ) const {
		assert( start <= end );
		size_t ret = 0;
		size_t length = end - start;

		if ( dst.typeID() != typeID() ) {
			LOG( Runtime, error )
					<< "Comparing to a TypePtr of different type(" << dst.typeName() << ", not " << typeName()
					<< "). Assuming all voxels to be different";
			return length;
		}

		LOG_IF( end >= len(), Runtime, error )
				<< "End of the range (" << end << ") is behind the end of this TypePtr (" << len() << ")";
		LOG_IF( length + dst_start >= dst.len(), Runtime, error )
				<< "End of the range (" << length + dst_start << ") is behind the end of the destination (" << dst.len() << ")";
		const TypePtr<TYPE> &compare = dst.cast_to_TypePtr<TYPE>();
		LOG( Debug, verbose_info ) << "Comparing " << dst.typeName() << " at " << &operator[]( 0 ) << " and " << &compare[0];

		for ( size_t i = start; i < end; i++ ) {
			if ( not ( operator[]( i ) == compare[i] ) )
				ret++;
		}

		return ret;
	}

	/// @copydoc Type::is()
	virtual bool is( const std::type_info & t )const {
		return t == typeid( TYPE );
	}
	/// @copydoc Type::toString()
	virtual std::string toString( bool labeled = false )const {
		std::string ret;

		if ( m_len ) {
			const TYPE* ptr = m_val.get();

			for ( size_t i = 0; i < m_len - 1; i++ )
				ret += Type<TYPE>( ptr[i] ).toString( false ) + "|";

			ret += Type<TYPE>( ptr[m_len-1] ).toString( labeled );
		}

		return boost::lexical_cast<std::string>( m_len ) + "#" + ret;
	}
	/// @copydoc Type::typeName()
	virtual std::string typeName()const {
		return staticName();
	}
	/// @copydoc Type::typeID()
	virtual unsigned short typeID()const {
		return staticID;
	}
	/// @copydoc Type::staticName()
	static std::string staticName() {
		return std::string( Type<TYPE>::staticName() ) + "*";
	}

	/**
	 * Reference element at at given index.
	 * If index is invalid, behaviour is undefined. Probably it will crash.
	 * \return reference to element at at given index.
	 */
	TYPE& operator[]( size_t idx ) {
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

	TypePtrBase::Reference cloneToMem( size_t length ) const {
		return TypePtrBase::Reference( new TypePtr( ( TYPE* )malloc( length * sizeof( TYPE ) ), length ) );
	}
	size_t bytes_per_elem() const {
		return sizeof( TYPE );
	}
	bool convertTo( TypePtrBase& dst )const {
		Type<TYPE> min, max;
		getMinMax( min, max );
		return TypePtrBase::convertTo( dst, min, max );
	}
	void getMinMax ( _internal::TypeBase& min, _internal::TypeBase& max, bool init = true ) const {
		assert( min.typeID() == max.typeID() );

		if( len() == 0 ) {
			LOG( Runtime, warning ) << "Skipping computation of min/max on an empty TypePtr";
			return;
		}

		if( init ) { // they haven't been set yet
			const Type<TYPE> el( this->operator[]( 0 ) );
			_internal::TypeBase::convert( el, min );
			_internal::TypeBase::convert( el, max );
		}

		const std::pair<Type<TYPE>, Type<TYPE> > result = _internal::getMinMaxImpl<TYPE, boost::is_arithmetic<TYPE>::value>()( *this );

		if( min > result.first ) {
			_internal::TypeBase::convert( result.first, min );
		}

		if( max < result.second ) {
			_internal::TypeBase::convert( result.second, max );
		}
	}

	std::vector<Reference> splice( size_t size )const {
		if ( size >= len() ) {
			LOG( Debug, warning )
					<< "splicing data of the size " << len() << " up into blocks of the size " << size << " is kind of useless ...";
		}

		const size_t fullSplices = len() / size;

		const size_t lastSize = len() % size;//rest of the division - size of the last splice

		const size_t splices = fullSplices + ( lastSize ? 1 : 0 );

		std::vector<Reference> ret( splices );

		DelProxy proxy( *this );

		for ( size_t i = 0; i < fullSplices; i++ )
			ret[i].reset( new TypePtr( m_val.get() + i * size, size, proxy ) );

		if ( lastSize )
			ret.back().reset( new TypePtr( m_val.get() + fullSplices * size, lastSize, proxy ) );

		return ret;
	}
};

}
/// @}
}

#endif //DATATYPE_INC
