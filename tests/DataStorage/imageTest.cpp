/*
 * imageTest.cpp
 *
 *  Created on: Oct 1, 2009
 *      Author: proeger
 */

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

/* create an image */
BOOST_AUTO_TEST_CASE ( image_init_test )
{
	data::ImageList list;
	list.push_back(boost::shared_ptr<data::Image>());
	{
		data::MemChunk<float> ch( 4, 4 );
		data::Image img;
		// inserting insufficient Chunk should fail
		data::enable_log<util::DefaultMsgPrint>( ( LogLevel )0 );
		BOOST_CHECK( ! img.insertChunk( ch ) );
		data::enable_log<util::DefaultMsgPrint>( warning );
		// but inserting a proper Chunk should work
		ch.setProperty( "indexOrigin", util::fvector4( 0, 0, 2 ) );
		ch.setProperty<uint32_t>( "acquisitionNumber", 0 );
		ch.setProperty<float>( "acquisitionTime", 0 );
		ch.setProperty( "readVec", util::fvector4( 1, 0 ) );
		ch.setProperty( "phaseVec", util::fvector4( 0, 1 ) );
		ch.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
		BOOST_REQUIRE( img.insertChunk( ch ) );
		//inserting the same chunk twice should fail
		BOOST_CHECK( ! img.insertChunk( ch ) );
		// but inserting another Chunk should work
		ch = data::MemChunk<float>( 4, 4 );
		ch.setProperty( "indexOrigin", util::fvector4( 0, 0, 0 ) );
		ch.setProperty<uint32_t>( "acquisitionNumber", 2 );
		ch.setProperty<float>( "acquisitionTime", 2 );
		ch.setProperty( "readVec", util::fvector4( 1, 0 ) );
		ch.setProperty( "phaseVec", util::fvector4( 0, 1 ) );
		ch.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
		BOOST_CHECK( img.insertChunk( ch ) );
		// Chunks should be inserted based on their position (lowest first)
		ch = data::MemChunk<float>( 4, 4 );
		ch.setProperty( "indexOrigin", util::fvector4( 0, 0, 1 ) );
		ch.setProperty<uint32_t>( "acquisitionNumber", 1 );
		ch.setProperty<float>( "acquisitionTime", 1 );
		ch.setProperty( "readVec", util::fvector4( 1, 0 ) );
		ch.setProperty( "phaseVec", util::fvector4( 0, 1 ) );
		ch.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
		BOOST_REQUIRE( img.insertChunk( ch ) );
		//Get a list of the sorted chunks
		BOOST_REQUIRE(img.reIndex());
		std::vector<boost::shared_ptr<data::Chunk> > list = img.getChunkList();
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
		data::Image img;

		for( int i = 0; i < 3; i++ ) {
			data::Chunk ch = data::MemChunk<float>( 4, 4 );
			ch.setProperty( "indexOrigin", util::fvector4( 0, 0, i, 0 ) );
			ch.setProperty<uint32_t>( "acquisitionNumber", 3 + i );
			ch.setProperty<uint32_t>( "acquisitionTime", 3 + i );
			ch.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
			ch.setProperty( "readVec", util::fvector4( 1, 0 ) );
			ch.setProperty( "phaseVec", util::fvector4( 0, 1 ) );
			BOOST_REQUIRE(img.insertChunk( ch ));
		}

		std::string str = "testString";
		img.setProperty<std::string>("testProp", str);
		BOOST_CHECK_EQUAL( img.getProperty<std::string>("testProp"), str );
		boost::shared_ptr<data::Chunk> ptr = img.getChunkList().back();
		//as all other chunks where timestep < 4 this must be at the end
		BOOST_CHECK_EQUAL( ptr->propertyValue( "indexOrigin" ), util::fvector4( 0, 0, 2 ) );
		BOOST_CHECK_EQUAL( ptr->propertyValue( "acquisitionNumber" ), 5  );


		// Check all dimensions
		data::Image img2;

		u_int32_t nrRows = 12;
		u_int32_t nrCols = 32;
		u_int32_t nrTimesteps = 17;
		u_int32_t nrSlices = 27;
		for( int t = 0; t < nrTimesteps; t++ ) {
			for( int s = 0; s < nrSlices; s++ ){
				data::Chunk ch = data::MemChunk<float>( nrCols, nrRows );
				ch.setProperty( "indexOrigin", util::fvector4( 0, 0, s) );
				ch.setProperty<uint32_t>( "acquisitionNumber", s+t*nrSlices );
				ch.setProperty<uint32_t>( "acquisitionTime", s+t*nrSlices );
				ch.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
				ch.setProperty( "readVec", util::fvector4( 1, 0 ) );
				ch.setProperty( "phaseVec", util::fvector4( 0, 1 ) );
				img2.insertChunk( ch );
			}
		}

		BOOST_REQUIRE(img2.reIndex());
		BOOST_CHECK_EQUAL(img2.volume(), nrRows*nrCols*nrTimesteps*nrSlices);
		BOOST_CHECK_EQUAL(img2.sizeToVector(), util::ivector4(nrCols, nrRows, nrSlices, nrTimesteps));


		// Check all dimensions with limit sizes
		data::Image img3;

		nrRows = 212;
		nrCols = 2;
		nrTimesteps = 2;
		nrSlices = 1;
		for( int t = 0; t < nrTimesteps; t++ ) {
			for( int s = 0; s < nrSlices; s++ ){
				data::Chunk ch = data::MemChunk<float>( nrCols, nrRows );
				ch.setProperty( "indexOrigin", util::fvector4( 0, 0, s) );
				ch.setProperty<uint32_t>( "acquisitionNumber", s+t*nrSlices );
				ch.setProperty<uint32_t>( "acquisitionTime", s+t*nrSlices );
				ch.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
				ch.setProperty( "readVec", util::fvector4( 1, 0 ) );
				ch.setProperty( "phaseVec", util::fvector4( 0, 1 ) );
				img3.insertChunk( ch );
			}
		}

		BOOST_REQUIRE(img3.reIndex());
		BOOST_CHECK_EQUAL(img3.volume(), nrRows*nrCols*nrTimesteps*nrSlices);
		BOOST_CHECK_EQUAL(img3.sizeToVector(), util::ivector4(nrCols, nrRows, nrSlices, nrTimesteps));

		data::Image img4;

		nrRows = 54;
		nrCols = 29;
		nrTimesteps = 1;
		nrSlices = 21;
		for( int t = 0; t < nrTimesteps; t++ ) {
			for( int s = 0; s < nrSlices; s++ ){
				data::Chunk ch = data::MemChunk<float>( nrCols, nrRows );
				ch.setProperty( "indexOrigin", util::fvector4( 0, 0, s, 0 ) );
				ch.setProperty<uint32_t>( "acquisitionNumber", s+t*nrSlices );
				ch.setProperty<uint32_t>( "acquisitionTime", s+t*nrSlices );
				ch.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
				ch.setProperty( "readVec", util::fvector4( 1, 0 ) );
				ch.setProperty( "phaseVec", util::fvector4( 0, 1 ) );
				img4.insertChunk( ch );
			}
		}

		BOOST_REQUIRE(img4.reIndex());
		BOOST_CHECK_EQUAL(img4.volume(), nrRows*nrCols*nrTimesteps*nrSlices);
		BOOST_CHECK_EQUAL(img4.sizeToVector(), util::ivector4(nrCols, nrRows, nrSlices, nrTimesteps));

	}
}

