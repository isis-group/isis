/*
 * isisTransformMerger.cpp
 *
 *  Created on: Oct 20, 2009
 *      Author: tuerke
 */
#ifndef ISISTRANSFORMMERGER2D_H_
#define ISISTRANSFORMMERGER2D_H_

#include "isisTransformMerger2D.hpp"

namespace isis {
namespace extitk {

TransformMerger2D::TransformMerger2D() {
    transformType_ = 0;
    tmpTransform_ = BSplineDeformableTransformType::New();
    outputTransform_ = BSplineDeformableTransformType::New();
    addImageFilter_ = AddImageFilterType::New();

}

void TransformMerger2D::merge(
    void) {
	
    DeformationFieldIteratorType fi(temporaryDeformationField_, imageRegion_);
	itk::Transform<double, 2, 2>::InputPointType fixedPoint;
	itk::Transform<double, 2, 2>::OutputPointType movingPoint;
	DeformationFieldType::IndexType index;
	VectorType displacement;
	
    //go through the transform list and create a vector deformation field (temporaryDeformationField_). Then combining this deformation field with the final vector field (deformationField_).
    for (transformIterator_ = this->begin(); transformIterator_ != this->end(); transformIterator_++) {
        fi.GoToBegin();
		itk::Transform<double, 2, 2>* transform =  dynamic_cast<itk::Transform<double, 2, 2>* >(*transformIterator_);
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

TransformMerger2D::DeformationFieldType::Pointer TransformMerger2D::getTransform(
    void) {
    return deformationField_;

}



}//end namespace extitk
}//end namespace isis

#endif //ISISTRANSFORMMERGER2D_H_
