#ifndef ORIENTATIONHANDLER_HPP
#define ORIENTATIONHANDLER_HPP

#include "ImageHolder.hpp"
#include "common.hpp"

namespace isis {
namespace viewer {
	
	
class OrientationHandler
{
public:
	enum PlaneOrientation { axial, sagittal, coronal };
	typedef util::FixedVector< util::FixedVector<float, 3>,4> VertexMatrix;
	
	static size_t getSliceIndex( const ImageHolder &image, PlaneOrientation orientation );
	static size_t getNumberOfSlices( const ImageHolder &image, PlaneOrientation orientation );
	static VertexMatrix getVertexMatrix( const ImageHolder &image, size_t slice, PlaneOrientation orientation );

	
	
};
	
}}// end namespace



#endif