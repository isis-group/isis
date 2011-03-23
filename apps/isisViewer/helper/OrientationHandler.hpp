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

	static size_t getNumberOfSlices( const ImageHolder &image, PlaneOrientation orientation );
	static util::FixedVector<float, 3> getNormalizedScaling( const ImageHolder &image );
	static MatrixType getOrientationMatrix( const ImageHolder &image, PlaneOrientation orientation, bool scaling = true );
	static MatrixType transformMatrix( MatrixType origMatrix, PlaneOrientation orientation );
	static MatrixType orientation2TextureMatrix( const MatrixType &origMatrix );
	static ViewPortCoords calculateViewPortCoords( size_t w, size_t h );
	static ViewerCoordinates normalizeCoordinates(size_t slice, size_t x, size_t y, const ImageHolder &image, const float *textureMatrix, ViewPortCoords viewport, PlaneOrientation orientation);
	
	template <typename T>
	static util::FixedVector<T,4> transformWithImageOrientation(const isis::viewer::ImageHolder& image, util::FixedVector<T,4> oldVec)
	{
		util::fvector4 sliceVec = image.getPropMap().getPropertyAs<util::fvector4>("sliceVec");
		util::fvector4 rowVec = image.getPropMap().getPropertyAs<util::fvector4>("rowVec");
		util::fvector4 columnVec = image.getPropMap().getPropertyAs<util::fvector4>("columnVec");	
		boost::numeric::ublas::matrix<T> orient = boost::numeric::ublas::zero_matrix<T>(3,3);
		boost::numeric::ublas::matrix<T> coords = boost::numeric::ublas::zero_matrix<T>(3,1);
		boost::numeric::ublas::matrix<T> retCoords = boost::numeric::ublas::zero_matrix<T>(3,1);
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
		util::FixedVector<T,4> retVec;
		retVec[0] = retCoords(0,0);
		retVec[1] = retCoords(1,0);
		retVec[2] = retCoords(2,0);
		return retVec;
	}
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