#include "property.hpp"


bool& isis::util::PropertyValue::needed() {	return m_needed;}
bool isis::util::PropertyValue::needed()const {	return m_needed;}

bool isis::util::PropertyValue::operator== ( const isis::util::PropertyValue& second )const {
	return operator==(*second);
}


bool isis::util::PropertyValue::operator== ( const isis::util::_internal::TypeBase& second )const {
	return get()->eq(second);
}


isis::util::PropertyValue::PropertyValue ( bool _needed ) : m_needed(_needed ) {}
