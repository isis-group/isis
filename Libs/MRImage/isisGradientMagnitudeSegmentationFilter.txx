/*
 * isisFastMarchingSegmentationFilter.txx
 *
 *  Created on: Jun 24, 2009
 *      Author: tuerke
 */

#include "isisGradientMagnitudeSegmentationFilter.h"


namespace isis {


template< class TInputImage, class TOutputImage >
GradientMagnitudeSegmentationFilter< TInputImage, TOutputImage >
::GradientMagnitudeSegmentationFilter()
{

	m_OutputImage = OutputImageType::New();

	m_Smoothing = SmoothingFilterType::New();
	m_GradientMagnitude = GradientFilterType::New();
	m_MinMaxFilter = MinMaxFilterType::New();

	//standard parameters for smoothingfilter
	m_SmoothingTimeStep = 0.125;
	m_SmoothingNumberOfIterations = 5;
	m_SmoothingConductanceParameter = 9.0;


}

template< class TInputImage, class TOutputImage >
void
GradientMagnitudeSegmentationFilter< TInputImage, TOutputImage >
::CalculateMinMax( void )
{

	OutputPixelType temp;

	m_MinMaxFilter->SetImage( this->GetInput() );
	m_MinMaxFilter->Compute();
	m_MinOutput = m_MinMaxFilter->GetMinimum();
	m_MaxOutput = m_MinMaxFilter->GetMaximum();
	temp = m_MaxOutput - m_MinOutput;
	m_MinOutput = 0;
	m_MaxOutput = temp;
}
/*
template< class TInputImage, class TOutputImage >
void
GradientMagnitudeSegmentationFilter< TInputImage, TOutputImage >
::SetOutputParameters( void )
{
	m_InputImage = this->GetInput();
	m_InputRegion = m_InputImage->GetBufferedRegion();
	m_InputIndex = m_InputRegion.GetIndex();
	m_InputSize = m_InputRegion.GetSize();
	m_InputSpacing = m_InputImage->GetSpacing();
	m_InputOrigin = m_InputImage->GetOrigin();
	m_InputDirection = m_InputImage->GetDirection();

	m_OutputSize = m_InputSize;
	m_OutputDirection = m_InputDirection;
	m_OutputIndex = m_InputIndex;
	m_OutputSpacing = m_InputSpacing;
	m_OutputOrigin = m_InputOrigin;
	m_OutputImage->SetSpacing( m_OutputSpacing );
	m_OutputImage->SetOrigin( m_OutputOrigin );
	m_OutputImage->SetDirection( m_OutputDirection );
	m_OutputRegion.SetSize( m_OutputSize );
	m_OutputRegion.SetIndex( m_OutputIndex );

	m_OutputImage->SetRegions( m_OutputRegion );
	m_OutputImage->Allocate();
	this->GraftOutput( m_OutputImage );


}
*/


template <class TInputImage, class TOutputImage>
void
GradientMagnitudeSegmentationFilter <TInputImage,TOutputImage>
::GenerateInputRequestedRegion() throw(itk::InvalidRequestedRegionError)
{
  // call the superclass' implementation of this method. this should
  // copy the output requested region to the input requested region
  Superclass::GenerateInputRequestedRegion();

  // get pointers to the input and output
  typename Superclass::InputImagePointer  inputPtr =
    const_cast< TInputImage *>( this->GetInput() );

  if ( !inputPtr )
    {
    return;
    }


  // get a copy of the input requested region (should equal the output
  // requested region)
  typename TInputImage::RegionType inputRequestedRegion;
  inputRequestedRegion = inputPtr->GetRequestedRegion();


  // crop the input requested region at the input's largest possible region
  if ( inputRequestedRegion.Crop(inputPtr->GetLargestPossibleRegion()) )
    {
    inputPtr->SetRequestedRegion( inputRequestedRegion );
    return;
    }
  else
    {
    // Couldn't crop the region (requested region is outside the largest
    // possible region).  Throw an exception.

    // store what we tried to request (prior to trying to crop)
    inputPtr->SetRequestedRegion( inputRequestedRegion );

    // build an exception
    itk::InvalidRequestedRegionError e(__FILE__, __LINE__);
    e.SetLocation(ITK_LOCATION);
    e.SetDescription("Requested region is (at least partially) outside the largest possible region.");
    e.SetDataObject(inputPtr);
    throw e;
    }
}

template< class TInputImage, class TOutputImage >
void
GradientMagnitudeSegmentationFilter< TInputImage, TOutputImage >
::GenerateData()
{

	this->CalculateMinMax();

	//smoothing filter
	m_Smoothing->SetInput( this->GetInput() );
	m_Smoothing->SetTimeStep( m_SmoothingTimeStep );
	m_Smoothing->SetNumberOfIterations( m_SmoothingNumberOfIterations );
	m_Smoothing->SetConductanceParameter( m_SmoothingConductanceParameter );

	//gradient filter
	m_GradientMagnitude->SetInput( m_Smoothing->GetOutput() );
	m_GradientMagnitude->SetSigma( m_Sigma );

	m_GradientMagnitude->Update();
	m_OutputImage = m_GradientMagnitude->GetOutput();
	m_OutputImage->Update();
	this->GraftOutput( m_OutputImage );

}
/*
template< class TInputImage, class TOutputImage >
typename GradientMagnitudeSegmentationFilter< TInputImage, TOutputImage >::OutputImagePointer
GradientMagnitudeSegmentationFilter< TInputImage, TOutputImage >
::GetOutput( void )
{
	return m_OutputImage;
}

*/
} //end namespace isis

