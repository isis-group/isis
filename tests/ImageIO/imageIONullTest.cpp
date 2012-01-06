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
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
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


