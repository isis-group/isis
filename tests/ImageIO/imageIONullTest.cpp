/*
* imageIONullTest.cpp
*
* Author: Thomas Proeger
* Description: TestSuite to check the read and write ability of the imageIONull plugin
*/

#include <DataStorage/image.hpp>
#include <DataStorage/io_factory.hpp>
#include <CoreUtils/log.hpp>
#include <CoreUtils/tmpfile.hpp>

#define BOOST_TEST_MODULE "imageIONullTest"
#include <boost/test/included/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <string>

namespace isis
{
namespace test
{

BOOST_AUTO_TEST_SUITE ( imageIONull_BaseTests )

BOOST_AUTO_TEST_CASE ( loadsaveImages )
{
	data::enable_log<util::DefaultMsgPrint>( warning );
	std::list<data::Image> images = data::IOFactory::load( "nix.null" );
	// the null-loader shall generate 5 50x50x50x10 images
	BOOST_CHECK( images.size() == 5 );
	//  check each image separately
	uint8_t cnt = 0;
	BOOST_FOREACH( data::Image & ref, images ) {
		//  check geometry
		BOOST_CHECK_EQUAL( ref.getSizeAsVector(), util::fvector4( 50, 50, 50, 10 ) );
		BOOST_CHECK_EQUAL( ref.voxel<uint8_t>( 0, 0 ), cnt * 40 );

		for ( int i = 0; i < 10; i++ )
			for ( int j = 10; j < 40; j++ )
				BOOST_CHECK_EQUAL( ref.voxel<uint8_t>( j, j, j, i ), 255 - i * 20 );

		cnt++;
	}
}

BOOST_AUTO_TEST_SUITE_END ()

}
}


