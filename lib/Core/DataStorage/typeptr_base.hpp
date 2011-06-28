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
#include "common.hpp"

namespace isis
{
namespace data
{
namespace _internal
{

class ValuePtrBase : public util::_internal::GenericValue
{
	friend class util::_internal::ValueReference<ValuePtrBase>;
	static const _internal::ValuePtrConverterMap &converters();
protected:
	size_t m_len;
	ValuePtrBase( size_t len = 0 );

	/// Create a ValuePtr of the same type pointing at the same address.
	virtual ValuePtrBase *clone()const = 0;

public:
	virtual const boost::weak_ptr<void> getRawAddress()const = 0;

	typedef util::_internal::ValueReference<ValuePtrBase> Reference;
	typedef ValuePtrConverterMap::mapped_type::mapped_type Converter;

	template<typename T> bool is()const;

	const Converter &getConverterTo( unsigned short ID )const;
	
	/**
	* Dynamically cast the ValueBase up to its actual ValuePtr\<T\>. Constant version.
	* Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	* \returns a constant reference of the ValuePtr.
	*/
	template<typename T> const ValuePtr<T>& castToValuePtr() const {
		return m_cast_to<ValuePtr<T> >();
	}
	
	/**
	 * Dynamically cast the ValueBase up to its actual ValuePtr\<T\>. Referenced version.
	 * Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	 * \returns a reference of the ValuePtr.
	 */
	template<typename T> ValuePtr<T>& castToValuePtr() {
		return m_cast_to<ValuePtr<T> >();
	}
	
	/// \returns the length (in elements) of the data pointed to
	size_t getLength()const;

	/**
	 * Splice up the ValuePtr into equal sized blocks.
	 * This virtually creates new data blocks of the given size by computing new pointers into the block and creating ValuePtr objects for them.
	 * This ValuePtr objects use the reference counting of the original ValuePtr via DelProxy, so the original data are only deleted (as a whole)
	 * when all spliced and all "normal" ValuePtr for this data are deleted.
	 * \param size the maximum size of the spliced parts of the data (the last part can be smaller)
	 * \returns a vector of references to ValuePtr's which point to the parts of the spliced data
	 */
	virtual std::vector<Reference> splice( size_t size )const = 0;

	///get the scaling (and offset) which would be used in an convertTo
	virtual scaling_pair getScalingTo( unsigned short typeID, autoscaleOption scaleopt = autoscale )const = 0;
	virtual scaling_pair getScalingTo( unsigned short typeID, const std::pair<util::ValueReference, util::ValueReference> &minmax, autoscaleOption scaleopt = autoscale )const;

	/**
	 * @copydoc copyToNewByID
	 * \param ID the ID of the type the new ValuePtr (referenced by the Reference returned) should have
	 * \param scaling the scaling to be used if a conversion is necessary (computed automatically if not given)
	 */
	Reference copyToNewByID( unsigned short ID, scaling_pair scaling = scaling_pair() ) const;

	/**
	 * Copies elements from this into another ValuePtr.
	 * This is allways a deep copy, regardless of the types.
	 * If necessary, a conversion will be done.
	 * If the this and the target are not of the same length:
	 * - the shorter length will be used
	 * - a warning about it will be sent to Debug
	 * \param dst the ValuePtr-object to copy into
	 * \param scaling the scaling to be used if a conversion is necessary (computed automatically if not given)
	 */
	bool copyTo(isis::data::_internal::ValuePtrBase& dst, scaling_pair scaling=scaling_pair() )const;

	/**
	 * Copies elements from this into raw memory.
	 * This is allways a deep copy, regardless of the types.
	 * If the this and the target are not of the same length:
	 * - the shorter length will be used
	 * - a warning about it will be sent to Debug
	 * \param dst pointer to the target memory
	 * \param len size (in elements) of the target memory
	 * \param scaling the scaling to be used if a conversion is necessary (computed automatically if not given)
	 */
	template<typename T> bool copyToMem(T *dst, size_t len, scaling_pair scaling=scaling_pair())const{
		ValuePtr<T> cont(dst,len, typename ValuePtr<T>::NonDeleter());
		return copyTo(cont,scaling);
	}

	/// Copy elements from raw memory
	template<typename T> bool copyFromMem( const T *const src, size_t _length, scaling_pair scaling=scaling_pair() ) {
		ValuePtr<T> cont(const_cast<T*>(src),_length, typename ValuePtr<T>::NonDeleter()); //its ok - we're no going to change it
		return cont.copyTo(cont,scaling);
	}


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
	template<typename T> ValuePtr<T> copyToNew( scaling_pair scaling = scaling_pair() )const {
		Reference ret = copyToNewByID( ValuePtr<T>::staticID, scaling );
		return ret->castToValuePtr<T>();
	}
	
	/**
	 * Create a new ValuePtr, of the same type, but differnent size in memory.
	 * (The actual data are _not_ copied)
	 * \param length length of the new memory block in elements of the given TYPE
	 */
	ValuePtrBase::Reference cloneToNew( size_t length )const;

	/// \returns the byte-size of the type of the data this ValuePtr points to.
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
	 * Get minimum/maximum of a ValuePtr.
	 * This computes the minimum and maximum value of the stored data and stores them in ValueReference-Objects.
	 * The computes min/max are of the same type as the stored data, but can be compared to other ValueReference without knowing this type via the lt/gt function of ValueBase.
	 * The following code checks if the value range of ValuePtr-object data1 is a real subset of data2:
	 * \code
	 * std::pair<util::ValueReference,util::ValueReference> minmax1=data1.getMinMax(), minmax2=data2.getMinMax();
	 * if(minmax1.first->gt(minmax2.second) && minmax1.second->lt(minmax2.second)
	 *  std::cout << minmax1 << " is a subset of " minmax2 << std::endl;
	 * \endcode
	 * \returns a pair of ValueReferences referring to the found minimum/maximum of the data
	 */
	virtual std::pair<util::ValueReference, util::ValueReference> getMinMax()const = 0;

	/**
	 * Compare the data of two ValuePtr.
	 * Counts how many elements in this and the given ValuePtr are different within the given range.
	 * If the type of this is not equal to the type of the given ValuePtr the whole length is assumed to be different.
	 * If the given range does not fit into this or the given ValuePtr an error is send to the runtime log and the function will probably crash.
	 * \param start the first element in this, which schould be compared to the first element in the given TyprPtr
	 * \param end the first element in this, which schould _not_ be compared anymore to the given TyprPtr
	 * \param dst the given ValuePtr this should be compared to
	 * \param dst_start the first element in the given TyprPtr, which schould be compared to the first element in this
	 * \returns the amount of elements which actually differ in both ValuePtr or the whole length of the range when the types are not equal.
	 */
	size_t compare( size_t start, size_t end, const ValuePtrBase &dst, size_t dst_start )const;
};
}

typedef _internal::ValuePtrBase::Reference ValuePtrReference;

}
}

#endif // TYPEPTRBASE_HPP
