/*
 * timeStepExtractorFilter.cxx
 *
 *  Created on: May 15, 2009
 *      Author: tuerke
 */
#ifndef TIMESTEPEXTRACTOR_TXX
#define TIMESTEPEXTRACTOR_TXX

#include "timeStepExtractorFilter.h"

namespace isis {


template< class TImageInput, class TImageOutput >
TimeStepExtractorFilter< TImageInput, TImageOutput >
::TimeStepExtractorFilter()
{
	m_InputDimension = InputImageType::GetImageDimension();
	m_OutputDimension = OutputImageType::GetImageDimension();
}



template< class TImageInput, class TImageOutput >
void
TimeStepExtractorFilter< TImageInput, TImageOutput >
::Start( void )
{
	typename ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
	m_InputRegion = m_InputImage->GetLargestPossibleRegion();
	ImageSizeType inputImageSize = m_InputRegion.GetSize();
	ImageIndexType inputImageStart = m_InputRegion.GetIndex();
	//this collapses the last dimension
	inputImageSize[m_OutputDimension] = 0;
	inputImageStart[m_OutputDimension] = m_sliceNumber;
	m_OutputRegion.SetSize( inputImageSize );
	m_OutputRegion.SetIndex( inputImageStart );
	extractFilter->SetExtractionRegion( m_OutputRegion );
	extractFilter->SetInput( m_InputImage );
	try
	{
		extractFilter->Update();
	}
	catch( itk::ExceptionObject & err )
	{
		std::cerr << "ExceptionObject caught !" << std::endl;
		std::cerr << err << std::endl;
	}
	m_OutputImage = extractFilter->GetOutput();
}


//setter methods
template< class TImageInput, class TImageOutput >
void
TimeStepExtractorFilter< TImageInput, TImageOutput >
::SetSliceNumber( unsigned int number )
{
	m_sliceNumber = number;
}

template< class TImageInput, class TImageOutput >
void
TimeStepExtractorFilter< TImageInput, TImageOutput >
::SetInputImage( Self::InputImagePointer inputImage )
{
	m_InputImage = inputImage;
	m_InputRegion = m_InputImage->GetLargestPossibleRegion();
	m_InputImageSize = m_InputRegion.GetSize();
	m_NumberOfTimeSteps = m_InputImageSize[m_InputDimension - 1];
}


//getter methods
template< class TImageInput, class TImageOutput >
unsigned int
TimeStepExtractorFilter< TImageInput, TImageOutput >
::GetSliceNumber( void )
{
	return m_sliceNumber;
}

template< class TImageInput, class TImageOutput >
typename TimeStepExtractorFilter< TImageInput, TImageOutput >::OutputImagePointer
TimeStepExtractorFilter< TImageInput, TImageOutput >
::GetOutputImage( void )
{
		return m_OutputImage;
}

template< class TImageInput, class TImageOutput >
unsigned int
TimeStepExtractorFilter< TImageInput, TImageOutput >
::GetNumberOfTimeSteps( void )
{
	return m_NumberOfTimeSteps;
}


}
#endif //TIMESTEPEXTRACTOR_TXX
