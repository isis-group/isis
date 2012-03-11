/*
 * imageTest.cpp
 *
 *  Created on: Oct 1, 2009
 *      Author: proeger
 */

#define BOOST_TEST_MODULE ImageTest
#define NOMINMAX 1
#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/foreach.hpp>
#include <DataStorage/image.hpp>
#include <DataStorage/io_factory.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

namespace isis
{
namespace test
{

template<typename T> data::Chunk genSlice( size_t columns = 4, size_t rows = 4, size_t at = 0, uint32_t acnum = 0 )
{
	data::MemChunk<T> ch( columns, rows );
	ch.setPropertyAs( "indexOrigin", util::fvector4( 0, 0, at ) );
	ch.setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
	ch.setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
	ch.setPropertyAs( "sliceVec", util::fvector4( 0, 0, 1 ) );
	ch.setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );

	ch.setPropertyAs( "acquisitionNumber", ( uint32_t )acnum );
	ch.setPropertyAs( "acquisitionTime", ( float )acnum );
	return ch;
}

// create an image
BOOST_AUTO_TEST_CASE ( image_init_test )
{
	{
		data::Chunk ch = genSlice<float>( 4, 4, 2, 0 );
		// inserting a proper Chunk should work
		data::Image img( ch );
		BOOST_CHECK( img.isClean() );


		// inserting insufficient Chunk should fail
		data::enableLog<util::DefaultMsgPrint>( ( LogLevel )0 );
		BOOST_CHECK( ! img.insertChunk( data::MemChunk<float>( 4, 4 ) ) );
		data::enableLog<util::DefaultMsgPrint>( error );

		//inserting the same chunk twice should fail
		BOOST_CHECK( ! img.insertChunk( ch ) );

		// but inserting another Chunk should work
		ch = genSlice<float>( 4, 4, 0, 2 );
		img.insertChunk( ch );

		// Chunks should be inserted based on their position (lowest first)
		ch = genSlice<float>( 4, 4, 1, 1 );
		BOOST_REQUIRE( img.insertChunk( ch ) );
		//Get a list of the sorted chunks
		BOOST_REQUIRE( img.reIndex() );
		std::vector<data::Chunk > list = img.copyChunksToVector( false );
		BOOST_CHECK_EQUAL( list.size(), 3 ); // the should be 3 chunks in the list by now

		for( unsigned int i = 0; i < list.size(); i++ ) {
			BOOST_CHECK_EQUAL( list[i].propertyValue( "indexOrigin" ), util::fvector4( 0, 0, i, 0 ) );
			BOOST_CHECK_EQUAL( list[i].propertyValue( "acquisitionNumber" ), 2 - i ); // AcqNumber and time are in the oposite direction
			BOOST_CHECK_EQUAL( list[i].propertyValue( "acquisitionTime" ), 2 - i );
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
			chunks.push_back( genSlice<float>( 4, 4, i, 3 + i ) );
		}

		data::Image img( chunks );
		BOOST_CHECK( img.isClean() );
		BOOST_CHECK( img.isValid() );

		std::string str = "testString";
		img.setPropertyAs<std::string>( "testProp", str );
		BOOST_CHECK_EQUAL( img.getPropertyAs<std::string>( "testProp" ), str );
		data::Chunk ch = img.copyChunksToVector( false ).back();
		//as all other chunks where timestep < 4 this must be at the end
		BOOST_CHECK_EQUAL( ch.propertyValue( "indexOrigin" ), util::fvector4( 0, 0, 2 ) );
		BOOST_CHECK_EQUAL( ch.propertyValue( "acquisitionNumber" ), 5  );

		// Check all dimensions
		uint32_t nrRows = 12;
		uint32_t nrCols = 32;
		uint32_t nrTimesteps = 17;
		uint32_t nrSlices = 27;

		chunks.clear();

		for( unsigned int t = 0; t < nrTimesteps; t++ ) {
			for( unsigned int s = 0; s < nrSlices; s++ ) {
				chunks.push_back( genSlice<float>( nrCols, nrRows, s, s + t * nrSlices ) );
			}
		}

		data::Image img2( chunks );

		BOOST_REQUIRE( img2.isClean() );
		BOOST_CHECK_EQUAL( img2.getVolume(), nrRows * nrCols * nrTimesteps * nrSlices );
		BOOST_CHECK_EQUAL( img2.getSizeAsVector(), util::ivector4( nrCols, nrRows, nrSlices, nrTimesteps ) );

		// Check all dimensions with limit sizes
		nrRows = 212;
		nrCols = 2;
		nrTimesteps = 2;
		nrSlices = 1;
		chunks.clear();

		for( unsigned int t = 0; t < nrTimesteps; t++ ) {
			for( unsigned int s = 0; s < nrSlices; s++ ) {
				chunks.push_back( genSlice<float>( nrCols, nrRows, s, s + t * nrSlices ) );
			}
		}

		data::Image img3( chunks );
		BOOST_REQUIRE( img3.isClean() );
		BOOST_CHECK_EQUAL( img3.getVolume(), nrRows * nrCols * nrTimesteps * nrSlices );
		BOOST_CHECK_EQUAL( img3.getSizeAsVector(), util::ivector4( nrCols, nrRows, nrSlices, nrTimesteps ) );



		nrRows = 54;
		nrCols = 29;
		nrTimesteps = 1;
		nrSlices = 21;
		chunks.clear();

		for( unsigned int t = 0; t < nrTimesteps; t++ ) {
			for( unsigned int s = 0; s < nrSlices; s++ ) {
				chunks.push_back( genSlice<float>( nrCols, nrRows, s, s + t * nrSlices ) );
			}
		}

		data::Image img4( chunks );

		BOOST_REQUIRE( img4.isClean() );
		BOOST_CHECK_EQUAL( img4.getVolume(), nrRows * nrCols * nrTimesteps * nrSlices );
		BOOST_CHECK_EQUAL( img4.getSizeAsVector(), util::ivector4( nrCols, nrRows, nrSlices, nrTimesteps ) );
	}
}

BOOST_AUTO_TEST_CASE ( minimal_image_test )
{
	data::Chunk ch = genSlice<float>( 4, 4, 2 ); //create chunk at 2 with acquisitionNumber 0
	std::list<data::MemChunk<float> > chunks( 2, ch ); //make a list with two copies of that
	chunks.back().setPropertyAs<uint32_t>( "acquisitionNumber", 1 ); //change the acquisitionNumber of that to 1
	chunks.back().setPropertyAs<float>( "acquisitionTime", 1 );

	data::Image img( chunks );
	const size_t size[] = {4, 4, 1, 2};
	BOOST_CHECK( img.isClean() );
	BOOST_CHECK( img.isValid() );
	BOOST_CHECK_EQUAL( img.getSizeAsVector(), ( util::vector4<size_t>( size ) ) );
}

BOOST_AUTO_TEST_CASE ( copyChunksToVector_test )
{
	data::Chunk ch = genSlice<float>( 4, 4, 2 ); //create chunk at 2 with acquisitionNumber 0
	std::list<data::MemChunk<float> > chunks( 2, ch ); //make a list with two copies of that
	chunks.back().setPropertyAs<uint32_t>( "acquisitionNumber", 1 ); //change the acquisitionNumber of that to 1
	chunks.back().setPropertyAs<float>( "acquisitionTime", 1 );

	data::Image img( chunks );

	// this is shared
	std::vector< data::Chunk > cheapchunks = img.copyChunksToVector();
	cheapchunks[0].voxel<float>( 0 ) = M_PI;
	BOOST_CHECK_EQUAL( img.voxel<float>( 0 ), ( float )M_PI ); //so image will be changed

	// this is not
	std::vector< data::MemChunk<float> > memchunks( cheapchunks.begin(), cheapchunks.end() );
	memchunks[0].voxel<float>( 0 ) = M_PI_2;
	BOOST_CHECK_EQUAL( memchunks[0].voxel<float>( 0 ), ( float )M_PI_2 ); //so image will _not_ be changed
	BOOST_CHECK_EQUAL( img.voxel<float>( 0 ), ( float )M_PI ); //so image will _not_ be changed
}

BOOST_AUTO_TEST_CASE ( proplist_image_test )
{
	data::MemChunk<uint8_t> ch( 4, 4, 4 ); //create a volume of size 4x4x4

	ch.setPropertyAs( "indexOrigin", util::fvector4( 0, 0, 0 ) );
	ch.setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
	ch.setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
	ch.setPropertyAs( "sliceVec", util::fvector4( 0, 0, 1 ) );
	ch.setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );

