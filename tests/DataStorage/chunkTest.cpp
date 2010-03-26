/*
* imageTest.cpp
*
*  Created on: Oct 1, 2009
*      Author: proeger
*/

#define BOOST_TEST_MODULE ChunkTest
#include <boost/test/included/unit_test.hpp>
#include "DataStorage/chunk.hpp"

namespace isis{namespace test{

/* create an image */
BOOST_AUTO_TEST_CASE (chunk_init_test)
{
	ENABLE_LOG(CoreLog,util::DefaultMsgPrint,warning);
	ENABLE_LOG(CoreDebug,util::DefaultMsgPrint,warning);
	ENABLE_LOG(DataLog,util::DefaultMsgPrint,warning);
	ENABLE_LOG(DataDebug,util::DefaultMsgPrint,warning);

	data::MemChunk<float> ch(4,3,2,1);
	BOOST_CHECK_EQUAL(ch.volume(),1*2*3*4);
	BOOST_CHECK_EQUAL(ch.dimSize(data::readDim),4);
	BOOST_CHECK_EQUAL(ch.dimSize(data::phaseDim),3);
	BOOST_CHECK_EQUAL(ch.dimSize(data::sliceDim),2);
	BOOST_CHECK_EQUAL(ch.dimSize(data::timeDim),1);
}

BOOST_AUTO_TEST_CASE (chunk_property_test)
{
	data::MemChunk<float> ch(4,3,2,1);

	//an basic Chunk must be invalid
	BOOST_CHECK(not ch.valid());
	BOOST_CHECK(not ch.hasProperty("indexOrigin"));

	//with an position its valid
	util::fvector4 pos(1,1,1,1);
	ch.setProperty("indexOrigin",pos);
	ch.setProperty("acquisitionNumber",0);
	BOOST_CHECK(ch.valid());

	//properties shall not be case sensitive
	BOOST_CHECK(ch.hasProperty("indexorigin"));

	// and of course the property shall be what it was set to
	BOOST_CHECK(pos == ch.getProperty<util::fvector4>("indexOrigin"));
}

BOOST_AUTO_TEST_CASE (chunk_data_test1)//Access Chunk elements via dimensional index
{
	data::MemChunk<float> ch(4,4,4,4);

	for(size_t i=0;i<ch.dimSize(data::readDim);i++)
		ch.voxel<float>(i,i,i,i)=i;
	for(size_t i=0;i<ch.dimSize(data::readDim);i++)
		BOOST_CHECK_EQUAL(ch.voxel<float>(i,i,i,i),i);

	data::Chunk ch2 = ch;
	for(size_t i=0;i<ch.dimSize(data::readDim);i++)
		BOOST_CHECK_EQUAL(ch2.voxel<float>(i,i,i,i),i);
}

BOOST_AUTO_TEST_CASE (chunk_data_test2)//Access Chunk elements via linear index (threat it as TypePtr)
{
	data::MemChunk<float> ch(4,3,2,1);

	std::ostringstream o;
	unsigned short sample[ch.volume()];
	for(size_t i=0;i<ch.volume();i++){
		ch.asTypePtr<float>()[i]=i;
		sample[i]=i;
	}
	for(size_t i=0;i<ch.volume();i++)
		BOOST_CHECK(ch.getTypePtr<float>()[i]==i);

	util::write_list(
		sample,sample+ch.volume(),o,
		"|",
		util::Type<int>(ch.volume()).toString(false)+"#",""
	);
 	BOOST_CHECK_EQUAL(o.str(),ch.getTypePtr<float>().toString());
}

BOOST_AUTO_TEST_CASE (chunk_copy_test)//Copy chunks
{
	data::MemChunk<float> ch1(4,3,2,1);
	for(size_t i=0;i<ch1.volume();i++)
		ch1.asTypePtr<float>()[i]=i;

	data::Chunk ch2=ch1;//This shall clone the underlying TypePtr-Object

	//but it should of course of the same type and contain the same data
	BOOST_CHECK(ch1.getTypePtrBase().isSameType(ch2.getTypePtrBase()));
	BOOST_CHECK(ch1.getTypePtrBase().is<float>());
	BOOST_CHECK_EQUAL(ch1.volume(),ch2.volume());
	for(size_t i=0;i<ch2.volume();i++)
		BOOST_CHECK_EQUAL(ch2.getTypePtr<float>()[i],i);

	//cloning chunks is a cheap copy, thus any copied chunk shares data
	for(size_t i=0;i<ch2.volume();i++){
		ch1.asTypePtr<float>()[i]=0;
		BOOST_CHECK_EQUAL(ch2.getTypePtr<float>()[i],0);
	}
}
BOOST_AUTO_TEST_CASE (memchunk_copy_test)//Copy chunks
{
	data::MemChunk<float> ch1(4,3,2,1);
	for(size_t i=0;i<ch1.volume();i++)
		ch1.asTypePtr<float>()[i]=i;
	
	data::MemChunk<short> ch2(ch1);//This shall deep copy the chunk and convert the float data to short
	
	//it should of course have the same size
	BOOST_CHECK_EQUAL(ch1.volume(),ch2.volume());
	for(size_t i=0;i<ch2.volume();i++)
		BOOST_CHECK_EQUAL(ch2.asTypePtr<short>()[i],i);
}
}}