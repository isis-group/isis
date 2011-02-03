/****************************************************************
 *
 * Copyright (C) 2010 Max Planck Institute for Human Cognitive
 * and Brain Sciences, Leipzig.
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
 * Author: Thomas Pröger, proeger@cbs.mpg.de, 2010
 *
 *****************************************************************/

#include "DataStorage/io_factory.hpp"
#include "CoreUtils/log.hpp"

//#define BOOST_TEST_MODULE "imageIOVistaTest"
//#define BOOST_TEST_DYN_LINK
//#include <boost/test/unit_test.hpp>
#include <string>
#include <iostream>

using namespace isis;

//BOOST_AUTO_TEST_SUITE( imageIOVista_BaseTests )

//BOOST_AUTO_TEST_CASE( loadsaveTest )

int main( int argc, char **argv )
{
	data::enable_log<util::DefaultMsgPrint>( warning );
	image_io::enable_log<util::DefaultMsgPrint>( warning );

	//  load the defautl NULL file
	// std::string tmpfile = ( ( std::string )tmpnam( NULL ) ) + ".null";
	// std::string vtmpfile = ( ( std::string )tmpnam( NULL ) ) + ".v";
	// fopen( tmpfile.c_str(), "w" );
	//  load images from temp file
	std::list<data::Image> images = data::IOFactory::get().load( "/scr/kastanie1/DATA/isis/vista/1st_PD.v", "" );
	// the null-loader shall generate 5 50x50x50x10 images
	//BOOST_CHECK( images.size() == 5 );
	//  print attributes
	//  ((data::ImageList::const_reference) images.front())->print(std::cout,true);
	// get first image and write it to disk
	//  data::ImageList::const_reference first = images.front();
	data::IOFactory::write( images, "", "/tmp/data1.v", "" );
	return 0;
	//BOOST_AUTO_TEST_SUITE_END()
}