	for( int i = 0; i < 4; i++ ) {
		ch.propertyValueAt( "acquisitionNumber", 3 - i ) = ( uint32_t )i; //change the acquisitionNumber of that to 1
		ch.propertyValueAt( "acquisitionTime", 3 - i ) = ( uint32_t )i;
	}

	data::Image img( ch );
	BOOST_REQUIRE_EQUAL( img.getChunk( 0 ).getRelevantDims(), 2 ); // the dim should be 2 now

	for( uint32_t i = 0; i < 4; i++ ) {
		BOOST_CHECK_EQUAL( img.getChunk( 0, 0, 3 - i ).propertyValue( "acquisitionTime" ), i );
		BOOST_CHECK_EQUAL( img.getChunk( 0, 0, 3 - i ).propertyValue( "acquisitionNumber" ), i );
	}
}

BOOST_AUTO_TEST_CASE ( minindexdim_test )
{
	std::list<data::Chunk> chunks1;

	for( int i = 0; i < 3; i++ ) {
		chunks1.push_back( genSlice<float>( 4, 4, i ) );
	}

	std::list<data::Chunk> chunks2( chunks1 );

	data::Image img1( chunks1 );
	data::Image img2( chunks2, data::timeDim );

	const size_t size1[] = {4, 4, 3, 1};
	const size_t size2[] = {4, 4, 1, 3};

	BOOST_REQUIRE( img1.isClean() );
	BOOST_REQUIRE( img1.isValid() );
	BOOST_REQUIRE( img2.isClean() );
	BOOST_REQUIRE( img2.isValid() );

	BOOST_CHECK_EQUAL( img1.getSizeAsVector(), ( util::vector4<size_t>( size1 ) ) );
	BOOST_CHECK_EQUAL( img2.getSizeAsVector(), ( util::vector4<size_t>( size2 ) ) );
}


BOOST_AUTO_TEST_CASE ( type_selection_test )
{
	float org = 0;
	std::list<data::Chunk> chunks;

	chunks.push_back( genSlice<int16_t>( 4, 4, org, org ) );
	++org;
	chunks.back().voxel<int16_t>( 0, 0, 0 ) = std::numeric_limits<int16_t>::min();

	chunks.push_back( genSlice<int8_t>( 4, 4, org, org ) );
	++org;
	chunks.back().voxel<int8_t>( 0, 0, 0 ) = std::numeric_limits<int8_t>::min();

	chunks.push_back( genSlice<uint8_t>( 4, 4, org, org ) );
	++org;
	chunks.back().voxel<uint8_t>( 0, 0, 0 ) = std::numeric_limits<uint8_t>::max();

	chunks.push_back( genSlice<uint16_t>( 4, 4, org, org ) );
	chunks.back().voxel<uint16_t>( 0, 0, 0 ) = std::numeric_limits<int16_t>::max(); // the maximum shall fit into int16_t

	data::Image img( chunks );
	const size_t size[] = {4, 4, 4, 1};
	BOOST_CHECK( img.isClean() );
	BOOST_CHECK( img.isValid() );
	BOOST_CHECK_EQUAL( img.copyChunksToVector( false ).size(), 4 );
	BOOST_CHECK_EQUAL( img.getSizeAsVector(), ( util::vector4<size_t>( size ) ) );
	BOOST_CHECK_EQUAL( img.getMajorTypeID(), data::ValueArray<int16_t>( NULL, 0 ).getTypeID() );
}

