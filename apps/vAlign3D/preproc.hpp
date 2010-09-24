

#include "itkVnlFFTRealToComplexConjugateImageFilter.h"
#include "itkVnlFFTComplexConjugateToRealImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkCastImageFilter.h"

namespace isis { namespace registration { namespace _internal {

template<typename ImageType>
void filterFrequencyDomain( typename ImageType::Pointer image )
{
	typedef unsigned char MaskPixelType;
	typedef itk::Image< MaskPixelType, 3 > MaskImageType;
	typedef itk::VnlFFTRealToComplexConjugateImageFilter< typename ImageType::PixelType, 3 > FFTFilterType;
	typedef itk::VnlFFTComplexConjugateToRealImageFilter< typename ImageType::PixelType, 3 > IFFTFilterType;
	typedef typename FFTFilterType::OutputImageType SpectralImageType;
	typedef itk::CastImageFilter< ImageType, MaskImageType > CastFilterType;
	typedef itk::MaskImageFilter< SpectralImageType,
									MaskImageType,
									SpectralImageType > MaskFilterType;
	typename CastFilterType::Pointer caster = CastFilterType::New();
	caster->SetInput( image );
	caster->Update();
	MaskImageType::Pointer maskImage = caster->GetOutput();




	typename FFTFilterType::Pointer fftFilter = FFTFilterType::New();
	typename IFFTFilterType::Pointer ifftFilter = IFFTFilterType::New();
	typename MaskFilterType::Pointer maskFilter = MaskFilterType::New();

	fftFilter->SetInput( image );
	maskFilter->SetInput1( fftFilter->GetOutput() );
	maskFilter->SetInput2( maskImage );
	ifftFilter->SetInput( maskFilter->GetOutput() );
	ifftFilter->Update();
	image->DisconnectPipeline();
	image = ifftFilter->GetOutput();
}

}
}
}
