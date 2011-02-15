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

bool &TypeValue::needed() { return m_needed;}
bool TypeValue::needed()const { return m_needed;}


bool TypeValue::operator== ( const util::TypeValue &second )const
{
	return !second.isEmpty() && operator==( *second );
}
bool TypeValue::operator!= ( const util::TypeValue &second )const
{
	return !( ( second.isEmpty() && isEmpty() ) || operator==( second ) );
}


bool TypeValue::operator== ( const _internal::TypeBase &second )const
{
	return !isEmpty() && get()->operator==( second );
}


TypeValue::TypeValue ( bool _needed ) : m_needed( _needed ) {}


}
}
