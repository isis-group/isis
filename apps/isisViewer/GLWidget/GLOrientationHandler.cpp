#include "GLOrientationHandler.hpp"


namespace isis
{
namespace viewer
{
util::fvector4 GLOrientationHandler::transformWithImageOrientation(const isis::viewer::ImageHolder& image, util::fvector4 oldVec)
	{
		util::fvector4 sliceVec = image.getPropMap().getPropertyAs<util::fvector4>("sliceVec");
		util::fvector4 rowVec = image.getPropMap().getPropertyAs<util::fvector4>("rowVec");
		util::fvector4 columnVec = image.getPropMap().getPropertyAs<util::fvector4>("columnVec");	
		boost::numeric::ublas::matrix<float> orient = boost::numeric::ublas::zero_matrix<float>(3,3);
		boost::numeric::ublas::matrix<float> coords = boost::numeric::ublas::zero_matrix<float>(3,1);
		boost::numeric::ublas::matrix<float> retCoords = boost::numeric::ublas::zero_matrix<float>(3,1);
		for (size_t i = 0; i < 3; i++)
		{
			orient( i, 0 ) = rowVec[i] < 0 ? ceil( rowVec[i] - 0.5 ) : floor( rowVec[i] + 0.5 );
			orient( i, 1 ) = columnVec[i] < 0 ? ceil( columnVec[i] - 0.5 ) : floor( columnVec[i] + 0.5 );
			orient( i, 2 ) = sliceVec[i] < 0 ? ceil( sliceVec[i] - 0.5 ) : floor( sliceVec[i] + 0.5 );
		}
		coords(0,0) = oldVec[0];
		coords(1,0) = oldVec[1];
		coords(2,0) = oldVec[2];
		retCoords = boost::numeric::ublas::prod(orient, coords);
		util::fvector4 retVec;
		retVec[0] = retCoords(0,0);
		retVec[1] = retCoords(1,0);
		retVec[2] = retCoords(2,0);
		return retVec;
	}


util::FixedVector<float, 3> GLOrientationHandler::getNormalizedScaling( const ImageHolder &image )
{
	util::FixedVector<float, 3> retScaling;
	util::fvector4 size = image.getImageSize();
	util::fvector4 scaling = image.getPropMap().getPropertyAs<util::fvector4>( "voxelSize" );
	util::fvector4 voxelGap = image.getPropMap().getPropertyAs<util::fvector4>( "voxelGap" );
	util::fvector4 physicalSize;

	for ( size_t index = 0; index < 3; index++ ) {
		physicalSize[index] = size[index] * ( scaling[index] + voxelGap[index]);
	}

	size_t biggestExtent = getBiggestVecElem<float>( physicalSize );

	for ( size_t index = 0; index < 3; index++ ) {
		retScaling[index] = 1.0 / physicalSize[biggestExtent] * physicalSize[index];
	}
	return retScaling;
}


GLOrientationHandler::MatrixType GLOrientationHandler::getOrientationMatrix( const ImageHolder &image, PlaneOrientation orientation, bool scaling )
{

	MatrixType retMat = boost::numeric::ublas::zero_matrix<float>( 4, 4 );
	retMat( 3, 3 ) = 1;
	util::fvector4 rowVec = image.getPropMap().getPropertyAs<util::fvector4>( "rowVec" );
	util::fvector4 columnVec = image.getPropMap().getPropertyAs<util::fvector4>( "columnVec" );
	util::fvector4 sliceVec = image.getPropMap().getPropertyAs<util::fvector4>( "sliceVec" );

	for ( size_t i = 0; i < 3; i++ ) {
		retMat( i, 0 ) = rowVec[i] < 0 ? ceil( rowVec[i] - 0.5 ) : floor( rowVec[i] + 0.5 );
	}

	for ( size_t i = 0; i < 3; i++ ) {
		retMat( i, 1 ) = columnVec[i] < 0 ? ceil( columnVec[i] - 0.5 ) : floor( columnVec[i] + 0.5 );
	}

	for ( size_t i = 0; i < 3; i++ ) {
		retMat( i, 2 ) = sliceVec[i] < 0 ? ceil( sliceVec[i] - 0.5 ) : floor( sliceVec[i] + 0.5 );
	}

	if ( scaling ) {
		MatrixType scaleMatrix = boost::numeric::ublas::identity_matrix<float>( 4, 4 );
		util::FixedVector<float, 3> scaling = getNormalizedScaling( image );
		scaleMatrix( 0, 0 ) = scaling[0];
		scaleMatrix( 1, 1 ) = scaling[1];
		scaleMatrix( 2, 2 ) = scaling[2];
		retMat = boost::numeric::ublas::prod( retMat, scaleMatrix );
	}

	retMat = transformToView( retMat, orientation );
	//TODO debug
	//  std::cout << retMat << std::endl;
	return retMat;
}


GLOrientationHandler::MatrixType GLOrientationHandler::transformToView( GLOrientationHandler::MatrixType origMatrix, PlaneOrientation orientation )
{
	MatrixType transformMatrix = boost::numeric::ublas::identity_matrix<float>( 4, 4 );

	switch ( orientation ) {
	case axial:
		/*setup axial matrix
		*-1  0  0
		* 0 -1  0
		* 0  0  1
		*/
		transformMatrix( 0, 0 ) = -1;
		transformMatrix( 1, 1 ) = -1;
		return boost::numeric::ublas::prod( transformMatrix, origMatrix );
		break;
	case sagittal:
		/*setup sagittal matrix
		* 0  1  0
		* 0  0  1
		* 1  0  0
		*/
		transformMatrix( 0, 0 ) = 0;
		transformMatrix( 2, 0 ) = 1;
		transformMatrix( 0, 1 ) = 1;
		transformMatrix( 2, 2 ) = 0;
		transformMatrix( 1, 2 ) = 1;
		transformMatrix( 1, 1 ) = 0;
		return boost::numeric::ublas::prod( transformMatrix, origMatrix );
		break;
	case coronal:
		/*setup coronal matrix
		* -1  0  0
		*  0  0  1
		*  0  1  0
		*/

		transformMatrix( 0, 0 ) = -1;
		transformMatrix( 1, 1 ) = 0;
		transformMatrix( 2, 2 ) = 0;
		transformMatrix( 2, 1 ) = 1;
		transformMatrix( 1, 2 ) = 1;
		return boost::numeric::ublas::prod( transformMatrix, origMatrix );
		break;
	}
}

void GLOrientationHandler::boostMatrix2Pointer( MatrixType boostMatrix, float *ret )
{
	size_t index = 0;

	for ( size_t i = 0; i < 4; i++ ) {
		for ( size_t j = 0; j < 4; j++ ) {
			ret[index++] = boostMatrix( i, j );
		}
	}
}

GLOrientationHandler::MatrixType GLOrientationHandler::orientation2TextureMatrix( const GLOrientationHandler::MatrixType &origMatrix )
{
	MatrixType retMat = boost::numeric::ublas::zero_matrix<float>( 4, 4 );

	for ( size_t i = 0; i < 4; i++ ) {
		for ( size_t j = 0; j < 4; j++ ) {
			retMat( i, j ) = origMatrix( i, j ) ? 1.0 / origMatrix( i, j ) : 0;

		}
	}
	for ( size_t i = 0; i < 3; i++ ) {
		for ( size_t j = 0; j < 3; j++ ) {
			if( retMat( i, j ) < 0 ) {
				retMat( 3, j ) = 1 + ( fabs( retMat( i, j ) ) - 1 ) / 2;
			} else if ( retMat( i, j ) > 0 ) {
				retMat( 3, j ) = -1 * ( fabs( retMat( i, j ) ) - 1 ) / 2;
			}
		}
	}
	return retMat;
}



GLOrientationHandler::ViewPortCoords GLOrientationHandler::calculateViewPort( size_t w, size_t h )
{
	ViewPortCoords retCoords;
	float scaleH = ( w < h ) ? ( ( float )w / ( float )h ) : 1;
	float scaleW = ( h < w ) ? ( ( float )h / ( float )w ) : 1;

	retCoords.h = h * scaleH;
	retCoords.w = w * scaleW;
	retCoords.x = ( w - ( w * scaleW ) ) / 2;
	retCoords.y = ( h - ( h * scaleH ) ) / 2;
	return retCoords;
}


}
} // end namespace