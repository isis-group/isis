/*
 * imageIONiiTest.cpp
 *
 *  Created on: Apr 12, 2010
 *      Author: Thomas Proeger
 */

#include "DataStorage/image.hpp"
#include "DataStorage/io_factory.hpp"
#include "CoreUtils/log.hpp"

using namespace isis;

#define BOOST_TEST_MODULE "imageIONiiTest"
#include <boost/test/included/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <string>

namespace isis
{
namespace test
{

BOOST_AUTO_TEST_SUITE ( imageIONii_NullTests )

BOOST_AUTO_TEST_CASE( loadsaveImage )
{
	data::ImageList images;
	data::enable_log<util::DefaultMsgPrint>( error );
//  The factory assumes that there is valid file before
//  calling the appropriate plugin.
//  We will use the Null plugin to get some image data
	std::string tmpfile = ( ( std::string )tmpnam( NULL ) ) + ".null";
	std::string niifile = ( ( std::string )tmpnam( NULL ) ) + ".nii";
	std::ofstream file( tmpfile.c_str() );
	//  load images from file
	images = data::IOFactory::load( tmpfile, "" );
	std::cout << "Writing Image to " << niifile << std::endl;
//  write images to file(s)
	data::IOFactory::write( images, niifile, "" );
}

BOOST_AUTO_TEST_SUITE_END()

}
}