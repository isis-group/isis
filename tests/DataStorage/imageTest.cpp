/*
 * imageTest.cpp
 *
 *  Created on: Oct 1, 2009
 *      Author: proeger
 */

#define BOOST_TEST_MODULE ImageTest
#include <boost/test/included/unit_test.hpp>
#include <DataStorage/image.hpp>

/* create an image */
BOOST_AUTO_TEST_CASE (image_init_test)
{
	isis::data::MemChunk<float> ch(1,1,4,4);
	isis::data::Image img;

	ch.setProperty("position",isis::util::fvector4(1,1,1,1));
	
	BOOST_CHECK(img.insertChunk(ch));
	BOOST_CHECK(not img.insertChunk(ch)); //inserting the same chunk twice should fail

	ch = isis::data::MemChunk<float>(1,1,4,4);
	ch.setProperty("position",isis::util::fvector4(1,2,1,1));
	BOOST_CHECK(img.insertChunk(ch));
	
//	TODO create an image out of an ChunkList

}

BOOST_AUTO_TEST_CASE (image_voxel_test)
{

//	TODO test the const and non-const version of voxel,
//
//	get a voxel from inside and outside the image

}

BOOST_AUTO_TEST_CASE(image_getChunk_test)
{

//	TODO test the getChunk function,
//
//	retrieve a voxel from inside and outside the chunk

}

BOOST_AUTO_TEST_CASE(image_insertChunk_test)
{

//	TODO test the insertChunk function
//
//	insert a valid and non-valid list of chunks

}
