/*
* imageTest.cpp
*
*  Created on: Oct 1, 2009
*      Author: proeger
*/

#define BOOST_TEST_MODULE ImageLoadTest
#include <boost/test/included/unit_test.hpp>
#include <boost/foreach.hpp>

#include "DataStorage/image.hpp"
#include "DataStorage/io_factory.hpp"
#include "CoreUtils/log.hpp"

namespace isis{namespace test{

BOOST_AUTO_TEST_CASE (imageLoadtest)
{
	ENABLE_LOG(util::CoreLog,util::DefaultMsgPrint,util::warning);
	ENABLE_LOG(util::CoreDebug,util::DefaultMsgPrint,util::warning);
	ENABLE_LOG(data::DataLog,util::DefaultMsgPrint,util::warning);
	ENABLE_LOG(data::DataDebug,util::DefaultMsgPrint,util::warning);
	

	// just to make sure the wanted file exists
	FILE* f = fopen("test.null", "w");
	fclose(f);
	// Das geht erst wieder wenn es ein SUPERTOLLES gz-plugin gibt!!
	//data::ImageList images=data::IOFactory::load("test.null.gz", "");
	data::ImageList images=data::IOFactory::load("test.null", "");

	// the null-loader shall generate 5 3x3x3x10 images 
	BOOST_CHECK(images.size() == 5);
	
	short cnt=0;
	BOOST_FOREACH(data::ImageList::value_type &ref,images){
		BOOST_CHECK(ref->size() == util::fvector4(3,3,3,10));
		for(int i=0;i<10;i++)
			BOOST_CHECK(ref->voxel<short>(0,0,0,i) == i+cnt);
		cnt++;
	}
	// Das geht erst wieder wenn es ein SUPERTOLLES gz-plugin gibt!!
//	BOOST_CHECK(data::IOFactory::write(images,"test.null.gz",""));
	BOOST_CHECK(data::IOFactory::write(images,"test.null",""));
}
}}
