/*
 * isisTransformMerger.cpp
 *
 *  Created on: Oct 20, 2009
 *      Author: tuerke
 */
#ifndef ISISTRANSFORMMERGER_H_
#define ISISTRANSFORMMERGER_H_

#include "isisTransformMerger.hpp"

namespace isis {
namespace extitk {

TransformMerger::TransformMerger() {
	outputType = 0;
	tmpTransform_ = BSplineDeformableTransformType::New();
	outputTransform_ = BSplineDeformableTransformType::New();

}

void TransformMerger::merge(
    void) {

	for(transformIterator_ = this->begin(); transformIterator_ != this->end(); transformIterator_++) {

		if(VersorRigid3DTransformType* transform = dynamic_cast<VersorRigid3DTransformType*> (*transformIterator_)) {
			outputType = 1;
			std::cout << transform->GetParameters() << std::endl;

			std::cout << tmpTransform_->GetParameters() << std::endl;
			std::cout << tmpTransform_->GetNumberOfParameters() << std::endl;
			//TODO versor rigid 3d type
		}
		if(AffineTransformType* transform = dynamic_cast<AffineTransformType*> (*transformIterator_)) {
			//TODO affine rigid 3d type
			outputType = 2;
			std::cout << transform->GetParameters() << std::endl;

		}
		if(BSplineDeformableTransformType* transform = dynamic_cast<BSplineDeformableTransformType*> (*transformIterator_)) {
			//TODO deformable transform type

		}
	}

}

TransformBasePointer TransformMerger::getTransform(
    void) {

}

}//end namespace extitk
}//end namespace isis

#endif //ISISTRANSFORMMERGER_H_
