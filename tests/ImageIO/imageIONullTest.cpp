/*
* imageIONullTest.cpp
* 
* Author: Thomas Proeger
* Description: TestSuite to check the read and write ability of the imageIONull plugin
*/

#include "DataStorage/image.hpp"
#include "DataStorage/io_factory.hpp"
#include "CoreUtils/log.hpp"

using namespace isis;
	
#define BOOST_TEST_MODULE "imageIONullTest"
#include <boost/test/included/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <string>

BOOST_AUTO_TEST_SUITE (imageIONull_BaseTests)

BOOST_AUTO_TEST_CASE (loadsaveImages)
{
	data::ImageList images;
	data::enable_log<util::DefaultMsgPrint>(error);
	
// The factory assumes that there is a valid file before
// calling the appropriate plugin
	std::string tmpfile = ((std::string)tmpnam(NULL)) + ".null";
	std::ofstream file(tmpfile.c_str());

// 	load images from file
	images = data::IOFactory::load(tmpfile, "");

	// the null-loader shall generate 5 50x50x50x10 images
	BOOST_CHECK(images.size() == 5);

// 	check each image separately
	short cnt=0;
	BOOST_FOREACH(data::ImageList::value_type &ref,images){
//  check geometry
		BOOST_CHECK_EQUAL(ref->sizeToVector(), util::fvector4(50,50,50,10));
		BOOST_CHECK_EQUAL(ref->voxel<u_int8_t>(0,0), cnt);
		for(int i=0;i<10;i++)
			for(int j=10;j<40;j++)
				BOOST_CHECK_EQUAL(ref->voxel<u_int8_t>(j,j,j,i), 255-i*20);
		cnt++;
	}

// remove temporary file
	boost::filesystem::path p(tmpfile);
	boost::filesystem::remove(p);

}

BOOST_AUTO_TEST_SUITE_END ()



