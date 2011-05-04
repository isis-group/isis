/*
* imageTest.cpp
*
*  Created on: Oct 1, 2009
*      Author: proeger
*/

#define BOOST_TEST_MODULE ImageLoadTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#include "DataStorage/image.hpp"
#include "DataStorage/io_factory.hpp"
#include "CoreUtils/log.hpp"

namespace isis
{
namespace test
{

BOOST_AUTO_TEST_CASE ( imageNameGenTest )
{
	data::MemChunk<uint8_t> ch( 5, 5, 5 );
	ch.setPropertyAs( "indexOrigin", util::fvector4( 0, 0, 2 ) );
	ch.setPropertyAs<uint32_t>( "acquisitionNumber", 0 );
	ch.setPropertyAs<float>( "acquisitionTime", 0 );
	ch.setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
	ch.setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
	ch.setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );

	data::Image img( ch );

	BOOST_REQUIRE( img.isClean() );
	BOOST_REQUIRE( img.isValid() );

	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{acquisitionNumber}.nii" ), "/tmp/S0.nii" );
	BOOST_CHECK_EQUAL( image_io::FileFormat::makeFilename( img, "/tmp/S{nich da}.nii" ), "/tmp/S.nii" ); // {nich da} does not exist - so we just remove it from the string
	BOOST_CHECK_EQUAL(
		image_io::FileFormat::makeFilename( img, "/tmp/acq{acquisitionTime}.nii" ),
		std::string( "/tmp/acq" ) + img.getPropertyAs<std::string>( "acquisitionTime" ) + ".nii"
	);
}

BOOST_AUTO_TEST_CASE ( imageUniqueName )
{
	data::MemChunk<uint8_t> ch1( 5, 5, 5 );
	std::list<data::Chunk> chunks;

	ch1.setPropertyAs( "indexOrigin", util::fvector4( 0, 0, 2 ) );
	ch1.setPropertyAs<uint32_t>( "acquisitionNumber", 0 );
	ch1.setPropertyAs<float>( "acquisitionTime", 0 );
	ch1.setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
	ch1.setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
	ch1.setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );

	for( uint32_t i = 0; i < 5; i++ ) { // make some copies of ch1 - change their acquisitionNumber and put the into the list
		data::MemChunk<uint8_t> ch( ch1 );
		ch.setPropertyAs<uint32_t>( "acquisitionNumber", i );
		chunks.push_back( ch );
	}


	std::list<data::Image> images = data::IOFactory::chunkListToImageList( chunks );

	BOOST_REQUIRE_EQUAL( images.size(), 5 );
// 	int number = 0;

	// @todo this needs an actual io-plugin
	/*  std::list<std::string> names=image_io::FileFormat::makeUniqueFilenames(images,"/tmp/S{acquisitionNumber}.nii");
	    BOOST_FOREACH(const std::string &ref,names){
	        BOOST_REQUIRE_EQUAL(ref,std::string("/tmp/S.nii").insert(6,util::Value<uint32_t>(number++).toString(false)));
	    }*/
}

}
}
