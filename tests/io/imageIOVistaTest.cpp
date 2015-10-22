#include <data/image.hpp>
#include <data/io_factory.hpp>
#include <util/log.hpp>
#include <util/tmpfile.hpp>

#define BOOST_TEST_MODULE "imageIONiiTest"
#include <boost/test/unit_test.hpp>
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#include <iostream>
#include <string>

namespace isis
{
namespace test
{

BOOST_AUTO_TEST_SUITE ( imageIOVista_NullTests )

BOOST_AUTO_TEST_CASE( loadsaveNullImage )
{

	std::list<data::Image> images = data::IOFactory::load( "nix.null" );
	BOOST_REQUIRE( images.size() >= 1 );
	for( data::Image & null :  images ) {
		util::TmpFile vfile( "", ".v" );
		BOOST_REQUIRE( data::IOFactory::write( null, vfile.native() ) );
	}
}
BOOST_AUTO_TEST_SUITE_END()


}
}
