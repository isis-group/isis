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

OrientationHandler::VertexMatrix OrientationHandler::getVertexMatrix( const ImageHolder &image, size_t slice, PlaneOrientation orientation )
{
	VertexMatrix retMat;
	util::fvector4 rowVec = image.getPropMap().getPropertyAs<util::fvector4>("rowVec");
	util::fvector4 columnVec = image.getPropMap().getPropertyAs<util::fvector4>("columnVec");
	util::fvector4 sliceVec = image.getPropMap().getPropertyAs<util::fvector4>("sliceVec");
	float normSlice = ( 1.0 / image.getImageSize()[ getSliceIndex( image, orientation ) ] ) * slice;
	switch ( orientation ) {
		case axial:
			for (size_t z = 0; z<4;z++)
			{
				retMat[z][2] = normSlice;
			}
			retMat[2][0] = 0.0;
			retMat[2][1] = 0.0;
			
			retMat[3][0] = 0.0;
			retMat[3][1] = 1.0;
			
			retMat[0][0] = 1.0;
			retMat[0][1] = 1.0;
			
			retMat[1][0] = 1.0;
			retMat[1][1] = 0.0;
			break;
		case coronal:
			for (size_t z = 0; z<4;z++)
			{
				retMat[z][1] = normSlice;
			}
			retMat[2][0] = 0.0;
			retMat[2][2] = 0.0;
			
			retMat[3][0] = 0.0;
			retMat[3][2] = 1.0;
			
			retMat[0][0] = 1.0;
			retMat[0][2] = 1.0;
			
			retMat[1][0] = 1.0;
			retMat[1][2] = 0.0;
			break;
		case sagittal:
			for (size_t z = 0; z<4;z++)
			{
				retMat[z][0] = normSlice;
			}
			retMat[2][1] = 0.0;
			retMat[2][2] = 0.0;
			
			retMat[3][1] = 0.0;
			retMat[3][2] = 1.0;
			
			retMat[0][1] = 1.0;
			retMat[0][2] = 1.0;
			
			retMat[1][1] = 1.0;
			retMat[1][2] = 0.0;
			break;
			
	}
	return retMat;
}
	
}} // end namespace