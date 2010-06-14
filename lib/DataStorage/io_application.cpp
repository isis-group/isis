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
		parameters["rf"].setDescription( "Override automatic detection of file suffix for reading with given value" );
	}

	if ( have_output ) {
		parameters["out"] = std::string();
		parameters["out"].setDescription( "output file" );
		parameters["wf"] = std::string();
		parameters["wf"].needed() = false;
		parameters["wf"].setDescription( "Override automatic detection of file suffix for writing with given value" );
	}
}

IOApplication::~IOApplication() {}

bool IOApplication::init( int argc, char **argv, bool exitOnError )
{
	if ( ! isis::util::Application::init( argc, argv, exitOnError ) )
		return false;

	if ( m_input ) {
		std::string input = parameters["in"];
		std::string rf = parameters["rf"];
		images = data::IOFactory::load( input, rf );

		if ( images.empty() ) {
			if ( exitOnError )
				exit( 1 );

			return false;
		}
	}

	return true;
}

}
}

