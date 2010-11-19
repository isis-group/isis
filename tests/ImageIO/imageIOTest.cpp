/*
* imageTest.cpp
*
*  Created on: Oct 1, 2009
*      Author: proeger
*/

#define BOOST_TEST_MODULE ImageLoadTest
#include <boost/test/included/unit_test.hpp>
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
	data::MemChunk<uint8_t> ch(5,5,5);
	data::Image img;
	ch.setProperty( "indexOrigin", util::fvector4( 0, 0, 2 ) );
	ch.setProperty<uint32_t>( "acquisitionNumber", 0 );
	ch.setProperty<float>( "acquisitionTime", 0 );
	ch.setProperty( "readVec", util::fvector4( 1, 0 ) );
	ch.setProperty( "phaseVec", util::fvector4( 0, 1 ) );
	ch.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
	BOOST_REQUIRE( img.insertChunk( ch ) );
	img.reIndex();

	BOOST_CHECK_EQUAL(image_io::FileFormat::makeFilename(img,"/tmp/S{acquisitionNumber}.nii"),"/tmp/S0.nii");
	BOOST_CHECK_EQUAL(image_io::FileFormat::makeFilename(img,"/tmp/S{nich da}.nii"),"/tmp/S.nii"); // {nich da} does not exist - so we just remove it from the string
	BOOST_CHECK_EQUAL(
		image_io::FileFormat::makeFilename(img,"/tmp/acq{acquisitionTime}.nii"),
		std::string("/tmp/acq")+img.getProperty<std::string>("acquisitionTime")+".nii" 
	);
}

BOOST_AUTO_TEST_CASE ( imageUniqueName )
{
	data::MemChunk<uint8_t> ch1(5,5,5);
	data::ChunkList chunks;
	data::Image img;
	ch1.setProperty( "indexOrigin", util::fvector4( 0, 0, 2 ) );
	ch1.setProperty<uint32_t>( "acquisitionNumber", 0 );
	ch1.setProperty<float>( "acquisitionTime", 0 );
	ch1.setProperty( "readVec", util::fvector4( 1, 0 ) );
	ch1.setProperty( "phaseVec", util::fvector4( 0, 1 ) );
	ch1.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );

	for(uint32_t i=0;i<5;i++){ // make some copies of ch1 - change their acquisitionNumber and put the into the list
		boost::shared_ptr<data::Chunk> ch(new data::MemChunk<uint8_t>(ch1));
		ch->setProperty<uint32_t>( "acquisitionNumber", i );
		chunks.push_back(ch);
	}
	data::ImageList images(chunks);

	BOOST_REQUIRE_EQUAL(images.size(),5);int number=0;
/*	BOOST_FOREACH(const std::string &ref,image_io::FileFormat::makeUniqueFilenames(images,"/tmp/S{acquisitionNumber}.nii")){
		BOOST_REQUIRE_EQUAL(ref,std::string("/tmp/S.nii").insert(6,util::Type<uint32_t>(number++).toString(false)));
	}*/
}

}
}
