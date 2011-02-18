/*
 * imageTest.cpp
 *
 *  Created on: Oct 1, 2009
 *      Author: proeger
 */

#define BOOST_TEST_MODULE ImageTest
#define NOMINMAX 1
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/foreach.hpp>
#include <DataStorage/image.hpp>
#include <DataStorage/io_factory.hpp>

namespace isis
{
namespace test
{

/* create an image */
BOOST_AUTO_TEST_CASE ( image_init_test )
{
	{
		data::MemChunk<float> ch( 4, 4 );
		data::enableLog<util::DefaultMsgPrint>( verbose_info );
		// inserting a proper Chunk should work
		ch.setPropertyAs( "indexOrigin", util::fvector4( 0, 0, 2 ) );
		ch.setPropertyAs<uint32_t>( "acquisitionNumber", 0 );
		ch.setPropertyAs<float>( "acquisitionTime", 0 );
		ch.setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
		ch.setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
		ch.setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
		data::Image img(ch);
		BOOST_CHECK( img.isClean() );


		// inserting insufficient Chunk should fail
		data::enableLog<util::DefaultMsgPrint>( ( LogLevel )0 );
		BOOST_CHECK( ! img.insertChunk( data::MemChunk<float>( 4, 4 ) ) );

		//inserting the same chunk twice should fail
		BOOST_CHECK( ! img.insertChunk( ch ) );

		// but inserting another Chunk should work
		ch = data::MemChunk<float>( 4, 4 );
		ch.setPropertyAs( "indexOrigin", util::fvector4( 0, 0, 0 ) );
		ch.setPropertyAs<uint32_t>( "acquisitionNumber", 2 );
		ch.setPropertyAs<float>( "acquisitionTime", 2 );
		ch.setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
		ch.setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
		ch.setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
		BOOST_CHECK( img.insertChunk( ch ) );

		// Chunks should be inserted based on their position (lowest first)
		ch = data::MemChunk<float>( 4, 4 );
		ch.setPropertyAs( "indexOrigin", util::fvector4( 0, 0, 1 ) );
		ch.setPropertyAs<uint32_t>( "acquisitionNumber", 1 );
		ch.setPropertyAs<float>( "acquisitionTime", 1 );
		ch.setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
		ch.setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
		ch.setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
		BOOST_REQUIRE( img.insertChunk( ch ) );
		//Get a list of the sorted chunks
		BOOST_REQUIRE( img.reIndex() );
		std::vector<boost::shared_ptr<data::Chunk> > list = img.getChunksAsVector();
		BOOST_CHECK_EQUAL( list.size(), 3 ); // the should be 3 chunks in the list by now

		for( unsigned int i = 0; i < list.size(); i++ ) {
			BOOST_CHECK_EQUAL( list[i]->propertyValue( "indexOrigin" ), util::fvector4( 0, 0, i, 0 ) );
			BOOST_CHECK_EQUAL( list[i]->propertyValue( "acquisitionNumber" ), 2 - i ); // AcqNumber and time are in the oposite direction
			BOOST_CHECK_EQUAL( list[i]->propertyValue( "acquisitionTime" ), 2 - i );
		}

		//Get a list of properties from the chunks in the image
		//List of the properties shall be as if every chunk of the image was asked for the property
		std::list<util::PropertyValue> origins = img.getChunksProperties( "indexOrigin" );
		unsigned int i = 0;
		BOOST_FOREACH( const util::PropertyValue & ref, origins ) {
			BOOST_CHECK( ref == util::fvector4( 0, 0, i++, 0 ) );
		}
	}
	{
		// Check for insertion in two dimensions
		std::list<data::Chunk> chunks;

		for( int i = 0; i < 3; i++ ) {
			chunks.push_back(data::MemChunk<float>( 4, 4 ));
			chunks.back().setPropertyAs( "indexOrigin", util::fvector4( 0, 0, i, 0 ) );
			chunks.back().setPropertyAs<uint32_t>( "acquisitionNumber", 3 + i );
			chunks.back().setPropertyAs<uint32_t>( "acquisitionTime", 3 + i );
			chunks.back().setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
			chunks.back().setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
			chunks.back().setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
		}
		data::Image img(chunks);
		BOOST_CHECK(img.isClean());
		BOOST_CHECK(img.isValid());

		std::string str = "testString";
		img.setPropertyAs<std::string>( "testProp", str );
		BOOST_CHECK_EQUAL( img.getPropertyAs<std::string>( "testProp" ), str );
		boost::shared_ptr<data::Chunk> ptr = img.getChunksAsVector().back();
		//as all other chunks where timestep < 4 this must be at the end
		BOOST_CHECK_EQUAL( ptr->propertyValue( "indexOrigin" ), util::fvector4( 0, 0, 2 ) );
		BOOST_CHECK_EQUAL( ptr->propertyValue( "acquisitionNumber" ), 5  );

		// Check all dimensions
		uint32_t nrRows = 12;
		uint32_t nrCols = 32;
		uint32_t nrTimesteps = 17;
		uint32_t nrSlices = 27;

		chunks.clear();
		for( int t = 0; t < nrTimesteps; t++ ) {
			for( int s = 0; s < nrSlices; s++ ) {
				chunks.push_back(data::MemChunk<float>( nrCols, nrRows ) );
				chunks.back().setPropertyAs( "indexOrigin", util::fvector4( 0, 0, s ) );
				chunks.back().setPropertyAs<uint32_t>( "acquisitionNumber", s + t * nrSlices );
				chunks.back().setPropertyAs<uint32_t>( "acquisitionTime", s + t * nrSlices );
				chunks.back().setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
				chunks.back().setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
				chunks.back().setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
			}
		}
		data::Image img2(chunks);

		BOOST_REQUIRE( img2.isClean() );
		BOOST_CHECK_EQUAL( img2.getVolume(), nrRows * nrCols * nrTimesteps * nrSlices );
		BOOST_CHECK_EQUAL( img2.getSizeAsVector(), util::ivector4( nrCols, nrRows, nrSlices, nrTimesteps ) );

		// Check all dimensions with limit sizes
		nrRows = 212;
		nrCols = 2;
		nrTimesteps = 2;
		nrSlices = 1;
		chunks.clear();
		for( int t = 0; t < nrTimesteps; t++ ) {
			for( int s = 0; s < nrSlices; s++ ) {
				chunks.push_back(data::MemChunk<float>( nrCols, nrRows ));
				chunks.back().setPropertyAs( "indexOrigin", util::fvector4( 0, 0, s ) );
				chunks.back().setPropertyAs<uint32_t>( "acquisitionNumber", s + t * nrSlices );
				chunks.back().setPropertyAs<uint32_t>( "acquisitionTime", s + t * nrSlices );
				chunks.back().setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
				chunks.back().setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
				chunks.back().setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
			}
		}
		data::Image img3(chunks);
		BOOST_REQUIRE( img3.isClean() );
		BOOST_CHECK_EQUAL( img3.getVolume(), nrRows * nrCols * nrTimesteps * nrSlices );
		BOOST_CHECK_EQUAL( img3.getSizeAsVector(), util::ivector4( nrCols, nrRows, nrSlices, nrTimesteps ) );



		nrRows = 54;
		nrCols = 29;
		nrTimesteps = 1;
		nrSlices = 21;
		chunks.clear();
		for( int t = 0; t < nrTimesteps; t++ ) {
			for( int s = 0; s < nrSlices; s++ ) {
				data::Chunk ch = data::MemChunk<float>( nrCols, nrRows );
				ch.setPropertyAs( "indexOrigin", util::fvector4( 0, 0, s, 0 ) );
				ch.setPropertyAs<uint32_t>( "acquisitionNumber", s + t * nrSlices );
				ch.setPropertyAs<uint32_t>( "acquisitionTime", s + t * nrSlices );
				ch.setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
				ch.setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
				ch.setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
			}
		}
		data::Image img4(chunks);

		BOOST_REQUIRE( img4.isClean() );
		BOOST_CHECK_EQUAL( img4.getVolume(), nrRows * nrCols * nrTimesteps * nrSlices );
		BOOST_CHECK_EQUAL( img4.getSizeAsVector(), util::ivector4( nrCols, nrRows, nrSlices, nrTimesteps ) );
	}
}

BOOST_AUTO_TEST_CASE ( minimal_image_test )
{
	data::MemChunk<float> ch( 4, 4 );
	ch.setPropertyAs( "indexOrigin", util::fvector4( 0, 0, 2 ) );
	ch.setPropertyAs<uint32_t>( "acquisitionNumber", 0 );
	ch.setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
	ch.setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
	ch.setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );

