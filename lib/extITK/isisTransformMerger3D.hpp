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


#include "itkTransformBase.h"
#include "itkVersorRigid3DTransform.h"
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

class TransformMerger3D : public std::list<TransformBasePointer>
{
public:

	typedef TransformMerger3D Self;
	typedef std::list<TransformBasePointer> Superclass;

	typedef itk::Vector<float, 3> VectorType;
	typedef itk::Image<VectorType, 3> DeformationFieldType;
	typedef itk::ImageRegionIterator<DeformationFieldType> DeformationFieldIteratorType;

	typedef itk::MatrixOffsetTransformBase<double, 3, 3> MatrixOffsetTransformType;

	typedef itk::VersorRigid3DTransform<double> VersorRigid3DTransformType;
	typedef itk::AffineTransform<double, 3> AffineTransformType;
	typedef itk::BSplineDeformableTransform<double, 3, 3> BSplineDeformableTransformType;

	typedef itk::AddImageFilter<DeformationFieldType, DeformationFieldType, DeformationFieldType> AddImageFilterType;


	TransformMerger3D();

	void merge(
		void );

	DeformationFieldType::Pointer getTransform(
		void );


	//here we setting up the temporaryDeformationField_ and deformationField_. The properties are defined be the templateImage which is specified by the setTemplateImage method,
	template <typename TImage> void setTemplateImage( TImage *templateImage ) {
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
	itk::ImageBase<3>::RegionType imageRegion_;

	AddImageFilterType::Pointer addImageFilter_;

};

}//end namespace itk
}//end namespace isis
