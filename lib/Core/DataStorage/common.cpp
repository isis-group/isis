//
// C++ Implementation: common (DataStorage)
//
// Description:
//
//
// Author: Thomas Pröger <proeger@cbs.mpg.de>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "common.hpp"
#include <boost/numeric/ublas/io.hpp>

namespace isis
{

namespace data
{

namespace _internal
{


void transformCoords( isis::util::PropertyMap &properties, util::FixedVector<size_t, 4> size, boost::numeric::ublas::matrix<float> transform )
{
	using namespace boost::numeric::ublas;
	// this implementation assumes that the PropMap properties is either a
	// data::Chunk or a data::Image object. Hence it should contain the
	// properties rowVec, columnVec, sliceVec and indexOrigin.
	// get row, column and slice vector from property map
	isis::util::fvector4 row = properties.getPropertyAs<util::fvector4>( "rowVec" );
	isis::util::fvector4 column = properties.getPropertyAs<util::fvector4>( "columnVec" );
	isis::util::fvector4 slice = properties.getPropertyAs<util::fvector4>( "sliceVec" );
	// get index origin from property map
	isis::util::fvector4 indexorig = properties.getPropertyAs<util::fvector4>( "indexOrigin" );
	isis::util::fvector4 scaling  = properties.getPropertyAs<util::fvector4>( "voxelSize" ) + properties.getPropertyAs<util::fvector4>( "voxelGap" );
	// create boost::numeric data structures
	// STEP 1 transform orientation matrix
	// input matrix
	matrix<float> R_in( 3, 3 );

	for( int i = 0; i < 3; i++ ) {
		R_in( i, 0 ) = row[i];
		R_in( i, 1 ) = column[i];
		R_in( i, 2 ) = slice[i];
	}

	matrix<float> R_in_inverse( R_in );
	_internal::inverseMatrix<float>( R_in, R_in_inverse );
	matrix<float> R_out( 3, 3 );
	R_out = prod( R_in, transform );

	for ( int i = 0; i < 3; i++ ) {
		row[i] = R_out( i, 0 );
		column[i] = R_out( i, 1 );
		slice[i] = R_out( i, 2 );
	}

	// STEP 2 transform index origin
	vector<float> origin_in( 3 );

	for( int i = 0; i < 3; i++ ) {
		origin_in( i ) = indexorig[i];
	}

	//we have to map the indexes of the image size into the scanner space

	vector<float> physicalSize( 3 );
	vector<float> boostScaling( 3 );

	for ( unsigned short i = 0; i < 3; i++ ) {
		physicalSize( i ) = size[i] * scaling[i];
		boostScaling( i ) = scaling[i];
	}

	vector<float>scalingToRemove( 3 );
	//  scalingToRemove = prod(R_in,boostScaling) - prod(R_out, boostScaling);

	// now we have to calculate the center of the image in physical space
	vector<float> half_image( 3 );

	for ( unsigned short i = 0; i < 3; i++ ) {
		scalingToRemove( i ) *= 0.5;
		half_image( i ) = ( physicalSize( i )  - boostScaling( i ) ) * 0.5;
	}

	vector<float> center_image = prod( R_in, half_image ) + origin_in ;

	//now translate this center to the center of the physical space and get the new image origin
	vector<float> io_translated = origin_in - center_image;
	//now multiply this translated origin with the inverse of the orientation matrix of the image
	vector<float> io_ortho = prod( R_in_inverse, io_translated );
	//now transform this matrix with the actual transformation matrix
	vector<float> transformed_io_ortho = prod( io_ortho, transform );
	//now transform ths point back with the orientation matrix of the image
	vector<float> transformed_io = prod( R_in, transformed_io_ortho );
	//and finally we have to retranslate this origin to get the image to our old position in physical space
	vector<float> origin_out = transformed_io + center_image;

	for( int i = 0; i < 3; i++ ) {
		indexorig[i] = origin_out( i );
	}

	// write modified values back into property map
	properties.setPropertyAs<util::fvector4>( "indexOrigin", indexorig );
	properties.setPropertyAs<util::fvector4>( "rowVec", row );
	properties.setPropertyAs<util::fvector4>( "columnVec", column );
	properties.setPropertyAs<util::fvector4>( "sliceVec", slice );
}

}
}
}
