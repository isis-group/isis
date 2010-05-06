/*
 *  selection.cpp
 *  isis
 *
 *  Created by Enrico Reimer on 03.04.10.
 *  Copyright 2010 cbs.mpg.de. All rights reserved.
 *
 */

#include "selection.hpp"
#include <boost/foreach.hpp>

namespace isis
{
namespace util
{

Selection::Selection( const char *entries ): m_set( 0 )
{
	int id = 1;
	BOOST_FOREACH( const std::string & ref, string2list<std::string>( std::string( entries ), ',' ) )
	ent_map[ref] = id++;
}
Selection::Selection(): m_set( 0 ) {}

Selection::operator const int()const {return m_set;}
Selection::operator const std::string()const
{
	BOOST_FOREACH( map_type::const_reference ref, ent_map ) {
		if ( ref.second == m_set )
			return ref.first;
	}
	return std::string( "<<NOT_SET>>" );
}

bool Selection::set( const char* entry )
{
	map_type::const_iterator found = ent_map.find( entry );

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
	return m_set == ref.m_set and ent_map == ref.ent_map;
}
bool Selection::operator==( const char ref[] ) const
{
	return strcasecmp ( ( ( std::string ) * this ).c_str(), ref )  == 0;
}
bool Selection::operator==( const int ref ) const
{
	return ( ( int ) * this ) == ref;
}


std::list<std::string> Selection::getEntries()const
{
	std::list<std::string> ret;
	BOOST_FOREACH( map_type::const_reference ref, ent_map )
	ret.push_back( ref.first );
	return ret;
}
}
}
