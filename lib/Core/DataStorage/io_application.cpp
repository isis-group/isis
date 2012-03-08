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
IOApplication::IOApplication( const char name[], bool have_input, bool have_output ):
	Application( name ),
	m_input( have_input ), m_output( have_output ), feedback( new util::ConsoleFeedback )
{
	if ( have_input )
		addInput( parameters );

	if ( have_output )
		addOutput( parameters );
}

IOApplication::~IOApplication()
{
	data::IOFactory::setProgressFeedback( boost::shared_ptr<util::ProgressFeedback>() );
}

bool IOApplication::init( int argc, char **argv, bool exitOnError )
{
	if ( ! util::Application::init( argc, argv, exitOnError ) )
		return false;

	if ( m_input ) {
		return autoload( exitOnError );
	}

	return true;
}

void IOApplication::addInput ( util::ParameterMap &parameters, bool needed, const std::string &suffix, const std::string &desc )
{
	parameters[std::string( "in" )+suffix] = util::slist();
	parameters[std::string( "in" )+suffix].setDescription( std::string( "input file(s) or directory(s)" ) + desc );
	parameters[std::string( "in" )+suffix].needed() = needed;

	parameters[std::string( "rf" )+suffix] = std::string();
	parameters[std::string( "rf" )+suffix].needed() = false;
	parameters[std::string( "rf" )+suffix].hidden() = true;

	parameters[std::string( "rf" )+suffix].setDescription( std::string( "Override automatic detection of file suffix for reading" + desc + " with given value" ) );
	parameters[std::string( "rdialect" )+suffix] = std::string();
	parameters[std::string( "rdialect" )+suffix].needed() = false;
	parameters[std::string( "rdialect" )+suffix].setDescription(
		std::string( "choose dialect for reading" ) + desc + " . The available dialects depend on the capabilities of IO plugins" );

	if( parameters.find( "np" ) == parameters.end() ) {
		parameters["np"] = false;
		parameters["np"].needed() = false;
		parameters["np"].setDescription( "suppress progress bar" );
		parameters["np"].hidden() = true;
	}
}

void IOApplication::addOutput ( util::ParameterMap &parameters, bool needed, const std::string &suffix, const std::string &desc )
{
	parameters[std::string( "out" )+suffix] = std::string();
	parameters[std::string( "out" )+suffix].setDescription( "output filename" + desc );
	parameters[std::string( "out" )+suffix].needed() = needed;

	parameters[std::string( "wf" )+suffix] = std::string();
	parameters[std::string( "wf" )+suffix].needed() = false;
	parameters[std::string( "wf" )+suffix].setDescription( "Override automatic detection of file suffix for writing" + desc + " with given value" );
	parameters[std::string( "wf" )+suffix].hidden() = true;

	parameters[std::string( "wdialect" )+suffix] = std::string();
	parameters[std::string( "wdialect" )+suffix].needed() = false;
	parameters[std::string( "wdialect" )+suffix].setDescription( "Choose dialect for writing" + desc + ". Use \"--help\" for a list of the plugins and their supported dialects" );
	std::map<unsigned short, std::string> types = util::getTypeMap( false, true );
	// remove some types which are useless as representation
	// "(unsigned short)" is needed because otherwise erase would take the reference of a static constant which is only there during compile time
	types.erase( ( unsigned short )data::ValuePtr<util::Selection>::staticID );
	types.erase( ( unsigned short )data::ValuePtr<std::string>::staticID );
	types.erase( ( unsigned short )data::ValuePtr<boost::posix_time::ptime>::staticID );
	types.erase( ( unsigned short )data::ValuePtr<boost::gregorian::date>::staticID );
	types.erase( ( unsigned short )data::ValuePtr<util::ilist>::staticID );
	types.erase( ( unsigned short )data::ValuePtr<util::dlist>::staticID );
	types.erase( ( unsigned short )data::ValuePtr<util::slist>::staticID );

	for( std::map<unsigned short, std::string>::iterator i = types.begin(); i != types.end(); i++ ) {
		i->second.resize( i->second.find_last_not_of( '*' ) + 1 );
	}

	parameters[std::string( "repn" )+suffix] = util::Selection( types );
	parameters[std::string( "repn" )+suffix].needed() = false;
	parameters[std::string( "repn" )+suffix].setDescription(
		"Representation in which the data" + desc + " shall be written" );

	parameters[std::string( "scale_mode" )+suffix] = util::Selection( "noscale,autoscale,noupscale,upscale", "autoscale" );
	parameters[std::string( "scale_mode" )+suffix].needed() = false;
	parameters[std::string( "scale_mode" )+suffix].hidden() = true;

	parameters[std::string( "scale_mode" )+suffix].setDescription(
		"Scaling strategy to be used when converting into the datatype given in with -repn" + suffix );

	if( parameters.find( "np" ) == parameters.end() ) {
		parameters["np"] = false;
		parameters["np"].needed() = false;
		parameters["np"].setDescription( "suppress progress bar" );
		parameters["np"].hidden() = true;
	}
}


