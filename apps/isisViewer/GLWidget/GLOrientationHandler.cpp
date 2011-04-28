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

	if( back ) {
		return boost::numeric::ublas::prod( trans( transformMatrix ), origMatrix );
	} else {
		return boost::numeric::ublas::prod( transformMatrix, origMatrix );
	}
}


void GLOrientationHandler::boostMatrix2Pointer( const MatrixType &boostMatrix, GLdouble *ret )
{
	size_t index = 0;

	for ( size_t row = 0; row < matrixSize; row++ ) {
		for ( size_t column = 0; column < matrixSize; column++ ) {
			ret[index++] = boostMatrix( row, column );
		}
	}
}
GLOrientationHandler::MatrixType GLOrientationHandler::addOffset( const GLOrientationHandler::MatrixType &matrix )
{
	MatrixType retMat = matrix;

	for ( size_t row = 0; row < matrixSize; row++ ) {
		for ( size_t column = 0; column < matrixSize - 1; column++ ) {
			if( matrix( row, column ) < 0 ) {
				retMat( 3, column ) = 1;
			}

		}
	}

	return retMat;
}

util::ivector4 GLOrientationHandler::transformObject2VoxelCoords( const util::fvector4 objectCoords, const boost::shared_ptr<ImageHolder> image, PlaneOrientation orientation )
{
	GLOrientationHandler::MatrixType planeOrientatioMatrix =
		GLOrientationHandler::transformToPlaneView( image->getNormalizedImageOrientation(), orientation );
	VectorType objectCoordsMatrix ( matrixSize );
	objectCoordsMatrix( 0 ) = ( objectCoords[0] + 1 ) / 2;
	objectCoordsMatrix( 1 ) = ( objectCoords[1] + 1 ) / 2;
	objectCoordsMatrix( 2 ) = objectCoords[2];
	VectorType oneVector( matrixSize );

	for( size_t i = 0; i < matrixSize; i++ ) {
		oneVector( i ) = 1;
	}

	VectorType direction = prod ( trans( planeOrientatioMatrix ), oneVector );
	VectorType transformedObjectCoords = prod( trans( planeOrientatioMatrix ), objectCoordsMatrix );
	short voxelx =  image->getImageSize()[0] * transformedObjectCoords( 0 );
	short voxely =  image->getImageSize()[1] * transformedObjectCoords( 1 );
	short voxelz =  image->getImageSize()[2] * transformedObjectCoords( 2 );
	voxelx = direction( 0 ) < 0 ? image->getImageSize()[0] + voxelx - 1 : voxelx;
	voxely = direction( 1 ) < 0 ? image->getImageSize()[1] + voxely - 1 : voxely;
	voxelz = direction( 2 ) < 0 ? image->getImageSize()[2] + voxelz - 1 : voxelz;
	return util::ivector4( voxelx, voxely, voxelz );

}

util::dvector4 GLOrientationHandler::transformVoxel2ObjectCoords( const isis::util::ivector4 voxelCoords, const boost::shared_ptr<ImageHolder> image, MatrixType orientation )
{
	util::dvector4 objectCoords;
	util::dvector4 oneHalfVoxel;

	for ( unsigned short i = 0; i < 3; i++ ) {
		objectCoords[i] = ( 1.0 / image->getImageSize()[i] ) * voxelCoords[i];
		oneHalfVoxel[i] = 0.5 / image->getImageSize()[i];
	}

	util::dvector4 transformedObjectCoords = transformVector<float>( objectCoords, orientation );
	util::dvector4 transformedOneHalfVoxel = transformVector<float>( oneHalfVoxel, orientation );
	util::dvector4 retVec;
	retVec[0] = transformedObjectCoords[0] <= 0 ? 1.0 + 2 * ( transformedObjectCoords[0] + transformedOneHalfVoxel[0] ) : -1.0 + 2 * ( transformedObjectCoords[0] + transformedOneHalfVoxel[0] );
	retVec[1] = transformedObjectCoords[1] <= 0 ? 1.0 + 2 * ( transformedObjectCoords[1] + transformedOneHalfVoxel[1] ) : -1.0 + 2 * ( transformedObjectCoords[1] + transformedOneHalfVoxel[1] );
	retVec[2] = transformedObjectCoords[2] <= 0 ? 1.0 + transformedObjectCoords[2] + transformedOneHalfVoxel[2] : transformedObjectCoords[2] + transformedOneHalfVoxel[2];
	return retVec;

}


void GLOrientationHandler::recalculateViewport( size_t w, size_t h, util::fvector4 mappedVoxelSize, util::ivector4 mappedImageSize , GLint *viewport, size_t border )
{
	//first we have to map the imagesize and scaling to our current planeview
	util::fvector4 physicalSize;

	for ( unsigned short i = 0; i < 3; i++ ) {
		physicalSize[i] = mappedVoxelSize[i] * mappedImageSize[i];
	}

	size_t wspace = w - 2 * border;
	size_t hspace = h - 2 * border;
	float scalew = wspace / physicalSize[0];
	float scaleh = hspace / physicalSize[1];
	float normh = scalew < scaleh ? scalew / scaleh : 1;
	float normw = scalew > scaleh ? scaleh / scalew : 1;
	viewport[2] = wspace * normw;
	viewport[3] = hspace * normh;
	viewport[0] = ( w - viewport[2] ) / 2;
	viewport[1] = ( h - viewport[3] ) / 2;

}


void GLOrientationHandler::makeIdentity( GLdouble *matrix )
{
	unsigned short index = 0;

	for ( unsigned short row = 0; row < matrixSize; row++ ) {
		for ( unsigned short column = 0; column < matrixSize; column++ ) {
			matrix[index++] = row == column ? 1 : 0;
		}
	}
}



}
} // end namespace