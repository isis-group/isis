#ifndef ORIENTATIONHANDLER_HPP
#define ORIENTATIONHANDLER_HPP

#include "ImageHolder.hpp"
#include "common.hpp"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/concept_check.hpp>

namespace isis
{
namespace viewer
{


class OrientationHandler
{
public:
	typedef boost::numeric::ublas::matrix<float> MatrixType;
	enum PlaneOrientation { axial, sagittal, coronal };
	typedef util::FixedVector< util::FixedVector<float, 3>, 4> VertexMatrix;

	struct ViewPortCoords {
		size_t w;
		size_t h;
		size_t x;
		size_t y;
	};
	
	struct ViewerCoordinates {
		float slice;
		size_t x;
		size_t y;
	};

	///gets the amount of slices along the normal of the plane specified by orientation
	static util::FixedVector<size_t, 3> getTransformedImageSize ( const ImageHolder &image );
	
	///just a helper function to get the normalized (for texture transform) scaling of an image
	static util::FixedVector<float, 3> getNormalizedScaling( const ImageHolder &image );
	
	///yields the orientation matrix with an optional scaling
	static MatrixType getOrientationMatrix( const ImageHolder &image, PlaneOrientation orientation, bool scaling = true );
	
	///transforms the common orienation of an image into the specific orienation of one view (axial, coronal, sagittal)
	static MatrixType transformMatrix( MatrixType origMatrix, PlaneOrientation orientation );
	
	///this function converts the specific orientation matrix for each view into the texture transformation matrix which then can be applied to the opengl texture
	static MatrixType orientation2TextureMatrix( const MatrixType &origMatrix );
	
	///recalculate the size and position of the current viewport. This has to be done durring resize
	static ViewPortCoords calculateViewPortCoords( size_t w, size_t h );
	
	///transform the voxelCoords into coords that can used by opengl. 
	static ViewerCoordinates normalizeCoordinates(size_t slice, size_t x, size_t y, const ImageHolder &image, const float *textureMatrix, ViewPortCoords viewport, PlaneOrientation orientation);
	
	static util::fvector4 transformWithImageOrientation(const isis::viewer::ImageHolder& image, util::fvector4 oldVec);

	static std::pair<size_t,size_t> voxelCoords2WidgetCoords(size_t x, size_t y, const ImageHolder &image, OrientationHandler::PlaneOrientation orientation, ViewPortCoords viewport);
	
	static void  boostMatrix2Pointer( MatrixType boostMatrix, float *ret );

	static void printMatrix( const float *mat ) {
		size_t index = 0;

		for ( size_t i = 0; i < 4; i++ ) {
			for ( size_t j = 0; j < 4; j++ ) {
				std::cout << mat[index++] << " ";
			}
			std::cout << std::endl;
		}
	}


};



}
}// end namespace



#endif