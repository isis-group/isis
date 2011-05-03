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
template<typename T, bool isNumber> struct getMinMaxImpl {
	std::pair<T, T> operator()( const ValuePtr<T> &/*ref*/ ) const {
		LOG( Debug, error ) << "min/max comparison of " << util::Value<T>::staticName() << " is not supportet";
		return std::pair<T, T>();
	}
};
template<typename T> struct getMinMaxImpl<T, true> {
	std::pair<T, T> operator()( const ValuePtr<T> &ref ) const {
		std::pair<T, T> result;

		for ( size_t i = 0; i < ref.getLength(); i++ ) {
			if ( result.second < ref[i] )result.second = ref[i];

			if ( result.first > ref[i] )result.first = ref[i];
		}

		return result;
	}
};

}

/**
 * Generic class for type (and length) - aware pointers.
 * The class is designed for arrays, but you can also "point" to an single element
 * by just use "1" for the length.
 * The pointers are reference counted and will be deleted automatically by a customizable deleter.
 * The copy is cheap, thus the copy of a ValuePtr will reference the same data.
 * The usual dereferencing pointer interface ("*" and "->") is supported.
 */
template<typename TYPE> class ValuePtr: public _internal::ValuePtrBase
{
	boost::shared_ptr<TYPE> m_val;
	template<typename T> ValuePtr( const util::Value<T>& value ); // Dont do this
protected:
	ValuePtrBase *clone() const {
		return new ValuePtr( *this );
	}
	/// Proxy-Deleter to encapsulate the real deleter/shared_ptr when creating shared_ptr for parts of a shared_ptr
	class DelProxy : public boost::shared_ptr<TYPE>
	{
	public:
		/**
		 * Create a proxy for a given master shared_ptr
		 * This increments the use_count of the master and thus keeps the
		 * master from being deleted while parts of it are still in use.
		 */
		DelProxy( const ValuePtr<TYPE> &master ): boost::shared_ptr<TYPE>( master ) {
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
	 * Contructor for empty pointer.
	 * length will be 0 and every attempt to dereference it will raise an exception.
	 */
	ValuePtr() {
		LOG( Debug, warning ) << "Creating an empty ValuePtr of type " << util::MSubject( staticName() ) << " you should overwrite it with a usefull pointer before using it";
	}

	/**
	 * Creates ValuePtr from a pointer of type TYPE.
	 * The pointers are automatically deleted by an instance of BasicDeleter and should not be used outside once used here.
	 * If ptr is a pointer to C++ objects (delete[] needed) you must use
	 * ValuePtr(ptr,len,ValuePtr\<TYPE\>::ObjectArrayDeleter())!
	 * \param ptr the pointer to the used array
	 * \param length the length of the used array (ValuePtr does NOT check for length,
	 * this is just here for child classes which may want to check)
	 */
	ValuePtr( TYPE *const ptr, size_t length ):
		_internal::ValuePtrBase( length ), m_val( ptr, BasicDeleter() ) {}

	/**
	 * Creates ValuePtr from a pointer of type TYPE.
	 * The pointers are automatically deleted by an copy of d and should not be used outside once used here
	 * (this does not apply, if d does not delete).
	 * D must implement operator()(TYPE *p).
	 * \param ptr the pointer to the used array
	 * \param length the length of the used array in elements (ValuePtr does NOT check for length),
	 * \param d the deleter to be used when the data shall be deleted ( d() is called then )
	 */

	template<typename D> ValuePtr( TYPE *const ptr, size_t length, D d ):
		_internal::ValuePtrBase( length ), m_val( ptr, d ) {}

	virtual ~ValuePtr() {}

	/**
	 * Create a new ValuePtr which uses newly allocated memory.
	 * \param len requested size of the memory block in elements
	 * \returns a ValuePtr\<TYPE\> of given len
	 */
	static ValuePtr allocate( size_t len ) {
		return ValuePtr( ( TYPE * )malloc( len * sizeof( TYPE ) ), len );
	}

	/**
	 * Get the raw address the ValuePtr points to.
	 * \returns a weak_ptr\<void\> with the memory address of the data handled by this ValuePtr.
	 */
	const boost::weak_ptr<void> getRawAddress()const {
		return boost::weak_ptr<void>( m_val );
	}

	/// Copy elements from raw memory
	void copyFromMem( const TYPE *const src, size_t _length ) {
		LOG_IF( _length > getLength(), Runtime, error )
				<< "Amount of the elements to copy from memory (" << _length << ") exceeds the length of the array (" << getLength() << ")";
		TYPE &dest = this->operator[]( 0 );
		LOG( Debug, info ) << "Copying " << _length *sizeof( TYPE ) << " bytes from " << ValuePtr<TYPE>::staticName() << src << " to " << getTypeName() << &dest;
		memcpy( &dest, src, _length * sizeof( TYPE ) );
	}
	/// Copy elements within a range [start,end] to raw memory
	void copyToMem( size_t start, size_t end, TYPE *const dst )const {
		assert( start <= end );
		const size_t _length = end - start + 1;
		LOG_IF( end >= getLength(), Runtime, error )
				<< "End of the range (" << end << ") is behind the end of this ValuePtr (" << getLength() << ")";
		const TYPE &source = this->operator[]( start );
		memcpy( dst, &source, _length * sizeof( TYPE ) );
	}

	size_t compare( size_t start, size_t end, const _internal::ValuePtrBase &dst, size_t dst_start ) const {
		assert( start <= end );
		size_t ret = 0;
		size_t _length = end - start;

		if ( dst.getTypeID() != getTypeID() ) {
			LOG( Debug, error )
					<< "Comparing to a ValuePtr of different type(" << dst.getTypeName() << ", not " << getTypeName()
					<< "). Assuming all voxels to be different";
			return _length;
		}

		LOG_IF( end >= getLength(), Runtime, error )
				<< "End of the range (" << end << ") is behind the end of this ValuePtr (" << getLength() << ")";
		LOG_IF( _length + dst_start >= dst.getLength(), Runtime, error )
				<< "End of the range (" << _length + dst_start << ") is behind the end of the destination (" << dst.getLength() << ")";
		const ValuePtr<TYPE> &cmp = dst.castToValuePtr<TYPE>();
		LOG( Debug, verbose_info ) << "Comparing " << dst.getTypeName() << " at " << &operator[]( 0 ) << " and " << &cmp[0];

		for ( size_t i = start; i < end; i++ ) {
			if ( ! ( operator[]( i ) == cmp[i] ) ) {
				ret++;
			}
		}

		return ret;
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
	/// @copydoc util::Value::getTypeName
	virtual std::string getTypeName()const {
		return staticName();
	}
	/// @copydoc util::Value::getTypeID
	virtual unsigned short getTypeID()const {
		return staticID;
	}
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

	ValuePtrBase::Reference cloneToNew( size_t _length ) const {
		return ValuePtrBase::Reference( new ValuePtr( ( TYPE * )malloc( _length * sizeof( TYPE ) ), _length ) );
	}

	size_t bytesPerElem() const {
		return sizeof( TYPE );
	}



	std::pair<util::ValueReference, util::ValueReference> getMinMax()const {
		if ( getLength() == 0 ) {
			LOG( Runtime, warning ) << "Skipping computation of min/max on an empty ValuePtr";
			std::pair<util::ValueReference, util::ValueReference>();
		}

		const std::pair<util::Value<TYPE>, util::Value<TYPE> > result = _internal::getMinMaxImpl<TYPE, boost::is_arithmetic<TYPE>::value>()( *this );

		return std::make_pair( util::ValueReference( result.first ), util::ValueReference( result.second ) );
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
	scaling_pair getScalingTo( unsigned short typeID, autoscaleOption scaleopt = autoscale )const{
		std::pair<util::ValueReference, util::ValueReference> minmax = getMinMax();
		assert( ! ( minmax.first.isEmpty() || minmax.second.isEmpty() ) );
		return ValuePtrBase::getScalingTo( typeID, minmax, scaleopt );
	}
};

// specialisation for complex - there shall be no scaling - and we cannot compute minmax
template<> scaling_pair ValuePtr<std::complex<float> >::getScalingTo( unsigned short /*typeID*/, autoscaleOption /*scaleopt*/ )const;
template<> scaling_pair ValuePtr<std::complex<double> >::getScalingTo( unsigned short /*typeID*/, autoscaleOption /*scaleopt*/ )const;

template<typename T> bool _internal::ValuePtrBase::is()const
{
	util::checkType<T>();
	return getTypeID() == ValuePtr<T>::staticID;
}


}
}
#endif // TYPEPTR_HPP
