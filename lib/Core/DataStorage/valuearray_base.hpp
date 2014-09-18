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

#include "../CoreUtils/value_base.hpp"
#include "valuearray_converter.hpp"
#include "common.hpp"

namespace isis
{
namespace data
{
/// @cond _internal
namespace _internal
{
class ConstValueAdapter
{
	template<bool BB> friend class GenericValueIterator; //allow the iterators (and only them) to create the adapter
public:
	typedef const util::ValueReference ( *Getter )( const void * );
	typedef void ( *Setter )( void *, const util::ValueBase & );
protected:
	const Getter getter;
	const uint8_t *const p;
	ConstValueAdapter( const uint8_t *const _p, Getter _getValueFunc );
public:
	// to make some algorithms work
	bool operator==( const util::ValueReference &val )const;
	bool operator!=( const util::ValueReference &val )const;

	bool operator<( const util::ValueReference &val )const;
	bool operator>( const util::ValueReference &val )const;

	operator const util::ValueReference()const;
	const util::ValueReference operator->() const;
	const std::string toString( bool label = false )const;
};
class WritingValueAdapter: public ConstValueAdapter
{
	Setter setValueFunc;
	size_t byteSize;
	template<bool BB> friend class GenericValueIterator; //allow the iterators (and only them) to create the adapter
protected:
	WritingValueAdapter( uint8_t *const _p, Getter _getValueFunc, Setter _setValueFunc, size_t _size );
public:
	WritingValueAdapter operator=( const util::ValueReference &val );
	WritingValueAdapter operator=( const util::ValueBase &val );
	void swapwith( const WritingValueAdapter &b )const; // the WritingValueAdapter is const not what its dereferencing
};

template<bool IS_CONST> class GenericValueIterator :
	public std::iterator < std::random_access_iterator_tag,
	typename boost::mpl::if_c<IS_CONST, ConstValueAdapter, WritingValueAdapter>::type,
	ptrdiff_t,
	typename boost::mpl::if_c<IS_CONST, ConstValueAdapter, WritingValueAdapter>::type,
	typename boost::mpl::if_c<IS_CONST, ConstValueAdapter, WritingValueAdapter>::type
	>
{
	typedef typename boost::mpl::if_c<IS_CONST, const uint8_t *, uint8_t *>::type ptr_type;
	ptr_type p, start; //we need the starting position for operator[]
	size_t byteSize;
	ConstValueAdapter::Getter getValueFunc;
	ConstValueAdapter::Setter setValueFunc;
	friend class GenericValueIterator<true>; //yes, I'm my own friend, sometimes :-) (enables the constructor below)
public:
	GenericValueIterator( const GenericValueIterator<false> &src ): //will become additional constructor from non const if this is const, otherwise overrride the default copy contructor
		p( src.p ), start( src.p ), byteSize( src.byteSize ), getValueFunc( src.getValueFunc ), setValueFunc( src.setValueFunc ) {}
	GenericValueIterator(): p( NULL ), start( p ), byteSize( 0 ), getValueFunc( NULL ), setValueFunc( NULL ) {}
	GenericValueIterator( ptr_type _p, ptr_type _start, size_t _byteSize, ConstValueAdapter::Getter _getValueFunc, ConstValueAdapter::Setter _setValueFunc ):
		p( _p ), start( _start ), byteSize( _byteSize ), getValueFunc( _getValueFunc ), setValueFunc( _setValueFunc )
	{}

	GenericValueIterator<IS_CONST>& operator++() {p += byteSize; return *this;}
	GenericValueIterator<IS_CONST>& operator--() {p -= byteSize; return *this;}

	GenericValueIterator<IS_CONST> operator++( int ) {GenericValueIterator<IS_CONST> tmp = *this; ++*this; return tmp;}
	GenericValueIterator<IS_CONST> operator--( int ) {GenericValueIterator<IS_CONST> tmp = *this; --*this; return tmp;}

	typename GenericValueIterator<IS_CONST>::reference operator*() const;
	typename GenericValueIterator<IS_CONST>::pointer  operator->() const {return operator*();}

	bool operator==( const GenericValueIterator<IS_CONST>& cmp )const {return p == cmp.p;}
	bool operator!=( const GenericValueIterator<IS_CONST>& cmp )const {return !( *this == cmp );}

	bool operator>( const GenericValueIterator<IS_CONST> &cmp )const {return p > cmp.p;}
	bool operator<( const GenericValueIterator<IS_CONST> &cmp )const {return p < cmp.p;}

	bool operator>=( const GenericValueIterator<IS_CONST> &cmp )const {return p >= cmp.p;}
	bool operator<=( const GenericValueIterator<IS_CONST> &cmp )const {return p <= cmp.p;}

	typename GenericValueIterator<IS_CONST>::difference_type operator-( const GenericValueIterator<IS_CONST> &cmp )const {return ( p - cmp.p ) / byteSize;}

	GenericValueIterator<IS_CONST> operator+( typename GenericValueIterator<IS_CONST>::difference_type n )const
	{return ( GenericValueIterator<IS_CONST>( *this ) += n );}
	GenericValueIterator<IS_CONST> operator-( typename GenericValueIterator<IS_CONST>::difference_type n )const
	{return ( GenericValueIterator<IS_CONST>( *this ) -= n );}


	GenericValueIterator<IS_CONST> &operator+=( typename GenericValueIterator<IS_CONST>::difference_type n )
	{p += ( n * byteSize ); return *this;}
	GenericValueIterator<IS_CONST> &operator-=( typename GenericValueIterator<IS_CONST>::difference_type n )
	{p -= ( n * byteSize ); return *this;}

	typename GenericValueIterator<IS_CONST>::reference operator[]( typename GenericValueIterator<IS_CONST>::difference_type n )const {
		//the book says it has to be the n-th elements of the whole object, so we have to start from what is hopefully the beginning
		return *( GenericValueIterator<IS_CONST>( start, start, byteSize, getValueFunc, setValueFunc ) += n );
	}

};
//specialise the dereferencing operators. They have to return (and create) different objects with different constructors
template<> GenericValueIterator<true>::reference GenericValueIterator<true>::operator*() const;
template<> GenericValueIterator<false>::reference GenericValueIterator<false>::operator*() const;

} //namespace _internal
/// @endcond _internal

class ValueArrayBase : public util::_internal::GenericValue
{
	friend class util::_internal::GenericReference<ValueArrayBase>;
	static const _internal::ValueArrayConverterMap &converters();
	scaling_pair getScaling( const scaling_pair &scale, unsigned short ID )const;
protected:
	size_t m_len;
	ValueArrayBase( size_t len = 0 );

