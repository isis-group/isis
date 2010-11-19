/*
 *  selection.cpp
 *  isis
 *
 *  Created by Enrico Reimer on 03.04.10.
 *  Copyright 2010 cbs.mpg.de. All rights reserved.
 *
 */

#include "CoreUtils/selection.hpp"
#include <boost/foreach.hpp>

namespace isis
{
namespace util
{

Selection::Selection( const char *entries ): m_set( 0 )
{
	int id = 1;
	BOOST_FOREACH( const util::istring & ref, string2list<util::istring>( util::istring( entries ), ',' ) ) {
		const MapType::value_type pair( ref, id++ );

		if( ! ent_map.insert( pair ).second ) {
			LOG( Debug, error ) << "Entry " << util::MSubject( pair ) << " could not be inserted";
		}
	}
}
Selection::Selection(): m_set( 0 ) {}

Selection::operator const int()const {return m_set;}
Selection::operator const util::istring()const
{
	BOOST_FOREACH( MapType::const_reference ref, ent_map ) {
		if ( ref.second == m_set )
			return ref.first;
	}
	return util::istring( "<<NOT_SET>>" );
}
Selection::operator const std::string()const
{
	util::istring buff=*this;
	return std::string( buff.begin(),buff.end() );
}

bool Selection::set( const char *entry )
{
	MapType::const_iterator found = ent_map.find( entry );

	if ( found != ent_map.end() ) {
		m_set = found->second;
		return true;
	} else {
		LOG( Runtime, error ) << "Failed to set " << MSubject( entry ) << ", valid values are " << getEntries();
		return false;
	}
}

bool Selection::operator==( const Selection &ref )const
{
	return m_set == ref.m_set && ent_map == ref.ent_map;
}
bool Selection::operator==( const char ref[] ) const
{
	return ((const util::istring&)*this ) == ref ;
}
bool Selection::operator==( const int ref ) const
{
	return ( ( int ) * this ) == ref;
}


std::list<util::istring> Selection::getEntries()const
{
	std::list<util::istring> ret;
	BOOST_FOREACH( MapType::const_reference ref, ent_map ){
		ret.push_back( ref.first );
	}
	return ret;
}
}
}
