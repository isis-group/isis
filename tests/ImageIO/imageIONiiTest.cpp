/*
 * imageIONiiTest.cpp
 *
 *  Created on: Apr 12, 2010
 *      Author: Thomas Proeger
 */

#include <DataStorage/image.hpp>
#include <DataStorage/io_factory.hpp>
#include <CoreUtils/log.hpp>
#include <CoreUtils/tmpfile.hpp>

using namespace isis;

#define BOOST_TEST_MODULE "imageIONiiTest"
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <string>

namespace isis
{
namespace test
{
const util::Selection formCodes( "SCANNER_ANAT,ALIGNED_ANAT,TALAIRACH,MNI_152" );

BOOST_AUTO_TEST_SUITE ( imageIONii_NullTests )

BOOST_AUTO_TEST_CASE( loadsaveNullImage )
{
	//  data::enableLog<util::DefaultMsgPrint>(info);
	//  image_io::enableLog<util::DefaultMsgPrint>( info );
	util::DefaultMsgPrint::stopBelow( warning );
	util::Selection formCode = formCodes;
	formCode.set( "SCANNER_ANAT" );

	std::list<data::Image> images = data::IOFactory::load( "nix.null" );
	BOOST_REQUIRE( images.size() >= 1 );

	BOOST_FOREACH( data::Image & null, images ) {

		// adapt additional/non supported properties
		null.setPropertyAs( "nifti/qform_code", formCode );
		null.remove( "performingPhysician" );

		util::TmpFile niifile( "", ".nii" );
		BOOST_REQUIRE( data::IOFactory::write( null, niifile.file_string() ) );

		// nifti does not know voxelGap - so some other properties have to be modified
		null.propertyValue( "voxelSize" ).castTo<util::fvector4>() += null.propertyValue( "voxelGap" ).castTo<util::fvector4>();
		null.remove( "voxelGap" );

		// that will be set by the nifti reader
		const std::pair<util::ValueReference, util::ValueReference> minmax = null.getMinMax();
		null.setPropertyAs( "nifti/cal_min", minmax.first->as<float>() );
		null.setPropertyAs( "nifti/cal_max", minmax.second->as<float>() );

		std::list< data::Image > niftilist = data::IOFactory::load( niifile.file_string() );
		BOOST_REQUIRE( niftilist.size() == 1 );
		data::Image &nii = niftilist.front();
		nii.spliceDownTo( ( data::dimensions )null.getChunk( 0, 0 ).getRelevantDims() ); // make the chunks of the nifti have the same dimensionality as the origin

		std::vector< data::Chunk > niiChunks = nii.copyChunksToVector();
		std::vector< data::Chunk > nullChunks = null.copyChunksToVector();

		BOOST_REQUIRE_EQUAL( niiChunks.size(), nullChunks.size() );

		for( size_t i = 0; i < niiChunks.size(); i++ ) { // compare the chunks
			niiChunks[i].remove( "source" );
			nullChunks[i].remove( "source" );

			// @todo because of the splice we get a big rounding error in indexOrigin (from summing up voxelSize)
			niiChunks[i].remove( "indexOrigin" );
			nullChunks[i].remove( "indexOrigin" );

			// nifti cannot store sequenceNumber - so its allways "0"
			niiChunks[i].remove( "sequenceNumber" );
			nullChunks[i].remove( "sequenceNumber" );

			// because of the quaternions we get some rounding errors in rowVec and columnVec
			const char *fuzzies[] = {"rowVec", "columnVec", "voxelSize"};
			BOOST_FOREACH( const char * fuzz, fuzzies ) {
				const util::fvector4 niiVec = niiChunks[i].getPropertyAs<util::fvector4>( fuzz );
				const util::fvector4 nullVec = nullChunks[i].getPropertyAs<util::fvector4>( fuzz );
				BOOST_REQUIRE( niiVec.fuzzyEqual( nullVec ) );
				niiChunks[i].remove( fuzz );
				nullChunks[i].remove( fuzz );
			}

			const util::PropertyMap::DiffMap diff = niiChunks[i].getDifference( nullChunks[i] );

			if( !diff.empty() )
				std::cout << diff << std::endl;

			BOOST_REQUIRE( diff.empty() );
		}

	}
}

BOOST_AUTO_TEST_CASE( loadsaveSFormImage )
{
	const size_t tsize[] = {128, 128, 2, 1};
	util::vector4<size_t> size ( tsize );
	util::Selection aligned = formCodes;
	aligned.set( "ALIGNED_ANAT" );

	data::MemChunk<short> ch( size[0], size[1] );
	ch.setPropertyAs( "indexOrigin", util::fvector4( 0, 0, 0 ) );
	ch.setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
	ch.setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
	ch.setPropertyAs( "sliceVec", util::fvector4( 0, 0, 1 ) );
	ch.setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );

	ch.setPropertyAs( "acquisitionNumber", ( uint32_t )0 );
	ch.setPropertyAs( "sequenceNumber", ( uint16_t )0 );
	ch.setPropertyAs( "acquisitionTime", ( float )0 );


	std::list<data::MemChunk<float> > chunks( 2, ch ); //make a list with two copies of that
	chunks.back().setPropertyAs<uint32_t>( "acquisitionNumber", 1 ); //change the acquisitionNumber of that to 1
	chunks.back().setPropertyAs<float>( "acquisitionTime", 1 );
	chunks.back().setPropertyAs( "indexOrigin", util::fvector4( 0, 0, 1 ) );

	data::Image img( chunks );
	BOOST_CHECK( img.isClean() );
	BOOST_CHECK( img.isValid() );
	img.setPropertyAs( "nifti/sform_code", aligned );
	img.setPropertyAs<std::string>( "sequenceDescription", "aligned sform" );


	BOOST_CHECK_EQUAL( img.getSizeAsVector(), size );

	util::TmpFile niifile( "", ".nii" );
	BOOST_REQUIRE( data::IOFactory::write( img, niifile.file_string() ) );

	data::Image img2 = data::IOFactory::load( niifile.file_string() ).front();


	img2.remove( "acquisitionNumber" ); //unique in the source, but since we get the back as one big chunk they are common now
	img2.remove( "source" ); //the original image obviously does not have a source

	const util::PropertyMap::DiffMap diff = img2.getDifference( img ) ;

	if( !diff.empty() )
		std::cout << diff << std::endl;

	BOOST_CHECK( diff.empty() );

}

BOOST_AUTO_TEST_SUITE_END()

}
}
