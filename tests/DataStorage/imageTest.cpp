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

namespace isis
{
namespace test
{

/* create an image */
BOOST_AUTO_TEST_CASE ( image_init_test )
{
	util::enable_log<util::DefaultMsgPrint>( error );
	data::enable_log<util::DefaultMsgPrint>( error );
	data::MemChunk<float> ch( 4, 4 );
	data::Image img;
	// inserting insufficient Chunk should fail
	BOOST_CHECK( not img.insertChunk( ch ) );
	// but inserting a proper Chunk should work
	ch.setProperty( "indexOrigin", util::fvector4( 0, 0, 2, 0 ) );
	ch.setProperty<u_int32_t>( "acquisitionNumber", 2 );
	ch.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
	BOOST_CHECK( img.insertChunk( ch ) );
	//inserting the same chunk twice should fail
	BOOST_CHECK( not img.insertChunk( ch ) );
	// but inserting another Chunk should work
	ch = data::MemChunk<float>( 4, 4 );
	ch.setProperty( "indexOrigin", util::fvector4( 0, 0, 0, 0 ) );
	ch.setProperty<u_int32_t>( "acquisitionNumber", 0 );
	ch.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
	BOOST_CHECK( img.insertChunk( ch ) );
	// Chunks should be inserted based on their position (lowest first)
	ch = data::MemChunk<float>( 4, 4 );
	ch.setProperty( "indexOrigin", util::fvector4( 0, 0, 1, 0 ) );
	ch.setProperty<u_int32_t>( "acquisitionNumber", 1 );
	ch.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
	BOOST_CHECK( img.insertChunk( ch ) );
	//threat image as a list of sorted chunks
	//Image-Chunk-List should be copyable into other lists (and its order should be correct)
	//Note: this ordering is not allways geometric correct
	//@todo equality test
	std::list<data::Chunk> list( img.chunksBegin(), img.chunksEnd() );
	unsigned short i = 0;
	BOOST_FOREACH( const data::Chunk & ref, list ) {
		BOOST_CHECK( ref.getPropertyValue( "indexOrigin" ) == util::fvector4( 0, 0, i, 0 ) );
		BOOST_CHECK( ref.getPropertyValue( "acquisitionNumber" ) == i++ );
	}
	//Get a list of properties from the chunks in the image
	//List of the properties shall be as if every chunk of the image was asked for the property
	i = 0;
	std::list<util::PropertyValue> origins = img.getChunksProperties( "indexOrigin" );
	BOOST_FOREACH( const util::PropertyValue & ref, origins ) {
		BOOST_CHECK( ref == util::fvector4( 0, 0, i++, 0 ) );
	}
	// Check for insertion in two dimensions
	ch = data::MemChunk<float>( 4, 4 );
	ch.setProperty( "indexOrigin", util::fvector4( 0, 0, 0, 1 ) );
	ch.setProperty<u_int32_t>( "acquisitionNumber", 4 );
	ch.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
	BOOST_CHECK( img.insertChunk( ch ) );
	data::Image::ChunkIterator it = img.chunksEnd();
	//as all other chunks where timestep 0 this must be at the end
	BOOST_CHECK( ( --it )->getPropertyValue( "indexOrigin" ) == util::fvector4( 0, 0, 0, 1 ) );
	BOOST_CHECK( ( it )->getPropertyValue( "acquisitionNumber" ) == int( 4 ) );
}

BOOST_AUTO_TEST_CASE ( image_chunk_test )
{
	u_int32_t acNum = 0;
	std::vector<std::vector<data::MemChunk<float> > > ch( 3, std::vector<data::MemChunk<float> >( 3, data::MemChunk<float>( 3, 3 ) ) );
	data::Image img;

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ ) {
			ch[i][j].setProperty( "indexOrigin", util::fvector4( 0, 0, j, i ) );
			ch[i][j].setProperty( "acquisitionNumber", acNum++ );
			ch[i][j].setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
			ch[i][j].voxel<float>( j, j ) = 42;
			BOOST_CHECK( img.insertChunk( ch[i][j] ) );
		}

