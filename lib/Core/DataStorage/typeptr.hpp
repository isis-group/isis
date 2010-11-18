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
#include "CoreUtils/type.hpp"
#include "common.hpp"

namespace isis
{
namespace data
{

namespace _internal
{
template<typename T, bool isNumber> struct getMinMaxImpl {
	std::pair<T, T> operator()( const TypePtr<T> &ref ) const {
		LOG( Debug, error ) << "min/max comparison of " << util::Type<T>::staticName() << " is not supportet";
		return std::pair<T, T>();
	}
};
template<typename T> struct getMinMaxImpl<T, true> {
	std::pair<T, T> operator()( const TypePtr<T> &ref ) const {
		std::pair<T, T> result;

		for ( size_t i = 0; i < ref.length(); i++ ) {
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
 * The copy is cheap, thus the copy of a TypePtr will reference the same data.
 * The usual dereferencing pointer interface ("*" and "->") is supported.
 */
template<typename TYPE> class TypePtr: public _internal::TypePtrBase
{
	boost::shared_ptr<TYPE> m_val;
	template<typename T> TypePtr( const util::Type<T>& value ); // Dont do this
protected:
	const boost::weak_ptr<void> address()const {
		return boost::weak_ptr<void>( m_val );
	}
	TypePtrBase *clone() const {
		return new TypePtr( *this );
	}
public:
	static const unsigned short staticID = util::_internal::TypeID<TYPE>::value << 8;
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
			LOG( Debug, info ) << "Not freeing pointer " << ( void * )p << " (" << TypePtr<TYPE>::staticName() << ") ";
		};
	};
	/// Default delete-functor for c-arrays (uses free()).
	struct BasicDeleter {
		void operator()( TYPE *p ) {
			//we have to cast the pointer to void* here, because in case of u_int8_t it will try to print the "string"
			LOG( Debug, verbose_info ) << "Freeing pointer " << ( void * )p << " (" << TypePtr<TYPE>::staticName() << ") ";
			free( p );
		};
	};
	/// Default delete-functor for arrays of objects (uses delete[]).
	struct ObjectArrayDeleter {
		void operator()( TYPE *p ) {
			//we have to cast the pointer to void* here, because in case of u_int8_t it will try to print the "string"
			LOG( Debug, info ) << "Deleting object array at " << ( void * )p << " (" << TypePtr<TYPE>::staticName() << ") ";
			delete[] p;
		};
	};
	/**
	 * Contructor for empty pointer.
	 * length will be 0 and every attempt to dereference it will raise an exception.
	 */
	TypePtr() {
		LOG( Debug, warning ) << "Creating an empty TypePtr of type " << util::MSubject( staticName() ) << " you should overwrite it with a usefull pointer before using it";
	}
	/**
	 * Creates TypePtr from a pointer of type TYPE.
	 * The pointers are automatically deleted by an instance of BasicDeleter and should not be used outside once used here.
	 * If ptr is a pointer to C++ objects (delete[] needed) you must use
	 * TypePtr(ptr,len,TypePtr\<TYPE\>::ObjectArrayDeleter())!
	 * \param ptr the pointer to the used array
	 * \param length the length of the used array (TypePtr does NOT check for length,
	 * this is just here for child classes which may want to check)
	 */
	TypePtr( TYPE *const ptr, size_t length ):
		_internal::TypePtrBase( length ), m_val( ptr, BasicDeleter() ) {}
	/**
	 * Creates TypePtr from a pointer of type TYPE.
	 * The pointers are automatically deleted by an copy of d and should not be used outside once used here
	 * (this does not apply, if d does not delete).
	 * D must implement operator()(TYPE *p).
	 * \param ptr the pointer to the used array
	 * \param length the length of the used array in elements (TypePtr does NOT check for length),
	 * \param d the deleter to be used when the data shall be deleted ( d() is called then )
	 */

	template<typename D> TypePtr( TYPE *const ptr, size_t length, D d ):
		_internal::TypePtrBase( length ), m_val( ptr, d ) {}

	virtual ~TypePtr() {}

	/// Copy elements from raw memory
	void copyFromMem( const TYPE *const src, size_t len ) {
		LOG_IF( len > length(), Runtime, error )
				<< "Amount of the elements to copy from memory (" << len << ") exceeds the length of the array (" << length() << ")";
		TYPE &dest = this->operator[]( 0 );
		LOG( Debug, info ) << "Copying " << len *sizeof( TYPE ) << " bytes of " << typeName() << " from " << src << " to " << &dest;
		memcpy( &dest, src, len * sizeof( TYPE ) );
	}
	/// Copy elements within a range [start,end] to raw memory
	void copyToMem( size_t start, size_t end, TYPE *const dst )const {
		assert( start <= end );
		const size_t len = end - start + 1;
		LOG_IF( end >= length(), Runtime, error )
				<< "End of the range (" << end << ") is behind the end of this TypePtr (" << length() << ")";
		const TYPE &source = this->operator[]( start );
		memcpy( dst, &source, len * sizeof( TYPE ) );
	}
	size_t compare( size_t start, size_t end, const _internal::TypePtrBase &dst, size_t dst_start ) const {
		assert( start <= end );
		size_t ret = 0;
		size_t len = end - start;

		if ( dst.typeID() != typeID() ) {
			LOG( Debug, error )
					<< "Comparing to a TypePtr of different type(" << dst.typeName() << ", not " << typeName()
					<< "). Assuming all voxels to be different";
			return len;
		}

		LOG_IF( end >= length(), Runtime, error )
				<< "End of the range (" << end << ") is behind the end of this TypePtr (" << length() << ")";
		LOG_IF( len + dst_start >= dst.length(), Runtime, error )
				<< "End of the range (" << len + dst_start << ") is behind the end of the destination (" << dst.length() << ")";
		const TypePtr<TYPE> &compare = dst.castToTypePtr<TYPE>();
		LOG( Debug, verbose_info ) << "Comparing " << dst.typeName() << " at " << &operator[]( 0 ) << " and " << &compare[0];

		for ( size_t i = start; i < end; i++ ) {
			if ( ! ( operator[]( i ) == compare[i] ) ){
				ret++;
			}
		}

		return ret;
	}

	/// @copydoc util::Type::toString
	virtual std::string toString( bool labeled = false )const {
		std::string ret;

		if ( m_len ) {
			const TYPE *ptr = m_val.get();

			for ( size_t i = 0; i < m_len - 1; i++ )
				ret += util::Type<TYPE>( ptr[i] ).toString( false ) + "|";

			ret += util::Type<TYPE>( ptr[m_len-1] ).toString( labeled );
		}

		return boost::lexical_cast<std::string>( m_len ) + "#" + ret;
	}
	/// @copydoc util::Type::typeName
	virtual std::string typeName()const {
		return staticName();
	}
	/// @copydoc util::Type::typeID
	virtual unsigned short typeID()const {
		return staticID;
	}
	/// @copydoc util::Type::staticName
	static std::string staticName() {
		return std::string( util::Type<TYPE>::staticName() ) + "*";
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

	TypePtrBase::Reference cloneToNew( size_t length ) const {
		return TypePtrBase::Reference( new TypePtr( ( TYPE * )malloc( length * sizeof( TYPE ) ), length ) );
	}
	size_t bytesPerElem() const {
		return sizeof( TYPE );
	}
	/// \copydoc _internal::TypePtrBase::getMinMax
	void getMinMax ( util::TypeReference &min, util::TypeReference &max ) const {
		if ( length() == 0 ) {
			LOG( Runtime, warning ) << "Skipping computation of min/max on an empty TypePtr";
			return;
		}

		const std::pair<util::Type<TYPE>, util::Type<TYPE> > result = _internal::getMinMaxImpl<TYPE, boost::is_arithmetic<TYPE>::value>()( *this );

		if ( min.empty() || min->gt( result.first ) )
			min = result.first;

		if ( max.empty() || max->lt( result.second ) )
			max = result.second;
	}

	std::vector<Reference> splice( size_t size )const {
		if ( size >= length() ) {
			LOG( Debug, warning )
					<< "splicing data of the size " << length() << " up into blocks of the size " << size << " is kind of useless ...";
		}

		const size_t fullSplices = length() / size;

		const size_t lastSize = length() % size;//rest of the division - size of the last splice

		const size_t splices = fullSplices + ( lastSize ? 1 : 0 );

		std::vector<Reference> ret( splices );

		DelProxy proxy( *this );

		for ( size_t i = 0; i < fullSplices; i++ )
			ret[i].reset( new TypePtr( m_val.get() + i * size, size, proxy ) );

		if ( lastSize )
			ret.back().reset( new TypePtr( m_val.get() + fullSplices * size, lastSize, proxy ) );

		return ret;
	}
	//

};

template<typename T> bool _internal::TypePtrBase::is()const
{
	util::check_type<T>();
	return typeID() == TypePtr<T>::staticID;
}


}
}
#endif // TYPEPTR_HPP
