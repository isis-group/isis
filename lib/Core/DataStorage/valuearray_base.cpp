#ifdef _MSC_VER
#define NOMINMAX 1
#endif

#include "valuearray_base.hpp"
#include "valuearray_converter.hpp"
#include "common.hpp"

namespace isis
{
namespace data
{

/**
 * Helper to sanitise scaling.
 * \retval scaling if !(scaling.first.isEmpty() || scaling.second.isEmpty())
 * \retval 1/0 if current type is equal to the requested type
 * \retval ValueArrayBase::getScalingTo elswise
 */
scaling_pair ValueArrayBase::getScaling( const scaling_pair &scaling, short unsigned int ID )const
{
	if( scaling.first.isEmpty() || scaling.second.isEmpty() )
		return getScalingTo( ID );
	else
		return scaling;
}

ValueArrayBase::ValueArrayBase( size_t length ): m_len( length ) {}

size_t ValueArrayBase::getLength() const { return m_len;}

ValueArrayBase::~ValueArrayBase() {}

ValueArrayBase::DelProxy::DelProxy( const isis::data::ValueArrayBase &master ): boost::shared_ptr<const void>( master.getRawAddress() )
{
	LOG( Debug, verbose_info ) << "Creating DelProxy for " << master.getTypeName() << " at " << this->get();
}

void ValueArrayBase::DelProxy::operator()( const void *at )
{
	LOG( Debug, verbose_info )
			<< "Deletion for " << this->get() << " called from proxy at offset "
			<< static_cast<const uint8_t *>( at ) - static_cast<const uint8_t *>( this->get() )
			<< ", current use_count: " << this->use_count();
	this->reset();//actually not needed, but we keep it here to keep obfuscation low
}


const _internal::ValueArrayConverterMap &ValueArrayBase::converters()
{
	static _internal::ValueArrayConverterMap ret; //@todo not using class Singleton because ValueArrayConverterMap is hidden
	return ret;
}

const ValueArrayBase::Converter &ValueArrayBase::getConverterTo( unsigned short ID )const
{
	const _internal::ValueArrayConverterMap::const_iterator f1 = converters().find( getTypeID() );
	LOG_IF( f1 == converters().end(), Debug, error ) << "There is no known conversion from " << util::getTypeMap()[getTypeID()];
	const _internal::ValueArrayConverterMap::mapped_type::const_iterator f2 = f1->second.find( ID );
	LOG_IF( f2 == f1->second.end(), Debug, error ) << "There is no known conversion from " << util::getTypeMap()[getTypeID()] << " to " << util::getTypeMap()[ID];
	return f2->second;
}

size_t ValueArrayBase::compare( size_t start, size_t end, const ValueArrayBase &dst, size_t dst_start ) const
{
	assert( start <= end );
	size_t ret = 0;
	size_t _length = end - start;

	if ( dst.getTypeID() != getTypeID() ) {
		LOG( Debug, error )
				<< "Comparing to a ValueArray of different type(" << dst.getTypeName() << ", not " << getTypeName()
				<< "). Assuming all voxels to be different";
		return _length;
	}

	LOG_IF( end >= getLength(), Runtime, error )
			<< "End of the range (" << end << ") is behind the end of this ValueArray (" << getLength() << ")";
	LOG_IF( _length + dst_start >= dst.getLength(), Runtime, error )
			<< "End of the range (" << _length + dst_start << ") is behind the end of the destination (" << dst.getLength() << ")";

	// lock the memory so we can mem-compare the elements (use uint8_t because some compilers do not like arith on void*)
	const boost::shared_ptr<const uint8_t>
	src_s = boost::static_pointer_cast<const uint8_t>( getRawAddress() ),
	dst_s = boost::static_pointer_cast<const uint8_t>( dst.getRawAddress() );
	const uint8_t *src_p = src_s.get(), *dst_p = dst_s.get();
	const size_t el_size = bytesPerElem();

	for ( size_t i = start; i < end; i++ ) {
		if ( memcmp( src_p + ( i * el_size ), dst_p + ( i * el_size ), el_size ) != 0 )
			ret++;
	}

	return ret;
}


ValueArrayBase::Reference ValueArrayBase::copyByID( unsigned short ID, scaling_pair scaling ) const
{
	const Converter &conv = getConverterTo( ID );

	if( conv ) {
		boost::scoped_ptr<ValueArrayBase> ret;
		conv->generate( *this, ret, getScaling( scaling, ID ) );
		return *ret;
	} else {
		LOG( Runtime, error )
				<< "I dont know any conversion from "
				<< util::MSubject( getTypeName() ) << " to " << util::MSubject( util::getTypeMap( false, true )[ID] );
		return Reference(); // return an empty Reference
	}
}

bool ValueArrayBase::copyTo( isis::data::ValueArrayBase &dst, scaling_pair scaling ) const
{
	const unsigned short dID = dst.getTypeID();
	const Converter &conv = getConverterTo( dID );

	if( conv ) {
		conv->convert( *this, dst, getScaling( scaling, dID ) );
		return true;
	} else {
		LOG( Runtime, error ) << "I dont know any conversion from " << util::MSubject( toString( true ) ) << " to " << dst.getTypeName();
		return false;
	}
}


ValueArrayBase::Reference ValueArrayBase::createByID( unsigned short ID, size_t len )
{
	const _internal::ValueArrayConverterMap::const_iterator f1 = converters().find( ID );
	_internal::ValueArrayConverterMap::mapped_type::const_iterator f2;

	// try to get a converter to convert the requestet type into itself - they 're there for all known types
	if( f1 != converters().end() && ( f2 = f1->second.find( ID ) ) != f1->second.end() ) {
		const _internal::ValueArrayConverterBase &conv = *( f2->second );
		boost::scoped_ptr<ValueArrayBase> ret;
		conv.create( ret, len );
		return *ret;
	} else {
		LOG( Debug, error ) << "There is no known creator for " << util::getTypeMap()[ID];
		return Reference(); // return an empty Reference
	}
}

ValueArrayBase::Reference ValueArrayBase::convertByID( short unsigned int ID, scaling_pair scaling )
{
	scaling = getScaling( scaling, ID );
	static const util::Value<uint8_t> one( 1 );
	static const util::Value<uint8_t> zero( 0 );

	if( scaling.first->eq( one ) && scaling.second->eq( zero ) && getTypeID() == ID ) { // if type is the same and scaling is 1/0
		return *this; //cheap copy
	} else {
		return copyByID( ID, scaling ); // convert into new
	}
}

ValueArrayBase::Reference ValueArrayBase::cloneToNew( size_t length ) const
{
	return createByID( getTypeID(), length );
}


void ValueArrayBase::copyRange( size_t start, size_t end, ValueArrayBase &dst, size_t dst_start )const
{
	assert( start <= end );
	const size_t len = end - start + 1;
	LOG_IF( ! dst.isSameType( *this ), Debug, error )
			<< "Range copy into a ValueArray of different type is not supportet. Its " << dst.getTypeName() << " not " << getTypeName();

	if( end >= getLength() ) {
		LOG( Runtime, error )
				<< "End of the range (" << end << ") is behind the end of this ValueArray (" << getLength() << ")";
	} else if( len + dst_start > dst.getLength() ) {
		LOG( Runtime, error )
				<< "End of the range (" << len + dst_start << ") is behind the end of the destination (" << dst.getLength() << ")";
	} else {
		boost::shared_ptr<void> daddr = dst.getRawAddress();
		const boost::shared_ptr<const void> saddr = getRawAddress();
		const size_t soffset = bytesPerElem() * start; //source offset in bytes
		const int8_t *const  src = ( int8_t * )saddr.get();
		const size_t doffset = bytesPerElem() * dst_start;//destination offset in bytes
		int8_t *const  dest = ( int8_t * )daddr.get();
		const size_t blength = len * bytesPerElem();//length in bytes
		memcpy( dest + doffset, src + soffset, blength );
	}
}

scaling_pair ValueArrayBase::getScalingTo( unsigned short typeID, const std::pair<util::ValueReference, util::ValueReference> &minmax, autoscaleOption scaleopt )const
{
	LOG_IF( minmax.first.isEmpty() || minmax.second.isEmpty(), Debug, error ) << "One of the ValueReference's in minmax is empty(). This will crash...";
	const Converter &conv = getConverterTo( typeID );

	if ( conv ) {
		return conv->getScaling( *minmax.first, *minmax.second, scaleopt );
	} else {
		LOG( Runtime, error )
				<< "I dont know any conversion from " << util::MSubject( getTypeName() ) << " to " << util::MSubject( util::getTypeMap( false, true )[typeID] );
		return scaling_pair();
	}
}
size_t ValueArrayBase::useCount() const
{
	return getRawAddress().use_count();
}
ValueArrayBase::value_iterator ValueArrayBase::endGeneric()
{
	return beginGeneric() + m_len;
}
ValueArrayBase::const_value_iterator ValueArrayBase::endGeneric()const
{
	return beginGeneric() + m_len;
}

/// @cond _internal
namespace _internal
{
template<> GenericValueIterator<true>::reference GenericValueIterator<true>::operator*() const
{
	assert( getValueFunc );
	return ConstValueAdapter( p, getValueFunc );
}
template<> GenericValueIterator<false>::reference GenericValueIterator<false>::operator*() const
{
	assert( getValueFunc );
	return WritingValueAdapter( p, getValueFunc, setValueFunc );
}

ConstValueAdapter::ConstValueAdapter( const uint8_t *const _p, Getter _getValueFunc ): util::ValueReference( _getValueFunc( _p ) ), p( _p ) {}
bool ConstValueAdapter::operator==( const util::ValueReference &val )const {return ( *this )->eq( *val );}
bool ConstValueAdapter::operator!=( const util::ValueReference &val )const {return !operator==( val );}

bool ConstValueAdapter::operator<( const util::ValueReference &val )const {return ( *this )->lt( *val );}
bool ConstValueAdapter::operator>( const util::ValueReference &val )const {return ( *this )->gt( *val );}

WritingValueAdapter::WritingValueAdapter( uint8_t *const _p, Getter _getValueFunc, Setter _setValueFunc ): ConstValueAdapter( _p, _getValueFunc ), setValueFunc( _setValueFunc ) {}
WritingValueAdapter WritingValueAdapter::operator=( const util::ValueReference &val )
{
	assert( setValueFunc );
	setValueFunc( const_cast<uint8_t * const>( p ), *val );
	return *this;
}
} // namespace _internal
/// @endcond _internal
}
}
