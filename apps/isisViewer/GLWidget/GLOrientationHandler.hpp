#ifndef ORIENTATIONHANDLER_HPP
#define ORIENTATIONHANDLER_HPP

#include "ImageHolder.hpp"
#include "common.hpp"
#include <GL/glu.h>
#include <GL/gl.h>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/concept_check.hpp>

struct stat;
namespace isis
{
namespace viewer
{


class GLOrientationHandler
{
	static const unsigned short matrixSize;
public:
	typedef boost::numeric::ublas::matrix<float> MatrixType;
	enum PlaneOrientation { axial, sagittal, coronal };
	
	static MatrixType transformToPlaneView( const MatrixType &origMatrix, PlaneOrientation orientation, bool back=false );
	
	static void recalculateViewport( size_t w, size_t h, const ImageHolder &image, const MatrixType &orientation, GLint  *viewport, size_t border = 0 );
	
	
	//some helper functions
	static void boostMatrix2Pointer( const MatrixType &boostMatrix, GLdouble *pointerMatrix );
	
	static void addOffset( MatrixType &matrix );
	
};



}
}// end namespace



#endif