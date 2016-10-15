/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2016  Enrico Reimer <reimer@cbs.mpg.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "transform.hpp"
#include "common.hpp"
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/lu.hpp>

namespace ublas=boost::numeric::ublas;

namespace isis{
namespace math {
namespace _internal{
template <typename TYPE>
bool inverseMatrix( const ublas::matrix<TYPE> &inMatrix, ublas::matrix<TYPE> &inverse )
{
	ublas::matrix<TYPE> A( inMatrix );
	ublas::permutation_matrix<TYPE> pm( A.size1() );

	if( ublas::lu_factorize( A, pm ) != 0 ) {
		return false;
	}

	inverse.assign( ublas::identity_matrix<TYPE>( inMatrix.size1() ) );
	ublas::lu_substitute( A, pm, inverse );
	return true;
}

bool transformCoords( util::PropertyMap& propertyObject, const util::vector4< size_t > size, ublas::matrix< float > transform, bool transformCenterIsImageCenter = false )
{
	LOG_IF( !propertyObject.hasProperty( "rowVec" ) || !propertyObject.hasProperty( "columnVec" ) || !propertyObject.hasProperty( "sliceVec" )
			|| !propertyObject.hasProperty( "voxelSize" ) || !propertyObject.hasProperty( "indexOrigin" ), Debug, error )
			<< "Missing one of the properties (rowVec, columnVec, sliceVec, voxelSize, indexOrigin)";

	using namespace ublas;
	// this implementation assumes that the PropMap properties is either a
	// data::Chunk or a data::Image object. Hence it should contain the
	// properties rowVec, columnVec, sliceVec and indexOrigin.
	// get row, column and slice vector from property map
	util::fvector3 row = propertyObject.getValueAs<util::fvector3>( "rowVec" );
	util::fvector3 column = propertyObject.getValueAs<util::fvector3>( "columnVec" );
	util::fvector3 slice = propertyObject.getValueAs<util::fvector3>( "sliceVec" );
	// get index origin from property map
	util::fvector3 indexorig = propertyObject.getValueAs<util::fvector3>( "indexOrigin" );
	vector<float> origin_out = vector<float>( 3 );
	//check if we have a property "voxelGap" to prevent isis from throwing a warning "blabla"
	util::fvector3 scaling;

	if( propertyObject.hasProperty( "voxelGap" ) ) {
		scaling  = propertyObject.getValueAs<util::fvector3>( "voxelSize" ) +  propertyObject.getValueAs<util::fvector3>( "voxelGap" );
	} else {
		scaling  = propertyObject.getValueAs<util::fvector3>( "voxelSize" );
	}

	// create boost::numeric data structures
	// STEP 1 transform orientation matrix
	// input matrix
	matrix<float> R_in( 3, 3 );

	for( int i = 0; i < 3; i++ ) {
		R_in( i, 0 ) = row[i];
		R_in( i, 1 ) = column[i];
		R_in( i, 2 ) = slice[i];
	}

	matrix<float> R_out( 3, 3 );

	if( transformCenterIsImageCenter ) {
		R_out = prod( R_in, transform );
	} else {
		R_out = prod( transform, R_in );
	}

	for ( int i = 0; i < 3; i++ ) {
		row[i] = R_out( i, 0 );
		column[i] = R_out( i, 1 );
		slice[i] = R_out( i, 2 );
	}

	vector<float> origin_in( 3 );

	for( int i = 0; i < 3; i++ ) {
		origin_in( i ) = indexorig[i];
	}

	//the center of the transformation is the image center (eg. spm transformation)
	if( transformCenterIsImageCenter ) {
		matrix<float> R_in_inverse( R_in );

		if( !_internal::inverseMatrix<float>( R_in, R_in_inverse ) ) {
			LOG( Runtime, error ) << "Can not inverse orientation matrix: " << R_in;
			return false;
		}

		//we have to map the indexes of the image size into the scanner space

		vector<float> physicalSize( 3 );
		vector<float> boostScaling( 3 );

		for ( unsigned short i = 0; i < 3; i++ ) {
			physicalSize( i ) = size[i] * scaling[i];
			boostScaling( i ) = scaling[i];
		}

		// now we have to calculate the center of the image in physical space
		vector<float> half_image( 3 );

		for ( unsigned short i = 0; i < 3; i++ ) {
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
		origin_out = transformed_io + center_image;

	} else {
		origin_out = prod( transform, origin_in );
	}

	for( int i = 0; i < 3; i++ ) {
		indexorig[i] = origin_out( i );
	}

	// write modified values back into property map
	propertyObject.setValueAs( "indexOrigin", indexorig );
	propertyObject.setValueAs( "rowVec", row );
	propertyObject.setValueAs( "columnVec", column );
	propertyObject.setValueAs( "sliceVec", slice );
	return true;
}

template<typename T, size_t N> T getBiggestVecElemAbs(std::array<T,N> arr){
	return *std::max_element(
		std::begin(arr),std::end(arr),
		[](const T &a, const T &b){return std::abs(a) < std::abs(b);}
	);
}

}

bool transformCoords(data::Chunk& chk, ublas::matrix< float > transform_matrix, bool transformCenterIsImageCenter)
{
	//for transforming we have to ensure to have the below properties in our chunks
	std::set<util::PropertyMap::PropPath> propPathList;
	for( const char * prop :  {"indexOrigin", "rowVec", "columnVec", "sliceVec", "voxelSize"} ) {
		const util::PropertyMap::PropPath pPath( prop );
		if ( !chk.hasProperty ( pPath ) ) {
			LOG( Runtime, error ) << "Cannot do transformCoords without " << prop;
			return false;
		}
	}

	if( !_internal::transformCoords( chk, chk.getSizeAsVector(), transform_matrix, transformCenterIsImageCenter ) ) {
		LOG( Runtime, error ) << "Error during transforming the coords of the chunk.";
		return false;
	}

	return true;
}

bool transformCoords(data::Image& img, ublas::matrix< float > transform_matrix, bool transformCenterIsImageCenter)
{
#pragma message("test me")
	// we transform an image by transforming its chunks
	std::vector< data::Chunk > chunks=img.copyChunksToVector();

	for( data::Chunk &chRef :  chunks ) {
		if ( !transformCoords (chRef, transform_matrix, transformCenterIsImageCenter ) ) {
			return false;
		}
	}
	//re-build image from transformed chunks
	img=data::Image(chunks);
	return img.isClean();
}

data::dimensions mapScannerAxisToImageDimension(const data::Image &img, data::scannerAxis scannerAxes)
{
#pragma message("test me")
	ublas::matrix<float> latchedOrientation = ublas::zero_matrix<float>( 4, 4 );
	ublas::vector<float>mapping( 4 );
	latchedOrientation( _internal::getBiggestVecElemAbs(img.getValueAs<util::fvector3>("rowVec")), 0 ) = 1;
	latchedOrientation( _internal::getBiggestVecElemAbs(img.getValueAs<util::fvector3>("columnVec")), 1 ) = 1;
	latchedOrientation( _internal::getBiggestVecElemAbs(img.getValueAs<util::fvector3>("sliceVec")), 2 ) = 1;
	latchedOrientation( 3, 3 ) = 1;

	for( size_t i = 0; i < 4; i++ ) {
		mapping( i ) = i;
	}

	return static_cast<data::dimensions>( ublas::prod( latchedOrientation, mapping )( scannerAxes ) );

}


}
}