BOOST_AUTO_TEST_CASE ( type_scale_test )
{
	float org = 0;
	std::list<data::Chunk> chunks;

	chunks.push_back( genSlice<int16_t>( 4, 4, org, org ) );
	++org;
	chunks.back().voxel<int16_t>( 0, 0, 0 ) = -50;

	chunks.push_back( genSlice<int8_t>( 4, 4, org, org ) );
	++org;
	chunks.back().voxel<int8_t>( 0, 0, 0 ) = -1;

	chunks.push_back( genSlice<uint8_t>( 4, 4, org, org ) );
	++org;
	chunks.back().voxel<uint8_t>( 0, 0, 0 ) = 1;

	chunks.push_back( genSlice<uint16_t>( 4, 4, org, org ) );
	++org;
	chunks.back().voxel<uint16_t>( 0, 0, 0 ) = 2500;

	data::Image img( chunks );
	BOOST_CHECK( img.isClean() );
	BOOST_CHECK( img.isValid() );

	data::scaling_pair scale = img.getScalingTo( data::ValueArray<uint8_t>::staticID );
	BOOST_CHECK_EQUAL( scale.first->as<double>(), 1. / 10 );
	BOOST_CHECK_EQUAL( scale.second->as<double>(), 5 );
}

BOOST_AUTO_TEST_CASE ( image_chunk_test )
{
	std::list<data::Chunk> chunks;

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ ) {
			chunks.push_back( genSlice<float>( 3, 3, j, j + i * 3 ) );
			chunks.back().voxel<float>( j, j ) = 42;
		}

	data::Image img( chunks );
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
	std::list<data::Chunk> chunks;

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ ) {
			chunks.push_back( genSlice<uint8_t>( 3, 3, j, j + i * 3 ) );
			chunks.back().voxel<uint8_t>( j, j ) = 42;
		}

	data::Image img( chunks );

	class : public data::ChunkOp
	{
		class : public data::VoxelOp<uint8_t>
		{
		public:
			bool operator()( uint8_t &vox, const util::vector4<size_t>& /*pos*/ ) {
				vox = 42;
				return true;
			}
		} vox42;
	public:
		bool operator()( data::Chunk &ch, util::vector4<size_t> /*posInImage*/ ) {
			return ch.foreachVoxel( vox42 ) == 0;
		}
	} set42;

	class setIdx: public data::VoxelOp<uint8_t>
	{
		data::_internal::NDimensional<4> geometry;
	public:
		setIdx( data::_internal::NDimensional<4> geo ): geometry( geo ) {}
		bool operator()( uint8_t &vox, const util::vector4<size_t>& pos ) {
			vox = geometry.getLinearIndex( &pos[0] );
			return true;
		}
	};

	BOOST_REQUIRE_EQUAL( img.foreachChunk( set42 ), 0 );

	util::vector4<size_t> imgSize = img.getSizeAsVector();

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
	std::list<data::Chunk> chunks;

	for( int i = 0; i < 3; i++ )
		chunks.push_back( genSlice<float>( 3, 3, i, i ) );

	std::list<data::Chunk>::iterator k = chunks.begin();
	( k++ )->voxel<float>( 0, 0 ) = 42.0;
	( k++ )->voxel<float>( 1, 1 ) = 42.0;
	( k++ )->voxel<float>( 2, 2 ) = 42;

	data::Image img( chunks );
	BOOST_REQUIRE( img.isClean() );
	BOOST_CHECK( img.isValid() );


	for ( int i = 0; i < 3; i++ ) {
		BOOST_CHECK( img.voxel<float>( i, i, i, 0 ) == 42 );
	}

	/// check for setting voxel data
	img.voxel<float>( 2, 2, 2, 0 ) = 23;
	BOOST_CHECK( img.voxel<float>( 2, 2, 2, 0 ) == 23 );
}