BOOST_AUTO_TEST_CASE ( minimal_image_test )
{
	data::MemChunk<float> ch1( 4, 4 );
	ch1.setProperty( "indexOrigin", util::fvector4( 0, 0, 2 ) );
	ch1.setProperty<uint32_t>( "acquisitionNumber", 0 );
	ch1.setProperty( "readVec", util::fvector4( 1, 0 ) );
	ch1.setProperty( "phaseVec", util::fvector4( 0, 1 ) );
	ch1.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
	
	data::MemChunk<float> ch2=ch1;
	ch2.setProperty<uint32_t>( "acquisitionNumber", 1 );
	
	data::Image img;
	const size_t size[]={4,4,1,2};
	BOOST_CHECK(img.insertChunk(ch1));
	BOOST_CHECK(img.insertChunk(ch2));
	BOOST_CHECK(img.reIndex());
	BOOST_CHECK_EQUAL(img.sizeToVector(),(util::FixedVector<size_t,4>(size)));
}

BOOST_AUTO_TEST_CASE ( type_selection_test )
{
	float org=0;
#define MAKE_CHUNK(type,name) \
	data::MemChunk<type> name( 4, 4 );\
	name.setProperty( "indexOrigin", util::fvector4( 0, 0, org ) );\
	name.setProperty<uint32_t>( "acquisitionNumber", org );\
	name.setProperty( "readVec", util::fvector4( 1, 0 ) );\
	name.setProperty( "phaseVec", util::fvector4( 0, 1 ) );\
	name.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );\
	org++;\
	
	MAKE_CHUNK(int16_t,ch_int16_t);
	MAKE_CHUNK(int8_t,ch_int8_t);
	MAKE_CHUNK(uint8_t,ch_uint8_t);
	MAKE_CHUNK(uint16_t,ch_uint16_t);

	ch_int16_t.voxel<int16_t>(0,0,0)=std::numeric_limits<int16_t>::min();
	ch_int8_t.voxel<int8_t>(0,0,0)=std::numeric_limits<int8_t>::min();
	ch_uint8_t.voxel<uint8_t>(0,0,0)=std::numeric_limits<uint8_t>::max();
	ch_uint16_t.voxel<uint16_t>(0,0,0)=std::numeric_limits<int16_t>::max(); // the maximum shall fit into int16_t
	

	data::Image img;
	const size_t size[]={4,4,4,1};
	BOOST_CHECK(img.insertChunk(ch_int16_t));
	BOOST_CHECK(img.insertChunk(ch_uint16_t));
	BOOST_CHECK(img.insertChunk(ch_int8_t));
	BOOST_CHECK(img.insertChunk(ch_uint8_t));
	BOOST_CHECK(img.reIndex());
	BOOST_CHECK_EQUAL(img.sizeToVector(),(util::FixedVector<size_t,4>(size)));
	BOOST_CHECK_EQUAL(img.typeID(),data::TypePtr<int16_t>(NULL,0).typeID());
}

