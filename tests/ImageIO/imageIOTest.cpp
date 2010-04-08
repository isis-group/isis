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
/*	util::enable_log<util::DefaultMsgPrint>(info);
	data::enable_log<util::DefaultMsgPrint>(info);*/
	
	// just to make sure the wanted file exists
	FILE* f = fopen("test.null", "w");
	fclose(f);
	data::ImageList images=data::IOFactory::load("test.null", "");

	// the null-loader shall generate 5 50x50x50x10 images
	BOOST_CHECK(images.size() == 5);
	
	short cnt=0;
	BOOST_FOREACH(data::ImageList::value_type &ref,images){
		BOOST_CHECK_EQUAL(ref->sizeToVector(), util::fvector4(50,50,50,10));
		BOOST_CHECK_EQUAL(ref->voxel<u_int8_t>(0,0), cnt);
		for(int i=0;i<10;i++)
			for(int j=10;j<40;j++)
				BOOST_CHECK_EQUAL(ref->voxel<u_int8_t>(j,j,j,i), 255-i*20);
		cnt++;
	}
	BOOST_CHECK(data::IOFactory::write(images,"test.null",""));
}
}}
