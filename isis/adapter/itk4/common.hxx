#ifndef ITK_COMMON_HXX
#define ITK_COMMON_HXX

#include "common.hpp"
#include "itkAdapter.hpp"

#include <itkScaleTransform.h>
#include <itkCenteredAffineTransform.h>
#include <itkResampleImageFilter.h>

namespace isis{

namespace itk4{
	
namespace _internal{
template<typename TYPE> typename itk::ResampleImageFilter<itk::Image<TYPE,4> , itk::Image<TYPE,4>>::Pointer
makeResampleFilter(itk::MatrixOffsetTransformBase< double, 4, 4 > *transform){
	auto ret=itk::ResampleImageFilter<itk::Image<TYPE,4> , itk::Image<TYPE,4>>::New();
	ret->SetTransform(transform);
	return ret;
}
template<typename TYPE> typename itk::ResampleImageFilter<itk::Image<TYPE,4> , itk::Image<TYPE,4>>::Pointer
makeResampleFilter(itk::MatrixOffsetTransformBase< double, 4, 4 > *transform, typename itk::Image<TYPE,4>::Pointer image){
	auto ret=makeResampleFilter<TYPE>(transform);
	ret->SetOutputOrigin(image->GetOrigin());
	ret->SetOutputDirection(image->GetDirection());
	ret->SetSize(image->GetLargestPossibleRegion().GetSize());
	ret->SetOutputSpacing(image->GetSpacing());
	ret->SetInput(image);
	return ret;
}
}

template<typename TYPE> data::TypedImage<TYPE> resample_impl(data::TypedImage<TYPE> source,util::vector4<size_t> in_newsize){
	typedef itk::Image<TYPE,4> ImageType;
	
	auto scaleTransform = itk::ScaleTransform<double, 4>::New();

	itkAdapter adapter;
	itk::Size<4> newsize;
	std::copy(std::begin(in_newsize),std::end(in_newsize),&newsize[0]);
	
	auto image=adapter.makeItkImageObject<ImageType>( source );

	auto oldsize=image->GetLargestPossibleRegion().GetSize();
	auto newspacing=image->GetSpacing();
	
	for(int i=0;i<4;i++){
		const double scale=(double)oldsize[i]/newsize[i];
		newspacing[i]*=scale;
	}
	scaleTransform->SetCenter(image->GetOrigin());
	
	LOG(Runtime,info) << "resampling image to " << in_newsize << " voxels, new voxelsize will be " << newspacing;

	auto resampleFilter = _internal::makeResampleFilter<TYPE>(scaleTransform.GetPointer(),image);
	resampleFilter->SetOutputSpacing(newspacing);
	resampleFilter->SetSize(newsize);
	resampleFilter->Update();

	return adapter.makeIsisImageObject<ImageType>(resampleFilter->GetOutput());
}

template<typename TYPE> data::TypedImage<TYPE> rotate_impl(data::TypedImage<TYPE> source,std::pair<int,int> rotation_plain, float angle, bool pix_center){
	typedef itk::Image<TYPE,4> ImageType;
	itkAdapter adapter;
	auto image=adapter.makeItkImageObject<ImageType>( source );
	
	// the rotation is done in real space, not (as we want it) in image space
	// so we have to "fake" the image into real space
	typename ImageType::DirectionType orientation=image->GetInverseDirection(),ident; 
	const auto origin = image->GetOrigin();

	ident.SetIdentity();
	image->SetDirection(ident);
	
	if(pix_center){
		const auto size=image->GetLargestPossibleRegion().GetSize();
		const auto spacing=image->GetSpacing();
		typename ImageType::PointType orig(0);

		for(int i=0;i<3;i++)
			orig[i]=size[i]*-spacing[i]*.5;

		image->SetOrigin(orig);
	} else 
		image->SetOrigin(typename ImageType::DirectionType(orientation.GetInverse())*origin);
	
	auto transform = itk::AffineTransform< double, 4 >::New(); 
	
	LOG(Runtime,info) << "rotating image by " << util::MSubject(std::to_string(angle)+"rad") << " on the " << char('x'+rotation_plain.first) << "/" << char('x'+rotation_plain.second) << " plane around " << image->GetOrigin(); 

	transform->Rotate(rotation_plain.first,rotation_plain.second,angle);
	
	auto resampleFilter = _internal::makeResampleFilter<TYPE>(transform.GetPointer(),image);
	resampleFilter->Update();
	
	//restore old geometry
	image = resampleFilter->GetOutput();
	image->SetDirection(orientation);
	image->SetOrigin(origin);
	
	return adapter.makeIsisImageObject<ImageType>(image);
}

template<typename TYPE> data::TypedImage<TYPE> translate_impl(data::TypedImage<TYPE> source,util::fvector3 translation){
	typedef itk::Image<TYPE,4> ImageType;
	itkAdapter adapter;
	auto image=adapter.makeItkImageObject<ImageType>( source );
	
	// the rotation is done in real space, not (as we want it) in image space
	// so we have to "fake" the image into real space
	typename ImageType::DirectionType orientation=image->GetInverseDirection(),ident; 

	ident.SetIdentity();
	image->SetDirection(ident);
	
	auto transform = itk::AffineTransform< double, 4 >::New(); 
	itk::AffineTransform< double, 4 >::OutputVectorType transl(0.0);
	
	for(int i=0;i<3;i++)
		transl[i]=translation[i];
	
	LOG(Runtime,info) << "translating image by " << translation; 

	transform->Translate(transl);
	
	auto resampleFilter = _internal::makeResampleFilter<TYPE>(transform.GetPointer(),image);
	resampleFilter->Update();
	
	//restore old geometry
	image = resampleFilter->GetOutput();
	image->SetDirection(orientation);
	
	return adapter.makeIsisImageObject<ImageType>(image);
}

}
}

#endif //ITK_COMMON_HXX