BOOST_AUTO_TEST_CASE ( type_scale_test )
{
	float org=0;
	#define MAKE_CHUNK(type,name) \
	data::MemChunk<type> name( 4, 4 );\
	name.setProperty( "indexOrigin", util::fvector4( 0, 0, org ) );\
	name.setProperty<uint32_t>( "acquisitionNumber", org );\
	name.setProperty( "readVec", util::fvector4( 1, 0 ) );\
	name.setProperty( "phaseVec", util::fvector4( 0, 1 ) );\
	name.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );\
	org++;\
	
	MAKE_CHUNK(int16_t,ch_int16_t);
	MAKE_CHUNK(int8_t,ch_int8_t);
	MAKE_CHUNK(uint8_t,ch_uint8_t);
	MAKE_CHUNK(uint16_t,ch_uint16_t);

	ch_int8_t.voxel<int8_t>(0,0,0)=-1;
	ch_uint8_t.voxel<uint8_t>(0,0,0)=1;
	ch_int16_t.voxel<int16_t>(0,0,0)=-50;
	ch_uint16_t.voxel<uint16_t>(0,0,0)=2500;
	
	
	data::Image img;
	const size_t size[]={4,4,4,1};
	BOOST_CHECK(img.insertChunk(ch_int16_t));
	BOOST_CHECK(img.insertChunk(ch_uint16_t));
	BOOST_CHECK(img.insertChunk(ch_int8_t));
	BOOST_CHECK(img.insertChunk(ch_uint8_t));
	BOOST_CHECK(img.reIndex());

	std::list<std::pair<util::TypeReference,util::TypeReference> > scale=img.getScalingTo(data::TypePtr<uint8_t>::staticID);

	typedef std::list<std::pair<util::TypeReference,util::TypeReference> >::const_reference scale_ref;

	BOOST_CHECK(scale.size()==1);
	BOOST_CHECK_EQUAL(scale.front().first->as<double>(),1./10);
	BOOST_CHECK_EQUAL(scale.front().second->as<double>(),5);
}

BOOST_AUTO_TEST_CASE ( image_chunk_test )
{
	uint32_t acNum = 0;
	std::vector<std::vector<data::MemChunk<float> > > ch( 3, std::vector<data::MemChunk<float> >( 3, data::MemChunk<float>( 3, 3 ) ) );
	data::Image img;

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ ) {
			ch[i][j].setProperty( "readVec", util::fvector4( 1, 0 ) );
			ch[i][j].setProperty( "phaseVec", util::fvector4( 0, 1 ) );
			ch[i][j].setProperty( "indexOrigin", util::fvector4( 0, 0, j ) );
			ch[i][j].setProperty( "acquisitionNumber", acNum++ );
			ch[i][j].setProperty( "voxelSize", util::fvector4( 1, 1, 1 ) );
			ch[i][j].voxel<float>( j, j ) = 42;
			BOOST_REQUIRE( img.insertChunk( ch[i][j] ) );
		}

	BOOST_REQUIRE( img.reIndex() );
	BOOST_CHECK_EQUAL( img.volume(), 9 * 9 );
	BOOST_CHECK_EQUAL( img.sizeToVector(), util::ivector4( 3, 3, 3, 3 ) );
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

