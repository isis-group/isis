/*
* imageTest.cpp
*
*  Created on: Oct 1, 2009
*      Author: proeger
*/

#define BOOST_TEST_MODULE ImageLoadTest
#include <boost/test/unit_test.hpp>

#include "data/image.hpp"
#include "data/io_factory.hpp"
#include "util/log.hpp"

namespace isis
{
namespace test
{

BOOST_AUTO_TEST_CASE ( imageNameGenTest )
{
	data::MemChunk<uint8_t> ch( 5, 5, 5 );
	ch.setValueAs( "indexOrigin", util::fvector3( 0, 0, 2 ) );
	ch.setValueAs<uint32_t>( "acquisitionNumber", 0 );
	ch.setValueAs<float>( "acquisitionTime", 1234 );
	ch.setValueAs( "rowVec", util::fvector3( 1, 0 ) );
	ch.setValueAs( "columnVec", util::fvector3( 0, 1 ) );
	ch.setValueAs( "voxelSize", util::fvector3( 1, 1, 1 ) );

	data::Image img( ch );

	BOOST_REQUIRE( img.isClean() );
	BOOST_REQUIRE( img.isValid() );

	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{acquisitionNumber}.nii" ), "/tmp/S0.nii" );
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{nich da}.nii" ), "/tmp/S{nich da}.nii" ); // {nich da} does not exist - so we do not touch it
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{acquisitionNumber}_acq{acquisitionTime}.nii" ), "/tmp/S0_acq1234.nii");
}

BOOST_AUTO_TEST_CASE ( imageNameUseFormatTest )
{
	data::MemChunk<uint8_t> ch( 5, 32, 1 );
	ch.setValueAs( "indexOrigin", util::fvector3( 7, 3, 2 ) );
	ch.setValueAs<uint32_t>( "acquisitionNumber", 0 );
	ch.setValueAs<float>( "acquisitionTime", 0 );
	ch.setValueAs( "rowVec", util::fvector3( 1, 0 ) );
	ch.setValueAs( "columnVec", util::fvector3( 0, 1 ) );
	ch.setValueAs( "voxelSize", util::fvector3( 1, 1, 1 ) );
	ch.setValueAs( "subjectName", std::string( "My Name is Bunny" ) );
	ch.setValueAs( "anotherName", std::string( "x" ) );

	data::Image img( ch );

	BOOST_REQUIRE( img.isClean() );
	BOOST_REQUIRE( img.isValid() );

	// check uint32 property
	// no format string
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{acquisitionNumber}.nii" ), "/tmp/S0.nii" );
	// minimum 0 with format
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{%d_acquisitionNumber}.nii" ), "/tmp/S0.nii" );
	// some value  with format
	img.setValueAs<uint32_t>( "acquisitionNumber", 123 );
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{%d_acquisitionNumber}.nii" ), "/tmp/S0000000123.nii" );
	// maximum with format
	img.setValueAs<uint32_t>( "acquisitionNumber", 4294967295 );
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{%d_acquisitionNumber}.nii" ), "/tmp/S4294967295.nii" );

	// check int32 property
	// no format string
	img.setValueAs<int32_t>( "myInt32Prop", 0 );
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{myInt32Prop}.nii" ), "/tmp/S0.nii" );
	// minimum 0 with format
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{%d_myInt32Prop}.nii" ), "/tmp/S0000000000.nii" );
	// some value  with format
	img.setValueAs<int32_t>( "myInt32Prop", 9732640 );
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{%d_myInt32Prop}.nii" ), "/tmp/S0009732640.nii" );
	// maximum  with format
	img.setValueAs<int32_t>( "myInt32Prop", 2147483647 );
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{%d_myInt32Prop}.nii" ), "/tmp/S2147483647.nii" );

	// check uint16 property
	// no format string
	img.setValueAs<uint16_t>( "myUInt16Prop", 0 );
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{myUInt16Prop}.nii" ), "/tmp/S0.nii" );
	// minimum 0 with format
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{%d_myUInt16Prop}.nii" ), "/tmp/S00000.nii" );
	// some value  with format
	img.setValueAs<uint16_t>( "myUInt16Prop", 2310 );
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{%d_myUInt16Prop}.nii" ), "/tmp/S02310.nii" );
	// maximum  with format
	img.setValueAs<uint16_t>( "myUInt16Prop", 65535 );
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{%d_myUInt16Prop}.nii" ), "/tmp/S65535.nii" );

	// check int16 property
	img.setValueAs<int16_t>( "myInt16Prop", 0 );
	// no format string
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{myInt16Prop}.nii" ), "/tmp/S0.nii" );
	// minimum 0 with format
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{%d_myInt16Prop}.nii" ), "/tmp/S00000.nii" );
	// some value  with format
	img.setValueAs<int16_t>( "myInt16Prop", 4 );
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{%d_myInt16Prop}.nii" ), "/tmp/S00004.nii" );
	// maximum  with format
	img.setValueAs<int16_t>( "myInt16Prop", 32767 );
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{%d_myInt16Prop}.nii" ), "/tmp/S32767.nii" );

	// check uint8 property
	// no format string
	img.setValueAs<uint8_t>( "myUInt8Prop", 0 );
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{myUInt8Prop}.nii" ), "/tmp/S0.nii" );
	// minimum 0 with format
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{%d_myUInt8Prop}.nii" ), "/tmp/S000.nii" );
	// some value  with format
	img.setValueAs<uint8_t>( "myUInt8Prop", 110 );
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{%d_myUInt8Prop}.nii" ), "/tmp/S110.nii" );
	// maximum  with format
	img.setValueAs<uint8_t>( "myUInt8Prop", 255 );
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{%d_myUInt8Prop}.nii" ), "/tmp/S255.nii" );

	// check int8 property
	img.setValueAs<int8_t>( "myInt8Prop", 0 );
	// no format string
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{myInt8Prop}.nii" ), "/tmp/S0.nii" );
	// minimum 0 with format
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{%d_myInt8Prop}.nii" ), "/tmp/S000.nii" );
	// some value  with format
	img.setValueAs<int8_t>( "myInt8Prop", 17 );
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{%d_myInt8Prop}.nii" ), "/tmp/S017.nii" );
	// maximum  with format
	img.setValueAs<int8_t>( "myInt8Prop", 127 );
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{%d_myInt8Prop}.nii" ), "/tmp/S127.nii" );


	// not formattable properties - shall be ignored
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/{subjectName}.nii" ), image_io::FileFormat::makeFilename( img, "/tmp/{%d_subjectName}.nii" ) );
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/{indexOrigin}.nii" ), image_io::FileFormat::makeFilename( img, "/tmp/{%d_indexOrigin}.nii" ) );

	// formattable and non formattable - doesn't influence each other
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/{%d_subjectName}_{%d_acquisitionNumber}.nii" ), "/tmp/My_Name_is_Bunny_4294967295.nii" ) ;
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/{%d_nichda}_{%d_acquisitionNumber}.nii" ), "/tmp/_4294967295.nii" ) ;
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/{%d_anotherName}_{%d_myUInt16Prop}.nii" ), "/tmp/x_65535.nii" ) ;

}

BOOST_AUTO_TEST_CASE ( imageUniqueName )
{
	data::MemChunk<uint8_t> ch1( 5, 5, 5 );
	std::list<data::Chunk> chunks;

	ch1.setValueAs( "indexOrigin", util::fvector3( 0, 0, 2 ) );
	ch1.setValueAs<uint32_t>( "acquisitionNumber", 0 );
	ch1.setValueAs<float>( "acquisitionTime", 0 );
	ch1.setValueAs( "rowVec", util::fvector3( 1, 0 ) );
	ch1.setValueAs( "columnVec", util::fvector3( 0, 1 ) );
	ch1.setValueAs( "voxelSize", util::fvector3( 1, 1, 1 ) );

	for( uint32_t i = 0; i < 5; i++ ) { // make some copies of ch1 - change their acquisitionNumber and put the into the list
		data::MemChunk<uint8_t> ch( ch1 );
		ch.setValueAs<uint32_t>( "acquisitionNumber", i );
		chunks.push_back( ch );
	}


	std::list<data::Image> images = data::IOFactory::chunkListToImageList( chunks );

	BOOST_REQUIRE_EQUAL( images.size(), 5 );
	//  int number = 0;

	// @todo this needs an actual io-plugin
	/*  std::list<std::string> names=image_io::FileFormat::makeUniqueFilenames(images,"/tmp/S{acquisitionNumber}.nii");
	    for(const std::string &ref : names){
	        BOOST_REQUIRE_EQUAL(ref,std::string("/tmp/S.nii").insert(6,util::Value<uint32_t>(number++).toString(false)));
	    }*/
}

}
}
