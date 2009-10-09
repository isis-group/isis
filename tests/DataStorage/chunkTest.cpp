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
	BOOST_CHECK(not ch.hasProperty("position"));

	//with an position its valid
	isis::util::fvector4 pos(1,1,1,1);
	ch.setProperty("Position",pos);
	BOOST_CHECK(ch.sufficient());

	//properties shall not be case sensitive
	BOOST_CHECK(ch.hasProperty("position"));

	// and of course the property shall be what it was set to
	BOOST_CHECK(pos == ch.getProperty<isis::util::fvector4>("position"));
}

BOOST_AUTO_TEST_CASE (chunk_data_test1)//Access Chunk elements via dimensional index
{
	isis::data::MemChunk<float> ch(1,2,3,4);

	for(size_t i=0;i<ch.size(isis::data::readDim);i++)
		ch(0,0,0,i)=i;
	for(size_t i=0;i<ch.size(isis::data::readDim);i++)
		BOOST_CHECK(ch(0,0,0,i)==i);
}

BOOST_AUTO_TEST_CASE (chunk_data_test2)//Access Chunk elements via linear index (threat it as TypePtr)
{
	isis::data::MemChunk<float> ch(1,2,3,4);
	
	std::ostringstream o;
	unsigned short sample[ch.volume()];
	for(size_t i=0;i<ch.volume();i++){
		ch[i]=i;
		sample[i]=i;
	}
	for(size_t i=0;i<ch.volume();i++)
		BOOST_CHECK(ch[i]==i);

	isis::util::write_list(
		sample,sample+ch.volume(),o,
		"|",isis::util::Type<int>(ch.volume()).toString(false)+"#"
	);
	BOOST_CHECK(o.str() == ch.toString());
}
