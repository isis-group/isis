/*
* imageTest.cpp
*
*  Created on: Oct 1, 2009
*      Author: proeger
*/

#define BOOST_TEST_MODULE ChunkTest
#include <boost/test/included/unit_test.hpp>
#include "DataStorage/chunk.hpp"
#include <boost/foreach.hpp>

namespace isis
{
namespace test
{

/* create an image */
BOOST_AUTO_TEST_CASE ( chunk_init_test )
{
	ENABLE_LOG( CoreLog, util::DefaultMsgPrint, warning );
	ENABLE_LOG( CoreDebug, util::DefaultMsgPrint, warning );
	ENABLE_LOG( DataLog, util::DefaultMsgPrint, warning );
	ENABLE_LOG( DataDebug, util::DefaultMsgPrint, warning );
	data::MemChunk<float> ch( 4, 3, 2, 1 );
	BOOST_CHECK_EQUAL( ch.volume(), 1 * 2 * 3 * 4 );
	BOOST_CHECK_EQUAL( ch.dimSize( data::readDim ), 4 );
	BOOST_CHECK_EQUAL( ch.dimSize( data::phaseDim ), 3 );
	BOOST_CHECK_EQUAL( ch.dimSize( data::sliceDim ), 2 );
	BOOST_CHECK_EQUAL( ch.dimSize( data::timeDim ), 1 );
}

BOOST_AUTO_TEST_CASE ( chunk_mem_init_test )
{
	const short data[3*3] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
	data::MemChunk<short> ch( data, 3, 3 );
	BOOST_CHECK_EQUAL( ch.volume(), 3 * 3 );

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ )
			BOOST_CHECK_EQUAL( ch.voxel<short>( i, j ), i + j * 3 );
}

BOOST_AUTO_TEST_CASE ( chunk_property_test )
{
	data::MemChunk<float> ch( 4, 3, 2, 1 );
	//an basic Chunk must be invalid
	BOOST_CHECK( !ch.valid() );
	BOOST_CHECK( !ch.hasProperty( "indexOrigin" ) );
	//with an position its valid
	util::fvector4 pos( 1, 1, 1, 1 );
	ch.setProperty( "indexOrigin", pos );
	ch.setProperty<int32_t>( "acquisitionNumber", 0 );
	ch.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
	BOOST_CHECK( ch.valid() );
	//properties shall not be case sensitive
	BOOST_CHECK( ch.hasProperty( "indexorigin" ) );
	// and of course the property shall be what it was set to
	BOOST_CHECK( pos == ch.getProperty<util::fvector4>( "indexOrigin" ) );
}

BOOST_AUTO_TEST_CASE ( chunk_data_test1 )//Access Chunk elements via dimensional index
{
	data::MemChunk<float> ch( 4, 4, 4, 4 );

	for ( size_t i = 0; i < ch.dimSize( data::readDim ); i++ )
		ch.voxel<float>( i, i, i, i ) = i;

	for ( size_t i = 0; i < ch.dimSize( data::readDim ); i++ )
		BOOST_CHECK_EQUAL( ch.voxel<float>( i, i, i, i ), i );

	data::Chunk ch2 = ch;

	for ( size_t i = 0; i < ch.dimSize( data::readDim ); i++ )
		BOOST_CHECK_EQUAL( ch2.voxel<float>( i, i, i, i ), i );
}

BOOST_AUTO_TEST_CASE ( chunk_data_test2 )//Access Chunk elements via linear index (threat it as TypePtr)
{
	data::MemChunk<float> ch( 4, 3, 2, 1 );
	std::ostringstream o;
	const size_t vol=4*3*2*1;
	BOOST_REQUIRE_EQUAL(vol,ch.volume());
	unsigned short sample[vol];

	for ( size_t i = 0; i < ch.volume(); i++ ) {
		ch.asTypePtr<float>()[i] = i;
		sample[i] = i;
	}

	for ( size_t i = 0; i < ch.volume(); i++ )
		BOOST_CHECK( ch.getTypePtr<float>()[i] == i );

	util::write_list(
		sample, sample + ch.volume(), o,
		"|",
		util::Type<int32_t>( ch.volume() ).toString( false ) + "#", ""
	);
	BOOST_CHECK_EQUAL( o.str(), ch.getTypePtr<float>().toString() );
}

BOOST_AUTO_TEST_CASE ( chunk_copy_test )//Copy chunks
{
	data::MemChunk<float> ch1( 4, 3, 2, 1 );

	for ( size_t i = 0; i < ch1.volume(); i++ )
		ch1.asTypePtr<float>()[i] = i;

	data::Chunk ch2 = ch1;//This shall clone the underlying TypePtr-Object
	//but it should of course of the same type and contain the same data
	BOOST_CHECK( ch1.getTypePtrBase().isSameType( ch2.getTypePtrBase() ) );
	BOOST_CHECK( ch1.getTypePtrBase().is<float>() );
	BOOST_CHECK_EQUAL( ch1.volume(), ch2.volume() );

	for ( size_t i = 0; i < ch2.volume(); i++ )
		BOOST_CHECK_EQUAL( ch2.getTypePtr<float>()[i], i );

	//cloning chunks is a cheap copy, thus any copied chunk shares data
	for ( size_t i = 0; i < ch2.volume(); i++ ) {
		ch1.asTypePtr<float>()[i] = 0;
		BOOST_CHECK_EQUAL( ch2.getTypePtr<float>()[i], 0 );
	}
}
BOOST_AUTO_TEST_CASE ( memchunk_copy_test )//Copy chunks
{
	static boost::numeric::converter <	short, double,
	boost::numeric::conversion_traits<short, double>,
	boost::numeric::def_overflow_handler,
	boost::numeric::RoundEven<double>
	> converter;

	data::MemChunk<float> ch1( 4, 3, 2, 1 );
	ch1.setProperty("indexOrigin",util::fvector4(1,2,3,4));

	for ( size_t i = 0; i < ch1.volume(); i++ )
		ch1.asTypePtr<float>()[i] = i;

	data::MemChunk<short> ch2( ch1 );//This shall deep copy the chunk and convert the float data to short
	data::MemChunk<short> ch3( ch2 );//This shall deep copy the chunk without converting it
	//it should of course have the same size
	BOOST_CHECK_EQUAL( ch1.volume(), ch2.volume() );
	BOOST_CHECK_EQUAL( ch2.volume(), ch3.volume() );

	//it should have the same properties
	BOOST_REQUIRE( ch2.hasProperty("indexOrigin") );
	BOOST_REQUIRE( ch3.hasProperty("indexOrigin") );
	BOOST_CHECK_EQUAL( ch1.propertyValue("indexOrigin"), ch2.propertyValue("indexOrigin") );
	BOOST_CHECK_EQUAL( ch2.propertyValue("indexOrigin"), ch3.propertyValue("indexOrigin") );

	const float scale = float(std::numeric_limits< short >::max()) / (ch2.volume()-1);

	for ( size_t i = 0; i < ch2.volume(); i++ ) {
		BOOST_CHECK_EQUAL( ch2.asTypePtr<short>()[i], converter(i*scale ));
		BOOST_CHECK_EQUAL( ch3.asTypePtr<short>()[i], converter(i*scale ));
	}

	data::MemChunk<short> ch4( 1, 1 );
	ch4 = ch3;
	BOOST_CHECK_EQUAL( ch3.sizeToVector(), ch4.sizeToVector() );

	//because MemChunk does deep copy changing ch3 should not change ch2
	for ( size_t i = 0; i < ch3.volume(); i++ ) {
		ch3.asTypePtr<short>()[i] = 200;
		BOOST_CHECK_EQUAL( ch2.asTypePtr<short>()[i], converter(i*scale ) );
		BOOST_CHECK_EQUAL( ch4.asTypePtr<short>()[i], converter(i*scale ) );
	}
}

BOOST_AUTO_TEST_CASE ( chunk_splice_test )//Copy chunks
{
	data::MemChunk<float> ch1( 3, 3, 3, 3 );
	ch1.setProperty("indexOrigin",util::fvector4(1,1,1,1));
	ch1.setProperty("readVec",util::fvector4(1,0,0));
	ch1.setProperty("phaseVec",util::fvector4(0,1,0));

	for ( size_t i = 0; i < ch1.volume(); i++ )
		ch1.asTypePtr<float>()[i] = i;

	const data::ChunkList splices=ch1.splice(data::sliceDim,util::fvector4(1,1,1,1),util::fvector4(1,1,1,1));
	unsigned short cnt=1;
	BOOST_CHECK_EQUAL(splices.size(),3*3);
	BOOST_FOREACH(data::ChunkList::const_reference ref,splices){
		BOOST_CHECK_EQUAL(ref.getProperty<util::fvector4>("indexOrigin"),util::fvector4(1,1,cnt,1));
		cnt+=2;
	}
}
}
}