BOOST_AUTO_TEST_CASE ( image_voxel_test )
{
	//  get a voxel from inside and outside the image
	std::vector<data::MemChunk<float> > ch( 3, data::MemChunk<float>( 3, 3 ) );
	data::Image img;

	for ( int i = 0; i < 3; i++ ) {
		ch[i].setProperty( "indexOrigin", util::fvector4( 0, 0, i, 0 ) );
		ch[i].setProperty<uint32_t>( "acquisitionNumber", i );
		ch[i].setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
	}

	ch[0].voxel<float>( 0, 0 ) = 42.0;
	ch[1].voxel<float>( 1, 1 ) = 42.0;
	ch[2].voxel<float>( 2, 2 ) = 42;

	for ( int i = 0; i < 3; i++ ) {
		ch[i].setProperty( "readVec", util::fvector4( 1, 0 ) );
		ch[i].setProperty( "phaseVec", util::fvector4( 0, 1 ) );
		BOOST_REQUIRE( img.insertChunk( ch[i] ) );
	}

	BOOST_REQUIRE( img.reIndex() );

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
			ch[i][j].setProperty( "readVec", util::fvector4( 1, 0 ) );
			ch[i][j].setProperty( "phaseVec", util::fvector4( 0, 1 ) );
			ch[i][j].setProperty( "indexOrigin", util::fvector4( 0, 0, j ) );
			ch[i][j].setProperty( "acquisitionNumber", acNum++ );
			ch[i][j].setProperty( "voxelSize", vSize );
			ch[i][j].voxel<float>( j, j ) = i * j;
			BOOST_REQUIRE( img.insertChunk( ch[i][j] ) );
		}

	BOOST_REQUIRE( img.reIndex() );
	{
		util::TypeReference min, max;
		img.getMinMax( min, max );
		BOOST_CHECK( min->is<float>() );
		BOOST_CHECK( max->is<float>() );
		BOOST_CHECK_EQUAL( min->as<float>(), 0 );
		BOOST_CHECK_EQUAL( max->as<float>(), 4 );
	}
	{
		util::TypeReference min, max;
		//this should be 0,0 because the first chunk only has zeros
		img.getChunk( 0, 0, 0, 0 ).getMinMax( min, max );
		BOOST_CHECK( min->is<float>() );
		BOOST_CHECK( max->is<float>() );
		BOOST_CHECK_EQUAL( min->as<float>(), 0 );
		BOOST_CHECK_EQUAL( max->as<float>(), 0 );
	}
}
BOOST_AUTO_TEST_CASE( orientation_test )
{
	data::MemChunk<float> ch( 3, 3, 3 );
	data::Image img;
	ch.setProperty( "indexOrigin", util::fvector4( 0, 0, 0 ) );
	ch.setProperty( "readVec", util::fvector4( 1, 0 ) );
	ch.setProperty( "phaseVec", util::fvector4( 0, 1 ) );
	ch.setProperty( "acquisitionNumber", ( uint32_t )0 );
	ch.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
	BOOST_REQUIRE( img.insertChunk( ch ) );
	data::enable_log<util::DefaultMsgPrint>( error );
	BOOST_REQUIRE( img.reIndex() );
	data::enable_log<util::DefaultMsgPrint>( warning );
	BOOST_CHECK_EQUAL( img.getMainOrientation(), data::Image::axial );
}

BOOST_AUTO_TEST_CASE( memimage_test )
{
	std::vector<std::vector<data::MemChunk<float> > > ch( 3, std::vector<data::MemChunk<float> >( 3, data::MemChunk<float>( 3, 3 ) ) );
	data::Image img;
	uint32_t acNum = 0;
	const util::fvector4 vSize( 1, 1, 1, 0 );

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ ) {
			ch[i][j].setProperty( "readVec", util::fvector4( 1, 0 ) );
			ch[i][j].setProperty( "phaseVec", util::fvector4( 0, 1 ) );
			ch[i][j].setProperty( "indexOrigin", util::fvector4( 0, 0, j ) );
			ch[i][j].setProperty( "acquisitionNumber", acNum++ );
			ch[i][j].setProperty( "voxelSize", vSize );
			ch[i][j].voxel<float>( j, j ) = i * j * 1000;
			BOOST_REQUIRE( img.insertChunk( ch[i][j] ) );
		}

	BOOST_REQUIRE( img.reIndex() );
	{
		util::TypeReference min, max;
		img.getMinMax( min, max );
		BOOST_CHECK( min->is<float>() );
		BOOST_CHECK( max->is<float>() );
		BOOST_CHECK_EQUAL( min->as<float>(), 0 );
		BOOST_CHECK_EQUAL( max->as<float>(), 2 * 2 * 1000 );
	}
	{
		// Conversion to uint8_t (will downscale [0-255])
		data::enable_log<util::DefaultMsgPrint>( error );
		data::MemImage<uint8_t> img2( img );
		data::enable_log<util::DefaultMsgPrint>( warning );
		BOOST_REQUIRE( img2.reIndex() );
		//Check if the metadata were copied correct
		BOOST_CHECK_EQUAL( static_cast<util::PropMap>( img ), static_cast<util::PropMap>( img2 ) );

		for ( int i = 0; i < 3; i++ )
			for ( int j = 0; j < 3; j++ ) {
				const util::PropMap &c1 = img.getChunk( 0, 0, i, j );
				const util::PropMap &c2 = img2.getChunk( 0, 0, i, j );
				BOOST_REQUIRE( c1.valid() );
				BOOST_CHECK( c2.valid() );
				BOOST_CHECK_EQUAL( c1, c2 );
			}

		util::TypeReference min, max;
		img2.getMinMax( min, max );
		BOOST_CHECK( min->is<uint8_t>() );
		BOOST_CHECK( max->is<uint8_t>() );
		BOOST_CHECK_EQUAL( min->as<uint8_t>(), 0 );
		BOOST_CHECK_EQUAL( max->as<uint8_t>(), 255 );
	}
	{
		// Conversion to int16_t (will upscale [0-32k) )
		data::MemImage<int16_t> img2( img );
		BOOST_REQUIRE( img2.reIndex() );
		BOOST_CHECK_EQUAL( static_cast<util::PropMap>( img ), static_cast<util::PropMap>( img2 ) );
		util::TypeReference min, max;
		img2.getMinMax( min, max );
		BOOST_CHECK( min->is<int16_t>() );
		BOOST_CHECK( max->is<int16_t>() );
		BOOST_CHECK_EQUAL( min->as<int16_t>(), 0 );
		BOOST_CHECK_EQUAL( max->as<int16_t>(), std::numeric_limits<int16_t>::max() );
	}
} // END memimage_test