BOOST_AUTO_TEST_CASE ( image_const_iterator_test )
{
	//  get a voxel from inside and outside the image
	std::list<data::Chunk> chunks;

	for( int i = 0; i < 3; i++ )
		chunks.push_back( genSlice<float>( 3, 3, i, i ) );

	std::list<data::Chunk>::iterator k = chunks.begin();
	( k++ )->voxel<float>( 0, 0 ) = 42.0;
	( k++ )->voxel<float>( 1, 1 ) = 42.0;
	( k++ )->voxel<float>( 2, 2 ) = 42;

	const data::Image img( chunks );

	std::list<data::Chunk> empty;
	BOOST_REQUIRE( img.isClean() );
	BOOST_CHECK( img.isValid() );

	const data::Image::const_iterator start = img.begin();
	const data::Image::const_iterator end = img.end();
	data::Image::const_iterator i = start;

	BOOST_CHECK( start < end );
	BOOST_CHECK_EQUAL( std::distance( start, end ), img.getVolume() );
	BOOST_CHECK_EQUAL( std::distance( end, start ), -img.getVolume() );

	BOOST_CHECK( start + img.getVolume() == end );

	BOOST_CHECK_EQUAL( *i, util::Value<int>( 42 ) ); // first voxel should be 42
	BOOST_CHECK_EQUAL( *( ++i ), util::Value<int>( 0 ) ); // but the second should be 0

	i += img.getLinearIndex( util::vector4<size_t>( 1, 1, 1 ) );
	BOOST_CHECK_EQUAL( *i, util::Value<int>( 0 ) ); // we missed the next 42 just by one voxel
	BOOST_CHECK_EQUAL( *( --i ), util::Value<int>( 42 ) ); // here it is

	BOOST_CHECK_EQUAL( std::distance( start, i ), img.getLinearIndex( util::vector4<size_t>( 1, 1, 1 ) ) ); //we should be exactly at the position of the second 42 now

	// @todo does not work yet
	//  const data::Image invalid(empty);
	//
	//  BOOST_REQUIRE( !invalid.isClean() );
	//  BOOST_CHECK( invalid.begin() == invalid.end() ); // in an empty image begin should be equal to end

	// this must not compile
	//(*i)=util::Value<int>(23);
}

BOOST_AUTO_TEST_CASE ( typed_image_const_iterator_test )
{
	//  get a voxel from inside and outside the image
	std::list<data::Chunk> chunks;

	for( int i = 0; i < 3; i++ )
		chunks.push_back( genSlice<float>( 3, 3, i, i ) );

	std::list<data::Chunk>::iterator k = chunks.begin();
	( k++ )->voxel<float>( 0, 0 ) = 42.0;
	( k++ )->voxel<float>( 1, 1 ) = 42.0;
	( k++ )->voxel<float>( 2, 2 ) = 42;

	const data::TypedImage<float> img = data::Image( chunks );

	std::list<data::Chunk> empty;
	BOOST_REQUIRE( img.isClean() );
	BOOST_CHECK( img.isValid() );

	data::TypedImage< float >::const_iterator start = img.begin();
	data::TypedImage< float >::const_iterator end = img.end();
	data::TypedImage< float >::const_iterator i = start;

	BOOST_CHECK( start < end );
	BOOST_CHECK_EQUAL( std::distance( start, end ), img.getVolume() );
	BOOST_CHECK_EQUAL( std::distance( end, start ), -img.getVolume() );

	BOOST_CHECK( start + img.getVolume() == end );

	BOOST_CHECK_EQUAL( *i, 42 ); // first voxel should be 42
	BOOST_CHECK_EQUAL( *( ++i ), 0 ); // but the second should be 0

	i += img.getLinearIndex( util::vector4<size_t>( 1, 1, 1 ) );
	BOOST_CHECK_EQUAL( *i, 0 ); // we missed the next 42 just by one voxel
	BOOST_CHECK_EQUAL( *( --i ), 42 ); // here it is

	BOOST_CHECK_EQUAL( std::distance( start, i ), img.getLinearIndex( util::vector4<size_t>( 1, 1, 1 ) ) ); //we should be exactly at the position of the second 42 now

	// @todo does not work yet
	//  const data::Image invalid(empty);
	//
	//  BOOST_REQUIRE( !invalid.isClean() );
	//  BOOST_CHECK( invalid.begin() == invalid.end() ); // in an empty image begin should be equal to end

	// this must not compile
	//(*i)=util::Value<int>(23);
}

BOOST_AUTO_TEST_CASE ( image_iterator_test )
{
	//  get a voxel from inside and outside the image
	std::list<data::Chunk> chunks;

	for( int i = 0; i < 3; i++ )
		chunks.push_back( genSlice<float>( 3, 3, i, i ) );

	std::list<data::Chunk>::iterator k = chunks.begin();
	( k++ )->voxel<float>( 0, 0 ) = 42.0;
	( k++ )->voxel<float>( 1, 1 ) = 42.0;
	( k++ )->voxel<float>( 2, 2 ) = 42;

	data::Image img( chunks );
	BOOST_REQUIRE( img.isClean() );
	BOOST_CHECK( img.isValid() );

	const data::Image::iterator start = img.begin();
	const data::Image::iterator end = img.end();
	data::Image::iterator i = start;
	data::Image::const_iterator j = i;

	BOOST_CHECK_EQUAL( std::distance( start, end ), img.getVolume() );
	BOOST_CHECK_EQUAL( std::distance( end, start ), -img.getVolume() );

	BOOST_CHECK( start + img.getVolume() == end );

	BOOST_CHECK_EQUAL( *i, util::Value<int>( 42 ) ); // first voxel should be 42
	BOOST_CHECK_EQUAL( *( ++i ), util::Value<int>( 0 ) ); // but the second should be 0

	i += img.getLinearIndex( util::vector4<size_t>( 1, 1, 1 ) );
	BOOST_CHECK_EQUAL( *i, util::Value<int>( 0 ) ); // we missed the next 42 just by one voxel
	BOOST_CHECK_EQUAL( *( --i ), util::Value<int>( 42 ) ); // here it is

	( *i ) = util::Value<int>( 23 );
	BOOST_CHECK_EQUAL( *i, util::Value<int>( 23 ) ); // not anymore

	BOOST_CHECK_EQUAL( std::distance( start, i ), img.getLinearIndex( util::vector4<size_t>( 1, 1, 1 ) ) ); //we should be exactly at the position of the second 42 now
}

