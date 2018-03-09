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

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#include "io_application.hpp"
#include "io_factory.hpp"
#include <boost/mpl/for_each.hpp>


namespace isis
{
namespace data
{
	IOApplication::IOApplication( const char name[], bool have_input, bool have_output, const char cfg[] ):
	Application( name, cfg ), m_input( have_input )
{
	if ( have_input )
		addInput();

	if ( have_output )
		addOutput();

	parameters["help-io"] = false;
	parameters["help-io"].needed() = false;
	parameters["help-io"].setDescription( "List all loaded IO plugins and their supported formats, exit after that" );
}

IOApplication::~IOApplication()
{
	data::IOFactory::setProgressFeedback( std::shared_ptr<util::ProgressFeedback>() );
}

bool IOApplication::init( int argc, char **argv, bool exitOnError )
{
	const bool ok = util::Application::init( argc, argv, exitOnError );

	if ( !ok  )
		return false;

	if ( m_input ) {
		images.splice(images.end(),autoload( exitOnError ));
	}

	return true;
}

void IOApplication::addInput ( util::ParameterMap &parameters, const std::string &desc, const std::string &suffix, bool needed)
{
	parameters[std::string( "in" ) + suffix] = util::slist();
	parameters[std::string( "in" ) + suffix].setDescription( std::string( "input file(s) or directory(s)" ) + desc );
	parameters[std::string( "in" ) + suffix].needed() = needed;

	parameters[std::string( "rf" ) + suffix] = util::slist();
	parameters[std::string( "rf" ) + suffix].needed() = false;
	parameters[std::string( "rf" ) + suffix].hidden() = true;

	parameters[std::string( "rf" ) + suffix].setDescription( std::string( "Override automatic detection of file suffix for reading" + desc + " with given value" ) );
	parameters[std::string( "rdialect" ) + suffix] = util::slist();
	parameters[std::string( "rdialect" ) + suffix].needed() = false;
	parameters[std::string( "rdialect" ) + suffix].setDescription(
		std::string( "choose dialect(s) for reading" ) + desc + ". The available dialects depend on the capabilities of the used IO plugin" );
}
void IOApplication::addInput(const std::string& desc, const std::string& suffix, bool needed)
{
	addInput(parameters,desc,suffix,needed);
}


void IOApplication::addOutput( util::ParameterMap &parameters, const std::string &desc, const std::string &suffix, bool needed)
{
	parameters[std::string( "out" ) + suffix] = std::string();
	parameters[std::string( "out" ) + suffix].setDescription( "output filename" + desc );
	parameters[std::string( "out" ) + suffix].needed() = needed;

	parameters[std::string( "wf" ) + suffix] = util::slist();
	parameters[std::string( "wf" ) + suffix].needed() = false;
	parameters[std::string( "wf" ) + suffix].setDescription( "Override automatic detection of file suffix for writing" + desc + " with given value" );
	parameters[std::string( "wf" ) + suffix].hidden() = true;

	parameters[std::string( "wdialect" ) + suffix] = util::slist();
	parameters[std::string( "wdialect" ) + suffix].needed() = false;
	parameters[std::string( "wdialect" ) + suffix].setDescription( "Choose dialect(s) for writing" + desc + ". Use \"--help-io\" for a list of the plugins and their supported dialects" );
	std::map<unsigned short, std::string> types = util::getTypeMap( false, true );
	// remove some types which are useless as representation
	// "(unsigned short)" is needed because otherwise erase would take the reference of a static constant which is only there during compile time
	types.erase( ( unsigned short )data::ValueArray<util::Selection>::staticID() );
	types.erase( ( unsigned short )data::ValueArray<std::string>::staticID() );
	types.erase( ( unsigned short )data::ValueArray<util::date>::staticID() );
	types.erase( ( unsigned short )data::ValueArray<util::ilist>::staticID() );
	types.erase( ( unsigned short )data::ValueArray<util::dlist>::staticID() );
	types.erase( ( unsigned short )data::ValueArray<util::slist>::staticID() );

	for( std::map<unsigned short, std::string>::iterator i = types.begin(); i != types.end(); i++ ) {
		i->second.resize( i->second.find_last_not_of( '*' ) + 1 );
	}

	parameters[std::string( "repn" ) + suffix] = util::Selection( types );
	parameters[std::string( "repn" ) + suffix].needed() = false;
	parameters[std::string( "repn" ) + suffix].setDescription(
		"Representation in which the data" + desc + " shall be written" );

	parameters[std::string( "scale_mode" ) + suffix] = util::Selection( "noscale,autoscale,noupscale,upscale", "autoscale" );
	parameters[std::string( "scale_mode" ) + suffix].needed() = false;
	parameters[std::string( "scale_mode" ) + suffix].hidden() = true;

	parameters[std::string( "scale_mode" ) + suffix].setDescription(
		"Scaling strategy to be used when converting into the datatype given in with -repn" + suffix );

	if( parameters.find( "np" ) == parameters.end() ) {
		parameters["np"] = false;
		parameters["np"].needed() = false;
		parameters["np"].setDescription( "suppress progress bar" );
		parameters["np"].hidden() = true;
	}
}
void IOApplication::addOutput(const std::string& desc, const std::string& suffix, bool needed)
{
	addOutput(parameters,desc,suffix,needed);
}


void IOApplication::printHelp( bool withHidden ) const
{
	if( !parameters["help-io"].isParsed() ) { // if help-io was not given - print normal help
		Application::printHelp( withHidden );
	} else if( parameters["help-io"].as<bool>() ) { // if help-io was set to true
		std::cerr << std::endl << "Available IO Plugins:" << std::endl;
		data::IOFactory::FileFormatList plugins = data::IOFactory::getFormats();
		for( data::IOFactory::FileFormatList::const_reference pi :  plugins ) {
			std::cerr << std::endl << "\t" << pi->getName() << " (" << pi->plugin_file << ")" << std::endl;
			std::cerr << "\t=======================================" << std::endl;
			const std::list<util::istring> suff = pi->getSuffixes();
			const std::list<util::istring> dialects = pi->dialects();
			std::cerr << "\tsupported suffixes: " << util::listToString<util::istring>( suff.begin(), suff.end(), "\", \"", "\"", "\"" ).c_str()  << std::endl;

			if( !dialects.empty() )
				std::cerr << "\tsupported dialects: " << util::listToString( dialects.begin(), dialects.end(), "\", \"", "\"", "\"" )  << std::endl;
		}
	}
}

std::list< Image > IOApplication::autoload( bool exitOnError,const std::string &suffix,optional< util::slist& > rejected)
{
	return autoload( parameters, images, exitOnError, suffix, feedback(),rejected );

}
std::list< Image > IOApplication::autoload ( const util::ParameterMap &parameters, std::list<Image> &images, bool exitOnError, const std::string &suffix,  std::shared_ptr<util::ProgressFeedback> feedback, optional< util::slist& > rejected)
{
	util::slist input = parameters[std::string( "in" ) + suffix];
	util::slist rf = parameters[std::string( "rf" ) + suffix];
	util::slist dl = parameters[std::string( "rdialect" ) + suffix];
	
	bool no_progress = parameters["np"];

	if( !no_progress && feedback ) {
		data::IOFactory::setProgressFeedback( feedback );
	}

	std::list<util::istring> formatstack=util::listToList<util::istring>(rf.begin(),rf.end());
		
	std::list< Image > tImages;
	if(input.size()==1 && input.front()=="-"){
		LOG( Runtime, info )
			<< "loading from stdin" 
			<< util::NoSubject( rf.empty() ? "" : std::string( " using the format stack: " ) + util::listToString(rf.begin(),rf.end()) )
			<< util::NoSubject( ( !rf.empty() && !dl.empty() ) ? " and" : "" )
			<< util::NoSubject( dl.empty() ? "" : std::string( " using the dialect: " ) + util::listToString(dl.begin(),dl.end()) );
		tImages = data::IOFactory::load( std::cin.rdbuf(), formatstack, util::listToList<util::istring>(dl.begin(),dl.end()),rejected );
	} else {
		LOG( Runtime, info )
			<< "loading " << util::MSubject( input )
			<< util::NoSubject( rf.empty() ? "" : std::string( " using the format stack: " ) + util::listToString(rf.begin(),rf.end()) )
			<< util::NoSubject( ( !rf.empty() && !dl.empty() ) ? " and" : "" )
			<< util::NoSubject( dl.empty() ? "" : std::string( " using the dialect: " ) + util::listToString(dl.begin(),dl.end()) );
		tImages = data::IOFactory::load( input, formatstack, util::listToList<util::istring>(dl.begin(),dl.end()),rejected );
	}

	if ( tImages.empty() ) {
		if ( exitOnError ) {
			LOG( Runtime, notice ) << "No data acquired, exiting...";
			exit( 1 );
		} 
	} else {
		for( std::list<data::Image>::const_iterator a = tImages.begin(); a != tImages.end(); a++ ) {
			for( std::list<data::Image>::const_iterator b = a; ( ++b ) != tImages.end(); ) {
				const data::Image &aref = *a, bref = *b;
				LOG_IF( aref.getDifference( bref ).empty(), Runtime, warning ) << "The metadata of the images "
						<< aref.identify(true,false) << ":" << std::distance<std::list<Image> ::const_iterator>( tImages.begin(), a )
						<< " and " << bref.identify(true,false) << ":" << std::distance<std::list<Image> ::const_iterator>( tImages.begin(), b )
						<< " are equal. Maybe they are duplicates.";
			}
		}
	}

	return tImages;
}


bool IOApplication::autowrite( Image out_image, bool exitOnError ) {return autowrite( std::list<Image>( 1, out_image ), exitOnError );}
bool IOApplication::autowrite( std::list<Image> out_images, bool exitOnError )
{
	return autowrite( parameters, out_images, exitOnError, "");
}

bool IOApplication::autowrite ( const util::ParameterMap &parameters, Image out_image, bool exitOnError, const std::string &suffix )
{
	return autowrite( parameters, std::list<Image>( 1, out_image ), exitOnError, suffix );
}
bool IOApplication::autowrite ( const util::ParameterMap &parameters, std::list< Image > out_images, bool exitOnError, const std::string &suffix)
{
	const util::Selection repn = parameters[std::string( "repn" ) + suffix];
	const util::Selection scale_mode = parameters[std::string( "scale_mode" ) + suffix];
	const std::string output = parameters[std::string( "out" ) + suffix];
	const util::slist wf = parameters[std::string( "wf" ) + suffix];
	const util::slist dl = parameters[std::string( "wdialect" ) + suffix];
	LOG( Runtime, info )
			<< "Writing " << out_images.size() << " images"
			<< ( repn ? std::string( " as " ) + ( std::string )repn : "" )
			<< " to " << util::MSubject( output )
			<< ( wf.empty() ? "" : std::string( " using the format: " ) + util::listToString(wf.begin(),wf.end()) )
			<< ( ( !wf.empty() && !dl.empty() ) ? " and" : "" )
			<< ( dl.empty() ? "" : std::string( " using the dialect: " ) + util::listToString(dl.begin(),dl.end()) );


	LOG_IF( parameters[std::string( "scale_mode" ) + suffix].isParsed() && repn == 0, Runtime, warning )
			<< "Ignoring -scale_mode" << suffix << " " << util::MSubject( scale_mode )
			<<  ", because -repn" << suffix << " was not given";

	if( repn != 0 ) {
		for( std::list<Image>::reference ref :  out_images ) {
			ref.convertToType( repn, static_cast<autoscaleOption>( scale_mode - 1 ) ); //noscale is 0 but 1 in the selection
		}
	}

	if( feedback() )
		data::IOFactory::setProgressFeedback( feedback() );

	if ( ! IOFactory::write( out_images, output, util::listToList<util::istring>(wf.begin(),wf.end()), util::listToList<util::istring>(dl.begin(),dl.end()) ) ) {
		if ( exitOnError ) {
			LOG( Runtime, notice ) << "Failed to write, exiting...";
			exit( 1 );
		}

		return false;
	} else
		return true;
}

Image IOApplication::fetchImage()
{
	assert( !images.empty() );
	Image ret = images.front();
	images.pop_front();
	return ret;
}

std::shared_ptr< util::MessageHandlerBase > IOApplication::getLogHandler( std::string module, LogLevel level ) const
{
	return isis::util::Application::getLogHandler( module, level );
}


}
}