BOOST_AUTO_TEST_CASE( typediamge_test )
{
	data::Image img;
	uint32_t acNum = 0;
	const util::fvector4 vSize( 1, 1, 1, 0 );

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ ) {
			data::MemChunk<uint8_t> ch( 3, 3 );
			ch.setProperty( "readVec", util::fvector4( 1, 0 ) );
			ch.setProperty( "phaseVec", util::fvector4( 0, 1 ) );
			ch.setProperty( "indexOrigin", util::fvector4( 0, 0, j ) );
			ch.setProperty( "acquisitionNumber", acNum++ );
			ch.setProperty( "voxelSize", vSize );
			ch.voxel<uint8_t>( j, j ) = std::numeric_limits<uint8_t>::max();
			BOOST_REQUIRE( img.insertChunk( ch ) );
		}

	for ( int i = 3; i < 10; i++ )
		for ( int j = 0; j < 3; j++ ) {
			data::MemChunk<int16_t> ch( 3, 3 );
			ch.setProperty( "readVec", util::fvector4( 1, 0 ) );
			ch.setProperty( "phaseVec", util::fvector4( 0, 1 ) );
			ch.setProperty( "indexOrigin", util::fvector4( 0, 0, j ) );
			ch.setProperty( "acquisitionNumber", acNum++ );
			ch.setProperty( "voxelSize", vSize );
			ch.voxel<int16_t>( j, j ) = std::numeric_limits<int16_t>::max();
			BOOST_REQUIRE( img.insertChunk( ch ) );
		}

	BOOST_REQUIRE( img.reIndex() );
	{
		util::TypeReference min, max;
		img.getMinMax( min, max );
		BOOST_CHECK( min->is<uint8_t>() );
		BOOST_CHECK( max->is<int16_t>() );
		BOOST_CHECK_EQUAL( min->as<int16_t>(), 0 );
		BOOST_CHECK_EQUAL( max->as<int16_t>(), std::numeric_limits<int16_t>::max() );
	}
	{
		// Conversion to uint8_t (will downscale [0-255])
		data::enable_log<util::DefaultMsgPrint>( error );
		data::TypedImage<uint8_t> img2( img );
		data::enable_log<util::DefaultMsgPrint>( warning );
		BOOST_REQUIRE( img2.reIndex() );
		//Check if the metadata were copied correct
		BOOST_CHECK_EQUAL( static_cast<util::PropMap>( img ), static_cast<util::PropMap>( img2 ) );

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

		util::TypeReference min, max;
		img2.getMinMax( min, max );
		BOOST_CHECK( min->is<uint8_t>() );
		BOOST_CHECK( max->is<uint8_t>() );
		BOOST_CHECK_EQUAL( min->as<uint8_t>(), 0 );
		BOOST_CHECK_EQUAL( max->as<uint8_t>(), std::numeric_limits<uint8_t>::max() );
	}
} // END typedimage_test