BOOST_AUTO_TEST_CASE ( typed_image_iterator_test )
{
	//  get a voxel from inside and outside the image
	std::list<data::Chunk> chunks;

	for( int i = 0; i < 3; i++ )
		chunks.push_back( genSlice<float>( 3, 3, i, i ) );

	std::list<data::Chunk>::iterator k = chunks.begin();
	( k++ )->voxel<float>( 0, 0 ) = 42.0;
	( k++ )->voxel<float>( 1, 1 ) = 42.0;
	( k++ )->voxel<float>( 2, 2 ) = 42;

	data::TypedImage<float> img = data::Image( chunks );
	BOOST_REQUIRE( img.isClean() );
	BOOST_CHECK( img.isValid() );

	const data::TypedImage<float>::iterator start = img.begin();
	const data::TypedImage<float>::iterator end = img.end();
	data::TypedImage<float>::iterator i = start;
	data::TypedImage<float>::const_iterator j = i;

	BOOST_CHECK_EQUAL( std::distance( start, end ), img.getVolume() );
	BOOST_CHECK_EQUAL( std::distance( end, start ), -img.getVolume() );

	BOOST_CHECK( start + img.getVolume() == end );

	BOOST_CHECK_EQUAL( *i, 42 ); // first voxel should be 42
	BOOST_CHECK_EQUAL( *( ++i ), 0 ); // but the second should be 0

	i += img.getLinearIndex( util::vector4<size_t>( 1, 1, 1 ) );
	BOOST_CHECK_EQUAL( *i, 0 ); // we missed the next 42 just by one voxel
	BOOST_CHECK_EQUAL( *( --i ), 42 ); // here it is

	( *i ) = 23;
	BOOST_CHECK_EQUAL( *i, 23 ); // not anymore

	BOOST_CHECK_EQUAL( std::distance( start, i ), img.getLinearIndex( util::vector4<size_t>( 1, 1, 1 ) ) ); //we should be exactly at the position of the second 42 now
}

BOOST_AUTO_TEST_CASE ( image_voxel_value_test )
{
	//  get a voxel from inside and outside the image
	std::list<data::Chunk> chunks;

	for( int i = 0; i < 3; i++ )
		chunks.push_back( genSlice<float>( 3, 3, i, i ) );

	std::list<data::Chunk>::iterator k = chunks.begin();
	( k++ )->voxel<float>( 0, 0 ) = 42.0;
	( k++ )->voxel<float>( 1, 1 ) = 42.0;
	( k++ )->voxel<float>( 2, 2 ) = 42;

	data::Image img( chunks );
	BOOST_REQUIRE( img.isClean() );
	BOOST_CHECK( img.isValid() );


	for ( int i = 0; i < 3; i++ ) {
		BOOST_CHECK_EQUAL( img.getVoxelValue( i, i, i )->as<int>(), 42 );
		img.setVoxelValue( util::Value<int>( 23 ), i, i, i );
	}

	/// check for setting voxel data
	for ( int i = 0; i < 3; i++ ) {
		BOOST_CHECK_EQUAL( img.getVoxelValue( i, i, i )->as<int>(), 23 );
	}
}

BOOST_AUTO_TEST_CASE( image_minmax_test )
{
	std::list<data::Chunk> chunks;

	for( int i = 0; i < 3; i++ )
		for( int j = 0; j < 3; j++ ) {
			chunks.push_back( genSlice<float>( 3, 3, j, i * 3 + j ) );
			chunks.back().voxel<float>( j, j ) = i * j;
		}

	data::Image img( chunks );
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

	data::Image img( ch );
	BOOST_REQUIRE( img.isClean() );
	BOOST_REQUIRE( img.isValid() );

	BOOST_CHECK_EQUAL( img.getMainOrientation(), data::Image::axial );
}

