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

	float temp;

	m_MinMaxFilter->SetImage( this->GetInput() );
	m_MinMaxFilter->Compute();
	m_MinOutput = m_MinMaxFilter->GetMinimum();
	m_MaxOutput = m_MinMaxFilter->GetMaximum();
	temp = m_MaxOutput - m_MinOutput;
	m_MinOutput = 0;
	m_MaxOutput = temp;
}
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

	m_OutputSize = m_InputSize;
	m_OutputIndex = m_InputIndex;
	m_OutputSpacing = m_InputSpacing;
	m_OutputOrigin = m_InputOrigin;
	m_OutputImage->SetSpacing( m_OutputSpacing );
	m_OutputImage->SetOrigin( m_OutputOrigin );
	m_OutputRegion.SetSize( m_OutputSize );
	m_OutputRegion.SetIndex( m_OutputIndex );

	m_OutputImage->SetRegions( m_OutputRegion );
	m_OutputImage->Allocate();


}


template< class TInputImage, class TOutputImage >
void
GradientMagnitudeSegmentationFilter< TInputImage, TOutputImage >
::Update( void )
{

	this->CalculateMinMax();
	this->SetOutputParameters();
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


}

template< class TInputImage, class TOutputImage >
typename GradientMagnitudeSegmentationFilter< TInputImage, TOutputImage >::OutputImagePointer
GradientMagnitudeSegmentationFilter< TInputImage, TOutputImage >
::GetOutput( void )
{
	return m_OutputImage;
}


} //end namespace isis

