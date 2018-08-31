/*
 *  selection.cpp
 *  isis
 *
 *  Created by Enrico Reimer on 03.04.10.
 *  Copyright 2010 cbs.mpg.de. All rights reserved.
 *
 */

#include "selection.hpp"

namespace isis
{
namespace util
{

Selection::Selection( const char *entries, const char *init_val ): m_set( 0 )
{
	int ID = 1;
	for( const util::istring & ref :  stringToList<util::istring>( entries ) ) {
		const MapType::value_type pair( ref, ID++ );

		if( ! ent_map.insert( pair ).second ) {
			LOG( Debug, error ) << "Entry " << pair << " could not be inserted";
		}
	}

	if( init_val[0] )
		set( init_val );
}
Selection::Selection(): m_set( 0 ) {}

Selection::operator int()const {return m_set;}
Selection::operator util::istring()const
{
	if(m_set){
		for( MapType::const_reference ref :  ent_map ) {
			if ( ref.second == m_set )
				return ref.first;
		}
		assert(false); // m_set should either be in the map or 0
		return util::istring( "<<UNKNOWN>>" );
	} else {
		return util::istring( "<<NOT_SET>>" );
	}
}
Selection::operator std::string()const
{
	util::istring buff = *this;
	return std::string( buff.begin(), buff.end() );
}

bool Selection::set( unsigned short entry )
{
	if( ent_map.size()+1 > entry ) {
		m_set = entry;
		return true;
	} else {
		return false;
	}

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

bool Selection::operator==( const Selection &ref )const{return comp_op(ref,std::equal_to<int>());}
bool Selection::operator==( const char ref[] )    const{return std::equal_to<util::istring>()(*this, ref);}
bool Selection::operator==( const int ref )       const{return std::equal_to<int>()(*this, ref);}

bool Selection::operator<( const Selection &ref )const{return comp_op(ref,std::less<int>());}
bool Selection::operator<( const int ref )       const{return std::greater<int>()(*this, ref);}

bool Selection::operator>( const Selection &ref )const{return comp_op(ref,std::less<int>());}
bool Selection::operator>( const int ref )       const{return std::greater<int>()(*this, ref);}

std::list<util::istring> Selection::getEntries()const
{
	std::list<MapType::value_type> buffer(ent_map.begin(),ent_map.end());
	buffer.sort([](const MapType::value_type &v1,const MapType::value_type &v2){return v1.second<v2.second;});
	
	std::list<util::istring> ret;
	for( MapType::value_type &ref :  buffer ) {
		ret.push_back( ref.first );
	}
	return ret;
}
}
}