void IOApplication::printHelp( bool withHidden ) const
{
	util::Application::printHelp( withHidden );

	if( withHidden ) {
		std::cerr << std::endl << "Available IO Plugins:" << std::endl;
		data::IOFactory::FileFormatList plugins = data::IOFactory::getFormats();
		BOOST_FOREACH( data::IOFactory::FileFormatList::const_reference pi, plugins ) {
			std::cerr << std::endl << "\t" << pi->getName() << " (" << pi->plugin_file.file_string() << ")" << std::endl;
			std::cerr << "\t=======================================" << std::endl;
			const std::list<util::istring> suff = pi->getSuffixes();
			const std::list<std::string> dialects = util::stringToList<std::string>( pi->dialects( "" ) );
			std::cerr << "\tsupported suffixes: " << util::listToString( suff.begin(), suff.end(), "\", \"", "\"", "\"" )  << std::endl;

			if( !dialects.empty() )
				std::cerr << "\tsupported dialects: " << util::listToString( dialects.begin(), dialects.end(), "\", \"", "\"", "\"" )  << std::endl;
		}
	}
}

bool IOApplication::autoload( bool exitOnError )
{
	return autoload( parameters, images, exitOnError, "", feedback );

}
bool IOApplication::autoload ( const util::ParameterMap &parameters, std::list<Image> &images, bool exitOnError, const std::string &suffix,  boost::shared_ptr<util::ConsoleFeedback> feedback )
{
	util::slist input = parameters[std::string( "in" )+suffix];
	std::string rf = parameters[std::string( "rf" )+suffix];
	std::string dl = parameters[std::string( "rdialect" )+suffix];
	LOG( Runtime, info )
			<< "loading " << util::MSubject( input )
			<< ( rf.empty() ? "" : std::string( " using the format: " ) + rf )
			<< ( ( !rf.empty() && !dl.empty() ) ? " and" : "" )
			<< ( dl.empty() ? "" : std::string( " using the dialect: " ) + dl );

	bool no_progress = parameters["np"];

	if( !no_progress && feedback ) {
		data::IOFactory::setProgressFeedback( feedback );
	}

	const std::list< Image > tImages = data::IOFactory::load( input, rf, dl );

	images.insert( images.end(), tImages.begin(), tImages.end() );

	if ( images.empty() ) {
		if ( exitOnError ) {
			LOG( Runtime, notice ) << "No data acquired, exiting...";
			exit( 1 );
		} else
			return false;
	} else {
		for( std::list<data::Image>::const_iterator a = images.begin(); a != images.end(); a++ ) {
			for( std::list<data::Image>::const_iterator b = a; ( ++b ) != images.end(); ) {
				const util::PropertyMap &aref = *a, bref = *b;
				LOG_IF( aref.getDifference( bref ).empty(), Runtime, warning ) << "The metadata of the images from "
						<< aref.getPropertyAs<std::string>( "source" ) << ":" << std::distance<std::list<Image> ::const_iterator>( images.begin(), a )
						<< " and " << bref.getPropertyAs<std::string>( "source" ) << ":" << std::distance<std::list<Image> ::const_iterator>( images.begin(), b )
						<< " are equal. Maybe they are duplicates.";
			}
		}
	}

	return true;
}


bool IOApplication::autowrite( Image out_image, bool exitOnError ) {return autowrite( std::list<Image>( 1, out_image ), exitOnError );}
bool IOApplication::autowrite( std::list<Image> out_images, bool exitOnError )
{
	const bool no_progress = parameters["np"];
	return autowrite( parameters, out_images, exitOnError, "", no_progress ? boost::shared_ptr<util::ConsoleFeedback>() : feedback );
}

bool IOApplication::autowrite ( const util::ParameterMap &parameters, Image out_image, bool exitOnError, const std::string &suffix, boost::shared_ptr<util::ConsoleFeedback> feedback )
{
	return autowrite( parameters, std::list<Image>( 1, out_image ), exitOnError, suffix, feedback );
}
bool IOApplication::autowrite ( const util::ParameterMap &parameters, std::list< Image > out_images, bool exitOnError, const std::string &suffix, boost::shared_ptr<util::ConsoleFeedback> feedback )
{
	const util::Selection repn = parameters[std::string( "repn" )+suffix];
	const util::Selection scale_mode = parameters[std::string( "scale_mode" )+suffix];
	const std::string output = parameters[std::string( "out" )+suffix];
	const std::string wf = parameters[std::string( "wf" )+suffix];
	const std::string dl = parameters[std::string( "wdialect" )+suffix];
	LOG( Runtime, info )
			<< "Writing " << out_images.size() << " images"
			<< ( repn ? std::string( " as " ) + ( std::string )repn : "" )
			<< " to " << util::MSubject( output )
			<< ( wf.empty() ? "" : std::string( " using the format: " ) + wf )
			<< ( ( !wf.empty() && !dl.empty() ) ? " and" : "" )
			<< ( dl.empty() ? "" : std::string( " using the dialect: " ) + dl );


	LOG_IF( parameters[std::string( "scale_mode" )+suffix].isSet() && repn == 0, Runtime, warning )
			<< "Ignoring -scale_mode" << suffix << " " << util::MSubject( scale_mode )
			<<  ", because -repn" << suffix << " was not given";

	if( repn != 0 ) {
		BOOST_FOREACH( std::list<Image>::reference ref, out_images ) {
			ref.convertToType( repn, static_cast<autoscaleOption>( scale_mode - 1 ) ); //noscale is 0 but 1 in the selection
		}
	}

	if( feedback )
		data::IOFactory::setProgressFeedback( feedback );

	if ( ! IOFactory::write( out_images, output, wf, dl ) ) {
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

boost::shared_ptr< util::MessageHandlerBase > IOApplication::getLogHandler( std::string module, LogLevel level ) const
{
	return isis::util::Application::getLogHandler( module, level );
}


}
}

