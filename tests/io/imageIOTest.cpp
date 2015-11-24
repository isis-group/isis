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

	util::PropertyMap props;

	props.setValueAs<uint32_t>( "acquisitionNumber", 0 );
	props.setValueAs( "subjectName", "doe" );
	props.setValueAs( "acquisitionTime", M_PI );
	
	// check uint32 property
	// no format string
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( props, "/tmp/S{acquisitionNumber}.nii" ), "/tmp/S0.nii" );
	// minimum 0 with format
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( props, "/tmp/S{acquisitionNumber%d}.nii" ), "/tmp/S0.nii" );
	// some value  with format
	props.setValueAs<uint32_t>( "acquisitionNumber", 123 );
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( props, "/tmp/S{acquisitionNumber%05d}.nii" ), "/tmp/S00123.nii" );
	// maximum with format
	props.setValueAs<uint32_t>( "acquisitionNumber", 4294967295 );
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( props, "/tmp/S{acquisitionNumber%05u}.nii" ), "/tmp/S4294967295.nii" );

	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( props, "/tmp/S{acquisitionTime%.4f}.nii" ), "/tmp/S3.1416.nii" );
	

	// multiple
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( props, "/tmp/{acquisitionNumber%u}_{acquisitionTime%.4f}.nii" ), "/tmp/4294967295_3.1416.nii" ) ;
}

BOOST_AUTO_TEST_CASE ( imageUniqueName )
{
	data::MemChunk<uint8_t> ch1( 5, 5, 5 );
	std::list<data::Chunk> chunks;

	ch1.setValueAs( "indexOrigin", util::fvector3( 0, 0, 2 ) );
	ch1.setValueAs( "sequenceNumber", 0 );
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
