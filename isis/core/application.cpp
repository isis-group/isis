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

#include <clocale>
#include "application.hpp"
#include "fileptr.hpp"
#define STR(s) _xstr_(s)
#define _xstr_(s) std::string(#s)

namespace isis
{
namespace util
{

Application::Application( const char name[], const char cfg[]): m_name( name )
{
	addLogging<CoreLog>("Core");
	addLogging<CoreDebug>("Core");
	addLogging<DataLog>("Data");
	addLogging<DataDebug>("Data");
	addLogging<ImageIoLog>("ImageIO");
	addLogging<ImageIoDebug>("ImageIO");
	addLogging<MathLog>("Math");
	addLogging<MathDebug>("Math");

	parameters["help"] = false;
	parameters["help"].setDescription( "Print help" );
	parameters["help"].needed() = false;

	parameters["locale"] = std::string("C");
	parameters["locale"].setDescription( "locale to use for parsing/printing (use empty string to enforce use of system locale)");
	parameters["locale"].needed() = false;
	
	if(strlen(cfg)){
		parameters["cfg"]=std::string(cfg);
		parameters["cfg"].setDescription("File to read configuration from");
		parameters["cfg"].needed()=false;
	}
	
	parameters["np"] = false;
	parameters["np"].needed() = false;
	parameters["np"].setDescription( "suppress progress bar" );
	parameters["np"].hidden() = true;
}
Application::~Application() {}

void Application::addLoggingParameter( std::string name )
{
	static const Selection dbg_levels( "error,warning,notice,info,verbose_info", "notice" );

	if( parameters.find( std::string( "d" ) + name ) == parameters.end() ) { //only add the parameter if it does not exist yet
		parameters[std::string( "d" ) + name] = dbg_levels;

		if( name.empty() )
			parameters[std::string( "d" ) + name].setDescription( "Log level for \"" + m_name + "\" itself" );
		else
			parameters[std::string( "d" ) + name].setDescription( "Log level for the \"" + name + "\" module(s)" );

		parameters[std::string( "d" ) + name].hidden() = true;
		parameters[std::string( "d" ) + name].needed() = false;
	}
}
void Application::removeLogging( std::string name )
{
	parameters.erase( std::string( "d" ) + name );
	logs.erase( name );
}

bool Application::addConfigFile(const std::string& filename)
{
	data::FilePtr f(filename);
	if(f.good()){
		const data::ValueArray< uint8_t > buffer=f.at<uint8_t>(0);
		if(configuration.readJson(&buffer[0],&buffer[buffer.getLength()],'/')==0){
			boost::optional< PropertyMap& > param=configuration.queryBranch("parameters");
			// if there is a "parameters" section in the file, use that as default parameters for the app
			if(param){
				for(PropertyMap::PropPath p:param->getLocalProps()){
					assert(p.size()==1);
					ProgParameter &dst=parameters[p.front().c_str()];
					PropertyValue &src=param->touchProperty(p);
					if(!dst.isParsed()){ // don't touch it, if its not just the default
						LOG(Runtime,verbose_info) << "Setting parameter " << std::make_pair(p,src) << " from configuration";
						if(dst.isEmpty())
							dst.swap(src);
						else if(!dst.front().apply(src.front())){ //replace builtin default with value from config if there is one already
							LOG(Runtime,warning) << "Failed to apply parameter " << std::make_pair(p,src) << " from configuration, skipping ..";
							continue;
						}
					}
					dst.needed()=false; //param has got its deafult from the configuration, so its not needed anymore
					param->remove(p);
				}
			}
			return true;
		} else {
			LOG(Runtime,warning) << "Failed to parse configuration file " << util::MSubject(filename);
		}
	} 
	return false;
}
const PropertyMap& Application::config() const{return configuration;}


void Application::addExample ( std::string params, std::string desc )
{
	m_examples.push_back( std::make_pair( params, desc ) );
}

bool Application::init( int argc, char **argv, bool exitOnError )
{
	bool err = false;
	m_filename=argv[0];

	if ( parameters.parse( argc, argv ) ) {
		if ( parameters["help"] ) {
			printHelp( true );
			exit( 0 );
		}
	} else {
		LOG( Runtime, error ) << "Failed to parse the command line";
		err = true;
	}
	std::map< std::string, ProgParameter >::iterator cfg=parameters.find("cfg");
	if(cfg!=parameters.end()){ 
		if(!addConfigFile(cfg->second.as<std::string>()) && exitOnError){
			std::cerr << "Exiting..." << std::endl;
			exit( 1 );
		}
	}

	resetLogging();
	
	if ( ! parameters.isComplete() ) {
		std::cerr << "Missing parameters: ";

		for ( ParameterMap::iterator iP = parameters.begin(); iP != parameters.end(); iP++ ) {
			if ( iP->second.isNeeded() ) {std::cerr << "-" << iP->first << "  ";}
		}

		std::cerr << std::endl;
		err = true;
	}

	if ( err ) {
		printHelp();

		if( exitOnError ) {
			std::cerr << "Exiting..." << std::endl;
			exit( 1 );
		}
	}
	
	if(!parameters["np"])
		feedback().reset(new util::ConsoleFeedback);


	const std::string loc=parameters["locale"];
	if(!loc.empty())
		std::setlocale(LC_ALL,loc.c_str());

	return ! err;
}
void Application::resetLogging()
{
	for( auto &ref: logs ) {
		const std::string dname = std::string( "d" ) + ref.first;
		assert( !parameters[dname].isEmpty() ); // this must have been set by addLoggingParameter (called via addLogging)
		const LogLevel level = ( LogLevel )( uint16_t )parameters[dname].as<Selection>();
		for( setLogFunction setter: ref.second ) {
			( this->*setter )( level );
		}
	}
}

void Application::printHelp( bool withHidden )const
{
	typedef std::list<std::pair<std::string, std::string> >::const_reference example_type;
	std::cerr << this->m_name << " (using isis " << getCoreVersion() << ")" << std::endl;
	std::cerr << "Usage: " << this->m_filename << " <options>" << std::endl << "Where <options> includes:" << std::endl;;

	for ( ParameterMap::const_iterator iP = parameters.begin(); iP != parameters.end(); iP++ ) {
		std::string pref;

		if ( iP->second.isNeeded() ) {
			pref = ". Required.";
		} else if( iP->second.isHidden() ) {
			if( !withHidden )
				continue; // if its hidden, not needed, and wie want the short version skip this parameter
		}

		if ( ! iP->second.isNeeded() ) {
			pref = ". Default: \"" + iP->second.toString() + "\".";
		}

		std::cerr
				<< "\t-" << iP->first << " <" << iP->second.getTypeName() << ">" << std::endl
				<< "\t\t" << iP->second.description() << pref << std::endl;

		if ( iP->second.is<Selection>() ) {
			const Selection &ref = iP->second.castTo<Selection>();
			const std::list< istring > entries = ref.getEntries();
			std::list< istring >::const_iterator i = entries.begin();
			std::cerr << "\t\tOptions are: \"" << *i << "\"";

			for( i++ ; i != entries.end(); i++ ) {
				std::list< istring >::const_iterator dummy = i;
				std::cout << ( ( ++dummy ) != entries.end() ? ", " : " or " ) << "\"" << *i << "\"";
			}

			std::cerr << "." << std::endl;
		}
	}

	if( !m_examples.empty() ) {
		std::cout << "Examples:" << std::endl;

		for( example_type ex :  m_examples ) {
			std::cout << '\t' << m_filename << " " << ex.first << '\t' << ex.second << std::endl;
		}
	}
}

std::shared_ptr< MessageHandlerBase > Application::getLogHandler( std::string /*module*/, isis::LogLevel level )const
{
	return std::shared_ptr< MessageHandlerBase >( level ? new util::DefaultMsgPrint( level ) : 0 );
}
const std::string Application::getCoreVersion( void )
{
#ifdef ISIS_RCS_REVISION
	return STR( _ISIS_VERSION_MAJOR ) + "." + STR( _ISIS_VERSION_MINOR ) + "." + STR( _ISIS_VERSION_PATCH ) + " [" + STR( ISIS_RCS_REVISION ) + " " + __DATE__ + "]";
#else
	return STR( _ISIS_VERSION_MAJOR ) + "." + STR( _ISIS_VERSION_MINOR ) + "." + STR( _ISIS_VERSION_PATCH ) + " [" + __DATE__ + "]";;
#endif
}

std::shared_ptr<util::ProgressFeedback>& Application::feedback(){
	static std::shared_ptr<util::ProgressFeedback> fbk;
	return fbk;
}
}
}
#undef STR
#undef _xstr_

