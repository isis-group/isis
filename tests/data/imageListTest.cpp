/*
 * imageTest.cpp
 *
 *  Created on: Oct 1, 2009
 *      Author: proeger
 */

#define BOOST_TEST_MODULE ImageListTest
#include <boost/test/unit_test.hpp>
#include <isis/data/image.hpp>
#include <isis/data/io_factory.hpp>

namespace isis
{
namespace test
{

/* create an image list from chunks*/
BOOST_AUTO_TEST_CASE ( imageList_chunk_test )
{
	const size_t images = 5;
	const size_t timesteps = 10;
	std::list<data::Chunk> chunks;

	for ( size_t i = 0; i < timesteps; i++ ) {
		for ( size_t c = 0; c < images; c++ ) {
			data::MemChunk<float> ch( 3, 3, 3 );
			ch.setValueAs( "indexOrigin", util::fvector3( ) );
			ch.setValueAs( "acquisitionNumber",  ( uint32_t )i );
			ch.setValueAs( "rowVec", util::fvector3( {1, 0} ) );
			ch.setValueAs( "columnVec", util::fvector3( {0, 1} ) );
			ch.setValueAs( "voxelSize", util::fvector3( {1, 1, 1} ) );
			ch.setValueAs( "sequenceNumber", ( uint16_t )0 );
			ch.voxel<float>( 0, 0, 0 ) = c + i;
			chunks.push_back( ch );
		}
	}

	std::list<data::Image> list = data::IOFactory::chunkListToImageList( chunks );
	BOOST_CHECK_EQUAL( list.size(), images );
	short cnt = 0;
	for( data::Image & ref : list ) {
		const util::vector4<size_t> cmp{3, 3, 3, timesteps};
		BOOST_CHECK_EQUAL( ref.getSizeAsVector(), cmp );

		for ( size_t i = 0; i < timesteps; i++ )
			BOOST_CHECK( ref.voxel<float>( 0, 0, 0, i ) == i + cnt );

		cnt++;
	}
}

}
}