	std::list<data::MemChunk<float> > chunks(2,ch);
	chunks.back().setPropertyAs<uint32_t>( "acquisitionNumber", 1 );

	data::Image img(chunks);
	const size_t size[] = {4, 4, 1, 2};
	BOOST_CHECK( img.isClean() );
	BOOST_CHECK( img.isValid() );
	BOOST_CHECK_EQUAL( img.getSizeAsVector(), ( util::FixedVector<size_t, 4>( size ) ) );
}

BOOST_AUTO_TEST_CASE ( type_selection_test )
{
	float org = 0;
	std::list<data::Chunk> chunks;

#define MAKE_CHUNK(type) \
	chunks.push_back(data::MemChunk<type>( 4, 4 ));\
	chunks.back().setPropertyAs( "indexOrigin", util::fvector4( 0, 0, org ) );\
	chunks.back().setPropertyAs<uint32_t>( "acquisitionNumber", org );\
	chunks.back().setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );\
	chunks.back().setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );\
	chunks.back().setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );\
	org++;

	MAKE_CHUNK( int16_t);
	MAKE_CHUNK( int8_t);
	MAKE_CHUNK( uint8_t);
	MAKE_CHUNK( uint16_t);

	std::list<data::Chunk>::iterator i=chunks.begin();

	(i++)->voxel<int16_t>( 0, 0, 0 ) = std::numeric_limits<int16_t>::min();
	(i++)->voxel<int8_t>( 0, 0, 0 ) = std::numeric_limits<int8_t>::min();
	(i++)->voxel<uint8_t>( 0, 0, 0 ) = std::numeric_limits<uint8_t>::max();
	(i++)->voxel<uint16_t>( 0, 0, 0 ) = std::numeric_limits<int16_t>::max(); // the maximum shall fit into int16_t

	data::Image img(chunks);
	const size_t size[] = {4, 4, 4, 1};
	BOOST_CHECK( img.isClean() );
	BOOST_CHECK( img.isValid() );
	BOOST_CHECK_EQUAL(img.getChunksAsVector().size(),4);
	BOOST_CHECK_EQUAL( img.getSizeAsVector(), ( util::FixedVector<size_t, 4>( size ) ) );
	BOOST_CHECK_EQUAL( img.getMajorTypeID(), data::ValuePtr<int16_t>( NULL, 0 ).getTypeID() );
#undef MAKE_CHUNK
}

BOOST_AUTO_TEST_CASE ( type_scale_test )
{
	float org = 0;
	std::list<data::Chunk> chunks;

	#define MAKE_CHUNK(type) \
	chunks.push_back(data::MemChunk<type>( 4, 4 ));\
	chunks.back().setPropertyAs( "indexOrigin", util::fvector4( 0, 0, org ) );\
	chunks.back().setPropertyAs<uint32_t>( "acquisitionNumber", org );\
	chunks.back().setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );\
	chunks.back().setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );\
	chunks.back().setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );\
	org++;

	MAKE_CHUNK( int16_t);
	MAKE_CHUNK( int8_t);
	MAKE_CHUNK( uint8_t);
	MAKE_CHUNK( uint16_t);

	std::list<data::Chunk>::iterator i=chunks.begin();

	(i++)->voxel<int16_t>( 0, 0, 0 ) = std::numeric_limits<int16_t>::min();
	(i++)->voxel<int8_t>( 0, 0, 0 ) = std::numeric_limits<int8_t>::min();
	(i++)->voxel<uint8_t>( 0, 0, 0 ) = std::numeric_limits<uint8_t>::max();
	(i++)->voxel<uint16_t>( 0, 0, 0 ) = std::numeric_limits<int16_t>::max(); // the maximum shall fit into int16_t

	data::Image img(chunks);
	const size_t size[] = {4, 4, 4, 1};
	BOOST_CHECK( img.isClean() );
	BOOST_CHECK( img.isValid() );

	data::scaling_pair scale = img.getScalingTo( data::ValuePtr<uint8_t>::staticID );
	BOOST_CHECK_EQUAL( scale.first->as<double>(), 1. / 10 );
	BOOST_CHECK_EQUAL( scale.second->as<double>(), 5 );
