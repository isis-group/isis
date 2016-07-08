#define BOOST_TEST_MODULE ImageTest
#define NOMINMAX 1
#include <boost/test/unit_test.hpp>

#include "data/image.hpp"
#include "data/io_factory.hpp"

namespace isis
{
namespace test
{

BOOST_AUTO_TEST_CASE ( chunklist_insert_test )
{
	data::_internal::SortedChunkList chunks( "rowVec,columnVec,sliceVec,coilChannelMask,sequenceNumber" );
	chunks.addSecondarySort( "acquisitionNumber" );
	chunks.addSecondarySort( "acquisitionTime" );

	data::MemChunk<float> ch( 4, 4 );
	ch.setValueAs( "acquisitionNumber", 2 );
	ch.setValueAs( "indexOrigin", util::fvector3( 0, 0, 2 ) );
	ch.setValueAs( "rowVec", util::fvector3( 1, 0 ) );
	ch.setValueAs( "columnVec", util::fvector3( 0, 1 ) );
	ch.setValueAs( "voxelSize", util::fvector3( 1, 1, 1 ) );
	ch.setValueAs( "acquisitionNumber", 2 );
	ch.setValueAs( "indexOrigin", util::fvector3( 0, 0, 2 ) );
	BOOST_REQUIRE( chunks.insert( ch ) );

	// there should be exactly one chunk in the list
	BOOST_CHECK_EQUAL( chunks.getLookup().size(), 1 );
}

BOOST_AUTO_TEST_CASE ( chunklist_sort_test )
{
	data::_internal::SortedChunkList chunks( "rowVec,columnVec,sliceVec,coilChannelMask,sequenceNumber" );
	chunks.addSecondarySort( "acquisitionNumber" );
	chunks.addSecondarySort( "acquisitionTime" );

	for ( int j = 0; j < 3; j++ ) {
		data::MemChunk<float> ch( 3, 3 );
		ch.setValueAs( "indexOrigin", util::fvector3( 0, 0, j ) );
		ch.setValueAs( "acquisitionNumber", 0 );
		ch.setValueAs( "rowVec", util::fvector3( 1, 0 ) );
		ch.setValueAs( "columnVec", util::fvector3( 0, 1 ) );
		ch.setValueAs( "voxelSize", util::fvector3( 1, 1, 1 ) );

		BOOST_REQUIRE( chunks.insert( ch ) );
	}

	// inserting duplicate Chunk should fail
	data::MemChunk<float> ch1( *( chunks.getLookup()[0] ) ); //just make copy of the first chunk
	ch1.setValueAs( "indexOrigin", util::fvector3( 0, 0, 0 ) );
	ch1.setValueAs( "acquisitionNumber", 0 );
	data::enableLog<util::DefaultMsgPrint>( error );
	BOOST_CHECK( ! chunks.insert( ch1 ) );

	// the list should still be rectangular
	data::enableLog<util::DefaultMsgPrint>( warning );
	BOOST_REQUIRE( chunks.getShape().size()==1 );

	// inserting Chunk with diffent secondary prop should be ok
	ch1.setValueAs( "acquisitionNumber", 1 );
	BOOST_CHECK( chunks.insert( ch1 ) );

	// and the list it should not be rectangular anymore
	static const size_t sizes[]={1,2};
	BOOST_CHECK_EQUAL( chunks.getShape(), std::set<size_t>(sizes,sizes+2) ); // thus two sizes (1 and 2)

	// add the renmaining acquisitionNumber=1-chunks
	for ( int j = 1; j < 3; j++ ) { //0 is already there
		data::MemChunk<float> ch( 3, 3 );
		ch.setValueAs( "indexOrigin", util::fvector3( 0, 0, j ) );
		ch.setValueAs( "acquisitionNumber", 1 );
		ch.setValueAs( "rowVec", util::fvector3( 1, 0 ) );
		ch.setValueAs( "columnVec", util::fvector3( 0, 1 ) );
		ch.setValueAs( "voxelSize", util::fvector3( 1, 1, 1 ) );


		BOOST_REQUIRE( chunks.insert( ch ) );
	}

	// there should be exactly 3*2 chunks in the list
	BOOST_CHECK_EQUAL( chunks.getLookup().size(), 6 );
	// and it should be rectangular by now
	BOOST_CHECK( chunks.getShape().size()==1 );
}

}
}
