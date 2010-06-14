
#include "typeptr_base.hpp"
#include "typeptr_converter.hpp"
#include "common.hpp"

namespace isis{ namespace data{ namespace _internal{

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
	assert( f1 != converters().end() );
	const TypePtrConverterMap::mapped_type::const_iterator f2 = f1->second.find( id );
	assert( f2 != f1->second.end() );
	return f2->second;
}

size_t TypePtrBase::cmp( const TypePtrBase &comp )const
{
	LOG_IF( len() != comp.len(), Runtime, info ) << "Comparing data of different length. The difference will be added to the retuned value.";
	return len() - comp.len() + cmp( 0, std::min( len(), comp.len() ) - 1, comp, 0 );
}

TypePtrBase::Reference TypePtrBase::cloneToMem() const
{
	return cloneToMem( len() );
}

TypePtrBase::Reference TypePtrBase::copyToMem() const
{
	TypePtrBase::Reference ret = cloneToMem();
	copyRange( 0, len() - 1, *ret, 0 );
	return ret;
}
void TypePtrBase::copyRange( size_t start, size_t end, TypePtrBase &dst, size_t dst_start )const
{
	assert( start <= end );
	const size_t length = end - start + 1;
	LOG_IF( ! dst.isSameType( *this ), Debug, error )
	<< "Copying into a TypePtr of different type. Its " << dst.typeName() << " not " << typeName();
	LOG_IF( end >= len(), Runtime, error )
	<< "End of the range (" << end << ") is behind the end of this TypePtr (" << len() << ")";
	LOG_IF( length + dst_start > dst.len(), Runtime, error )
	<< "End of the range (" << length + dst_start << ") is behind the end of the destination (" << dst.len() << ")";
	boost::shared_ptr<void> daddr = dst.address().lock();
	boost::shared_ptr<void> saddr = address().lock();
	const size_t soffset = bytes_per_elem() * start; //source offset in bytes
	const int8_t *const  src = ( int8_t * )saddr.get();
	const size_t doffset = bytes_per_elem() * dst_start;//destination offset in bytes
	int8_t *const  dest = ( int8_t * )daddr.get();
	const size_t blength = length * bytes_per_elem();//length in bytes
	memcpy( dest + doffset, src + soffset, blength );
}
bool TypePtrBase::convertTo( TypePtrBase &dst, const util::_internal::TypeBase &min, const util::_internal::TypeBase &max ) const
{
	const Converter &conv = getConverterTo( dst.typeID() );
	LOG_IF( len() < dst.len(), Runtime, error ) << "The target is longer than the the source (" << dst.len() << ">" << len() << ")";
	LOG_IF( len() > dst.len(), Debug, info ) << "The target is shorter than the the source (" << dst.len() << ">" << len() << "). Will only copy " << len() << " elements";

	if ( conv ) {
		conv->convert( *this, dst, min, max );
		return true;
	} else {
		LOG( Runtime, error )
		<< "I dont know any conversion from " << util::MSubject( typeName() ) << " to " << util::MSubject( dst.typeName() );
		return false;
	}
}

}}}