	img.reIndex();
	BOOST_CHECK_EQUAL( img.volume(), 9 * 9 );
	BOOST_CHECK_EQUAL( img.sizeToVector(), util::ivector4( 3, 3, 3, 3 ) );
	const data::Chunk &ref11 = img.getChunk( 0, 0, 0 );
	const data::Chunk &ref12 = img.getChunk( 1, 1, 1 );
	const data::Chunk &ref13 = img.getChunk( 2, 2, 2 );
	//                                         r,p,s
	const data::Chunk &ref22 = img.getChunk( 1, 1, 1, 1 );
	const data::Chunk &ref21 = img.getChunk( 0, 0, 0, 1 );
	const data::Chunk &ref23 = img.getChunk( 2, 2, 2, 1 );
	BOOST_CHECK_EQUAL( ref11.getPropertyValue( "indexOrigin" ), util::fvector4( 0, 0, 0, 0 ) );
	BOOST_CHECK_EQUAL( ref12.getPropertyValue( "indexOrigin" ), util::fvector4( 0, 0, 1, 0 ) );
	BOOST_CHECK_EQUAL( ref13.getPropertyValue( "indexOrigin" ), util::fvector4( 0, 0, 2, 0 ) );
	BOOST_CHECK_EQUAL( ref11.getPropertyValue( "acquisitionNumber" ), 0 );
	BOOST_CHECK_EQUAL( ref12.getPropertyValue( "acquisitionNumber" ), 1 );
	BOOST_CHECK_EQUAL( ref13.getPropertyValue( "acquisitionNumber" ), 2 );
	BOOST_CHECK_EQUAL( ref21.getPropertyValue( "acquisitionNumber" ), 3 );
	BOOST_CHECK_EQUAL( ref22.getPropertyValue( "acquisitionNumber" ), 4 );
	BOOST_CHECK_EQUAL( ref23.getPropertyValue( "acquisitionNumber" ), 5 );
	BOOST_CHECK_EQUAL( ref22.getPropertyValue( "indexOrigin" ), util::fvector4( 0, 0, 1, 1 ) );
	BOOST_CHECK( not ( ref22.getPropertyValue( "indexOrigin" ) == util::fvector4( 0, 0, 1, 0 ) ) );
	BOOST_CHECK_EQUAL( ref11.voxel<float>( 0, 0 ), 42 );
	BOOST_CHECK_EQUAL( ref12.voxel<float>( 1, 1 ), 42 );
	BOOST_CHECK_EQUAL( ref13.voxel<float>( 2, 2 ), 42 );
	BOOST_CHECK_EQUAL( ref22.voxel<float>( 1, 1 ), 42 );
	BOOST_CHECK_EQUAL( ref23.voxel<float>( 2, 2 ), 42 );
	BOOST_CHECK_EQUAL( ref23.voxel<float>( 2, 2 ), 42 );
}

BOOST_AUTO_TEST_CASE ( image_voxel_test )
{
	//  get a voxel from inside and outside the image
	std::vector<data::MemChunk<float> > ch( 3, data::MemChunk<float>( 3, 3 ) );
	data::Image img;

	for ( int i = 0; i < 3; i++ ) {
		ch[i].setProperty( "indexOrigin", util::fvector4( 0, 0, i, 0 ) );
		ch[i].setProperty<u_int32_t>( "acquisitionNumber", i );
		ch[i].setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
	}

	ch[0].voxel<float>( 0, 0 ) = 42.0;
	ch[1].voxel<float>( 1, 1 ) = 42.0;
	ch[2].voxel<float>( 2, 2 ) = 42;

	for ( int i = 0; i < 3; i++ ) {
		BOOST_CHECK( img.insertChunk( ch[i] ) );
	}

	for ( int i = 0; i < 3; i++ ) {
		BOOST_CHECK( img.voxel<float>( i, i, i, 0 ) == 42 );
	}

	/// check for setting voxel data
	img.voxel<float>( 2, 2, 2, 0 ) = 23;
	BOOST_CHECK( img.voxel<float>( 2, 2, 2, 0 ) == 23 );
}

BOOST_AUTO_TEST_CASE( image_minmax_test )
{
	std::vector<std::vector<data::MemChunk<float> > > ch( 3, std::vector<data::MemChunk<float> >( 3, data::MemChunk<float>( 3, 3 ) ) );
	data::Image img;
	unsigned short acNum = 0;
	const util::fvector4 vSize( 1, 1, 1, 0 );

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ ) {
			ch[i][j].setProperty( "indexOrigin", util::fvector4( 0, 0, j, i ) );
			ch[i][j].setProperty( "acquisitionNumber", acNum++ );
			ch[i][j].setProperty( "voxelSize", vSize );
			ch[i][j].voxel<float>( j, j ) = i * j;
			BOOST_CHECK( img.insertChunk( ch[i][j] ) );
		}

	img.reIndex();
	{
		util::_internal::TypeBase::Reference min, max;
		img.getMinMax( min, max );
		BOOST_CHECK(min->is<float>());
		BOOST_CHECK(max->is<float>());
		BOOST_CHECK_EQUAL( min->as<float>(), 0 );
		BOOST_CHECK_EQUAL( max->as<float>(), 4 );
	}
	{
		util::_internal::TypeBase::Reference min, max;
		//this should be 0,0 because the first chunk only has zeros
		img.getChunk( 0, 0, 0, 0 ).getMinMax( min, max );
		BOOST_CHECK(min->is<float>());
		BOOST_CHECK(max->is<float>());
		BOOST_CHECK_EQUAL( min->as<float>(), 0 );
		BOOST_CHECK_EQUAL( max->as<float>(), 0 );
	}
}
BOOST_AUTO_TEST_CASE( memimage_test )
{
	std::vector<std::vector<data::MemChunk<float> > > ch( 3, std::vector<data::MemChunk<float> >( 3, data::MemChunk<float>( 3, 3 ) ) );
	data::Image img;
	unsigned short acNum = 0;
	const util::fvector4 vSize( 1, 1, 1, 0 );

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ ) {
			ch[i][j].setProperty( "indexOrigin", util::fvector4( 0, 0, j, i ) );
			ch[i][j].setProperty( "acquisitionNumber", acNum++ );
			ch[i][j].setProperty( "voxelSize", vSize );
			ch[i][j].voxel<float>( j, j ) = i * j * 1000;
			BOOST_CHECK( img.insertChunk( ch[i][j] ) );
		}

	img.reIndex();
	data::MemImage<u_int8_t> img2( img );
	util::_internal::TypeBase::Reference min, max;
	img2.getMinMax( min, max );
	BOOST_CHECK(min->is<u_int8_t>());
	BOOST_CHECK(max->is<u_int8_t>());
	BOOST_CHECK_EQUAL( min->as<u_int8_t>(), 0 );
	BOOST_CHECK_EQUAL( max->as<u_int8_t>(), 255 );
}
}
}
