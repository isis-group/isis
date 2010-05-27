/*
* imageIONullTest.cpp
*
* Author: Thomas Proeger
* Description: TestSuite to check the read and write ability of the imageIONull plugin
*/

#include "DataStorage/image.hpp"
#include "DataStorage/io_factory.hpp"
#include "CoreUtils/log.hpp"
#include "CoreUtils/tmpfile.h"

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
	data::ImageList images;
	data::enable_log<util::DefaultMsgPrint>( error );
	images = data::IOFactory::load( "nix.null" );
	// the null-loader shall generate 5 50x50x50x10 images
	BOOST_CHECK( images.size() == 5 );
	//  check each image separately
	u_int8_t cnt = 0;
	BOOST_FOREACH( data::ImageList::value_type & ref, images ) {
		//  check geometry
		BOOST_CHECK_EQUAL( ref->sizeToVector(), util::fvector4( 50, 50, 50, 10 ) );
		BOOST_CHECK_EQUAL( ref->voxel<u_int8_t>( 0, 0 ), cnt * 40 );

		for ( int i = 0; i < 10; i++ )
			for ( int j = 10; j < 40; j++ )
				BOOST_CHECK_EQUAL( ref->voxel<u_int8_t>( j, j, j, i ), 255 - i * 20 );

		cnt++;
	}
}

BOOST_AUTO_TEST_SUITE_END ()

}
}


