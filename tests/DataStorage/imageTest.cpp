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
	util::enable_log<util::DefaultMsgPrint>( error );
	data::enable_log<util::DefaultMsgPrint>( error );
	data::MemChunk<float> ch( 4, 4 );
	data::Image img;
	// inserting insufficient Chunk should fail
	BOOST_CHECK( ! img.insertChunk( ch ) );
	// but inserting a proper Chunk should work
	ch.setProperty( "indexOrigin", util::fvector4( 0, 0, 2, 0 ) );
	ch.setProperty<uint32_t>( "acquisitionNumber", 2 );
	ch.setProperty( "readVec", util::fvector4( 1, 0 ) );
	ch.setProperty( "phaseVec", util::fvector4( 0, 1 ) );

	ch.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
	BOOST_REQUIRE( img.insertChunk( ch ) );
	//inserting the same chunk twice should fail
	BOOST_CHECK( ! img.insertChunk( ch ) );
	// but inserting another Chunk should work
	ch = data::MemChunk<float>( 4, 4 );
	ch.setProperty( "indexOrigin", util::fvector4( 0, 0, 0, 0 ) );
	ch.setProperty<uint32_t>( "acquisitionNumber", 0 );
	ch.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
	BOOST_REQUIRE( img.insertChunk( ch ) );
	// Chunks should be inserted based on their position (lowest first)
	ch = data::MemChunk<float>( 4, 4 );
	ch.setProperty( "indexOrigin", util::fvector4( 0, 0, 1, 0 ) );
	ch.setProperty<uint32_t>( "acquisitionNumber", 1 );
	ch.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
	BOOST_REQUIRE( img.insertChunk( ch ) );
	//threat image as a list of sorted chunks
	//Image-Chunk-List should be copyable into other lists (and its order should be correct)
	//Note: this ordering is not allways geometric correct
	//@todo equality test
	std::list<data::Chunk> list( img.chunksBegin(), img.chunksEnd() );
	unsigned short i = 0;
	BOOST_FOREACH( const data::Chunk & ref, list ) {
		BOOST_REQUIRE( ref.propertyValue( "indexOrigin" ) == util::fvector4( 0, 0, i, 0 ) );
		BOOST_REQUIRE( ref.propertyValue( "acquisitionNumber" ) == i++ );
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
	ch.setProperty<uint32_t>( "acquisitionNumber", 4 );
	ch.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
	BOOST_REQUIRE( img.insertChunk( ch ) );
	data::Image::ChunkIterator it = img.chunksEnd();
	//as all other chunks where timestep 0 this must be at the end
	BOOST_CHECK( ( --it )->propertyValue( "indexOrigin" ) == util::fvector4( 0, 0, 0, 1 ) );
	BOOST_CHECK( ( it )->propertyValue( "acquisitionNumber" ) == (int32_t)4  );
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
			ch[i][j].setProperty( "indexOrigin", util::fvector4( 0, 0, j, i ) );
			ch[i][j].setProperty( "acquisitionNumber", acNum++ );
			ch[i][j].setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
			ch[i][j].voxel<float>( j, j ) = 42;
			BOOST_REQUIRE( img.insertChunk( ch[i][j] ) );
		}

	BOOST_REQUIRE(img.reIndex());
	BOOST_CHECK_EQUAL( img.volume(), 9 * 9 );
	BOOST_CHECK_EQUAL( img.sizeToVector(), util::ivector4( 3, 3, 3, 3 ) );
	const data::Chunk &ref11 = img.getChunk( 0, 0, 0 );
	const data::Chunk &ref12 = img.getChunk( 1, 1, 1 );
	const data::Chunk &ref13 = img.getChunk( 2, 2, 2 );
	//                                         r,p,s
	const data::Chunk &ref22 = img.getChunk( 1, 1, 1, 1 );
	const data::Chunk &ref21 = img.getChunk( 0, 0, 0, 1 );
	const data::Chunk &ref23 = img.getChunk( 2, 2, 2, 1 );
	BOOST_CHECK_EQUAL( ref11.propertyValue( "indexOrigin" ), util::fvector4( 0, 0, 0, 0 ) );
	BOOST_CHECK_EQUAL( ref12.propertyValue( "indexOrigin" ), util::fvector4( 0, 0, 1, 0 ) );
	BOOST_CHECK_EQUAL( ref13.propertyValue( "indexOrigin" ), util::fvector4( 0, 0, 2, 0 ) );
	BOOST_CHECK_EQUAL( ref11.propertyValue( "acquisitionNumber" ), (int32_t)0 );
	BOOST_CHECK_EQUAL( ref12.propertyValue( "acquisitionNumber" ), (int32_t)1 );
	BOOST_CHECK_EQUAL( ref13.propertyValue( "acquisitionNumber" ), (int32_t)2 );
	BOOST_CHECK_EQUAL( ref21.propertyValue( "acquisitionNumber" ), (int32_t)3 );
	BOOST_CHECK_EQUAL( ref22.propertyValue( "acquisitionNumber" ), (int32_t)4 );
	BOOST_CHECK_EQUAL( ref23.propertyValue( "acquisitionNumber" ), (int32_t)5 );
	BOOST_CHECK_EQUAL( ref22.propertyValue( "indexOrigin" ), util::fvector4( 0, 0, 1, 1 ) );
	BOOST_CHECK( ! ( ref22.propertyValue( "indexOrigin" ) == util::fvector4( 0, 0, 1, 0 ) ) );
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

	BOOST_REQUIRE(img.reIndex());
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
			ch[i][j].setProperty( "indexOrigin", util::fvector4( 0, 0, j, i ) );
			ch[i][j].setProperty( "acquisitionNumber", acNum++ );
			ch[i][j].setProperty( "voxelSize", vSize );
			ch[i][j].voxel<float>( j, j ) = i * j;
			BOOST_REQUIRE( img.insertChunk( ch[i][j] ) );
		}

	BOOST_REQUIRE(img.reIndex());
	{
		util::_internal::TypeBase::Reference min, max;
		img.getMinMax( min, max );
		BOOST_CHECK( min->is<float>() );
		BOOST_CHECK( max->is<float>() );
		BOOST_CHECK_EQUAL( min->as<float>(), 0 );
		BOOST_CHECK_EQUAL( max->as<float>(), 4 );
	}
	{
		util::_internal::TypeBase::Reference min, max;
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
	ch.setProperty( "acquisitionNumber", (int32_t)0 );
	ch.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
	BOOST_REQUIRE( img.insertChunk( ch ) );
	BOOST_REQUIRE(img.reIndex());
	BOOST_CHECK_EQUAL( img.getMainOrientation(), data::Image::axial );
}

