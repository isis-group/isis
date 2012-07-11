#define BOOST_TEST_MODULE ImageTest
#define NOMINMAX 1
#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/foreach.hpp>
#include "DataStorage/image.hpp"
#include "DataStorage/io_factory.hpp"

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
	ch.setPropertyAs( "acquisitionNumber", 2 );
	ch.setPropertyAs( "indexOrigin", util::fvector3( 0, 0, 2 ) );
	ch.setPropertyAs( "rowVec", util::fvector3( 1, 0 ) );
	ch.setPropertyAs( "columnVec", util::fvector3( 0, 1 ) );
	ch.setPropertyAs( "voxelSize", util::fvector3( 1, 1, 1 ) );
	ch.setPropertyAs( "acquisitionNumber", 2 );
	ch.setPropertyAs( "indexOrigin", util::fvector3( 0, 0, 2 ) );
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
		ch.setPropertyAs( "indexOrigin", util::fvector3( 0, 0, j ) );
		ch.setPropertyAs( "acquisitionNumber", 0 );
		ch.setPropertyAs( "rowVec", util::fvector3( 1, 0 ) );
		ch.setPropertyAs( "columnVec", util::fvector3( 0, 1 ) );
		ch.setPropertyAs( "voxelSize", util::fvector3( 1, 1, 1 ) );

		BOOST_REQUIRE( chunks.insert( ch ) );
	}

	// inserting duplicate Chunk should fail
	data::MemChunk<float> ch1( *( chunks.getLookup()[0] ) ); //just make copy of the first chunk
	ch1.setPropertyAs( "indexOrigin", util::fvector3( 0, 0, 0 ) );
	ch1.setPropertyAs( "acquisitionNumber", 0 );
	data::enableLog<util::DefaultMsgPrint>( error );
	BOOST_CHECK( ! chunks.insert( ch1 ) );

	// the list should still be rectangular
	data::enableLog<util::DefaultMsgPrint>( warning );
	BOOST_REQUIRE( chunks.isRectangular() );

	// inserting Chunk with diffent secondary prop should be ok
	ch1.setPropertyAs( "acquisitionNumber", 1 );
	BOOST_CHECK( chunks.insert( ch1 ) );

	// and the list it should not be rectangular anymore
	BOOST_CHECK( ! chunks.isRectangular() );

	// add the renmaining acquisitionNumber=1-chunks
	for ( int j = 0; j < 3; j++ ) {
		data::MemChunk<float> ch( 3, 3 );
		ch.setPropertyAs( "indexOrigin", util::fvector3( 0, 0, j ) );
		ch.setPropertyAs( "acquisitionNumber", 1 );
		ch.setPropertyAs( "rowVec", util::fvector3( 1, 0 ) );
		ch.setPropertyAs( "columnVec", util::fvector3( 0, 1 ) );
		ch.setPropertyAs( "voxelSize", util::fvector3( 1, 1, 1 ) );


		BOOST_REQUIRE( chunks.insert( ch ) );
	}

	// there should be exactly 3*3 *2 chunks in the list
	BOOST_CHECK_EQUAL( chunks.getLookup().size(), 18 );
	// and it should be rectangular by now
	BOOST_CHECK( chunks.isRectangular() );
}

}
}