#undef MAKE_CHUNK
}

BOOST_AUTO_TEST_CASE ( image_chunk_test )
{
	uint32_t acNum = 0;
	std::list<data::Chunk> chunks( 9, data::MemChunk<float>( 3, 3 ) );
	std::list<data::Chunk>::iterator k=chunks.begin();

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++,k++ ) {
			k->setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
			k->setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
			k->setPropertyAs( "indexOrigin", util::fvector4( 0, 0, j ) );
			k->setPropertyAs( "acquisitionNumber", acNum++ );
			k->setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1 ) );
			k->voxel<float>( j, j ) = 42;
		}

	data::Image img(chunks);
	BOOST_REQUIRE( img.isClean() );
	BOOST_REQUIRE( img.isValid() );
	BOOST_CHECK_EQUAL( img.getVolume(), 9 * 9 );
	BOOST_CHECK_EQUAL( img.getSizeAsVector(), util::ivector4( 3, 3, 3, 3 ) );

	const data::Chunk &ref11 = img.getChunk( 0, 0, 0 );
	const data::Chunk &ref12 = img.getChunk( 1, 1, 1 );
	const data::Chunk &ref13 = img.getChunk( 2, 2, 2 );
	//                                         r,p,s
	const data::Chunk &ref22 = img.getChunk( 1, 1, 1, 1 );
	const data::Chunk &ref21 = img.getChunk( 0, 0, 0, 1 );
	const data::Chunk &ref23 = img.getChunk( 2, 2, 2, 1 );

	BOOST_CHECK_EQUAL( ref11.propertyValue( "indexOrigin" ), util::fvector4( 0, 0, 0 ) );
	BOOST_CHECK_EQUAL( ref12.propertyValue( "indexOrigin" ), util::fvector4( 0, 0, 1 ) );
	BOOST_CHECK_EQUAL( ref13.propertyValue( "indexOrigin" ), util::fvector4( 0, 0, 2 ) );
	BOOST_CHECK_EQUAL( ref11.propertyValue( "acquisitionNumber" ), ( uint32_t )0 );
	BOOST_CHECK_EQUAL( ref12.propertyValue( "acquisitionNumber" ), ( uint32_t )1 );
	BOOST_CHECK_EQUAL( ref13.propertyValue( "acquisitionNumber" ), ( uint32_t )2 );
	BOOST_CHECK_EQUAL( ref21.propertyValue( "acquisitionNumber" ), ( uint32_t )3 );
	BOOST_CHECK_EQUAL( ref22.propertyValue( "acquisitionNumber" ), ( uint32_t )4 );
	BOOST_CHECK_EQUAL( ref23.propertyValue( "acquisitionNumber" ), ( uint32_t )5 );
	BOOST_CHECK_EQUAL( ref22.propertyValue( "indexOrigin" ), util::fvector4( 0, 0, 1 ) );
	BOOST_CHECK( ref22.propertyValue( "indexOrigin" ) == ref12.propertyValue( "indexOrigin" ) );
	BOOST_CHECK( !( ref22.propertyValue( "acquisitionNumber" ) == ref12.propertyValue( "acquisitionNumber" ) ) );
	BOOST_CHECK_EQUAL( ref11.voxel<float>( 0, 0 ), 42 );
	BOOST_CHECK_EQUAL( ref12.voxel<float>( 1, 1 ), 42 );
	BOOST_CHECK_EQUAL( ref13.voxel<float>( 2, 2 ), 42 );
	BOOST_CHECK_EQUAL( ref22.voxel<float>( 1, 1 ), 42 );
	BOOST_CHECK_EQUAL( ref23.voxel<float>( 2, 2 ), 42 );
	BOOST_CHECK_EQUAL( ref23.voxel<float>( 2, 2 ), 42 );
}

BOOST_AUTO_TEST_CASE ( image_foreach_chunk_test )
{
	uint32_t acNum = 0;
	std::list<data::Chunk> chunks( 9, data::MemChunk<float>( 3, 3 ) );
	std::list<data::Chunk>::iterator k=chunks.begin();

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++,k++ ) {
			k->setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
			k->setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
			k->setPropertyAs( "indexOrigin", util::fvector4( 0, 0, j ) );
			k->setPropertyAs( "acquisitionNumber", acNum++ );
			k->setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1 ) );
			k->voxel<float>( j, j ) = 42;
		}
	data::Image img(chunks);

	class : public data::Image::ChunkOp
	{
		class : public data::Chunk::VoxelOp<uint8_t>
		{
		public:
			bool operator()( uint8_t &vox, const util::FixedVector< size_t, 4 >& pos ) {
				vox = 42;
				return true;
			}
		} vox42;
	public:
		bool operator()( data::Chunk &ch, util::FixedVector<size_t, 4> posInImage ) {
			return ch.foreachVoxel( vox42 ) == 0;
		}
	} set42;

	class setIdx: public data::Chunk::VoxelOp<uint8_t>
	{
		data::_internal::NDimensional<4> geometry;
	public:
		setIdx( data::_internal::NDimensional<4> geo ): geometry( geo ) {}
		bool operator()( uint8_t &vox, const util::FixedVector< size_t, 4 >& pos ) {
			vox = geometry.getLinearIndex( &pos[0] );
			return true;
		}
	};

	BOOST_REQUIRE_EQUAL( img.foreachChunk( set42 ), 0 );

	util::FixedVector<size_t, 4> imgSize = img.getSizeAsVector();

	for( size_t t = 0; t < imgSize[data::timeDim]; t++ ) {
		for( size_t z = 0; z < imgSize[data::sliceDim]; z++ ) {
			for( size_t y = 0; y < imgSize[data::columnDim]; y++ ) {
				for( size_t x = 0; x < imgSize[data::rowDim]; x++ ) {
					BOOST_CHECK_EQUAL( img.voxel<uint8_t>( x, y, z, t ), 42 );
				}
			}
		}
	}

	setIdx setidx( img );
	BOOST_REQUIRE_EQUAL( img.foreachVoxel<uint8_t>( setidx ), 0 );
	uint8_t cnt = 0;

	for( size_t t = 0; t < imgSize[data::timeDim]; t++ ) {
		for( size_t z = 0; z < imgSize[data::sliceDim]; z++ ) {
			for( size_t y = 0; y < imgSize[data::columnDim]; y++ ) {
				for( size_t x = 0; x < imgSize[data::rowDim]; x++ ) {
					BOOST_CHECK_EQUAL( img.voxel<uint8_t>( x, y, z, t ), cnt++ );
				}
			}
		}
	}

}

