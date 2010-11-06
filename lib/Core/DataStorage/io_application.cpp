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

#include "DataStorage/io_application.hpp"
#include "DataStorage/io_factory.hpp"
#include <boost/mpl/for_each.hpp>

namespace isis
{
namespace data
{
IOApplication::IOApplication( const char name[], bool have_input, bool have_output ): Application( name ), m_input( have_input ), m_output( have_output )
{
	if ( have_input ) {
		parameters["in"] = std::string();
		parameters["in"].setDescription( "input file or dataset" );

		parameters["rf"] = std::string();
		parameters["rf"].needed() = false;
		parameters["rf"].hidden() = true;
		
		parameters["rf"].setDescription( "Override automatic detection of file suffix for reading with given value" );
		parameters["rdialect"] = std::string();
		parameters["rdialect"].needed() = false;
		parameters["rdialect"].setDescription(
			"choose dialect for reading. The available dialects depend on the capabilities of IO plugins" );
	}

	if ( have_output ) {
		parameters["out"] = std::string();
		parameters["out"].setDescription( "output file" );

		parameters["wf"] = std::string();
		parameters["wf"].needed() = false;
		parameters["wf"].setDescription( "Override automatic detection of file suffix for writing with given value" );
		parameters["wf"].hidden() = true;
		
		parameters["wdialect"] = std::string();
		parameters["wdialect"].needed() = false;
		parameters["wdialect"].setDescription( "choose dialect for writing. The available dialects depend on the capabilities of IO plugins" );
		std::map<unsigned short, std::string> types = util::getTypeMap( false, true );

		// remove some types which are useless as representation
		// "(unsigned short)" is needed because otherwise erase would take the reference of a static constant which is only there during compile time
		types.erase( ( unsigned short )data::TypePtr<util::Selection>::staticID );
		types.erase( ( unsigned short )data::TypePtr<std::string>::staticID );
		types.erase( ( unsigned short )data::TypePtr<boost::posix_time::ptime>::staticID );
		types.erase( ( unsigned short )data::TypePtr<boost::gregorian::date>::staticID );
		types.erase( ( unsigned short )data::TypePtr<util::ilist>::staticID );
		types.erase( ( unsigned short )data::TypePtr<util::dlist>::staticID );
		types.erase( ( unsigned short )data::TypePtr<util::slist>::staticID );

		for( std::map<unsigned short, std::string>::iterator i = types.begin(); i != types.end(); i++ ) {
			i->second.resize( i->second.find_last_not_of( '*' ) + 1 );
		}

		parameters["repn"] = util::Selection( types );
		parameters["repn"].needed() = false;
		parameters["repn"].setDescription(
			"Representation in which the data shall be written." );
	}

	if( have_input || have_output){
		parameters["np"] = false;
		parameters["np"].needed() = false;
		parameters["np"].setDescription( "suppress progress bar" );
		parameters["np"].hidden() = true;
	}
}

IOApplication::~IOApplication()
{
	data::IOFactory::setProgressFeedback( 0 );
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
void IOApplication::printHelp(bool withHidden) const
{
    util::Application::printHelp(withHidden);
	if(withHidden){
		std::cout << std::endl << "Available IO Plugins:" << std::endl;
		data::IOFactory::FileFormatList plugins=data::IOFactory::getFormats();
		BOOST_FOREACH(data::IOFactory::FileFormatList::const_reference pi,plugins){
			
			std::cout << std::endl << "\t" << pi->name() << std::endl << "\t=======================================" << std::endl;

			const std::list<util::istring> suff=pi->getSuffixes();
			const std::list<std::string> dialects= util::string2list<std::string>(pi->dialects(""));
			
			std::cout << "\tsupported suffixes: " << util::list2string(suff.begin(),suff.end(),"\", \"","\"","\"")  << std::endl;

			if(!dialects.empty())
				std::cout << "\tsupported dialects: " << util::list2string(dialects.begin(),dialects.end(),"\", \"","\"","\"")  << std::endl;
			
		}
	}
}

bool IOApplication::autoload( bool exitOnError )
{
	std::string input = parameters["in"];
	std::string rf = parameters["rf"];
	std::string dl = parameters["rdialect"];
	bool no_progress=parameters["np"];

	LOG( Runtime, info )
			<< "loading " << util::MSubject( input )
			<< ( rf.empty() ? "" : std::string( " using the format: " ) + rf )
			<< ( ( !rf.empty() && !dl.empty() ) ? " and" : "" )
			<< ( dl.empty() ? "" : std::string( " using the dialect: " ) + dl );

	if(!no_progress){
		data::IOFactory::setProgressFeedback( &feedback );
	}

	images = data::IOFactory::load( input, rf, dl );

	if ( images.empty() ) {
		if ( exitOnError )
			exit( 1 );
		else
			return false;
	} else {
		for( ImageList::const_iterator a = images.begin(); a != images.end(); a++ ) {
			for( ImageList::const_iterator b = a; ( ++b ) != images.end(); ) {
				const util::PropMap &aref = **a, bref = **b;
				LOG_IF( aref.getDifference( bref ).empty(), Runtime, warning ) << "The metadata of the images from "
						<< aref.getProperty<std::string>( "source" ) << ":" << std::distance<ImageList::const_iterator>( images.begin(), a )
						<< " and " << bref.getProperty<std::string>( "source" ) << ":" << std::distance<ImageList::const_iterator>( images.begin(), b )
						<< " are equal. Maybe they are duplicates.";
			}
		}
	}

	return true;
}

bool IOApplication::autowrite( const Image& out_image, bool exitOnError )
{
	ImageList list;
	list.push_back(boost::shared_ptr<Image>(new Image(out_image)));
	autowrite(list,exitOnError);
}

bool IOApplication::autowrite( const ImageList& out_images, bool exitOnError )
{
	const util::Selection repn = parameters["repn"];
	const std::string output = parameters["out"];
	const std::string wf = parameters["wf"];
	const std::string dl = parameters["wdialect"];
	LOG( Runtime, info )
			<< "Writing " << out_images.size() << " images"
			<< ( repn ? std::string( " as " ) + ( std::string )repn : "" )
			<< " to " << util::MSubject( output )
			<< ( wf.empty() ? "" : std::string( " using the format: " ) + wf )
			<< ( ( !wf.empty() && !dl.empty() ) ? " and" : "" )
			<< ( dl.empty() ? "" : std::string( " using the dialect: " ) + dl );

	if( repn != 0 ) {
		BOOST_FOREACH( ImageList::const_reference ref, out_images ) {
			ref->makeOfTypeId( repn );
		}
	}

	data::IOFactory::setProgressFeedback( &feedback );

	if ( ! IOFactory::write( out_images, output, wf, dl ) ) {
		if ( exitOnError )
			exit( 1 );

		return false;
	} else
		return true;
}

}
}