BOOST_AUTO_TEST_CASE ( image_transformCoords_test )
{
	// dummy image
	data::ImageList images = data::IOFactory::load( "nix.null" );
	BOOST_REQUIRE( !images.empty() );
	boost::shared_ptr<data::Image> img = *( images.begin() );
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
	img->setProperty( "readVec", util::fvector4( 1, 0, 0, 0 ) );
	img->setProperty( "phaseVec", util::fvector4( 0, 1, 0, 0 ) );
	img->setProperty( "sliceVec", util::fvector4( 0, 0, 1, 0 ) );
	// set index origin to DICOM space index origin
	img->setProperty( "indexOrigin", util::fvector4( -1, -2, -3, 0 ) );
	// apply transformation
	img->transformCoords( T );
	// CHECKS
	BOOST_CHECK( img->getProperty<util::fvector4>( "readVec" ).fuzzyEqual( util::fvector4( -1, 0, 0, 0 ) ) );
	BOOST_CHECK( img->getProperty<util::fvector4>( "phaseVec" ).fuzzyEqual( util::fvector4( 0, -1, 0, 0 ) ) );
	BOOST_CHECK( img->getProperty<util::fvector4>( "sliceVec" ).fuzzyEqual( util::fvector4( 0, 0, 1, 0 ) ) );
	BOOST_CHECK( img->getProperty<util::fvector4>( "indexOrigin" ).fuzzyEqual( util::fvector4( 1, 2, -3, 0 ) ) );
	;
	// **** SAGITTAL ****
	// set orientation SAGITTAL in DCIOM space
	img->setProperty( "readVec", util::fvector4( 0, 1, 0, 0 ) );
	img->setProperty( "phaseVec", util::fvector4( 0, 0, 1, 0 ) );
	img->setProperty( "sliceVec", util::fvector4( 1, 0, 0, 0 ) );
	// set index origin to DICOM space index origin
	img->setProperty( "indexOrigin", util::fvector4( -3, -1, -2, 0 ) );
	// apply transformation
	img->transformCoords( T );
	// CHECKS
	BOOST_CHECK( img->getProperty<util::fvector4>( "readVec" ).fuzzyEqual( util::fvector4( 0, -1, 0, 0 ) ) );
	BOOST_CHECK( img->getProperty<util::fvector4>( "phaseVec" ).fuzzyEqual( util::fvector4( 0, 0, -1, 0 ) ) );
	BOOST_CHECK( img->getProperty<util::fvector4>( "sliceVec" ).fuzzyEqual( util::fvector4( 1, 0, 0, 0 ) ) );
	BOOST_CHECK( img->getProperty<util::fvector4>( "indexOrigin" ).fuzzyEqual( util::fvector4( -3, 1, 2, 0 ) ) );
	// **** CORONAL ****
	// set orientation CORONAL in DCIOM space
	img->setProperty( "readVec", util::fvector4( 1, 0, 0, 0 ) );
	img->setProperty( "phaseVec", util::fvector4( 0, 0, 1, 0 ) );
	img->setProperty( "sliceVec", util::fvector4( 0, -1, 0, 0 ) );
	// set index origin to DICOM space index origin
	img->setProperty( "indexOrigin", util::fvector4( -1, 3, -2, 0 ) );
	// apply transformation
	img->transformCoords( T );
	// CHECKS
	BOOST_CHECK( img->getProperty<util::fvector4>( "readVec" ).fuzzyEqual( util::fvector4( -1, 0, 0, 0 ) ) );
	BOOST_CHECK( img->getProperty<util::fvector4>( "phaseVec" ).fuzzyEqual( util::fvector4( 0, 0, -1, 0 ) ) );
	BOOST_CHECK( img->getProperty<util::fvector4>( "sliceVec" ).fuzzyEqual( util::fvector4( 0, -1, 0, 0 ) ) );
	BOOST_CHECK( img->getProperty<util::fvector4>( "indexOrigin" ).fuzzyEqual( util::fvector4( 1, 3, 2, 0 ) ) );
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
	data::Image img;

	for ( unsigned int is = 0; is < nrS; is++ ) {
		for ( unsigned int it = 0; it < nrT; it++ ) {
			data::MemChunk<float> ch( nrX, nrY );
			ch.setProperty( "indexOrigin", util::fvector4( 0, 0, is ) );
			BOOST_CHECK( ch.propertyValue( "indexOrigin" ).needed() );
			ch.setProperty( "readVec", util::fvector4( 17, 0, 0 ) );
			ch.setProperty( "phaseVec", util::fvector4( 0, 17, 0 ) );
			ch.setProperty( "sliceVec", util::fvector4( 0, 0, 31 ) );
			ch.setProperty( "voxelSize", util::fvector4( 3, 3, 3 ) );
			ch.setProperty<uint32_t>( "acquisitionNumber", is + it * nrS );
			ch.setProperty<uint16_t>( "sequenceNumber", 1 );
			BOOST_REQUIRE( img.insertChunk( ch ) );
		}
	}

	srand ( time( NULL ) );
	const size_t dummy[] = {nrX, nrY, nrS, nrT};
	const util::FixedVector<size_t, 4> sizeVec( dummy );
	img.reIndex();
	BOOST_REQUIRE_EQUAL( img.getChunkList().size(), nrT * nrS );
	BOOST_REQUIRE_EQUAL( img.sizeToVector(), sizeVec );

	for ( unsigned int ix = 0; ix < nrX; ix++ ) {
		for ( unsigned int iy = 0; iy < nrY; iy++ ) {
			for ( unsigned int is = 0; is < nrS; is++ ) {
				for ( unsigned int it = 0; it < nrT; it++ ) {
					img.voxel<float>( ix, iy, is, it ) = ix+iy+is+it;
				}
			}
		}
	}

	float min, max;
	img.getMinMax( min, max );
	BOOST_REQUIRE_EQUAL( min, 0 );
	double scale = std::numeric_limits<uint16_t>::max() / max;
	data::enable_log<util::DefaultMsgPrint>( error );
	data::MemImage<uint16_t> copyImg( img );
	data::enable_log<util::DefaultMsgPrint>( warning );
	copyImg.reIndex();
	BOOST_REQUIRE_EQUAL( copyImg.sizeToVector(), sizeVec );

	for ( unsigned int ix = 0; ix < nrX; ix++ ) {
		for ( unsigned int iy = 0; iy < nrY; iy++ ) {
			for ( unsigned int is = 0; is < nrS; is++ ) {
				for ( unsigned int it = 0; it < nrT; it++ ) {
					BOOST_CHECK_EQUAL( converter( img.voxel<float>( ix, iy, is, it )*scale ) , copyImg.voxel<uint16_t>( ix, iy, is, it ) );
				}
			}
		}
	}


	data::MemChunk<float> chSlice(nrX, nrY);
	img.getChunk(0,0,12,8,false).copySlice(0,0,chSlice,0,0);

	float * pValues = ((boost::shared_ptr<float>) chSlice.getTypePtr<float>()).get();
	float *pRun = pValues;
	for ( unsigned int iy = 0; iy < nrY; iy++ ) {
			for ( unsigned int ix = 0; ix < nrX; ix++ ) {
				BOOST_CHECK_EQUAL(static_cast<float>(20+ix+iy), chSlice.voxel<float>(ix,iy));
				BOOST_CHECK_EQUAL(static_cast<float>(20+ix+iy), *pRun++);

			}
		}
}

