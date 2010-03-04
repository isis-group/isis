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


}}}
