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

#include "io_application.hpp"
#include "io_factory.hpp"

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
		parameters["rf"].setDescription( "Override automatic detection of file suffix for reading with given value." );
		parameters["rdialect"] = std::string();
		parameters["rdialect"].needed() = false;
		parameters["rdialect"].setDescription(
			"choose dialect for reading. The available dialects depend on the capabilities of IO plugins." );
	}

	if ( have_output ) {
		parameters["out"] = std::string();
		parameters["out"].setDescription( "output file" );
		parameters["wf"] = std::string();
		parameters["wf"].needed() = false;
		parameters["wf"].setDescription( "Override automatic detection of file suffix for writing with given value." );
		parameters["wdialect"] = std::string();
		parameters["wdialect"].needed() = false;
		parameters["wdialect"].setDescription(
			"choose dialect for writing. The available dialects depend on the capabilities of IO plugins." );
		parameters["repn"] = util::Selection(util::getTypeMap());
		parameters["repn"].needed() = false;
		parameters["repn"].setDescription(
			"representation in which the data shall be written (not implemented yet)." );
	}

}

IOApplication::~IOApplication() {}

bool IOApplication::init( int argc, char **argv, bool exitOnError )
{
	if ( ! util::Application::init( argc, argv, exitOnError ) )
		return false;

	if ( m_input ) {
	}

	return true;
}
size_t IOApplication::autoload(bool exitOnError)
{
	std::string input = parameters["in"];
	std::string rf = parameters["rf"];
	std::string dl = parameters["rdialect"];
	LOG(DataLog, info) << "loading " << parameters["in"].toString(false) << " with rf: " << parameters["rf"].toString(false) << " and rdialect: " << parameters["rdialect"].toString(false);
	images = data::IOFactory::load( input, rf, dl );

	if ( images.empty() ) {
		if ( exitOnError )
			exit( 1 );

		return false;
	} else {
		for( ImageList::const_iterator a = images.begin(); a != images.end(); a++ ) {
			for( ImageList::const_iterator b = a; ( ++b ) != images.end(); ) {
				const util::PropMap &aref = **a, bref = **b;
				LOG_IF( aref.getDifference( bref ).empty(), Runtime, warning ) << "The metadata of the images from "
						<< aref.propertyValue( "source" ).toString( false ) << ":" << std::distance<ImageList::const_iterator>( images.begin(), a )
						<< " and " << bref.propertyValue( "source" ).toString( false ) << ":" << std::distance<ImageList::const_iterator>( images.begin(), b )
						<< " are equal. Maybe they are duplicates.";
			}
		}
	}
}
size_t IOApplication::autowrite(ImageList out_images,bool exitOnError)
{
	util::Selection repn=parameters["repn"];

	LOG_IF(out_images.empty(),Runtime,warning) << "There are not images for writing.";

	if (! IOFactory::write(out_images,parameters["out"],parameters["rf"],parameters["wdialect"]) ) {
		if ( exitOnError )
			exit( 1 );
		return false;
	} else
		return true;
}

}
}

