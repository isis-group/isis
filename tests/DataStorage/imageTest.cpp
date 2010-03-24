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

namespace isis{namespace test{

/* create an image */
BOOST_AUTO_TEST_CASE (image_init_test)
{
	ENABLE_LOG(CoreLog,util::DefaultMsgPrint,error);
	ENABLE_LOG(CoreDebug,util::DefaultMsgPrint,error);
	ENABLE_LOG(DataLog,util::DefaultMsgPrint,error);
	ENABLE_LOG(DataDebug,util::DefaultMsgPrint,error);
// 	util::DefaultMsgPrint::stopBelow(warning);
	
	data::MemChunk<float> ch(4,4);
	data::Image img;

	// inserting insufficient Chunk should fail
	BOOST_CHECK(!img.insertChunk(ch));

	// but inserting a proper Chunk should work
	ch.setProperty("indexOrigin",util::fvector4(0,0,2,0));
	ch.setProperty<u_int32_t>("acquisitionNumber",2);
	BOOST_CHECK(img.insertChunk(ch));

	//inserting the same chunk twice should fail
	BOOST_CHECK(not img.insertChunk(ch)); 

	// but inserting another Chunk should work
	ch = data::MemChunk<float>(4,4);
	ch.setProperty("indexOrigin",util::fvector4(0,0,0,0));
	ch.setProperty<u_int32_t>("acquisitionNumber",0);
	BOOST_CHECK(img.insertChunk(ch));

	// Chunks should be inserted based on their position (lowest first)
	ch = data::MemChunk<float>(4,4);
	ch.setProperty("indexOrigin",util::fvector4(0,0,1,0));
	ch.setProperty<u_int32_t>("acquisitionNumber",1);
	BOOST_CHECK(img.insertChunk(ch));

	//threat image as a list of sorted chunks
	//Image-Chunk-List should be copyable into other lists (and its order should be correct)
	//Note: this ordering is not allways geometric correct
	//@todo equality test
	std::list<data::Chunk> list(img.chunksBegin(),img.chunksEnd());
	unsigned short i=0;
	BOOST_FOREACH(const data::Chunk &ref,list){
		BOOST_CHECK(ref.getPropertyValue("indexOrigin") == util::fvector4(0,0,i,0));
		BOOST_CHECK(ref.getPropertyValue("acquisitionNumber") == i++);
	}

	//Get a list of properties from the chunks in the image 
	//List of the properties shall be as if every chunk of the image was asked for the property
	i=0;
	std::list<util::PropertyValue> origins=img.getChunksProperties("indexOrigin");
	BOOST_FOREACH(const util::PropertyValue &ref,origins){
		BOOST_CHECK(ref == util::fvector4(0,0,i++,0));
	}
	
	// Check for insertion in two dimensions
	ch = data::MemChunk<float>(4,4);
	ch.setProperty("indexOrigin",util::fvector4(0,0,0,1));
	ch.setProperty<u_int32_t>("acquisitionNumber",4);
	BOOST_CHECK(img.insertChunk(ch));
	data::Image::ChunkIterator it=img.chunksEnd();
	//as all other chunks where timestep 0 this must be at the end
	BOOST_CHECK((--it)->getPropertyValue("indexOrigin")==util::fvector4(0,0,0,1));
	BOOST_CHECK((it)->getPropertyValue("acquisitionNumber")==int(4));

}

BOOST_AUTO_TEST_CASE (image_chunk_test)
{
	u_int32_t acNum=0;
	data::MemChunk<float> ch11(3,3);
	data::MemChunk<float> ch12(3,3);
	data::MemChunk<float> ch13(3,3);
	data::MemChunk<float> ch21(3,3);
	data::MemChunk<float> ch22(3,3);
	data::MemChunk<float> ch23(3,3);
	data::Image img;

	const util::fvector4 vSize(1,1,1,0);
	ch11.setProperty("indexOrigin",util::fvector4(0,0,0,0));
	ch12.setProperty("indexOrigin",util::fvector4(0,0,1,0));
	ch13.setProperty("indexOrigin",util::fvector4(0,0,2,0));
	ch21.setProperty("indexOrigin",util::fvector4(0,0,0,1));
	ch22.setProperty("indexOrigin",util::fvector4(0,0,1,1));
	ch23.setProperty("indexOrigin",util::fvector4(0,0,2,1));

	ch11.setProperty("acquisitionNumber",acNum++);
	ch12.setProperty("acquisitionNumber",acNum++);
	ch13.setProperty("acquisitionNumber",acNum++);
	ch21.setProperty("acquisitionNumber",acNum++);
	ch22.setProperty("acquisitionNumber",acNum++);
	ch23.setProperty("acquisitionNumber",acNum++);
	
	ch11.setProperty("voxelSize",vSize);
	ch12.setProperty("voxelSize",vSize);
	ch13.setProperty("voxelSize",vSize);
	ch21.setProperty("voxelSize",vSize);
	ch22.setProperty("voxelSize",vSize);
	ch23.setProperty("voxelSize",vSize);


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


	const data::Chunk &ref11=img.getChunk(0,0,0);
	const data::Chunk &ref12=img.getChunk(1,1,1);
	const data::Chunk &ref13=img.getChunk(2,2,2);
	//                                         r,p,s

	const data::Chunk &ref22=img.getChunk(1,1,1,1);
	const data::Chunk &ref21=img.getChunk(0,0,0,1);
	const data::Chunk &ref23=img.getChunk(2,2,2,1);

	BOOST_CHECK_EQUAL(ref11.getPropertyValue("indexOrigin"),util::fvector4(0,0,0,0));
	BOOST_CHECK_EQUAL(ref12.getPropertyValue("indexOrigin"),util::fvector4(0,0,1,0));
	BOOST_CHECK_EQUAL(ref13.getPropertyValue("indexOrigin"),util::fvector4(0,0,2,0));
	BOOST_CHECK_EQUAL(ref11.getPropertyValue("acquisitionNumber"),0);
	BOOST_CHECK_EQUAL(ref12.getPropertyValue("acquisitionNumber"),1);
	BOOST_CHECK_EQUAL(ref13.getPropertyValue("acquisitionNumber"),2);
	BOOST_CHECK_EQUAL(ref21.getPropertyValue("acquisitionNumber"),3);
	BOOST_CHECK_EQUAL(ref22.getPropertyValue("acquisitionNumber"),4);
	BOOST_CHECK_EQUAL(ref23.getPropertyValue("acquisitionNumber"),5);

	BOOST_CHECK_EQUAL(ref22.getPropertyValue("indexOrigin"),util::fvector4(0,0,1,1));
	BOOST_CHECK(not (ref22.getPropertyValue("indexOrigin")==util::fvector4(0,0,1,0)));

	BOOST_CHECK_EQUAL(ref11.voxel<float>(0,0),42);
	BOOST_CHECK_EQUAL(ref12.voxel<float>(1,1),42);
	BOOST_CHECK_EQUAL(ref13.voxel<float>(2,2),42);
	BOOST_CHECK_EQUAL(ref22.voxel<float>(1,1),42);
	BOOST_CHECK_EQUAL(ref23.voxel<float>(2,2),42);
	BOOST_CHECK_EQUAL(ref23.voxel<float>(2,2),42);
 	BOOST_CHECK_EQUAL(img.volume(),6*9);
	BOOST_CHECK_EQUAL(img.sizeToVector(),util::ivector4(3,3,3,2));

}

BOOST_AUTO_TEST_CASE (image_voxel_test)
{

//	//	TODO test the const and non-const version of voxel,
//	//
//	//	get a voxel from inside and outside the image
//	data::MemChunk<float> ch11(3,3);
//	data::MemChunk<float> ch12(3,3);
//	data::MemChunk<float> ch13(3,3);
//	data::Image img;
//
//	ch11.setProperty("indexOrigin",util::fvector4(0,0,0,0));
//	ch11.setProperty<u_int32_t>("acquisitionNumber",0);
//	ch12.setProperty("indexOrigin",util::fvector4(0,0,1,0));
//	ch12.setProperty<u_int32_t>("acquisitionNumber",1);
//	ch13.setProperty("indexOrigin",util::fvector4(0,0,2,0));
//	ch13.setProperty<u_int32_t>("acquisitionNumber",2);
//
//	ch11.voxel<float>(0,0)=42.0;
//	ch12.voxel<float>(1,1)=42.0;
//	ch13.voxel<float>(2,2)=42;
//
//	BOOST_CHECK(img.insertChunk(ch11));
//	BOOST_CHECK(img.insertChunk(ch12));
//	BOOST_CHECK(img.insertChunk(ch13));
//
//	BOOST_CHECK(img.voxel<float>(0,0,0,0)==42);
//	BOOST_CHECK(img.voxel<float>(1,1,1,0)==42);
//	BOOST_CHECK(img.voxel<float>(2,2,2,0)==42);
//
//	/// check for setting voxel data
//	img.voxel<float>(2,2,2,0)=23;
//	BOOST_CHECK(img.voxel<float>(2,2,2,0)==23);
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
}}
