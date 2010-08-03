/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If !, see <http://www.gnu.org/licenses/>.

*/

#include "progparameter.hpp"
#include <boost/foreach.hpp>

isis::util::ProgParameter::ProgParameter()
{
	needed() = true;
}
bool isis::util::ProgParameter::parse( const Type<std::string> &props )
{
	_internal::TypeBase &me = **this;
	bool ret = false;

	if ( ( ( std::string )props ).empty() ) {
		if ( me.is<bool>() ) {
			me.cast_to<bool>() = true;
			ret = true;
		}
	} else {
		ret = _internal::TypeBase::convert( props, me );
	}

	LOG_IF( ret, Debug, info ) << "Parsed " << MSubject( props.toString() ) << " as " << me.toString( true );
	return ret;
}
const std::string &isis::util::ProgParameter::description()const
{
	return m_description;
}
void isis::util::ProgParameter::setDescription( const std::string &desc )
{
	m_description = desc;
}

isis::util::ParameterMap::ParameterMap(): parsed( false ) {}

bool isis::util::ParameterMap::parse( int argc, char **argv )
{
	parsed = true;
	std::string pName;

	for ( int i = 1; i < argc; ) { // scan through the command line, to find next parameter
		if ( argv[i][0] == '-' ) { //seems like we found a new parameter here
			pName = argv[i];
			pName.erase( 0, pName.find_first_not_of( '-' ) );
			i++;
		}

		if( !pName.empty() ) { // if we got a parameter before
			const int begin = i;

			while( i < argc && argv[i][0] != '-' ) { //collect its properties, while there are some ..
				i++;
			}

			iterator found = find( pName );

			if( found == end() ) {
				LOG( Runtime, warning ) << "Ignoring unknown parameter " << MSubject( std::string( "-" ) + pName + " " + list2string( argv + begin, argv + i, " ", "", "" ) );
			} else if ( found->second.parse( list2string( argv + begin, argv + i, ",", "", "" ) ) ) { // parse the collected properties
				found->second.needed() = false;//remove needed flag, because the value is set (aka "not needed anymore")
			} else {
				LOG( Runtime, error )
						<< "Failed to parse value(s) "
						<< MSubject( list2string( argv + begin, argv + i, " ", "", "" ) )
						<< " for "  << found->first << "(" << found->second->typeName() << ")";
				parsed = false;
			}
		}
	}

	return parsed ;
}
bool isis::util::ParameterMap::isComplete()const
{
	LOG_IF( ! parsed, Debug, error ) << "You did not run parse() yet. This is very likely an error";
	return std::find_if( begin(), end(), neededP() ) == end();
}
void isis::util::ParameterMap::printAll()const
{
	std::cout << *this << std::endl;
}
void isis::util::ParameterMap::printNeeded()const
{
	std::map<key_type, mapped_type, key_compare> needed( *this );

	for (
		std::map<key_type, mapped_type, key_compare>::iterator at = std::find_if( needed.begin(), needed.end(), notneededP() );
		at != needed.end();
		at = std::find_if( at, needed.end(), notneededP() )
	)
		needed.erase( at++ );

	std::cout << needed << std::endl;
}

isis::util::ProgParameter::operator boost::scoped_ptr<_internal::TypeBase>::unspecified_bool_type()const
{
	boost::scoped_ptr<_internal::TypeBase> dummy;

	if( ( *this )->cast_to<bool>() )dummy.reset( new Type<int16_t> );

	return  dummy;
}
