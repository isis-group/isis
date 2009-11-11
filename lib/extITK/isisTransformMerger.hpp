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
#include "itkAddImageFilter.h"

#include <list>

namespace isis {
namespace extitk {

typedef itk::TransformBase* TransformBasePointer;

class TransformMerger : public std::list<TransformBasePointer>
{
public:
	
    typedef TransformMerger Self;
    typedef std::list<TransformBasePointer> Superclass;
        
    typedef itk::Vector<float, 3> VectorType;
    typedef itk::Image<VectorType, 3> DeformationFieldType;
    typedef itk::ImageRegionIterator<DeformationFieldType> DeformationFieldIteratorType;

    typedef itk::MatrixOffsetTransformBase<double, 3, 3> MatrixOffsetTransformType;

    typedef itk::VersorRigid3DTransform<double> VersorRigid3DTransformType;
    typedef itk::AffineTransform<double, 3> AffineTransformType;
    typedef itk::BSplineDeformableTransform<double, 3, 3> BSplineDeformableTransformType;
	
	typedef itk::AddImageFilter<DeformationFieldType, DeformationFieldType, DeformationFieldType> AddImageFilterType;


    TransformMerger();

    void merge(
        void);

    DeformationFieldType::Pointer getTransform(
        void);

    void setTemplateImage(itk::ImageBase<3>::Pointer);

private:
    unsigned int transformType_;
    Self::iterator transformIterator_;
    MatrixOffsetTransformType::MatrixType temporaryMatrix_;
    MatrixOffsetTransformType::TranslationType temporaryTranslation_;
    MatrixOffsetTransformType::OffsetType temporaryOffset_;

    BSplineDeformableTransformType::Pointer tmpTransform_;
    BSplineDeformableTransformType::Pointer outputTransform_;
    
    DeformationFieldType::Pointer deformationField_;
    DeformationFieldType::Pointer temporaryDeformationField_;
    itk::ImageBase<3>::RegionType imageRegion_;
	
	AddImageFilterType::Pointer addImageFilter_;

};

}//end namespace itk
}//end namespace isis
