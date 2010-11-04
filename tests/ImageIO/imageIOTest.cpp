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


}
}
