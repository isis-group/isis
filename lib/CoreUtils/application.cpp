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

#include "application.hpp"
#include <boost/foreach.hpp>

namespace isis
{
namespace util
{

const LogLevel Application::LLMap[] = {LogLevel( 0 ), error, warning, info, verbose_info}; //uhh I'm soo devious

Application::Application( const char name[] ): m_name( name )
{
	Selection dbg_levels( "error,warning,info,verbose_info" );
	dbg_levels.set( "warning" );
	parameters["dCore"] = dbg_levels;
	parameters["dCore"].setDescription( "Debugging level for the Core module" );
	parameters["dData"] = dbg_levels;
	parameters["dData"].setDescription( "Debugging level for the Data module" );
	parameters["dImageIO"] = dbg_levels;
	parameters["dImageIO"].setDescription( "Debugging level for the ImageIO module" );
	parameters["help"] = false;
	parameters["help"].setDescription( "Print help" );
	BOOST_FOREACH( ParameterMap::reference ref, parameters ) //none of these is needed
	ref.second.needed() = false;
}
Application::~Application() {}

bool Application::init( int argc, char** argv, bool exitOnError )
{
	bool err = false;

	if ( parameters.parse( argc, argv ) ) {
		if ( parameters["help"] ) {
			printHelp();
			exit( 0 );
		}
	} else {
		LOG( Runtime, error ) << "Failed to parse the command line";
		err = true;
	}

	if ( not parameters.isComplete() ) {
		std::cout << "Missing parameters: ";

		for ( ParameterMap::iterator iP = parameters.begin(); iP != parameters.end(); iP++ ) {
			if ( iP->second.needed() ) {std::cout << iP->first << "  ";}
		}

		err = true;
	}

	setLog<CoreDebug>( LLMap[parameters["dCore"]] );
	setLog<CoreLog>( LLMap[parameters["dCore"]] );
	setLog<DataDebug>( LLMap[parameters["dData"]] );
	setLog<DataLog>( LLMap[parameters["dData"]] );
	setLog<ImageIoDebug>( LLMap[parameters["dImageIO"]] );
	setLog<ImageIoLog>( LLMap[parameters["dImageIO"]] );

	if ( err and exitOnError ) {
		printHelp();
		std::cout << "Exiting..." << std::endl;
		exit( 1 );
	}

	return not err;
}
void Application::printHelp()const
{
	std::cout << std::endl;
	std::cout << "Usage: " << this->m_name << " <options>, where <options> includes:" << std::endl;

	for ( ParameterMap::const_iterator iP = parameters.begin(); iP != parameters.end(); iP++ ) {
		std::string pref;

		if ( iP->second.needed() ) {pref = " Required.";}

		if ( not iP->second.needed() and not iP->second->is<dlist>() ) {pref = " Default: " + iP->second.toString();};

		std::cout << "\t-" << iP->first << " <" << iP->second->typeName() << ">" << std::endl;

		if ( iP->second->is<Selection>() ) {
			const Selection &ref = iP->second->cast_to_Type<Selection>();
			std::cout << "\t\tOptions are: " <<  ref.getEntries() << std::endl;
		}

		std::cout << "\t\t" << iP->second.description() << pref << std::endl;
	}
}

boost::shared_ptr< _internal::MessageHandlerBase > Application::getLogHandler( std::string module, isis::LogLevel level )const
{
	return boost::shared_ptr< _internal::MessageHandlerBase >( level ? new util::DefaultMsgPrint( level ) : 0 );
}

}
}
