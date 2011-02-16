/*
    Copyright (C) 2010  reimer@cbs.mpg.de

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

#include "../CoreUtils/type_base.hpp"
#include "typeptr_converter.hpp"

namespace isis
{
namespace data
{
namespace _internal
{

class ValuePtrBase : public util::_internal::GenericType
{
	friend class util::_internal::TypeReference<ValuePtrBase>;
	static const _internal::ValuePtrConverterMap &converters();
protected:
	size_t m_len;
	ValuePtrBase( size_t len = 0 );

	/// Create a ValuePtr of the same type pointing at the same address.
	virtual ValuePtrBase *clone()const = 0;

public:
	virtual const boost::weak_ptr<void> getRawAddress()const = 0;

	typedef util::_internal::TypeReference<ValuePtrBase> Reference;
	typedef ValuePtrConverterMap::mapped_type::mapped_type Converter;

	template<typename T> bool is()const;

	const Converter &getConverterTo( unsigned short ID )const;
	/**
	* Dynamically cast the TypeBase up to its actual ValuePtr\<T\>. Constant version.
	* Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	* \returns a constant reference of the pointer.
	*/
	template<typename T> const ValuePtr<T>& castToTypePtr() const {
		return m_cast_to<ValuePtr<T> >();
	}
	/**
	 * Dynamically cast the TypeBase up to its actual ValuePtr\<T\>. Referenced version.
	 * Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	 * \returns a reference of the pointer.
	 */
	template<typename T> ValuePtr<T>& castToTypePtr() {
		return m_cast_to<ValuePtr<T> >();
	}
	/// \returns the length of the data pointed to
	size_t length()const;

	/**
	 * Split up into cheap copies of given length.
	 * This will create ValuePtr's which will point at elements within this data block.
	 * - They will have a distance of size and therefore have the have the same length (exept the last one which will point an the rest).
	 * - They will use a special proxy-reference-counting (If at least one of them is still used, the whole original ValuePtr will be kept).
	 * \returns a vector of ceil(len()/size) not intersecting ValuePtrBase::Reference's of the length<=size.
	 */
	virtual std::vector<Reference> splice( size_t size )const = 0;

	/// Copy (or Convert) data from this to another ValuePtr of maybe another type and the same length.
	bool convertTo( ValuePtrBase &dst )const;
	bool convertTo( ValuePtrBase &dst, const scaling_pair &scaling )const;

	///get the scaling (and offset) which would be used in an convertTo
	scaling_pair getScalingTo( unsigned short typeID, autoscaleOption scaleopt = autoscale )const;
	scaling_pair getScalingTo( unsigned short typeID, const util::_internal::TypeBase &min, const util::_internal::TypeBase &max, autoscaleOption scaleopt = autoscale )const;
	scaling_pair getScalingTo( unsigned short typeID, const std::pair<util::TypeReference, util::TypeReference> &minmax, autoscaleOption scaleopt = autoscale )const;


	/// Convert (or Copy) data from this to existing memory of maybe another type and the given length.
	template<typename T> bool convertTo( T *dst, size_t len ) const {
		ValuePtr<T> dest( dst, len, ValuePtr<T>::NonDeleter() );
		return convertTo( dest );
	}

	/**
	 * Create new data in memory containg a (converted) copy of this.
	 * Allocates new memory of the requested ID and copies the content of this into that memory.
	 * \param ID the ID of the type the new ValuePtr (referenced by the Reference returned) should have
	 */
	Reference copyToNewByID( unsigned short ID ) const;
	/**
	 * @copydoc copyToNewByID
	 * \param ID the ID of the type the new ValuePtr (referenced by the Reference returned) should have
	 * \param scaling the scaling to be used if a conversion is necessary
	 */	
	Reference copyToNewByID( unsigned short ID, const scaling_pair &scaling ) const;
	
	/**
	 * Create a ValuePtr of given type and length.
	 * This allocates memory as needed but does not initialize it.
	 * \returns a Reference to a ValuePtr pointing to the allocated memory. Or an empty Reference if the creation failed.
	 */
	static Reference createById( unsigned short id, size_t len );

	/**
	 * Copy this to a new ValuePtr\<T\> using newly allocated memory.
	 * This will create a new ValuePtr of type T and the length of this.
	 * The memory will be allocated and the data of this will be copy-converted to T using min/max as value range.
	 * If the conversion fails, an error will be send to CoreLog and the data of the newly created ValuePtr will be undefined.
	 * \returns a the newly created ValuePtr
	 */
	template<typename T> ValuePtr<T> copyToNew( const scaling_pair &scaling )const {
		Reference ret = copyToNewByID( ValuePtr<T>::staticID, scaling );
		return ret->castToTypePtr<T>();
	}
	/**
	 * Copy this to a new ValuePtr\<T\> using newly allocated memory.
	 * This will create a new ValuePtr of type T and the length of this.
	 * The memory will be allocated and the data of this will be copy-converted to T.
	 * If the conversion fails, an error will be send to CoreLog and the data of the newly created ValuePtr will be undefined.
	 * \returns a the newly created ValuePtr
	 */
	template<typename T> ValuePtr<T> copyToNew()const {
		Reference ret = copyToNewByID( ValuePtr<T>::staticID );
		return ret->castToTypePtr<T>();
	}
	/**
	 * Create a new ValuePtr, of the same type, but differnent size in memory.
	 * \param length length of the new memory block in elements of the given TYPE
	 */
	virtual ValuePtrBase::Reference cloneToNew( size_t length )const = 0;
	ValuePtrBase::Reference cloneToNew();

	virtual size_t bytesPerElem()const = 0;
	virtual ~ValuePtrBase();
	/**
	 * Copy a range of elements to another ValuePtr of the same type.
	 * \param start first element in this to be copied
	 * \param end last element in this to be copied
	 * \param dst target for the copy
	 * \param dst_start starting element in dst to be overwritten
	 */
	void copyRange( size_t start, size_t end, ValuePtrBase &dst, size_t dst_start )const;

	/// \returns the number of references using the same memory as this.
	size_t useCount()const;

	/**
	 * Get minimum/maximum from a ValuePtr.
	 * This computes the minimum and maximum value of the stored data and stores them in TypeReference-Objects.
	 * The computes min/max are of the same type as the stored data, but can be compared to other TypeReference without knowing this type via the lt/gt function of TypeBase.
	 * The following code checks if the value range of ValuePtr-object data1 is a real subset of data2:
	 * \code
	 * std::pair<util::TypeReference,util::TypeReference> minmax1=data1.getMinMax(), minmax2=data2.getMinMax();
	 * if(minmax1.first->gt(minmax2.second) && minmax1.second->lt(minmax2.second)
	 *  std::cout << minmax1 << " is a subset of " minmax2 << std::endl;
	 * \endcode
	 * \returns a pair of TypeReferences referring to the found minimum/maximum of the data
	 */
	virtual std::pair<util::TypeReference, util::TypeReference> getMinMax()const = 0;
	/**
	 * Compare to another ValuePtr.
	 * This counts the elements between start and end, which are not equal to the corresponding elements in dst.
	 * If dst is of another type all element are assumed to be different
	 * \param start starting index for the comparison
	 * \param end end index for the comparison (this element the first element which is _not_ compared)
	 * \param dst the ValuePtr to compare against
	 * \param dst_start the index where to start comparison in dst
	 */
	virtual size_t compare( size_t start, size_t end, const ValuePtrBase &dst, size_t dst_start )const = 0;
	/**
	 * Compare to another ValuePtr.
	 * Short hand version of compare( size_t start, size_t end, const ValuePtrBase &dst, size_t dst_start )const
	 */
	size_t compare( const ValuePtrBase &comp )const;
};
}

typedef _internal::ValuePtrBase::Reference ValuePtrReference;

}
}

#endif // TYPEPTRBASE_HPP
