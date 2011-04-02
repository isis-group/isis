#include "GLOrientationHandler.hpp"


namespace isis
{
namespace viewer
{

const unsigned short GLOrientationHandler::matrixSize = 4;

using namespace boost::numeric::ublas;


GLOrientationHandler::MatrixType GLOrientationHandler::transformToPlaneView( const MatrixType &origMatrix, PlaneOrientation orientation, bool back )
{
	MatrixType transformMatrix = identity_matrix<float>( matrixSize, matrixSize );

	switch ( orientation ) {
	case axial:
		/*setup axial matrix
		*-1  0  0
		* 0 -1  0
		* 0  0  1
		*/
		transformMatrix( 0, 0 ) = -1;
		transformMatrix( 1, 1 ) = -1;
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
		break;
	}
	if(back)
	{
		return boost::numeric::ublas::prod( trans(transformMatrix), origMatrix );
	} else {
		return boost::numeric::ublas::prod( transformMatrix, origMatrix );
	}
}


void GLOrientationHandler::boostMatrix2Pointer( const MatrixType &boostMatrix, GLdouble *ret )
{
	size_t index = 0;
	for ( size_t row = 0; row < matrixSize; row++ ) {
		for ( size_t column = 0; column < matrixSize; column++ ) {
			ret[index++] = boostMatrix( row, column);
		}
	}
}
void GLOrientationHandler::addOffset(GLOrientationHandler::MatrixType& matrix)
{
	for (size_t row = 0; row < matrixSize; row++ ) {
		for (size_t column = 0; column < matrixSize-1; column++ ) {
			if( matrix(row, column) < 0 ){
				matrix(3,column) = 1;
			}
			
		}
	}
}


/*
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
}*/


}
} // end namespace