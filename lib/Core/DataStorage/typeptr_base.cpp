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

TypePtrBase::TypePtrBase( size_t length ): m_len( length ) {}

size_t TypePtrBase::length() const { return m_len;}

TypePtrBase::~TypePtrBase() {}

const TypePtrConverterMap &TypePtrBase::converters()
{
	return util::Singletons::get<_internal::TypePtrConverterMap, 0>();
}

const TypePtrBase::Converter &TypePtrBase::getConverterTo( unsigned short ID )const
{
	const TypePtrConverterMap::const_iterator f1 = converters().find( getTypeID() );
	LOG_IF( f1 == converters().end(), Debug, error ) << "There is no known conversion from " << util::getTypeMap()[getTypeID()];
	const TypePtrConverterMap::mapped_type::const_iterator f2 = f1->second.find( ID );
	LOG_IF( f2 == f1->second.end(), Debug, error ) << "There is no known conversion from " << util::getTypeMap()[getTypeID()] << " to " << util::getTypeMap()[ID];
	return f2->second;
}

size_t TypePtrBase::compare( const TypePtrBase &comp )const
{
	LOG_IF( length() != comp.length(), Runtime, info ) << "Comparing data of different length. The difference will be added to the returned value.";
	return length() - comp.length() + compare( 0, std::min( length(), comp.length() ) - 1, comp, 0 );
}

TypePtrBase::Reference TypePtrBase::copyToNewByID( unsigned short ID ) const
{
	return copyToNewByID( ID, getScalingTo( ID ) );
}
TypePtrBase::Reference TypePtrBase::copyToNewByID( unsigned short ID, const scaling_pair &scaling ) const
{
	const Converter &conv = getConverterTo( ID );

	if( conv ) {
		boost::scoped_ptr<TypePtrBase> ret;
		conv->generate( *this, ret, scaling );
		return *ret;
	} else {
		LOG( Runtime, error )
				<< "I dont know any conversion from "
				<< util::MSubject( toString( true ) ) << " to " << util::MSubject( util::getTypeMap( false, true )[ID] );
		return Reference(); // return an empty Reference
	}
}

TypePtrBase::Reference TypePtrBase::createById( unsigned short id, size_t len )
{
	const TypePtrConverterMap::const_iterator f1 = converters().find( id );
	TypePtrConverterMap::mapped_type::const_iterator f2;

	// try to get a converter to convert the requestet type into itself - they 're there for all known types
	if( f1 != converters().end() && ( f2 = f1->second.find( id ) ) != f1->second.end() ) {
		const _internal::TypePtrConverterBase &conv = *( f2->second );
		boost::scoped_ptr<TypePtrBase> ret;
		conv.create( ret, len );
		return *ret;
	} else {
		LOG( Debug, error ) << "There is no known creator for " << util::getTypeMap()[id];
		return Reference(); // return an empty Reference
	}
}

void TypePtrBase::copyRange( size_t start, size_t end, TypePtrBase &dst, size_t dst_start )const
{
	assert( start <= end );
	const size_t len = end - start + 1;
	LOG_IF( ! dst.isSameType( *this ), Debug, error )
			<< "Copying into a ValuePtr of different type. Its " << dst.getTypeName() << " not " << getTypeName();

	if( end >= length() ) {
		LOG( Runtime, error )
				<< "End of the range (" << end << ") is behind the end of this ValuePtr (" << length() << ")";
	} else if( len + dst_start > dst.length() ) {
		LOG( Runtime, error )
				<< "End of the range (" << len + dst_start << ") is behind the end of the destination (" << dst.length() << ")";
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

scaling_pair TypePtrBase::getScalingTo( unsigned short typeID, autoscaleOption scaleopt )const
{
	std::pair<util::TypeReference, util::TypeReference> minmax = getMinMax();
	assert( ! ( minmax.first.isEmpty() || minmax.second.isEmpty() ) );
	return TypePtrBase::getScalingTo( typeID, minmax, scaleopt );
}

scaling_pair TypePtrBase::getScalingTo( unsigned short typeID, const std::pair<util::TypeReference, util::TypeReference> &minmax, autoscaleOption scaleopt )const
{
	LOG_IF( minmax.first.isEmpty() || minmax.second.isEmpty(), Debug, error ) << "One of the TypeReference's in minmax is empty(). This will crash...";
	return getScalingTo( typeID, *minmax.first, *minmax.second, scaleopt );
}

scaling_pair TypePtrBase::getScalingTo( unsigned short typeID, const util::_internal::TypeBase &min, const util::_internal::TypeBase &max, autoscaleOption scaleopt )const
{
	const Converter &conv = getConverterTo( typeID );

	if ( conv ) {
		return conv->getScaling( min, max, scaleopt );
	} else {
		LOG( Runtime, error )
				<< "I dont know any conversion from " << util::MSubject( getTypeName() ) << " to " << util::MSubject( util::getTypeMap( false, true )[typeID] );
		return scaling_pair();
	}
}
bool TypePtrBase::convertTo( TypePtrBase &dst )const
{
	return convertTo( dst, getScalingTo( dst.getTypeID() ) );
}
bool TypePtrBase::convertTo( TypePtrBase &dst, const scaling_pair &scaling ) const
{
	const Converter &conv = getConverterTo( dst.getTypeID() );

	if ( conv ) {
		conv->convert( *this, dst, scaling );
		return true;
	} else {
		LOG( Runtime, error )
				<< "I dont know any conversion from " << util::MSubject( getTypeName() ) << " to " << util::MSubject( dst.getTypeName() );
		return false;
	}
}
size_t TypePtrBase::useCount() const
{
	return getRawAddress().use_count();
}

}
}
}
