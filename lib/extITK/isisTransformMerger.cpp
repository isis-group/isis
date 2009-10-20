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

void TransformMerger::merge(
    void) {

	itk::TransformBase* tmpTransform;
	unsigned count = 0;
	for(transformIterator_ = this->begin(); transformIterator_ != this->end(); transformIterator_++) {

		if(!count)
			tmpTransform = *transformIterator_;
		if(VersorRigid3DTransformType* transform = dynamic_cast<VersorRigid3DTransformType*> (*transformIterator_)) {
			//TODO versor rigid 3d type
		}
		if(AffineTransformType* transform = dynamic_cast<AffineTransformType*> (*transformIterator_)) {
			//TODO affine rigid 3d type
		}
		if(BSplineDeformableTransformType* transform = dynamic_cast<BSplineDeformableTransformType*> (*transformIterator_)) {
			//TODO deformable transform type
		}

		count++;
	}

}

}
}

#endif //ISISTRANSFORMMERGER_H_
