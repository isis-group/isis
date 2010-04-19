/*
 * isisTransformMerger.cpp
 *
 *  Created on: Oct 20, 2009
 *      Author: tuerke
 */
#ifndef ISISTRANSFORMMERGER3D_H_
#define ISISTRANSFORMMERGER3D_H_

#include "isisTransformMerger3D.hpp"

namespace isis {
namespace extitk {

TransformMerger3D::TransformMerger3D() {
    transformType_ = 0;
    tmpTransform_ = BSplineDeformableTransformType::New();
    outputTransform_ = BSplineDeformableTransformType::New();
    addImageFilter_ = AddImageFilterType::New();

}

void TransformMerger3D::merge(
    void) {
	
    DeformationFieldIteratorType fi(temporaryDeformationField_, imageRegion_);
	itk::Transform<double, 3, 3>::InputPointType fixedPoint;
	itk::Transform<double, 3, 3>::OutputPointType movingPoint;
	DeformationFieldType::IndexType index;
	VectorType displacement;
	
    //go through the transform list and create a vector deformation field (temporaryDeformationField_). Then combining this deformation field with the final vector field (deformationField_).
    for (transformIterator_ = this->begin(); transformIterator_ != this->end(); transformIterator_++) {
        fi.GoToBegin();
		itk::Transform<double, 3, 3>* transform =  dynamic_cast<itk::Transform<double, 3, 3>* >(*transformIterator_);
		while(!fi.IsAtEnd())
		{
			index = fi.GetIndex();
			temporaryDeformationField_->TransformIndexToPhysicalPoint(index, fixedPoint);
			movingPoint = transform->TransformPoint(fixedPoint);
			displacement = movingPoint - fixedPoint;
			fi.Set(displacement);
			++fi;	
		}
		addImageFilter_->SetInput1(deformationField_);
		addImageFilter_->SetInput2(temporaryDeformationField_);
		addImageFilter_->Update();
		deformationField_ = addImageFilter_->GetOutput();

    }

}

TransformMerger3D::DeformationFieldType::Pointer TransformMerger3D::getTransform(
    void) {
    return deformationField_;

}


}//end namespace extitk
}//end namespace isis

#endif //ISISTRANSFORMMERGER3D_H_
