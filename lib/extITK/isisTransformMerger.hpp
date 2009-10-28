/*
 * isisTransformMerger.hpp
 *
 *  Created on: Oct 20, 2009
 *      Author: tuerke
 */

#include "itkTransformBase.h"
#include "itkVersorRigid3DTransform.h"
#include "itkAffineTransform.h"
#include "itkBSplineDeformableTransform.h"
#include "itkImageRegistrationMethod.h"

#include <list>

namespace isis {
namespace extitk {

typedef itk::TransformBase* TransformBasePointer;

class TransformMerger : public std::list<TransformBasePointer>
{
public:

	typedef TransformMerger Self;
	typedef std::list<TransformBasePointer> Superclass;

	typedef itk::MatrixOffsetTransformBase<double, 3, 3> MatrixOffsetTransformType;

	typedef itk::VersorRigid3DTransform<double> VersorRigid3DTransformType;
	typedef itk::AffineTransform<double, 3> AffineTransformType;
	typedef itk::BSplineDeformableTransform<double, 3, 3> BSplineDeformableTransformType;

	TransformMerger();

	void merge(
	    void);

	TransformBasePointer getTransform(
	    void);

private:
	unsigned int outputType;
	Self::iterator transformIterator_;
	MatrixOffsetTransformType::MatrixType temporaryMatrix_;
	MatrixOffsetTransformType::TranslationType temporaryTranslation_;
	MatrixOffsetTransformType::OffsetType temporaryOffset_;

	BSplineDeformableTransformType::Pointer tmpTransform_;
	BSplineDeformableTransformType::Pointer outputTransform_;

};

}//end namespace itk
}//end namespace isis