BOOST_AUTO_TEST_CASE ( image_voxel_test )
{
	//  get a voxel from inside and outside the image
	std::list<data::Chunk> chunks( 3, data::MemChunk<float>( 3, 3 ) );
	unsigned short acNum = 0;

	BOOST_FOREACH(data::Chunk &ch,chunks){
		ch.setPropertyAs( "indexOrigin", util::fvector4( 0, 0, acNum, 0 ) );
		ch.setPropertyAs<uint32_t>( "acquisitionNumber", acNum );
		ch.setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
		ch.setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
		ch.setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
		acNum++;
	}

	std::list<data::Chunk>::iterator k=chunks.begin();
	(k++)->voxel<float>( 0, 0 ) = 42.0;
	(k++)->voxel<float>( 1, 1 ) = 42.0;
	(k++)->voxel<float>( 2, 2 ) = 42;

	data::Image img(chunks);
	BOOST_REQUIRE( img.isClean() );
	BOOST_CHECK( img.isValid() );


	for ( int i = 0; i < 3; i++ ) {
		BOOST_CHECK( img.voxel<float>( i, i, i, 0 ) == 42 );
	}

	/// check for setting voxel data
	img.voxel<float>( 2, 2, 2, 0 ) = 23;
	BOOST_CHECK( img.voxel<float>( 2, 2, 2, 0 ) == 23 );
}

BOOST_AUTO_TEST_CASE( image_minmax_test )
{
	std::list<data::Chunk> chunks( 3, data::MemChunk<float>( 3, 3 ) );
	unsigned short acNum = 0;

	BOOST_FOREACH(data::Chunk &ch,chunks){
		ch.setPropertyAs( "indexOrigin", util::fvector4( 0, 0, acNum, 0 ) );
		ch.setPropertyAs<uint32_t>( "acquisitionNumber", acNum );
		ch.setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
		ch.setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
		ch.setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
		acNum++;
	}
	data::Image img(chunks);
	BOOST_REQUIRE( img.isClean() );
	BOOST_CHECK( img.isValid() );

	{
		std::pair<util::ValueReference, util::ValueReference> minmax = img.getMinMax();
		BOOST_CHECK( minmax.first->is<float>() );
		BOOST_CHECK( minmax.second->is<float>() );
		BOOST_CHECK_EQUAL( minmax.first->as<float>(), 0 );
		BOOST_CHECK_EQUAL( minmax.second->as<float>(), 4 );
	}
	{
		//this should be 0,0 because the first chunk only has zeros
		std::pair<util::ValueReference, util::ValueReference> minmax = img.getChunk( 0, 0, 0, 0 ).getMinMax();
		BOOST_CHECK( minmax.first->is<float>() );
		BOOST_CHECK( minmax.second->is<float>() );
		BOOST_CHECK_EQUAL( minmax.first->as<float>(), 0 );
		BOOST_CHECK_EQUAL( minmax.second->as<float>(), 0 );
	}
}
BOOST_AUTO_TEST_CASE( orientation_test )
{
	data::MemChunk<float> ch( 3, 3, 3 );
	ch.setPropertyAs( "indexOrigin", util::fvector4( 0, 0, 0 ) );
	ch.setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
	ch.setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
	ch.setPropertyAs( "acquisitionNumber", ( uint32_t )0 );
	ch.setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );

	data::Image img(ch);
	BOOST_REQUIRE( img.isClean() );
	BOOST_REQUIRE( img.isValid() );

// 	data::enableLog<util::DefaultMsgPrint>( error );
// 	data::enableLog<util::DefaultMsgPrint>( warning );
	BOOST_CHECK_EQUAL( img.getMainOrientation(), data::Image::axial );
}

BOOST_AUTO_TEST_CASE( memimage_test )
{
	std::list<data::Chunk> chunks( 9, data::MemChunk<float>( 3, 3 ) );
	std::list<data::Chunk>::iterator k=chunks.begin();


	uint32_t acNum = 0;
	const util::fvector4 vSize( 1, 1, 1, 0 );

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++,k++ ) {
			k->setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
			k->setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
			k->setPropertyAs( "indexOrigin", util::fvector4( 0, 0, j ) );
			k->setPropertyAs( "acquisitionNumber", acNum++ );
			k->setPropertyAs( "voxelSize", vSize );
			k->voxel<float>( j, j ) = i * j * 1000;
		}

	data::Image img(chunks);
	BOOST_REQUIRE( img.isClean() );
	{
		std::pair<util::ValueReference, util::ValueReference> minmax = img.getMinMax();
		BOOST_CHECK( minmax.first->is<float>() );
		BOOST_CHECK( minmax.second->is<float>() );
		BOOST_CHECK_EQUAL( minmax.first->as<float>(), 0 );
		BOOST_CHECK_EQUAL( minmax.second->as<float>(), 2 * 2 * 1000 );
	}
	{
		// Conversion to uint8_t (will downscale [0-255])
		data::enableLog<util::DefaultMsgPrint>( error );
		data::MemImage<uint8_t> img2( img );
		data::enableLog<util::DefaultMsgPrint>( warning );
		BOOST_REQUIRE( img2.reIndex() );
		//Check if the metadata were copied correct
		BOOST_CHECK_EQUAL( static_cast<util::PropertyMap>( img ), static_cast<util::PropertyMap>( img2 ) );

		for ( int i = 0; i < 3; i++ )
			for ( int j = 0; j < 3; j++ ) {
				const util::PropertyMap
				&c1 = img.getChunk( 0, 0, i, j );
				const util::PropertyMap
				&c2 = img2.getChunk( 0, 0, i, j );
				BOOST_REQUIRE( c1.isValid
							   () );
				BOOST_CHECK( c2.isValid
							 () );
				BOOST_CHECK_EQUAL( c1, c2 );
			}

		std::pair<util::ValueReference, util::ValueReference> minmax = img2.getMinMax();
		BOOST_CHECK( minmax.first->is<uint8_t>() );
		BOOST_CHECK( minmax.second->is<uint8_t>() );
		BOOST_CHECK_EQUAL( minmax.first->as<uint8_t>(), 0 );
		BOOST_CHECK_EQUAL( minmax.second->as<uint8_t>(), 255 );
	}
	{
		// Conversion to int16_t (will upscale [0-32k) )
		data::MemImage<int16_t> img2( img );
		BOOST_REQUIRE( img2.reIndex() );
		BOOST_CHECK_EQUAL( static_cast<util::PropertyMap>( img ), static_cast<util::PropertyMap>( img2 ) );
		std::pair<util::ValueReference, util::ValueReference> minmax = img2.getMinMax();
		BOOST_CHECK( minmax.first->is<int16_t>() );
		BOOST_CHECK( minmax.second->is<int16_t>() );
		BOOST_CHECK_EQUAL( minmax.first->as<int16_t>(), 0 );
		BOOST_CHECK_EQUAL( minmax.second->as<int16_t>(), std::numeric_limits<int16_t>::max() );
	}
} // END memimage_test

