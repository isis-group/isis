/****************************************************************
 *
 * Copyright (C) ISIS Dev-Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Author: Enrico Reimer, <reimer@cbs.mpg.de>, 2010
 *
 *****************************************************************/

#include "application.hpp"
#include <boost/foreach.hpp>

#define STR(s) _xstr_(s)
#define _xstr_(s) std::string(#s)

namespace isis
{
namespace util
{

Application::Application( const char name[] ): m_name( name )
{
	Selection dbg_levels( "error,warning,notice,info,verbose_info" );
	dbg_levels.set( "warning" );
	parameters["dCore"] = dbg_levels;
	parameters["dCore"].setDescription( "Debugging level for the Core module" );
	parameters["dCore"].hidden() = true;

	parameters["dData"] = dbg_levels;
	parameters["dData"].setDescription( "Debugging level for the Data module" );
	parameters["dData"].hidden() = true;

	parameters["dImageIO"] = dbg_levels;
	parameters["dImageIO"].setDescription( "Debugging level for the ImageIO module" );
	parameters["dImageIO"].hidden() = true;

	parameters["help"] = false;
	parameters["help"].setDescription( "Print help" );
	BOOST_FOREACH( ParameterMap::reference ref, parameters ) //none of these is needed
	ref.second.needed() = false;
}
Application::~Application() {}

bool Application::init( int argc, char **argv, bool exitOnError )
{
	bool err = false;
	m_filename = argv[0];

	if ( parameters.parse( argc, argv ) ) {
		if ( parameters["help"] ) {
			printHelp( true );
			exit( 0 );
		}
	} else {
		LOG( Runtime, error ) << "Failed to parse the command line";
		err = true;
	}

	if ( ! parameters.isComplete() ) {
		std::cerr << "Missing parameters: ";

		for ( ParameterMap::iterator iP = parameters.begin(); iP != parameters.end(); iP++ ) {
			if ( iP->second.isNeeded() ) {std::cerr << "-" << iP->first << "  ";}
		}

		std::cerr << std::endl;
		err = true;
	}

	if( parameters["dCore"].isSet() ) {
		setLog<CoreDebug>( ( LogLevel )( uint16_t )parameters["dCore"]->as<Selection>() ); //trigger explicit cast from Selection to int and then to LogLevel
		setLog<CoreLog>( ( LogLevel )( uint16_t )parameters["dCore"]->as<Selection>() );
	}

	if( parameters["dData"].isSet() ) {
		setLog<DataDebug>( ( LogLevel )( uint16_t )parameters["dData"]->as<Selection>() );
		setLog<DataLog>( ( LogLevel )( uint16_t )parameters["dData"]->as<Selection>() );
	}

	if( parameters["dImageIO"].isSet() ) {
		setLog<ImageIoDebug>( ( LogLevel )( uint16_t )parameters["dImageIO"]->as<Selection>() );
		setLog<ImageIoLog>( ( LogLevel )( uint16_t )parameters["dImageIO"]->as<Selection>() );
	}

	if ( err ) {
		printHelp();

		if( exitOnError ) {
			std::cerr << "Exiting..." << std::endl;
			exit( 1 );
		}
	}

	return ! err;
}
void Application::printHelp( bool withHidden )const
{
	std::cerr << this->m_name << " (using isis " << getCoreVersion() << ")" << std::endl;
	std::cerr << "Usage: " << this->m_filename << " <options>, where <options> includes:" << std::endl;

	for ( ParameterMap::const_iterator iP = parameters.begin(); iP != parameters.end(); iP++ ) {
		std::string pref;

		if ( iP->second.isNeeded() ) {
			pref = ". Required.";
		} else if( iP->second.isHidden() ) {
			if( !withHidden )
				continue; // if its hidden, not needed, and wie want the short version skip this parameter
		}

		if ( ! iP->second.isNeeded() ) {
			pref = ". Default: \"" + iP->second.toString() + "\"";
		}

		std::cerr << "\t-" << iP->first << " <" << iP->second->getTypeName() << ">" << std::endl;

		if ( iP->second->is<Selection>() ) {
			const Selection &ref = iP->second->castTo<Selection>();
			std::cerr << "\t\tOptions are: " <<  ref.getEntries() << std::endl;
		}

		std::cerr << "\t\t" << iP->second.description() << pref << std::endl;
	}
}

boost::shared_ptr< _internal::MessageHandlerBase > Application::getLogHandler( std::string /*module*/, isis::LogLevel level )const
{
	return boost::shared_ptr< _internal::MessageHandlerBase >( level ? new util::DefaultMsgPrint( level ) : 0 );
}
const std::string Application::getCoreVersion( void )
{
#ifdef ISIS_RCS_REVISION
	return STR( _ISIS_VERSION_MAJOR ) + "." + STR( _ISIS_VERSION_MINOR ) + "." + STR( _ISIS_VERSION_PATCH ) + " [" + STR( ISIS_RCS_REVISION ) + "]";
#else
	return STR( _ISIS_VERSION_MAJOR ) + "." + STR( _ISIS_VERSION_MINOR ) + "." + STR( _ISIS_VERSION_PATCH );
#endif
}

}
}
#undef STR
#undef _xstr_

