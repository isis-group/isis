/****************************************************************
 *
 * Copyright (C) 2010 Max Planck Institute for Human Cognitive and Brain Sciences, Leipzig
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Author: Erik Tuerke, tuerke@cbs.mpg.de, 2010
 *
 *****************************************************************/


#ifndef ISISTRANSFORMMERGER3D_H_
#define ISISTRANSFORMMERGER3D_H_

#include "isisTransformMerger3D.hpp"

namespace isis
{
namespace extitk
{

TransformMerger3D::TransformMerger3D()
{
	transformType_ = 0;
	tmpTransform_ = BSplineDeformableTransformType::New();
	outputTransform_ = BSplineDeformableTransformType::New();
	addImageFilter_ = AddImageFilterType::New();
}

void TransformMerger3D::merge(
	void )
{
	DeformationFieldIteratorType fi( temporaryDeformationField_, imageRegion_ );
	itk::Transform<double, 3, 3>::InputPointType fixedPoint;
	itk::Transform<double, 3, 3>::OutputPointType movingPoint;
	DeformationFieldType::IndexType index;
	VectorType displacement;

	//go through the transform list and create a vector deformation field (temporaryDeformationField_). Then combining this deformation field with the final vector field (deformationField_).
	for ( transformIterator_ = this->begin(); transformIterator_ != this->end(); transformIterator_++ ) {
		fi.GoToBegin();
		itk::Transform<double, 3, 3>* transform =  dynamic_cast<itk::Transform<double, 3, 3>* >( *transformIterator_ );

		while ( !fi.IsAtEnd() ) {
			index = fi.GetIndex();
			temporaryDeformationField_->TransformIndexToPhysicalPoint( index, fixedPoint );
			movingPoint = transform->TransformPoint( fixedPoint );
			displacement = movingPoint - fixedPoint;
			fi.Set( displacement );
			++fi;
		}

		addImageFilter_->SetInput1( deformationField_ );
		addImageFilter_->SetInput2( temporaryDeformationField_ );
		addImageFilter_->Update();
		deformationField_ = addImageFilter_->GetOutput();
	}
}

TransformMerger3D::DeformationFieldType::Pointer TransformMerger3D::getTransform(
	void )
{
	return deformationField_;
}


}//end namespace extitk
}//end namespace isis

#endif //ISISTRANSFORMMERGER3D_H_
