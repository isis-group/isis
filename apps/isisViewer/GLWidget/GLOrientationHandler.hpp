#ifndef ORIENTATIONHANDLER_HPP
#define ORIENTATIONHANDLER_HPP

#include "ImageHolder.hpp"
#include "common.hpp"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/concept_check.hpp>

struct stat;
namespace isis
{
namespace viewer
{


class GLOrientationHandler
{
	static const unsigned short textureSize;
public:
	typedef boost::numeric::ublas::matrix<float> MatrixType;
	enum PlaneOrientation { axial, sagittal, coronal };
	typedef util::FixedVector< util::FixedVector<float, 3>, 4> TextureMatrixType;

	struct ViewPortCoords {
		size_t w;
		size_t h;
		size_t x;
		size_t y;
	};

	struct GLCoordinates {
		float slice;
		size_t x;
		size_t y;
	};

	///just a helper function to get the normalized (for texture transform) scaling of an image
	static util::FixedVector<float, 3> getNormalizedScaling( const ImageHolder &image );

	///yields the orientation matrix with an optional scaling
	static MatrixType getOrientationMatrix( const ImageHolder &image, PlaneOrientation orientation, bool scaling = true );

	///transforms the common orienation of an image into the specific orienation of one view (axial, coronal, sagittal)
	static MatrixType transformToView( MatrixType origMatrix, PlaneOrientation orientation );

	///this function converts the specific orientation matrix for each view into the texture transformation matrix which then can be applied to the opengl texture
	static MatrixType orientation2TextureMatrix( const MatrixType &origMatrix );

	///recalculate the size and position of the current viewport. This has to be done durring resize
	static ViewPortCoords calculateViewPort( size_t w, size_t h );

	static util::fvector4 transformVectorWithImageOrientation( const isis::viewer::ImageHolder &image, util::fvector4 vector );

	static util::fvector4 transformVectorWithImageAndPlaneOrientation( const ImageHolder &image, util::fvector4 vector, PlaneOrientation orientation );

	static void  boostMatrix2Pointer( MatrixType boostMatrix, float *ret );
	
	static GLCoordinates transformImageCoords2GLCoords( const util::ivector4 imageCoords, const ImageHolder &image, const ViewPortCoords viewport, PlaneOrientation orientation );

	static void printMatrix( const float *mat ) {
		size_t index = 0;

		for ( size_t i = 0; i < textureSize; i++ ) {
			for ( size_t j = 0; j < textureSize; j++ ) {
				std::cout << mat[index++] << " ";
			}

			std::cout << std::endl;
		}
	}
	static void printMatrix( const MatrixType mat, unsigned short _i = textureSize, unsigned short _j = textureSize ) {
		size_t index = 0;

		for ( size_t i = 0; i < _i; i++ ) {
			for ( size_t j = 0; j < _j; j++ ) {
				std::cout << mat( i, j ) << " ";
			}

			std::cout << std::endl;
		}
	}

};



}
}// end namespace



#endif