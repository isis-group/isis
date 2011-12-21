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

namespace isis
{
namespace util
{
namespace _internal
{
bool GenericValue::isSameType ( const GenericValue &second ) const
{
	return getTypeID() == second.getTypeID();
}
}

ValueBase::~ValueBase() {}


const _internal::ValueConverterMap &ValueBase::converters()
{
	static _internal::ValueConverterMap ret; //@todo not using class Singleton because ValuePtrConverterMap is hidden
	return ret;
}

const ValueBase::Converter &ValueBase::getConverterTo( unsigned short ID )const
{
	const _internal::ValueConverterMap::const_iterator f1 = converters().find( getTypeID() );
	assert( f1 != converters().end() );
	const _internal::ValueConverterMap::mapped_type::const_iterator f2 = f1->second.find( ID );
	assert( f2 != f1->second.end() );
	return f2->second;
}
bool ValueBase::convert( const ValueBase &from, ValueBase &to )
{
	const Converter &conv = from.getConverterTo( to.getTypeID() );

	if ( conv ) {
		switch ( conv->convert( from, to ) ) {
		case boost::numeric::cPosOverflow:
			LOG( Runtime, error ) << "Positive overflow when converting " << from.toString( true ) << " to " << to.getTypeName() << ".";
			break;
		case boost::numeric::cNegOverflow:
			LOG( Runtime, error ) << "Negative overflow when converting " << from.toString( true ) << " to " << to.getTypeName() << ".";
			break;
		case boost::numeric::cInRange:
			return true;
			break;
		}
	} else {
		LOG( Runtime, error )
				<< "I dont know any conversion from "
				<< MSubject( from.toString( true ) ) << " to " << MSubject( to.getTypeName() );
	}

	return false;
}

bool ValueBase::fitsInto( short unsigned int ID ) const
{
	boost::scoped_ptr<ValueBase> to;
	const Converter &conv = getConverterTo( ID );

	if ( conv ) {
		return ( conv->generate( *this, to ) ==  boost::numeric::cInRange );
	} else {
		LOG( Runtime, warning )
				<< "I dont know any conversion from "
				<< MSubject( toString( true ) ) << " to " << MSubject( getTypeMap( true, false )[ID] );
		return false; // return an empty Reference
	}
}

ValueBase::Reference ValueBase::copyByID( short unsigned int ID ) const
{
	boost::scoped_ptr<ValueBase> to;
	const Converter &conv = getConverterTo( ID );

	if ( conv ) {
		switch ( conv->generate( *this, to ) ) {
		case boost::numeric::cPosOverflow:
			LOG( Runtime, error ) << "Positive overflow when converting " << MSubject( toString( true ) ) << " to " << MSubject( getTypeMap( true, false )[ID] ) << ".";
			break;
		case boost::numeric::cNegOverflow:
			LOG( Runtime, error ) << "Negative overflow when converting " << MSubject( toString( true ) ) << " to " << MSubject( getTypeMap( true, false )[ID] ) << ".";
			break;
		case boost::numeric::cInRange:
			break;
		}

		return *to; // return the generated Value-Object - wrapping it into Reference
	} else {
		LOG( Runtime, error )
				<< "I dont know any conversion from "
				<< MSubject( toString( true ) ) << " to " << MSubject( getTypeMap( true, false )[ID] );
		return Reference(); // return an empty Reference
	}
}

}
}
