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

namespace isis
{

namespace data
{

namespace _internal
{

void transformCoords( isis::util::PropertyMap &properties, boost::numeric::ublas::matrix<float> transform )
{
	// this implementation assumes that the PropMap properties is either a
	// data::Chunk or a data::Image object. Hence it should contain the
	// properties rowVec, columnVec, sliceVec and indexOrigin.
	// get row, column and slice vector from property map
	isis::util::fvector4 row = properties.getPropertyAs<util::fvector4>( "rowVec" );
	isis::util::fvector4 column = properties.getPropertyAs<util::fvector4>( "columnVec" );
	isis::util::fvector4 slice = properties.getPropertyAs<util::fvector4>( "sliceVec" );
	// get index origin from property map
	isis::util::fvector4 indexorig = properties.getPropertyAs<util::fvector4>( "indexOrigin" );
	// create boost::numeric data structures
	// STEP 1 transform orientation matrix
	// input matrix
	boost::numeric::ublas::matrix<float> R_in( 3, 3 );

	for( int i = 0; i < 3; i++ ) {
		R_in( i, 0 ) = row[i];
		R_in( i, 1 ) = column[i];
		R_in( i, 2 ) = slice[i];
	}

	// output matrix
	boost::numeric::ublas::matrix<float> R_out( 3, 3 );
	R_out = boost::numeric::ublas::prod( transform, R_in );

	for ( int i = 0; i < 3; i++ ) {
		row[i] = R_out( i, 0 );
		column[i] = R_out( i, 1 );
		slice[i] = R_out( i, 2 );
	}

	// STEP 2 transform index origin
	boost::numeric::ublas::vector<float> origin_in( 3 );

	for( int i = 0; i < 3; i++ ) {
		origin_in( i ) = indexorig[i];
	}

	boost::numeric::ublas::vector<float> origin_out( 3 );
	origin_out = boost::numeric::ublas::prod( transform, origin_in );

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