BOOST_AUTO_TEST_CASE( memimage_test )
{
	std::list<data::Chunk> chunks;

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ ) {
			chunks.push_back( genSlice<float>( 3, 3, j, j + i * 3 ) );
			chunks.back().voxel<float>( j, j ) = i * j * 1000;
		}

	data::Image img( chunks );
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
		data::MemImage<uint8_t> img2( img );
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

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ ) {
			chunks.push_back( genSlice<uint8_t>( 3, 3, j, j + i * 3 ) );
			chunks.back().voxel<uint8_t>( j, j ) = std::numeric_limits<uint8_t>::max();
		}

	for ( int i = 3; i < 10; i++ )
		for ( int j = 0; j < 3; j++ ) {
			chunks.push_back( genSlice<int16_t>( 3, 3, j, j + i * 3 ) );
			chunks.back().voxel<int16_t>( j, j ) = std::numeric_limits<int16_t>::max();
		}

	data::Image img( chunks );
	BOOST_REQUIRE( img.isClean() );
	BOOST_REQUIRE( chunks.empty() );
	{
		std::pair<util::ValueReference, util::ValueReference> minmax = img.getMinMax();
		BOOST_CHECK( minmax.first->is<uint8_t>() );
		BOOST_CHECK( minmax.second->is<int16_t>() );
		BOOST_CHECK_EQUAL( minmax.first->as<int16_t>(), 0 );
		BOOST_CHECK_EQUAL( minmax.second->as<int16_t>(), std::numeric_limits<int16_t>::max() );
	}
	{
		// Conversion to uint8_t (will downscale [0-255])
		data::TypedImage<uint8_t> img2( img );
		BOOST_REQUIRE( img2.reIndex() );
		//Check if the metadata were copied correct
		BOOST_CHECK_EQUAL( static_cast<util::PropertyMap>( img ), static_cast<util::PropertyMap>( img2 ) );

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

	std::list<data::Chunk> chunks;

	for ( unsigned int is = 0; is < nrS; is++ ) {
		for ( unsigned int it = 0; it < nrT; it++ ) {
			chunks.push_back( genSlice<float>( nrX, nrY, is, is + it * nrS ) );
			BOOST_CHECK( chunks.back().propertyValue( "indexOrigin" ).needed() );
			chunks.back().setPropertyAs( "rowVec", util::fvector4( 17, 0, 0 ) );
			chunks.back().setPropertyAs( "columnVec", util::fvector4( 0, 17, 0 ) );
			chunks.back().setPropertyAs( "sliceVec", util::fvector4( 0, 0, 31 ) );
			chunks.back().setPropertyAs( "voxelSize", util::fvector4( 3, 3, 3 ) );
			chunks.back().setPropertyAs<uint16_t>( "sequenceNumber", 1 );
		}
	}

	data::Image img( chunks );
	BOOST_REQUIRE( img.isClean() );

	srand ( time( NULL ) );
	const size_t dummy[] = {nrX, nrY, nrS, nrT};
	const util::vector4<size_t> sizeVec( dummy );

	BOOST_REQUIRE_EQUAL( img.copyChunksToVector( false ).size(), nrT * nrS );
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
	data::MemImage<uint16_t> copyImg( img );
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


	const data::Chunk cpSource = img.getChunk( 0, 0, 12, 8, false );
	const size_t S1[] = {0, 0, 0, 0};
	const size_t S2[] = {cpSource.getSizeAsVector()[0] - 1, cpSource.getSizeAsVector()[1] - 1, 0, 0};
	const size_t D[] = {0, 0, 0, 0};

	cpSource.copyRange( S1, S2, chSlice, D );
	float *pValues = ( ( boost::shared_ptr<float> ) chSlice.getValueArray<float>() ).get();
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
	data::Image img( original );
	BOOST_REQUIRE( img.isClean() );
	BOOST_REQUIRE( img.isValid() );
	BOOST_REQUIRE( !img.isEmpty() );

	img.spliceDownTo( data::sliceDim );
	std::vector<data::Chunk > chunks = img.copyChunksToVector( false );
	BOOST_CHECK_EQUAL( chunks.size(), 100 );

	for( size_t i = 0; i < chunks.size(); i++ ) {
		BOOST_CHECK_EQUAL( chunks[i].getPropertyAs<int32_t>( "acquisitionNumber" ), i + 1 );
		BOOST_CHECK_EQUAL( chunks[i].getPropertyAs<util::fvector4>( "indexOrigin" ), util::fvector4( 0, 0, i % 10, 0 ) );
	}
}

BOOST_AUTO_TEST_CASE ( image_init_test_sizes )
{
	unsigned int nrX = 64;
	unsigned int nrY = 64;
	unsigned int nrS = 20;
	unsigned int nrT = 20;

	std::list<data::Chunk> chunks;

	for ( unsigned int is = 0; is < nrS; is++ ) {
		for ( unsigned int it = 0; it < nrT; it++ ) {
			chunks.push_back( genSlice<float>( nrX, nrY, is, is + it * nrS ) );
		}
	}

	const size_t dummy[] = {nrX, nrY, nrS, nrT};

	const util::vector4<size_t> sizeVec( dummy );

	data::Image img( chunks );

	BOOST_REQUIRE( img.isClean() );

	BOOST_REQUIRE_EQUAL( img.getSizeAsVector(), sizeVec );

	//***************************************************************
	nrX = 64;

	nrS = 20;

	nrT = 20;

	std::list<data::Chunk> chunks2;

	for ( unsigned int is = 0; is < nrS; is++ ) {
		for ( unsigned int it = 0; it < nrT; it++ ) {
			chunks2.push_back( genSlice<float>( nrX, 1, is, is + it * nrS ) );
		}
	}

	data::Image img2( chunks2 );
	BOOST_REQUIRE( img2.isClean() );

	const size_t dummy2[] = {nrX, nrS, 1, nrT};

	const util::vector4<size_t> sizeVec2( dummy2 );


	BOOST_REQUIRE_EQUAL( img2.getSizeAsVector(), sizeVec2 );

	//***************************************************************
	nrX = 1;

	nrY = 64;

	nrS = 20;

	nrT = 20;

	std::list<data::Chunk> chunks3;

	for ( unsigned int is = 0; is < nrS; is++ ) {
		for ( unsigned int it = 0; it < nrT; it++ ) {
			chunks3.push_back( genSlice<float>( nrX, nrY, is, is + it * nrS ) );
		}
	}

	data::Image img3( chunks3 );
	BOOST_REQUIRE( img3.isClean() );

	const size_t dummy3[] = {nrX, nrY, nrS, nrT};

	const util::vector4<size_t> sizeVec3( dummy3 );

	BOOST_REQUIRE_EQUAL( img3.getSizeAsVector(), sizeVec3 );

	//***************************************************************
	nrX = 64;

	nrY = 64;

	nrS = 1;

	nrT = 20;

	std::list<data::Chunk> chunks4;

	for ( unsigned int is = 0; is < nrS; is++ ) {
		for ( unsigned int it = 0; it < nrT; it++ ) {
			chunks4.push_back( genSlice<float>( nrX, nrY, is, is + it * nrS ) );
		}
	}

	data::Image img4( chunks4 );
	BOOST_REQUIRE( img4.isClean() );

	const size_t dummy4[] = {nrX, nrY, nrS, nrT};

	const util::vector4<size_t> sizeVec4( dummy4 );

	BOOST_REQUIRE_EQUAL( img4.getSizeAsVector(), sizeVec4 );

	//***************************************************************
	nrX = 64;

	nrY = 64;

	nrS = 20;

	nrT = 1;

	std::list<data::Chunk> chunks5;

	for ( unsigned int is = 0; is < nrS; is++ ) {
		for ( unsigned int it = 0; it < nrT; it++ ) {
			chunks5.push_back( genSlice<float>( nrX, nrY, is, is + it * nrS ) );
		}
	}

	const size_t dummy5[] = {nrX, nrY, nrS, nrT};

	const util::vector4<size_t> sizeVec5( dummy5 );

	data::Image img5( chunks5 );

	BOOST_REQUIRE( img5.isClean() );


	BOOST_REQUIRE_EQUAL( img5.getSizeAsVector(), sizeVec5 );

	//***************************************************************
	nrX = 0;

	nrY = 0;

	nrS = 0;

	nrT = 0;

	std::list<data::Chunk> empty;

	data::Image img6( empty );

	const size_t dummy6[] = {nrX, nrY, nrS, nrT};

	const util::vector4<size_t> sizeVec6( dummy6 );

	BOOST_REQUIRE( !img6.isClean() ); //reIndex on an empty image shall fail (size will be undefined)

	BOOST_REQUIRE( !img6.isValid() ); //reIndex on an empty image shall fail (size will be undefined)
}

