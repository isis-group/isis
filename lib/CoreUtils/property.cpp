//
// C++ Implementation: property
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "property.hpp"

namespace isis{ namespace util{

bool& PropertyValue::needed() {	return m_needed;}
bool PropertyValue::needed()const {	return m_needed;}


bool PropertyValue::operator== ( const util::PropertyValue& second )const {
	return !second.empty() && operator==(*second);
}


bool PropertyValue::operator== ( const _internal::TypeBase& second )const {
	return get()->eq(second);
}


PropertyValue::PropertyValue ( bool _needed ) : m_needed(_needed ) {}

}}