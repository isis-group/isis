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

#include <DataStorage/io_factory.hpp>
#include <CoreUtils/log.hpp>

#define BOOST_TEST_MODULE "imageIOVistaTest"
#include <boost/test/included/unit_test.hpp>
#include <string>
#include <iostream>

namespace isis {

namespace test {

BOOST_AUTO_TEST_SUITE(imageIOVista_BaseTests)

BOOST_AUTO_TEST_CASE(loadsaveTest)
{
	data::ImageList images;
	data::enable_log<util::DefaultMsgPrint>(error);
//	load the defautl NULL file
	std::string tmpfile = ( ( std::string )tmpnam( NULL ) ) + ".null";
	std::ostream(tmpfile);

//	load images from temp file
	images = data::IOFactory::get().load(tmpfile, "");

//	TODO write images to a vista file.

}


}

}