BOOST_AUTO_TEST_CASE ( image_size_test )
{
	data::MemChunk<uint8_t> original( 11, 23, 90, 12 );
	original.setPropertyAs<uint32_t>( "acquisitionNumber", 1 );
	original.setPropertyAs<util::fvector4>( "indexOrigin", util::fvector4() );
	original.setPropertyAs<util::fvector4>( "voxelSize", util::fvector4( 1, 1, 1 ) );
	original.setPropertyAs<util::fvector4>( "rowVec", util::fvector4( 1, 0, 0 ) );
	original.setPropertyAs<util::fvector4>( "columnVec", util::fvector4( 0, 1, 0 ) );
	data::Image img( original );
	BOOST_REQUIRE( img.isClean() );
	BOOST_REQUIRE( img.isValid() );
	BOOST_REQUIRE( !img.isEmpty() );

	BOOST_CHECK_EQUAL( img.getNrOfColumns(), 11 );
	BOOST_CHECK_EQUAL( img.getNrOfRows(), 23 );
	BOOST_CHECK_EQUAL( img.getNrOfSlices(), 90 );
	BOOST_CHECK_EQUAL( img.getNrOfTimesteps(), 12 );

}


BOOST_AUTO_TEST_CASE( image_get_coords_test )
{
	size_t imageSize = 20;
	data::MemChunk<uint8_t> minChunk( imageSize, imageSize, imageSize, 1 );
	minChunk.setPropertyAs<uint32_t>( "acquisitionNumber", 1 );
	minChunk.setPropertyAs<uint16_t>( "sequenceNumber", 1 );
	minChunk.setPropertyAs<util::fvector4>( "indexOrigin", util::fvector4( -10, 110.5, -99.8 ) );
	minChunk.setPropertyAs<util::fvector4>( "rowVec", util::fvector4( 1, 1.17296e-16, -9.64207e-17 ) );
	minChunk.setPropertyAs<util::fvector4>( "columnVec", util::fvector4( -1.05222e-16, 0.957823, -0.287361 ) );
	minChunk.setPropertyAs<util::fvector4>( "sliceVec", util::fvector4( -5.74721e-17, 0.287361, 0.957823 ) );
	minChunk.setPropertyAs<util::fvector4>( "voxelSize", util::fvector4( 1, 0.5, 3.5 ) );
	data::Image img( minChunk );
	BOOST_REQUIRE( img.isClean() );
	BOOST_REQUIRE( img.isValid() );
	BOOST_REQUIRE( !img.isEmpty() );

	for( size_t z = 0; z < imageSize; z++ ) {
		for( size_t y = 0; y < imageSize; y++ ) {
			for ( size_t x = 0; x < imageSize; x++ ) {
				util::fvector4 physicalCoords = img.getPhysicalCoordsFromIndex( util::ivector4( x, y, z ) );
				util::ivector4 index = img.getIndexFromPhysicalCoords( physicalCoords );
				BOOST_CHECK_EQUAL( index, util::ivector4( x, y, z ) );
			}
		}

	}
}

BOOST_AUTO_TEST_CASE( image_transformCoords_test_spm )
{
	/*this first transformCoordsTest based on the outcome of the SPM8 dicom import. SPM flips the columnVec
	and so has to recalculate the index origin of the image. The flip of the columnVector is described by
	he transform matrix.
	At the end we compare the output of the our flipped isis image and the outcome of the spm dicom import.
	*/
	//ground truth (pure irony, since it comes from SPM :-) )
	util::fvector4 SPMIo = util::fvector4( 92.5167, -159.366, -108.687 );
	util::fvector4 SPMrow = util::fvector4( -0.0105192, 0.999945, -6.52652e-09 );
	util::fvector4 SPMcolumn = util::fvector4( -0.041812, -0.000439848, 0.999125 );
	util::fvector4 SPMslice = util::fvector4( -0.99907, -0.01051, -0.0418143 );

	data::MemChunk<uint8_t> minChunk( 320, 320, 240, 1 );
	minChunk.setPropertyAs<uint32_t>( "acquisitionNumber", 1 );
	minChunk.setPropertyAs<uint16_t>( "sequenceNumber", 1 );
	minChunk.setPropertyAs<util::fvector4>( "indexOrigin", util::fvector4( 83.1801, -159.464, 114.418 ) );
	minChunk.setPropertyAs<util::fvector4>( "rowVec", util::fvector4( -0.0105192, 0.999945, -6.52652e-09 ) );
	minChunk.setPropertyAs<util::fvector4>( "columnVec", util::fvector4( 0.041812, 0.000439848, -0.999125 ) );
	minChunk.setPropertyAs<util::fvector4>( "sliceVec", util::fvector4( -0.99907, -0.01051, -0.0418143 ) );
	minChunk.setPropertyAs<util::fvector4>( "voxelSize", util::fvector4( 0.7, 0.7, 0.7 ) );
	minChunk.setPropertyAs<util::fvector4>( "voxelGap", util::fvector4() );
	data::Image img( minChunk );
	BOOST_REQUIRE( img.isClean() );
	BOOST_REQUIRE( img.isValid() );
	BOOST_REQUIRE( !img.isEmpty() );
	boost::numeric::ublas::matrix<float> transformMatrix = boost::numeric::ublas::identity_matrix<float>( 3, 3 );
	transformMatrix( 1, 1 ) = -1;
	BOOST_REQUIRE( img.transformCoords( transformMatrix, true ) );
	float err = 0.0005;

	for ( size_t i = 0; i < 3; i++ ) {
		//for some reason util::fuzzycheck does not work as expected - so we do it our own way
		BOOST_CHECK( fabs( SPMIo[i] - img.getPropertyAs<util::fvector4>( "indexOrigin" )[i] ) < err );
		BOOST_CHECK( fabs( SPMrow[i] - img.getPropertyAs<util::fvector4>( "rowVec" )[i] ) < err );
		BOOST_CHECK( fabs( SPMcolumn[i] - img.getPropertyAs<util::fvector4>( "columnVec" )[i] ) < err );
		BOOST_CHECK( fabs( SPMslice[i] - img.getPropertyAs<util::fvector4>( "sliceVec" )[i] ) < err );
	}
}