	/// Create a ValueArray of the same type pointing at the same address.
	virtual ValueArrayBase *clone()const = 0;

	void operator=(const ValueArrayBase &ref){m_len=ref.m_len;}//prevent direct usage

public:
	typedef _internal::GenericValueIterator<false> value_iterator;
	typedef _internal::GenericValueIterator<true> const_value_iterator;
	typedef value_iterator value_reference;
	typedef const_value_iterator const_value_reference;
	/// Proxy-Deleter to encapsulate the real deleter/shared_ptr when creating shared_ptr for parts of a shared_ptr
	class DelProxy : public std::shared_ptr<const void>
	{
	public:
		/**
		 * Create a proxy for a given master shared_ptr
		 * This increments the use_count of the master and thus keeps the
		 * master from being deleted while parts of it are still in use.
		 */
		DelProxy( const ValueArrayBase &master );
		/// decrement the use_count of the master when a specific part is not referenced anymore
		void operator()( const void *at );
	};

	/**
	 * Get the raw address the ValueArray points to.
	 * An offset can be added to the result. If it is not zero, the resulting shared_ptr will use DelProxy as deleter.
	 * Thus, it will increase  the reference count of the original pointer by one and decrease it when the deletion of the offset pointer is triggered.
	 * \param offset ammount of bytes to displace the resulting pointer from the actual pointer
	 * \returns a shared_ptr with the memory address of the data handled by this ValueArray.
	 */
	virtual std::shared_ptr<const void> getRawAddress( size_t offset = 0 )const = 0;

	/// \copydoc getRawAddress
	virtual std::shared_ptr<void> getRawAddress( size_t offset = 0 ) = 0;

	virtual value_iterator beginGeneric() = 0;
	value_iterator endGeneric();
	virtual const_value_iterator beginGeneric()const = 0;
	const_value_iterator endGeneric()const;

	typedef util::_internal::GenericReference<ValueArrayBase> Reference;
	typedef _internal::ValueArrayConverterMap::mapped_type::mapped_type Converter;

	template<typename T> bool is()const;

	const Converter &getConverterTo( unsigned short ID )const;

	/**
	* Dynamically cast the ValueBase up to its actual ValueArray\<T\>. Constant version.
	* Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	* \returns a constant reference of the ValueArray.
	*/
	template<typename T> const ValueArray<T>& castToValueArray() const {
		return m_cast_to<ValueArray<T> >();
	}

	/**
	 * Dynamically cast the ValueBase up to its actual ValueArray\<T\>. Referenced version.
	 * Will send an error if T is not the actual type and _ENABLE_CORE_LOG is true.
	 * \returns a reference of the ValueArray.
	 */
	template<typename T> ValueArray<T>& castToValueArray() {
		return m_cast_to<ValueArray<T> >();
	}

