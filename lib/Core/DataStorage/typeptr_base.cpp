#ifdef _MSC_VER
#define NOMINMAX 1
#endif

#include "typeptr_base.hpp"
#include "typeptr_converter.hpp"
#include "common.hpp"

namespace isis
{
namespace data
{
namespace _internal
{

scaling_pair ValuePtrBase::getScaling(const scaling_pair& scaling, short unsigned int ID)const
{
	if(scaling.first.isEmpty() || scaling.second.isEmpty())
		return getScalingTo(ID);
	else
		return scaling;
}

ValuePtrBase::ValuePtrBase( size_t length ): m_len( length ) {}

size_t ValuePtrBase::getLength() const { return m_len;}

ValuePtrBase::~ValuePtrBase() {}

const ValuePtrConverterMap &ValuePtrBase::converters()
{
	return util::Singletons::get<_internal::ValuePtrConverterMap, 0>();
}

const ValuePtrBase::Converter &ValuePtrBase::getConverterTo( unsigned short ID )const
{
	const ValuePtrConverterMap::const_iterator f1 = converters().find( getTypeID() );
	LOG_IF( f1 == converters().end(), Debug, error ) << "There is no known conversion from " << util::getTypeMap()[getTypeID()];
	const ValuePtrConverterMap::mapped_type::const_iterator f2 = f1->second.find( ID );
	LOG_IF( f2 == f1->second.end(), Debug, error ) << "There is no known conversion from " << util::getTypeMap()[getTypeID()] << " to " << util::getTypeMap()[ID];
	return f2->second;
}

size_t ValuePtrBase::compare( size_t start, size_t end, const _internal::ValuePtrBase &dst, size_t dst_start ) const {
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

	// lock the memory so we can mem-compare the elements (use uint8_t because some compilers do not like arith on void*)
	const boost::shared_ptr<const uint8_t>
		src_s = boost::static_pointer_cast<const uint8_t>(getRawAddress().lock()),
		dst_s = boost::static_pointer_cast<const uint8_t>(dst.getRawAddress().lock());
	const uint8_t *src_p=src_s.get(),*dst_p=dst_s.get();
	const size_t el_size=bytesPerElem();

	for ( size_t i = start; i < end; i++ ) {
		if ( memcmp(src_p+(i*el_size), dst_p+(i*el_size), el_size) != 0)
			ret++;
	}

	return ret;
}


ValuePtrBase::Reference ValuePtrBase::copyToNewByID( unsigned short ID, scaling_pair scaling ) const
{
	const Converter &conv = getConverterTo( ID );

	if( conv ) {
		boost::scoped_ptr<ValuePtrBase> ret;
		conv->generate( *this, ret, getScaling(scaling,ID) );
		return *ret;
	} else {
		LOG( Runtime, error )
				<< "I dont know any conversion from "
				<< util::MSubject( getTypeName() ) << " to " << util::MSubject( util::getTypeMap( false, true )[ID] );
		return Reference(); // return an empty Reference
	}
}

bool ValuePtrBase::copyTo(isis::data::_internal::ValuePtrBase& dst, scaling_pair scaling) const
{
	const unsigned short dID=dst.getTypeID();
	const Converter &conv = getConverterTo( dID );

	if( conv ) {
		conv->convert(*this,dst,getScaling(scaling,dID));
		return true;
	} else {
		LOG( Runtime, error ) << "I dont know any conversion from "	<< util::MSubject( toString( true ) ) << " to " << dst.getTypeName();
		return false;
	}
}


ValuePtrBase::Reference ValuePtrBase::createByID( unsigned short ID, size_t len )
{
	const ValuePtrConverterMap::const_iterator f1 = converters().find( ID );
	ValuePtrConverterMap::mapped_type::const_iterator f2;

	// try to get a converter to convert the requestet type into itself - they 're there for all known types
	if( f1 != converters().end() && ( f2 = f1->second.find( ID ) ) != f1->second.end() ) {
		const _internal::ValuePtrConverterBase &conv = *( f2->second );
		boost::scoped_ptr<ValuePtrBase> ret;
		conv.create( ret, len );
		return *ret;
	} else {
		LOG( Debug, error ) << "There is no known creator for " << util::getTypeMap()[ID];
		return Reference(); // return an empty Reference
	}
}

ValuePtrBase::Reference ValuePtrBase::convertToID(short unsigned int ID, scaling_pair scaling)
{
	if(scaling.first.isEmpty() && scaling.second.isEmpty() && getTypeID()==ID ){ // if type is the same, scaling is not given
		return *this; //cheap copy
	} else {
		return copyToNewByID(ID,scaling); // convert into new
	}
}

ValuePtrBase::Reference ValuePtrBase::cloneToNew(size_t length) const
{
	return createByID(getTypeID(),length);
}


void ValuePtrBase::copyRange( size_t start, size_t end, ValuePtrBase &dst, size_t dst_start )const
{
	assert( start <= end );
	const size_t len = end - start + 1;
	LOG_IF( ! dst.isSameType( *this ), Debug, error )
			<< "Copying into a ValuePtr of different type. Its " << dst.getTypeName() << " not " << getTypeName();

	if( end >= getLength() ) {
		LOG( Runtime, error )
				<< "End of the range (" << end << ") is behind the end of this ValuePtr (" << getLength() << ")";
	} else if( len + dst_start > dst.getLength() ) {
		LOG( Runtime, error )
				<< "End of the range (" << len + dst_start << ") is behind the end of the destination (" << dst.getLength() << ")";
	} else {
		boost::shared_ptr<void> daddr = dst.getRawAddress().lock();
		boost::shared_ptr<void> saddr = getRawAddress().lock();
		const size_t soffset = bytesPerElem() * start; //source offset in bytes
		const int8_t *const  src = ( int8_t * )saddr.get();
		const size_t doffset = bytesPerElem() * dst_start;//destination offset in bytes
		int8_t *const  dest = ( int8_t * )daddr.get();
		const size_t blength = len * bytesPerElem();//length in bytes
		memcpy( dest + doffset, src + soffset, blength );
	}
}

scaling_pair ValuePtrBase::getScalingTo( unsigned short typeID, const std::pair<util::ValueReference, util::ValueReference> &minmax, autoscaleOption scaleopt )const
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
size_t ValuePtrBase::useCount() const
{
	return getRawAddress().use_count();
}

}
}
}
