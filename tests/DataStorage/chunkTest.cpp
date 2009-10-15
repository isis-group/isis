/*
* imageTest.cpp
*
*  Created on: Oct 1, 2009
*      Author: proeger
*/

#define BOOST_TEST_MODULE ChunkTest
#include <boost/test/included/unit_test.hpp>
#include <DataStorage/chunk.hpp>

/* create an image */
BOOST_AUTO_TEST_CASE (chunk_init_test)
{
	ENABLE_LOG(isis::util::CoreLog,isis::util::DefaultMsgPrint,isis::util::warning);
	ENABLE_LOG(isis::util::CoreDebug,isis::util::DefaultMsgPrint,isis::util::warning);

	isis::data::MemChunk<float> ch(1,2,3,4);
	BOOST_CHECK(ch.volume()==1*2*3*4);
	BOOST_CHECK(ch.size(isis::data::MemChunk<float>::read)==1);
	BOOST_CHECK(ch.size(isis::data::MemChunk<float>::phase)==2);
	BOOST_CHECK(ch.size(isis::data::MemChunk<float>::slice)==3);
	BOOST_CHECK(ch.size(isis::data::MemChunk<float>::time)==4);
}

BOOST_AUTO_TEST_CASE (chunk_property_test)
{
	isis::data::MemChunk<float> ch(1,2,3,4);

	//an basic Cunk must be invalid
	BOOST_CHECK(not ch.sufficient());
	BOOST_CHECK(not ch.hasProperty("indexOrigin"));

	//with an position its valid
	isis::util::fvector4 pos(1,1,1,1);
	ch.setProperty("indexOrigin",pos);
	BOOST_CHECK(ch.sufficient());

	//properties shall not be case sensitive
	BOOST_CHECK(ch.hasProperty("indexorigin"));

	// and of course the property shall be what it was set to
	BOOST_CHECK(pos == ch.getProperty<isis::util::fvector4>("indexOrigin"));
}

BOOST_AUTO_TEST_CASE (chunk_data_test1)//Access Chunk elements via dimensional index
{
	isis::data::MemChunk<float> ch(1,2,3,4);

	for(size_t i=0;i<ch.size(isis::data::readDim);i++)
		ch.voxel<float>(0,0,0,i)=i;
	for(size_t i=0;i<ch.size(isis::data::readDim);i++)
		BOOST_CHECK(ch.voxel<float>(0,0,0,i)==i);

	isis::data::Chunk ch2 = ch;
	for(size_t i=0;i<ch.size(isis::data::readDim);i++)
		BOOST_CHECK(ch2.voxel<float>(0,0,0,i)==i);
}

BOOST_AUTO_TEST_CASE (chunk_data_test2)//Access Chunk elements via linear index (threat it as TypePtr)
{
	isis::data::MemChunk<float> ch(1,2,3,4);

	std::ostringstream o;
	unsigned short sample[ch.volume()];
	for(size_t i=0;i<ch.volume();i++){
		ch.getTypePtr<float>()[i]=i;
		sample[i]=i;
	}
	for(size_t i=0;i<ch.volume();i++)
		BOOST_CHECK(ch.getTypePtr<float>()[i]==i);

	isis::util::write_list(
		sample,sample+ch.volume(),o,
		"|",isis::util::Type<int>(ch.volume()).toString(false)+"#"
	);
	BOOST_CHECK(o.str() == ch.getTypePtr<float>().toString());
}

BOOST_AUTO_TEST_CASE (chunk_copy_test)//Copy chunks
{
	isis::data::MemChunk<float> ch1(1,2,3,4);
	for(size_t i=0;i<ch1.volume();i++)
		ch1.getTypePtr<float>()[i]=i;

	isis::data::Chunk ch2=ch1;//This shall clone the underlying TypePtr-Object

	//but it should of course be the of the same type an contain the same data
	BOOST_CHECK(ch1->isSameType(*ch2));
	BOOST_CHECK(ch1.volume()==ch2.volume());
	for(size_t i=0;i<ch2.volume();i++)
		BOOST_CHECK(ch2.getTypePtr<float>()[i]==i);

	//cloning chunks is a cheap copy, thus any copied chunk shares data
	for(size_t i=0;i<ch2.volume();i++){
		ch1.getTypePtr<float>()[i]=0;
		BOOST_CHECK(ch2.getTypePtr<float>()[i]==0);
	}
}
