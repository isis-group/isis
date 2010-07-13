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
	}

	if ( have_output ) {
		parameters["out"] = std::string();
		parameters["out"].setDescription( "output file" );
		parameters["wf"] = std::string();
		parameters["wf"].needed() = false;
		parameters["wf"].setDescription( "Override automatic detection of file suffix for writing with given value." );
	}

	parameters["dialect"] = std::string();
	parameters["dialect"].needed() = false;
	parameters["dialect"].setDescription(
		"choose dialect of data set. The available dialects depend on the capabilities of IO plugins." );
}

IOApplication::~IOApplication() {}

bool IOApplication::init( int argc, char **argv, bool exitOnError )
{
	if ( ! util::Application::init( argc, argv, exitOnError ) )
		return false;

	if ( m_input ) {
		std::string input = parameters["in"];
		std::string rf = parameters["rf"];
		std::string dl = parameters["dialect"];
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

	return true;
}

}
}