BOOST_AUTO_TEST_CASE( typediamge_test )
{
	std::list<data::Chunk> chunks;
	std::list<data::Chunk>::iterator k=chunks.begin();

	uint32_t acNum = 0;
	const util::fvector4 vSize( 1, 1, 1, 0 );

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++,k++ ) {
			chunks.push_back(data::MemChunk<float>( 3, 3 ));
			chunks.back().setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
			chunks.back().setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
			chunks.back().setPropertyAs( "indexOrigin", util::fvector4( 0, 0, j ) );
			chunks.back().setPropertyAs( "acquisitionNumber", acNum++ );
			chunks.back().setPropertyAs( "voxelSize", vSize );
			chunks.back().voxel<uint8_t>( j, j ) = std::numeric_limits<uint8_t>::max();
		}

	for ( int i = 3; i < 10; i++ )
		for ( int j = 0; j < 3; j++ ) {
			chunks.push_back(data::MemChunk<int16_t>( 3, 3 ));
			chunks.back().setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
			chunks.back().setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
			chunks.back().setPropertyAs( "indexOrigin", util::fvector4( 0, 0, j ) );
			chunks.back().setPropertyAs( "acquisitionNumber", acNum++ );
			chunks.back().setPropertyAs( "voxelSize", vSize );
			chunks.back().voxel<int16_t>( j, j ) = std::numeric_limits<int16_t>::max();
		}

	data::Image img(chunks);
	BOOST_REQUIRE( img.isClean() );
	{
		std::pair<util::ValueReference, util::ValueReference> minmax = img.getMinMax();
		BOOST_CHECK( minmax.first->is<uint8_t>() );
		BOOST_CHECK( minmax.second->is<int16_t>() );
		BOOST_CHECK_EQUAL( minmax.first->as<int16_t>(), 0 );
		BOOST_CHECK_EQUAL( minmax.second->as<int16_t>(), std::numeric_limits<int16_t>::max() );
	}
	{
		// Conversion to uint8_t (will downscale [0-255])
		data::enableLog<util::DefaultMsgPrint>( error );
		data::TypedImage<uint8_t> img2( img );
		data::enableLog<util::DefaultMsgPrint>( warning );
		BOOST_REQUIRE( img2.reIndex() );
		//Check if the metadata were copied correct
		BOOST_CHECK_EQUAL( static_cast<util::PropertyMap>( img ), static_cast<util::PropertyMap>( img2 ) );

		for ( int i = 0; i < 3; i++ )
			for ( int j = 0; j < 3; j++ ) {
				data::Chunk c1 = img.getChunk( 0, 0, j, i );
				data::Chunk c2 = img2.getChunk( 0, 0, j, i );
				BOOST_REQUIRE( c1.is<uint8_t>() ); // this was 8bit
				BOOST_REQUIRE( c2.is<uint8_t>() ); // could be kept
				// As TypedImage does cheap copy if possible this should still be the same memory
				BOOST_CHECK_EQUAL( ( void * )&c1.voxel<uint8_t>( 0, 0 ), ( void * )&c2.voxel<uint8_t>( 0, 0 ) );
			}

		for ( int i = 3; i < 10; i++ )
			for ( int j = 0; j < 3; j++ ) {
				data::Chunk c1 = img.getChunk( 0, 0, j, i );
				data::Chunk c2 = img2.getChunk( 0, 0, j, i );
				BOOST_REQUIRE( c1.is<int16_t>() ); // this was 16bit
				BOOST_REQUIRE( c2.is<uint8_t>() ); // not anymore
				// Here we had to copy/convert the data - so not the same memory anymore
				BOOST_CHECK( ( void * )&c1.voxel<int16_t>( 0, 0 ) != ( void * )&c2.voxel<uint8_t>( 0, 0 ) );
			}

		std::pair<util::ValueReference, util::ValueReference> minmax = img2.getMinMax();
		BOOST_CHECK( minmax.first->is<uint8_t>() );
		BOOST_CHECK( minmax.second->is<uint8_t>() );
		BOOST_CHECK_EQUAL( minmax.first->as<uint8_t>(), 0 );
		BOOST_CHECK_EQUAL( minmax.second->as<uint8_t>(), std::numeric_limits<uint8_t>::max() );
	}
} // END typedimage_test

