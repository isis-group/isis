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
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <string>

namespace isis
{
namespace test
{

BOOST_AUTO_TEST_SUITE ( imageIONii_NullTests )

BOOST_AUTO_TEST_CASE( loadsaveImage )
{
	//  data::enableLog<util::DefaultMsgPrint>(info);
	//  image_io::enableLog<util::DefaultMsgPrint>( info );
	util::Selection formCode( "SCANNER_ANAT,ALIGNED_ANAT,TALAIRACH,MNI_152" );
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
		null.propertyValue( "voxelSize" )->castTo<util::fvector4>() += null.propertyValue( "voxelGap" )->castTo<util::fvector4>();
		null.remove( "voxelGap" );

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

			// because of the quaternions we get some rounding errors in rowVec and columnVec
			const char *fuzzies[] = {"rowVec", "columnVec"};
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

BOOST_AUTO_TEST_SUITE_END()

}
}
