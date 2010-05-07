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

namespace isis
{
namespace extitk
{

typedef itk::TransformBase *TransformBasePointer;

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
		void );

	DeformationFieldType::Pointer getTransform(
		void );
	//here we setting up the temporaryDeformationField_ and deformationField_. The properties are defined be the templateImage which is specified by the setTemplateImage method,
	template <typename TImage> void setTemplateImage( const TImage *templateImage ) {
		imageRegion_ = templateImage->GetLargestPossibleRegion();
		deformationField_ = DeformationFieldType::New();
		deformationField_->SetRegions( imageRegion_.GetSize() );
		deformationField_->SetOrigin( templateImage->GetOrigin() );
		deformationField_->SetSpacing( templateImage->GetSpacing() );
		deformationField_->SetDirection( templateImage->GetDirection() );
		deformationField_->Allocate();
		temporaryDeformationField_ = DeformationFieldType::New();
		temporaryDeformationField_->SetRegions( imageRegion_.GetSize() );
		temporaryDeformationField_->SetOrigin( templateImage->GetOrigin() );
		temporaryDeformationField_->SetSpacing( templateImage->GetSpacing() );
		temporaryDeformationField_->SetDirection( templateImage->GetDirection() );
		temporaryDeformationField_->Allocate();
	}

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
