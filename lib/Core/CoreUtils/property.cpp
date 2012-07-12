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

namespace isis
{
namespace util
{

bool &PropertyValue::needed() { return m_needed;}
bool PropertyValue::isNeeded()const { return m_needed;}


bool PropertyValue::operator== ( const util::PropertyValue &second )const
{
	return !second.isEmpty() && operator==( *second );
}
bool PropertyValue::operator!= ( const util::PropertyValue &second )const
{
	return !( ( second.isEmpty() && isEmpty() ) || operator==( second ) );
}


bool PropertyValue::operator== ( const ValueBase &second )const
{
	return !isEmpty() && get()->operator==( second );
}


PropertyValue::PropertyValue ( ) : m_needed( false ) {}

std::string PropertyValue::getTypeName() const {
	LOG_IF(!*this, Debug,error) << "Doing getTypeName on empty property, this will crash";
	return ( **this ).getTypeName();
}

short unsigned int PropertyValue::getTypeID() const {
	LOG_IF(!*this, Debug,error) << "Doing getTypeID on empty property, this will crash";
	return ( **this ).getTypeID();
}

}
}
