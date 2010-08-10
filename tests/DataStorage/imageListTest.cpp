/*
 * imageTest.cpp
 *
 *  Created on: Oct 1, 2009
 *      Author: proeger
 */

#define BOOST_TEST_MODULE ImageListTest
#include <boost/test/included/unit_test.hpp>
#include <boost/foreach.hpp>
#include "DataStorage/image.hpp"

namespace isis
{
namespace test
{

/* create an image list from chunks*/
BOOST_AUTO_TEST_CASE ( imageList_chunk_test )
{
	const size_t images = 5;
	const size_t timesteps = 10;
	ENABLE_LOG( CoreLog, util::DefaultMsgPrint, warning );
	ENABLE_LOG( CoreDebug, util::DefaultMsgPrint, warning );
	ENABLE_LOG( DataLog, util::DefaultMsgPrint, info );
	ENABLE_LOG( DataDebug, util::DefaultMsgPrint, info );
	data::ChunkList chunks;

	for ( size_t i = 0; i < timesteps; i++ ) {
		for ( size_t c = 0; c < images; c++ ) {
			data::ChunkList::value_type  ch( new data::MemChunk<float>( 3, 3, 3 ) );
			ch->setProperty( "indexOrigin", util::fvector4( 0, 0, 0, i ) );
			ch->voxel<float>( 0, 0, 0 ) = c + i;
			chunks.push_back( ch );
		}
	}

	data::ImageList list( chunks );
	BOOST_CHECK( list.size() == images );
	short cnt = 0;
	BOOST_FOREACH( data::ImageList::value_type & ref, list ) {
		BOOST_CHECK( ref->sizeToVector() == util::fvector4( 3, 3, 3, timesteps ) );

		for ( size_t i = 0; i < timesteps; i++ )
			BOOST_CHECK( ref->voxel<float>( 0, 0, 0, i ) == i + cnt );

		cnt++;
	}
}

}
}