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
	image_io::enable_log<util::DefaultMsgPrint>( info );
	//  We will use the Null plugin to get some image data
	std::string nullfile = ( ( std::string )tmpnam( NULL ) ) + ".null";
	std::string niifile = ( ( std::string )tmpnam( NULL ) ) + ".nii";
	// the null-loader shall generate 5 3x3x3x10 images
	images = data::IOFactory::load( nullfile, "" );

	//  write images to file(s)
	if ( data::IOFactory::write( images, "/tmp/delme.nii", "" ) )
		std::cout << "Wrote Image to " << niifile << std::endl;

	data::IOFactory::load( "/tmp/delme.nii", "" );
}

BOOST_AUTO_TEST_SUITE_END()

}
}