	/// \returns the length (in elements) of the data pointed to
	size_t getLength()const;

	/**
	 * Splice up the ValueArray into equal sized blocks.
	 * This virtually creates new data blocks of the given size by computing new pointers into the block and creating ValueArray objects for them.
	 * This ValueArray objects use the reference counting of the original ValueArray via DelProxy, so the original data are only deleted (as a whole)
	 * when all spliced and all "normal" ValueArray for this data are deleted.
	 * \param size the maximum size of the spliced parts of the data (the last part can be smaller)
	 * \returns a vector of references to ValueArray's which point to the parts of the spliced data
	 */
	virtual std::vector<Reference> splice( size_t size )const = 0;

	///get the scaling (and offset) which would be used in an conversion
	virtual scaling_pair getScalingTo( unsigned short typeID, autoscaleOption scaleopt = autoscale )const = 0;
	virtual scaling_pair getScalingTo( unsigned short typeID, const std::pair<util::ValueReference, util::ValueReference> &minmax, autoscaleOption scaleopt = autoscale )const;

	/**
	 * Create new data in memory containg a (converted) copy of this.
	 * Allocates new memory of the requested type and copies the (converted) content of this into that memory.
	 * \param ID the ID of the type the new ValueArray (referenced by the Reference returned) should have (if not given, type of the source is used)
	 * \param scaling the scaling to be used if a conversion is necessary (computed automatically if not given)
	 */
	Reference copyByID( unsigned short ID = 0, scaling_pair scaling = scaling_pair() ) const;

	/**
	 * Copies elements from this into another ValueArray.
	 * This is allways a deep copy, regardless of the types.
	 * If necessary, a conversion will be done.
	 * If the this and the target are not of the same length:
	 * - the shorter length will be used
	 * - a warning about it will be sent to Debug
	 * \param dst the ValueArray-object to copy into
	 * \param scaling the scaling to be used if a conversion is necessary (computed automatically if not given)
	 */
	bool copyTo( isis::data::ValueArrayBase &dst, scaling_pair scaling = scaling_pair() )const;

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
	template<typename T> bool copyToMem( T *dst, size_t len, scaling_pair scaling = scaling_pair() )const {
		ValueArray<T> cont( dst, len, typename ValueArray<T>::NonDeleter() );
		return copyTo( cont, scaling );
	}

	/**
	 * Copies elements from raw memory into  this.
	 * This is allways a deep copy, regardless of the types.
	 * If the this and the target are not of the same length:
	 * - the shorter length will be used
	 * - a warning about it will be sent to Debug
	 * \param src pointer to the target memory
	 * \param len size (in elements) of the target memory
	 * \param scaling the scaling to be used if a conversion is necessary (computed automatically if not given)
	 */
	template<typename T> bool copyFromMem( const T *const src, size_t len, scaling_pair scaling = scaling_pair() ) {
		ValueArray<T> cont( const_cast<T *>( src ), len, typename ValueArray<T>::NonDeleter() ); //its ok - we're no going to change it
		return cont.copyTo( *this, scaling );
	}

	/**
	 * Create a ValueArray of given type and length.
	 * This allocates memory as needed but does not initialize it.
	 * \returns a Reference to a ValueArray pointing to the allocated memory. Or an empty Reference if the creation failed.
	 */
	static Reference createByID( unsigned short ID, size_t len );

	/**
	 * Copy this to a new ValueArray\<T\> using newly allocated memory.
	 * This will create a new ValueArray of type T and the length of this.
	 * The memory will be allocated and the data of this will be copy-converted to T using min/max as value range.
	 * If the conversion fails, an error will be send to CoreLog and the data of the newly created ValueArray will be undefined.
	 * \returns a the newly created ValueArray
	 */
	template<typename T> ValueArray<T> copyAs( scaling_pair scaling = scaling_pair() )const {
		Reference erg = copyByID( ValueArray<T>::staticID(), scaling );
		return erg.isEmpty() ? ValueArray<T>( 0 ) : erg->castToValueArray<T>();
	}

	/**
	 * Get this as a ValueArray of a specific type.
	 * This does an automatic conversion into a new ValueArray if one of following is true:
	 * - the target type is not the current type
	 * - scaling.first (the scaling factor) is not 1
	 * - scaling.first (the scaling offset) is not 0
	 *
	 * Otherwise a cheap copy is done.
	 * \param ID the ID of the requeseted type (use ValueArray::staticID())
	 * \param scaling the scaling to be used (determined automatically if not given)
	 * \returns a reference of eigther a cheap copy or a newly created ValueArray
	 * \returns an empty reference if the conversion failed
	 */
	ValueArrayBase::Reference  convertByID( unsigned short ID, scaling_pair scaling = scaling_pair() );


