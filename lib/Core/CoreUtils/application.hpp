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
	boost::filesystem::path m_filename;
	std::list<std::pair<std::string, std::string> > m_examples;
	void addLoggingParameter( std::string name );

protected:
	typedef void ( Application::*setLogFunction )( LogLevel level )const;
	std::map<std::string, std::list<setLogFunction> > logs;
	virtual std::shared_ptr<MessageHandlerBase> getLogHandler( std::string module, isis::LogLevel level )const;

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
	 * Add a logging module.
	 * This enables a logging module MODULE and adds related program parameters to control its logging level.
	 * This logging level then applies to LOG-commands using that specific module.
	 *
	 * The MODULE must be a struct like
	 * \code struct MyLogModule {static const char *name() {return "MyModuleLog";}; enum {use = _ENABLE_LOG};}; \endcode
	 * than \code addLogging<MyLogModule>("MyLog"); \endcode will install a parameter "-dMyLog" which will control all calls to
	 * \code LOG(MyLogModule,...) << ... \endcode if _ENABLE_LOG was set for the compilation. Otherwise this commands will not have an effect.
	 *
	 * Multiple MODLUES can have the same parameter name, so
	 * \code
	 * struct MyLogModule {static const char *name() {return "MyModuleLog";}; enum {use = _ENABLE_LOG};};
	 * struct MyDebugModule {static const char *name() {return "MyModuleDbg";}; enum {use = _ENABLE_DEBUG};};
	 *
	 * addLogging<MyLogModule>("MyLog");
	 * addLogging<MyDebugModule>("MyLog");
	 * \endcode will control \code LOG(MyLogModule,...) << ... \endcode and \code LOG(MyDebugModule,...) << ... \endcode through the parameter "-dMyLog".
	 *
	 * \param name the name used for the program parameters and their description
	 *
	 * \note This does not set the logging handler. That is done by reimplementing getLogHandler( std::string module, isis::LogLevel level )const.
	 * \note If name is an empty string (""), its assumed as the logging for the application.
	 */
	template<typename MODULE> void addLogging( std::string name="" ) {
		addLoggingParameter( name );
		logs[name].push_back( &Application::setLog<MODULE> );
	}
	/**
	 * Removes a logging parameter from the application.
	 * \param name the parameter name without "-d"
	 * \note the logging module cannot be removed at runtime - its usage is controled by the _ENABLE_LOG / _ENABLE_DEBUG defines at compile time.
	 */
	void removeLogging( std::string name );

	/**
	 * Add an example for the programs usage.
	 * \param parameters the parameter string for the example call
	 * \param desc the description of the example
	 */
	void addExample( std::string parameters, std::string desc );

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
	template<typename MODULE> void setLog( LogLevel level ) const {
		if ( !MODULE::use );
		else _internal::Log<MODULE>::setHandler( getLogHandler( MODULE::name(), level ) );

		LOG( Debug, info ) << "Setting logging for module " << MSubject( MODULE::name() ) << " to level " << level;
	}
	//get the version of the coreutils
	static const std::string getCoreVersion( void );
};
}
}
#endif // APPLICATION_HPP
