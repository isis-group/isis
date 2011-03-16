#ifndef ORIENTATIONHANDLER_HPP
#define ORIENTATIONHANDLER_HPP

#include "ImageHolder.hpp"
#include "common.hpp"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>

namespace isis {
namespace viewer {
	
	
class OrientationHandler
{
public:
	typedef boost::numeric::ublas::matrix<float> MatrixType;
	enum PlaneOrientation { axial, sagittal, coronal };
	typedef util::FixedVector< util::FixedVector<float, 3>,4> VertexMatrix;
	
	static size_t getSliceIndex( const ImageHolder &image, PlaneOrientation orientation );
	static size_t getNumberOfSlices( const ImageHolder &image, PlaneOrientation orientation );
	static util::FixedVector<float, 3> getNormalizedScaling( const ImageHolder &image );
	static MatrixType getOrientationMatrix( const ImageHolder &image, PlaneOrientation orientation, bool scaling = true );
	static MatrixType transformMatrix( MatrixType origMatrix, PlaneOrientation orientation );
	static MatrixType orientation2TextureMatrix( const MatrixType &origMatrix );
	
	static void  boostMatrix2Pointer( MatrixType boostMatrix, float *ret );
	
	static void printMatrix( const float *mat )
	{
		size_t index = 0;
		for (size_t i = 0; i<4; i++ )
		{
			for (size_t j = 0; j<4; j++ )
			{
				std::cout << mat[index++] << " ";
			}
		std::cout << std::endl;
		}
	}
	
};
	
	
	
}}// end namespace



#endif