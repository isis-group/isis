/*
 * isisTransformMerger.hpp
 *
 *  Created on: Oct 20, 2009
 *      Author: tuerke
 */

#include "itkTransformBase.h"
#include "itkRigid2DTransform.h"
#include "itkAffineTransform.h"
#include "itkBSplineDeformableTransform.h"
#include "itkImageRegistrationMethod.h"
#include "itkAddImageFilter.h"

#include <list>

namespace isis {
namespace extitk {

typedef itk::TransformBase* TransformBasePointer;

class TransformMerger2D : public std::list<TransformBasePointer>
{
public:
	
    typedef TransformMerger2D Self;
    typedef std::list<TransformBasePointer> Superclass;
        
    typedef itk::Vector<float, 2> VectorType;
    typedef itk::Image<VectorType, 2> DeformationFieldType;
    typedef itk::ImageRegionIterator<DeformationFieldType> DeformationFieldIteratorType;

    typedef itk::MatrixOffsetTransformBase<double, 2, 2> MatrixOffsetTransformType;

    typedef itk::Rigid2DTransform<double> Rigid2DTransformType;
    typedef itk::AffineTransform<double, 2> AffineTransformType;
    typedef itk::BSplineDeformableTransform<double, 2, 3> BSplineDeformableTransformType;
	
    typedef itk::AddImageFilter<DeformationFieldType, DeformationFieldType, DeformationFieldType> AddImageFilterType;


    TransformMerger2D();

    void merge(
        void);

    DeformationFieldType::Pointer getTransform(
        void);

    void setTemplateImage(itk::ImageBase<2>::Pointer);

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
    itk::ImageBase<2>::RegionType imageRegion_;
	
	AddImageFilterType::Pointer addImageFilter_;

};

}//end namespace itk
}//end namespace isis
