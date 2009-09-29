#include "property.hpp"


bool& isis::util::PropertyValue::needed() {
	return m_needed;
}


bool isis::util::PropertyValue::operator== ( const isis::util::PropertyValue& second ) {
	return get()->eq(*second.get());
}


isis::util::PropertyValue::PropertyValue ( bool _needed ) : m_needed(_needed ) {}
