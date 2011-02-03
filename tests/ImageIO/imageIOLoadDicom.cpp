/*
* imageTest.cpp
*
*  Created on: Oct 1, 2009
*      Author: proeger
*/

#define BOOST_TEST_MODULE ImageLoadTest
#include <boost/test/included/unit_test.hpp>
#include <boost/foreach.hpp>

#include <DataStorage/image.hpp>
#include <DataStorage/io_factory.hpp>
#include <CoreUtils/log.hpp>

namespace isis
{
namespace test
{

BOOST_AUTO_TEST_CASE ( imageDicomtest )
{
	ENABLE_LOG( CoreLog, util::DefaultMsgPrint, warning );
	ENABLE_LOG( CoreDebug, util::DefaultMsgPrint, warning );
	ENABLE_LOG( DataLog, util::DefaultMsgPrint, warning );
	ENABLE_LOG( DataDebug, util::DefaultMsgPrint, warning );
	ENABLE_LOG( ImageIoLog, util::DefaultMsgPrint, warning );
	ENABLE_LOG( ImageIoDebug, util::DefaultMsgPrint, warning );
	std::list<data::Image> images = data::IOFactory::load( "/SCR/isis_build/testDicom.ima", "" );
	BOOST_CHECK( images.size() == 1 );
	/*      BOOST_FOREACH(data::ImageList::value_type &ref,images){
	            BOOST_CHECK(ref->size() == util::fvector4(3,3,3,10));
	            for(int i=0;i<10;i++)
	                BOOST_CHECK(ref->voxel<short>(0,0,0,i) == i+cnt);
	            cnt++;
	        }
	        BOOST_CHECK(data::IOFactory::write(images,"test.null.gz",""));*/
}
}
}