	/**
	 * Get this as a ValueArray of a specific type.
	 * This does an automatic conversion into a new ValueArray if one of following is true:
	 * - the target type is not the current type
	 * - scaling.first (the scaling factor) is not 1
	 * - scaling.first (the scaling offset) is not 0
	 *
	 * Otherwise a cheap copy is done.
	 * \param scaling the scaling to be used (determined automatically if not given)
	 * \returns eigther a cheap copy or a newly created ValueArray
	 */
	template<typename T> ValueArray<T> as( scaling_pair scaling = scaling_pair() ) {
		Reference erg = convertByID( ValueArray<T>::staticID(), scaling );
		return erg.isEmpty() ? ValueArray<T>( 0 ) : erg->castToValueArray<T>();
	}

	/**
	 * Create a new ValueArray, of the same type, but differnent size in memory.
	 * (The actual data are _not_ copied)
	 * \param length length of the new memory block in elements of the given TYPE
	 */
	ValueArrayBase::Reference cloneToNew( size_t length )const;

	/// \returns the byte-size of the type of the data this ValueArray points to.
	virtual size_t bytesPerElem()const = 0;

	virtual ~ValueArrayBase();

	/**
	 * Copy a range of elements to another ValueArray of the same type.
	 * \param start first element in this to be copied
	 * \param end last element in this to be copied
	 * \param dst target for the copy
	 * \param dst_start starting element in dst to be overwritten
	 */
	void copyRange( size_t start, size_t end, ValueArrayBase &dst, size_t dst_start )const;

	/// \returns the number of references using the same memory as this.
	size_t useCount()const;

	/**
	 * Get minimum/maximum of a ValueArray.
	 * This computes the minimum and maximum value of the stored data and stores them in ValueReference-Objects.
	 * This actually returns the bounding box of the values in the value space of the type. This means:
	 * - min/max numbers for numbers from a 1-D value space (aka real numbers)
	 * - complex(lowest real value,lowest imaginary value) / complex(biggest real value,biggest imaginary value) for complex numbers
	 * - color(lowest red value,lowest green value, lowest blue value)/color(biggest red value,biggest green value, biggest blue value) for color
	 * The computed min/max are of the same type as the stored data, but can be compared to other ValueReference without knowing this type via the lt/gt function of ValueBase.
	 * The following code checks if the value range of ValueArray-object data1 is a real subset of data2:
	 * \code
	 * std::pair<util::ValueReference,util::ValueReference> minmax1=data1.getMinMax(), minmax2=data2.getMinMax();
	 * if(minmax1.first->gt(minmax2.second) && minmax1.second->lt(minmax2.second)
	 *  std::cout << minmax1 << " is a subset of " minmax2 << std::endl;
	 * \endcode
	 * \returns a pair of ValueReferences referring to the found minimum/maximum of the data
	 */
	virtual std::pair<util::ValueReference, util::ValueReference> getMinMax()const = 0;

	/**
	 * Compare the data of two ValueArray.
	 * Counts how many elements in this and the given ValueArray are different within the given range.
	 * If the type of this is not equal to the type of the given ValueArray the whole length is assumed to be different.
	 * If the given range does not fit into this or the given ValueArray an error is send to the runtime log and the function will probably crash.
	 * \param start the first element in this, which schould be compared to the first element in the given TyprPtr
	 * \param end the first element in this, which should _not_ be compared anymore to the given TyprPtr
	 * \param dst the given ValueArray this should be compared to
	 * \param dst_start the first element in the given TyprPtr, which schould be compared to the first element in this
	 * \returns the amount of elements which actually differ in both ValueArray or the whole length of the range when the types are not equal.
	 */
	size_t compare( size_t start, size_t end, const ValueArrayBase &dst, size_t dst_start )const;

	virtual void endianSwap() = 0;
};


typedef ValueArrayBase::Reference ValueArrayReference;

}
}

namespace std
{
	void swap(const isis::data::_internal::WritingValueAdapter &a,const isis::data::_internal::WritingValueAdapter &b);
	/// Streaming output for ConstValueAdapter (use it as a const ValueReference)
	template<typename charT, typename traits> basic_ostream<charT, traits>&
	operator<<( basic_ostream<charT, traits> &out, const isis::data::_internal::ConstValueAdapter &v )
	{
		return out << (isis::util::ValueReference)v;
	}
}
#endif // TYPEPTRBASE_HPP
