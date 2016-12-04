#ifndef ITK_COMMON_HXX
#define ITK_COMMON_HXX

#include "common.hpp"
#include "itkAdapter.hpp"

#include <itkScaleTransform.h>
#include <itkResampleImageFilter.h>

namespace isis{

namespace itk4{

	template<typename TYPE> data::TypedImage<TYPE> resample_impl(data::TypedImage<TYPE> source,util::vector4<size_t> in_newsize){
		typedef itk::Image<TYPE,4> ImageType;
		
		auto scaleTransform = itk::ScaleTransform<double, 4>::New();

		itkAdapter adapter;
		itk::Size<4> newsize;
		std::copy(std::begin(in_newsize),std::end(in_newsize),&newsize[0]);
		
		auto image=adapter.makeItkImageObject<ImageType>( source );

		auto oldsize=image->GetLargestPossibleRegion().GetSize();
		auto newspacing=image->GetSpacing();
		auto neworigin=image->GetOrigin();
		
		for(int i=0;i<4;i++){
			const double scale=(double)oldsize[i]/newsize[i];
			newspacing[i]*=scale;
			neworigin[i]/=scale;
		}
		scaleTransform->SetCenter(image->GetOrigin());
		
		LOG(Runtime,info) << "resampling image to " << in_newsize << " voxels, new voxelsize will be " << newspacing;

		auto resampleFilter = itk::ResampleImageFilter<ImageType, ImageType>::New();
		resampleFilter->SetTransform(scaleTransform);
		resampleFilter->SetOutputSpacing(newspacing);
		resampleFilter->SetOutputOrigin(image->GetOrigin());
		resampleFilter->SetOutputDirection(image->GetDirection());
		resampleFilter->SetSize(newsize);
		resampleFilter->SetInput(image);
		resampleFilter->Update();

		return adapter.makeIsisImageObject<ImageType>(resampleFilter->GetOutput());
	}
}
}

#endif //ITK_COMMON_HXX
