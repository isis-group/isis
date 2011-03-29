#include "OrientationHandler.hpp"


namespace isis
{
namespace viewer
{
util::fvector4 OrientationHandler::transformWithImageOrientation(const isis::viewer::ImageHolder& image, util::fvector4 oldVec)
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
	
util::FixedVector<size_t, 3>  OrientationHandler::getTransformedImageSize( const ImageHolder &image )
{
	util::ivector4 coords = image.getImageSize();
	util::ivector4 transformedSize = transformWithImageOrientation(image, coords);
	util::FixedVector<size_t, 3> retSize;
	retSize[0] = abs(transformedSize[0]);
	retSize[1] = abs(transformedSize[1]);
	retSize[2] = abs(transformedSize[2]);
	return retSize;
}

OrientationHandler::ViewerCoordinates OrientationHandler::normalizeCoordinates(size_t slice, size_t x, size_t y, const isis::viewer::ImageHolder& image, const float* textureMatrix, OrientationHandler::ViewPortCoords viewport, OrientationHandler::PlaneOrientation orientation)
{
	ViewerCoordinates retCoords;
	using namespace boost::numeric::ublas;
	//here we normalize and transform the slice coords to work with our transformed matrix
	OrientationHandler::printMatrix(textureMatrix);
	size_t sliceIndex = 0;
	size_t xIndex = 0;
	size_t yIndex = 0;
	switch(orientation)
	{
		case axial:
			sliceIndex = 2;
			yIndex = 1;
			xIndex = 0;
			break;
		case sagittal:
			sliceIndex = 0;
			yIndex = 2;
			xIndex = 1;
			break;
		case coronal:
			sliceIndex = 1;
			yIndex = 2;
			xIndex = 0;
			break;
	}
	util::FixedVector<size_t, 3> transformedImageSize = getTransformedImageSize(image);
	util::fvector4 xVec = util::fvector4(textureMatrix[0], textureMatrix[1], textureMatrix[2], 0);
	util::fvector4 yVec = util::fvector4(textureMatrix[4], textureMatrix[5], textureMatrix[6], 0);
	util::fvector4 sliceVec = util::fvector4(textureMatrix[8], textureMatrix[9], textureMatrix[10], 0);
	util::fvector4 translationVec = util::fvector4( textureMatrix[12], textureMatrix[13], textureMatrix[14], 0 );
	size_t relevantIndexSlice = getBiggestVecElem<float>(sliceVec);
	size_t relevantIndexX = getBiggestVecElem<float>(xVec);
	size_t relevantIndexY = getBiggestVecElem<float>(yVec);
	
	float oneHalfSlice = 1.0 / transformedImageSize[sliceIndex] / 2;
	retCoords.slice = (1.0 / transformedImageSize[sliceIndex]) * slice;
	retCoords.slice -= translationVec[relevantIndexSlice];
	retCoords.slice *= 1.0 / sliceVec[relevantIndexSlice];
	retCoords.slice += sliceVec[relevantIndexSlice] < 0 ? -oneHalfSlice : oneHalfSlice;
	//here we adapt our x, y coords to the current viewport
	
	size_t offsetX = viewport.x + ((float)viewport.w * (
		fabs(translationVec[relevantIndexX]) >= 1 ? fabs(translationVec[relevantIndexX]) - 1 : fabs(translationVec[relevantIndexX]) ) );
	size_t offsetY = viewport.y + ((float)viewport.h * (
		fabs(translationVec[relevantIndexY]) >= 1 ? fabs(translationVec[relevantIndexY]) - 1 : fabs(translationVec[relevantIndexY]) ) );
	//we also have to transform these coords to work with our texture matrix
	retCoords.x = offsetX + ( ( (float)viewport.w * fabs(1.0 / xVec[relevantIndexX]) / (float)transformedImageSize[xIndex] ) * x);
	retCoords.y = offsetY + ( ( (float)viewport.h * fabs(1.0 / yVec[relevantIndexY]) / (float)transformedImageSize[yIndex] ) * y);
	
	return retCoords;

	
}

util::FixedVector<float, 3> OrientationHandler::getNormalizedScaling( const ImageHolder &image )
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


OrientationHandler::MatrixType OrientationHandler::getOrientationMatrix( const ImageHolder &image, PlaneOrientation orientation, bool scaling )
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

	retMat = transformMatrix( retMat, orientation );
	//TODO debug
	//  std::cout << retMat << std::endl;
	return retMat;
}


OrientationHandler::MatrixType OrientationHandler::transformMatrix( OrientationHandler::MatrixType origMatrix, PlaneOrientation orientation )
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

void OrientationHandler::boostMatrix2Pointer( MatrixType boostMatrix, float *ret )
{
	size_t index = 0;

	for ( size_t i = 0; i < 4; i++ ) {
		for ( size_t j = 0; j < 4; j++ ) {
			ret[index++] = boostMatrix( i, j );
		}
	}
}

OrientationHandler::MatrixType OrientationHandler::orientation2TextureMatrix( const OrientationHandler::MatrixType &origMatrix )
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

std::pair<size_t, size_t> OrientationHandler::voxelCoords2WidgetCoords(size_t _x, size_t _y, const isis::viewer::ImageHolder& image, OrientationHandler::PlaneOrientation orientation, OrientationHandler::ViewPortCoords viewport)
{
	util::ivector4 coords = image.getImageSize();
	util::ivector4 transformedCoords = transformWithImageOrientation(image, coords);
	std::pair<size_t, size_t> retPair;
	size_t xsize = 0;
	size_t ysize = 0;
	switch(orientation)
	{
		case axial:
			xsize = abs(transformedCoords[0]);
			ysize = abs(transformedCoords[1]);
			break;
		case sagittal:
			xsize = abs(transformedCoords[1]);
			ysize = abs(transformedCoords[2]);
			break;
		case coronal:
			xsize = abs(transformedCoords[0]);
			ysize = abs(transformedCoords[2]);
			break;
	}
	retPair.first = viewport.x + (((float)viewport.w / (float)xsize) * _x);
	retPair.second = viewport.y + (((float)viewport.h / (float)ysize) * _y);
	return retPair;
}


OrientationHandler::ViewPortCoords OrientationHandler::calculateViewPortCoords( size_t w, size_t h )
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