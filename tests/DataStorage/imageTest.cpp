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

namespace test{

/* create an image */
BOOST_AUTO_TEST_CASE (image_init_test)
{
	ENABLE_LOG(isis::util::CoreLog,isis::util::DefaultMsgPrint,isis::util::warning);
	ENABLE_LOG(isis::util::CoreDebug,isis::util::DefaultMsgPrint,isis::util::warning);
	ENABLE_LOG(isis::data::DataLog,isis::util::DefaultMsgPrint,isis::util::info);
	ENABLE_LOG(isis::data::DataDebug,isis::util::DefaultMsgPrint,isis::util::info);
	
	isis::data::MemChunk<float> ch(4,4);
	isis::data::Image img;

	// inserting insufficient Chunk should fail
	BOOST_CHECK(!img.insertChunk(ch)); 

	// but inserting a proper Chunk should work
	ch.setProperty("indexOrigin",isis::util::fvector4(0,0,2,0));
	BOOST_CHECK(img.insertChunk(ch));

	//inserting the same chunk twice should fail
	BOOST_CHECK(not img.insertChunk(ch)); 

	// but inserting another Chunk should work
	ch = isis::data::MemChunk<float>(4,4);
	ch.setProperty("indexOrigin",isis::util::fvector4(0,0,0,0));
	BOOST_CHECK(img.insertChunk(ch));

	// Chunks should be inserted based on their position (lowest first)
	ch = isis::data::MemChunk<float>(4,4);
	ch.setProperty("indexOrigin",isis::util::fvector4(0,0,1,0));
	BOOST_CHECK(img.insertChunk(ch));

	//threat image as a list of sorted chunks
	//Image-Chunk-List should be copyable into other lists (and its order should be correct)
	//@todo equality test
	std::list<isis::data::Chunk> list(img.chunksBegin(),img.chunksEnd());
	unsigned short i=0;
	BOOST_FOREACH(const isis::data::Chunk &ref,list){
		BOOST_CHECK(ref.getPropertyValue("indexOrigin") == isis::util::fvector4(0,0,i++,0));
	}

	// Check for insertion in two dimensions
	ch = isis::data::MemChunk<float>(4,4);
	ch.setProperty("indexOrigin",isis::util::fvector4(0,0,0,1));
	BOOST_CHECK(img.insertChunk(ch));
	isis::data::Image::ChunkIterator it=img.chunksEnd();
	//as all other chunks where timestep 0 this must be at the end
	BOOST_CHECK((--it)->getPropertyValue("indexOrigin")==isis::util::fvector4(0,0,0,1));

//	TODO create an image out of an ChunkList

}

BOOST_AUTO_TEST_CASE (image_chunk_test)
{
	isis::data::MemChunk<float> ch11(3,3);
	isis::data::MemChunk<float> ch12(3,3);
	isis::data::MemChunk<float> ch13(3,3);
	isis::data::MemChunk<float> ch21(3,3);
	isis::data::MemChunk<float> ch22(3,3);
	isis::data::MemChunk<float> ch23(3,3);
	isis::data::Image img;

	ch11.setProperty("indexOrigin",isis::util::fvector4(0,0,0,0));
	ch12.setProperty("indexOrigin",isis::util::fvector4(0,0,1,0));
	ch13.setProperty("indexOrigin",isis::util::fvector4(0,0,2,0));

	ch21.setProperty("indexOrigin",isis::util::fvector4(0,0,0,1));
	ch22.setProperty("indexOrigin",isis::util::fvector4(0,0,1,1));
	ch23.setProperty("indexOrigin",isis::util::fvector4(0,0,2,1));
	
	ch11.voxel<float>(0,0)=42;
	ch12.voxel<float>(1,1)=42;
	ch13.voxel<float>(2,2)=42;

	ch21.voxel<float>(0,0)=42;
	ch22.voxel<float>(1,1)=42;
	ch23.voxel<float>(2,2)=42;
	
	BOOST_CHECK(img.insertChunk(ch11));
	BOOST_CHECK(img.insertChunk(ch12));
	BOOST_CHECK(img.insertChunk(ch13));

	BOOST_CHECK(img.insertChunk(ch21));
	BOOST_CHECK(img.insertChunk(ch22));
	BOOST_CHECK(img.insertChunk(ch23));
	
	const isis::data::Chunk &ref11=img.getChunk(0,0,0);
	const isis::data::Chunk &ref12=img.getChunk(1,1,1);
	const isis::data::Chunk &ref13=img.getChunk(2,2,2);
	//                                         r,p,s

	const isis::data::Chunk &ref22=img.getChunk(1,1,1,1);

	BOOST_CHECK(ref11.getPropertyValue("indexOrigin")==isis::util::fvector4(0,0,0,0));
	BOOST_CHECK(ref12.getPropertyValue("indexOrigin")==isis::util::fvector4(0,0,1,0));
	BOOST_CHECK(ref13.getPropertyValue("indexOrigin")==isis::util::fvector4(0,0,2,0));

	BOOST_CHECK(ref22.getPropertyValue("indexOrigin")==isis::util::fvector4(0,0,1,1));
	BOOST_CHECK(not (ref22.getPropertyValue("indexOrigin")==isis::util::fvector4(0,0,1,0)));
	
	BOOST_CHECK(ref11.voxel<float>(0,0)==42);
	BOOST_CHECK(ref12.voxel<float>(1,1)==42);
	BOOST_CHECK(ref13.voxel<float>(2,2)==42);
	BOOST_CHECK(ref22.voxel<float>(1,1)==42);
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
}