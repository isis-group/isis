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

#include "CoreUtils/type_base.hpp"
#include "CoreUtils/singletons.hpp"

namespace isis
{
namespace util
{
namespace _internal
{

bool GenericType::isSameType ( const GenericType &second ) const
{
	return typeID() == second.typeID();
}

TypeBase::~TypeBase() {}


const TypeConverterMap &TypeBase::converters()
{
	return Singletons::get<_internal::TypeConverterMap, 0>();
}

const TypeBase::Converter &TypeBase::getConverterTo( unsigned short id )const
{
	const TypeConverterMap::const_iterator f1 = converters().find( typeID() );
	assert( f1 != converters().end() );
	const TypeConverterMap::mapped_type::const_iterator f2 = f1->second.find( id );
	assert( f2 != f1->second.end() );
	return f2->second;
}
bool TypeBase::convert( const TypeBase &from, TypeBase &to )
{
	const Converter &conv = from.getConverterTo( to.typeID() );

	if ( conv ) {
		switch ( conv->convert( from, to ) ) {
		case boost::numeric::cPosOverflow:
			LOG( Runtime, error ) << "Positive overflow when converting " << from.toString( true ) << " to " << to.typeName() << ".";
			break;
		case boost::numeric::cNegOverflow:
			LOG( Runtime, error ) << "Negative overflow when converting " << from.toString( true ) << " to " << to.typeName() << ".";
			break;
		case boost::numeric::cInRange:
			return true;
			break;
		}
	} else {
		LOG( Runtime, error )
				<< "I dont know any conversion from "
				<< MSubject( from.toString( true ) ) << " to " << MSubject( to.typeName() );
	}

	return false;
}

bool TypeBase::fitsInto(short unsigned int id) const
{
	boost::scoped_ptr<TypeBase> to;
	const Converter &conv = getConverterTo( id );

	if ( conv ) {
		return ( conv->generate( *this, to ) ==  boost::numeric::cInRange);
	} else {
		LOG( Runtime, warning )
				<< "I dont know any conversion from "
				<< MSubject( toString( true ) ) << " to " << MSubject( getTypeMap( true, false )[id] );
		return false; // return an empty Reference
	}
}

TypeBase::Reference TypeBase::copyToNewById( short unsigned int id ) const
{
	boost::scoped_ptr<TypeBase> to;
	const Converter &conv = getConverterTo( id );

	if ( conv ) {
		switch ( conv->generate( *this, to ) ) {
		case boost::numeric::cPosOverflow:
			LOG( Runtime, error ) << "Positive overflow when converting " << MSubject( toString( true ) ) << " to " << MSubject( getTypeMap( true, false )[id] ) << ".";
			break;
		case boost::numeric::cNegOverflow:
			LOG( Runtime, error ) << "Negative overflow when converting " << MSubject( toString( true ) ) << " to " << MSubject( getTypeMap( true, false )[id] ) << ".";
			break;
		case boost::numeric::cInRange:
			break;
		}

		return *to; // return the generated Type-Object - wrapping it into Reference
	} else {
		LOG( Runtime, error )
				<< "I dont know any conversion from "
				<< MSubject( toString( true ) ) << " to " << MSubject( getTypeMap( true, false )[id] );
		return Reference(); // return an empty Reference
	}
}

}
}
}
