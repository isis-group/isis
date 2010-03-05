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
	return Singletons::get<_internal::TypeConverterMap,0>();
}

TypeBase::Converter TypeBase::getConverterTo(int id)const {
	return converters()[typeID()][id];
}

bool TypePtrBase::memcmp(size_t start,size_t end,const TypePtrBase &dst,size_t dst_start)const
{
	assert(start<=end);
	size_t length=end-start;
	LOG_IF(dst.typeID()!=typeID(),CoreLog,error)
		<< "Comparing to a TypePtr of different type. Its " << dst.typeName() << " not " << typeName();
	LOG_IF(end>=len(),CoreLog,error)
		<< "End of the range ("<< end << ") is behind the end of this TypePtr ("<< len() << ")";
	LOG_IF(length+dst_start>=dst.len(),CoreLog,error)
		<< "End of the range ("<< length+dst_start << ") is behind the end of the destination ("<< dst.len() << ")";

	void * p1=(u_int8_t*)(address().lock().get())+bytes_per_elem()*start;
	void * p2=(u_int8_t*)(dst.address().lock().get())+dst.bytes_per_elem()*dst_start;
	return std::memcmp(p1,p2,length*bytes_per_elem())==0;
}
bool TypePtrBase::memcmp(const TypePtrBase& comp)const{
	if(len()!=comp.len()){
		LOG(CoreLog,info) << "Comparing data of different length will return false";
		return false;
	} else
		return len()==comp.len() and memcmp(0,len()-1,comp,0);
}


}}}