BOOST_AUTO_TEST_CASE ( image_transformCoords_test )
{
	// dummy image
	std::list<data::Image> images = data::IOFactory::load( "nix.null" );
	BOOST_REQUIRE( !images.empty() );
	data::Image &img = images.front();
	//TODO rewrite this test to use BOST_UNIT_TEST_ASSERTS with the help of
	// util::fuzzyEqual
	// ************************************************************************
	// Transformation: DICOM -> Nifti
	// ************************************************************************
	boost::numeric::ublas::matrix<float> T( 3, 3 );
	T( 0, 0 ) = -1;
	T( 0, 1 ) = 0;
	T( 0, 2 ) = 0;
	T( 1, 0 ) = 0;
	T( 1, 1 ) = -1;
	T( 1, 2 ) = 0;
	T( 2, 0 ) = 0;
	T( 2, 1 ) = 0;
	T( 2, 2 ) = 1;
	// **** AXIAL ****
	// set orientation AXIAL in DCIOM space
	img.setPropertyAs( "rowVec", util::fvector4( 1, 0, 0, 0 ) );
	img.setPropertyAs( "columnVec", util::fvector4( 0, 1, 0, 0 ) );
	img.setPropertyAs( "sliceVec", util::fvector4( 0, 0, 1, 0 ) );
	// set index origin to DICOM space index origin
	img.setPropertyAs( "indexOrigin", util::fvector4( -1, -2, -3, 0 ) );
	// apply transformation
	img.transformCoords( T );
	// CHECKS
	BOOST_CHECK( img.getPropertyAs<util::fvector4>( "rowVec" ).fuzzyEqual( util::fvector4( -1, 0, 0, 0 ) ) );
	BOOST_CHECK( img.getPropertyAs<util::fvector4>( "columnVec" ).fuzzyEqual( util::fvector4( 0, -1, 0, 0 ) ) );
	BOOST_CHECK( img.getPropertyAs<util::fvector4>( "sliceVec" ).fuzzyEqual( util::fvector4( 0, 0, 1, 0 ) ) );
	BOOST_CHECK( img.getPropertyAs<util::fvector4>( "indexOrigin" ).fuzzyEqual( util::fvector4( 1, 2, -3, 0 ) ) );
	;
	// **** SAGITTAL ****
	// set orientation SAGITTAL in DCIOM space
	img.setPropertyAs( "rowVec", util::fvector4( 0, 1, 0, 0 ) );
	img.setPropertyAs( "columnVec", util::fvector4( 0, 0, 1, 0 ) );
	img.setPropertyAs( "sliceVec", util::fvector4( 1, 0, 0, 0 ) );
	// set index origin to DICOM space index origin
	img.setPropertyAs( "indexOrigin", util::fvector4( -3, -1, -2, 0 ) );
	// apply transformation
	img.transformCoords( T );
	// CHECKS
	BOOST_CHECK( img.getPropertyAs<util::fvector4>( "rowVec" ).fuzzyEqual( util::fvector4( 0, -1, 0, 0 ) ) );
	BOOST_CHECK( img.getPropertyAs<util::fvector4>( "columnVec" ).fuzzyEqual( util::fvector4( 0, 0, -1, 0 ) ) );
	BOOST_CHECK( img.getPropertyAs<util::fvector4>( "sliceVec" ).fuzzyEqual( util::fvector4( 1, 0, 0, 0 ) ) );
	BOOST_CHECK( img.getPropertyAs<util::fvector4>( "indexOrigin" ).fuzzyEqual( util::fvector4( -3, 1, 2, 0 ) ) );
	// **** CORONAL ****
	// set orientation CORONAL in DCIOM space
	img.setPropertyAs( "rowVec", util::fvector4( 1, 0, 0, 0 ) );
	img.setPropertyAs( "columnVec", util::fvector4( 0, 0, 1, 0 ) );
	img.setPropertyAs( "sliceVec", util::fvector4( 0, -1, 0, 0 ) );
	// set index origin to DICOM space index origin
	img.setPropertyAs( "indexOrigin", util::fvector4( -1, 3, -2, 0 ) );
	// apply transformation
	img.transformCoords( T );
	// CHECKS
	BOOST_CHECK( img.getPropertyAs<util::fvector4>( "rowVec" ).fuzzyEqual( util::fvector4( -1, 0, 0, 0 ) ) );
	BOOST_CHECK( img.getPropertyAs<util::fvector4>( "columnVec" ).fuzzyEqual( util::fvector4( 0, 0, -1, 0 ) ) );
	BOOST_CHECK( img.getPropertyAs<util::fvector4>( "sliceVec" ).fuzzyEqual( util::fvector4( 0, -1, 0, 0 ) ) );
	BOOST_CHECK( img.getPropertyAs<util::fvector4>( "indexOrigin" ).fuzzyEqual( util::fvector4( 1, 3, 2, 0 ) ) );
} // END transformCoords_test

BOOST_AUTO_TEST_CASE ( image_init_test_sizes_and_values )
{
	unsigned int nrX = 45;
	unsigned int nrY = 64;
	unsigned int nrS = 20;
	unsigned int nrT = 20;
	static boost::numeric::converter < uint16_t, double,
		   boost::numeric::conversion_traits<uint16_t, double>,
		   boost::numeric::def_overflow_handler,
		   boost::numeric::RoundEven<double>
		   > converter;

	std::list<data::Chunk> chunks( nrS*nrT, data::MemChunk<float>( 3, 3 ) );
	std::list<data::Chunk>::iterator k=chunks.begin();

	for ( unsigned int is = 0; is < nrS; is++ ) {
		for ( unsigned int it = 0; it < nrT; it++,k++ ) {
			k->setPropertyAs( "indexOrigin", util::fvector4( 0, 0, is ) );
			BOOST_CHECK( k->propertyValue( "indexOrigin" ).needed() );
			k->setPropertyAs( "rowVec", util::fvector4( 17, 0, 0 ) );
			k->setPropertyAs( "columnVec", util::fvector4( 0, 17, 0 ) );
			k->setPropertyAs( "sliceVec", util::fvector4( 0, 0, 31 ) );
			k->setPropertyAs( "voxelSize", util::fvector4( 3, 3, 3 ) );
			k->setPropertyAs<uint32_t>( "acquisitionNumber", is + it * nrS );
			k->setPropertyAs<uint16_t>( "sequenceNumber", 1 );
		}
	}

	data::Image img(chunks);
	BOOST_REQUIRE(img.isClean());

	srand ( time( NULL ) );
	const size_t dummy[] = {nrX, nrY, nrS, nrT};
	const util::FixedVector<size_t, 4> sizeVec( dummy );

	BOOST_REQUIRE_EQUAL( img.getChunksAsVector().size(), nrT * nrS );
	BOOST_REQUIRE_EQUAL( img.getSizeAsVector(), sizeVec );

	for ( unsigned int ix = 0; ix < nrX; ix++ ) {
		for ( unsigned int iy = 0; iy < nrY; iy++ ) {
			for ( unsigned int is = 0; is < nrS; is++ ) {
				for ( unsigned int it = 0; it < nrT; it++ ) {
					img.voxel<float>( ix, iy, is, it ) = ix + iy + is + it;
				}
			}
		}
	}

	std::pair<float, float> minmax = img.getMinMaxAs<float>();
	BOOST_REQUIRE_EQUAL( minmax.first, 0 );
	double scale = std::numeric_limits<uint16_t>::max() / minmax.second;
	data::enableLog<util::DefaultMsgPrint>( error );
	data::MemImage<uint16_t> copyImg( img );
	data::enableLog<util::DefaultMsgPrint>( warning );
	copyImg.reIndex();
	BOOST_REQUIRE_EQUAL( copyImg.getSizeAsVector(), sizeVec );

	for ( unsigned int ix = 0; ix < nrX; ix++ ) {
		for ( unsigned int iy = 0; iy < nrY; iy++ ) {
			for ( unsigned int is = 0; is < nrS; is++ ) {
				for ( unsigned int it = 0; it < nrT; it++ ) {
					BOOST_CHECK_EQUAL( converter( img.voxel<float>( ix, iy, is, it )*scale ) , copyImg.voxel<uint16_t>( ix, iy, is, it ) );
				}
			}
		}
	}

	data::MemChunk<float> chSlice( nrX, nrY );
	img.getChunk( 0, 0, 12, 8, false ).copySlice( 0, 0, chSlice, 0, 0 );
	float *pValues = ( ( boost::shared_ptr<float> ) chSlice.getValuePtr<float>() ).get();
	float *pRun = pValues;

	for ( unsigned int iy = 0; iy < nrY; iy++ ) {
		for ( unsigned int ix = 0; ix < nrX; ix++ ) {
			BOOST_CHECK_EQUAL( static_cast<float>( 20 + ix + iy ), chSlice.voxel<float>( ix, iy ) );
			BOOST_CHECK_EQUAL( static_cast<float>( 20 + ix + iy ), *pRun++ );
		}
	}
}

