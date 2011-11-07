/*
    Copyright (C) 2010  reimer@cbs.mpg.de

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

namespace isis
{
namespace util
{

ProgParameter::ProgParameter()
{
	needed() = true;
	hidden() = false;
}
ProgParameter::ProgParameter( const ProgParameter &ref ): PropertyValue( static_cast<const PropertyValue &>( ref ) ), m_hidden( false ) {}

bool ProgParameter::isHidden() const
{
	return m_hidden;
}
bool &ProgParameter::hidden()
{
	return m_hidden;
}

bool ProgParameter::parse( const Value<std::string> &props )
{
	_internal::ValueBase &me = **this;
	bool ret = false;

	if ( ( ( std::string )props ).empty() ) {
		if ( me.is<bool>() ) {
			me.castTo<bool>() = true;
			ret = true;
		}
	} else {
		ret = _internal::ValueBase::convert( props, me );
	}

	LOG_IF( ret, Debug, info ) << "Parsed " << MSubject( props.toString() ) << " as " << me.toString( true );

	if( ret )m_set = true;

	return ret;
}
const std::string &ProgParameter::description()const
{
	return m_description;
}
void ProgParameter::setDescription( const std::string &desc )
{
	m_description = desc;
}
bool ProgParameter::isSet() const
{
	return m_set;
}


ParameterMap::ParameterMap(): parsed( false ) {}

bool ParameterMap::parse( int argc, char **argv )
{
	parsed = true;
	util::istring pName;

	for ( int i = 1; i < argc; ) { // scan through the command line, to find next parameter
		if ( argv[i][0] == '-' ) { //seems like we found a new parameter here
			pName = argv[i];
			pName.erase( 0, pName.find_first_not_of( '-' ) );
		} else {
			LOG( Runtime, error ) << "Ignoring unexpected non-parameter " << MSubject( argv[i] );
		}

		i++;

		if( !pName.empty() ) { // if we got a parameter before
			const int start = i;

			while( i < argc && argv[i][0] != '-' ) { //collect its properties, while there are some ..
				i++;
			}

			std::list<util::istring> matchingStrings;
			BOOST_FOREACH( ParameterMap::const_reference parameterRef, *this ) {
				if( parameterRef.first.find( pName ) == 0 ) {
					matchingStrings.push_back( parameterRef.first );
				}
			}

			if( matchingStrings.size() > 1 ) {
				std::stringstream matchingStringStream;
				BOOST_FOREACH( std::list<util::istring>::const_reference stringRef, matchingStrings ) {
					matchingStringStream << stringRef << " ";
				}
				LOG( Runtime, warning )
						<< "The parameter \"" << pName << "\" is ambiguous. The parameters \""
						<< matchingStringStream.str().erase( matchingStringStream.str().size() - 1, 1 )
						<< "\" are possible. Ignoring this parameter!";
			} else if ( !matchingStrings.size() ) {
				LOG( Runtime, warning ) << "Ignoring unknown parameter " << MSubject( util::istring( "-" ) + pName + " " + listToString( argv + start, argv + i, " ", "", "" ).c_str() );
			} else if ( at( matchingStrings.front() ).parse( listToString( argv + start, argv + i, ",", "", "" ) ) ) { // parse the collected properties
				at( matchingStrings.front() ).needed() = false; //remove needed flag, because the value is set (aka "not needed anymore")
			} else {
				LOG( Runtime, error )
						<< "Failed to parse value(s) "
						<< MSubject( listToString( argv + start, argv + i, " ", "", "" ) )
						<< " for "  << matchingStrings.front() << "(" << at( matchingStrings.front() )->getTypeName() << ")";
				parsed = false;
			}
		}
	}

	return parsed ;
}
bool ParameterMap::isComplete()const
{
	LOG_IF( ! parsed, Debug, error ) << "You did not run parse() yet. This is very likely an error";
	return std::find_if( begin(), end(), neededP() ) == end();
}

ProgParameter::operator boost::scoped_ptr<_internal::ValueBase>::unspecified_bool_type()const
{
	boost::scoped_ptr<_internal::ValueBase> dummy;

	if( ( *this )->castTo<bool>() )dummy.reset( new Value<int16_t> );

	return  dummy;
}

}
}