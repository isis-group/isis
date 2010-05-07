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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
			me.cast_to_Type<bool>() = true;
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
	int begin = 0, end = 0;
	bool ret = true;

	for ( int i = 1; i < argc; i++ ) { // scan through the command line, to find next parameter
		if ( argv[i][0] == '-' ) {
			if ( begin )end = i;
			else begin = i;
		}

		if ( i >= argc - 1 and begin )end = argc;

		if ( end ) {
			std::string pName( argv[begin] );
			pName.erase( 0, pName.find_first_not_of( '-' ) );
			iterator found = find( pName );

			if ( found != this->end() ) { //ok, we have a parameter with that name
				assert( begin + 1 <= end ); //must be <= because we could have an empty property-list for this parameter

				if ( found->second.parse( list2string( argv + begin + 1, argv + end, ",", "", "" ) ) ) {
					found->second.needed() = false;//remove needed flag, because the value is set (aka "not needed anymore")
				} else {
					LOG( Runtime, error )
							<< "Failed to parse parameter " << MSubject( list2string( argv + begin, argv + end, " ", "", "" ) ) << " for "
							<< found->first << "(" << found->second->typeName() << ")";
					ret = false;
				}
			} else {
				LOG( Runtime, warning ) << "Ignoring unknown parameter " << MSubject( list2string( argv + begin, argv + end, " ", "", "" ) );
			}

			begin = end;
			end = 0;
		}
	}

	return ( parsed = ret );
}
bool isis::util::ParameterMap::isComplete()const
{
	LOG_IF( not parsed, Debug, error ) << "You did not run parse() yet. This is very likely an error";
	return std::find_if( begin(), end(), neededP() ) == end();
}
void isis::util::ParameterMap::printAll()const
{
	std::cout << *this << std::endl;
}
void isis::util::ParameterMap::printNeeded()const
{
	std::map<key_type, mapped_type> needed( *this );

	for (
		std::map<key_type, mapped_type>::iterator at = std::find_if( needed.begin(), needed.end(), notneededP() );
		at != needed.end();
		at = std::find_if( at, needed.end(), notneededP() )
	)
		needed.erase( at++ );

	std::cout << needed << std::endl;
}