BOOST_AUTO_TEST_CASE( memimage_test )
{
	std::vector<std::vector<data::MemChunk<float> > > ch( 3, std::vector<data::MemChunk<float> >( 3, data::MemChunk<float>( 3, 3 ) ) );
	data::Image img;
	unsigned short acNum = 0;
	const util::fvector4 vSize( 1, 1, 1, 0 );

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ ) {
			ch[i][j].setProperty( "readVec", util::fvector4( 1, 0 ) );
			ch[i][j].setProperty( "phaseVec", util::fvector4( 0, 1 ) );
			ch[i][j].setProperty( "indexOrigin", util::fvector4( 0, 0, j, i ) );
			ch[i][j].setProperty( "acquisitionNumber", acNum++ );
			ch[i][j].setProperty( "voxelSize", vSize );
			ch[i][j].voxel<float>( j, j ) = i * j * 1000;
			BOOST_REQUIRE( img.insertChunk( ch[i][j] ) );
		}

	BOOST_REQUIRE(img.reIndex());
	{
		util::_internal::TypeBase::Reference min, max;
		img.getMinMax( min, max );
		BOOST_CHECK( min->is<float>() );
		BOOST_CHECK( max->is<float>() );
		BOOST_CHECK_EQUAL( min->as<float>(), 0 );
		BOOST_CHECK_EQUAL( max->as<float>(), 2 * 2 * 1000 );
	}
	{
		// Conversion to uint8_t (will downscale [0-255])
		data::MemImage<uint8_t> img2( img );
		BOOST_REQUIRE(img2.reIndex());
		BOOST_CHECK_EQUAL(static_cast<util::PropMap>(img), static_cast<util::PropMap>(img2));
		util::_internal::TypeBase::Reference min, max;
		img2.getMinMax( min, max );
		BOOST_CHECK( min->is<uint8_t>() );
		BOOST_CHECK( max->is<uint8_t>() );
		BOOST_CHECK_EQUAL( min->as<uint8_t>(), 0 );
		BOOST_CHECK_EQUAL( max->as<uint8_t>(), 255 );
	}
	{
		// Conversion to int16_t (will upscale [0-32k])
		data::MemImage<int16_t> img2( img );
		BOOST_REQUIRE(img2.reIndex());
		BOOST_CHECK_EQUAL(static_cast<util::PropMap>(img), static_cast<util::PropMap>(img2));
		util::_internal::TypeBase::Reference min, max;
		img2.getMinMax( min, max );
		BOOST_CHECK( min->is<int16_t>() );
		BOOST_CHECK( max->is<int16_t>() );
		BOOST_CHECK_EQUAL( min->as<int16_t>(), 0 );
		BOOST_CHECK_EQUAL( max->as<int16_t>(), std::numeric_limits<int16_t>::max() );
	}
} // END memimage_test

