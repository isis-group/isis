// -*- fill-column: 120; c-basic-offset: 4 -*-
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

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "progparameter.hpp"

namespace isis
{
namespace util
{

/**
 * The ISIS base application class.
 *
 * To speed up the creation of new command line tools, the ISIS core library contains a number of helper classes based
 * on Application. An ISIS application class provides a parameter parser that tokenizes the program argument list
 * according to given parameter names and types and saves the results in an internal parameter map.
 *
 * An application needs to be initialized explicitly with a call to init().
 *
 * Usage instructions and a short paramter list can be shown with the method printHelp().
 *
 * See also: isis::data::IOApplication, isis::util::ParameterMap and \link isis::LogLevel isis::LogLevel \endlink
 */
class Application
{
	const std::string m_name;
	std::string m_filename;

protected:
	virtual boost::shared_ptr<MessageHandlerBase> getLogHandler( std::string module, isis::LogLevel level )const;

public:

	ParameterMap parameters;

	/**
	 * Default Constructor.
	 *
	 * Creates the application and its parameter map.
	 * No programm parameter is parsed here. To do that use init()
	 *
	 * \param name name of the application.
	 */
	Application( const char name[] );
	virtual ~Application();

	/**
	 * Initializes the programm parameter map using argc/argv.
	 * For every entry in parameters an corresponding command line parameter is searched and parsed.
	 * A command line parameter corresponds to an entry of parameters if it string-equals caseless to this entry and is preceded by "-".
	 *
	 * \param argc ammount of command line parameters in argv
	 * \param argv array of const char[] containing the command line parameters
	 * \param exitOnError if true the programm will exit, if there is a problem during the initialisation (like missing parameters).
	 */
	virtual bool init( int argc, char **argv, bool exitOnError = true );
	/**
	 * Virtual function to display a short help text.
	 * Ths usually shall print the programm name plus all entries of parameters with their description.
	 */
	virtual void printHelp( bool withHidden = false )const;
	/// Set the logging level for the specified module
	template<typename MODULE> void setLog( LogLevel level ) {
		if ( !MODULE::use );
		else _internal::Log<MODULE>::setHandler( getLogHandler( MODULE::name(), level ) );
	}
	//get the version of the coreutils
	static const std::string getCoreVersion( void );
};
}
}
#endif // APPLICATION_HPP
