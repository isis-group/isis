//
// C++ Implementation: type_base
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "type_base.hpp"
#include "singletons.hpp"

namespace isis{ namespace util{ namespace _internal{

bool GenericType::isSameType ( const GenericType& second ) const {
	return typeID() == second.typeID();
}

TypePtrBase::TypePtrBase(size_t length): m_len(length) {}

size_t TypePtrBase::len() const { return m_len;}

TypePtrBase::~TypePtrBase() {}
TypeBase::~TypeBase() {}


TypeConverterMap& TypeBase::converters() {
	return *Singletons::get<_internal::TypeConverterMap,0>();
}

TypeBase::Converter TypeBase::getConverterTo(int id)const {
	return converters()[typeID()][id];
}

size_t TypePtrBase::cmp(const TypePtrBase& comp)const{
	LOG_IF(len()!=comp.len(),CoreLog,info) << "Comparing data of different length. The difference will be added to the retuned value.";
	return len()-comp.len() + cmp(0,std::min(len(),comp.len())-1,comp,0);
}

TypePtrBase::Reference TypePtrBase::cloneToMem() const
{
	return cloneToMem(len());
}

TypePtrBase::Reference TypePtrBase::copyToMem() const
{
	TypePtrBase::Reference ret=cloneToMem();
	copyRange(0,len()-1,*ret,0);
	return ret;
}
void TypePtrBase::copyRange(size_t start,size_t end,TypePtrBase &dst,size_t dst_start)const
{
	assert(start<=end);
	const size_t length=end-start+1;
	LOG_IF(not dst.isSameType(*this),CoreDebug,error)
		<< "Copying into a TypePtr of different type. Its " << dst.typeName() << " not " << typeName();
	LOG_IF(end>=len(),CoreLog,error)
		<< "End of the range ("<< end << ") is behind the end of this TypePtr ("<< len() << ")";
	LOG_IF(length+dst_start>dst.len(),CoreLog,error)
		<< "End of the range ("<< length+dst_start << ") is behind the end of the destination ("<< dst.len() << ")";
	boost::shared_ptr<void> daddr=dst.address().lock();
	boost::shared_ptr<void> saddr=address().lock();

	const size_t soffset=bytes_per_elem()*start; //source offset in bytes
	const int8_t* const  src= (int8_t*)saddr.get();

	const size_t doffset=bytes_per_elem()*dst_start;//destination offset in bytes
	int8_t* const  dest= (int8_t*)daddr.get();

	const size_t blength=length*bytes_per_elem();//length in bytes
	
	memcpy(dest+doffset,src+soffset,blength);
}


}}}
