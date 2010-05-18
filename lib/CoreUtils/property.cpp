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
bool PropertyValue::needed()const { return m_needed;}


bool PropertyValue::operator== ( const util::PropertyValue &second )const
{
	return !second.empty() && operator==( *second );
}
bool PropertyValue::operator!= ( const util::PropertyValue &second )const
{
	return not( ( second.empty() and empty() ) or operator==( second ) );
}


bool PropertyValue::operator== ( const _internal::TypeBase &second )const
{
	return !empty() && get()->operator==( second );
}


PropertyValue::PropertyValue ( bool _needed ) : m_needed( _needed ) {}


bool PropertyValue::transformTo( PropertyValue &dst, int typeId )const
{
	LOG_IF( empty(), Debug, error ) << "Cannot transform an empty property. Program will stop.";
	_internal::TypeBase &src = operator*();
	_internal::TypeBase::Converter conv = src.getConverterTo( typeId );

	if ( conv ) {
		conv->generate( *this, dst );
		return true;
	} else
		LOG( Runtime, error )
		<< "Cannot transform " << src.toString( true ) << " no converter available";

	// @todo we need a typeId to typeName mapping
	return false;
}

}
}
