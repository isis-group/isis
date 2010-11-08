
#include "DataStorage/typeptr_base.hpp"
#include "DataStorage/typeptr_converter.hpp"
#include "DataStorage/common.hpp"

namespace isis
{
namespace data
{
namespace _internal
{

TypePtrBase::TypePtrBase( size_t length ): m_len( length ) {}

size_t TypePtrBase::len() const { return m_len;}

TypePtrBase::~TypePtrBase() {}

const TypePtrConverterMap &TypePtrBase::converters()
{
	return util::Singletons::get<_internal::TypePtrConverterMap, 0>();
}

const TypePtrBase::Converter &TypePtrBase::getConverterTo( unsigned short id )const
{
	const TypePtrConverterMap::const_iterator f1 = converters().find( typeID() );
	LOG_IF( f1 == converters().end(), Debug, error ) << "There is no known conversion from " << util::getTypeMap()[typeID()];
	const TypePtrConverterMap::mapped_type::const_iterator f2 = f1->second.find( id );
	LOG_IF( f2 == f1->second.end(), Debug, error ) << "There is no known conversion from " << util::getTypeMap()[typeID()] << " to " << util::getTypeMap()[id];
	return f2->second;
}

size_t TypePtrBase::cmp( const TypePtrBase &comp )const
{
	LOG_IF( len() != comp.len(), Runtime, info ) << "Comparing data of different length. The difference will be added to the returned value.";
	return len() - comp.len() + cmp( 0, std::min( len(), comp.len() ) - 1, comp, 0 );
}

TypePtrBase::Reference TypePtrBase::copyToNewById( unsigned short id ) const
{
	std::pair<util::TypeReference,util::TypeReference> scale=getScalingTo(id);
	return copyToNewById( id, *scale.first, *scale.second);
}
TypePtrBase::Reference TypePtrBase::copyToNewById( unsigned short id, const util::_internal::TypeBase &scale, const util::_internal::TypeBase &offset ) const
{
	const Converter &conv = getConverterTo( id );

	if( conv ) {
		boost::scoped_ptr<TypePtrBase> ret;
		conv->generate( *this, ret, scale, offset );
		return *ret;
	} else {
		LOG( Runtime, error )
				<< "I dont know any conversion from "
				<< util::MSubject( toString( true ) ) << " to " << util::MSubject( util::getTypeMap( false, true )[id] );
		return Reference(); // return an empty Reference
	}
}

void TypePtrBase::copyRange( size_t start, size_t end, TypePtrBase &dst, size_t dst_start )const
{
	assert( start <= end );
	const size_t length = end - start + 1;
	LOG_IF( ! dst.isSameType( *this ), Debug, error )
			<< "Copying into a TypePtr of different type. Its " << dst.typeName() << " not " << typeName();
	if(end >= len()){
		LOG( Runtime, error )
			<< "End of the range (" << end << ") is behind the end of this TypePtr (" << len() << ")";
	} else if(length + dst_start > dst.len()){
		LOG( Runtime, error )
			<< "End of the range (" << length + dst_start << ") is behind the end of the destination (" << dst.len() << ")";
	} else {
		boost::shared_ptr<void> daddr = dst.address().lock();
		boost::shared_ptr<void> saddr = address().lock();
		const size_t soffset = bytes_per_elem() * start; //source offset in bytes
		const int8_t *const  src = ( int8_t * )saddr.get();
		const size_t doffset = bytes_per_elem() * dst_start;//destination offset in bytes
		int8_t *const  dest = ( int8_t * )daddr.get();
		const size_t blength = length * bytes_per_elem();//length in bytes
		memcpy( dest + doffset, src + soffset, blength );
	}
}

std::pair<util::TypeReference,util::TypeReference> TypePtrBase::getScalingTo( unsigned short typeID, autoscaleOption scaleopt )const
{
	util::TypeReference min, max;
	getMinMax( min, max );
	assert( ! ( min.empty() || max.empty() ) );
	return TypePtrBase::getScalingTo( typeID, *min, *max );
}
std::pair<util::TypeReference,util::TypeReference>
TypePtrBase::getScalingTo( unsigned short typeID, const util::_internal::TypeBase &min, const util::_internal::TypeBase &max, autoscaleOption scaleopt )const
{
	const Converter &conv = getConverterTo( typeID );
	
	if ( conv ) {
		return conv->getScaling(min, max, scaleopt);
	} else {
		LOG( Runtime, error )
		<< "I dont know any conversion from " << util::MSubject( typeName() ) << " to " << util::MSubject( util::getTypeMap(false,true)[typeID] );
		return std::pair<util::TypeReference,util::TypeReference>();
	}
}
bool TypePtrBase::convertTo( TypePtrBase &dst )const
{
	std::pair<util::TypeReference,util::TypeReference> scale=getScalingTo(dst.typeID());
	return TypePtrBase::convertTo( dst, *scale.first, *scale.second);
}
bool TypePtrBase::convertTo( TypePtrBase &dst, const util::_internal::TypeBase &scale, const util::_internal::TypeBase &offset ) const
{
	const Converter &conv = getConverterTo( dst.typeID() );

	if ( conv ) {
		conv->convert( *this, dst, scale, offset );
		return true;
	} else {
		LOG( Runtime, error )
				<< "I dont know any conversion from " << util::MSubject( typeName() ) << " to " << util::MSubject( dst.typeName() );
		return false;
	}
}
size_t TypePtrBase::use_count() const
{
	return address().use_count();
}


bool TypePtrBase::swapAlong( TypePtrBase &dst, const size_t dim, const size_t dims[]  ) const
{
	boost::shared_ptr<void> daddr = dst.address().lock();
	boost::shared_ptr<void> saddr = address().lock();
	const int8_t *const  src = ( int8_t * )saddr.get();
	int8_t *const dest = ( int8_t * )daddr.get();

	if ( dim == 0 ) {
		size_t index_forward = 0;
		size_t index_y = 0;

		for ( size_t z = 0; z < dims[2]; z++ ) {
			for ( size_t y = 0; y < dims[1]; y++ ) {
				index_y++;

				for ( size_t direction = 0; direction < dims[0]; direction++ ) {
					memcpy( dest + index_forward * bytes_per_elem(), src + ( ( index_y * dims[0] ) - direction - 1 ) * bytes_per_elem(), bytes_per_elem() );
					index_forward++;
				}
			}
		}

		return 1;
	} else if ( dim == 1 ) {
		for ( size_t z = 0; z < dims[2]; z++ ) {
			for ( size_t direction = 0; direction < dims[dim]; direction++ ) {
				memcpy( dest + ( ( dims[0] * direction ) + z * dims[0] * dims[1] ) * bytes_per_elem(), src + ( ( dims[0] * ( dims[1] - direction - 1 ) ) + z * dims[0] * dims[1] ) * bytes_per_elem(), bytes_per_elem() * dims[0] );
			}
		}

		return 1;
	} else if ( dim == 2 ) {
		for ( size_t direction = 0; direction < dims[dim]; direction++ ) {
			memcpy( dest + direction * dims[0]*dims[1]*bytes_per_elem(), src + ( dims[dim] - direction - 1 )*dims[0]*dims[1]*bytes_per_elem(), bytes_per_elem()*dims[0]*dims[1] );
		}

		return 1;
	} else {
		LOG( Runtime, error ) << "Swapping along axis referred by " << dim << " is not possible!";
		return 0;
	}
}

}
}
}
