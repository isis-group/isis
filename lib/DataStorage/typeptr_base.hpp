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

#ifndef TYPEPTRBASE_HPP
#define TYPEPTRBASE_HPP

#include "CoreUtils/type_base.hpp"
#include "typeptr_converter.hpp"

namespace isis{ namespace data{ namespace _internal{

class TypePtrBase : public util::_internal::GenericType
{
	friend class util::_internal::TypeReference<TypePtrBase>;
	static const _internal::TypePtrConverterMap &converters();
protected:
	size_t m_len;
	TypePtrBase( size_t len = 0 );
	virtual const boost::weak_ptr<void> address()const = 0;
	/// Create a TypePtr of the same type pointing at the same address.
	virtual TypePtrBase *clone()const = 0;
public:
	typedef util::_internal::TypeReference<TypePtrBase> Reference;
	typedef TypePtrConverterMap::mapped_type::mapped_type Converter;

	const Converter &getConverterTo( unsigned short id )const;
	/**
	* Dynamically cast the TypeBase up to its actual TypePtr\<T\>. Constant version.
	* Will throw std::bad_cast if T is not the actual type.
	* Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	* \returns a constant reference of the pointer.
	*/
	template<typename T> const TypePtr<T>& cast_to_TypePtr() const {
		return m_cast_to<TypePtr<T> >();
	}
	/**
	* Dynamically cast the TypeBase up to its actual TypePtr\<T\>. Referenced version.
	* Will throw std::bad_cast if T is not the actual type.
	* Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	* \returns a reference of the pointer.
	*/
	template<typename T> TypePtr<T>& cast_to_TypePtr() throw( std::bad_cast ) {
		return m_cast_to<TypePtr<T> >();
	}
	/// \returns the length of the data pointed to
	size_t len()const;

	virtual std::vector<Reference> splice( size_t size )const = 0;

	/** Create a TypePtr of the same type pointing at a newly allocated memory.
	 * This will not copy contents of this TypePtr, just its type and length.
	 * \returns a reference to the newly created TypePtr
	 */
	TypePtrBase::Reference cloneToMem()const;
	/**
	 * Copy this to a new TypePtr using newly allocated memory.
	 * This copies the contents of this TypePtr, its type and its length.
	 * \returns a reference to the newly created TypePtr
	 */
	TypePtrBase::Reference copyToMem()const;

	/// Copy (or Convert) data from this to another TypePtr of maybe another type and the same length.
	virtual bool convertTo( TypePtrBase &dst )const = 0;
	bool convertTo( TypePtrBase &dst, const util::_internal::TypeBase &min, const util::_internal::TypeBase &max )const;

	/// Copy (or Convert) data from this to memory of maybe another type and the given length.
	template<typename T> bool convertTo( T *dst, size_t len ) const {
		TypePtr<T> dest( dst, len, TypePtr<T>::NonDeleter() );
		return convertTo( dest );
	}

	/**
	 * Copy this to a new TypePtr\<T\> using newly allocated memory.
	 * This will create a new TypePtr of type T and the length of this.
	 * The memory will be allocated and the data of this will be copy-converted to T.
	 * If the conversion fails, an error will be send to CoreLog and the data of the newly created TypePtr will be undefined.
	 * \returns a the newly created TypePtr
	 */
	template<typename T> const TypePtr<T> copyToNew()const {
		TypePtr<T> ret( ( T * )malloc( sizeof( T )*len() ), len() );
		convertTo( ret );
		return ret;
	}
	/**
	 * Copy this to a new TypePtr\<T\> using newly allocated memory.
	 * This will create a new TypePtr of type T and the length of this.
	 * The memory will be allocated and the data of this will be copy-converted to T using min/max as value range.
	 * If the conversion fails, an error will be send to CoreLog and the data of the newly created TypePtr will be undefined.
	 * \returns a the newly created TypePtr
	 */
	template<typename T> const TypePtr<T> copyToNew( const util::_internal::TypeBase &min, const util::_internal::TypeBase &max )const {
		TypePtr<T> ret( ( T * )malloc( sizeof( T )*len() ), len() );
		convertTo( ret, min, max );
		return ret;
	}
	/**
	 * \copydoc cloneToMem
	 * \param length length of the new memory block in elements of the given TYPE
	 */
	virtual TypePtrBase::Reference cloneToMem( size_t length )const = 0;

	virtual size_t bytes_per_elem()const = 0;
	virtual ~TypePtrBase();
	/**
	 * Copy a range of elements to another TypePtr of the same type.
	 * \param start first element in this to be copied
	 * \param end last element in this to be copied
	 * \param dst_start starting element in dst to be overwritten
	 */
	void copyRange( size_t start, size_t end, TypePtrBase &dst, size_t dst_start )const;

	/**
	 * Get minimum/maximum from a TypePtr.
	 * The parameters are reverences to the current maximum/minimum found.
	 * max will be replaced by a value from the array if:
	 * - max is empty
	 * - max is less than that value from the array
	 *
	 * min will be replaced by a value from the array if:
	 * - min is empty
	 * - min is greater than that value from the array
	 *
	 * Note, that min/max will also adopt the type of the value.
	 * \param max TypeBase::Reference for the current greatest value
	 * \param min TypeBase::Reference for the current lowest value
	 */
	virtual void getMinMax( util::_internal::TypeBase::Reference &min, util::_internal::TypeBase::Reference &max )const = 0;
	virtual size_t cmp( size_t start, size_t end, const TypePtrBase &dst, size_t dst_start )const = 0;
	size_t cmp( const TypePtrBase &comp )const;
};

}}}

#endif // TYPEPTRBASE_HPP
