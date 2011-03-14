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
	
}
	
}} // end namespace