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

namespace isis
{
namespace data
{
namespace _internal
{

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

	template<typename T> bool is()const;

	const Converter &getConverterTo( unsigned short id )const;
	/**
	* Dynamically cast the TypeBase up to its actual TypePtr\<T\>. Constant version.
	* Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	* \returns a constant reference of the pointer.
	*/
	template<typename T> const TypePtr<T>& castToTypePtr() const {
		return m_cast_to<TypePtr<T> >();
	}
	/**
	 * Dynamically cast the TypeBase up to its actual TypePtr\<T\>. Referenced version.
	 * Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	 * \returns a reference of the pointer.
	 */
	template<typename T> TypePtr<T>& castToTypePtr() {
		return m_cast_to<TypePtr<T> >();
	}
	/// \returns the length of the data pointed to
	size_t length()const;

	/**
	 * Split up into cheap copies of given length.
	 * This will create TypePtr's which will point at elements within this data block.
	 * - They will have a distance of size and therefore have the have the same length (exept the last one which will point an the rest).
	 * - They will use a special proxy-reference-counting (If at least one of them is still used, the whole original TypePtr will be kept).
	 * \returns a vector of ceil(len()/size) not intersecting TypePtrBase::Reference's of the length<=size.
	 */
	virtual std::vector<Reference> splice( size_t size )const = 0;

	/// Copy (or Convert) data from this to another TypePtr of maybe another type and the same length.
	bool convertTo( TypePtrBase &dst )const;
	bool convertTo( TypePtrBase &dst, const scaling_pair &scaling )const;

	///get the scaling (and offset) which would be used in an convertTo 
	scaling_pair getScalingTo( unsigned short typeID, autoscaleOption scaleopt = autoscale )const;
	scaling_pair getScalingTo( unsigned short typeID, const util::_internal::TypeBase &min, const util::_internal::TypeBase &max, autoscaleOption scaleopt = autoscale )const;
	

	/// Copy (or Convert) data from this to memory of maybe another type and the given length.
	template<typename T> bool convertTo( T *dst, size_t len ) const {
		TypePtr<T> dest( dst, len, TypePtr<T>::NonDeleter() );
		return convertTo( dest );
	}

	Reference copyToNewById( unsigned short id ) const;
	Reference copyToNewById( unsigned short id, const scaling_pair &scaling ) const;

	/**
	 * Copy this to a new TypePtr\<T\> using newly allocated memory.
	 * This will create a new TypePtr of type T and the length of this.
	 * The memory will be allocated and the data of this will be copy-converted to T using min/max as value range.
	 * If the conversion fails, an error will be send to CoreLog and the data of the newly created TypePtr will be undefined.
	 * \returns a the newly created TypePtr
	 */
	template<typename T> TypePtr<T> copyToNew( const scaling_pair &scaling )const {
		Reference ret = copyToNewById( TypePtr<T>::staticID, scaling );
		return ret->castToTypePtr<T>();
	}
	/**
	 * Copy this to a new TypePtr\<T\> using newly allocated memory.
	 * This will create a new TypePtr of type T and the length of this.
	 * The memory will be allocated and the data of this will be copy-converted to T.
	 * If the conversion fails, an error will be send to CoreLog and the data of the newly created TypePtr will be undefined.
	 * \returns a the newly created TypePtr
	 */
	template<typename T> TypePtr<T> copyToNew()const {
		Reference ret = copyToNewById( TypePtr<T>::staticID );
		return ret->castToTypePtr<T>();
	}
	/**
	 * Create a new TypePtr, of the same type, but differnent size in memory.
	 * \param length length of the new memory block in elements of the given TYPE
	 */
	virtual TypePtrBase::Reference cloneToNew( size_t length )const = 0;
	TypePtrBase::Reference cloneToNew();

	virtual size_t bytesPerElem()const = 0;
	virtual ~TypePtrBase();
	/**
	 * Copy a range of elements to another TypePtr of the same type.
	 * \param start first element in this to be copied
	 * \param end last element in this to be copied
	 * \param dst target for the copy
	 * \param dst_start starting element in dst to be overwritten
	 */
	void copyRange( size_t start, size_t end, TypePtrBase &dst, size_t dst_start )const;

	size_t useCount()const;

	bool swapAlong( TypePtrBase &dst, const size_t dim, const size_t dims[] ) const;
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
	virtual void getMinMax( util::TypeReference &min, util::TypeReference &max )const = 0;
	/**
	 * Compare to another TypePtr.
	 * This counts the elements between start and end, which are not equal to the corresponding elements in dst.
	 * If dst is of another type all element are assumed to be different
	 * \param start starting index for the comparison
	 * \param end end index for the comparison (this element the first element which is _not_ compared)
	 * \param dst the TypePtr to compare against
	 * \param dst_start the index where to start comparison in dst
	 */
	virtual size_t compare( size_t start, size_t end, const TypePtrBase &dst, size_t dst_start )const = 0;
	/**
	 * Compare to another TypePtr.
	 * Short hand version of compare( size_t start, size_t end, const TypePtrBase &dst, size_t dst_start )const
	 */
	size_t compare( const TypePtrBase &comp )const;
};
}

typedef _internal::TypePtrBase::Reference TypePtrReference;

}
}

#endif // TYPEPTRBASE_HPP
