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
#include "ImageIO/common.hpp"

namespace isis{namespace test{
	
	BOOST_AUTO_TEST_CASE (imageDicomtest)
	{
		ENABLE_LOG(util::CoreLog,util::DefaultMsgPrint,util::warning);
		ENABLE_LOG(util::CoreDebug,util::DefaultMsgPrint,util::warning);
		ENABLE_LOG(data::DataLog,util::DefaultMsgPrint,util::warning);
		ENABLE_LOG(data::DataDebug,util::DefaultMsgPrint,util::warning);
		ENABLE_LOG(image_io::ImageIoLog,util::DefaultMsgPrint,util::verbose_info);
		ENABLE_LOG(image_io::ImageIoDebug,util::DefaultMsgPrint,util::verbose_info);
		
		data::ImageList	images=data::IOFactory::load("/SCR/isis_build/testDicom.ima","");
		
		// the null-loader shall generate 5 3x3x3x10 images
		BOOST_CHECK(images.size() == 1);
		
		short cnt=0;
/*		BOOST_FOREACH(data::ImageList::value_type &ref,images){
			BOOST_CHECK(ref->size() == util::fvector4(3,3,3,10));
			for(int i=0;i<10;i++)
				BOOST_CHECK(ref->voxel<short>(0,0,0,i) == i+cnt);
			cnt++;
		}
		BOOST_CHECK(data::IOFactory::write(images,"test.null.gz",""));*/
	}
}}