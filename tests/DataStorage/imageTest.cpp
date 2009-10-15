/*
 * imageTest.cpp
 *
 *  Created on: Oct 1, 2009
 *      Author: proeger
 */

#define BOOST_TEST_MODULE ImageTest
#include <boost/test/included/unit_test.hpp>
#include <boost/foreach.hpp>
#include "DataStorage/image.hpp"


/* create an image */
BOOST_AUTO_TEST_CASE (image_init_test)
{
	ENABLE_LOG(isis::util::CoreLog,isis::util::DefaultMsgPrint,isis::util::warning);
	ENABLE_LOG(isis::util::CoreDebug,isis::util::DefaultMsgPrint,isis::util::warning);
	
	isis::data::MemChunk<float> ch(1,1,4,4);
	isis::data::Image img;

	// inserting insufficient Chunk should fail
	BOOST_CHECK(!img.insertChunk(ch)); 

	// but inserting a proper Chunk should work
	ch.setProperty("indexOrigin",isis::util::fvector4(0,2,0,0));
	BOOST_CHECK(img.insertChunk(ch));

	//inserting the same chunk twice should fail
	BOOST_CHECK(not img.insertChunk(ch)); 

	// but inserting another Chunk should work
	ch = isis::data::MemChunk<float>(1,1,4,4);
	ch.setProperty("indexOrigin",isis::util::fvector4(0,0,0,0));
	BOOST_CHECK(img.insertChunk(ch));

	// Chunks should be inserted based on their position (lowest first)
	ch = isis::data::MemChunk<float>(1,1,4,4);
	ch.setProperty("indexOrigin",isis::util::fvector4(0,1,0,0));
	BOOST_CHECK(img.insertChunk(ch));

/*	unsigned short i=0;
	BOOST_FOREACH(const isis::data::Chunk &ref,img){
		BOOST_CHECK(ref.getPropertyValue("indexOrigin") == isis::util::fvector4(0,i++,0,0));
	}*/
	
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