BOOST_AUTO_TEST_CASE ( image_splice_test )
{
	data::MemChunk<uint8_t> original( 10, 10, 10, 10 );
	original.setPropertyAs<uint32_t>( "acquisitionNumber", 1 );
	original.setPropertyAs<util::fvector4>( "indexOrigin", util::fvector4() );
	original.setPropertyAs<util::fvector4>( "voxelSize", util::fvector4( 1, 1, 1 ) );
	original.setPropertyAs<util::fvector4>( "rowVec", util::fvector4( 1, 0, 0 ) );
	original.setPropertyAs<util::fvector4>( "columnVec", util::fvector4( 0, 1, 0 ) );
	data::Image img(original);
	BOOST_REQUIRE( img.isClean() );
	BOOST_REQUIRE( img.isValid() );
	BOOST_REQUIRE( !img.isEmpty() );
	img.spliceDownTo( data::sliceDim );
	std::vector<boost::shared_ptr<data::Chunk> > chunks = img.getChunksAsVector();
	BOOST_CHECK_EQUAL( chunks.size(), 100 );

	for( size_t i = 0; i < chunks.size(); i++ ) {
		BOOST_CHECK_EQUAL( chunks[i]->getPropertyAs<int32_t>( "acquisitionNumber" ), i + 1 );
	}
}

