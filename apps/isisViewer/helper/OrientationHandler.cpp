#include "OrientationHandler.hpp"


namespace isis {
namespace viewer {
	
size_t OrientationHandler::getSliceIndex( const ImageHolder &image, OrientationHandler::PlaneOrientation orientation )
{
	switch (orientation) {
		case axial:
			return getBiggestVecElem<float>( image.getPropMap().getPropertyAs<util::fvector4>("sliceVec") );
			break;
		case sagittal:
			return getBiggestVecElem<float>( image.getPropMap().getPropertyAs<util::fvector4>("columnVec") );
			break;
		case coronal:
			return getBiggestVecElem<float>( image.getPropMap().getPropertyAs<util::fvector4>("rowVec") );
			break;
	}
}

size_t OrientationHandler::getNumberOfSlices( const ImageHolder &image, OrientationHandler::PlaneOrientation orientation )
{
	return image.getImageSize()[ getSliceIndex( image, orientation ) ];
}

util::FixedVector<float,3> OrientationHandler::getNormalizedScaling( const ImageHolder &image )
{
	util::FixedVector<float, 3> retScaling;
	util::fvector4 size = image.getImageSize();
	util::fvector4 scaling = image.getPropMap().getPropertyAs<util::fvector4>("voxelSize");
	util::fvector4 physicalSize;
	for ( size_t index = 0; index < 3; index++ )
	{
		physicalSize[index] = size[index] * scaling[index];
	}	
	size_t biggestExtent = getBiggestVecElem<float>(physicalSize);
	for ( size_t index = 0; index < 3; index++ )
	{
		retScaling[index] = 1.0 / physicalSize[biggestExtent] * physicalSize[index];
	}
	return retScaling;
}

OrientationHandler::MatrixType OrientationHandler::getOrientationMatrix( const ImageHolder &image, PlaneOrientation orientation, bool scaling )
{
	
	MatrixType retMat = boost::numeric::ublas::zero_matrix<float>(4,4);	
	retMat(3,3) = 1;
	util::fvector4 rowVec = image.getPropMap().getPropertyAs<util::fvector4>("rowVec");
	util::fvector4 columnVec = image.getPropMap().getPropertyAs<util::fvector4>("columnVec");
	util::fvector4 sliceVec = image.getPropMap().getPropertyAs<util::fvector4>("sliceVec");
	for ( size_t i = 0; i < 3; i++ ) {
		retMat( i, 0 ) = rowVec[i] < 0 ? ceil( rowVec[i] - 0.5 ) : floor( rowVec[i] + 0.5 );
	}
	for ( size_t i = 0; i < 3; i++ ) {
		retMat( i, 1 ) = columnVec[i] < 0 ? ceil( columnVec[i] - 0.5 ) : floor( columnVec[i] + 0.5 );
	}
	for ( size_t i = 0; i < 3; i++ ) {
		retMat( i, 2 ) = sliceVec[i] < 0 ? ceil( sliceVec[i] - 0.5 ) : floor( sliceVec[i] + 0.5 );
	}
	if (scaling) {
		MatrixType scaleMatrix = boost::numeric::ublas::identity_matrix<float>(4,4);
		util::FixedVector<float, 3> scaling = getNormalizedScaling(image);
		scaleMatrix(0,0) = scaling[0];
		scaleMatrix(1,1) = scaling[1];
		scaleMatrix(2,2) = scaling[2];
		retMat = boost::numeric::ublas::prod( retMat, scaleMatrix );
	}
	
	retMat = transformMatrix( retMat, orientation );
	//TODO debug
// 	std::cout << retMat << std::endl;
	return retMat;
}


OrientationHandler::MatrixType OrientationHandler::transformMatrix(OrientationHandler::MatrixType origMatrix, PlaneOrientation orientation)
{
	MatrixType transformMatrix = boost::numeric::ublas::identity_matrix<float>(4,4);
	
	switch ( orientation )
	{
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

void OrientationHandler::boostMatrix2Pointer( MatrixType boostMatrix, float* ret )
{
	size_t index = 0;
	for (size_t i = 0; i<4;i++)
	{
		for (size_t j = 0; j< 4;j++)
		{
			ret[index++] = boostMatrix( i, j );
		}
	}
}

OrientationHandler::MatrixType OrientationHandler::orientation2TextureMatrix( const OrientationHandler::MatrixType &origMatrix )
{
	MatrixType retMat = boost::numeric::ublas::zero_matrix<float>(4,4);
	for (size_t i = 0; i<4; i++ )
	{
		for ( size_t j = 0; j<4;j++)
		{
			retMat(i,j) = origMatrix(i,j) ? 1.0 / origMatrix(i,j) : 0;
		
		}
	}
	for (size_t i = 0; i<3; i++ )
	{
		for ( size_t j = 0; j<3;j++)
		{
			if(retMat(i,j) < 0)
			{
				retMat(3,j) = 1 + (fabs(retMat(i,j)) - 1) / 2;
			} else if (retMat(i,j) > 0)
			{
				retMat(3,j) = -1 * (fabs(retMat(i,j)) - 1) / 2;
			}
		}
	}
	
	return retMat;
}

OrientationHandler::ViewPortCoords OrientationHandler::calculateViewPortCoords(const isis::viewer::ImageHolder& image, OrientationHandler::PlaneOrientation orientation, size_t w, size_t h)
{
	ViewPortCoords retCoords;	
	float scaleH = (w < h) ? ((float)w / (float)h ) : 1;
	float scaleW = (h < w) ? ((float)h / (float)w ) : 1;
	
	retCoords.h = h * scaleH;
	retCoords.w = w * scaleW;
	retCoords.x = (w - (w*scaleW)) / 2;
	retCoords.y = (h - (h*scaleH)) / 2;
	return retCoords;
}


}} // end namespace