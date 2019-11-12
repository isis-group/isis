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
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "progparameter.hpp"

namespace isis
{
namespace util
{

ProgParameter::ProgParameter(): m_hidden( false ), m_parsed( false )
{
	needed() = true;
}

bool ProgParameter::isHidden() const
{
	return m_hidden;
}
bool &ProgParameter::hidden()
{
	return m_hidden;
}

bool ProgParameter::parse( const std::string &prop )
{
	ValueBase &me = this->front();
	bool ret = false;

	if ( prop.empty() ) {
		if ( me.is<bool>() ) {
			me.castTo<bool>() = true;
			ret = true;
		}
	} else {
		ret = ValueBase::convert( Value<std::string>(prop), me );
	}

	LOG_IF( ret, Debug, info ) << "Parsed " << MSubject( prop ) << " as " << me.toString( true );

	if( ret ) {
		needed()= false;//remove needed flag, because the value is set (aka "not needed anymore")
		m_parsed = true; 
	}

	return ret;
}
bool ProgParameter::parse_list( const slist& theList )
{
	ValueBase &me = this->front();
	bool ret = false;

	ret = ValueBase::convert( Value<slist>(theList), me );
	if(theList.empty()){
		LOG_IF( ret, Debug, info )
		<< "Parsed empty parameter list as " << me.toString( true );
	}else{
		LOG_IF( ret, Debug, info )
			<< "Parsed parameter list " << MSubject( util::listToString( theList.begin(), theList.end(), " ", "", "" ) ) << " as " << me.toString( true );
	}

	if( ret ) {
		needed()= false;//remove needed flag, because the value is set (aka "not needed anymore")
		m_parsed = true; 
	}
	
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
bool ProgParameter::isParsed() const
{
	return m_parsed;
}


ParameterMap::ParameterMap(): parsed( false ) {}

bool ParameterMap::parse( int argc, char **argv )
{
	parsed = true;
	std::string pName;

	for ( int i = 1; i < argc; ) { // scan through the command line, to find next parameter
		if ( argv[i][0] == '-' && argv[i][1]!=0 ) { //seems like we found a new parameter here
			pName = argv[i];
			pName.erase( 0, pName.find_first_not_of( '-' ) );
		} else {
			LOG( Runtime, error ) << "Ignoring unexpected non-parameter " << MSubject( argv[i] );
		}

		i++;

		if( !pName.empty() ) { // if we got a parameter before
			const int start = i;

			std::list<std::string> matchingStrings;
			for( ParameterMap::const_reference parameterRef :  *this ) {
				if( parameterRef.first.find( pName ) == 0 ) {
					if( parameterRef.first.length() == pName.length() ) { //if its an exact match
						matchingStrings = std::list<std::string>( 1, pName ); //use that
						break;// and stop searching for partial matches
					} else
						matchingStrings.push_back( parameterRef.first );
				}
			}

			if( matchingStrings.size() > 1 ) { // multiple matches
				std::stringstream matchingStringStream;
				for( std::list<std::string>::const_reference stringRef :  matchingStrings ) {
					matchingStringStream << stringRef << " ";
				}
				LOG( Runtime, warning )
						<< "The parameter \"" << pName << "\" is ambiguous. The parameters \""
						<< matchingStringStream.str().erase( matchingStringStream.str().size() - 1, 1 )
						<< "\" are possible. Ignoring this parameter!";
				continue;
			} else if ( matchingStrings.empty() ) { // no match
				LOG( Runtime, warning ) << "Ignoring unknown parameter " << MSubject( std::string( "-" ) + pName + " " + listToString( argv + start, argv + i, " ", "", "" ) );
				continue;
			} 
				
			// exact one match
			ProgParameter &match=at(matchingStrings.front() );
			
			// check if parameter can be empty (bool only)
			if(match.is<bool>()){ //no parameters expected
				match.parse(""); // ok thats it nothing else to do here
			} else { //go through all parameters 
				i++;//skip the first one, that should be here (enables us to have parameters like "-20")
				while( i < argc && !(argv[i][0] == '-' && argv[i][1] != 0) ) {  
					i++;//collect the remaining parameters, while there are some ..
				}

				parsed = match.is<util::slist>() ? //dont do tokenizing if the target is an slist (is already done by the shell)
					match.parse_list( util::slist( argv + start, argv + i ) ) :
					match.parse( listToString( argv + start, argv + i, ",", "", "" ) );
				LOG_IF(!parsed, Runtime, error )
					<< "Failed to parse the parameter " << MSubject( std::string( "-" ) + matchingStrings.front() ) << ": "
					<< ( start == i ? "nothing" : listToString( argv + start, argv + i, " ", "\"", "\"" ) )
					<< " was given, but a " << match.getTypeName() << " was expected.";
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

const ProgParameter ParameterMap::operator[] ( const std::string key ) const
{
	std::map<std::string, ProgParameter>::const_iterator at = find( key );

	if( at != end() )
		return at->second;
	else {
		LOG( Debug, error ) << "The requested parameter " << util::MSubject( key ) << " does not exist";
		return ProgParameter();
	}
}
ProgParameter &ParameterMap::operator[] ( const std::string key ) {return std::map<std::string, ProgParameter>::operator[]( key );}

#ifdef BOOST_NO_EXPLICIT_CONVERSION_OPERATORS
ProgParameter::operator std::unique_ptr<ValueBase>::unspecified_bool_type()const
{
	std::unique_ptr<ValueBase> dummy;

	if( ( *this ).castTo<bool>() )dummy.reset( new Value<int16_t> );

	return  dummy;
}
#else
ProgParameter::operator bool()const
{
	return ( ( *this ).castTo<bool>() );
}
#endif

}
}
