#define BOOST_TEST_MODULE ImageTest
#define NOMINMAX 1
#include <boost/test/included/unit_test.hpp>
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
	data::_internal::SortedChunkList chunks( "indexOrigin", "readVec,phaseVec,sliceVec,coilChannelMask,sequenceNumber" );
	chunks.addSecondarySort( "acquisitionNumber" );
	chunks.addSecondarySort( "acquisitionTime" );
	data::MemChunk<float> ch( 4, 4 );
	// inserting insufficient Chunk should fail
	data::enable_log<util::DefaultMsgPrint>( error );
	BOOST_CHECK( ! chunks.insert( ch ) );
	data::enable_log<util::DefaultMsgPrint>( warning );
	// inserting Chunk without indexOrigin should still fail
	ch.setProperty<uint32_t>( "acquisitionNumber", 2 );
	data::enable_log<util::DefaultMsgPrint>( error );
	BOOST_CHECK( ! chunks.insert( ch ) );
	data::enable_log<util::DefaultMsgPrint>( warning );
	// inserting Chunk with indexOrigin and acquisitionNumber should be ok
	ch.setProperty( "indexOrigin", util::fvector4( 0, 0, 2, 0 ) );
	BOOST_REQUIRE( chunks.insert( ch ) );
	// there should be exactly one chunk in the list
	BOOST_CHECK_EQUAL( chunks.getLookup().size(), 1 );
}

BOOST_AUTO_TEST_CASE ( chunklist_sort_test )
{
	data::_internal::SortedChunkList chunks( "indexOrigin", "readVec,phaseVec,sliceVec,coilChannelMask,sequenceNumber" );
	chunks.addSecondarySort( "acquisitionNumber" );
	chunks.addSecondarySort( "acquisitionTime" );

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ ) {
			data::MemChunk<float> ch( 3, 3 );
			ch.setProperty( "indexOrigin", util::fvector4( 0, 0, j, i ) );
			ch.setProperty( "acquisitionNumber", 0 );
			BOOST_REQUIRE( chunks.insert( ch ) );
		}

	// inserting duplicate Chunk should fail
	data::MemChunk<float> ch1( 3, 3 );
	ch1.setProperty( "indexOrigin", util::fvector4( 0, 0, 0, 0 ) );
	ch1.setProperty<uint32_t>( "acquisitionNumber", 0 );
	data::enable_log<util::DefaultMsgPrint>( error );
	BOOST_CHECK( ! chunks.insert( ch1 ) );
	data::enable_log<util::DefaultMsgPrint>( warning );
	BOOST_CHECK( chunks.isRectangular() );
	// inserting Chunk with diffent secondary prop should be ok
	ch1.setProperty<uint32_t>( "acquisitionNumber", 1 );
	BOOST_CHECK( chunks.insert( ch1 ) );
	// and the list it should not be rectangular anymore
	BOOST_CHECK( ! chunks.isRectangular() );

	// add the renmaining acquisitionNumber=1-chunks
	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ ) {
			data::MemChunk<float> ch( 3, 3 );
			ch.setProperty( "indexOrigin", util::fvector4( 0, 0, j, i ) );
			ch.setProperty<uint32_t>( "acquisitionNumber", 1 );

			if( i == 0 && j == 0 ) { // this is allready there
				data::enable_log<util::DefaultMsgPrint>( error );
				BOOST_REQUIRE( ! chunks.insert( ch ) );
				data::enable_log<util::DefaultMsgPrint>( warning );
			} else
				BOOST_REQUIRE( chunks.insert( ch ) );
		}

	// there should be exactly 3*3 *2 chunks in the list
	BOOST_CHECK_EQUAL( chunks.getLookup().size(), 18 );
	// and it should be rectangular by now
	BOOST_CHECK( chunks.isRectangular() );
}

BOOST_AUTO_TEST_CASE ( chunklist_secondary_sort_test )
{
	data::MemChunk<float> chNumber( 3, 3 ), chTime( 3, 3 );
	chNumber.setProperty<uint32_t>( "acquisitionNumber", 0 );
	chNumber.setProperty( "indexOrigin", util::fvector4( 0, 0, 0, 0 ) );
	chTime.setProperty( "acquisitionTime", 0 );
	chTime.setProperty( "indexOrigin", util::fvector4( 0, 0, 0, 0 ) );
	{
		// inserting a not-first chunk which lacks the current secondary sort property should fail
		data::_internal::SortedChunkList chunks( "indexOrigin", "readVec,phaseVec,sliceVec,coilChannelMask,sequenceNumber" );
		chunks.addSecondarySort( "acquisitionNumber" );
		chunks.addSecondarySort( "acquisitionTime" );
		BOOST_REQUIRE( chunks.insert( chTime ) ); // this is ok - has "acquisitionTime" which is the top secondary sort-property
		data::enable_log<util::DefaultMsgPrint>( error );
		BOOST_CHECK( !chunks.insert( chNumber ) ); // this should fail - it does not have "acquisitionTime"
		data::enable_log<util::DefaultMsgPrint>( warning );
	}
	{
		// inserting a not-first chunk which lacks the fallback secondary sort property should fail
		data::_internal::SortedChunkList chunks( "indexOrigin", "readVec,phaseVec,sliceVec,coilChannelMask,sequenceNumber" );
		chunks.addSecondarySort( "acquisitionNumber" );
		chunks.addSecondarySort( "acquisitionTime" );
		// this is ok - has "acquisitionNumber" to which the sorting will fallback
		BOOST_REQUIRE( chunks.insert( chNumber ) );
		data::enable_log<util::DefaultMsgPrint>( error );
		BOOST_CHECK( !chunks.insert( chTime ) ); // this should fail - it does not have "acquisitionNumber" - the current secondary sorting
		data::enable_log<util::DefaultMsgPrint>( warning );
	}
}

}
}
