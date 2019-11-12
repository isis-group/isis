/*
* imageIONullTest.cpp
*
* Author: Thomas Proeger
* Description: TestSuite to check the read and write ability of the imageIONull plugin
*/

#include <isis/core/image.hpp>
#include <isis/core/io_factory.hpp>
#include <isis/core/log.hpp>
#include <isis/core/tmpfile.hpp>

#define BOOST_TEST_MODULE "imageIONullTest"
#include <boost/test/unit_test.hpp>
#define BOOST_FILESYSTEM_VERSION 3
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
	data::enableLog<util::DefaultMsgPrint>( warning );
	std::list<data::Image> images = data::IOFactory::load( "nix.null" );

	BOOST_CHECK_EQUAL( images.size(), 2 );
	BOOST_CHECK( data::IOFactory::write( images, "nix.null" ) );
}

BOOST_AUTO_TEST_SUITE_END ()

}
}


