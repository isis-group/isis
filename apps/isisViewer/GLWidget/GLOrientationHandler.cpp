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
void GLOrientationHandler::addOffset( GLOrientationHandler::MatrixType &matrix )
{
	for ( size_t row = 0; row < matrixSize; row++ ) {
		for ( size_t column = 0; column < matrixSize - 1; column++ ) {
			if( matrix( row, column ) < 0 ) {
				matrix( 3, column ) = 1;
			}

		}
	}
}

util::ivector4 GLOrientationHandler::transformObject2VoxelCoords( const util::fvector4 objectCoords, const isis::viewer::ImageHolder &image, GLOrientationHandler::PlaneOrientation orientation )
{
	GLOrientationHandler::MatrixType planeOrientatioMatrix =
		GLOrientationHandler::transformToPlaneView( image.getNormalizedImageOrientation(), orientation );
	MatrixType objectCoordsMatrix = zero_matrix<float>( matrixSize, 1 );
	objectCoordsMatrix( 0, 0 ) = ( objectCoords[0] + 1 ) / 2;
	objectCoordsMatrix( 1, 0 ) = ( objectCoords[1] + 1 ) / 2;
	objectCoordsMatrix( 2, 0 ) = objectCoords[2];
	MatrixType transformedObjectCoords = prod( trans( planeOrientatioMatrix ), objectCoordsMatrix );
	short voxelx =  image.getImageSize()[0] * transformedObjectCoords( 0, 0 );
	short voxely =  image.getImageSize()[1] * transformedObjectCoords( 1, 0 );
	short voxelz =  image.getImageSize()[2] * transformedObjectCoords( 2, 0 );
	voxelx = voxelx < 0 ? image.getImageSize()[0] + voxelx - 1 : voxelx;
	voxely = voxely < 0 ? image.getImageSize()[1] + voxely - 1 : voxely;
	voxelz = voxelz < 0 ? image.getImageSize()[2] + voxelz - 1 : voxelz;
	return util::ivector4( voxelx, voxely, voxelz );

}

util::fvector4 GLOrientationHandler::transformVoxel2ObjectCoords( const isis::util::ivector4 voxelCoords, const isis::viewer::ImageHolder &image, GLOrientationHandler::PlaneOrientation orientation )
{
	GLOrientationHandler::MatrixType planeOrientatioMatrix =
		GLOrientationHandler::transformToPlaneView( image.getNormalizedImageOrientation(), orientation );
	MatrixType objectCoords = zero_matrix<float>( matrixSize, 1 );
	MatrixType oneHalfVoxel = zero_matrix<float>( matrixSize, 1 );

	for ( unsigned short i = 0; i < 3; i++ ) {
		objectCoords( i, 0 ) = ( 1.0 / image.getImageSize()[i] ) * voxelCoords[i];
		oneHalfVoxel( i, 0 ) = 0.5 / image.getImageSize()[i];
	}

	MatrixType transformedObjectCoords = prod( planeOrientatioMatrix, objectCoords );
	MatrixType transformedOneHalfVoxel = prod( planeOrientatioMatrix, oneHalfVoxel );
	util::fvector4 retVec;
	retVec[0] = transformedObjectCoords( 0, 0 ) < 0 ? 1.0 + 2 * ( transformedObjectCoords( 0, 0 ) + transformedOneHalfVoxel( 0, 0 ) ) : -1.0 + 2 * ( transformedObjectCoords( 0, 0 ) + transformedOneHalfVoxel( 0, 0 ) );
	retVec[1] = transformedObjectCoords( 1, 0 ) < 0 ? 1.0 + 2 * ( transformedObjectCoords( 1, 0 ) + transformedOneHalfVoxel( 1, 0 ) ) : -1.0 + 2 * ( transformedObjectCoords( 1, 0 ) + transformedOneHalfVoxel( 1, 0 ) );
	retVec[2] = transformedObjectCoords( 2, 0 ) < 0 ? 1.0 + transformedObjectCoords( 2, 0 ) + transformedOneHalfVoxel( 2, 0 )  : transformedObjectCoords( 2, 0 ) + transformedOneHalfVoxel( 2, 0 );
	return retVec;

}


void GLOrientationHandler::recalculateViewport( size_t w, size_t h, const ImageHolder &image, const MatrixType &orientation, GLint *viewport, size_t border )
{
	//first we have to map the imagesize and scaling to our current planeview
	MatrixType imageSize = zero_matrix<float>( matrixSize, 1 );
	MatrixType imageScaling = zero_matrix<float>( matrixSize, 1 );
	util::fvector4 scaling = image.getPropMap().getPropertyAs<util::fvector4>( "voxelSize" ) + image.getPropMap().getPropertyAs<util::fvector4>( "voxelGap" );

	for ( unsigned short i = 0; i < 3; i++ ) {
		imageSize( i, 0 ) = image.getImageSize()[i];
		imageScaling( i, 0 ) = scaling[i];
	}

	MatrixType transformedScaling = prod( orientation, imageScaling );
	MatrixType transformedSize = prod( orientation, imageSize );
	util::fvector4 physicalSize;

	for ( unsigned short i = 0; i < 3; i++ ) {
		physicalSize[i] = transformedScaling( i, 0 ) * transformedSize( i, 0 );
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


}
} // end namespace