BOOST_AUTO_TEST_CASE ( image_init_test_sizes )
{
	unsigned int nrX = 64;
	unsigned int nrY = 64;
	unsigned int nrS = 20;
	unsigned int nrT = 20;

	std::list<data::Chunk> chunks( nrS*nrT, data::MemChunk<float>( nrX, nrY ) );
	std::list<data::Chunk>::iterator k=chunks.begin();


	for ( unsigned int is = 0; is < nrS; is++ ) {
		for ( unsigned int it = 0; it < nrT; it++,k++ ) {
			k->setPropertyAs( "indexOrigin", util::fvector4( 0, 0, is ) );
			BOOST_CHECK( k->propertyValue( "indexOrigin" ).needed() );
			k->setPropertyAs( "rowVec", util::fvector4( 17, 0, 0 ) );
			k->setPropertyAs( "columnVec", util::fvector4( 0, 17, 0 ) );
			k->setPropertyAs( "sliceVec", util::fvector4( 0, 0, 31 ) );
			k->setPropertyAs( "voxelSize", util::fvector4( 3, 3, 3 ) );
			k->setPropertyAs<uint32_t>( "acquisitionNumber", is + it * nrS );
			k->setPropertyAs<uint16_t>( "sequenceNumber", 1 );
		}
	}

	const size_t dummy[] = {nrX, nrY, nrS, nrT};

	const util::FixedVector<size_t, 4> sizeVec( dummy );

	data::Image img(chunks);
	BOOST_REQUIRE(img.isClean());
	BOOST_REQUIRE_EQUAL( img.getSizeAsVector(), sizeVec );

	//***************************************************************
	nrX = 64;

	nrS = 20;

	nrT = 20;

	std::list<data::Chunk> chunks2( nrS*nrT, data::MemChunk<float>( nrX, nrY ) );
	std::list<data::Chunk>::iterator k2=chunks2.begin();

	for ( unsigned int is = 0; is < nrS; is++ ) {
		for ( unsigned int it = 0; it < nrT; it++,k2++ ) {
			k2->setPropertyAs( "indexOrigin", util::fvector4( 0, 0, is ) );
			BOOST_CHECK( k2->propertyValue( "indexOrigin" ).needed() );
			k2->setPropertyAs( "rowVec", util::fvector4( 17, 0, 0 ) );
			k2->setPropertyAs( "columnVec", util::fvector4( 0, 17, 0 ) );
			k2->setPropertyAs( "sliceVec", util::fvector4( 0, 0, 31 ) );
			k2->setPropertyAs( "voxelSize", util::fvector4( 3, 3, 3 ) );
			k2->setPropertyAs<uint32_t>( "acquisitionNumber", is + it * nrS );
			k2->setPropertyAs<uint16_t>( "sequenceNumber", 1 );
		}
	}

	data::Image img2(chunks2);
	BOOST_REQUIRE(img2.isClean());

	const size_t dummy2[] = {nrX, nrS, 1, nrT};

	const util::FixedVector<size_t, 4> sizeVec2( dummy2 );


	BOOST_REQUIRE_EQUAL( img2.getSizeAsVector(), sizeVec2 );

	//***************************************************************
	nrX = 1;

	nrY = 64;

	nrS = 20;

	nrT = 20;

	std::list<data::Chunk> chunks3( nrS*nrT, data::MemChunk<float>( nrX, nrY ) );
	std::list<data::Chunk>::iterator k3=chunks3.begin();

	for ( unsigned int is = 0; is < nrS; is++ ) {
		for ( unsigned int it = 0; it < nrT; it++,k3++ ) {
			k3->setPropertyAs( "indexOrigin", util::fvector4( 0, 0, is ) );
			BOOST_CHECK( k3->propertyValue( "indexOrigin" ).needed() );
			k3->setPropertyAs( "rowVec", util::fvector4( 17, 0, 0 ) );
			k3->setPropertyAs( "columnVec", util::fvector4( 0, 17, 0 ) );
			k3->setPropertyAs( "sliceVec", util::fvector4( 0, 0, 31 ) );
			k3->setPropertyAs( "voxelSize", util::fvector4( 3, 3, 3 ) );
			k3->setPropertyAs<uint32_t>( "acquisitionNumber", is + it * nrS );
			k3->setPropertyAs<uint16_t>( "sequenceNumber", 1 );
		}
	}

	data::Image img3(chunks3);
	BOOST_REQUIRE(img3.isClean());

	const size_t dummy3[] = {nrX, nrY, nrS, nrT};

	const util::FixedVector<size_t, 4> sizeVec3( dummy3 );

	BOOST_REQUIRE_EQUAL( img3.getSizeAsVector(), sizeVec3 );

	//***************************************************************
	nrX = 64;

	nrY = 64;

	nrS = 1;

	nrT = 20;

	std::list<data::Chunk> chunks4( nrS*nrT, data::MemChunk<float>( nrX, nrY ) );
	std::list<data::Chunk>::iterator k4=chunks4.begin();

	for ( unsigned int is = 0; is < nrS; is++ ) {
		for ( unsigned int it = 0; it < nrT; it++,k4++ ) {
			k4->setPropertyAs( "indexOrigin", util::fvector4( 0, 0, is ) );
			BOOST_CHECK( k4->propertyValue( "indexOrigin" ).needed() );
			k4->setPropertyAs( "rowVec", util::fvector4( 17, 0, 0 ) );
			k4->setPropertyAs( "columnVec", util::fvector4( 0, 17, 0 ) );
			k4->setPropertyAs( "sliceVec", util::fvector4( 0, 0, 31 ) );
			k4->setPropertyAs( "voxelSize", util::fvector4( 3, 3, 3 ) );
			k4->setPropertyAs<uint32_t>( "acquisitionNumber", is + it * nrS );
			k4->setPropertyAs<uint16_t>( "sequenceNumber", 1 );
		}
	}

	data::Image img4(chunks4);
	BOOST_REQUIRE(img4.isClean());

	const size_t dummy4[] = {nrX, nrY, nrS, nrT};

	const util::FixedVector<size_t, 4> sizeVec4( dummy4 );

	BOOST_REQUIRE_EQUAL( img4.getSizeAsVector(), sizeVec4 );

	//***************************************************************
	nrX = 64;

	nrY = 64;

	nrS = 20;

	nrT = 1;

	std::list<data::Chunk> chunks5( nrS*nrT, data::MemChunk<float>( nrX, nrY ) );
	std::list<data::Chunk>::iterator k5=chunks5.begin();


	for ( unsigned int is = 0; is < nrS; is++ ) {
		for ( unsigned int it = 0; it < nrT; it++,k5++ ) {
			k5->setPropertyAs( "indexOrigin", util::fvector4( 0, 0, is ) );
			BOOST_CHECK( k5->propertyValue( "indexOrigin" ).needed() );
			k5->setPropertyAs( "rowVec", util::fvector4( 17, 0, 0 ) );
			k5->setPropertyAs( "columnVec", util::fvector4( 0, 17, 0 ) );
			k5->setPropertyAs( "sliceVec", util::fvector4( 0, 0, 31 ) );
			k5->setPropertyAs( "voxelSize", util::fvector4( 3, 3, 3 ) );
			k5->setPropertyAs<uint32_t>( "acquisitionNumber", is + it * nrS );
			k5->setPropertyAs<uint16_t>( "sequenceNumber", 1 );
		}
	}

	const size_t dummy5[] = {nrX, nrY, nrS, nrT};

	const util::FixedVector<size_t, 4> sizeVec5( dummy5 );

	data::Image img5(chunks5);
	BOOST_REQUIRE(img5.isClean());


	BOOST_REQUIRE_EQUAL( img5.getSizeAsVector(), sizeVec5 );

	//***************************************************************
	nrX = 0;

	nrY = 0;

	nrS = 0;

	nrT = 0;

	std::list<data::Chunk> empty;
	data::Image img6(empty);

	const size_t dummy6[] = {nrX, nrY, nrS, nrT};

	const util::FixedVector<size_t, 4> sizeVec6( dummy6 );

	data::enableLog<util::DefaultMsgPrint>( error );

	BOOST_REQUIRE( !img6.isClean() ); //reIndex on an empty image shall fail (size will be undefined)
	BOOST_REQUIRE( !img6.isValid() ); //reIndex on an empty image shall fail (size will be undefined)

	data::enableLog<util::DefaultMsgPrint>( warning );
}

BOOST_AUTO_TEST_CASE ( image_size_test )
{
	data::MemChunk<uint8_t> original( 11, 23, 90, 12 );
	original.setPropertyAs<uint32_t>( "acquisitionNumber", 1 );
	original.setPropertyAs<util::fvector4>( "indexOrigin", util::fvector4() );
	original.setPropertyAs<util::fvector4>( "voxelSize", util::fvector4( 1, 1, 1 ) );
	original.setPropertyAs<util::fvector4>( "rowVec", util::fvector4( 1, 0, 0 ) );
	original.setPropertyAs<util::fvector4>( "columnVec", util::fvector4( 0, 1, 0 ) );
	data::Image img(original);
	BOOST_REQUIRE( img.isClean() );
	BOOST_REQUIRE( img.isValid() );
	BOOST_REQUIRE( !img.isEmpty() );

	BOOST_CHECK_EQUAL( img.getNrOfColumms(), 11 );
	BOOST_CHECK_EQUAL( img.getNrOfRows(), 23 );
	BOOST_CHECK_EQUAL( img.getNrOfSlices(), 90 );
	BOOST_CHECK_EQUAL( img.getNrOfTimesteps(), 12 );

}


} // END namespace test
} // END namespace isis