BOOST_AUTO_TEST_CASE (transformCoords_test) {

	// dummy image
	boost::shared_ptr<data::Image> img = *(data::IOFactory::load( "nix.null" ).begin());

	//TODO rewrite this test to use BOST_UNIT_TEST_ASSERTS with the help of
	// util::fuzzyEqual

	// ************************************************************************
	// Transformation: DICOM -> Nifti
	// ************************************************************************
	boost::numeric::ublas::matrix<float> T(3,3);
	T(0,0) = -1;T(0,1) = 0;T(0,2) = 0;
	T(1,0) = 0;T(1,1) = -1;T(1,2) = 0;
	T(2,0) = 0;T(2,1) = 0;T(2,2) = 1;

	// **** AXIAL ****
	// set orientation AXIAL in DCIOM space
	img->setProperty( "readVec", util::fvector4( 1, 0, 0, 0 ) );
	img->setProperty( "phaseVec", util::fvector4( 0, 1, 0, 0 ) );
	img->setProperty( "sliceVec", util::fvector4( 0, 0, 1, 0 ) );
	// set index origin to DICOM space index origin
	img->setProperty("indexOrigin", util::fvector4(-1, -2, -3, 0));
	// apply transformation
	img->transformCoords(T);

	// **** OUTPUT ****
	std::cout << "DICOM (axial) --> Nifti (axial)" << std::endl;
	std::cout << img->getProperty<util::fvector4>("readVec") << std::endl;
	std::cout << img->getProperty<util::fvector4>("phaseVec") << std::endl;
	std::cout << img->getProperty<util::fvector4>("sliceVec") << std::endl;

	std::cout << img->getProperty<util::fvector4>("indexOrigin") << std::endl;

	// **** SAGITTAL ****
	// set orientation SAGITTAL in DCIOM space
	img->setProperty( "readVec", util::fvector4( 0, 1, 0, 0 ) );
	img->setProperty( "phaseVec", util::fvector4( 0, 0, 1, 0 ) );
	img->setProperty( "sliceVec", util::fvector4( 1, 0, 0, 0 ) );
	// set index origin to DICOM space index origin
	img->setProperty("indexOrigin", util::fvector4(-3, -1, -2, 0));
	// apply transformation
	img->transformCoords(T);

	// **** OUTPUT ****
	std::cout << "DICOM (sagittal) --> Nifti (sagittal)" << std::endl;
	std::cout << img->getProperty<util::fvector4>("readVec") << std::endl;
	std::cout << img->getProperty<util::fvector4>("phaseVec") << std::endl;
	std::cout << img->getProperty<util::fvector4>("sliceVec") << std::endl;

	std::cout << img->getProperty<util::fvector4>("indexOrigin") << std::endl;

	// **** CORONAL ****
	// set orientation CORONAL in DCIOM space
	img->setProperty( "readVec", util::fvector4( 1, 0, 0, 0 ) );
	img->setProperty( "phaseVec", util::fvector4( 0, 0, 1, 0 ) );
	img->setProperty( "sliceVec", util::fvector4( 0, -1, 0, 0 ) );
	// set index origin to DICOM space index origin
	img->setProperty("indexOrigin", util::fvector4(-1, 3, -2, 0));
	// apply transformation
	img->transformCoords(T);

	// **** OUTPUT ****
	std::cout << "DICOM (coronal) --> Nifti (coronal)" << std::endl;
	std::cout << img->getProperty<util::fvector4>("readVec") << std::endl;
	std::cout << img->getProperty<util::fvector4>("phaseVec") << std::endl;
	std::cout << img->getProperty<util::fvector4>("sliceVec") << std::endl;

	std::cout << img->getProperty<util::fvector4>("indexOrigin") << std::endl;


} // END transformCoords_test


} // END namespace test
} // END namespace isis