BOOST_AUTO_TEST_CASE( image_transformCoords_test_common )
{
	data::MemChunk<uint8_t> minChunk( 100, 100, 100, 1 );
	minChunk.setPropertyAs<uint32_t>( "acquisitionNumber", 1 );
	minChunk.setPropertyAs<uint16_t>( "sequenceNumber", 1 );
	minChunk.setPropertyAs<util::fvector4>( "indexOrigin", util::fvector4( -49.5, -49.5, -49.5 ) );
	minChunk.setPropertyAs<util::fvector4>( "rowVec", util::fvector4( 1, 0, 0 ) );
	minChunk.setPropertyAs<util::fvector4>( "columnVec", util::fvector4( 0, 1, 0 ) );
	minChunk.setPropertyAs<util::fvector4>( "sliceVec", util::fvector4( 0, 0, 1 ) );
	minChunk.setPropertyAs<util::fvector4>( "voxelGap", util::fvector4() );
	minChunk.setPropertyAs<util::fvector4>( "voxelSize", util::fvector4( 1, 1, 1 ) );
	data::Image img( minChunk );
	BOOST_REQUIRE( img.isClean() );
	BOOST_REQUIRE( img.isValid() );
	BOOST_REQUIRE( !img.isEmpty() );
	boost::numeric::ublas::matrix<float> transform = boost::numeric::ublas::zero_matrix<float>( 3, 3 );
	//here we are flipping all vectors
	transform( 2, 0 ) = -1;
	transform( 1, 1 ) = -1;
	transform( 0, 2 ) = -1;
	BOOST_REQUIRE( img.transformCoords( transform, true ) );
	BOOST_CHECK_EQUAL( img.getPropertyAs<util::fvector4>( "indexOrigin" ), util::fvector4( 49.5, 49.5, 49.5 ) );
	BOOST_CHECK_EQUAL( img.getPropertyAs<util::fvector4>( "rowVec" ), util::fvector4( 0, 0, -1 ) );
	BOOST_CHECK_EQUAL( img.getPropertyAs<util::fvector4>( "columnVec" ), util::fvector4( 0, -1, 0 ) );
	BOOST_CHECK_EQUAL( img.getPropertyAs<util::fvector4>( "sliceVec" ), util::fvector4( -1, 0, 0 ) );
	//here we rotate
	transform = boost::numeric::ublas::zero_matrix<float>( 3, 3 );
	transform( 0, 0 ) = 1;
	transform( 1, 1 ) = transform( 2, 2 ) = cos( 45 * M_PI / 180 );
	transform( 1, 2 ) = -sin( 45 * M_PI / 180 );
	transform( 2, 1 ) = sin( 45 * M_PI / 180 );
	BOOST_REQUIRE( img.transformCoords( transform, true ) );
	float err = 0.0005;
	//what we should get
	util::fvector4 trueIO = util::fvector4( 0, 70.0036, 49.5 );
	util::fvector4 trueRowVec = util::fvector4( 0, 0, -1 );
	util::fvector4 trueColumnVec = util::fvector4( -sqrt( 2 ) * 0.5, -sqrt( 2 ) * 0.5, 0 );
	util::fvector4 trueSliceVec = util::fvector4( -sqrt( 2 ) * 0.5, sqrt( 2 ) * 0.5, 0 );

	for ( size_t i = 0; i < 3; i++ ) {
		//for some reason util::fuzzycheck does not work as expected - so we do it our own way
		BOOST_CHECK( fabs( trueIO[i] - img.getPropertyAs<util::fvector4>( "indexOrigin" )[i] ) < err );
		BOOST_CHECK( fabs( trueRowVec[i] - img.getPropertyAs<util::fvector4>( "rowVec" )[i] ) < err );
		BOOST_CHECK( fabs( trueColumnVec[i] - img.getPropertyAs<util::fvector4>( "columnVec" )[i] ) < err );
		BOOST_CHECK( fabs( trueSliceVec[i] - img.getPropertyAs<util::fvector4>( "sliceVec" )[i] ) < err );
	}
}


} // END namespace test
} // END namespace isis
