/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2016  Enrico Reimer <reimer@cbs.mpg.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "../core/data/image.hpp"
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/lu.hpp>

namespace isis
{

namespace math
{
namespace _internal{
template <typename TYPE>
bool inverseMatrix( const boost::numeric::ublas::matrix<TYPE> &inMatrix, boost::numeric::ublas::matrix<TYPE> &inverse )
{
	boost::numeric::ublas::matrix<TYPE> A( inMatrix );
	boost::numeric::ublas::permutation_matrix<TYPE> pm( A.size1() );

	if( boost::numeric::ublas::lu_factorize( A, pm ) != 0 ) {
		return false;
	}

	inverse.assign( boost::numeric::ublas::identity_matrix<TYPE>( inMatrix.size1() ) );
	boost::numeric::ublas::lu_substitute( A, pm, inverse );
	return true;
}

bool transformCoords( isis::util::PropertyMap& propertyObject, const isis::util::vector4< size_t > size, boost::numeric::ublas::matrix< float > transform, bool transformCenterIsImageCenter = false );
}

/**
 * Transforms the image coordinate system into an other system by multiplying
 * the orientation matrix with a user defined transformation matrix. Additionally,
 * the index origin will be transformed into the new coordinate system. This
 * function only changes the
 *
 * <B>IMPORTANT!</B>: If you call this function with a matrix other than the
 * identidy matrix, it's not guaranteed that the image is still in ISIS space
 * according to the DICOM conventions. Maybe some ISIS algorithms that
 * depend on correct image orientations won't work as expected. Use this method
 * with caution!
 */
bool transformCoords(isis::data::Chunk& chk, boost::numeric::ublas::matrix<float> transform_matrix, bool transformCenterIsImageCenter = false );
/**
 * Transforms the image coordinate system into an other system by multiplying
 * the orientation matrix with a user defined transformation matrix. Additionally,
 * the index origin will be transformed into the new coordinate system. This
 * function only changes the orientation information (rowVec, columnVec, sliceVec, indexOrigin)
 * of the image but will not change the image itself.
 *
 * \warning If you call this function with a matrix other than the
 * identidy matrix, it's not guaranteed that the image is still in ISIS space
 * according to the DICOM conventions. Eventuelly some ISIS algorithms that
 * depend on correct image orientations won't work as expected. Use this method
 * with caution!
 * \param transform_matrix the transformation matrix can be any type of rigid and affine transformation
 * \param transformCenterIsImageCenter if this parameter is true, the center of the image will be translated to the
 *  isocenter of the scanner prior applying the transform_matrix. Afterwards, it will be translated to its
 *  initial position. For example this is the way SPM flips its images when converting from DICOM to nifti.
 * \return returns if the transformation was successfuly
 */
bool transformCoords(isis::data::Image& img, boost::numeric::ublas::matrix< float > transform_matrix, bool transformCenterIsImageCenter = false );

/** Maps the given scanner Axis to the dimension with the minimal angle.
*  This is done by latching the orientation of the image by setting the biggest absolute
*  value of each orientation vector to 1 and the others to 0.
*  Example:
*          (-0.8)      (1)
*          ( 0.2)  ->  (0)   (this is done for the rowVec, columnVec and sliceVec)
*          (-0.1)      (0)
*
*  This latched orientation is used to map from the scanner axes to the dimension.
*  \param scannerAxes the axes of the scanner you want to map to dimension of the image.
*  \return the mapped image dimension
*/

isis::data::dimensions mapScannerAxisToImageDimension ( const isis::data::Image& img, isis::data::scannerAxis scannerAxes );

}
}

#endif // TRANSFORM_H