BOOST_AUTO_TEST_CASE ( image_splice_test )
{
	data::MemChunk<uint8_t> original( 10, 10, 10, 10 );
	data::Image img;
	original.setProperty<uint32_t>( "acquisitionNumber", 1 );
	original.setProperty<util::fvector4>( "indexOrigin", util::fvector4() );
	original.setProperty<util::fvector4>( "voxelSize", util::fvector4( 1, 1, 1 ) );
	original.setProperty<util::fvector4>( "readVec", util::fvector4( 1, 0, 0 ) );
	original.setProperty<util::fvector4>( "phaseVec", util::fvector4( 0, 1, 0 ) );
	BOOST_REQUIRE( img.insertChunk( original ) );
	BOOST_REQUIRE( img.reIndex() );
	BOOST_REQUIRE( !img.empty() );
	img.spliceDownTo( data::sliceDim );
	std::vector<boost::shared_ptr<data::Chunk> > chunks = img.getChunkList();
	BOOST_CHECK_EQUAL( chunks.size(), 100 );

	for( size_t i = 0; i < chunks.size(); i++ ) {
		BOOST_CHECK_EQUAL( chunks[i]->getProperty<int32_t>( "acquisitionNumber" ), i + 1 );
	}
}

BOOST_AUTO_TEST_CASE ( image_init_test_sizes )
{
	unsigned int nrX = 64;
	unsigned int nrY = 64;
	unsigned int nrS = 20;
	unsigned int nrT = 20;
	data::Image img;

	for ( unsigned int is = 0; is < nrS; is++ ) {
		for ( unsigned int it = 0; it < nrT; it++ ) {
			data::MemChunk<float> ch( nrX, nrY );
			ch.setProperty( "indexOrigin", util::fvector4( 0, 0, is ) );
			BOOST_CHECK( ch.propertyValue( "indexOrigin" ).needed() );
			ch.setProperty( "readVec", util::fvector4( 17, 0, 0 ) );
			ch.setProperty( "phaseVec", util::fvector4( 0, 17, 0 ) );
			ch.setProperty( "sliceVec", util::fvector4( 0, 0, 31 ) );
			ch.setProperty( "voxelSize", util::fvector4( 3, 3, 3 ) );
			ch.setProperty<uint32_t>( "acquisitionNumber", is + it * nrS );
			ch.setProperty<uint16_t>( "sequenceNumber", 1 );
			BOOST_REQUIRE( img.insertChunk( ch ) );
		}
	}

	const size_t dummy[] = {nrX, nrY, nrS, nrT};

	const util::FixedVector<size_t, 4> sizeVec( dummy );

	img.reIndex();

	BOOST_REQUIRE_EQUAL( img.sizeToVector(), sizeVec );

	//***************************************************************
	nrX = 64;

	nrS = 20;

	nrT = 20;

	data::Image img2;

	for ( unsigned int is = 0; is < nrS; is++ ) {
		for ( unsigned int it = 0; it < nrT; it++ ) {
			data::MemChunk<float> ch( nrX );
			ch.setProperty( "indexOrigin", util::fvector4( 0, 0, is ) );
			BOOST_CHECK( ch.propertyValue( "indexOrigin" ).needed() );
			ch.setProperty( "readVec", util::fvector4( 17, 0, 0 ) );
			ch.setProperty( "phaseVec", util::fvector4( 0, 17, 0 ) );
			ch.setProperty( "sliceVec", util::fvector4( 0, 0, 31 ) );
			ch.setProperty( "voxelSize", util::fvector4( 3, 3, 3 ) );
			ch.setProperty<uint32_t>( "acquisitionNumber", is + it * nrS );
			ch.setProperty<uint16_t>( "sequenceNumber", 1 );
			BOOST_REQUIRE( img2.insertChunk( ch ) );
		}
	}

	const size_t dummy2[] = {nrX, nrS, 1, nrT};

	const util::FixedVector<size_t, 4> sizeVec2( dummy2 );

	img2.reIndex();

	BOOST_REQUIRE_EQUAL( img2.sizeToVector(), sizeVec2 );

	//***************************************************************
	nrX = 1;

	nrY = 64;

	nrS = 20;

	nrT = 20;

	data::Image img3;

	for ( unsigned int is = 0; is < nrS; is++ ) {
		for ( unsigned int it = 0; it < nrT; it++ ) {
			data::MemChunk<float> ch( nrX, nrY );
			ch.setProperty( "indexOrigin", util::fvector4( 0, 0, is ) );
			BOOST_CHECK( ch.propertyValue( "indexOrigin" ).needed() );
			ch.setProperty( "readVec", util::fvector4( 17, 0, 0 ) );
			ch.setProperty( "phaseVec", util::fvector4( 0, 17, 0 ) );
			ch.setProperty( "sliceVec", util::fvector4( 0, 0, 31 ) );
			ch.setProperty( "voxelSize", util::fvector4( 3, 3, 3 ) );
			ch.setProperty<uint32_t>( "acquisitionNumber", is + it * nrS );
			ch.setProperty<uint16_t>( "sequenceNumber", 1 );
			BOOST_REQUIRE( img3.insertChunk( ch ) );
		}
	}

	const size_t dummy3[] = {nrX, nrY, nrS, nrT};

	const util::FixedVector<size_t, 4> sizeVec3( dummy3 );

	img3.reIndex();

	BOOST_REQUIRE_EQUAL( img3.sizeToVector(), sizeVec3 );

	//***************************************************************
	nrX = 64;

	nrY = 64;

	nrS = 1;

	nrT = 20;

	data::Image img4;

	for ( unsigned int is = 0; is < nrS; is++ ) {
		for ( unsigned int it = 0; it < nrT; it++ ) {
			data::MemChunk<float> ch( nrX, nrY );
			ch.setProperty( "indexOrigin", util::fvector4( 0, 0, is ) );
			BOOST_CHECK( ch.propertyValue( "indexOrigin" ).needed() );
			ch.setProperty( "readVec", util::fvector4( 17, 0, 0 ) );
			ch.setProperty( "phaseVec", util::fvector4( 0, 17, 0 ) );
			ch.setProperty( "sliceVec", util::fvector4( 0, 0, 31 ) );
			ch.setProperty( "voxelSize", util::fvector4( 3, 3, 3 ) );
			ch.setProperty<uint32_t>( "acquisitionNumber", is + it * nrS );
			ch.setProperty<uint16_t>( "sequenceNumber", 1 );
			BOOST_REQUIRE( img4.insertChunk( ch ) );
		}
	}

	const size_t dummy4[] = {nrX, nrY, nrS, nrT};

	const util::FixedVector<size_t, 4> sizeVec4( dummy4 );

	img4.reIndex();

	BOOST_REQUIRE_EQUAL( img4.sizeToVector(), sizeVec4 );

	//***************************************************************
	nrX = 64;

	nrY = 64;

	nrS = 20;

	nrT = 1;

	data::Image img5;

	for ( unsigned int is = 0; is < nrS; is++ ) {
		for ( unsigned int it = 0; it < nrT; it++ ) {
			data::MemChunk<float> ch( nrX, nrY );
			ch.setProperty( "indexOrigin", util::fvector4( 0, 0, is ) );
			BOOST_CHECK( ch.propertyValue( "indexOrigin" ).needed() );
			ch.setProperty( "readVec", util::fvector4( 17, 0, 0 ) );
			ch.setProperty( "phaseVec", util::fvector4( 0, 17, 0 ) );
			ch.setProperty( "sliceVec", util::fvector4( 0, 0, 31 ) );
			ch.setProperty( "voxelSize", util::fvector4( 3, 3, 3 ) );
			ch.setProperty<uint32_t>( "acquisitionNumber", is + it * nrS );
			ch.setProperty<uint16_t>( "sequenceNumber", 1 );
			BOOST_REQUIRE( img5.insertChunk( ch ) );
		}
	}

	const size_t dummy5[] = {nrX, nrY, nrS, nrT};

	const util::FixedVector<size_t, 4> sizeVec5( dummy5 );

	img5.reIndex();

	BOOST_REQUIRE_EQUAL( img5.sizeToVector(), sizeVec5 );

	//***************************************************************
	nrX = 0;

	nrY = 0;

	nrS = 0;

	nrT = 0;

	data::Image img6;

	const size_t dummy6[] = {nrX, nrY, nrS, nrT};

	const util::FixedVector<size_t, 4> sizeVec6( dummy6 );

	data::enable_log<util::DefaultMsgPrint>( error );

	BOOST_REQUIRE( !img6.reIndex() ); //reIndex on an empty image shall fail (size will be undefined)

	data::enable_log<util::DefaultMsgPrint>( warning );
}




} // END namespace test
} // END namespace isis
